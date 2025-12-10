#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "utils.h"

namespace fs = std::filesystem;

void part_two_sol(std::vector<std::pair<unsigned long long, unsigned long long>> range_vec) {
    std::sort(range_vec.begin(), range_vec.end());

    unsigned long long answer_p2 = 0;
    unsigned long long current_start = range_vec[0].first;
    unsigned long long current_end = range_vec[0].second;

    for (size_t i = 1; i < range_vec.size(); ++i) {
        if (range_vec[i].first <= current_end) {
            current_end = std::max(current_end, range_vec[i].second);
        } else {
            answer_p2 += (current_end - current_start + 1);
            current_start = range_vec[i].first;
            current_end = range_vec[i].second;
        }
    }

    answer_p2 += (current_end - current_start + 1);

    std::println("answer part 2: {}", answer_p2);
}

auto part_one_sol(const std::vector<unsigned long long>& ids,
                  const std::vector<std::pair<unsigned long long, unsigned long long>>& range_vec) {
    auto count = std::ranges::count_if(ids, [&](auto id) {
        return std::ranges::any_of(
            range_vec, [&](const auto& range) { return id >= range.first && id <= range.second; });
    });

    std::println("answer part 1: {}", count);
}

auto get_range_vec(const std::vector<std::string>& data, int index) {
    std::vector<std::pair<unsigned long long, unsigned long long>> vec{};

    for (size_t i = 0; i < static_cast<size_t>(index); i++) {
        std::string copy_line = data[i];

        auto pos = copy_line.find('-');

        if (pos != std::string::npos) {
            auto first_part = stoul(copy_line.substr(0, pos));
            copy_line.erase(0, pos + 1);
            auto second_part = stoul(copy_line);
            vec.push_back({first_part, second_part});
        }
    }

    return vec;
}

auto get_ids(const std::vector<std::string>& data, int index) {
    std::vector<unsigned long long> vec{};
    for (size_t i = index + 1; i < data.size(); i++) {
        vec.push_back(stoul(data[i]));
    }

    return vec;
}

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

    std::unordered_map<unsigned long int, bool> map{};

    std::vector<std::string> file_content;
    auto index_of_empty_line = -1;
    int i = 0;

    while (std::getline(file, line)) {
        if (line == "") {
            index_of_empty_line = i;
        }
        file_content.push_back(line);
        i++;
    }

    if (file_content.empty()) {
        throw std::runtime_error("Error reading file content");
    }

    return std::tuple{file_content, index_of_empty_line};
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& [data, index] = read_lines(filename);

        const auto& id_vec = get_ids(data, index);
        auto rvec = get_range_vec(data, index);

        part_one_sol(id_vec, rvec);
        part_two_sol(rvec);
        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}