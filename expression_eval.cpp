#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <map>
#include <sstream>
#include <cctype>
#include <cstdlib>

// ─────────────────────────────────────────
//  TOKEN
// ─────────────────────────────────────────
enum TokenType {
    TOK_NUMBER,
    TOK_VARIABLE,
    TOK_OP,
    TOK_LPAREN,   // ( [ {
    TOK_RPAREN    // ) ] }
};

struct Token {
    TokenType type;
    std::string value;
};

// ─────────────────────────────────────────
//  TOKENIZER  (no spaces guaranteed)
// ─────────────────────────────────────────
std::vector<Token> tokenize(const std::string& expr) {
    std::vector<Token> tokens;
    int i = 0;
    int n = (int)expr.size();

    while (i < n) {
        char ch = expr[i];

        // skip whitespace
        if (std::isspace(ch)) { i++; continue; }

        // multi-digit / multi-char number
        if (std::isdigit(ch)) {
            std::string num;
            while (i < n && std::isdigit(expr[i]))
                num += expr[i++];
            tokens.push_back({TOK_NUMBER, num});
            continue;
        }

        // variable / keyword (C++ identifier: letter or _, then letter/digit/_)
        if (std::isalpha(ch) || ch == '_') {
            std::string var;
            while (i < n && (std::isalnum(expr[i]) || expr[i] == '_'))
                var += expr[i++];
            tokens.push_back({TOK_VARIABLE, var});
            continue;
        }

        // operators
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            tokens.push_back({TOK_OP, std::string(1, ch)});
            i++; continue;
        }

        // open grouping
        if (ch == '(' || ch == '[' || ch == '{') {
            tokens.push_back({TOK_LPAREN, std::string(1, ch)});
            i++; continue;
        }

        // close grouping
        if (ch == ')' || ch == ']' || ch == '}') {
            tokens.push_back({TOK_RPAREN, std::string(1, ch)});
            i++; continue;
        }

        // unknown character → syntax error
        std::cerr << "Syntax error: unexpected character '" << ch << "'\n";
        exit(1);
    }
    return tokens;
}

// ─────────────────────────────────────────
//  HELPERS
// ─────────────────────────────────────────
int precedence(const std::string& op) {
    if (op == "*" || op == "/") return 2;
    if (op == "+" || op == "-") return 1;
    return 0;
}

// returns matching open bracket for a close bracket
char matchingOpen(char close) {
    if (close == ')') return '(';
    if (close == ']') return '[';
    if (close == '}') return '{';
    return 0;
}

// ─────────────────────────────────────────
//  INFIX → POSTFIX  (Shunting-Yard)
// ─────────────────────────────────────────
std::vector<Token> toPostfix(const std::vector<Token>& tokens) {
    std::vector<Token> output;
    std::stack<Token> ops;

    for (const Token& tok : tokens) {
        switch (tok.type) {

        case TOK_NUMBER:
        case TOK_VARIABLE:
            output.push_back(tok);
            break;

        case TOK_OP:
            // pop operators of >= precedence (left-associative)
            while (!ops.empty() &&
                   ops.top().type == TOK_OP &&
                   precedence(ops.top().value) >= precedence(tok.value)) {
                output.push_back(ops.top());
                ops.pop();
            }
            ops.push(tok);
            break;

        case TOK_LPAREN:
            ops.push(tok);
            break;

        case TOK_RPAREN: {
            char needed = matchingOpen(tok.value[0]);
            bool found = false;
            while (!ops.empty()) {
                Token top = ops.top(); ops.pop();
                if (top.type == TOK_LPAREN) {
                    if (top.value[0] != needed) {
                        std::cerr << "Syntax error: mismatched brackets '"
                                  << top.value << "' and '" << tok.value << "'\n";
                        exit(1);
                    }
                    found = true;
                    break;
                }
                output.push_back(top);
            }
            if (!found) {
                std::cerr << "Syntax error: unmatched closing bracket '"
                          << tok.value << "'\n";
                exit(1);
            }
            break;
        }
        }
    }

    // drain remaining operators
    while (!ops.empty()) {
        Token top = ops.top(); ops.pop();
        if (top.type == TOK_LPAREN || top.type == TOK_RPAREN) {
            std::cerr << "Syntax error: unmatched opening bracket '"
                      << top.value << "'\n";
            exit(1);
        }
        output.push_back(top);
    }

    return output;
}

// ─────────────────────────────────────────
//  COLLECT UNIQUE VARIABLES
// ─────────────────────────────────────────
std::vector<std::string> collectVars(const std::vector<Token>& tokens) {
    std::vector<std::string> vars;
    std::map<std::string, bool> seen;
    for (const Token& t : tokens) {
        if (t.type == TOK_VARIABLE && !seen[t.value]) {
            vars.push_back(t.value);
            seen[t.value] = true;
        }
    }
    return vars;
}

// ─────────────────────────────────────────
//  POSTFIX EVALUATION
// ─────────────────────────────────────────
double evaluate(const std::vector<Token>& postfix,
                const std::map<std::string, double>& varValues) {
    std::stack<double> stk;

    for (const Token& tok : postfix) {
        if (tok.type == TOK_NUMBER) {
            stk.push(std::stod(tok.value));

        } else if (tok.type == TOK_VARIABLE) {
            auto it = varValues.find(tok.value);
            if (it == varValues.end()) {
                std::cerr << "Runtime error: no value for variable '"
                          << tok.value << "'\n";
                exit(2);
            }
            stk.push(it->second);

        } else if (tok.type == TOK_OP) {
            if (stk.size() < 2) {
                std::cerr << "Syntax error: not enough operands for '"
                          << tok.value << "'\n";
                exit(1);
            }
            double b = stk.top(); stk.pop();
            double a = stk.top(); stk.pop();

            if (tok.value == "+") stk.push(a + b);
            else if (tok.value == "-") stk.push(a - b);
            else if (tok.value == "*") stk.push(a * b);
            else if (tok.value == "/") {
                if (b == 0) {
                    std::cerr << "Runtime error: division by zero\n";
                    exit(2);
                }
                stk.push(a / b);
            }
        }
    }

    if (stk.size() != 1) {
        std::cerr << "Logical error: expression did not reduce to a single value\n";
        exit(3);
    }

    return stk.top();
}

// ─────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────
int main() {
    // read full line
    std::string expr;
    std::getline(std::cin, expr);

    if (expr.empty()) {
        std::cerr << "Syntax error: empty expression\n";
        return 1;
    }

    // 1. Tokenize
    std::vector<Token> tokens = tokenize(expr);

    if (tokens.empty()) {
        std::cerr << "Syntax error: no valid tokens found\n";
        return 1;
    }

    // basic structural check: expression shouldn't start/end with an operator
    if (tokens.front().type == TOK_OP || tokens.back().type == TOK_OP) {
        std::cerr << "Syntax error: expression starts or ends with an operator\n";
        return 1;
    }

    // 2. Convert to postfix
    std::vector<Token> postfix = toPostfix(tokens);

    // 3. Print postfix to stdout
    for (int i = 0; i < (int)postfix.size(); i++) {
        if (i) std::cout << " ";
        std::cout << postfix[i].value;
    }
    std::cout << "\n";

    // 4. Collect variables and prompt user (via stderr)
    std::vector<std::string> vars = collectVars(tokens);
    std::map<std::string, double> varValues;

    for (const std::string& var : vars) {
        std::cerr << "Enter value for " << var << ": ";
        double val;
        if (!(std::cin >> val)) {
            std::cerr << "Runtime error: invalid input for variable '" << var << "'\n";
            return 2;
        }
        varValues[var] = val;
    }

    // 5. Evaluate and print result to stdout
    double result = evaluate(postfix, varValues);

    // print as integer if it's a whole number, else as decimal
    if (result == (long long)result)
        std::cout << (long long)result << "\n";
    else
        std::cout << result << "\n";

    return 0;
}
