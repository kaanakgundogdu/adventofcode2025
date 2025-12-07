#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <set>
#include <string>
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
    line.reserve(64);

    std::vector<std::string> file_content;

    while (std::getline(file, line)) {
        file_content.push_back(line);
    }

    if (file_content.empty()) {
        throw std::runtime_error("Error reading file content");
    }

    return file_content;
}

void print_map(std::map<size_t, bool>& m) {
    std::cout << "map:";
    for (const auto& [key, value] : m) std::cout << '[' << key << "] = " << value << "; ";
    std::cout << '\n';
}

auto part_two_sol(const std::vector<std::string>& data) {
    auto find_start = [&]() -> size_t {
        for (auto [idx, c] : data[0] | std::views::enumerate) {
            if (c == 'S') return idx;
        }
        return std::string::npos;
    };

    size_t start_col = find_start();
    if (start_col == std::string::npos) return 0ULL;

    std::map<size_t, unsigned long long> timelines;
    timelines[start_col] = 1;

    for (size_t row = 0; row < data.size(); ++row) {
        if (timelines.empty()) break;

        std::map<size_t, unsigned long long> next_timelines;

        for (auto const& [col, count] : timelines) {
            char c = data[row][col];

            if (c == '^') {
                if (col > 0) {
                    next_timelines[col - 1] += count;
                }
                if (col < data[row].size() - 1) {
                    next_timelines[col + 1] += count;
                }
            } else {
                next_timelines[col] += count;
            }
        }

        timelines = std::move(next_timelines);
    }

    unsigned long long total_timelines = 0;
    for (const auto& [col, count] : timelines) {
        total_timelines += count;
    }

    return total_timelines;
}

auto part_one_sol(const std::vector<std::string>& data) {
    unsigned long long answer = 0;

    auto find_source_index = [&]() -> size_t {
        for (auto [index, value] : data[0] | std::views::enumerate) {
            if (value == 'S') return index;
        }
        return std::string::npos;
    };

    std::set<size_t> active_beams;
    size_t start_idx = find_source_index();

    if (start_idx != std::string::npos) {
        active_beams.insert(start_idx);
    }

    for (size_t i = 0; i < data.size(); ++i) {
        if (active_beams.empty()) break;

        std::set<size_t> next_beams;

        for (size_t col : active_beams) {
            char c = data[i][col];

            if (c == '^') {
                answer++;

                if (col > 0) {
                    next_beams.insert(col - 1);
                }
                if (col < data[i].size() - 1) {
                    next_beams.insert(col + 1);
                }
            } else {
                next_beams.insert(col);
            }
        }

        active_beams = std::move(next_beams);
    }

    return answer;
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = read_lines(filename);

        std::println("part 1 answer: {} ", part_one_sol(data));
        std::println("part 2 answer: {} ", part_two_sol(data));

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}