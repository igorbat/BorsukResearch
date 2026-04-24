#include <iostream>
#include <string>
#include <map>
#include "../include/helpers.h"
#include "../include/bruteforce.h"

const int N = 15;
const int N2 = 1 << N; 
const int K = 6;

std::string command_1 = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/kissat ";
std::string FILENAME = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/build/types.txt";
std::string command_2 = " -q --time=10";

std::string toBinary(unsigned int num) {
    int res = 0;
    std::string binary;
    if (num == 0) {
        binary =  "0";
        res ++;
    }
    

    while (num > 0) {
        binary = (num & 1 ? "1" : "0") + binary;  // Prepend bit
        num >>= 1;// Right shift by 1
        res += 1;  
    }
    while (res < N) {
        binary = "0" + binary;
    }
    return binary;
}

int main() {
    std::map<int, int> results;
    ConfQueue que;

    // IMPORTANT -- we use as a TIP that all starting verticies are grouped into clique. We do not look up for max clique manually. 
    std::vector<int> starting_vertexes = {0, 0b111111000000000, 0b111000111000000};
    
    std::vector<int> the_rest_vrtx;

    for (auto v1: starting_vertexes) {
        for (auto v2: starting_vertexes) {
            if (dist(v1, v2) > 6) {
                std::cout << "ERROR" << std::endl;
                return 0;
            }
        }
    }
    std::vector<FuncPtr> filters;
    filters.push_back(is_inside);

    for (int i = 0; i < N2; i ++) {
        if (dist(i, 0) % 2 == 1) continue;
        if (is_allowed(i, starting_vertexes, N, K, filters)) {
           the_rest_vrtx.push_back(i);
        }
    }
    std::cout << the_rest_vrtx.size() << std::endl;

    std::set<std::pair<int, uint64_t>> seen_configs;


    for (int i = 0; i < the_rest_vrtx.size(); i++) {
        for(int j = i + 1; j < the_rest_vrtx.size(); j++)
    }

  

    return 0;
}
