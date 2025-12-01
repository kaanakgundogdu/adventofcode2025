#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <numeric>
#include <algorithm>
#include <thread>
#include <future>
#include <chrono>
#include <ranges>

// GCC 13/Clang 17 polyfill for C++23 std::print/println
#if __has_include(<print>)
    #include <print>
#else
    #include <format>
    namespace std {
        template <typename... Args>
        void print(format_string<Args...> fmt, Args&&... args) {
            cout << format(fmt, std::forward<Args>(args)...);
        }
        template <typename... Args>
        void println(format_string<Args...> fmt, Args&&... args) {
            cout << format(fmt, std::forward<Args>(args)...) << '\n';
        }
    }
#endif

std::vector<std::string> read_input(std::string_view filename) {
    std::ifstream file(filename.data());
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Failed to open: {}", filename));
    }
    
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    return lines;
}

// C++23 logic helper: constexpr allows compile-time evaluation if inputs are known
constexpr long long parse_and_process(std::string_view line) {
    // Example logic
    return line.length(); 
}

// Parallel worker helper
template <typename T, typename Func>
T run_parallel(const std::vector<std::string>& data, Func process_chunk) {
    const size_t count = data.size();
    const unsigned int threads = std::thread::hardware_concurrency() ?: 2;
    const size_t chunk_size = (count + threads - 1) / threads;

    std::vector<std::future<T>> futures;
    futures.reserve(threads);

    for (unsigned int i = 0; i < threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(start + chunk_size, count);
        
        if (start >= count) break;

        futures.push_back(std::async(std::launch::async, [start, end, &data, process_chunk]() {
            return process_chunk(data, start, end);
        }));
    }

    T total = 0;
    for (auto& f : futures) {
        total += f.get();
    }
    return total;
}

// just a template not a real solution
void solve_part_1(const std::vector<std::string>& lines) {
    long long sum = 0;
    
    // C++23: views::enumerate gives index (i) and value (line)
    for (auto [i, line] : std::views::enumerate(lines)) {
        sum += parse_and_process(line);
    }
    
    std::println("Part 1: {}", sum);
}

// just a template not a real solution
void solve_part_2(const std::vector<std::string>& lines) {
    auto parallel_logic = [](const std::vector<std::string>& d, size_t start, size_t end) {
        long long local_sum = 0;
        for (size_t i = start; i < end; ++i) {
            local_sum += parse_and_process(d[i]);
        }
        return local_sum;
    };

    long long result = run_parallel<long long>(lines, parallel_logic);
    std::println("Part 2 (Parallel): {}", result);
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();
        
        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        auto lines = read_input(filename);

        solve_part_1(lines);
        solve_part_2(lines);

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::println("Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::println("Error: {}", e.what());
        return 1;
    }
    return 0;
}