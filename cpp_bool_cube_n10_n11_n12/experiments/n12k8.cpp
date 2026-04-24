#include <iostream>
#include <string>
#include <map>
#include "../include/helpers.h"
#include "../include/bruteforce.h"

const int N = 12;
const int N2 = 1 << N; 
const int K = 8;

std::string command_1 = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/kissat ";
std::string FILENAME = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/build/n12k8_3.txt";
std::string command_2 = " -q --time=120";

int main() {
    std::map<int, int> results;
    ConfQueue que;

    // IMPORTANT -- we use as a TIP that all starting verticies are grouped into clique. We do not look up for max clique manually. 
    std::vector<int> starting_vertexes = { 0, 0b111111110000, 0b111100001111 };

    std::vector<int> vertexes_8 = { 0b111111110000, 0b111100001111 };
    std::vector<int> vertexes_4;
    std::vector<int> vertexes_6;

    
    std::vector<FuncPtr> filters;
    filters.push_back(is_inside);

    for (int i = 0; i < N2; i ++) {
        if (dist(i, 0) % 2 == 1) continue;
        if (is_allowed(i, starting_vertexes, N, K, filters)) {
            if (dist(i, 0) == 8) {
                vertexes_8.push_back(i);
            }
            if (dist(i, 0) == 6) {
                vertexes_6.push_back(i);
            }
            if (dist(i, 0) == 4) {
                vertexes_4.push_back(i);
            }
        }
    }

    for (int i = 3; i < 8; i++) {
        if (is_colorable(vertexes_4, 0, i, K, FILENAME, command_1 + FILENAME + command_2)) {
            std::cout << "Vertexes_4 is colorable in " << i << " colors" << std::endl;
            break;
        } else {
            std::cout << "Vertexes_4 is not colorable in " << i << " colors" << std::endl;
        }
    }

    for (int i = 3; i < 8; i++) {
        if (is_colorable(vertexes_6, 0, i, K, FILENAME, command_1 + FILENAME + command_2)) {
            std::cout << "Vertexes_6 is colorable in " << i << " colors" << std::endl;
            break;
        } else {
            std::cout << "Vertexes_6 is not colorable in " << i << " colors" << std::endl;
        }
    }

    for (int i = 3; i < 8; i++) {
        if (is_colorable(vertexes_8, 0, i, K, FILENAME, command_1 + FILENAME + command_2)) {
            std::cout << "Vertexes_8 is colorable in " << i << " colors" << std::endl;
            break;
        } else {
            std::cout << "Vertexes_8 is not colorable in " << i << " colors" << std::endl;
        }
    }

    for (int i = 3; i < 8; i++) {
        if (is_colorable(vertexes_8, 0, i, K, FILENAME, command_1 + FILENAME + command_2)) {
            std::cout << "Vertexes_8 is colorable in " << i << " colors" << std::endl;
            break;
        } else {
            std::cout << "Vertexes_8 is not colorable in " << i << " colors" << std::endl;
        }
    }

    return 0;
}
