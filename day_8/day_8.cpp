#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <numeric>  // for std::iota
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

struct Position {
    int x, y, z;
};

// Disjoint Set Union (DSU) / Union-Find helper
struct DSU {
    std::vector<size_t> parent;
    size_t components;

    explicit DSU(size_t n) : parent(n), components(n) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    size_t find(size_t i) {
        if (parent[i] == i) return i;
        return parent[i] = find(parent[i]);  // Path compression
    }

    // Returns true if a merge happened (u and v were in different sets)
    bool unite(size_t i, size_t j) {
        size_t root_i = find(i);
        size_t root_j = find(j);
        if (root_i != root_j) {
            parent[root_i] = root_j;
            components--;
            return true;
        }
        return false;
    }
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

auto part_two_sol(const std::vector<std::string>& data) {
    auto positions = convert_input_to_pos(data);
    size_t n = positions.size();

    if (n < 2) return 0LL;

    struct Edge {
        size_t u, v;
        long long dist_sq;
    };

    std::vector<Edge> edges;
    edges.reserve(n * (n - 1) / 2);

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            long long dx = positions[i].x - positions[j].x;
            long long dy = positions[i].y - positions[j].y;
            long long dz = positions[i].z - positions[j].z;
            edges.push_back({i, j, dx * dx + dy * dy + dz * dz});
        }
    }

    std::ranges::sort(edges, {}, &Edge::dist_sq);

    // 3. Kruskal's Algorithm
    DSU dsu(n);

    for (const auto& edge : edges) {
        if (dsu.unite(edge.u, edge.v)) {
            if (dsu.components == 1) {
                return static_cast<long long>(positions[edge.u].x) * positions[edge.v].x;
            }
        }
    }

    return 0LL;  // Should not reach here if graph is connected
}

auto find_small_distances(const std::vector<Position>& positions, size_t num_connections = 1000) {
    struct Edge {
        size_t u, v;
        long long dist_sq;
    };

    std::vector<Edge> all_edges;
    // Pre-allocate to avoid reallocations
    all_edges.reserve(positions.size() * (positions.size() - 1) / 2);

    // 1. Generate all pairs
    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
            long long dx = positions[i].x - positions[j].x;
            long long dy = positions[i].y - positions[j].y;
            long long dz = positions[i].z - positions[j].z;
            all_edges.push_back({i, j, dx * dx + dy * dy + dz * dz});
        }
    }

    // 2. Sort to find the shortest distances
    // Using partial_sort is more efficient if we only need the top K
    if (num_connections < all_edges.size()) {
        std::ranges::partial_sort(all_edges, all_edges.begin() + num_connections, {},
                                  &Edge::dist_sq);
        all_edges.resize(num_connections);
    } else {
        std::ranges::sort(all_edges, {}, &Edge::dist_sq);
    }

    // 3. Build Adjacency Graph
    std::vector<std::vector<size_t>> adj(positions.size());
    for (const auto& edge : all_edges) {
        adj[edge.u].push_back(edge.v);
        adj[edge.v].push_back(edge.u);
    }

    // 4. Find Connected Components (Circuits)
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

    // 5. Calculate Product of Top 3
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