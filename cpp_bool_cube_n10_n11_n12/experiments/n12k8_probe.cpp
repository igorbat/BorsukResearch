#include <iostream>
#include <string>
#include <vector>
#include "../include/helpers.h"

const int N = 12;
const int N2 = 1 << N;
const int K = 8;

std::string command_1 = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/kissat ";
std::string FILENAME = "/root/science/bruteforce_via_kissat/Borsuk_plus_kissat/build/n12k8_probe.txt";
std::string command_2 = " -q --time=300";

int main() {
    // Стартовая конструкция: 4 вершины разных цветов
    std::vector<int> starting_vertexes = {0, 0b111111110000, 0b111100001111, 0b000011111111};
    int tip = starting_vertexes.size();

    // Собираем все вершины на расстоянии 8 от 0 + саму 0
    std::vector<int> vertices = starting_vertexes;
    for (int i = 1; i < N2; i++) {
        if (dist(i, 0) != K) continue;
        // Не дублируем стартовые
        bool already = false;
        for (int s : starting_vertexes)
            if (i == s) { already = true; break; }
        if (!already) vertices.push_back(i);
    }

    std::cout << "Total vertices: " << vertices.size() << std::endl;
    std::string command = command_1 + FILENAME + command_2;

    for (int ncolors = 16; ncolors >= 1; ncolors--) {
        std::cout << "Trying " << ncolors << " colors... " << std::flush;
        bool ok = is_colorable(vertices, tip, ncolors, K, FILENAME, command);
        if (ok) {
            std::cout << "COLORABLE with " << ncolors << " colors" << std::endl;
        } else {
            std::cout << "NOT colorable with " << ncolors << " colors" << std::endl;
            std::cout << "Chromatic number is " << ncolors + 1 << std::endl;
            break;
        }
    }

    return 0;
}
