#include <algorithm>
#include <charconv>
#include <chrono>
#include <cmath>
#include <fstream>
#include <future>
#include <numeric>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

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
}  // namespace std
#endif

struct SimulationData {
    std::vector<long long> start_positions;  // Absolute position before move i
    std::vector<long long> deltas;           // The move amount for i
    size_t size() const {
        return deltas.size();
    }
};

struct Results {
    long long part1 = 0;
    long long part2 = 0;

    Results& operator+=(const Results& other) {
        part1 += other.part1;
        part2 += other.part2;
        return *this;
    }
};

std::vector<std::string> read_input(std::string_view filename) {
    std::ifstream file(filename.data());
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Failed to open: {}", filename));
    }
    std::vector<std::string> lines;
    std::string line;
    // Pre-reserving reduces allocation overhead
    lines.reserve(2000);
    while (std::getline(file, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

template <typename ResultT, typename Func>
ResultT run_parallel_task(size_t total_count, Func process_chunk) {
    const unsigned int hc = std::thread::hardware_concurrency();
    const unsigned int threads = (hc != 0) ? hc : 2;
    const size_t chunk_size = (total_count + threads - 1) / threads;

    std::vector<std::future<ResultT>> futures;
    futures.reserve(threads);

    for (unsigned int i = 0; i < threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = std::min(start + chunk_size, total_count);

        if (start >= total_count) break;

        // Launch async task
        futures.push_back(std::async(std::launch::async, [start, end, process_chunk]() {
            return process_chunk(start, end);
        }));
    }

    // Accumulate results
    ResultT total{};  // Default construct
    for (auto& f : futures) {
        total += f.get();  // Assumes ResultT has operator+=
    }
    return total;
}

SimulationData prepare_data_parallel(const std::vector<std::string>& lines) {
    size_t count = lines.size();
    std::vector<long long> deltas(count);

    // 1. Parallel Parse
    // We use a lambda that returns int (dummy) to fit the reduction template,
    // but we act on the captured 'deltas' vector via side-effects.
    auto parse_chunk = [&](size_t start, size_t end) -> int {
        for (size_t i = start; i < end; ++i) {
            std::string_view line = lines[i];
            if (line.empty()) {
                deltas[i] = 0;
                continue;
            }

            char dir = line[0];
            int mag = 0;
            std::from_chars(line.data() + 1, line.data() + line.size(), mag);
            deltas[i] = (dir == 'R') ? mag : -mag;
        }
        return 0;
    };

    run_parallel_task<int>(count, parse_chunk);

    // 2. Serial Scan (Prefix Sum)
    // Calculating absolute start positions is inherently sequential (dependency chain),
    // but extremely fast for simple additions.
    std::vector<long long> starts(count);
    starts[0] = 50;  // Initial position
    if (count > 1) {
        std::inclusive_scan(deltas.begin(), deltas.end() - 1, starts.begin() + 1,
                            std::plus<long long>(), 50LL);
    }

    return {std::move(starts), std::move(deltas)};
}

void solve_all_fast(const SimulationData& data) {
    // Combined Parallel Logic for Part 1 & Part 2
    auto logic = [&](size_t start_idx, size_t end_idx) -> Results {
        Results local_res;

        for (size_t i = start_idx; i < end_idx; ++i) {
            long long start_abs = data.start_positions[i];
            long long delta = data.deltas[i];
            long long final_abs = start_abs + delta;

            // --- Part 1 Logic ---
            // Check if final position aligns with 0 mod 100
            if (final_abs % 100 == 0) {
                local_res.part1++;
            }

            // --- Part 2 Logic ---
            // Count integer multiples of 100 in the interval
            long long high, low;

            if (delta > 0) {
                // Moving Right: Range (start, start + delta]
                low = start_abs;
                high = final_abs;
                // Count multiples: floor(high/100) - floor(low/100)
                local_res.part2 += (long long)(std::floor(high / 100.0) - std::floor(low / 100.0));
            } else if (delta < 0) {
                // Moving Left: Range [start + delta, start)
                // Maps to interval [start + delta, start - 1]
                low = final_abs - 1;
                high = start_abs - 1;
                local_res.part2 += (long long)(std::floor(high / 100.0) - std::floor(low / 100.0));
            }
        }
        return local_res;
    };

    Results final_res = run_parallel_task<Results>(data.size(), logic);

    std::println("Part 1: {}", final_res.part1);
    std::println("Part 2: {}", final_res.part2);
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";

        auto lines = read_input(filename);
        SimulationData data = prepare_data_parallel(lines);
        solve_all_fast(data);

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::println("Error: {}", e.what());
        return 1;
    }
    return 0;
}