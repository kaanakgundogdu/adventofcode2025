#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
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

std::vector<std::string> read_lines(const std::string& filename) {
    if (!std::filesystem::exists(filename)) {
        throw std::runtime_error("File '" + filename + "' does not exist.");
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file '" + filename + "'.");
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    return lines;
}

int64_t calculate_max_bank_joltage(std::string_view bank, size_t k) {
    if (bank.length() < k) {
        throw std::runtime_error("Bank length is smaller than required selection count");
    }

    std::string result;
    result.reserve(bank.length());
    size_t to_drop = bank.length() - k;

    for (char digit : bank) {
        if (!isdigit(digit)) {
            throw std::invalid_argument("Bank contains non-digit characters");
        }

        while (to_drop > 0 && !result.empty() && result.back() < digit) {
            result.pop_back();
            to_drop--;
        }
        result.push_back(digit);
    }

    result.resize(k);
    return std::stoll(result);
}

void part_one_sol(const std::vector<std::string>& banks) {
    int64_t total = 0;
    for (const auto& bank : banks) {
        try {
            total += calculate_max_bank_joltage(bank, 2);
        } catch (...) {
            continue;
        }
    }
    std::println("Part 1 Total Output: {}", total);
}

void part_two_sol(const std::vector<std::string>& banks) {
    int64_t total = 0;
    for (const auto& bank : banks) {
        try {
            total += calculate_max_bank_joltage(bank, 12);
        } catch (...) {
            continue;
        }
    }
    std::println("Part 2 Total Output: {}", total);
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = read_lines(filename);

        part_one_sol(data);
        part_two_sol(data);

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}