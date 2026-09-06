#include "sat.h"

#include <cstdio>
#include <fstream>
#include <memory>
#include <stdexcept>

std::atomic<long long> sat_calls_total{0};

static bool call_binary(const std::string& command) {
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("failed to call kissat");
    }
    char buffer[128];
    std::string response;
    while (fgets(buffer, sizeof(buffer), pipe.get()) != nullptr) {
        response += buffer;
    }
    sat_calls_total.fetch_add(1, std::memory_order_relaxed);
    return response.find(" SATISFIABLE") != std::string::npos;
}

static int encode(int v_index, int c, int ncolors) {
    return v_index * ncolors + c;
}

bool is_colorable(const std::vector<int>& vertices, int tip, int ncolors, int k, int /*n*/,
                  const std::string& filename, const std::string& command) {
    std::vector<std::vector<int>> clauses;
    int col = 1;

    for (int i = 0; i < tip; i++) {
        clauses.push_back({encode(i, col, ncolors)});
        ++col;
    }

    // Рёбра: вершины на расстоянии ровно k не могут иметь одинаковый цвет
    for (size_t i = 0; i < vertices.size(); ++i) {
        for (size_t j = i + 1; j < vertices.size(); ++j) {
            if (dist(vertices[i], vertices[j]) == k) {
                for (int c = 1; c <= ncolors; ++c) {
                    clauses.push_back({-encode(i, c, ncolors), -encode(j, c, ncolors)});
                }
            }
        }
    }

    // Каждая вершина — хотя бы один цвет
    for (size_t i = 0; i < vertices.size(); ++i) {
        std::vector<int> clause;
        for (int c = 1; c <= ncolors; ++c) {
            clause.push_back(encode(i, c, ncolors));
        }
        clauses.push_back(clause);
    }

    std::ofstream out(filename, std::ios::trunc);
    if (!out.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    int n_vars = vertices.size() * ncolors;
    out << "p cnf " << n_vars << " " << clauses.size() << "\n";
    for (const auto& clause : clauses) {
        for (int lit : clause) out << lit << " ";
        out << "0\n";
    }
    out.close();

    return call_binary(command);
}

std::vector<int> prepare_to_color(
    const std::vector<int>& base_v,
    const std::vector<int>& brute_v, size_t from1,
    const std::vector<int>& other_v, size_t from2,
    int n, int k, const std::vector<FuncPtr>& functions) {
    std::vector<int> to_color;
    to_color.reserve(base_v.size() + brute_v.size() + other_v.size());
    to_color.insert(to_color.end(), base_v.begin(), base_v.end());
    for (size_t i = from1; i < brute_v.size(); i++) {
        if (is_allowed(brute_v[i], base_v, n, k, functions)) to_color.push_back(brute_v[i]);
    }
    for (size_t i = from2; i < other_v.size(); i++) {
        if (is_allowed(other_v[i], base_v, n, k, functions)) to_color.push_back(other_v[i]);
    }
    return to_color;
}
