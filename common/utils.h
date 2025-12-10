#pragma once

#include <filesystem>  // IWYU pragma: export
#include <fstream>     // IWYU pragma: export
#include <iostream>    // IWYU pragma: export
#include <optional>    // IWYU pragma: export
#include <string>      // IWYU pragma: export
#include <vector>      // IWYU pragma: export

// Polyfill for std::print (C++23)
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

namespace aoc {

struct Point {
    long long col;
    long long row;
};

inline std::optional<fs::path> find_file(const fs::path& filename, std::string_view context = "") {
    if (fs::exists(filename)) return filename;

    if (!context.empty()) {
        fs::path folder_path = fs::path(context) / filename;
        if (fs::exists(folder_path)) return folder_path;
    }

    if (!context.empty()) {
        fs::path parent_path = fs::path("..") / context / filename;
        if (fs::exists(parent_path)) return parent_path;
    }

    return std::nullopt;
}

inline auto read_lines(const fs::path& filename, std::string_view context = "") {
    auto resolvedPath = find_file(filename, context);

    if (!resolvedPath) {
        std::cerr << "[Error] Could not find file: " << filename << "\n";
        std::cerr << "        Checked CWD: " << fs::current_path() << "\n";
        if (!context.empty()) {
            std::cerr << "        Checked context: " << context << "\n";
        }
        throw std::runtime_error("File not found");
    }

    auto fileSize = fs::file_size(*resolvedPath);
    if (fileSize == 0) throw std::runtime_error("File empty");

    std::ifstream file(*resolvedPath);
    if (!file) throw std::runtime_error("Unable to open file: " + resolvedPath->string());

    std::string line;
    std::vector<std::string> file_content;
    while (std::getline(file, line)) {
        file_content.push_back(line);
    }
    return file_content;
}
}  // namespace aoc