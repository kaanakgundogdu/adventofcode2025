
#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "utils.h"

struct Container {
    int rows;
    int cols;
    std::vector<int> shape_quantities;
};

struct Shape {
    int id;
    int height;
    int width;
    int area;
    std::vector<std::pair<int, int>> offsets;
};

auto normalize_grids(const std::map<int, std::vector<std::pair<int, int>>>& raw_grids) {
    std::vector<Shape> shapes;
    int max_id = -1;
    for (const auto& [id, _] : raw_grids) max_id = std::max(max_id, id);
    shapes.resize(max_id + 1);

    for (const auto& [id, coords] : raw_grids) {
        if (coords.empty()) continue;

        int min_r = 10000, min_c = 10000;
        int max_r = -1, max_c = -1;

        for (const auto& p : coords) {
            if (p.first < min_r) min_r = p.first;
            if (p.second < min_c) min_c = p.second;
            if (p.first > max_r) max_r = p.first;
            if (p.second > max_c) max_c = p.second;
        }

        Shape s;
        s.id = id;
        s.height = max_r - min_r + 1;
        s.width = max_c - min_c + 1;
        s.area = (int)coords.size();

        for (const auto& p : coords) {
            s.offsets.emplace_back(p.first - min_r, p.second - min_c);
        }
        shapes[id] = s;
    }
    return shapes;
}

auto get_grids(const std::vector<std::string>& data) {
    std::map<int, std::vector<std::pair<int, int>>> grids;
    int current_id = -1;
    int row = 0;

    for (const auto& line : data) {
        if (line.empty()) continue;
        if (line.find('x') != std::string::npos && line.find(':') != std::string::npos) break;

        if (line.back() == ':') {
            current_id = std::stoi(line.substr(0, line.size() - 1));
            row = 0;
        } else if (current_id != -1) {
            for (int col = 0; col < (int)line.size(); ++col) {
                if (line[col] == '#') {
                    grids[current_id].emplace_back(row, col);
                }
            }
            row++;
        }
    }
    return grids;
}

auto get_containers(const std::vector<std::string>& data) {
    std::vector<Container> containers;
    bool parsing_grids = true;

    for (const auto& line : data) {
        if (parsing_grids) {
            if (line.find('x') != std::string::npos && line.find(':') != std::string::npos) {
                parsing_grids = false;
            } else {
                continue;
            }
        }

        size_t x_pos = line.find('x');
        size_t colon_pos = line.find(':');

        if (x_pos != std::string::npos && colon_pos != std::string::npos) {
            Container c;
            c.cols = std::stoi(line.substr(0, x_pos));
            c.rows = std::stoi(line.substr(x_pos + 1, colon_pos - x_pos - 1));

            std::stringstream ss(line.substr(colon_pos + 1));
            int val;
            while (ss >> val) {
                c.shape_quantities.push_back(val);
            }
            containers.push_back(c);
        }
    }
    return containers;
}

bool can_place(const std::vector<bool>& board, int board_w, int r, int c, const Shape& shape) {
    if (r + shape.height > (int)(board.size() / board_w) || c + shape.width > board_w) return false;

    for (const auto& offset : shape.offsets) {
        int idx = (r + offset.first) * board_w + (c + offset.second);
        if (board[idx]) return false;
    }
    return true;
}

void toggle_shape(std::vector<bool>& board, int board_w, int r, int c, const Shape& shape,
                  bool state) {
    for (const auto& offset : shape.offsets) {
        int idx = (r + offset.first) * board_w + (c + offset.second);
        board[idx] = state;
    }
}

bool solve(std::vector<bool>& board, int board_w, int board_h, const std::vector<int>& piece_ids,
           size_t index, const std::vector<Shape>& shapes) {
    if (index == piece_ids.size()) return true;

    int current_id = piece_ids[index];
    const Shape& shape = shapes[current_id];

    for (int r = 0; r <= board_h - shape.height; ++r) {
        for (int c = 0; c <= board_w - shape.width; ++c) {
            if (can_place(board, board_w, r, c, shape)) {
                toggle_shape(board, board_w, r, c, shape, true);

                if (solve(board, board_w, board_h, piece_ids, index + 1, shapes)) return true;

                toggle_shape(board, board_w, r, c, shape, false);
            }
        }
    }
    return false;
}

auto part_one_sol(const std::vector<std::string>& data) {
    unsigned long long answer = 0;

    auto raw_grids = get_grids(data);
    auto containers = get_containers(data);
    auto shapes = normalize_grids(raw_grids);

    for (size_t i = 0; i < containers.size(); ++i) {
        const auto& cont = containers[i];

        std::vector<int> pieces;
        for (size_t sid = 0; sid < cont.shape_quantities.size(); ++sid) {
            int qty = cont.shape_quantities[sid];
            if (sid < shapes.size() && shapes[sid].area > 0) {
                for (int k = 0; k < qty; ++k) pieces.push_back((int)sid);
            }
        }

        long long total_area = 0;
        for (int pid : pieces) total_area += shapes[pid].area;
        long long container_area = (long long)cont.rows * cont.cols;

        if (total_area > container_area) {
            continue;
        }

        std::sort(pieces.begin(), pieces.end(),
                  [&](int a, int b) { return shapes[a].area > shapes[b].area; });

        std::vector<bool> board(cont.rows * cont.cols, false);

        if (solve(board, cont.cols, cont.rows, pieces, 0, shapes)) {
            answer++;
        }
    }

    return answer;
}

int main(int argc, char* argv[]) {
    try {
        const auto start = std::chrono::high_resolution_clock::now();
        std::string filename = (argc > 1) ? argv[1] : "input.txt";
        const auto& data = aoc::read_lines(filename, "day_10");

        std::println("Part 1 Answer: {}", part_one_sol(data));

        const auto end = std::chrono::high_resolution_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        std::println("Total Time: {} µs", duration.count());

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}