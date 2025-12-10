#include <charconv>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "utils.h"

struct Point {
    long long col;
    long long row;
};

auto convert_input_to_point(const std::vector<std::string>& data) {
    auto parsed_view = data | std::views::transform([](std::string_view line) -> Point {
                           int x = 0, y = 0;
                           const char* end = line.data() + line.size();

                           auto [ptr, ec] = std::from_chars(line.data(), end, x);

                           if (ec == std::errc() && ptr != end && *ptr == ',') {
                               std::from_chars(ptr + 1, end, y);
                           }

                           return {y, x};
                       });

    return std::vector<Point>(parsed_view.begin(), parsed_view.end());
}

auto part_one_sol(const std::vector<std::string>& data) {
    auto positions = convert_input_to_point(data);

    unsigned long long max_area = 0;

    for (size_t i = 0; i < positions.size(); ++i) {
        for (size_t j = i + 1; j < positions.size(); ++j) {
            unsigned long long width = std::abs(positions[i].col - positions[j].col) + 1;
            unsigned long long height = std::abs(positions[i].row - positions[j].row) + 1;
            unsigned long long area = width * height;

            if (area > max_area) {
                max_area = area;
            }
        }
    }

    return max_area;
}

bool is_between(double v, long long a, long long b) {
    auto [min_v, max_v] = std::minmax(a, b);
    return v > min_v && v < max_v;
}

bool edge_intersects_rect_interior(const Point& p1, const Point& p2, long long min_x,
                                   long long max_x, long long min_y, long long max_y) {
    if (p1.col == p2.col) {
        if (p1.col > min_x && p1.col < max_x) {
            long long edge_y_min = std::min(p1.row, p2.row);
            long long edge_y_max = std::max(p1.row, p2.row);
            if (std::max(edge_y_min, min_y) < std::min(edge_y_max, max_y)) {
                return true;
            }
        }
    } else if (p1.row == p2.row) {
        if (p1.row > min_y && p1.row < max_y) {
            long long edge_x_min = std::min(p1.col, p2.col);
            long long edge_x_max = std::max(p1.col, p2.col);
            if (std::max(edge_x_min, min_x) < std::min(edge_x_max, max_x)) {
                return true;
            }
        }
    }
    return false;
}

bool is_inside_polygon(const std::vector<Point>& poly, double x, double y) {
    bool inside = false;
    size_t n = poly.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++) {
        double xi = poly[i].col, yi = poly[i].row;
        double xj = poly[j].col, yj = poly[j].row;

        bool on_segment_x = (x >= std::min(xi, xj) && x <= std::max(xi, xj));
        bool on_segment_y = (y >= std::min(yi, yj) && y <= std::max(yi, yj));
        if (on_segment_x && on_segment_y) {
            if ((xi == xj && std::abs(x - xi) < 0.0001) || (yi == yj && std::abs(y - yi) < 0.0001))
                return true;
        }

        bool intersect = ((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi);
        if (intersect) inside = !inside;
    }
    return inside;
}

auto part_two_sol(const std::vector<std::string>& data) {
    auto positions = convert_input_to_point(data);
    if (positions.empty()) {
        return 0ull;
    }

    unsigned long long max_area = 0;
    size_t n = positions.size();

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = i + 1; j < n; ++j) {
            long long width_val = std::abs(positions[i].col - positions[j].col);
            long long height_val = std::abs(positions[i].row - positions[j].row);
            unsigned long long area = (width_val + 1) * (height_val + 1);

            if (area <= max_area) {
                continue;
            }

            long long min_x = std::min(positions[i].col, positions[j].col);
            long long max_x = std::max(positions[i].col, positions[j].col);
            long long min_y = std::min(positions[i].row, positions[j].row);
            long long max_y = std::max(positions[i].row, positions[j].row);

            bool edge_cuts = false;
            for (size_t k = 0; k < n; ++k) {
                const auto& p1 = positions[k];
                const auto& p2 = positions[(k + 1) % n];

                if (edge_intersects_rect_interior(p1, p2, min_x, max_x, min_y, max_y)) {
                    edge_cuts = true;
                    break;
                }
            }
            if (edge_cuts) {
                continue;
            }

            double center_x = (min_x + max_x) / 2.0;
            double center_y = (min_y + max_y) / 2.0;

            if (is_inside_polygon(positions, center_x, center_y)) {
                max_area = area;
            }
        }
    }

    return max_area;
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();

        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = aoc::read_lines(filename, "day_9");

        std::println("part 1 answer: {} ", part_one_sol(data));
        std::println("part 2 answer: {} ", part_two_sol(data));

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}