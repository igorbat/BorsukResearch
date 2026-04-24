#include <iostream>
#include <string>
#include <map>
#include "../include/helpers.h"
#include "../include/bruteforce.h"

const int N = 10;
const int N2 = 1 << N; 
const int K = 4;

std::string command_1 = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/kissat ";
std::string FILENAME = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/build/n10k4.txt";
std::string command_2 = " -q --time=100";

int main() {
    std::map<int, int> results;
    ConfQueue que;

    // IMPORTANT -- we use as a TIP that all starting verticies are grouped into clique. We do not look up for max clique manually. 
    std::vector<int> starting_vertexes = {0, 0b1111000000 };

    std::vector<FuncPtr> filters;
    filters.push_back(is_inside);
    // filters.push_back(is_not_created_k4);

    std::vector<int> vertexes_set;
    for (int i = 0; i < N2; i ++) {
        if (dist(i, 0) % 2 == 1) continue;
        if (is_allowed(i, starting_vertexes, N, K, filters)) vertexes_set.push_back(i);
    }

    vertexes_set = reorder_vertices(starting_vertexes, vertexes_set, 0, vertexes_set.size());

    Conf start_conf(starting_vertexes, vertexes_set);
    que.add(start_conf);

    // int l = 15;
    // int r = vertexes_set.len();

    // while (r - l > 1) {

    // }

    BruteforceSolver solver(results, que, filters, command_1 + FILENAME + command_2, FILENAME, N, K, starting_vertexes.size(), 100);
    solver.run();

    // //std::cout << "COLORINGS: " << colorings << std::endl;  // if need, can add solver.get_colorings() but since commented, leave

    // std::cout << "results: " <<std::endl;

    // for (auto& [k, v]: results) {
    //     std::cout << k << ": " << v << std::endl;
    // }

    // std::cout << "queue: " <<std::endl;
    // for (auto& conf: que.queue) {
    //     std::cout << "[";
    //     for (auto v: conf.vertexes_in_conf) {
    //         std::cout << v << ", ";
    //     }
    //     std::cout << "]";  // removed "idx in others" since no idx_in_all_v, but if need, can print remaining.size() or something; as per code, adapt to not print idx
    //     std::cout << std::endl;
    // }

    return 0;
}
