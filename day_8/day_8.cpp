#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <iterator>
#include <map>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "utils.h"

struct Position {
    int x, y, z;
};

auto convert_input_to_pos(const std::vector<std::string>& data) {
    auto parse_line = [](std::string_view line) -> Position {
        auto parts = line | std::views::split(',');
        auto it = parts.begin();

        auto to_int = [&](auto&& rng) {
            int value;
            std::from_chars(rng.data(), rng.data() + rng.size(), value);
            return value;
        };

        return Position{to_int(*it++), to_int(*it++), to_int(*it)};
    };

    auto view = data | std::views::transform(parse_line);
    std::vector<Position> positions;
    positions.reserve(data.size());

    std::ranges::copy(view, std::back_inserter(positions));

    return positions;
}

bool is_graph_fully_connected(size_t total_nodes, std::map<size_t, std::vector<size_t>>& adj_map) {
    if (total_nodes == 0) return false;

    std::map<size_t, bool> visited;
    std::vector<size_t> queue;
    queue.reserve(total_nodes);

    size_t start_node = 0;
    queue.push_back(start_node);
    visited[start_node] = true;

    size_t count_visited = 0;
    size_t head = 0;

    while (head < queue.size()) {
        size_t current_node = queue[head++];
        count_visited++;

        for (size_t neighbor : adj_map[current_node]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                queue.push_back(neighbor);
            }
        }
    }

    return count_visited == total_nodes;
}

auto part_two_sol(const std::vector<std::string>& data) {
    auto positions = convert_input_to_pos(data);
    size_t positions_size = positions.size();

    struct Edge {
        size_t u, v;
        long long dist_sq;
    };

    std::vector<Edge> edges;
    edges.reserve(positions_size * (positions_size - 1) / 2);

    for (size_t i = 0; i < positions_size; ++i) {
        for (size_t j = i + 1; j < positions_size; ++j) {
            long long dx = positions[i].x - positions[j].x;
            long long dy = positions[i].y - positions[j].y;
            long long dz = positions[i].z - positions[j].z;
            edges.push_back({i, j, dx * dx + dy * dy + dz * dz});
        }
    }

    std::ranges::sort(edges, {}, &Edge::dist_sq);

    std::map<size_t, std::vector<size_t>> adj_map;

    for (const auto& edge : edges) {
        adj_map[edge.u].push_back(edge.v);
        adj_map[edge.v].push_back(edge.u);

        if (is_graph_fully_connected(positions_size, adj_map)) {
            return static_cast<long long>(positions[edge.u].x) * positions[edge.v].x;
        }
    }

    return 0LL;
}

auto find_small_distances(const std::vector<Position>& positions, size_t num_connections = 1000) {
    struct Edge {
        size_t u, v;
        long long dist_sq;
    };

    std::vector<Edge> all_edges;
    all_edges.reserve(positions.size() * (positions.size() - 1) / 2);

    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
            long long dx = positions[i].x - positions[j].x;
            long long dy = positions[i].y - positions[j].y;
            long long dz = positions[i].z - positions[j].z;
            all_edges.push_back({i, j, dx * dx + dy * dy + dz * dz});
        }
    }

    if (num_connections < all_edges.size()) {
        std::ranges::partial_sort(all_edges, all_edges.begin() + num_connections, {},
                                  &Edge::dist_sq);
        all_edges.resize(num_connections);
    } else {
        std::ranges::sort(all_edges, {}, &Edge::dist_sq);
    }

    std::vector<std::vector<size_t>> adj(positions.size());
    for (const auto& edge : all_edges) {
        adj[edge.u].push_back(edge.v);
        adj[edge.v].push_back(edge.u);
    }

    std::vector<bool> visited(positions.size(), false);
    std::vector<long long> circuit_sizes;

    for (size_t i = 0; i < positions.size(); ++i) {
        if (!visited[i]) {
            long long current_size = 0;
            std::vector<size_t> stack = {i};
            visited[i] = true;

            while (!stack.empty()) {
                size_t u = stack.back();
                stack.pop_back();
                current_size++;

                for (size_t v : adj[u]) {
                    if (!visited[v]) {
                        visited[v] = true;
                        stack.push_back(v);
                    }
                }
            }
            circuit_sizes.push_back(current_size);
        }
    }

    std::ranges::sort(circuit_sizes, std::greater<>());

    std::println("Found {} circuits. Largest 3: {}, {}, {}", circuit_sizes.size(),
                 circuit_sizes.size() > 0 ? circuit_sizes[0] : 0,
                 circuit_sizes.size() > 1 ? circuit_sizes[1] : 0,
                 circuit_sizes.size() > 2 ? circuit_sizes[2] : 0);

    return circuit_sizes;
}

auto part_one_sol(const std::vector<std::string>& data) {
    unsigned long long answer = 1;

    auto positions = convert_input_to_pos(data);
    auto circuit_sizes = find_small_distances(positions);

    for (size_t i = 0; i < 3 && i < circuit_sizes.size(); ++i) {
        answer *= circuit_sizes[i];
    }

    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
        }
    }

    return answer;
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = aoc::read_lines(filename, "day_8");

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