#include <charconv>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
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

struct Point {
    long long col;
    long long row;
};

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

auto convert_input_to_point(const std::vector<std::string>& data) {
    auto parsed_view = data | std::views::transform([](std::string_view line) -> Point {
                           int x = 0, y = 0;
                           const char* end = line.data() + line.size();

                           auto [ptr, ec] = std::from_chars(line.data(), end, x);

                           if (ec == std::errc() && ptr != end && *ptr == ',') {
                               std::from_chars(ptr + 1, end, y);
                           }

                           return {y, x};
                       });

    return std::vector<Point>(parsed_view.begin(), parsed_view.end());
}

auto part_one_sol(const std::vector<std::string>& data) {
    auto positions = convert_input_to_point(data);

    unsigned long long max_area = 0;

    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
            unsigned long long width = std::abs(positions[i].col - positions[j].col) + 1;
            unsigned long long height = std::abs(positions[i].row - positions[j].row) + 1;
            unsigned long long area = width * height;

            if (area > max_area) {
                max_area = area;
            }
        }
    }

    return max_area;
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = read_lines(filename);

        std::println("part 1 answer: {} ", part_one_sol(data));

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}