// Larman / Borsuk на срезах булева куба.
//
// Срез веса k куба {0,1}^n = все вершины ровно с k единицами (constant-weight).
// Граф диаметров: рёбра соединяют пары на Хэмминговом расстоянии ровно d
// (d чётно — на срезе все расстояния чётные). Вопрос: красится ли в n-1 цветов.
//
// Мы фиксируем стартовое диаметральное ребро (WLOG по симметрии среза):
//   v1 = 1^k 0^{n-k}
//   v2 = 1^{k-dh} 0^{dh} 1^{dh} 0^{n-k-dh},  dh = d/2
// dist(v1,v2) = d, оба веса k.
//
// Красим "линзу" — пересечение шаров радиуса d вокруг v1 и v2 внутри среза:
//   кандидаты = {w : popcount(w)=k, dist(w,v1)<=d, dist(w,v2)<=d, w != v1,v2}
// Рёбра внутри {v1,v2}∪кандидаты — пары на расстоянии ровно d.
// v1,v2 предкрашены в цвета 1,2 (tip=2). Цветов = n-1. Таймаут TL секунд.
//
// Классификация: SAT (покрасилось) / UNSAT (доказано не красится) /
// TIMEOUT (за TL не решилось — нужен полноценный перебор).

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <iostream>

const int NMAX = 11;
const int TL = 60;  // секунд на инстанс
static std::string KISSAT;   // задаётся в main()
static std::string DIR;      // задаётся в main()

static int popcount(int x) { return __builtin_popcount((unsigned)x); }
static int dist(int a, int b) { return __builtin_popcount((unsigned)(a ^ b)); }

enum Status { SAT, UNSAT, TIMEOUT };

static Status solve(const std::vector<int>& verts, int tip, int ncolors, int d, int n,
                    const std::string& fname, int& n_edges) {
    // var(v,c) = v*ncolors + c, c in 1..ncolors
    auto enc = [&](int v, int c) { return v * ncolors + c; };
    std::vector<std::vector<int>> clauses;
    for (int i = 0; i < tip; i++) clauses.push_back({enc(i, i + 1)});  // seed: v_i -> color i+1
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
    std::string resp;
    char buf[128];
    while (pipe && fgets(buf, sizeof(buf), pipe.get())) resp += buf;
    if (resp.find("UNSATISFIABLE") != std::string::npos) return UNSAT;
    if (resp.find("SATISFIABLE") != std::string::npos) return SAT;
    return TIMEOUT;
}

int main(int argc, char** argv) {
    // Пути можно задать аргументами:  larman_sweep <путь к kissat> <рабочий каталог>
    KISSAT = argc > 1 ? argv[1] : "/Users/ibatmanov/personal/science/kissat/build/kissat";
    DIR    = argc > 2 ? argv[2] : "build";

    std::ofstream tsv(DIR + "/results.tsv", std::ios::trunc);
    tsv << "n\tk\td\tverts\tedges\tcolors\tstatus\n";
    tsv.flush();
    std::cout << "n\tk\td\tverts\tedges\tcolors\tstatus" << std::endl;

    for (int n = 1; n <= NMAX; n++) {
        for (int k = 1; k <= n; k++) {
            int dmax = 2 * std::min(k, n - k);  // настоящий диаметр среза
            for (int d = 2; d <= dmax; d += 2) {
                int dh = d / 2;
                if (k - dh < 0 || k + dh > n) continue;  // подстраховка
                int v1 = (1 << k) - 1;
                int v2 = ((1 << (k - dh)) - 1) | (((1 << dh) - 1) << k);
                // (assert-и по построению: вес и расстояние)
                if (popcount(v1) != k || popcount(v2) != k || dist(v1, v2) != d) continue;

                int ncolors = n - 1;
                std::string tag = "n=" + std::to_string(n) + " k=" + std::to_string(k) +
                                  " d=" + std::to_string(d);
                if (ncolors < 2) {
                    // диаметральное ребро требует >=2 цветов — вырожденно "не красится"
                    std::cout << n << "\t" << k << "\t" << d << "\t2\t1\t" << ncolors
                              << "\tDEGENERATE" << std::endl;
                    tsv << n << "\t" << k << "\t" << d << "\t2\t1\t" << ncolors
                        << "\tDEGENERATE\n"; tsv.flush();
                    continue;
                }

                // кандидаты линзы
                std::vector<int> verts = {v1, v2};
                for (int w = 0; w < (1 << n); w++) {
                    if (w == v1 || w == v2) continue;
                    if (popcount(w) != k) continue;
                    if (dist(w, v1) <= d && dist(w, v2) <= d) verts.push_back(w);
                }

                std::string fname = DIR + "/cur.cnf";
                int edges = 0;
                Status st = solve(verts, 2, ncolors, d, n, fname, edges);
                const char* s = st == SAT ? "SAT" : st == UNSAT ? "UNSAT" : "TIMEOUT";
                std::cout << n << "\t" << k << "\t" << d << "\t" << verts.size() << "\t"
                          << edges << "\t" << ncolors << "\t" << s << std::endl;
                std::cout.flush();
                tsv << n << "\t" << k << "\t" << d << "\t" << verts.size() << "\t"
                    << edges << "\t" << ncolors << "\t" << s << "\n";
                tsv.flush();
            }
        }
    }
    std::cout << "SWEEP DONE" << std::endl;
    return 0;
}
