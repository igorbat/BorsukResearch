#include <optional>
#include <set>
#include <utility>
#include <cassert>
#include <algorithm>  // Для std::min
// ... other includes
#include "../include/helpers.h"

struct Conf {
    std::vector<int> vertexes_in_conf;
    std::vector<int> remaining_vertices;

    Conf(const std::vector<int>& verts, const std::vector<int>& rem)
        : vertexes_in_conf(verts), remaining_vertices(rem) {}
};

struct ConfQueue {
    std::vector<Conf> queue;
    size_t cur_idx = 0;

    void add(const Conf& c) {
        queue.push_back(c);
    }
};

class BruteforceSolver {
public:
    BruteforceSolver(
        std::map<int, int> res,
        ConfQueue& q,
        const std::vector<FuncPtr>& filts,
        const std::string& cmd,
        const std::string& fn,
        int nn,
        int kk,
        size_t ttip,
        int chsz
    ) : results(res), que(q), filters(filts), command(cmd), filename(fn), n(nn), k(kk), 
        start_len(ttip), chunk_size(chsz), coloringsb(0) {

            std::cout << "created a new task: cmd: " << command << "; filename: " << filename << std::endl;
        }

    void run() {
        ops = all_perms_optimised(que.queue[0].vertexes_in_conf, n);
        staged_bruteforce();

        std::cout << "results: " <<std::endl;

        for (auto& [k, v]: results) {
            std::cout << k << ": " << v << std::endl;
        }

        std::cout << "already seen: (size: count): " <<std::endl;
        for (auto& [k, v]: seen_constructions) {
            std::cout << k << ": " << v << std::endl;
        }


        std::cout << "queue: " <<std::endl;
        for (auto& conf: que.queue) {
            std::cout << "[";
            for (auto v: conf.vertexes_in_conf) {
                std::cout << v << ", ";
            }
            std::cout << "] others_left: ";
            std::cout << conf.remaining_vertices.size();
            std::cout << std::endl;
        }

    }

private:
    // Метод Z: хэширует отсортированные add_verts, если size <= 64/n
    std::optional<uint64_t> Z(const std::vector<int>& add_verts) const {
        size_t num = add_verts.size();
        if (num == 0) return std::nullopt;  // Не применимо для 0
        int max_pack = 64 / n;
        if (static_cast<int>(num) > max_pack) return std::nullopt;

        std::vector<int> sorted_add = add_verts;  // Копируем и сортируем
        std::sort(sorted_add.begin(), sorted_add.end());

        uint64_t hash = 0;
        for (size_t i = 0; i < num; ++i) {
            uint64_t vert_bits = static_cast<uint64_t>(sorted_add[i]) & ((1ULL << n) - 1);  // Маскируем на n бит
            hash |= (vert_bits << (i * n));
        }
        return hash;
    }

    void staged_bruteforce() {
        while (que.cur_idx < que.queue.size()) {
            auto& current = que.queue[que.cur_idx];
            // "we must finish brute force in advance of the last chunk"
            std::vector<int> real_rem;
            std::copy_if(current.remaining_vertices.begin(), current.remaining_vertices.end(), std::back_inserter(real_rem),
                     [this](int num) { 
                         return is_allowed(num, que.queue[que.cur_idx].vertexes_in_conf, n, k, filters);
                     });
            size_t rem_size = real_rem.size();
            if ( rem_size == 0 ) {
                std::cout << "FAILED TO FINISH WITHIN A BLOCK" << std::endl;
                std::cout << "The last constuction is:" << std::endl;
                std::cout << "[";
                for (auto v: que.queue[que.cur_idx].vertexes_in_conf) {
                    std::cout << v << ", ";
                }
                return;
            }
            // Переупорядочиваем весь remaining_vertices
            std::vector<int> reordered = reorder_vertices(
                current.vertexes_in_conf,
                real_rem,
                0,
                static_cast<int>(rem_size)
            );
            // Разделяем на vertexes_to_work_on и after_chunk
            size_t work_size = std::min(static_cast<size_t>(chunk_size), rem_size);
            std::vector<int> vertexes_to_work_on(reordered.begin(), reordered.begin() + work_size);
            std::vector<int> after_chunk(reordered.begin() + work_size, reordered.end());

            bruteforce_nolimits(
                current.vertexes_in_conf, 
                vertexes_to_work_on,
                0,
                after_chunk
            );
            que.cur_idx += 1;
        }
    }

    void bruteforce_nolimits(
        std::vector<int> base_v, 
        const std::vector<int>& vertexes_to_work_on,
        size_t id_in_work,
        const std::vector<int>& remaining_v
    ) {
        //bool evil_flag = false;
        bool evil_flag = true;
        // if ( coloringsb > 1000000 ) evil_flag = true;
        // color all
        if (coloringsb % 1000 == 0) {
            std::cout << "Colored for now bruteforce: " << coloringsb << " queue state: " << que.cur_idx << "/" << que.queue.size() << std::endl;
            // std::cout.flush();
        }

        // Новая логика дедупликации
        if (base_v.size() > static_cast<size_t>(start_len)) {
            std::vector<int> add_verts(base_v.begin() + start_len, base_v.end());
            std::sort(add_verts.begin(), add_verts.end());
            auto maybe_hash = Z(add_verts);
            if (maybe_hash) {
                std::pair<int, uint64_t> key{static_cast<int>(add_verts.size()), *maybe_hash};
                if (seen_configs.count(key)) {
                    seen_constructions[add_verts.size()]++;
                    return;  // Скип bruteforce
                }
                // Если нет — добавляем эквивалентные
                std::vector<int> start_verts(base_v.begin(), base_v.begin() + start_len);
                for (const auto& op : ops) {
                    int a = op.first;
                    const auto& perm = op.second;
                    std::vector<int> new_add = add_verts;  // Копируем
                    for (int& vert : new_add) {
                        vert = permute_bits(vert ^ a, perm, n);
                    }
                    std::sort(new_add.begin(), new_add.end());
                    auto new_hash = Z(new_add);
                    if (new_hash) {
                        std::pair<int, uint64_t> new_key{static_cast<int>(add_verts.size()), *new_hash};
                        seen_configs.insert(new_key);
                    }
                }
                // Добавляем оригинальный ключ
                seen_configs.insert(key);
            }
            // Если Z не применимо — продолжаем без дедупа
        }


        bool is_c;
        if (id_in_work < vertexes_to_work_on.size()) {
            coloringsb++;
            is_c = is_colorable(
                prepare_to_color(base_v, vertexes_to_work_on, id_in_work, remaining_v, 0, n, k, filters),  // id_in_poten=0
                start_len,
                n + 1,
                k,
                filename,
                command
            );
            if (is_c) {
                results[1000 + base_v.size()]++;
                return;
            }
        }


        coloringsb++;
        // auto vv = prepare_to_color(base_v, {}, 0, remaining_v, 0, n, k, filters);
        // for (auto i: vv) std::cout << i << " " ; std::cout << std::endl;
        // assert(false);

        // std::cout << start_len << " " << n + 1 << " " << k << " " << filename << " " << command << std::endl;
        // assert(false);
        is_c = is_colorable(
            prepare_to_color(base_v, {}, 0, remaining_v, 0, n, k, filters),  // id_in_poten=0
            start_len,
            n + 1,
            k,
            filename,
            command
        );
        if (is_c) {
            results[base_v.size()]++;
        } else {
            Conf new_conf(base_v, remaining_v);  // весь remaining_v, без среза
            que.queue.push_back(new_conf);
            if (evil_flag) { std::cout << "new constr by v: "; for (size_t i = start_len; i < base_v.size(); i++) std::cout << base_v[i] << " " ; std::cout << std::endl; }
            if (que.queue.size() > 200000 && que.queue.size() % 100 == 0) { std::cout << "new constr by v: "; for (size_t i = start_len; i < base_v.size(); i++) std::cout << base_v[i] << " " ; std::cout << std::endl; }
        }

        std::vector<int> new_base(base_v.begin(), base_v.end());
        new_base.push_back(0);
        for (size_t i = id_in_work; i < vertexes_to_work_on.size(); i++) {
            if (!is_allowed(vertexes_to_work_on[i], base_v, n, k, filters)) {
                continue;
            }
            new_base[new_base.size() - 1] = vertexes_to_work_on[i];
            bruteforce_nolimits(
                new_base, 
                reorder_vertices(
                    new_base,
                    vertexes_to_work_on,
                    i+1,
                    static_cast<int>(vertexes_to_work_on.size())
                ),
                0,
                remaining_v
            );
        }
    }

    std::map<int, int>& results;
    std::map<int, int> seen_constructions;
    std::vector<std::pair<int, std::vector<int>>> ops;
    ConfQueue& que;
    std::vector<FuncPtr> filters;
    std::string command;
    std::string filename;
    int n;
    int k;
    size_t start_len;
    int chunk_size;
    int coloringsb;

    std::set<std::pair<int, uint64_t>> seen_configs;  // для дедупа
};
