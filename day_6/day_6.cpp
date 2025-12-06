#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
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

namespace fs = std::filesystem;

auto get_kth_string(const std::string& str, int K) {
    std::stringstream iss(str);
    std::string kth;

    while (iss >> kth && K) {
        K--;
        if (K == 0) {
            break;
        }
    }

    return kth;
}

auto get_math_symbols(const std::string& str) {
    std::stringstream ss(str);
    std::string symbol;
    std::vector<std::string> symbols;

    while (ss >> symbol) {
        symbols.push_back(symbol);
    }

    return symbols;
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

    std::vector<std::string> file_content;

    while (std::getline(file, line)) {
        file_content.push_back(line);
    }

    if (file_content.empty()) {
        throw std::runtime_error("Error reading file content");
    }

    return file_content;
}

auto part_one_sol(const std::vector<std::string>& data, std::vector<std::string> symbols) {
    unsigned long long answer = 0;

    for (size_t i = 0; i < symbols.size(); i++) {
        unsigned long long x = 1;
        unsigned long long y = 0;

        for (size_t j = 0; j < data.size() - 1; j++) {
            if (symbols[i] == "*") {
                x *= std::stoul(get_kth_string(data[j], i + 1));
            }
            if (symbols[i] == "+") {
                y += std::stoul(get_kth_string(data[j], i + 1));
            }
        }

        if (symbols[i] == "*") {
            answer += x;
        }
        if (symbols[i] == "+") {
            answer += y;
        }
    }

    std::println("answer: {}", answer);

    return answer;
}

auto part_two_sol(const std::vector<std::string>& data,
                  const std::vector<std::string>& /*symbols*/) {
    if (data.empty()) return 0ULL;

    size_t max_width = 0;
    for (const auto& line : data) {
        max_width = std::max(max_width, line.size());
    }
    size_t rows = data.size();

    unsigned long long grand_total = 0;
    std::vector<int> current_block_cols;

    auto process_block = [&](const std::vector<int>& cols) {
        if (cols.empty()) return;

        char op = ' ';
        const std::string& op_row = data.back();
        for (int c : cols) {
            if (static_cast<size_t>(c) < op_row.size() && op_row[c] != ' ') {
                op = op_row[c];
                break;
            }
        }

        std::vector<unsigned long long> numbers;

        for (int c : cols) {
            std::string num_str;
            for (size_t r = 0; r < rows - 1; ++r) {
                if (static_cast<size_t>(c) < data[r].size() && std::isdigit(data[r][c])) {
                    num_str += data[r][c];
                }
            }
            if (!num_str.empty()) {
                numbers.push_back(std::stoull(num_str));
            }
        }

        if (numbers.empty()) return;

        unsigned long long result = numbers[0];
        for (size_t i = 1; i < numbers.size(); ++i) {
            if (op == '+')
                result += numbers[i];
            else if (op == '*')
                result *= numbers[i];
        }
        grand_total += result;
    };

    for (int c = static_cast<int>(max_width) - 1; c >= 0; --c) {
        bool is_separator = true;

        for (const auto& line : data) {
            if (static_cast<size_t>(c) < line.size() && line[c] != ' ') {
                is_separator = false;
                break;
            }
        }

        if (is_separator) {
            process_block(current_block_cols);
            current_block_cols.clear();
        } else {
            current_block_cols.push_back(c);
        }
    }

    process_block(current_block_cols);

    std::println("answer part 2: {}", grand_total);

    return grand_total;
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = read_lines(filename);

        auto symbols = get_math_symbols(data[data.size() - 1]);

        part_one_sol(data, symbols);
        part_two_sol(data, symbols);
        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}