#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>
#include <charconv>
#include <filesystem>

struct Range {
    long long start;
    long long end;
};


std::string read_content(const std::filesystem::path& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return {};
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

long long parse_number(std::string_view sv) {
    long long val = 0;
    std::from_chars(sv.data(), sv.data() + sv.size(), val);
    return val;
}

Range make_range(std::string_view sv) {
    auto pivot = sv.find('-');
    if (pivot == std::string_view::npos) 
    {
        return {0, 0};
    }

    return { parse_number(sv.substr(0, pivot)), parse_number(sv.substr(pivot + 1)) };
}

bool is_valid_segment(std::string_view sv) {
    return !sv.empty() && sv.find('-') != std::string_view::npos;
}

std::vector<Range> parse_file(std::string_view content) {
    auto view = content 
        | std::views::split(',') 
        | std::views::transform([](auto&& rng) { 
            return std::string_view(&*rng.begin(), std::ranges::distance(rng)); 
        })
        | std::views::filter(is_valid_segment)
        | std::views::transform(make_range);

    return std::vector<Range>(view.begin(), view.end());
}


bool is_repeated_pattern(long long n) {
    char buf[24]; // ample space for long long (max ~19 digits)
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), n);
    std::string_view s(buf, ptr - buf);
    
    int len = s.length();

    // Check every possible "block size" (sub_len)
    // A block size must be a divisor of the total length
    // We only go up to len/2 because a pattern must repeat at least twice
    for (int sub_len = 1; sub_len <= len / 2; ++sub_len) {
        if (len % sub_len == 0) {
            bool match = true;
            // Optimization: Instead of substring compare, just check back-reference
            // s[i] must equal s[i - block_size]
            for (int i = sub_len; i < len; ++i) {
                if (s[i] != s[i - sub_len]) {
                    match = false;
                    break;
                }
            }
            // If we finished the loop without breaking, we found a valid pattern
            if (match) return true; 
        }
    }
    return false;
}

bool is_double_repeated(long long n) {
    std::string s = std::to_string(n);
    if (s.length() % 2 != 0) {
        return false;
    }
    auto half = s.length() / 2;
    return std::string_view(s).substr(0, half) == std::string_view(s).substr(half);
}

std::tuple<long long,long long> loop_range_elements(const Range& r) {
    unsigned long long sum_of_double_repeated = 0;
    unsigned long long sum_of_repeated_pattern = 0;

    for (auto i : std::views::iota(r.start, r.end + 1)) {
        if (is_double_repeated(i)) {
            sum_of_double_repeated += i;
        }
        if (is_repeated_pattern(i)) {
            sum_of_repeated_pattern += i;
        }
    }

    return {sum_of_double_repeated, sum_of_repeated_pattern};
}

int main() {
    const std::string content = read_content("input.txt");
    const auto ranges = parse_file(content);
    unsigned long long answer_p1 = 0;
    unsigned long long answer_p2 = 0;

    for (const auto& r : ranges) {
        auto [x, y]  = loop_range_elements(r);
        answer_p1 += x;
        answer_p2 += y;
    }

    std::cout << "answer_p1: " << answer_p1 << "\n";
    std::cout << "answer_p2: " << answer_p2 << "\n";

    return 0;
}