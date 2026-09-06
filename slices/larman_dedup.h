#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <utility>
#include <vector>

// Дедуп для срезов булева куба.
//
// На срезе веса k симметрии, сохраняющие вес И хэммингово расстояние, — это
// ТОЛЬКО перестановки координат S_n (никаких XOR-трансляций: v⊕a меняет вес,
// выводит из среза — поэтому булев all_perms_optimised здесь МАТЕМАТИЧЕСКИ
// неверен). Идея применения дедупа — старая (пометить орбиту добавленного
// множества, скипнуть если видели); поправлена только группа.
//
// larman_stab возвращает поточечный стабилизатор seed-множества в S_n:
// перестановки, фиксирующие каждую seed-вершину. Формат (0, perm) совместим
// с orbit-application из cnc.h: permute_bits(v ^ 0, perm) == permute_bits(v, perm).
//
// Перечисление — по ТОЧНОМУ профилю координаты (битовый вектор по всем seed),
// перестановки только внутри блоков одинакового профиля. Это ровно размер
// группы = ∏ (|блок|!), без комбинаторного взрыва (в отличие от группировки
// по col-sum, где (1,0) и (0,1) слиплись бы в блок 2·dh и дали (2dh)!).
//
// Своп seed-вершин (для ребра — обмен концов, ×2) сознательно НЕ включён:
// подгруппа для дедупа всегда корректна (лишь меньше прунинга). При нужде
// добавляется отдельным косетом.

inline int permute_bits(int num, const std::vector<int>& perm, int n) {
    int r = 0;
    for (int i = 0; i < n; i++)
        if (num & (1 << i)) r |= (1 << perm[i]);
    return r;
}

inline std::vector<std::pair<int, std::vector<int>>>
larman_stab(std::vector<int> seed, int n) {
    std::vector<std::pair<int, std::vector<int>>> result;
    std::vector<int> sorted_seed = seed;
    std::sort(sorted_seed.begin(), sorted_seed.end());

    // Профиль координаты i = целое, чьи биты — значения координаты i по seed.
    std::map<long long, std::vector<int>> groups;
    for (int i = 0; i < n; i++) {
        long long prof = 0;
        for (size_t s = 0; s < seed.size(); s++)
            if (seed[s] & (1 << i)) prof |= (1LL << s);
        groups[prof].push_back(i);
    }

    std::vector<std::vector<int>> blocks;
    for (auto& [prof, coords] : groups) blocks.push_back(coords);

    std::vector<int> perm(n), permuted(seed.size());
    std::function<void(int)> rec = [&](int bi) {
        if (bi == (int)blocks.size()) {
            for (size_t i = 0; i < seed.size(); i++)
                permuted[i] = permute_bits(seed[i], perm, n);
            std::sort(permuted.begin(), permuted.end());
            if (permuted == sorted_seed) result.emplace_back(0, perm);
            return;
        }
        std::vector<int> tgt = blocks[bi];
        std::sort(tgt.begin(), tgt.end());
        const std::vector<int>& src = blocks[bi];
        do {
            for (size_t i = 0; i < src.size(); i++) perm[src[i]] = tgt[i];
            rec(bi + 1);
        } while (std::next_permutation(tgt.begin(), tgt.end()));
    };
    rec(0);
    return result;
}
