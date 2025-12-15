// #include <algorithm>
// #include <chrono>
// #include <iomanip>
// #include <iostream>
// #include <map>
// #include <sstream>
// #include <string>
// #include <thread>
// #include <vector>

// // --- ANSI Colors ---
// const std::vector<std::string> COLORS = {"\033[41m",  "\033[42m",  "\033[43m",  "\033[44m",
//                                          "\033[45m",  "\033[46m",  "\033[101m", "\033[102m",
//                                          "\033[103m", "\033[104m", "\033[105m", "\033[106m"};
// const std::string RESET = "\033[0m";

// // --- Structs ---
// struct Container {
//     int rows;
//     int cols;
//     std::vector<int> shape_quantities;
// };

// struct Shape {
//     int id;
//     int height;
//     int width;
//     int area;
//     std::vector<std::pair<int, int>> offsets;
// };

// // --- Visualization ---
// void draw_board(const std::vector<int>& board, int width, int height, int placed_count,
//                 int total_pieces) {
//     std::cout << "\033[H";  // Move cursor to home

//     std::string buffer;
//     buffer.reserve(width * height * 10);

//     for (int r = 0; r < height; ++r) {
//         for (int c = 0; c < width; ++c) {
//             int val = board[r * width + c];
//             if (val == -1) {
//                 buffer += ".";
//             } else {
//                 buffer += COLORS[val % COLORS.size()] + "#" + RESET;
//             }
//         }
//         buffer += "\n";
//     }

//     // Status Bar
//     float percent = (float)placed_count / total_pieces * 100.0f;
//     buffer += "\nProgress: " + std::to_string(placed_count) + " / " +
//     std::to_string(total_pieces) +
//               " pieces placed.";

//     std::cout << buffer << std::flush;

//     // Speed: 1ms delay for smooth animation
//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
// }

// // --- Data Parsing ---
// auto normalize_grids(const std::map<int, std::vector<std::pair<int, int>>>& raw_grids) {
//     std::vector<Shape> shapes;
//     int max_id = -1;
//     for (const auto& [id, _] : raw_grids) max_id = std::max(max_id, id);
//     shapes.resize(max_id + 1);

//     for (const auto& [id, coords] : raw_grids) {
//         if (coords.empty()) continue;

//         int min_r = 10000, min_c = 10000;
//         int max_r = -1, max_c = -1;

//         for (const auto& p : coords) {
//             if (p.first < min_r) min_r = p.first;
//             if (p.second < min_c) min_c = p.second;
//             if (p.first > max_r) max_r = p.first;
//             if (p.second > max_c) max_c = p.second;
//         }

//         Shape s;
//         s.id = id;
//         s.height = max_r - min_r + 1;
//         s.width = max_c - min_c + 1;
//         s.area = (int)coords.size();

//         for (const auto& p : coords) {
//             s.offsets.emplace_back(p.first - min_r, p.second - min_c);
//         }
//         shapes[id] = s;
//     }
//     return shapes;
// }

// auto get_grids(const std::vector<std::string>& data) {
//     std::map<int, std::vector<std::pair<int, int>>> grids;
//     int current_id = -1;
//     int row = 0;

//     for (const auto& line : data) {
//         if (line.empty()) continue;
//         if (line.find('x') != std::string::npos && line.find(':') != std::string::npos) break;

//         if (line.back() == ':') {
//             current_id = std::stoi(line.substr(0, line.size() - 1));
//             row = 0;
//         } else if (current_id != -1) {
//             for (int col = 0; col < (int)line.size(); ++col) {
//                 if (line[col] == '#') {
//                     grids[current_id].emplace_back(row, col);
//                 }
//             }
//             row++;
//         }
//     }
//     return grids;
// }

// auto get_containers(const std::vector<std::string>& data) {
//     std::vector<Container> containers;
//     bool parsing_grids = true;

//     for (const auto& line : data) {
//         if (parsing_grids) {
//             if (line.find('x') != std::string::npos && line.find(':') != std::string::npos) {
//                 parsing_grids = false;
//             } else {
//                 continue;
//             }
//         }
//         size_t x_pos = line.find('x');
//         size_t colon_pos = line.find(':');
//         if (x_pos != std::string::npos && colon_pos != std::string::npos) {
//             Container c;
//             c.cols = std::stoi(line.substr(0, x_pos));
//             c.rows = std::stoi(line.substr(x_pos + 1, colon_pos - x_pos - 1));
//             std::stringstream ss(line.substr(colon_pos + 1));
//             int val;
//             while (ss >> val) c.shape_quantities.push_back(val);
//             containers.push_back(c);
//         }
//     }
//     return containers;
// }

// // --- Solver Logic ---

// bool can_place(const std::vector<int>& board, int board_w, int r, int c, const Shape& shape) {
//     if (r + shape.height > (int)(board.size() / board_w) || c + shape.width > board_w) return
//     false;

//     for (const auto& offset : shape.offsets) {
//         int idx = (r + offset.first) * board_w + (c + offset.second);
//         if (board[idx] != -1) return false;  // Collision
//     }
//     return true;
// }

// void place_shape(std::vector<int>& board, int board_w, int r, int c, const Shape& shape,
//                  int value) {
//     for (const auto& offset : shape.offsets) {
//         int idx = (r + offset.first) * board_w + (c + offset.second);
//         board[idx] = value;
//     }
// }

// // GREEDY SOLVER: Iterate pieces, find first valid spot, place it.
// void solve_greedy(std::vector<int>& board, int board_w, int board_h, std::vector<int>&
// all_pieces,
//                   const std::vector<Shape>& shapes) {
//     int pieces_placed = 0;

//     for (int piece_id : all_pieces) {
//         const Shape& shape = shapes[piece_id];
//         bool placed = false;

//         // Scan board for first valid spot
//         for (int r = 0; r <= board_h - shape.height; ++r) {
//             for (int c = 0; c <= board_w - shape.width; ++c) {
//                 if (can_place(board, board_w, r, c, shape)) {
//                     place_shape(board, board_w, r, c, shape, piece_id);
//                     placed = true;
//                     break;
//                 }
//             }
//             if (placed) break;
//         }

//         pieces_placed++;
//         // Animate regardless of whether it fit (to show progress)
//         // If it didn't fit, it just gets skipped.
//         if (placed) {
//             draw_board(board, board_w, board_h, pieces_placed, all_pieces.size());
//         }
//     }
// }

// int main() {
//     std::cout << "\033[2J";  // Clear screen

//     std::string input_str = R"(
// 0:
// #..
// ##.
// ###

// 1:
// ###
// .#.
// ###

// 2:
// ..#
// ###
// ###

// 3:
// #.#
// ###
// ##.

// 4:
// #..
// ##.
// .##

// 5:
// ###
// #.#
// #.#

// 49x41: 44 45 69 52 47 50
// )";

//     std::vector<std::string> data;
//     std::stringstream ss(input_str);
//     std::string line;
//     while (std::getline(ss, line)) data.push_back(line);

//     auto raw_grids = get_grids(data);
//     auto containers = get_containers(data);
//     auto shapes = normalize_grids(raw_grids);

//     for (const auto& cont : containers) {
//         // Flatten the quantities into a single list of piece IDs
//         std::vector<int> all_pieces;
//         for (size_t sid = 0; sid < cont.shape_quantities.size(); ++sid) {
//             int qty = cont.shape_quantities[sid];
//             if (sid < shapes.size()) {
//                 for (int i = 0; i < qty; ++i) all_pieces.push_back(sid);
//             }
//         }

//         // KEY OPTIMIZATION: Sort Largest Area -> Smallest Area
//         // This makes the greedy fill much more efficient and "full" looking
//         std::sort(all_pieces.begin(), all_pieces.end(),
//                   [&](int a, int b) { return shapes[a].area > shapes[b].area; });

//         std::vector<int> board(cont.rows * cont.cols, -1);

//         std::cout << "Starting Greedy Fill for " << cont.cols << "x" << cont.rows << "...\n";
//         solve_greedy(board, cont.cols, cont.rows, all_pieces, shapes);

//         std::cout << "\n\nFinished!\n";
//     }

//     return 0;
// }

// #include <algorithm>
// #include <chrono>
// #include <iomanip>
// #include <iostream>
// #include <map>
// #include <sstream>
// #include <string>
// #include <thread>
// #include <vector>

// // --- Extended ANSI Colors (256-color mode) ---
// // A mix of 56 distinct colors for maximum variety
// const std::vector<std::string> COLORS = {
//     // Standard Brights
//     "\033[38;5;196m", "\033[38;5;46m", "\033[38;5;226m", "\033[38;5;39m", "\033[38;5;201m",
//     "\033[38;5;51m",
//     // Oranges & Golds
//     "\033[38;5;208m", "\033[38;5;214m", "\033[38;5;220m", "\033[38;5;172m", "\033[38;5;130m",
//     "\033[38;5;202m",
//     // Greens & Teals
//     "\033[38;5;118m", "\033[38;5;154m", "\033[38;5;82m", "\033[38;5;35m", "\033[38;5;49m",
//     "\033[38;5;87m",
//     // Blues
//     "\033[38;5;27m", "\033[38;5;33m", "\033[38;5;63m", "\033[38;5;21m", "\033[38;5;117m",
//     "\033[38;5;159m",
//     // Purples & Pinks
//     "\033[38;5;129m", "\033[38;5;93m", "\033[38;5;171m", "\033[38;5;207m", "\033[38;5;213m",
//     "\033[38;5;219m", "\033[38;5;198m", "\033[38;5;162m", "\033[38;5;141m", "\033[38;5;105m",
//     // Reds & Browns
//     "\033[38;5;160m", "\033[38;5;124m", "\033[38;5;88m", "\033[38;5;166m", "\033[38;5;94m",
//     "\033[38;5;136m",
//     // Cyans & Turquoises
//     "\033[38;5;30m", "\033[38;5;37m", "\033[38;5;43m", "\033[38;5;80m", "\033[38;5;122m",
//     "\033[38;5;123m",
//     // Pastels/Lights
//     "\033[38;5;229m", "\033[38;5;192m", "\033[38;5;157m", "\033[38;5;158m", "\033[38;5;195m",
//     "\033[38;5;189m", "\033[38;5;225m", "\033[38;5;231m"};
// const std::string RESET = "\033[0m";

// // --- Structs ---
// struct Container {
//     int rows;
//     int cols;
//     std::vector<int> shape_quantities;
// };

// struct Shape {
//     int id;
//     int height;
//     int width;
//     int area;
//     std::vector<std::pair<int, int>> offsets;
// };

// // --- Visualization ---
// void draw_board(const std::vector<int>& board, int width, int height, int placed_count,
//                 int total_pieces) {
//     std::cout << "\033[H";  // Move cursor to home

//     std::string buffer;
//     buffer.reserve(width * height * 15);

//     for (int r = 0; r < height; ++r) {
//         for (int c = 0; c < width; ++c) {
//             int val = board[r * width + c];
//             if (val == -1) {
//                 buffer += ".";  // Just a dot for empty
//             } else {
//                 // Use the text color (foreground) on a # symbol
//                 buffer += COLORS[val % COLORS.size()] + "#" + RESET;
//             }
//         }
//         buffer += "\n";
//     }

//     // Status Bar
//     float percent = (float)placed_count / total_pieces * 100.0f;
//     buffer += "\nProgress: " + std::to_string(placed_count) + " / " +
//     std::to_string(total_pieces) +
//               " pieces placed.";

//     std::cout << buffer << std::flush;

//     // Speed: 1ms delay for smooth animation
//     std::this_thread::sleep_for(std::chrono::milliseconds(60));
// }

// // --- Data Parsing ---
// auto normalize_grids(const std::map<int, std::vector<std::pair<int, int>>>& raw_grids) {
//     std::vector<Shape> shapes;
//     int max_id = -1;
//     for (const auto& [id, _] : raw_grids) max_id = std::max(max_id, id);
//     shapes.resize(max_id + 1);

//     for (const auto& [id, coords] : raw_grids) {
//         if (coords.empty()) continue;

//         int min_r = 10000, min_c = 10000;
//         int max_r = -1, max_c = -1;

//         for (const auto& p : coords) {
//             if (p.first < min_r) min_r = p.first;
//             if (p.second < min_c) min_c = p.second;
//             if (p.first > max_r) max_r = p.first;
//             if (p.second > max_c) max_c = p.second;
//         }

//         Shape s;
//         s.id = id;
//         s.height = max_r - min_r + 1;
//         s.width = max_c - min_c + 1;
//         s.area = (int)coords.size();

//         for (const auto& p : coords) {
//             s.offsets.emplace_back(p.first - min_r, p.second - min_c);
//         }
//         shapes[id] = s;
//     }
//     return shapes;
// }

// auto get_grids(const std::vector<std::string>& data) {
//     std::map<int, std::vector<std::pair<int, int>>> grids;
//     int current_id = -1;
//     int row = 0;

//     for (const auto& line : data) {
//         if (line.empty()) continue;
//         if (line.find('x') != std::string::npos && line.find(':') != std::string::npos) break;

//         if (line.back() == ':') {
//             current_id = std::stoi(line.substr(0, line.size() - 1));
//             row = 0;
//         } else if (current_id != -1) {
//             for (int col = 0; col < (int)line.size(); ++col) {
//                 if (line[col] == '#') {
//                     grids[current_id].emplace_back(row, col);
//                 }
//             }
//             row++;
//         }
//     }
//     return grids;
// }

// auto get_containers(const std::vector<std::string>& data) {
//     std::vector<Container> containers;
//     bool parsing_grids = true;

//     for (const auto& line : data) {
//         if (parsing_grids) {
//             if (line.find('x') != std::string::npos && line.find(':') != std::string::npos) {
//                 parsing_grids = false;
//             } else {
//                 continue;
//             }
//         }
//         size_t x_pos = line.find('x');
//         size_t colon_pos = line.find(':');
//         if (x_pos != std::string::npos && colon_pos != std::string::npos) {
//             Container c;
//             c.cols = std::stoi(line.substr(0, x_pos));
//             c.rows = std::stoi(line.substr(x_pos + 1, colon_pos - x_pos - 1));
//             std::stringstream ss(line.substr(colon_pos + 1));
//             int val;
//             while (ss >> val) c.shape_quantities.push_back(val);
//             containers.push_back(c);
//         }
//     }
//     return containers;
// }

// // --- Solver Logic ---

// bool can_place(const std::vector<int>& board, int board_w, int r, int c, const Shape& shape) {
//     if (r + shape.height > (int)(board.size() / board_w) || c + shape.width > board_w) return
//     false;

//     for (const auto& offset : shape.offsets) {
//         int idx = (r + offset.first) * board_w + (c + offset.second);
//         if (board[idx] != -1) return false;  // Collision
//     }
//     return true;
// }

// void place_shape(std::vector<int>& board, int board_w, int r, int c, const Shape& shape,
//                  int value) {
//     for (const auto& offset : shape.offsets) {
//         int idx = (r + offset.first) * board_w + (c + offset.second);
//         board[idx] = value;
//     }
// }

// // GREEDY SOLVER: Iterate pieces, find first valid spot, place it.
// void solve_greedy(std::vector<int>& board, int board_w, int board_h, std::vector<int>&
// all_pieces,
//                   const std::vector<Shape>& shapes) {
//     int pieces_placed = 0;

//     for (int piece_id : all_pieces) {
//         const Shape& shape = shapes[piece_id];
//         bool placed = false;

//         // Scan board for first valid spot
//         for (int r = 0; r <= board_h - shape.height; ++r) {
//             for (int c = 0; c <= board_w - shape.width; ++c) {
//                 if (can_place(board, board_w, r, c, shape)) {
//                     place_shape(board, board_w, r, c, shape, piece_id);
//                     placed = true;
//                     break;
//                 }
//             }
//             if (placed) break;
//         }

//         pieces_placed++;
//         // Animate regardless of whether it fit (to show progress)
//         // If it didn't fit, it just gets skipped.
//         if (placed) {
//             draw_board(board, board_w, board_h, pieces_placed, all_pieces.size());
//         }
//     }
// }

// int main() {
//     std::cout << "\033[2J";  // Clear screen

//     std::string input_str = R"(
// 0:
// #..
// ##.
// ###

// 1:
// ###
// .#.
// ###

// 2:
// ..#
// ###
// ###

// 3:
// #.#
// ###
// ##.

// 4:
// #..
// ##.
// .##

// 5:
// ###
// #.#
// #.#

// 49x41: 44 45 69 52 47 50
// )";

//     std::vector<std::string> data;
//     std::stringstream ss(input_str);
//     std::string line;
//     while (std::getline(ss, line)) data.push_back(line);

//     auto raw_grids = get_grids(data);
//     auto containers = get_containers(data);
//     auto shapes = normalize_grids(raw_grids);

//     for (const auto& cont : containers) {
//         // Flatten the quantities into a single list of piece IDs
//         std::vector<int> all_pieces;
//         for (size_t sid = 0; sid < cont.shape_quantities.size(); ++sid) {
//             int qty = cont.shape_quantities[sid];
//             if (sid < shapes.size()) {
//                 for (int i = 0; i < qty; ++i) all_pieces.push_back(sid);
//             }
//         }

//         // KEY OPTIMIZATION: Sort Largest Area -> Smallest Area
//         std::sort(all_pieces.begin(), all_pieces.end(),
//                   [&](int a, int b) { return shapes[a].area > shapes[b].area; });

//         std::vector<int> board(cont.rows * cont.cols, -1);

//         std::cout << "Starting Greedy Fill for " << cont.cols << "x" << cont.rows << "...\n";
//         solve_greedy(board, cont.cols, cont.rows, all_pieces, shapes);

//         std::cout << "\n\nFinished!\n";
//     }

//     return 0;
// }
#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// --- Simple ANSI Colors ---
// Red, Blue, Yellow, Green
// const std::vector<std::string> COLORS = {
//     "\033[31m",  // Red
//     "\033[34m",  // Blue
//     "\033[33m",  // Yellow
//     "\033[32m"   // Green
// };
// const std::string RESET = "\033[0m";

// --- Structs ---
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

const std::string RESET = "\033[0m";

// 8 Distinct "Vivid" Foreground Colors
const std::vector<std::string> COLORS = {
    "\033[38;5;208m",  // Orange
    "\033[38;5;226m",  // Pure Yellow

    "\033[38;5;196m",  // Bright Red
    "\033[38;5;46m",   // Neon Green
    "\033[38;5;39m",   // Deep Sky Blue (much clearer than standard blue)
    "\033[38;5;201m",  // Hot Pink
    "\033[38;5;51m",   // Cyan
    "\033[38;5;93m",   // Purple
};

void draw_board(const std::vector<int>& board, int width, int height, int placed_count,
                int total_pieces) {
    std::cout << "\033[H";  // Move cursor to home

    std::string buffer;
    buffer.reserve(width * height * 15);

    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            int val = board[r * width + c];
            if (val == -1) {
                buffer += ".";  // Dot for empty
            } else {
                // Using the new vivid palette on the # character
                buffer += COLORS[val % COLORS.size()] + "#" + RESET;
            }
        }
        buffer += "\n";
    }

    // Status Bar
    buffer += "\nProgress: " + std::to_string(placed_count) + " / " + std::to_string(total_pieces) +
              " pieces placed.";

    std::cout << buffer << std::flush;

    // Speed: 60ms delay
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
}

// // --- Visualization ---
// void draw_board(const std::vector<int>& board, int width, int height, int placed_count,
//                 int total_pieces) {
//     std::cout << "\033[H";  // Move cursor to home

//     std::string buffer;
//     buffer.reserve(width * height * 10);

//     for (int r = 0; r < height; ++r) {
//         for (int c = 0; c < width; ++c) {
//             int val = board[r * width + c];
//             if (val == -1) {
//                 buffer += ".";  // Dot for empty
//             } else {
//                 // Cycle through the 4 colors deterministically
//                 buffer += COLORS[val % COLORS.size()] + "#" + RESET;
//             }
//         }
//         buffer += "\n";
//     }

//     // Status Bar
//     buffer += "\nProgress: " + std::to_string(placed_count) + " / " +
//     std::to_string(total_pieces) +
//               " pieces placed.";

//     std::cout << buffer << std::flush;

//     // Speed: 60ms delay
//     std::this_thread::sleep_for(std::chrono::milliseconds(60));
// }

// --- Data Parsing ---
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
            while (ss >> val) c.shape_quantities.push_back(val);
            containers.push_back(c);
        }
    }
    return containers;
}

// --- Solver Logic ---

bool can_place(const std::vector<int>& board, int board_w, int r, int c, const Shape& shape) {
    if (r + shape.height > (int)(board.size() / board_w) || c + shape.width > board_w) return false;

    for (const auto& offset : shape.offsets) {
        int idx = (r + offset.first) * board_w + (c + offset.second);
        if (board[idx] != -1) return false;  // Collision
    }
    return true;
}

void place_shape(std::vector<int>& board, int board_w, int r, int c, const Shape& shape,
                 int value) {
    for (const auto& offset : shape.offsets) {
        int idx = (r + offset.first) * board_w + (c + offset.second);
        board[idx] = value;
    }
}

void solve_greedy(std::vector<int>& board, int board_w, int board_h, std::vector<int>& all_pieces,
                  const std::vector<Shape>& shapes) {
    int pieces_placed = 0;

    for (int piece_id : all_pieces) {
        const Shape& shape = shapes[piece_id];
        bool placed = false;

        // Scan board for first valid spot
        for (int r = 0; r <= board_h - shape.height; ++r) {
            for (int c = 0; c <= board_w - shape.width; ++c) {
                if (can_place(board, board_w, r, c, shape)) {
                    place_shape(board, board_w, r, c, shape, piece_id);
                    placed = true;
                    break;
                }
            }
            if (placed) break;
        }

        pieces_placed++;
        if (placed) {
            draw_board(board, board_w, board_h, pieces_placed, all_pieces.size());
        }
    }
}

int main() {
    std::cout << "\033[2J";  // Clear screen

    // 3. Open File
    std::ifstream file("terminal_input.txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file terminal_input.txt" << std::endl;
        return 1;
    }

    std::vector<std::string> data;
    std::string line;
    while (std::getline(file, line)) {
        data.push_back(line);
    }
    file.close();

    auto raw_grids = get_grids(data);
    auto containers = get_containers(data);
    auto shapes = normalize_grids(raw_grids);

    for (const auto& cont : containers) {
        std::vector<int> all_pieces;
        for (size_t sid = 0; sid < cont.shape_quantities.size(); ++sid) {
            int qty = cont.shape_quantities[sid];
            if (sid < shapes.size()) {
                for (int i = 0; i < qty; ++i) all_pieces.push_back(sid);
            }
        }

        std::sort(all_pieces.begin(), all_pieces.end(),
                  [&](int a, int b) { return shapes[a].area > shapes[b].area; });

        std::vector<int> board(cont.rows * cont.cols, -1);

        std::cout << "Starting Greedy Fill for " << cont.cols << "x" << cont.rows << "...\n";
        solve_greedy(board, cont.cols, cont.rows, all_pieces, shapes);

        std::cout << "\n\nFinished!\n";
    }

    return 0;
}