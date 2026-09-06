// Larman/Borsuk на срезах — раскраска в n цветов (порог Борсука для среза:
// срез веса k лежит в гиперплоскости {Σx=k} размерности n-1 → Борсук = n частей).
//
// Прогоняем:
//  - для n<=11: только тройки, что НЕ покрасились в n-1 (UNSAT/TIMEOUT из
//    build/results.tsv). Покрасившиеся в n-1 тривиально красятся и в n
//    (монотонность), их не трогаем.
//  - для n=12,13: полный валидный грид (k,d) — фильтра нет.
// Всё на n цветах, линза и стартовое ребро как в larman_sweep.cpp, TL=60с.

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>

const int TL = 60;
static std::string KISSAT;   // задаётся в main()
static std::string DIR;      // задаётся в main()

static int popcount(int x) { return __builtin_popcount((unsigned)x); }
static int dist(int a, int b) { return __builtin_popcount((unsigned)(a ^ b)); }

enum Status { SAT, UNSAT, TIMEOUT };

// строит линзу для (n,k,d); false если ребра не существует
static bool build_lens(int n, int k, int d, std::vector<int>& verts) {
    int dh = d / 2;
    if (k - dh < 0 || k + dh > n) return false;
    int v1 = (1 << k) - 1;
    int v2 = ((1 << (k - dh)) - 1) | (((1 << dh) - 1) << k);
    if (popcount(v1) != k || popcount(v2) != k || dist(v1, v2) != d) return false;
    verts = {v1, v2};
    for (int w = 0; w < (1 << n); w++) {
        if (w == v1 || w == v2) continue;
        if (popcount(w) != k) continue;
        if (dist(w, v1) <= d && dist(w, v2) <= d) verts.push_back(w);
    }
    return true;
}

static Status solve(const std::vector<int>& verts, int tip, int ncolors, int d, int n,
                    const std::string& fname, int& n_edges) {
    auto enc = [&](int v, int c) { return v * ncolors + c; };
    std::vector<std::vector<int>> clauses;
    for (int i = 0; i < tip; i++) clauses.push_back({enc(i, i + 1)});
    n_edges = 0;
    for (size_t i = 0; i < verts.size(); i++)
        for (size_t j = i + 1; j < verts.size(); j++)
            if (dist(verts[i], verts[j]) == d) {
                n_edges++;
                for (int c = 1; c <= ncolors; c++)
                    clauses.push_back({-enc((int)i, c), -enc((int)j, c)});
            }
    for (size_t i = 0; i < verts.size(); i++) {
        std::vector<int> cl;
        for (int c = 1; c <= ncolors; c++) cl.push_back(enc((int)i, c));
        clauses.push_back(cl);
    }
    std::ofstream out(fname, std::ios::trunc);
    out << "p cnf " << verts.size() * ncolors << " " << clauses.size() << "\n";
    for (auto& cl : clauses) { for (int l : cl) out << l << " "; out << "0\n"; }
    out.close();

    std::string cmd = KISSAT + " " + fname + " -q --time=" + std::to_string(TL);
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    std::string resp; char buf[128];
    while (pipe && fgets(buf, sizeof(buf), pipe.get())) resp += buf;
    if (resp.find("UNSATISFIABLE") != std::string::npos) return UNSAT;
    if (resp.find("SATISFIABLE") != std::string::npos) return SAT;
    return TIMEOUT;
}

int main(int argc, char** argv) {
    // Пути можно задать аргументами:  larman_ncolors <путь к kissat> <рабочий каталог>
    KISSAT = argc > 1 ? argv[1] : "/Users/ibatmanov/personal/science/kissat/build/kissat";
    DIR    = argc > 2 ? argv[2] : "build";

    std::vector<std::tuple<int, int, int>> todo;

    // 1) non-SAT тройки из n-1 свипа (n<=11)
    std::ifstream in(DIR + "/results.tsv");
    std::string line;
    std::getline(in, line);  // header
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        int n, k, d, verts, edges, colors; std::string status;
        ss >> n >> k >> d >> verts >> edges >> colors >> status;
        if (status == "UNSAT" || status == "TIMEOUT") todo.emplace_back(n, k, d);
    }
    int from_prev = (int)todo.size();

    // 2) полный грид n=12,13
    for (int n = 12; n <= 13; n++)
        for (int k = 1; k <= n; k++)
            for (int d = 2; d <= 2 * std::min(k, n - k); d += 2)
                todo.emplace_back(n, k, d);

    std::cout << "todo: " << todo.size() << " (из n-1: " << from_prev
              << ", n=12-13 грид: " << todo.size() - from_prev << ")" << std::endl;

    std::ofstream tsv(DIR + "/results_ncolors.tsv", std::ios::trunc);
    tsv << "n\tk\td\tverts\tedges\tcolors\tstatus\n"; tsv.flush();
    std::cout << "n\tk\td\tverts\tedges\tcolors\tstatus" << std::endl;

    for (auto& [n, k, d] : todo) {
        std::vector<int> verts;
        if (!build_lens(n, k, d, verts)) continue;
        int ncolors = n;  // <-- порог Борсука для среза
        std::string fname = DIR + "/cur_nc.cnf";
        int edges = 0;
        Status st = solve(verts, 2, ncolors, d, n, fname, edges);
        const char* s = st == SAT ? "SAT" : st == UNSAT ? "UNSAT" : "TIMEOUT";
        std::cout << n << "\t" << k << "\t" << d << "\t" << verts.size() << "\t"
                  << edges << "\t" << ncolors << "\t" << s << std::endl;
        std::cout.flush();
        tsv << n << "\t" << k << "\t" << d << "\t" << verts.size() << "\t"
            << edges << "\t" << ncolors << "\t" << s << "\n"; tsv.flush();
    }
    std::cout << "SWEEP DONE" << std::endl;
    return 0;
}
