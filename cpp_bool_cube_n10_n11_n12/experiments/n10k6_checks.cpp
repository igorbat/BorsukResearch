#include <iostream>
#include <string>
#include <map>
#include "../include/helpers.h"
#include "../include/bruteforce.h"

const int N = 11;
const int N2 = 1 << N; 
const int K = 6;

std::string command_1 = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/kissat ";
std::string FILENAME = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/build/n10k6_checks.txt";
std::string command_2 = " -q --time=10";

int main() {
    std::map<int, int> results;
    ConfQueue que;

    // IMPORTANT -- we use as a TIP that all starting verticies are grouped into clique. We do not look up for max clique manually. 
    std::vector<int> starting_vertexes = {0, 0b1111110000, 0b1110001110, 0b0101011011, 0b1000110111 };
    // , 178, 180, 184, 190, 192, 204, 212, 216, 222, 232, 250, 264, 282, 284
    //std::vector<int> starting_vertexes = { 0, 0b1111110000, 0b1110001110, 54, 411, 485, 92, 533, 429, 246, 669, 380, 277, 469, 149, 413, 437};
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

    // the_rest_vrtx = reorder_vertices(starting_vertexes, the_rest_vrtx, 0, the_rest_vrtx.size());

    // std::vector<int> all_vrtx(starting_vertexes.begin(), starting_vertexes.end());
    // std::vector<int> all_vrtx = {};
    // int mid = 100;
    // all_vrtx.insert(all_vrtx.end(), the_rest_vrtx.begin() + mid, the_rest_vrtx.end());

    // auto vv = prepare_to_color(starting_vertexes, {}, 0, all_vrtx, 0, N, K, filters);
    // for (auto i: vv) std::cout << i << " " ; std::cout << std::endl;
    
    // std::cout <<  starting_vertexes.size() << " " << N + 1<< " " << K<< " " << FILENAME<< " " << command_1 + FILENAME + command_2 << std::endl;
    // bool is_c = is_colorable(
    //         prepare_to_color(starting_vertexes, {}, 0, all_vrtx, 0, N, K, filters),  // id_in_poten=0
    //         starting_vertexes.size(),
    //         N + 1,
    //         K,
    //         FILENAME,
    //         command_1 + FILENAME + command_2
    //     );
    // if (is_c) {
    //     std::cout <<  " colorable" << std::endl;
    // } else {
    //     std::cout <<  "not colorable" << std::endl; 
    // }
    int l = 15;
    int r = the_rest_vrtx.size();
    the_rest_vrtx = reorder_vertices(
                starting_vertexes,
                the_rest_vrtx,
                0,
                static_cast<int>(the_rest_vrtx.size())
            );

    while (r - l > 1) {
        int mid = (r + l) / 2;

        std::vector<int> all_vrtx(starting_vertexes.begin(), starting_vertexes.end());
        all_vrtx.insert(all_vrtx.end(), the_rest_vrtx.begin() + mid, the_rest_vrtx.end());
        if (is_colorable(all_vrtx, 0, 11, K, FILENAME, command_1 + FILENAME + command_2)) {
            std::cout << mid << " colorable" << std::endl;
            r = mid;
        } else {
            l = mid;
            std::cout << mid << " not colorable" << std::endl;
        }
    }

  

    return 0;
}
