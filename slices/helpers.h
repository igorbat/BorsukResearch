#pragma once

#include <vector>
#include <algorithm>
#include <utility>

// {0,1}^n куб: вершины — числа 0..2^n-1, расстояние — Хэмминг.
// Дедупа в этом проекте нет сознательно (см. README), поэтому
// здесь нет симметрий/пермутаций — только фильтры и реордер.

using FuncPtr = bool (*)(int, const std::vector<int>&, int, int);

inline int dist(int v1, int v2) {
    return __builtin_popcount(static_cast<unsigned>(v1 ^ v2));
}

// Вершина на расстоянии <= k от всех base вершин, и != 0 до каждой
inline bool is_inside(int vertex, const std::vector<int>& vertexes, int /*n*/, int k) {
    for (auto v : vertexes) {
        int dst = dist(v, vertex);
        if (dst > k || dst == 0) return false;
    }
    return true;
}

// Запрет треугольника: v не образует K3 с парой вершин graph_vertexes на расстоянии k
inline bool is_not_created_k3(int v, const std::vector<int>& graph_vertexes, int /*n*/, int k) {
    int ln = graph_vertexes.size();
    for (int i1 = 0; i1 < ln; ++i1) {
        int v1 = graph_vertexes[i1];
        if (dist(v, v1) != k) continue;
        for (int i2 = i1 + 1; i2 < ln; ++i2) {
            int v2 = graph_vertexes[i2];
            if (dist(v1, v2) == k && dist(v, v2) == k) return false;
        }
    }
    return true;
}

inline bool is_allowed(int v, const std::vector<int>& graph_vertexes, int n, int k,
                       const std::vector<FuncPtr>& functions) {
    for (auto f : functions) {
        if (!f(v, graph_vertexes, n, k)) return false;
    }
    return true;
}

// Реордер по числу убиваемых вершин: кандидаты, у которых dist > k
// до максимального числа других кандидатов, идут первыми.
inline std::vector<int> reorder_by_max_elimination(
    const std::vector<int>& poten_v, int from, int to, int k) {
    int sz = to - from;
    std::vector<std::pair<int, int>> vctr(sz);
    for (int i = from; i < to; i++) {
        int candidate = poten_v[i];
        int eliminates = 0;
        for (int j = from; j < to; j++) {
            if (i == j) continue;
            if (dist(candidate, poten_v[j]) > k) eliminates++;
        }
        vctr[i - from] = {-eliminates, candidate};
    }
    std::sort(vctr.begin(), vctr.end());
    std::vector<int> result(sz);
    std::transform(vctr.begin(), vctr.end(), result.begin(),
                   [](const std::pair<int, int>& p) { return p.second; });
    return result;
}
