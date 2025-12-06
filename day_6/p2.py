def solve_cephalopod_math(input_text):
    lines = input_text.split('\n')
    if lines and not lines[-1].strip():
        lines.pop()
    
    if not lines: return 0

    max_len = max(len(line) for line in lines)
    grid = [line.ljust(max_len) for line in lines]
    rows = len(grid)
    cols = max_len

    grand_total = 0
    current_problem_cols = []

    def process_block(col_indices):
        if not col_indices: return 0

        operator = None
        for c in col_indices:
            char = grid[-1][c]
            if char not in (' ', '\t'):
                operator = char
                break

        numbers = []
        for c in col_indices:
            num_str = ""
            for r in range(rows - 1): 
                if grid[r][c].isdigit():
                    num_str += grid[r][c]
            
            if num_str:
                numbers.append(int(num_str))
        
        if not numbers: return 0

        result = numbers[0]
        for i in range(1, len(numbers)):
            if operator == '+':
                result += numbers[i]
            elif operator == '*':
                result *= numbers[i]
            
        return result

    for x in range(cols - 1, -1, -1):
        is_separator = True
        for r in range(rows):
            if grid[r][x] != ' ':
                is_separator = False
                break
        
        if is_separator:
            if current_problem_cols:
                grand_total += process_block(current_problem_cols)
                current_problem_cols = []
        else:
            current_problem_cols.append(x)

    if current_problem_cols:
        grand_total += process_block(current_problem_cols)
        
    return grand_total



#TO USE : Replace 'example_input' with your actual puzzle input
with open("input.txt") as f:
    print(solve_cephalopod_math(f.read()))