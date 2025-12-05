#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

// GCC 13/Clang 17 polyfill for C++23 std::print
#if __has_include(<print>)
#include <print>
#else
#include <format>
namespace std {
template <typename... Args>
void println(format_string<Args...> fmt, Args&&... args) {
    cout << format(fmt, std::forward<Args>(args)...) << '\n';
}
}  // namespace std
#endif

namespace fs = std::filesystem;

auto read_lines(const fs::path& filePath) {
    auto fileSize = fs::file_size(filePath);
    if (fileSize == 0) {
        throw std::runtime_error("File empty");
    }

    std::ifstream file(filePath);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + filePath.string());
    }

    std::string line;
    line.reserve(135);
    std::vector<std::string> grid{};
    while (std::getline(file, line)) {
        grid.push_back(line);
    }

    if (grid.empty()) {
        throw std::runtime_error("Error reading file content");
    }

    return grid;
}

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

                if (neigh_row_index >= 0 && neigh_row_index < data.size() && neigh_col_index >= 0 &&
                    neigh_col_index < data[i].size() &&
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

                if (neigh_row_index >= 0 && neigh_row_index < data.size() && neigh_col_index >= 0 &&
                    neigh_col_index < data[i].size() &&
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
        auto grid = read_lines(filename);

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