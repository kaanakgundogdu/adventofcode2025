use std::cmp::{max, min};
use std::env;
use std::fs;
use std::path::Path;
use std::time::Instant;

#[derive(Debug, Clone, Copy)]
struct Point {
    col: i64,
    row: i64,
}

fn read_lines(path: &str) -> Result<String, std::io::Error> {
    let file_path = Path::new(path);

    match fs::read_to_string(file_path) {
        Ok(content) => {
            if content.is_empty() {
                return Err(std::io::Error::new(
                    std::io::ErrorKind::InvalidData,
                    "File empty",
                ));
            }
            Ok(content)
        }
        Err(e) => Err(e),
    }
}

fn convert_input_to_point(data: &str) -> Result<Vec<Point>, String> {
    let mut points = Vec::new();

    for (line_num, line) in data.lines().enumerate() {
        if line.trim().is_empty() {
            continue;
        }

        let parts: Vec<&str> = line.split(',').collect();
        if parts.len() != 2 {
            return Err(format!(
                "Line {}: Invalid format, expected 'x,y'",
                line_num + 1
            ));
        }

        let x = match parts[0].trim().parse::<i64>() {
            Ok(val) => val,
            Err(_) => {
                return Err(format!(
                    "Line {}: Invalid X value '{}'",
                    line_num + 1,
                    parts[0]
                ));
            }
        };

        let y = match parts[1].trim().parse::<i64>() {
            Ok(val) => val,
            Err(_) => {
                return Err(format!(
                    "Line {}: Invalid Y value '{}'",
                    line_num + 1,
                    parts[1]
                ));
            }
        };

        points.push(Point { col: y, row: x });
    }

    if points.is_empty() {
        return Err("Error reading file content: No valid points found".to_string());
    }

    Ok(points)
}

fn part_one_sol(points: &[Point]) -> u64 {
    let mut max_area: u64 = 0;

    for (i, p1) in points.iter().enumerate() {
        for p2 in points.iter().skip(i + 1) {
            let width = p1.col.abs_diff(p2.col) + 1;
            let height = p1.row.abs_diff(p2.row) + 1;

            let area = width * height;

            if area > max_area {
                max_area = area;
            }
        }
    }

    max_area
}

fn edge_intersects_rect_interior(
    p1: Point,
    p2: Point,
    min_c: i64,
    max_c: i64,
    min_r: i64,
    max_r: i64,
) -> bool {
    if p1.col == p2.col {
        if p1.col > min_c && p1.col < max_c {
            let edge_r_min = min(p1.row, p2.row);
            let edge_r_max = max(p1.row, p2.row);
            if max(edge_r_min, min_r) < min(edge_r_max, max_r) {
                return true;
            }
        }
    } else if p1.row == p2.row {
        if p1.row > min_r && p1.row < max_r {
            let edge_c_min = min(p1.col, p2.col);
            let edge_c_max = max(p1.col, p2.col);
            if max(edge_c_min, min_c) < min(edge_c_max, max_c) {
                return true;
            }
        }
    }
    false
}

fn is_inside_polygon(poly: &[Point], x: f64, y: f64) -> bool {
    let mut inside = false;
    let n = poly.len();
    let mut j = n - 1;

    for i in 0..n {
        let xi = poly[i].col as f64;
        let yi = poly[i].row as f64;
        let xj = poly[j].col as f64;
        let yj = poly[j].row as f64;

        let min_x = xi.min(xj);
        let max_x = xi.max(xj);
        let min_y = yi.min(yj);
        let max_y = yi.max(yj);

        if x >= min_x && x <= max_x && y >= min_y && y <= max_y {
            if (xi == xj && (x - xi).abs() < 0.0001) || (yi == yj && (y - yi).abs() < 0.0001) {
                return true;
            }
        }

        let intersect = ((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi);

        if intersect {
            inside = !inside;
        }
        j = i;
    }
    inside
}

fn part_two_sol(points: &[Point]) -> u64 {
    let n = points.len();
    if n < 2 {
        return 0;
    }

    let mut max_area: u64 = 0;

    for i in 0..n {
        for j in (i + 1)..n {
            let p1 = points[i];
            let p2 = points[j];

            let width = p1.col.abs_diff(p2.col) + 1;
            let height = p1.row.abs_diff(p2.row) + 1;
            let area = width * height;

            if area <= max_area {
                continue;
            }

            let min_c = min(p1.col, p2.col);
            let max_c = max(p1.col, p2.col);
            let min_r = min(p1.row, p2.row);
            let max_r = max(p1.row, p2.row);

            let mut edge_cuts = false;
            for k in 0..n {
                let pk1 = points[k];
                let pk2 = points[(k + 1) % n];
                if edge_intersects_rect_interior(pk1, pk2, min_c, max_c, min_r, max_r) {
                    edge_cuts = true;
                    break;
                }
            }
            if edge_cuts {
                continue;
            }

            let center_c = (min_c + max_c) as f64 / 2.0;
            let center_r = (min_r + max_r) as f64 / 2.0;

            if is_inside_polygon(points, center_c, center_r) {
                max_area = area;
            }
        }
    }

    max_area
}

fn main() {
    let start = Instant::now();

    let args: Vec<String> = env::args().collect();
    let filename = if args.len() > 1 {
        &args[1]
    } else {
        "input.txt"
    };

    let file_content = match read_lines(filename) {
        Ok(c) => c,
        Err(e) => {
            eprintln!("Error reading file '{}': {}", filename, e);
            std::process::exit(1);
        }
    };

    let points = match convert_input_to_point(&file_content) {
        Ok(p) => p,
        Err(e) => {
            eprintln!("Error parsing data: {}", e);
            std::process::exit(1);
        }
    };

    let result = part_one_sol(&points);
    println!("part 1 answer: {}", result);

    let result = part_two_sol(&points);
    println!("part 2 answer: {}", result);

    let duration = start.elapsed();
    println!("Total Time: {} µs", duration.as_micros());
}
