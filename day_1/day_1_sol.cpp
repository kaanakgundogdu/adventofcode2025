#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <charconv>
#include <filesystem>
#include <system_error>
#include <chrono>

#if __has_include(<print>)
    #include <print>
#else
    #include <format>
    namespace std {
        template <typename... Args>
        void println(format_string<Args...> fmt, Args&&... args) {
            cout << format(fmt, std::forward<Args>(args)...) << '\n';
        }
    }
#endif

namespace fs = std::filesystem;

constexpr int DIAL_START = 50;
constexpr int DIAL_MOD = 100;

struct StepResult
{
    int final_pos;
    long long intermediate_zeros;
};

struct SimulationResult
{
    long long part1;
    long long part2;
};

constexpr StepResult update_dial(int current, char direction, int value)
{
    long long hits = 0;

    if (direction == 'R')
    {
        long long total_steps = static_cast<long long>(current) + value;
        hits = total_steps / DIAL_MOD;
        current = total_steps % DIAL_MOD;
    }
    else if (direction == 'L')
    {
        int dist_to_0 = (current == 0) ? DIAL_MOD : current;

        if (value >= dist_to_0)
        {
            hits++;
            value -= dist_to_0;

            hits += (value / DIAL_MOD);

            int remainder = value % DIAL_MOD;
            current = (DIAL_MOD - remainder) % DIAL_MOD;
        }
        else
        {
            current -= value;
            if (current < 0) current += DIAL_MOD;
        }
    }
    else
    {
        throw std::runtime_error("Unable get direction!");
    }

    return {current, hits};
}

SimulationResult process_instructions(std::string_view data)
{
    int dial = DIAL_START;
    long long p1_hits = 0;
    long long p2_hits = 0;

    const char *ptr = data.data();
    const char *end = data.data() + data.size();

    while (ptr < end)
    {
        if (static_cast<unsigned char>(*ptr) <= ' ')
        {
            ptr++;
            continue;
        }

        const char direction = *ptr++;

        if (ptr >= end) break;

        int value = 0;
        auto [next_ptr, ec] = std::from_chars(ptr, end, value);

        if (ptr == next_ptr)
        {
            ptr++;
            continue;
        }
        ptr = next_ptr;

        StepResult step = update_dial(dial, direction, value);
        
        dial = step.final_pos;

        p2_hits += step.intermediate_zeros;

        if (dial == 0) {
            p1_hits++;
        }
    }

    return {p1_hits, p2_hits};
}

std::vector<char> load_file(const fs::path &filePath)
{
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file)
    {
        throw std::runtime_error("Unable to open file: " + filePath.string());
    }

    const auto fileSize = file.tellg();

    if (fileSize == -1) throw std::runtime_error("Failed to determine file size");
    if (fileSize == 0) return {};

    std::vector<char> buffer(fileSize);

    file.seekg(0, std::ios::beg);
    if (!file.read(buffer.data(), fileSize))
    {
        throw std::runtime_error("Error reading file content");
    }

    return buffer;
}

int main(int argc, char *argv[])
{
    try
    {
        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        
        std::vector<char> buffer = load_file(filename);
        std::string_view data_view(buffer.data(), buffer.size());

        const auto start = std::chrono::high_resolution_clock::now();

        SimulationResult result = process_instructions(data_view);
        
        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::println("Part 1: {}", result.part1);
        std::println("Part 2: {}", result.part2);
        std::println("Total Time: {} µs", duration.count());
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}