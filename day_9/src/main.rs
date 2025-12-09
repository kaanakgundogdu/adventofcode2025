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

    let duration = start.elapsed();
    println!("Total Time: {} µs", duration.as_micros());
}
