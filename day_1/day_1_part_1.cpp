#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "utils.h"

namespace fs = std::filesystem;

constexpr int DIAL_START = 50;
constexpr int DIAL_MOD = 100;

struct DialResult {
    int pos;
    long long hits;
};

std::vector<char> load_file(const fs::path& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + filePath.string());
    }

    const auto fileSize = file.tellg();

    if (fileSize == -1) {
        throw std::runtime_error("Failed to determine file size");
    }
    if (fileSize == 0) {
        return {};
    }

    std::vector<char> buffer(fileSize);

    file.seekg(0, std::ios::beg);
    if (!file.read(buffer.data(), fileSize)) {
        throw std::runtime_error("Error reading file content");
    }

    return buffer;
}

constexpr DialResult update_dial(int current, char direction, int value) {
    value %= DIAL_MOD;

    if (direction == 'L') {
        current -= value;
        if (current < 0) {
            current += DIAL_MOD;
        }
    } else if (direction == 'R') {
        current += value;
        if (current >= DIAL_MOD) {
            current -= DIAL_MOD;
        }
    } else {
        throw std::runtime_error("Unable get direction!");
    }

    long long hits = (current == 0) ? 1 : 0;
    return {current, hits};
}

long long process_instructions(std::string_view data) {
    int dial = DIAL_START;
    long long total_hits = 0;

    const char* ptr = data.data();
    const char* end = data.data() + data.size();

    while (ptr < end) {
        if (static_cast<unsigned char>(*ptr) <= ' ') {
            ptr++;
            continue;
        }

        const char direction = *ptr++;

        if (ptr >= end) {
            break;
        }

        int value = 0;
        auto [next_ptr, ec] = std::from_chars(ptr, end, value);

        if (ptr == next_ptr) {
            ptr++;
            continue;
        }

        ptr = next_ptr;

        DialResult result = update_dial(dial, direction, value);
        dial = result.pos;
        total_hits += result.hits;
    }

    return total_hits;
}

long long get_pass(const fs::path& filePath) {
    const auto buffer = load_file(filePath);
    return process_instructions({buffer.data(), buffer.size()});
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        long long result = get_pass(filename);

        std::cout << "Part 1: " << result << std::endl;

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}