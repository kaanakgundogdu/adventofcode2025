#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <execution>
#include <filesystem>
#include <iostream>
#include <limits>
#include <numeric>
#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "utils.h"

auto get_light_diagrams(const std::vector<std::string>& data) {
    std::vector<std::string> dia;
    dia.reserve(data.size());

    for (const auto& x : data) {
        std::string s;
        s.reserve(x.size());
        for (char y : x) {
            if (y == '[') continue;
            if (y == ']') break;
            s.push_back(y);
        }
        dia.push_back(std::move(s));
    }
    return dia;
}

std::vector<int> parse_numbers(const std::string& s) {
    std::vector<int> nums;
    std::stringstream ss(s);
    std::string segment;
    while (std::getline(ss, segment, ',')) {
        if (!segment.empty()) nums.push_back(std::stoi(segment));
    }
    return nums;
}

auto get_button_schema(const std::vector<std::string>& data) {
    std::vector<std::vector<std::vector<int>>> schema;
    schema.reserve(data.size());

    for (const auto& line : data) {
        std::vector<std::vector<int>> line_groups;
        size_t start_pos = line.find(']');
        if (start_pos == std::string::npos) start_pos = 0;

        for (size_t i = start_pos; i < line.length(); ++i) {
            char c = line[i];
            if (c == '(' || c == '{') {
                char closing_char = (c == '(') ? ')' : '}';
                size_t end = line.find(closing_char, i);
                if (end != std::string::npos) {
                    std::string content = line.substr(i + 1, end - (i + 1));
                    line_groups.push_back(parse_numbers(content));
                    i = end;
                }
            }
        }
        schema.push_back(std::move(line_groups));
    }
    return schema;
}

// --- PART 1 SOLVER ---

// Note: 'buttons' passed by value to ensure thread safety (each thread gets a copy)
int solve_single_machine_p1(const std::string& target_pattern,
                            std::vector<std::vector<int>> buttons) {
    int num_lights = target_pattern.length();
    std::vector<int> target(num_lights);
    for (int i = 0; i < num_lights; ++i) {
        target[i] = (target_pattern[i] == '#') ? 1 : 0;
    }

    if (!buttons.empty()) buttons.pop_back();

    int num_buttons = buttons.size();
    int min_presses = std::numeric_limits<int>::max();
    bool found_solution = false;
    unsigned long long limit = 1ULL << num_buttons;

    for (unsigned long long mask = 0; mask < limit; ++mask) {
        int current_presses = std::popcount(mask);
        if (found_solution && current_presses >= min_presses) continue;

        std::vector<int> state(num_lights, 0);
        for (int b = 0; b < num_buttons; ++b) {
            if ((mask >> b) & 1) {
                for (int light_idx : buttons[b]) {
                    if (light_idx < num_lights) state[light_idx] ^= 1;
                }
            }
        }

        if (state == target) {
            min_presses = current_presses;
            found_solution = true;
        }
    }
    return found_solution ? min_presses : 0;
}

auto part_one_sol(const std::vector<std::string>& light_diagrams,
                  const std::vector<std::vector<std::vector<int>>>& all_buttons) {
    // Create a range of indices [0, N)
    auto indices = std::views::iota(size_t{0}, light_diagrams.size());

    // Parallel Transform Reduce:
    // 1. Splits indices among threads (std::execution::par)
    // 2. Runs the lambda for each index
    // 3. Sums (reduces) the results using std::plus
    return std::transform_reduce(std::execution::par, indices.begin(), indices.end(),
                                 0ULL,            // Initial value
                                 std::plus<>(),   // Reduction operation (Sum)
                                 [&](size_t i) {  // Transform operation (Solve)
                                     return solve_single_machine_p1(light_diagrams[i],
                                                                    all_buttons[i]);
                                 });
}

// --- PART 2 SOLVER ---

long long solve_linear_system(const std::vector<std::vector<int>>& buttons,
                              const std::vector<int>& targets) {
    int rows = targets.size();
    int cols = buttons.size();

    // Local matrix allocation (thread-safe as it's stack/local heap)
    std::vector<std::vector<double>> M(rows, std::vector<double>(cols + 1, 0.0));

    for (int c = 0; c < cols; ++c) {
        for (int r : buttons[c]) {
            if (r < rows) M[r][c] = 1.0;
        }
    }
    for (int r = 0; r < rows; ++r) M[r][cols] = static_cast<double>(targets[r]);

    int pivot_row = 0;
    std::vector<int> col_to_pivot_row(cols, -1);
    std::vector<int> pivot_cols;
    std::vector<int> free_cols;

    // Gaussian Elimination
    for (int c = 0; c < cols && pivot_row < rows; ++c) {
        int sel = -1;
        for (int r = pivot_row; r < rows; ++r) {
            if (std::abs(M[r][c]) > 1e-9) {
                sel = r;
                break;
            }
        }
        if (sel == -1) continue;

        std::swap(M[pivot_row], M[sel]);
        double div = M[pivot_row][c];
        for (int j = c; j <= cols; ++j) M[pivot_row][j] /= div;
        for (int r = 0; r < rows; ++r) {
            if (r != pivot_row && std::abs(M[r][c]) > 1e-9) {
                double factor = M[r][c];
                for (int j = c; j <= cols; ++j) M[r][j] -= factor * M[pivot_row][j];
            }
        }
        col_to_pivot_row[c] = pivot_row;
        pivot_cols.push_back(c);
        pivot_row++;
    }

    for (int c = 0; c < cols; ++c) {
        if (col_to_pivot_row[c] == -1) free_cols.push_back(c);
    }

    for (int r = pivot_row; r < rows; ++r) {
        if (std::abs(M[r][cols]) > 1e-4) return 0;
    }

    long long min_total_presses = -1;

    // Recursive search for free variables
    auto recursive_search = [&](auto&& self, size_t free_idx, std::vector<int>& free_vals) -> void {
        if (free_idx == free_cols.size()) {
            long long current_presses = 0;
            for (int v : free_vals) current_presses += v;

            bool possible = true;
            for (int pc : pivot_cols) {
                int r = col_to_pivot_row[pc];
                double val = M[r][cols];
                for (size_t i = 0; i < free_cols.size(); ++i) {
                    val -= M[r][free_cols[i]] * free_vals[i];
                }

                if (val < -1e-4) {
                    possible = false;
                    break;
                }
                long long ival = std::llround(val);
                if (std::abs(val - ival) > 1e-4) {
                    possible = false;
                    break;
                }
                current_presses += ival;

                if (min_total_presses != -1 && current_presses >= min_total_presses) {
                    possible = false;
                    break;
                }
            }

            if (possible) {
                if (min_total_presses == -1 || current_presses < min_total_presses) {
                    min_total_presses = current_presses;
                }
            }
            return;
        }

        for (int v = 0; v <= 300; ++v) {
            free_vals[free_idx] = v;
            self(self, free_idx + 1, free_vals);
            if (min_total_presses != -1) {
                long long partial_sum = 0;
                for (size_t k = 0; k <= free_idx; ++k) partial_sum += free_vals[k];
                if (partial_sum >= min_total_presses) break;
            }
        }
    };

    std::vector<int> free_vals(free_cols.size());
    recursive_search(recursive_search, 0, free_vals);

    return (min_total_presses == -1) ? 0 : min_total_presses;
}

auto part_two_sol(const std::vector<std::vector<std::vector<int>>>& all_schema) {
    // Parallel Transform Reduce
    return std::transform_reduce(std::execution::par, all_schema.begin(), all_schema.end(), 0ULL,
                                 std::plus<>(), [](const auto& group) -> long long {
                                     if (group.empty()) return 0;

                                     // Make a local copy of data to modify safely
                                     std::vector<std::vector<int>> buttons = group;
                                     std::vector<int> targets = buttons.back();
                                     buttons.pop_back();

                                     return solve_linear_system(buttons, targets);
                                 });
}

// --- MAIN ---

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = aoc::read_lines(filename, "day_10");

        // Parse once (Sequential parsing is usually fast enough)
        auto light_diagrams = get_light_diagrams(data);
        auto all_buttons = get_button_schema(data);

        // Solve in Parallel
        auto p1 = part_one_sol(light_diagrams, all_buttons);
        auto p2 = part_two_sol(all_buttons);

        std::println("Part 1 Answer: {}", p1);
        std::println("Part 2 Answer: {}", p2);

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}