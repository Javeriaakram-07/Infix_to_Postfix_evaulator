# Expression Conversion and Evaluation

A C++ program that takes an infix arithmetic expression, converts it to postfix (Reverse Polish Notation), and evaluates it.

## Features

- **Robust tokenizer** — handles expressions with or without spaces (`a+b*(c+2)` and `a + b * (c + 2)` both work)
- **Multi-character identifiers** — variable names like `my_var`, `x2`, `total_sum` are fully supported
- **Mixed bracket types** — `()`, `[]`, `{}` all handled; mismatches are caught as errors
- **Proper operator precedence** — `*` and `/` bind tighter than `+` and `-`
- **Exit codes** for automated testing

## Compilation

```bash
g++ -std=c++17 -o expression_eval expression_eval.cpp
```

## Usage

```bash
./expression_eval
```

The program reads one line from stdin (the infix expression), then prompts for variable values on stderr.

### Output

- **stdout**: postfix form, then the evaluated result
- **stderr**: all prompts and error messages

## Example

```
Input:    a + b * (c + 2)
Enter value for a: 3
Enter value for b: 5
Enter value for c: 2

Postfix:  a b c 2 + * +
Result:   23
```

## Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Syntax error (bad token, mismatched brackets, malformed expression) |
| 2 | Runtime error (division by zero, missing variable value) |
| 3 | Logical error (expression doesn't reduce to a single value) |

## Supported Syntax

- **Operators**: `+`, `-`, `*`, `/`
- **Brackets**: `()`, `[]`, `{}`
- **Numbers**: integer constants (e.g. `42`, `0`, `100`)
- **Variables**: C++ identifier rules — start with letter or `_`, followed by letters, digits, or `_`

## Error Cases Handled

- Unknown/invalid characters → exit 1
- Mismatched brackets e.g. `(a + b]` → exit 1
- Unmatched brackets e.g. `a + (b` → exit 1
- Expression starting or ending with an operator → exit 1
- Division by zero → exit 2
- Too few operands for an operator → exit 1
