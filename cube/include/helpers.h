#pragma once

#include <set>
#include <map>
#include <vector>
#include <string>
#include <cstdio>
#include <memory>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <cassert>

using FuncPtr = bool (*)(int, const std::vector<int>&, int, int);

int dist(int v1, int v2);

bool is_inside(int vertex, const std::vector<int>& vertexes, int n, int k);
std::vector<int> bitmask(int value, int n);

std::vector<int> bitmask_sum(const std::vector<int>& bitmask1, const std::vector<int>& bitmask2);

std::vector<int> bitmask_minus(const std::vector<int>& bitmask1, const std::vector<int>& bitmask2);

std::vector<int> build_bitmask(const std::vector<int>& values, int n);

bool is_not_created_k3(int v, const std::vector<int>& graph_vertexes, int n, int k);

std::vector<bool> which_types_k4_with_v(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k4_4(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k4_4_or_3(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k4_4_or_3_or_2(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k4(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k5(int v, const std::vector<int>& graph_vertexes, int n, int k);

std::vector<bool> which_types_k5_with_v(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k5_5(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k5_5_or_4(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k5_5_or_4_or_3(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_not_created_k5_5_or_4_or_3_or_2(int v, const std::vector<int>& graph_vertexes, int n, int k);

bool is_allowed(int v, const std::vector<int>& graph_vertexes, int n, int k, const std::vector<FuncPtr>& functions);

/////////////////////////////////////////////////

// struct Conf {
//     std::vector<int> vertexes_in_conf;
//     int start_len;
//     int idx_in_all_v;

//     bool operator != (const Conf& other) {
//         if (idx_in_all_v != other.idx_in_all_v) {
//             return true;
//         }
//         if (vertexes_in_conf.size() != other.vertexes_in_conf.size()) {
//             return true;
//         }
//         for (int i = start_len; i < vertexes_in_conf.size(); i++) {
//             if (vertexes_in_conf[i] != other.vertexes_in_conf[i]) {
//                 return true;
//             }
//         }
//         return false;
//     }

//     Conf (std::vector<int> vrtxs, int st_len, int idd) {
//         vertexes_in_conf = vrtxs;
//         start_len = st_len;
//         idx_in_all_v = idd;
//     }   
// };

// struct ConfQueue{
//     std::vector<Conf> queue;
//     int cur_idx = 0;

//     void add(const Conf& other) {
//         for (auto& c: queue) {
//             if (!(c != other)) {
//                 return;
//             }
//         }
//         queue.push_back(other);
//     }
// };

///////////////////////////////////////////////
bool call_binary(std::string command);
///////////////////////////////////////////////
// Кодирование переменной CNF: вершина v (индекс), цвет c (1-based)
int encode(int v_index, int c, int n_vertices);


bool is_colorable(const std::vector<int>& vertices, int tip, int ncolors, int k, std::string filename, std::string command_to_run);


std::vector<int> reorder_by_sum_distance(
    const std::vector<int>& base_v,
    const std::vector<int>& poten_v, int from, int to);

std::vector<int> reorder_by_max_elimination(
    const std::vector<int>& base_v,
    const std::vector<int>& poten_v, int from, int to, int k);

std::vector<int> reorder_vertices(
    const std::vector<int>& base_v,
    const std::vector<int>& poten_v, int from, int to,
    const std::string& strategy = "sum_distance",
    int k = 0);

std::vector<int> prepare_to_color(
    const std::vector<int>& base_v,
    const std::vector<int>& brute_v,
    size_t from1,
    const std::vector<int>& other_v,
    size_t from2,
    int n, int k, const std::vector<FuncPtr>& functions

);

// void bruteforce_nolimits(
//     std::map<int, int>& results,
//     ConfQueue& que,
//     std::vector<int> base_v, 
//     const std::vector<int>& vertexes_to_work_on,
//     int id_in_work,
//     const std::vector<int>& poten_v,
//     int id_in_poten,
//     const std::vector<FuncPtr>& filters,
//     const std::string& command,
//     const std::string& filename,
//     int n,
//     int k,
//     int tip
// );

// void staged_bruteforce(
//     std::map<int, int>& results,
//     ConfQueue& que,
//     const std::vector<int>& poten_v,
//     const std::vector<FuncPtr>& filters,
//     const std::string& command,
//     const std::string& filename,
//     int n,
//     int k,
//     int tip,
//     int chunk_size);

std::vector<std::pair<int, std::vector<int>>> all_perms(std::vector<int> vertexes, int n);

std::vector<std::pair<int, std::vector<int>>> all_perms_optimised(std::vector<int> vertexes, int n);

int permute_bits(int num, const std::vector<int>& perm, int n);
