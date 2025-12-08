#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <ranges>
#include <set>
#include <string>
#include <thread>  // Added for sleep
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

// --- Animation Helper: ANSI Colors ---
const std::string ANSI_RESET = "\033[0m";
const std::string ANSI_BEAM = "\033[1;32m";      // Bright Green
const std::string ANSI_SPLIT = "\033[1;31m";     // Bright Red
const std::string ANSI_DIM = "\033[90m";         // Dark Gray
const std::string ANSI_CLEAR = "\033[2J\033[H";  // Clear screen + Home
const std::string ANSI_HOME = "\033[H";          // Home cursor

auto read_lines(const fs::path& filePath) {
    if (!fs::exists(filePath)) {
        // Fallback for demo purposes if no file is provided
        throw std::runtime_error("File not found: " + filePath.string());
    }

    auto fileSize = fs::file_size(filePath);
    if (fileSize == 0) throw std::runtime_error("File empty");

    std::ifstream file(filePath);
    std::string line;
    std::vector<std::string> file_content;

    while (std::getline(file, line)) {
        file_content.push_back(line);
    }
    return file_content;
}

// --- Animation Logic ---
void animate_solution(const std::vector<std::string>& original_data) {
    // Create a canvas that we will draw trails onto
    std::vector<std::string> canvas = original_data;

    // Find Start
    size_t start_col = std::string::npos;
    for (auto [idx, c] : original_data[0] | std::views::enumerate) {
        if (c == 'S') {
            start_col = idx;
            break;
        }
    }

    if (start_col == std::string::npos) {
        std::println("No Start (S) found for animation.");
        return;
    }

    std::set<size_t> active_beams;
    active_beams.insert(start_col);

    std::cout << "\033[?25l";  // Hide Cursor
    std::cout << ANSI_CLEAR;   // Clear initial screen

    for (size_t row = 0; row < original_data.size(); ++row) {
        if (active_beams.empty()) break;

        std::set<size_t> next_beams;

        // 1. Update Canvas for current row based on active beams
        for (size_t col : active_beams) {
            char c = original_data[row][col];

            if (c == '^') {
                // Determine next moves
                if (col > 0) next_beams.insert(col - 1);
                if (col < original_data[row].size() - 1) next_beams.insert(col + 1);

                // Draw splitter on canvas (using a placeholder we can colorize later)
                canvas[row][col] = '#';
            } else {
                // Continue straight
                next_beams.insert(col);

                // Draw beam trail
                if (canvas[row][col] == '.') canvas[row][col] = '|';
            }
        }

        // 2. Render the entire frame
        std::cout << ANSI_HOME;
        for (const auto& line : canvas) {
            for (char c : line) {
                if (c == '|')
                    std::cout << ANSI_BEAM << '|' << ANSI_RESET;
                else if (c == '#')
                    std::cout << ANSI_SPLIT << '^' << ANSI_RESET;
                else if (c == 'S')
                    std::cout << ANSI_BEAM << 'S' << ANSI_RESET;
                else if (c == '.')
                    std::cout << ANSI_DIM << '.' << ANSI_RESET;
                else
                    std::cout << c;  // Original obstacles/other chars
            }
            std::cout << '\n';
        }

        // 3. Wait to create animation effect
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 4. Move to next state
        active_beams = std::move(next_beams);
    }

    std::cout << "\033[?25h";  // Show Cursor again
    std::println("\nAnimation Finished.");
}

// --- Original Logic (Preserved) ---
auto part_two_sol(const std::vector<std::string>& data) {
    auto find_start = [&]() -> size_t {
        for (auto [idx, c] : data[0] | std::views::enumerate) {
            if (c == 'S') return idx;
        }
        return std::string::npos;
    };

    size_t start_col = find_start();
    if (start_col == std::string::npos) return 0ULL;

    std::map<size_t, unsigned long long> timelines;
    timelines[start_col] = 1;

    for (size_t row = 0; row < data.size(); ++row) {
        if (timelines.empty()) break;
        std::map<size_t, unsigned long long> next_timelines;
        for (auto const& [col, count] : timelines) {
            char c = data[row][col];
            if (c == '^') {
                if (col > 0) next_timelines[col - 1] += count;
                if (col < data[row].size() - 1) next_timelines[col + 1] += count;
            } else {
                next_timelines[col] += count;
            }
        }
        timelines = std::move(next_timelines);
    }

    unsigned long long total_timelines = 0;
    for (const auto& [col, count] : timelines) total_timelines += count;
    return total_timelines;
}

auto part_one_sol(const std::vector<std::string>& data) {
    unsigned long long answer = 0;
    auto find_source_index = [&]() -> size_t {
        for (auto [index, value] : data[0] | std::views::enumerate) {
            if (value == 'S') return index;
        }
        return std::string::npos;
    };

    std::set<size_t> active_beams;
    size_t start_idx = find_source_index();
    if (start_idx != std::string::npos) active_beams.insert(start_idx);

    for (size_t i = 0; i < data.size(); ++i) {
        if (active_beams.empty()) break;
        std::set<size_t> next_beams;
        for (size_t col : active_beams) {
            char c = data[i][col];
            if (c == '^') {
                answer++;
                if (col > 0) next_beams.insert(col - 1);
                if (col < data[i].size() - 1) next_beams.insert(col + 1);
            } else {
                next_beams.insert(col);
            }
        }
        active_beams = std::move(next_beams);
    }
    return answer;
}

int main(int argc, char* argv[]) {
    try {
        std::string filename = (argc > 1) ? argv[1] : "input_anim.txt";

        // If file doesn't exist, use the hardcoded example for demonstration
        std::vector<std::string> data;
        if (fs::exists(filename)) {
            data = read_lines(filename);
        } else {
            // Using the user's example if no file found
            std::cout << "File not found, using example data...\n";
            data = {".......S.......", "...............", ".......^.......", "...............",
                    "......^.^......", "...............", ".....^.^.^.....", "...............",
                    "....^.^...^....", "...............", "...^.^...^.^...", "...............",
                    "..^...^.....^..", "...............", ".^.^.^.^.^...^.", "..............."};
        }

        // Run Logic
        const auto start = std::chrono::high_resolution_clock::now();
        std::println("part 1 answer: {} ", part_one_sol(data));
        std::println("part 2 answer: {} ", part_two_sol(data));
        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::println("Total Time: {} µs", duration.count());

        // Run Animation
        std::cout << "\nPress ENTER to start animation...";
        std::cin.get();
        animate_solution(data);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}