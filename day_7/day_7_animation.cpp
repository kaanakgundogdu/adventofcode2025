#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <iostream>
#include <ranges>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include "utils.h"

// --- Animation Constants ---
const int VIEW_HEIGHT = 40;
const std::string ANSI_RESET = "\033[0m";
const std::string ANSI_BEAM = "\033[1;32m";      // Bright Green
const std::string ANSI_SPLIT = "\033[1;31m";     // Bright Red
const std::string ANSI_DIM = "\033[90m";         // Dark Gray (Inside dots)
const std::string ANSI_CLEAR = "\033[2J\033[H";  // Clear screen + Home
const std::string ANSI_HOME = "\033[H";          // Home cursor
const std::string ANSI_CLEAR_BELOW = "\033[J";   // Clear rest of screen

// --- Animation Logic ---
void animate_solution(const std::vector<std::string>& original_data) {
    std::vector<std::string> canvas = original_data;
    long long total_splits = 0;  // Solution Counter

    // Find Start
    size_t start_col = std::string::npos;
    for (auto [idx, c] : original_data[0] | std::views::enumerate) {
        if (c == 'S') {
            start_col = idx;
            break;
        }
    }

    if (start_col == std::string::npos) {
        std::println("No Start (S) found.");
        return;
    }

    std::set<size_t> active_beams;
    active_beams.insert(start_col);

    std::cout << "\033[?25l";  // Hide Cursor
    std::cout << ANSI_CLEAR;

    for (size_t row = 0; row < original_data.size(); ++row) {
        if (active_beams.empty()) break;

        std::set<size_t> next_beams;

        // --- 1. Update Logic & Count Splits ---
        for (size_t col : active_beams) {
            char c = original_data[row][col];

            if (c == '^') {
                // We hit a splitter!
                total_splits++;
                canvas[row][col] = '#';  // Mark as hit visually

                // Split Left
                if (col > 0) next_beams.insert(col - 1);
                // Split Right
                if (col < original_data[row].size() - 1) next_beams.insert(col + 1);

            } else {
                // Continue Straight
                next_beams.insert(col);
                // Draw Trail
                if (canvas[row][col] == '.') canvas[row][col] = '|';
            }
        }

        // --- 2. Calculate Camera View ---
        int map_height = static_cast<int>(original_data.size());
        int cam_start = static_cast<int>(row) - (VIEW_HEIGHT / 2);
        if (cam_start < 0) cam_start = 0;
        int cam_end = std::min(map_height, cam_start + VIEW_HEIGHT);
        if (cam_end == map_height) cam_start = std::max(0, map_height - VIEW_HEIGHT);

        // --- 3. Render Window ---
        std::cout << ANSI_HOME;
        std::println("Row: {} / {} | Total Splits: {}", row, map_height, total_splits);
        std::println("--------------------------------------------------");

        for (int r = cam_start; r < cam_end; ++r) {
            const std::string& line = canvas[r];

            size_t first_content = line.find_first_not_of(". ");
            size_t last_content = line.find_last_not_of(". ");

            for (size_t c_idx = 0; c_idx < line.size(); ++c_idx) {
                char c = line[c_idx];

                // COLOR LOGIC
                if (c == '|')
                    std::cout << ANSI_BEAM << '|' << ANSI_RESET;
                else if (c == '#')
                    std::cout << ANSI_SPLIT << '^' << ANSI_RESET;
                else if (c == 'S')
                    std::cout << ANSI_BEAM << 'S' << ANSI_RESET;
                else if (c == '.') {
                    // MASKING LOGIC:
                    // Only draw the dot if it is "inside" the current triangle structure
                    if (first_content != std::string::npos && c_idx > first_content &&
                        c_idx < last_content) {
                        std::cout << ANSI_DIM << '.' << ANSI_RESET;
                    } else {
                        std::cout << ' ';  // Hide outside dots
                    }
                } else {
                    std::cout << c;
                }
            }
            std::cout << '\n';
        }

        std::cout << ANSI_CLEAR_BELOW;

        // --- 4. Timing ---
        // Slight speed up since it can get long
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        active_beams = std::move(next_beams);
    }

    std::cout << "\033[?25h";  // Show Cursor
    std::println("\nAnimation Finished. Final Split Count: {}", total_splits);
}

int main(int argc, char* argv[]) {
    try {
        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        auto data = aoc::read_lines(filename, "day_7");

        std::cout << "\nPress ENTER to start animation...";
        std::cin.get();
        animate_solution(data);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}