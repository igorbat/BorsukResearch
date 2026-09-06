#pragma once

#include <atomic>
#include <string>
#include <vector>

#include "helpers.h"

// Счётчик реальных вызовов kissat (все треды). Печать прогресса — на стороне солвера.
extern std::atomic<long long> sat_calls_total;

// Пишет CNF в filename и зовёт kissat командой command.
// Раскраска в ncolors цветов, первые tip вершин получают фиксированные цвета 1..tip.
// Рёбра — пары на расстоянии ровно k. Thread-safe при уникальном filename на тред.
bool is_colorable(const std::vector<int>& vertices, int tip, int ncolors, int k, int n,
                  const std::string& filename, const std::string& command);

// base_v + те из brute_v[from1..] и other_v[from2..], что проходят фильтры относительно base_v
std::vector<int> prepare_to_color(
    const std::vector<int>& base_v,
    const std::vector<int>& brute_v, size_t from1,
    const std::vector<int>& other_v, size_t from2,
    int n, int k, const std::vector<FuncPtr>& functions);
