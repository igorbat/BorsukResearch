#include <iostream>
#include <string>
#include <map>
#include "../include/helpers.h"

const int N = 12;
const int N2 = 1 << N; 
const int K = 6;

std::string command_1 = "./kissat ";
std::string FILENAME = "n12k6_44.txt";
std::string command_2 = " -q --time=1";

int main() {
    std::map<int, int> results;
    ConfQueue que;

    // IMPORTANT -- we use as a TIP that all starting verticies are grouped into clique. We do not look up for max clique manually. 
    std::vector<int> starting_vertexes = {0, 0b111111000000, 0b111000111000111, 0b111000000111 };
     Conf start_conf(starting_vertexes, starting_vertexes.size(), 0);
    que.add(start_conf);

    std::vector<FuncPtr> filters;
    filters.push_back(is_inside);

    std::vector<int> vertexes_set;
    for (int i = 0; i < N2; i ++) {
        if (dist(i, 0) % 2 == 1) continue;
        if (is_allowed(i, starting_vertexes, N, K, filters)) vertexes_set.push_back(i);
    }

    staged_bruteforce(results, que, vertexes_set, filters, command_1 + FILENAME + command_2, FILENAME, N, K, starting_vertexes.size(), 30);

    //std::cout << "COLORINGS: " << colorings << std::endl;

    std::cout << "results: " <<std::endl;

    for (auto& [k, v]: results) {
        std::cout << k << ": " << v << std::endl;
    }

    std::cout << "queue: " <<std::endl;
    for (auto& conf: que.queue) {
        std::cout << "[";
        for (auto v: conf.vertexes_in_conf) {
            std::cout << v << ", ";
        }
        std::cout << "] idx in others: " << conf.idx_in_all_v << std::endl;
    }

    return 0;
}
