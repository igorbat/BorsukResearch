#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "../include/helpers.h"

TEST_CASE("dist", "[dist]") {
    SECTION("Add positive numbers") {
        REQUIRE(dist(0, 1) == 1);
        REQUIRE(dist(0, 2) == 1);
        REQUIRE(dist(0, 3) == 2);
        REQUIRE(dist(0, 7) == 3);
    }
}

TEST_CASE("all_perms basic clique", "[all_perms]") {
    SECTION("basic clique") {
        auto res = all_perms({0b00000, 0b00011, 0b01100, 0b10000}, 5);
        // for (auto rs: res) {
        //     std::cout << rs.first << " {";
        //     for (auto v: rs.second) {
        //         std::cout <<v << ", ";
        //     }
        //     std::cout<<"}\n";
        // }
        REQUIRE(res.size() == 8);
        std::vector<std::pair<int, std::vector<int>>> expected_res = {
            {0, {0, 1, 2, 3, 4}},
            {0, {0, 1, 3, 2, 4}},
            {0, {1, 0, 2, 3, 4}},
            {0, {1, 0, 3, 2, 4}},
            {0, {2, 3, 0, 1, 4}},
            {0, {2, 3, 1, 0, 4}},
            {0, {3, 2, 0, 1, 4}},
            {0, {3, 2, 1, 0, 4}},
        
        };
        REQUIRE(res == expected_res);
    }
}

TEST_CASE("all_perms_optimised", "[all_perms_optimised]") {
    SECTION("matches all_perms on N10K6_4_2") {
        std::vector<int> vertexes = {0, 0b1111110000, 0b1110001110, 0b0101011011};
        int n = 10;
        auto old_res = all_perms(vertexes, n);
        auto new_res = all_perms_optimised(vertexes, n);
        std::sort(old_res.begin(), old_res.end());
        std::sort(new_res.begin(), new_res.end());
        REQUIRE(new_res.size() == old_res.size());
        REQUIRE(new_res == old_res);
    }
}

TEST_CASE("all_perms_optimised_easy_cases", "[all_perms_optimised]") {
    SECTION("matches all_perms on basic clique") {
        std::vector<int> vertexes = {0b00000, 0b00011, 0b01100, 0b10000};
        auto old_res = all_perms(vertexes, 5);
        auto new_res = all_perms_optimised(vertexes, 5);
        std::sort(old_res.begin(), old_res.end());
        std::sort(new_res.begin(), new_res.end());
        REQUIRE(new_res.size() == old_res.size());
        REQUIRE(new_res == old_res);
    }

    SECTION("trivial single vertex {0}") {
        auto res = all_perms_optimised({0}, 3);
        REQUIRE(res.size() == 6);
        for (auto& [x, perm] : res)
            REQUIRE(x == 0);
    }

    SECTION("pair of vertices") {
        auto old_res = all_perms({0, 3}, 3);
        auto new_res = all_perms_optimised({0, 3}, 3);
        std::sort(old_res.begin(), old_res.end());
        std::sort(new_res.begin(), new_res.end());
        REQUIRE(new_res.size() == old_res.size());
        REQUIRE(new_res == old_res);
    }

    SECTION("full cube n=3") {
        std::vector<int> cube;
        for (int i = 0; i < 8; i++) cube.push_back(i);
        auto old_res = all_perms(cube, 3);
        auto new_res = all_perms_optimised(cube, 3);
        std::sort(old_res.begin(), old_res.end());
        std::sort(new_res.begin(), new_res.end());
        REQUIRE(new_res.size() == old_res.size());
        REQUIRE(new_res == old_res);
    }

    SECTION("n=4 k=2 style set") {
        std::vector<int> vertexes = {0, 3, 12, 15};
        auto old_res = all_perms(vertexes, 4);
        auto new_res = all_perms_optimised(vertexes, 4);
        std::sort(old_res.begin(), old_res.end());
        std::sort(new_res.begin(), new_res.end());
        REQUIRE(new_res.size() == old_res.size());
        REQUIRE(new_res == old_res);
    }

    SECTION("asymmetric set") {
        std::vector<int> vertexes = {0, 1, 3};
        auto old_res = all_perms(vertexes, 3);
        auto new_res = all_perms_optimised(vertexes, 3);
        std::sort(old_res.begin(), old_res.end());
        std::sort(new_res.begin(), new_res.end());
        REQUIRE(new_res.size() == old_res.size());
        REQUIRE(new_res == old_res);
    }
}
