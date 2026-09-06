#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <vector>

#include "helpers.h"
#include "sat.h"
#include "larman_dedup.h"

// Cube-and-conquer для {0,1}^n: координатор + SAT-оракулы.
//
// Воркеры отвечают ровно на один вопрос: «красится ли этот граф за таймер».
// Вся логика дерева перебора — у однопоточного координатора.
//
// Препроцесс: V сортируется по max_elimination, бинпоиском ищется граница lo
// (минимальный сброс верхушки, при котором start + V[lo..] красится).
// Чанк = V[0 .. lo+margin), граница выравнивается по смене elimination-счёта
// (чтобы remaining был инвариантен относительно stab(start) — иначе дедуп
// сравнивает неизоморфные графы). Remaining фиксирован навсегда.
//
// Узел дерева = (base, pos). Жизненный цикл:
//   A1 (attempt 1): base ∪ chunk[pos..] ∪ remaining. SAT → prune поддерева.
//   A1 UNSAT → диспатчим A2 и рожаем детей (base+v, pos+1) — параллельно.
//   A2 (attempt 2): base ∪ remaining — ожидаемо SAT (инвариант «нет 00»).
//   A2 UNSAT → ретрай с tl_retry. Снова UNSAT → АНОМАЛИЯ: лог + файл.
//   Каскада пушей нет: чанк жирный, аномалия — научное событие, не рутина.
//
// Дедуп: при рождении ребёнка глубины ≤ dedup_depth конфигурация проверяется
// по seen-set (орбита нового представителя вставляется целиком, ключ — набор
// добавленных вершин). Глубже — без дедупа.

struct CncParams {
    int n = 0;
    int k = 0;
    int ncolors = 0;              // 0 → n+1
    int dedup_depth = 3;
    int chunk_margin = 100;       // чанк = бинпоиск-граница + margin
    int n_workers = 8;
    int tl = 1;                   // сек, обычные вызовы
    int tl_retry = 10;            // сек, ретрай A2 («ебланские случаи»)
    int tl_binsearch = 30;        // сек, пробы бинпоиска
    int snapshot_period_s = 300;
    int checkpoint_period_s = 1800;  // дамп стека+results+seen каждые N сек
    bool probe_only = false;      // только сортировка + бинпоиск + статистика
    bool resume = false;          // продолжить с чекпоинта, если он есть
    std::string kissat_path;
    std::string filename_prefix;
    std::string anomaly_file;
    std::string checkpoint_file;  // пусто → <filename_prefix>_ckpt.bin
};

class CncSolver {
public:
    explicit CncSolver(const CncParams& params) : P(params) {
        if (P.ncolors == 0) P.ncolors = P.n + 1;
        if (P.checkpoint_file.empty())
            P.checkpoint_file = P.filename_prefix + "_ckpt.bin";
    }

    void run(const std::vector<int>& start_base,
             const std::vector<int>& candidates,
             const std::vector<FuncPtr>& filters) {
        t_start_ = std::chrono::steady_clock::now();
        start_ = start_base;
        start_len_ = start_base.size();
        filters_ = filters;

        std::cout << "CncSolver: n=" << P.n << " k=" << P.k
                  << " ncolors=" << P.ncolors
                  << " workers=" << P.n_workers
                  << " tl=" << P.tl << "/" << P.tl_retry
                  << " dedup_depth=" << P.dedup_depth
                  << " margin=" << P.chunk_margin << std::endl;

        V_ = reorder_by_max_elimination(candidates, 0, (int)candidates.size(), P.k);
        std::cout << "|V|=" << V_.size() << " (sorted by max_elimination)" << std::endl;

        if (P.probe_only) {
            if (binsearch_boundary()) {
                std::cout << "probe done: chunk=" << chunk_end_
                          << " remaining=" << V_.size() - chunk_end_ << std::endl;
            }
            return;
        }

        seen_.reserve(1 << 22);
        bool resumed = P.resume && try_load_checkpoint();
        if (!resumed) {
            if (!binsearch_boundary()) return;  // полный граф покрасился — кейс закрыт
            Job root;
            root.base = start_;
            root.pos = 0;
            root.kind = KIND_A1;
            lo_.push_back(std::move(root));
            nodes_created_ = 1;
        }

        ops_ = larman_stab(start_, P.n);
        std::cout << "|stab(start)|=" << ops_.size() << std::endl;

        cur_job_.resize(P.n_workers);
        cur_busy_.assign(P.n_workers, false);

        std::vector<std::thread> ws;
        ws.reserve(P.n_workers);
        for (int w = 0; w < P.n_workers; w++) {
            ws.emplace_back(&CncSolver::worker, this, w);
        }
        coordinator();
        for (auto& th : ws) th.join();
        write_checkpoint();  // финальный дамп (при естественном завершении — пустой стек)
        final_report();
    }

private:
    enum { KIND_A1 = 0, KIND_A2 = 1, KIND_A2R = 2 };

    struct Job {
        std::vector<int> base;
        int pos = 0;      // для A1: откуда начинается хвост чанка
        int kind = KIND_A1;
    };
    struct Result {
        Job job;
        bool sat = false;
    };

    CncParams P;
    std::vector<int> start_;
    size_t start_len_ = 3;
    std::vector<FuncPtr> filters_;
    std::vector<int> V_;
    int chunk_end_ = 0;
    std::vector<int> chunk_;      // V_[0..chunk_end_)
    std::vector<int> remaining_;  // V_[chunk_end_..)

    std::vector<std::pair<int, std::vector<int>>> ops_;
    std::unordered_set<uint64_t> seen_;

    // Общение координатора и воркеров
    std::mutex m_;
    std::condition_variable cv_work_, cv_res_;
    std::deque<Job> hi_;      // A2 и ретраи — первыми (аномалии всплывают сразу)
    std::vector<Job> lo_;     // A1 новых узлов, LIFO ≈ DFS
    std::vector<Result> res_;
    int in_flight_ = 0;
    bool shutdown_ = false;
    std::vector<Job> cur_job_;     // что сейчас в работе у каждого воркера
    std::vector<bool> cur_busy_;   // (для чекпоинта: in-flight не должны теряться)

    // Счётчики (трогает только координатор, кроме sat_calls_total из sat.cpp)
    long long nodes_created_ = 0;
    long long nodes_pruned_ = 0;
    long long dedup_skipped_ = 0;
    long long results_handled_ = 0;
    long long anomalies_ = 0;
    std::map<int, int> results_;
    std::chrono::steady_clock::time_point t_start_, t_last_snapshot_;

    // ---------- препроцесс ----------

    int elim_count(int v) const {
        int c = 0;
        for (int u : V_) if (u != v && dist(v, u) > P.k) c++;
        return c;
    }

    bool solve_inline(const std::vector<int>& verts, int tl, const std::string& fname) {
        std::string cmd = P.kissat_path + " " + fname + " -q --time=" + std::to_string(tl);
        return is_colorable(verts, start_len_, P.ncolors, P.k, P.n, fname, cmd);
    }

    // false → полный граф покрасился, кейс закрыт
    bool binsearch_boundary() {
        const std::string fname = P.filename_prefix + "_bs.cnf";
        auto graph_from = [&](int m) {
            std::vector<int> tail(V_.begin() + m, V_.end());
            return prepare_to_color(start_, tail, 0, {}, 0, P.n, P.k, filters_);
        };

        auto t0 = std::chrono::steady_clock::now();
        bool full_sat = solve_inline(graph_from(0), P.tl_binsearch, fname);
        std::cout << "binsearch probe m=0 |g|=" << V_.size() + start_len_
                  << " -> " << (full_sat ? "SAT" : "UNSAT") << std::endl;
        if (full_sat) {
            std::cout << "FULL GRAPH COLORABLE — case closed, nothing to do" << std::endl;
            return false;
        }

        int lo = 0, hi = (int)V_.size();  // V[hi..] пусто → start один → SAT
        while (lo + 1 < hi) {
            int mid = (lo + hi) / 2;
            bool sat = solve_inline(graph_from(mid), P.tl_binsearch, fname);
            std::cout << "binsearch probe m=" << mid << " -> "
                      << (sat ? "SAT" : "UNSAT") << std::endl;
            if (sat) hi = mid; else lo = mid;
        }
        int boundary = hi;

        chunk_end_ = std::min((int)V_.size(), boundary + P.chunk_margin);
        // Выравнивание границы по смене elimination-счёта: remaining должен быть
        // setwise-инвариантен относительно stab(start), иначе дедуп-скипы сравнивают
        // неизоморфные A1/A2-графы.
        while (chunk_end_ < (int)V_.size() && chunk_end_ > 0 &&
               elim_count(V_[chunk_end_]) == elim_count(V_[chunk_end_ - 1])) {
            chunk_end_++;
        }
        chunk_.assign(V_.begin(), V_.begin() + chunk_end_);
        remaining_.assign(V_.begin() + chunk_end_, V_.end());

        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - t0).count();
        std::cout << "binsearch done: boundary=" << boundary
                  << " chunk=" << chunk_end_ << " (margin " << P.chunk_margin
                  << " + tie-alignment), remaining=" << remaining_.size()
                  << " (" << ms << " ms)" << std::endl;
        return true;
    }

    // ---------- дедуп ----------

    static uint64_t pack_addset(const std::vector<int>& sorted_add) {
        uint64_t key = 1;  // сентинель отделяет длины
        for (int v : sorted_add) key = (key << 12) | (uint64_t)v;
        return key;
    }

    // true → конфигурация уже видена (скип)
    bool dedup_check_insert(const std::vector<int>& base) {
        size_t depth = base.size() - start_len_;
        if (depth == 0 || depth > (size_t)P.dedup_depth) return false;
        std::vector<int> add(base.begin() + start_len_, base.end());
        std::sort(add.begin(), add.end());
        uint64_t key = pack_addset(add);
        if (seen_.count(key)) return true;
        // Предохранитель по памяти: перестаём вставлять новые орбиты, дедуп
        // деградирует в "ловим только уже вставленное" — корректность не страдает,
        // поздние дубликаты просто пересчитаются.
        if (seen_.size() > SEEN_CAP) {
            if (!seen_cap_hit_) {
                seen_cap_hit_ = true;
                std::cout << "WARNING: dedup seen-set cap reached ("
                          << seen_.size() << " keys), new orbits not inserted"
                          << std::endl;
            }
            return false;
        }
        seen_.insert(key);
        std::vector<int> tmp(add.size());
        for (const auto& [x, perm] : ops_) {
            for (size_t i = 0; i < add.size(); i++)
                tmp[i] = permute_bits(add[i] ^ x, perm, P.n);
            std::sort(tmp.begin(), tmp.end());
            seen_.insert(pack_addset(tmp));
        }
        return false;
    }

    static constexpr size_t SEEN_CAP = 60000000;  // ~2.5-3 GB
    bool seen_cap_hit_ = false;

    // ---------- воркер: SAT-оракул ----------

    void worker(int wid) {
        const std::string fname =
            P.filename_prefix + "_w" + std::to_string(wid) + ".cnf";
        const std::string cmd_tl =
            P.kissat_path + " " + fname + " -q --time=" + std::to_string(P.tl);
        const std::string cmd_retry =
            P.kissat_path + " " + fname + " -q --time=" + std::to_string(P.tl_retry);

        std::unique_lock<std::mutex> lk(m_);
        while (true) {
            cv_work_.wait(lk, [&] { return shutdown_ || !hi_.empty() || !lo_.empty(); });
            if (shutdown_) return;
            Job j;
            if (!hi_.empty()) { j = std::move(hi_.front()); hi_.pop_front(); }
            else { j = std::move(lo_.back()); lo_.pop_back(); }
            in_flight_++;
            cur_job_[wid] = j;
            cur_busy_[wid] = true;
            lk.unlock();

            std::vector<int> g;
            if (j.kind == KIND_A1) {
                g = prepare_to_color(j.base, chunk_, j.pos, remaining_, 0, P.n, P.k, filters_);
            } else {
                g = prepare_to_color(j.base, {}, 0, remaining_, 0, P.n, P.k, filters_);
            }
            bool sat = is_colorable(g, start_len_, P.ncolors, P.k, P.n, fname,
                                    j.kind == KIND_A2R ? cmd_retry : cmd_tl);

            lk.lock();
            in_flight_--;
            cur_busy_[wid] = false;
            res_.push_back({std::move(j), sat});
            cv_res_.notify_one();
        }
    }

    // ---------- координатор ----------

    void coordinator() {
        t_last_snapshot_ = std::chrono::steady_clock::now();
        std::unique_lock<std::mutex> lk(m_);
        cv_work_.notify_all();
        while (true) {
            bool all_idle = res_.empty() && hi_.empty() && lo_.empty() && in_flight_ == 0;
            if (all_idle) break;
            cv_res_.wait_for(lk, std::chrono::seconds(1), [&] { return !res_.empty(); });

            std::vector<Result> batch;
            batch.swap(res_);
            lk.unlock();

            std::vector<Job> new_hi, new_lo;
            for (auto& r : batch) handle(r, new_hi, new_lo);
            maybe_snapshot();
            maybe_checkpoint();

            lk.lock();
            for (auto& j : new_hi) hi_.push_back(std::move(j));
            for (auto& j : new_lo) lo_.push_back(std::move(j));
            if (!hi_.empty() || !lo_.empty()) cv_work_.notify_all();
        }
        shutdown_ = true;
        cv_work_.notify_all();
    }

    void handle(Result& r, std::vector<Job>& new_hi, std::vector<Job>& new_lo) {
        results_handled_++;
        int sz = (int)r.job.base.size();

        if (r.job.kind == KIND_A1) {
            if (r.sat) {
                results_[1000 + sz]++;
                nodes_pruned_++;
                return;
            }
            // A2 этого узла — приоритетно
            Job a2;
            a2.base = r.job.base;
            a2.kind = KIND_A2;
            new_hi.push_back(std::move(a2));
            // Дети (в обратном порядке: первый ребёнок наверху LIFO)
            std::vector<Job> kids;
            for (int p = r.job.pos; p < chunk_end_; p++) {
                int v = V_[p];
                if (!is_allowed(v, r.job.base, P.n, P.k, filters_)) continue;
                std::vector<int> child_base = r.job.base;
                child_base.push_back(v);
                if (dedup_check_insert(child_base)) { dedup_skipped_++; continue; }
                nodes_created_++;
                Job c;
                c.base = std::move(child_base);
                if (p + 1 >= chunk_end_) {
                    c.kind = KIND_A2;  // хвост пуст: A1-граф == A2-графу
                    new_hi.push_back(std::move(c));
                } else {
                    c.kind = KIND_A1;
                    c.pos = p + 1;
                    kids.push_back(std::move(c));
                }
            }
            for (auto it = kids.rbegin(); it != kids.rend(); ++it)
                new_lo.push_back(std::move(*it));
            return;
        }

        if (r.job.kind == KIND_A2) {
            if (r.sat) { results_[sz]++; return; }
            Job retry = r.job;
            retry.kind = KIND_A2R;
            new_hi.push_back(std::move(retry));
            return;
        }

        // KIND_A2R
        if (r.sat) { results_[2000 + sz]++; return; }
        anomalies_++;
        results_[3000 + sz]++;
        std::ostringstream os;
        os << "ANOMALY: base+remaining UNSAT after retry: [";
        for (int v : r.job.base) os << v << ", ";
        os << "]";
        std::cout << os.str() << std::endl;
        if (!P.anomaly_file.empty()) {
            std::ofstream f(P.anomaly_file, std::ios::app);
            for (int v : r.job.base) f << v << " ";
            f << "\n";
        }
    }

    void maybe_snapshot() {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - t_last_snapshot_).count()
            < P.snapshot_period_s) return;
        t_last_snapshot_ = now;
        long long up = std::chrono::duration_cast<std::chrono::seconds>(now - t_start_).count();
        size_t hi_sz, lo_sz;
        int fly;
        {
            std::lock_guard<std::mutex> lk(m_);
            hi_sz = hi_.size(); lo_sz = lo_.size(); fly = in_flight_;
        }
        std::cout << "--- SNAPSHOT t=" << up << "s"
                  << " nodes=" << nodes_created_
                  << " pruned=" << nodes_pruned_
                  << " dedup_skipped=" << dedup_skipped_
                  << " seen=" << seen_.size()
                  << " handled=" << results_handled_
                  << " sat_calls=" << sat_calls_total.load()
                  << " queues=" << hi_sz << "/" << lo_sz << "/" << fly
                  << " anomalies=" << anomalies_ << std::endl;
        std::cout << "results:";
        for (auto& [key, cnt] : results_) std::cout << " " << key << ":" << cnt;
        std::cout << std::endl;
        std::cout.flush();
    }

    // ---------- чекпоинт ----------
    // Формат (бинарный, той же машины): magic, [n,k,|V|,chunk_end],
    // счётчики, results, pending-джобы (стек + in-flight), seen-ключи.

    std::chrono::steady_clock::time_point t_last_ckpt_ = std::chrono::steady_clock::now();

    void maybe_checkpoint() {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - t_last_ckpt_).count()
            < P.checkpoint_period_s) return;
        t_last_ckpt_ = now;
        write_checkpoint();
    }

    void write_checkpoint() {
        auto t0 = std::chrono::steady_clock::now();
        std::vector<Job> pending;
        {
            std::lock_guard<std::mutex> lk(m_);
            pending.reserve(hi_.size() + lo_.size() + P.n_workers);
            for (auto& j : hi_) pending.push_back(j);
            for (auto& j : lo_) pending.push_back(j);
            for (int w = 0; w < (int)cur_busy_.size(); w++)
                if (cur_busy_[w]) pending.push_back(cur_job_[w]);  // in-flight перезапустятся
        }
        std::string tmp = P.checkpoint_file + ".tmp";
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        if (!f.is_open()) {
            std::cout << "CHECKPOINT ERROR: cannot open " << tmp << std::endl;
            return;
        }
        auto w64 = [&](uint64_t v) { f.write((const char*)&v, 8); };
        auto w32 = [&](int32_t v) { f.write((const char*)&v, 4); };
        w64(0xC3C30001ULL);
        w32(P.n); w32(P.k); w32((int32_t)V_.size()); w32(chunk_end_);
        w64((uint64_t)nodes_created_); w64((uint64_t)nodes_pruned_);
        w64((uint64_t)dedup_skipped_); w64((uint64_t)anomalies_);
        w64(results_.size());
        for (auto& [key, cnt] : results_) { w32(key); w32(cnt); }
        w64(pending.size());
        for (auto& j : pending) {
            w32(j.kind); w32(j.pos); w32((int32_t)j.base.size());
            for (int v : j.base) w32(v);
        }
        w64(seen_.size());
        for (uint64_t key : seen_) w64(key);
        f.close();
        std::rename(tmp.c_str(), P.checkpoint_file.c_str());
        long long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - t0).count();
        std::cout << "CHECKPOINT: " << pending.size() << " jobs, "
                  << results_.size() << " result-keys, " << seen_.size()
                  << " seen (" << ms << " ms)" << std::endl;
    }

    bool try_load_checkpoint() {
        std::ifstream f(P.checkpoint_file, std::ios::binary);
        if (!f.is_open()) {
            std::cout << "no checkpoint at " << P.checkpoint_file
                      << " — starting fresh" << std::endl;
            return false;
        }
        auto r64 = [&]() { uint64_t v = 0; f.read((char*)&v, 8); return v; };
        auto r32 = [&]() { int32_t v = 0; f.read((char*)&v, 4); return v; };
        if (r64() != 0xC3C30001ULL) {
            std::cout << "CHECKPOINT: bad magic, ignoring" << std::endl;
            return false;
        }
        int cn = r32(), ck = r32(), cv = r32(), cend = r32();
        if (cn != P.n || ck != P.k || cv != (int)V_.size()) {
            std::cout << "CHECKPOINT: params mismatch (n/k/|V|), ignoring" << std::endl;
            return false;
        }
        chunk_end_ = cend;
        chunk_.assign(V_.begin(), V_.begin() + chunk_end_);
        remaining_.assign(V_.begin() + chunk_end_, V_.end());
        nodes_created_ = (long long)r64();
        nodes_pruned_ = (long long)r64();
        dedup_skipped_ = (long long)r64();
        anomalies_ = (long long)r64();
        uint64_t nres = r64();
        for (uint64_t i = 0; i < nres; i++) {
            int key = r32(); int cnt = r32();
            results_[key] = cnt;
        }
        uint64_t njobs = r64();
        for (uint64_t i = 0; i < njobs; i++) {
            Job j;
            j.kind = r32(); j.pos = r32();
            int bl = r32();
            j.base.resize(bl);
            for (int b = 0; b < bl; b++) j.base[b] = r32();
            if (j.kind == KIND_A1) lo_.push_back(std::move(j));
            else hi_.push_back(std::move(j));
        }
        uint64_t nseen = r64();
        seen_.reserve(nseen + (1 << 20));
        for (uint64_t i = 0; i < nseen; i++) seen_.insert(r64());
        if (!f) {
            std::cout << "CHECKPOINT: truncated file, refusing resume" << std::endl;
            hi_.clear(); lo_.clear(); seen_.clear(); results_.clear();
            return false;
        }
        std::cout << "RESUMED: chunk=" << chunk_end_
                  << " jobs=" << njobs << " (hi=" << hi_.size() << " lo=" << lo_.size()
                  << ") nodes=" << nodes_created_
                  << " pruned=" << nodes_pruned_
                  << " seen=" << seen_.size()
                  << " anomalies=" << anomalies_ << std::endl;
        return true;
    }

    void final_report() {
        long long up = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::steady_clock::now() - t_start_).count();
        std::cout << "\n=== FINAL REPORT (t=" << up << "s) ===" << std::endl;
        std::cout << "chunk=" << chunk_end_ << " remaining=" << remaining_.size()
                  << " |stab|=" << ops_.size() << std::endl;
        std::cout << "nodes=" << nodes_created_
                  << " pruned=" << nodes_pruned_
                  << " dedup_skipped=" << dedup_skipped_
                  << " sat_calls=" << sat_calls_total.load()
                  << " anomalies=" << anomalies_ << std::endl;
        std::cout << "results:" << std::endl;
        for (auto& [key, cnt] : results_) std::cout << key << ": " << cnt << std::endl;
        if (anomalies_ == 0) {
            std::cout << "no anomalies: invariant 'base+remaining colors' held everywhere"
                      << std::endl;
        }
    }
};
