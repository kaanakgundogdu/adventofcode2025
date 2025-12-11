#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "utils.h"

unsigned long long count_paths_memo(
    const std::string& current, const std::string& target,
    const std::unordered_map<std::string, std::vector<std::string>>& graph,
    std::unordered_map<std::string, unsigned long long>& memo) {
    if (memo.count(current)) {
        return memo[current];
    }

    if (current == target) {
        return 1;
    }

    if (graph.find(current) == graph.end()) {
        return 0;
    }

    unsigned long long total_paths = 0;
    const std::vector<std::string>& neighbors = graph.at(current);

    for (const std::string& neighbor : neighbors) {
        total_paths += count_paths_memo(neighbor, target, graph, memo);
    }

    return memo[current] = total_paths;
}

unsigned long long count_paths(
    const std::string& current, const std::string& target,
    const std::unordered_map<std::string, std::vector<std::string>>& graph) {
    std::unordered_map<std::string, unsigned long long> memo;
    return count_paths_memo(current, target, graph, memo);
}

auto create_map(const std::vector<std::string>& data) {
    std::unordered_map<std::string, std::vector<std::string>> graph;

    for (const auto& line : data) {
        size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, colon_pos);

        std::string values_part = line.substr(colon_pos + 1);

        std::stringstream ss(values_part);
        std::string neighbor;
        std::vector<std::string> neighbors_list;

        while (ss >> neighbor) {
            neighbors_list.push_back(neighbor);
        }

        graph[key] = neighbors_list;
    }

    return graph;
}

auto part_one_sol(const std::vector<std::string>& data) {
    unsigned long long answer = 0;
    auto graph = create_map(data);

    answer = count_paths("you", "out", graph);

    return answer;
}

auto part_two_sol(const std::vector<std::string>& data) {
    auto graph = create_map(data);

    unsigned long long leg1_a = count_paths("svr", "dac", graph);
    unsigned long long leg2_a = count_paths("dac", "fft", graph);
    unsigned long long leg3_a = count_paths("fft", "out", graph);
    unsigned long long total_order_a = leg1_a * leg2_a * leg3_a;

    unsigned long long leg1_b = count_paths("svr", "fft", graph);
    unsigned long long leg2_b = count_paths("fft", "dac", graph);
    unsigned long long leg3_b = count_paths("dac", "out", graph);
    unsigned long long total_order_b = leg1_b * leg2_b * leg3_b;

    unsigned long long answer = total_order_a + total_order_b;

    return answer;
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = aoc::read_lines(filename, "day_10");

        std::println("Part 1 Answer: {}", part_one_sol(data));
        std::println("Part 2 Answer: {}", part_two_sol(data));

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}