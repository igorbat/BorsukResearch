#include <iostream>
#include <string>
#include <map>
#include "../include/helpers.h"
#include "../include/bruteforce.h"

const int N = 12;
const int N2 = 1 << N;
const int K = 6;

std::string command_1 = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/kissat ";
std::string FILENAME = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/build/n12k6_5_5.txt";
std::string command_2 = " -q --time=10";

int main() {
    std::map<int, int> results;
    ConfQueue que;

    // IMPORTANT -- we use as a TIP that all starting verticies are grouped into clique. We do not look up for max clique manually.
    std::vector<int> starting_vertexes = {0, 0b111111000000, 0b111000111000, 0b101010010101, 0b110010100110 };

    std::vector<FuncPtr> filters;
    filters.push_back(is_inside);

    std::vector<int> vertexes_set;
    for (int i = 0; i < N2; i ++) {
        if (dist(i, 0) % 2 == 1) continue;
        if (is_allowed(i, starting_vertexes, N, K, filters)) vertexes_set.push_back(i);
    }

    vertexes_set = reorder_vertices(starting_vertexes, vertexes_set, 0, vertexes_set.size());

    Conf start_conf(starting_vertexes, vertexes_set);
    que.add(start_conf);

    BruteforceSolver solver(results, que, filters, command_1 + FILENAME + command_2, FILENAME, N, K, starting_vertexes.size(), 216);
    solver.run();

    return 0;
}
