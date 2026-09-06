#include <set>
#include <map>
#include <vector>
#include <string>
#include <cstdio>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cassert>
#include <functional>

#include "../include/helpers.h"

signed long long colorings = 0;

using FuncPtr = bool (*)(int, const std::vector<int>&, int, int);

int dist(int v1, int v2) {
    int xor_result = v1 ^ v2;
    int count = 0;
    while (xor_result) {
        count += xor_result & 1;
        xor_result >>= 1;
    }
    return count;
}

bool is_inside(int vertex, const std::vector<int>& vertexes, int n, int k) {
    for (auto v: vertexes) {
        int dst = dist(v, vertex);
        if (dst > k || dst == 0) {
            return false;
        }
    }
    return true;
}

std::vector<int> bitmask(int value, int n) {
    std::vector<int> ans(n, 0);
    int idx = n - 1;
    while (value) {
        ans[idx] = value & 1;
        value >>= 1;
        idx--;
    }
    return ans;
}

std::vector<int> bitmask_sum(const std::vector<int>& bitmask1, const std::vector<int>& bitmask2) {
    std::vector<int> result(bitmask1.size());
    for (size_t i = 0; i < bitmask1.size(); i++) {
        result[i] = bitmask1[i] + bitmask2[i];
    }
    return result;
}

std::vector<int> bitmask_minus(const std::vector<int>& bitmask1, const std::vector<int>& bitmask2) {
    std::vector<int> result(bitmask1.size());
    for (size_t i = 0; i < bitmask1.size(); i++) {
        result[i] = bitmask1[i] - bitmask2[i];
    }
    return result;
}

std::vector<int> build_bitmask(const std::vector<int>& values, int n) {
    std::vector<int> result(n, 0);
    for (int value : values) {
        result = bitmask_sum(result, bitmask(value, n));
    }
    return result;
}

bool is_not_created_k3(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    int ln = graph_vertexes.size();
    for (int i1 = 0; i1 < ln; ++i1) {
        int v1 = graph_vertexes[i1];
        for (int i2 = i1 + 1; i2 < ln; ++i2) {
            int v2 = graph_vertexes[i2];
            if (dist(v1, v2) != k) continue;
            if (dist(v, v1) == k && dist(v, v2) == k) return false;
        }
    }
    return true;
}

std::vector<bool> which_types_k4_with_v(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    assert(k == 6);
    std::vector<bool> answer(4, false);
    int ln = graph_vertexes.size();
    for (int i1 = 0; i1 < ln; ++i1) {
        int v1 = graph_vertexes[i1];
        for (int i2 = i1 + 1; i2 < ln; ++i2) {
            int v2 = graph_vertexes[i2];
            if (dist(v1, v2) != k) continue;
            for (int i3 = i2 + 1; i3 < ln; ++i3) {
                int v3 = graph_vertexes[i3];
                if (dist(v1, v3) != k || dist(v2, v3) != k) continue;
                if (dist(v, v1) != k || dist(v, v2) != k || dist(v, v3) != k) continue;
                std::vector<int> mask = build_bitmask({v1 ^ v1, v2 ^ v1, v3 ^ v1, v ^ v1}, n);
                int zeros = 0;
                for (int x : mask) if (x == 0) ++zeros;
                assert(zeros < std::min(12, n) - 8);
                if (n > 12) {
                    answer[3 - (zeros - n + std::min(12, n))] = true;
                } else {
                    answer[3 - (12 - n) - zeros] = true;
                }
            }
        }
    }
    return answer;
}

bool is_not_created_k4_4(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    std::vector<bool> flgs = which_types_k4_with_v(v, graph_vertexes, n, k);
    return !flgs[3];
}

bool is_not_created_k4_4_or_3(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    std::vector<bool> flgs = which_types_k4_with_v(v, graph_vertexes, n, k);
    return !(flgs[3] || flgs[2]);
}

bool is_not_created_k4_4_or_3_or_2(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    std::vector<bool> flgs = which_types_k4_with_v(v, graph_vertexes, n, k);
    return !(flgs[3] || flgs[2] || flgs[1]);
}

bool is_not_created_k4(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    int ln = graph_vertexes.size();
    for (int i1 = 0; i1 < ln; ++i1) {
        int v1 = graph_vertexes[i1];
        for (int i2 = i1 + 1; i2 < ln; ++i2) {
            int v2 = graph_vertexes[i2];
            if (dist(v1, v2) != k) continue;
            for (int i3 = i2 + 1; i3 < ln; ++i3) {
                int v3 = graph_vertexes[i3];
                if (dist(v1, v3) != k || dist(v2, v3) != k) continue;
                if (dist(v, v1) == k && dist(v, v2) == k && dist(v, v3) == k) return false;
            }
        }
    }
    return true;
}

bool is_not_created_k5(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    int ln = graph_vertexes.size();
    for (int i1 = 0; i1 < ln; ++i1) {
        int v1 = graph_vertexes[i1];
        for (int i2 = i1 + 1; i2 < ln; ++i2) {
            int v2 = graph_vertexes[i2];
            if (dist(v1, v2) != k) continue;
            for (int i3 = i2 + 1; i3 < ln; ++i3) {
                int v3 = graph_vertexes[i3];
                if (dist(v1, v3) != k || dist(v2, v3) != k) continue;
                for (int i4 = i3 + 1; i4 < ln; ++i4) {
                    int v4 = graph_vertexes[i4];
                    if (dist(v1, v4) != k || dist(v2, v4) != k || dist(v3, v4) != k) continue;
                    if (dist(v, v1) == k && dist(v, v2) == k && dist(v, v3) == k && dist(v, v4) == k) return false;
                }
            }
        }
    }
    return true;
}

// K5 types for k=6, n=12:
// type 0 (K'_5):     zeros=2, fours=0  bitmask [0,0,2,2,2,2,2,2,3,3,3,3]
// type 1 (K''_5):    zeros=1, fours=1  bitmask [0,1,1,2,2,2,2,2,2,3,3,4]
// type 2 (K'''_5):   zeros=1, fours=0  bitmask [0,1,1,1,2,2,2,3,3,3,3,3]
// type 3 (K''''_5):  zeros=0, fours=2  bitmask [1,1,1,1,2,2,2,2,2,2,4,4]
// type 4 (K'''''_5): zeros=0, fours=1  bitmask [1,1,1,1,1,2,2,2,3,3,3,4]
std::vector<bool> which_types_k5_with_v(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    assert(k == 6);
    std::vector<bool> answer(5, false);
    int ln = graph_vertexes.size();
    for (int i1 = 0; i1 < ln; ++i1) {
        int v1 = graph_vertexes[i1];
        for (int i2 = i1 + 1; i2 < ln; ++i2) {
            int v2 = graph_vertexes[i2];
            if (dist(v1, v2) != k) continue;
            for (int i3 = i2 + 1; i3 < ln; ++i3) {
                int v3 = graph_vertexes[i3];
                if (dist(v1, v3) != k || dist(v2, v3) != k) continue;
                for (int i4 = i3 + 1; i4 < ln; ++i4) {
                    int v4 = graph_vertexes[i4];
                    if (dist(v1, v4) != k || dist(v2, v4) != k || dist(v3, v4) != k) continue;
                    if (dist(v, v1) != k || dist(v, v2) != k || dist(v, v3) != k || dist(v, v4) != k) continue;
                    std::vector<int> mask = build_bitmask({v1 ^ v1, v2 ^ v1, v3 ^ v1, v4 ^ v1, v ^ v1}, n);
                    int zeros = 0;
                    int fours = 0;
                    for (int x : mask) {
                        if (x == 0) ++zeros;
                        if (x == 4) ++fours;
                    }
                    if (zeros == 2 && fours == 0) answer[0] = true;
                    else if (zeros == 1 && fours == 1) answer[1] = true;
                    else if (zeros == 1 && fours == 0) answer[2] = true;
                    else if (zeros == 0 && fours == 2) answer[3] = true;
                    else if (zeros == 0 && fours == 1) answer[4] = true;
                }
            }
        }
    }
    return answer;
}

bool is_not_created_k5_5(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    std::vector<bool> flgs = which_types_k5_with_v(v, graph_vertexes, n, k);
    return !flgs[4];
}

bool is_not_created_k5_5_or_4(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    std::vector<bool> flgs = which_types_k5_with_v(v, graph_vertexes, n, k);
    return !(flgs[4] || flgs[3]);
}

bool is_not_created_k5_5_or_4_or_3(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    std::vector<bool> flgs = which_types_k5_with_v(v, graph_vertexes, n, k);
    return !(flgs[4] || flgs[3] || flgs[2]);
}

bool is_not_created_k5_5_or_4_or_3_or_2(int v, const std::vector<int>& graph_vertexes, int n, int k) {
    std::vector<bool> flgs = which_types_k5_with_v(v, graph_vertexes, n, k);
    return !(flgs[4] || flgs[3] || flgs[2] || flgs[1]);
}

bool is_allowed(int v, const std::vector<int>& graph_vertexes, int n, int k, const std::vector<FuncPtr>& functions) {
    for (auto f: functions) {
        if (!f(v, graph_vertexes, n, k)) {
            return false;
        }
    }
    return true;
}

bool call_binary(std::string command) {
    // std::cout << "Will be running command: "<<  command << std::endl;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
	throw std::runtime_error("failed to call kissat");
    }

    char buffer[128];
    std::string response;
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        response += buffer;
    }
    //std::cout<< "binary call: " << response << std::endl;
    colorings += 1;
    if (colorings % 1000 == 0) {
        std::cout << "real colorings happeneed: " << colorings << std::endl;
    }
    std::cout << "response length: " << response.length() << std::endl;
    return response.find(" SATISFIABLE") != std::string::npos;
}
///////////////////////////////////////////////
// Кодирование переменной CNF: вершина v (индекс), цвет c (1-based)
int encode(int v_index, int c, int n_vertices) {
    return v_index * n_vertices + c;
}

bool is_colorable(const std::vector<int>& vertices, int tip, int ncolors, int k, std::string filename, std::string command_to_run) {
    std::vector<std::vector<int>> clauses;
    int n_vertices = vertices.size();
    int col = 1;
    
    for (int i = 0; i < tip; i++) {
        clauses.push_back({encode(i, col, ncolors)});
        ++col;
    }

    // 2. Условия для рёбер (вершины, соединённые ребром, не могут иметь одинаковый цвет)
    for (size_t i = 0; i < vertices.size(); ++i) {
        for (size_t j = i + 1; j < vertices.size(); ++j) {
            if (dist(vertices[i], vertices[j]) == k) {
                for (int c = 1; c <= ncolors; ++c) {
                    clauses.push_back({-encode(i, c, ncolors), -encode(j, c, ncolors)});
                }
            }
        }
    }

    // 3. Каждая вершина имеет хотя бы один цвет
    for (size_t i = 0; i < vertices.size(); ++i) {
        std::vector<int> clause;
        for (int c = 1; c <= ncolors; ++c) {
            clause.push_back(encode(i, c, ncolors));
        }
        clauses.push_back(clause);
    }

    // Запись в файл в формате DIMACS
    std::ofstream out(filename, std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }

    int n_vars = n_vertices * ncolors;
    out << "p cnf " << n_vars << " " << clauses.size() << "\n";
    for (const auto& clause : clauses) {
        for (int lit : clause) {
            out << lit << " ";
        }
        out << "0\n";
    }
    out.close();

    return call_binary(command_to_run);
}

// std::vector<int> filter_vertices(const std::vector<int>& base_v, const std::vector<int>& poten_v, int from, const std::vector<int>& poten_v2, int from2, int n, int k, const std::vector<FuncPtr>& functions) {
//     std::vector<int> result(base_v);
//     for (int i = from; i < poten_v.size(); i++) {
//         if (is_allowed(poten_v[i], base_v, n, k, functions)) {
//             result.push_back(poten_v[i]);
//         }
//     }
//     for (int i = from2; i < poten_v2.size(); i++) {
//         if (is_allowed(poten_v2[i], base_v, n, k, functions)) {
//             result.push_back(poten_v2[i]);
//         }
//     }
//     return result;
// }

std::vector<int> reorder_by_sum_distance(
    const std::vector<int>& base_v,
    const std::vector<int>& poten_v, int from, int to)
{
    std::vector<std::pair<int, int>> vctr(to - from , {0, 0});
    for (int i = from; i < to; i++) {
        vctr[i - from].second = poten_v[i];
        for (auto vv: base_v) {
            vctr[i - from].first -= dist(vv, poten_v[i]);
        }
    }
    sort(vctr.begin(), vctr.end());
    std::vector<int> result(vctr.size());
    std::transform(vctr.begin(), vctr.end(), result.begin(),
                   [](const std::pair<int, int>& p) { return p.second; });
    return result;
}

std::vector<int> reorder_by_max_elimination(
    const std::vector<int>& base_v,
    const std::vector<int>& poten_v, int from, int to, int k)
{
    // Для каждого кандидата считаем, сколько других кандидатов он убьёт
    // (т.е. окажутся на расстоянии > k от base_v + кандидат)
    int sz = to - from;
    std::vector<std::pair<int, int>> vctr(sz);
    for (int i = from; i < to; i++) {
        int candidate = poten_v[i];
        int eliminates = 0;
        for (int j = from; j < to; j++) {
            if (i == j) continue;
            if (dist(candidate, poten_v[j]) > k) {
                eliminates++;
            }
        }
        vctr[i - from] = {-eliminates, candidate};  // минус — чтобы sort дал наибольшие первыми
    }
    sort(vctr.begin(), vctr.end());
    std::vector<int> result(sz);
    std::transform(vctr.begin(), vctr.end(), result.begin(),
                   [](const std::pair<int, int>& p) { return p.second; });
    return result;
}

std::vector<int> reorder_vertices(
    const std::vector<int>& base_v,
    const std::vector<int>& poten_v, int from, int to,
    const std::string& strategy,
    int k)
{
    if (strategy == "max_elimination") {
        return reorder_by_max_elimination(base_v, poten_v, from, to, k);
    }
    return reorder_by_sum_distance(base_v, poten_v, from, to);
}

std::vector<int> prepare_to_color(
    const std::vector<int>& base_v,
    const std::vector<int>& brute_v,
    size_t from1,
    const std::vector<int>& other_v,
    size_t from2,
    int n, int k, const std::vector<FuncPtr>& functions

) {
    std::vector<int> to_color;
    to_color.reserve(base_v.size() + brute_v.size() + other_v.size());
    to_color.insert(to_color.end(), base_v.begin(), base_v.end());
    for (size_t i = from1; i < brute_v.size(); i++) {
        if (is_allowed(brute_v[i], base_v, n,k,functions)) {
            to_color.push_back(brute_v[i]);
        }
    }
    for (size_t i = from2; i < other_v.size(); i++) {
        if (is_allowed(other_v[i], base_v, n,k,functions)) {
            to_color.push_back(other_v[i]);
        }
    }
    return to_color;
}

// void bruteforce_nolimits(
//     std::map<int, int>& results,
//     ConfQueue& que,
//     std::vector<int> base_v, 
//     const std::vector<int>& vertexes_to_work_on,
//     int id_in_work,
//     const std::vector<int>& poten_v,
//     int id_in_poten,
//     const std::vector<FuncPtr>& filters,
//     const std::string& command,
//     const std::string& filename,
//     int n,
//     int k,
//     int tip
// ) {
//     // color all
//     if (colorings % 1000 == 0) {
//         std::cout << "Colored for now: " << colorings << " queue state: " << que.cur_idx << "/" << que.queue.size() << std::endl;
//         std::cout.flush();
//     }

//     bool is_c;
//     if (id_in_work < vertexes_to_work_on.size()) {
//         is_c = is_colorable(
//             prepare_to_color(base_v, vertexes_to_work_on, id_in_work, poten_v, id_in_poten,n,k,filters),
//             tip,
//             n+1,
//             k,
//             filename,
//             command
//         );
//         if (is_c) {
//             results[base_v.size()]++;
//             return;
//         }
//     }


//     is_c = is_colorable(
//         prepare_to_color(base_v, {}, 0, poten_v, id_in_poten,n,k,filters),
//         tip,
//         n+1,
//         k,
//         filename,
//         command
//     );
//     if (is_c) {
//         results[base_v.size()]++;
//     } else {
//         Conf new_conf(base_v, que.queue[que.cur_idx].start_len, id_in_poten); 
//         que.queue.push_back(new_conf);
//     }

//     std::vector<int> new_base(base_v.begin(), base_v.end());
//     new_base.push_back(0);
//     for (int i = id_in_work; i < vertexes_to_work_on.size(); i++) {
//         if (!is_allowed(vertexes_to_work_on[i], base_v, n, k, filters)) {
//             continue;
//         }
//         new_base[new_base.size() - 1] = vertexes_to_work_on[i];
//         bruteforce_nolimits(
//             results,
//             que,
//             new_base, 
//             vertexes_to_work_on,
//             i + 1,
//             poten_v,
//             id_in_poten,
//             filters,
//             command,
//             filename,
//             n,
//             k,
//             tip
//         );
//     }

// }


// void staged_bruteforce(
//     std::map<int, int>& results,
//     ConfQueue& que,
//     const std::vector<int>& poten_v,
//     const std::vector<FuncPtr>& filters,
//     const std::string& command,
//     const std::string& filename,
//     int n,
//     int k,
//     int tip,
//     int chunk_size) 
// {
//     while (que.cur_idx < que.queue.size()) {
//         // "we must finish brute force in advance of the last chunk"
//         if (que.queue[que.cur_idx].idx_in_all_v + chunk_size + 3 > poten_v.size()) {
//             std::cout << "COLORINGS: " << colorings << std::endl;

//             std::cout << "results: " <<std::endl;

//             for (auto& [k, v]: results) {
//                 std::cout << k << ": " << v << std::endl;
//             }

//             std::cout << "queue: " <<std::endl;
//             for (auto& conf: que.queue) {
//                 std::cout << "[";
//                 for (auto v: conf.vertexes_in_conf) {
//                     std::cout << v << ", ";
//                 }
//                 std::cout << "] idx in others: " << conf.idx_in_all_v << std::endl;
//             }
//             // "we must finish brute force in advance of the last chunk"
//             std::cout << "que.queue[que.cur_idx].idx_in_all_v + chunk_size > poten_v.size()" << std::endl;
//             std::cout <<  que.queue[que.cur_idx].idx_in_all_v << " " << chunk_size << " " << poten_v.size() << std::endl;
//             assert(false);
//         }
//         bruteforce_nolimits(
//             results,
//             que,
//             que.queue[que.cur_idx].vertexes_in_conf, 
//             reorder_vertices(
//                 que.queue[que.cur_idx].vertexes_in_conf,
//                 poten_v,
//                 que.queue[que.cur_idx].idx_in_all_v,
//                 que.queue[que.cur_idx].idx_in_all_v + chunk_size
//             ),
//             0,
//             poten_v,
//             que.queue[que.cur_idx].idx_in_all_v + chunk_size,
//             filters,
//             command,
//             filename,
//             n,
//             k,
//             tip
//         );
//         que.cur_idx += 1;
//     }
// }

int permute_bits(int num, const std::vector<int>& perm, int n) {
    int result = 0;
    for (int i = 0; i < n; ++i) {
        // Бит на позиции i в num становится битом на позиции perm[i]
        if (num & (1 << i)) {
            result |= (1 << perm[i]);
        }
    }
    return result;
}
// Оптимизированная версия: перестановки только внутри групп координат с одинаковой col_sum
std::vector<std::pair<int, std::vector<int>>> all_perms_optimised(std::vector<int> vertexes, int n) {
    std::vector<std::pair<int, std::vector<int>>> result;

    std::vector<int> sorted_vertexes = vertexes;
    std::sort(sorted_vertexes.begin(), sorted_vertexes.end());

    // Аллоцируем один раз, reuse в горячем цикле
    std::vector<int> permuted(vertexes.size());
    std::vector<int> transformed(vertexes.size());
    std::vector<int> col_sums_trans(n);
    std::vector<int> col_sums_orig(n);
    std::vector<int> perm(n);

    // Поколонные суммы оригинальных vertexes (константа)
    std::fill(col_sums_orig.begin(), col_sums_orig.end(), 0);
    for (int v : vertexes)
        for (int bit = 0; bit < n; bit++)
            if (v & (1 << bit)) col_sums_orig[bit]++;

    for (int x : vertexes) {
        // 1. XOR-нормализация
        for (size_t i = 0; i < vertexes.size(); i++)
            transformed[i] = vertexes[i] ^ x;

        // 2. Поколонные суммы transformed
        std::fill(col_sums_trans.begin(), col_sums_trans.end(), 0);
        for (int v : transformed)
            for (int bit = 0; bit < n; bit++)
                if (v & (1 << bit)) col_sums_trans[bit]++;

        // 3. Группируем: src координаты по col_sums_trans, tgt по col_sums_orig
        std::map<int, std::vector<int>> src_groups, tgt_groups;
        for (int i = 0; i < n; i++) {
            src_groups[col_sums_trans[i]].push_back(i);
            tgt_groups[col_sums_orig[i]].push_back(i);
        }

        // 4. Проверяем совместимость: одинаковые ключи с одинаковыми размерами
        bool compatible = (src_groups.size() == tgt_groups.size());
        if (compatible) {
            for (auto& [val, src_coords] : src_groups) {
                auto it = tgt_groups.find(val);
                if (it == tgt_groups.end() || it->second.size() != src_coords.size()) {
                    compatible = false;
                    break;
                }
            }
        }
        if (!compatible) continue;

        // 5. Строим пары (src_coords, tgt_coords) для каждого значения col_sum
        std::vector<std::pair<std::vector<int>, std::vector<int>>> group_pairs;
        for (auto& [val, src_coords] : src_groups)
            group_pairs.emplace_back(src_coords, tgt_groups[val]);

        // 6. Рекурсивный перебор: для каждой группы — все биекции src→tgt
        std::function<void(int)> enumerate = [&](int g_idx) {
            if (g_idx == (int)group_pairs.size()) {
                // Применяем perm, сравниваем (reuse permuted — без аллокации)
                for (size_t i = 0; i < vertexes.size(); i++)
                    permuted[i] = permute_bits(transformed[i], perm, n);
                std::sort(permuted.begin(), permuted.end());
                if (permuted == sorted_vertexes)
                    result.emplace_back(x, perm);
                return;
            }
            auto& [src_coords, tgt_coords] = group_pairs[g_idx];
            std::vector<int> tgt_vals = tgt_coords;
            std::sort(tgt_vals.begin(), tgt_vals.end());
            do {
                for (size_t i = 0; i < src_coords.size(); i++)
                    perm[src_coords[i]] = tgt_vals[i];
                enumerate(g_idx + 1);
            } while (std::next_permutation(tgt_vals.begin(), tgt_vals.end()));
        };

        enumerate(0);
    }
    return result;
}

// we assume 0 is within vertexes
// v in vertexes, 0 <= v < 2^n
std::vector<std::pair<int, std::vector<int>>> all_perms(std::vector<int> vertexes, int n) {
    std::vector<std::pair<int, std::vector<int>>> result;
    
    // Функция для применения перестановки к битам числа

    
    // Перебираем x из vertexes
    for (int x : vertexes) {
        // Вычисляем {v_i ^ x}
        std::vector<int> transformed(vertexes.size());
        for (size_t i = 0; i < vertexes.size(); ++i) {
            transformed[i] = vertexes[i] ^ x;
        }
        
        // Генерируем все перестановки {0, 1, ..., n-1}
        std::vector<int> perm(n);
        for (int i = 0; i < n; ++i) {
            perm[i] = i;
        }
        
        do {
            // Применяем перестановку к битам каждого transformed[i]
            std::vector<int> permuted(vertexes.size());
            for (size_t i = 0; i < vertexes.size(); ++i) {
                permuted[i] = permute_bits(transformed[i], perm, n);
            }
            
            // Сравниваем множества через сортировку
            std::vector<int> sorted_permuted = permuted;
            std::vector<int> sorted_vertexes = vertexes;
            std::sort(sorted_permuted.begin(), sorted_permuted.end());
            std::sort(sorted_vertexes.begin(), sorted_vertexes.end());
            
            if (sorted_permuted == sorted_vertexes) {
                result.emplace_back(x, perm);
            }
        } while (std::next_permutation(perm.begin(), perm.end()));
    }
    
    return result;
}
