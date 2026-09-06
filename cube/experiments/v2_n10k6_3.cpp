#include <iostream>
#include <string>
#include <map>
#include "../include/helpers.h"
#include "../include/bruteforce.h"

const int N = 10;
const int N2 = 1 << N; 
const int K = 6;


int main(int argc, char** argv) {
    // Пути можно задать аргументами:  v2_n10k6_3 <путь к kissat> <рабочий каталог>
    // Без аргументов используются значения по умолчанию.
    const std::string kissat  = argc > 1 ? argv[1] : "/Users/ibatmanov/personal/science/kissat/build/kissat";
    const std::string workdir = argc > 2 ? argv[2] : "build";

    const std::string FILENAME  = workdir + "/n10k6_3.txt";
    const std::string command_1 = kissat + " ";
    const std::string command_2 = " -q --time=10";

    std::map<int, int> results;
    ConfQueue que;

    // IMPORTANT -- we use as a TIP that all starting verticies are grouped into clique. We do not look up for max clique manually. 
    std::vector<int> starting_vertexes = {0, 0b1111110000, 0b1110001110 };

    std::vector<FuncPtr> filters;
    filters.push_back(is_inside);
    filters.push_back(is_not_created_k4);

    std::vector<int> vertexes_set;
    for (int i = 0; i < N2; i ++) {
        if (dist(i, 0) % 2 == 1) continue;
        if (is_allowed(i, starting_vertexes, N, K, filters)) vertexes_set.push_back(i);
    }

    vertexes_set = reorder_vertices(starting_vertexes, vertexes_set, 0, vertexes_set.size());

    Conf start_conf(starting_vertexes, vertexes_set);
    que.add(start_conf);

    BruteforceSolver solver(results, que, filters, command_1 + FILENAME + command_2, FILENAME, N, K, starting_vertexes.size(), 100);
    solver.run();

    return 0;
}
