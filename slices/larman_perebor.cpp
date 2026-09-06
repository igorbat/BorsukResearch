// Перебор для незакрытых (TIMEOUT) троек Larman-свипа на n цветах.
// Читает build/results_ncolors.tsv, берёт TIMEOUT, дедуплицирует по
// (n, min(k,n-k), d) [комплементарная симметрия — тот же граф], и для каждой
// запускает cube-and-conquer перебор (larman_cnc.h) с S_n-дедупом (larman_stab).
//
// Как обычно: бинпоиск отсечки на старте, жадный max_elimination-порядок,
// дедуп орбит стартового ребра. Раскраска в n цветов, TL=1с.

#include <algorithm>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <iostream>

#include "larman_cnc.h"

static std::string DIR;      // задаётся в main()
static std::string KISSAT;   // задаётся в main()

static int popcnt(int x) { return __builtin_popcount((unsigned)x); }

int main(int argc, char** argv) {
    // Пути можно задать аргументами:  larman_perebor <путь к kissat> <рабочий каталог>
    KISSAT = argc > 1 ? argv[1] : "/Users/ibatmanov/personal/science/kissat/build/kissat";
    DIR    = argc > 2 ? argv[2] : "build";

    // собрать TIMEOUT-тройки, дедуп по канону (n, min(k,n-k), d)
    std::vector<std::tuple<int,int,int>> cases;
    std::set<std::tuple<int,int,int>> seen_canon;
    std::ifstream in(DIR + "/results_ncolors.tsv");
    std::string line; std::getline(in, line);
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        int n,k,d,v,e,c; std::string st;
        ss >> n >> k >> d >> v >> e >> c >> st;
        if (st != "TIMEOUT") continue;
        auto canon = std::make_tuple(n, std::min(k, n - k), d);
        if (seen_canon.count(canon)) continue;
        seen_canon.insert(canon);
        cases.emplace_back(n, k, d);
    }
    std::cout << "TIMEOUT-троек (после дедупа комплементов): " << cases.size() << std::endl;

    std::ofstream summary(DIR + "/perebor_summary.tsv", std::ios::app);

    for (auto& [n, k, d] : cases) {
        int dh = d / 2;
        int v1 = (1 << k) - 1;
        int v2 = ((1 << (k - dh)) - 1) | (((1 << dh) - 1) << k);
        std::vector<int> start = {v1, v2};

        // линза: вершины веса k в пересечении шаров радиуса d вокруг v1,v2
        std::vector<int> candidates;
        for (int w = 0; w < (1 << n); w++) {
            if (w == v1 || w == v2) continue;
            if (popcnt(w) != k) continue;
            if (dist(w, v1) <= d && dist(w, v2) <= d) candidates.push_back(w);
        }

        std::cout << "\n########## n=" << n << " k=" << k << " d=" << d
                  << " |V|=" << candidates.size() + 2 << " ##########" << std::endl;

        CncParams P;
        P.n = n;
        P.k = d;             // расстояние ребра графа = диаметр d
        P.ncolors = n;       // порог Борсука для среза
        P.dedup_depth = 3;
        P.chunk_margin = 50;
        P.n_workers = 2;     // скромно — рядом крутится K3-прогон
        P.tl = 1;            // 1 секунда на раскраску
        P.tl_retry = 0;      // без ретрая
        P.tl_binsearch = 1;  // бинпоиск тоже 1с
        P.snapshot_period_s = 120;
        P.checkpoint_period_s = 100000;  // фактически без чекпоинтов (кейсы короткие)
        P.kissat_path = KISSAT;
        P.filename_prefix = DIR + "/pb_n" + std::to_string(n) + "k" + std::to_string(k) +
                            "d" + std::to_string(d);
        P.anomaly_file = DIR + "/perebor_kernels.txt";

        std::vector<FuncPtr> filters = {is_inside};
        CncSolver solver(P);
        solver.run(start, candidates, filters);
    }

    std::cout << "\nPEREBOR DONE" << std::endl;
    return 0;
}
