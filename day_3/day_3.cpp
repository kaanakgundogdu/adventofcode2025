#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "utils.h"

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
        const auto& data = aoc::read_lines(filename, "day_3");

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