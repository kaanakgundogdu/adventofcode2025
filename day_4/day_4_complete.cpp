#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "utils.h"

auto part_1_logic(const std::vector<std::string>& data) -> int {
    const std::vector<std::pair<int, int>> directions{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                                                      {0, 1},   {1, -1}, {1, 0},  {1, 1}};
    auto answer = 0;

    for (size_t i = 0; i < data.size(); ++i) {
        for (size_t j = 0; j < data[i].size(); ++j) {
            if (data[i][j] == '.') {
                continue;
            }

            auto rulo_counter = 0;
            for (const auto& [dr, dc] : directions) {
                int neigh_row_index = static_cast<int>(i) + dr;
                int neigh_col_index = static_cast<int>(j) + dc;

                if (neigh_row_index >= 0 && neigh_row_index < std::ssize(data) &&
                    neigh_col_index >= 0 && neigh_col_index < std::ssize(data) &&
                    data[neigh_row_index][neigh_col_index] == '@') {
                    rulo_counter++;
                }
            }

            if (rulo_counter < 4) {
                answer++;
            }
        }
    }

    return answer;
}

auto part_2_logic(std::vector<std::string>& data) -> int {
    const std::vector<std::pair<int, int>> directions{{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                                                      {0, 1},   {1, -1}, {1, 0},  {1, 1}};
    auto answer = 0;
    std::vector<std::pair<int, int>> removed_indexes{};

    for (size_t i = 0; i < data.size(); ++i) {
        for (size_t j = 0; j < data[i].size(); ++j) {
            if (data[i][j] == '.') {
                continue;
            }

            auto rulo_counter = 0;
            for (const auto& [dr, dc] : directions) {
                int neigh_row_index = static_cast<int>(i) + dr;
                int neigh_col_index = static_cast<int>(j) + dc;

                if (neigh_row_index >= 0 && neigh_row_index < std::ssize(data) &&
                    neigh_col_index >= 0 && neigh_col_index < std::ssize(data) &&
                    data[neigh_row_index][neigh_col_index] == '@') {
                    rulo_counter++;
                }
            }

            if (rulo_counter < 4) {
                answer++;
                removed_indexes.push_back({i, j});
            }
        }
    }

    for (const auto& [i, j] : removed_indexes) {
        data[i][j] = '.';
    }

    return answer;
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        auto grid = aoc::read_lines(filename, "day_4");

        auto answer = part_1_logic(grid);

        auto answer_p2 = 0;
        while (true) {
            auto ret = part_2_logic(grid);
            answer_p2 += ret;
            if (ret == 0) {
                break;
            }
        }

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());
        std::println("Answer {} ", answer);
        std::println("Answer {} ", answer_p2);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}