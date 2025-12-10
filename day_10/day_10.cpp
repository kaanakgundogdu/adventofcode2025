#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "utils.h"

auto get_light_diagrams(const std::vector<std::string>& data) {
    std::vector<std::string> dia{};

    for (auto x : data) {
        std::string s{""};
        for (auto y : x) {
            if (y == '[') {
                continue;
            }
            if (y == ']') {
                break;
            }

            s.push_back(y);
        }

        dia.push_back(s);
    }
    return dia;
}

std::vector<int> parse_numbers(const std::string& s) {
    std::vector<int> nums;
    std::stringstream ss(s);
    std::string segment;

    while (std::getline(ss, segment, ',')) {
        if (!segment.empty()) {
            nums.push_back(std::stoi(segment));
        }
    }
    return nums;
}

auto get_button_schema(const std::vector<std::string>& data) {
    std::vector<std::vector<std::vector<int>>> schema;

    for (const auto& line : data) {
        std::vector<std::vector<int>> line_groups;

        size_t start_pos = line.find(']');
        if (start_pos == std::string::npos) {
            start_pos = 0;
        }

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
        schema.push_back(line_groups);
    }
    return schema;
}

// --- PART 1 LOGIC ---

int solve_single_machine_p1(const std::string& target_pattern,
                            std::vector<std::vector<int>> buttons) {
    int num_lights = target_pattern.length();
    std::vector<int> target(num_lights);
    for (int i = 0; i < num_lights; ++i) {
        target[i] = (target_pattern[i] == '#') ? 1 : 0;
    }

    // Remove the last group (targets for part 2)
    if (!buttons.empty()) {
        buttons.pop_back();
    }

    int num_buttons = buttons.size();
    int min_presses = std::numeric_limits<int>::max();
    bool found_solution = false;

    // Brute Force: 2^N subsets
    unsigned long long limit = 1ULL << num_buttons;

    for (unsigned long long mask = 0; mask < limit; ++mask) {
        int current_presses = std::popcount(mask);

        if (found_solution && current_presses >= min_presses) {
            continue;
        }

        std::vector<int> state(num_lights, 0);

        for (int b = 0; b < num_buttons; ++b) {
            if ((mask >> b) & 1) {
                for (int light_idx : buttons[b]) {
                    if (light_idx < num_lights) {
                        state[light_idx] ^= 1;
                    }
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

auto part_one_sol(const std::vector<std::string>& data) {
    unsigned long long answer = 0;
    auto light_diagrams = get_light_diagrams(data);
    auto all_buttons = get_button_schema(data);

    for (size_t i = 0; i < light_diagrams.size(); ++i) {
        answer += solve_single_machine_p1(light_diagrams[i], all_buttons[i]);
    }
    return answer;
}

// --- PART 2 LOGIC ---

long long solve_linear_system(const std::vector<std::vector<int>>& buttons,
                              const std::vector<int>& targets) {
    int rows = targets.size();  // Number of counters
    int cols = buttons.size();  // Number of buttons

    // Matrix M [rows x (cols + 1)] using double for Gaussian Elimination
    std::vector<std::vector<double>> M(rows, std::vector<double>(cols + 1, 0.0));

    // Fill Matrix
    // M[r][c] = 1 if button c affects counter r
    for (int c = 0; c < cols; ++c) {
        for (int r : buttons[c]) {
            if (r < rows) {
                M[r][c] = 1.0;
            }
        }
    }
    // Fill augmented column with targets
    for (int r = 0; r < rows; ++r) {
        M[r][cols] = static_cast<double>(targets[r]);
    }

    // Gaussian Elimination
    int pivot_row = 0;
    std::vector<int> col_to_pivot_row(cols, -1);
    std::vector<int> pivot_cols;
    std::vector<int> free_cols;

    for (int c = 0; c < cols && pivot_row < rows; ++c) {
        // Find pivot
        int sel = -1;
        for (int r = pivot_row; r < rows; ++r) {
            if (std::abs(M[r][c]) > 1e-9) {
                sel = r;
                break;
            }
        }

        if (sel == -1) {
            continue;  // This column is a free variable candidate
        }

        // Swap rows
        std::swap(M[pivot_row], M[sel]);

        // Normalize pivot row
        double div = M[pivot_row][c];
        for (int j = c; j <= cols; ++j) {
            M[pivot_row][j] /= div;
        }

        // Eliminate other rows
        for (int r = 0; r < rows; ++r) {
            if (r != pivot_row && std::abs(M[r][c]) > 1e-9) {
                double factor = M[r][c];
                for (int j = c; j <= cols; ++j) {
                    M[r][j] -= factor * M[pivot_row][j];
                }
            }
        }

        col_to_pivot_row[c] = pivot_row;
        pivot_cols.push_back(c);
        pivot_row++;
    }

    // Identify free columns
    for (int c = 0; c < cols; ++c) {
        if (col_to_pivot_row[c] == -1) free_cols.push_back(c);
    }

    // Check for inconsistency (0 = non-zero) in remaining rows
    for (int r = pivot_row; r < rows; ++r) {
        if (std::abs(M[r][cols]) > 1e-4) return 0;  // No solution
    }

    long long min_total_presses = -1;

    // Search function for free variables
    // Given the small input size (targets ~100-200), we can iterate reasonable bounds for free
    // variables.
    auto recursive_search = [&](auto&& self, size_t free_idx, std::vector<int>& free_vals) -> void {
        if (free_idx == free_cols.size()) {
            // Calculate dependent variables (pivots) based on free variables
            long long current_presses = 0;
            for (int v : free_vals) current_presses += v;

            bool possible = true;

            for (int pc : pivot_cols) {
                int r = col_to_pivot_row[pc];
                double val = M[r][cols];  // Start with constant

                // Subtract contributions from free variables
                for (size_t i = 0; i < free_cols.size(); ++i) {
                    val -= M[r][free_cols[i]] * free_vals[i];
                }

                // Check if result is non-negative integer
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

                // Optimization: prune if already worse
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

        // Iterate free variable value.
        // Based on input (targets ~100-200), free vars shouldn't exceed max target significantly.
        // A limit of 300 is safe and fast for this puzzle size.
        for (int v = 0; v <= 300; ++v) {
            free_vals[free_idx] = v;
            self(self, free_idx + 1, free_vals);

            // Heuristic pruning: if just free vars sum exceeds best, stop loop
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

auto part_two_sol(const std::vector<std::string>& data) {
    unsigned long long answer = 0;
    auto all_schema = get_button_schema(data);

    for (size_t i = 0; i < all_schema.size(); ++i) {
        auto& group = all_schema[i];
        if (group.empty()) continue;

        // Last element is the target vector {3, 5, ...}
        std::vector<int> targets = group.back();

        // Previous elements are buttons (0,1)...
        std::vector<std::vector<int>> buttons = group;
        buttons.pop_back();

        long long presses = solve_linear_system(buttons, targets);

        // Uncomment to debug specific machines
        // std::println("Machine {}: fewest presses = {}", i + 1, presses);

        answer += presses;
    }
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