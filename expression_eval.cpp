#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include <map>
#include <cctype>
#include <cstdlib>

using namespace std;

// each token is a pair: ("NUM"/"VAR"/"OP"/"LPAR"/"RPAR", actual value)
typedef pair<string, string> Token;

// tokenize char by char so spaces dont matter
vector<Token> tokenize(const string &expr)
{
    vector<Token> tokens;
    int i = 0, n = expr.size();

    while (i < n)
    {
        char ch = expr[i];

        if (isspace(ch))
        {
            i++;
            continue;
        }

        if (isdigit(ch))
        {
            string num;
            while (i < n && isdigit(expr[i]))
                num += expr[i++];
            tokens.push_back({"NUM", num});
            continue;
        }

        if (isalpha(ch) || ch == '_')
        {
            string var;
            while (i < n && (isalnum(expr[i]) || expr[i] == '_'))
                var += expr[i++];
            tokens.push_back({"VAR", var});
            continue;
        }

        if (ch == '+' || ch == '-' || ch == '*' || ch == '/')
        {
            tokens.push_back({"OP", string(1, ch)});
            i++;
            continue;
        }

        if (ch == '(' || ch == '[' || ch == '{')
        {
            tokens.push_back({"LPAR", string(1, ch)});
            i++;
            continue;
        }

        if (ch == ')' || ch == ']' || ch == '}')
        {
            tokens.push_back({"RPAR", string(1, ch)});
            i++;
            continue;
        }

        cerr << "Syntax error: unexpected character '" << ch << "'\n";
        exit(1);
    }
    return tokens;
}

int precedence(const string &op)
{
    if (op == "*" || op == "/")
        return 2;
    if (op == "+" || op == "-")
        return 1;
    return 0;
}

char matchingOpen(char close)
{
    if (close == ')')
        return '(';
    if (close == ']')
        return '[';
    return '{';
}

// catch bad token sequences before conversion: consecutive ops, op next to bracket, empty brackets
void validateTokens(const vector<Token> &tokens)
{
    for (int i = 0; i + 1 < (int)tokens.size(); i++)
    {
        string cur = tokens[i].first;
        string next = tokens[i + 1].first;

        if (cur == "OP" && next == "OP")
        {
            cerr << "Syntax error: consecutive operators '" << tokens[i].second << tokens[i + 1].second << "'\n";
            exit(1);
        }

        if (cur == "LPAR" && next == "RPAR")
        {
            cerr << "Syntax error: empty brackets\n";
            exit(1);
        }

        if (cur == "LPAR" && next == "OP")
        {
            cerr << "Syntax error: operator after opening bracket\n";
            exit(1);
        }

        if (cur == "OP" && next == "RPAR")
        {
            cerr << "Syntax error: operator before closing bracket\n";
            exit(1);
        }

        if ((cur == "NUM" || cur == "VAR") && (next == "NUM" || next == "VAR"))
        {
            cerr << "Syntax error: missing operator between operands\n";
            exit(1);
        }
    }
}

// shunting-yard: infix tokens -> postfix tokens
vector<Token> toPostfix(const vector<Token> &tokens)
{
    vector<Token> output;
    stack<Token> ops;

    for (const Token &tok : tokens)
    {
        if (tok.first == "NUM" || tok.first == "VAR")
        {
            output.push_back(tok);
        }
        else if (tok.first == "OP")
        {
            while (!ops.empty() && ops.top().first == "OP" &&
                   precedence(ops.top().second) >= precedence(tok.second))
            {
                output.push_back(ops.top());
                ops.pop();
            }
            ops.push(tok);
        }
        else if (tok.first == "LPAR")
        {
            ops.push(tok);
        }
        else if (tok.first == "RPAR")
        {
            char needed = matchingOpen(tok.second[0]);
            bool found = false;
            while (!ops.empty())
            {
                Token top = ops.top();
                ops.pop();
                if (top.first == "LPAR")
                {
                    if (top.second[0] != needed)
                    {
                        cerr << "Syntax error: mismatched brackets\n";
                        exit(1);
                    }
                    found = true;
                    break;
                }
                output.push_back(top);
            }
            if (!found)
            {
                cerr << "Syntax error: unmatched closing bracket\n";
                exit(1);
            }
        }
    }

    while (!ops.empty())
    {
        if (ops.top().first == "LPAR")
        {
            cerr << "Syntax error: unmatched opening bracket\n";
            exit(1);
        }
        output.push_back(ops.top());
        ops.pop();
    }

    return output;
}

vector<string> collectVars(const vector<Token> &tokens)
{
    vector<string> vars;
    map<string, bool> seen;
    for (const Token &t : tokens)
    {
        if (t.first == "VAR" && !seen[t.second])
        {
            vars.push_back(t.second);
            seen[t.second] = true;
        }
    }
    return vars;
}

// stack-based postfix evaluation
double evaluate(const vector<Token> &postfix, const map<string, double> &varValues)
{
    stack<double> stk;

    for (const Token &tok : postfix)
    {
        if (tok.first == "NUM")
        {
            stk.push(stod(tok.second));
        }
        else if (tok.first == "VAR")
        {
            auto it = varValues.find(tok.second);
            if (it == varValues.end())
            {
                cerr << "Runtime error: no value for '" << tok.second << "'\n";
                exit(2);
            }
            stk.push(it->second);
        }
        else if (tok.first == "OP")
        {
            if (stk.size() < 2)
            {
                cerr << "Syntax error: not enough operands\n";
                exit(1);
            }
            double b = stk.top();
            stk.pop();
            double a = stk.top();
            stk.pop();
            if (tok.second == "+")
                stk.push(a + b);
            else if (tok.second == "-")
                stk.push(a - b);
            else if (tok.second == "*")
                stk.push(a * b);
            else if (tok.second == "/")
            {
                if (b == 0)
                {
                    cerr << "Runtime error: division by zero\n";
                    exit(2);
                }
                stk.push(a / b);
            }
        }
    }

    if (stk.size() != 1)
    {
        cerr << "Logical error: invalid expression\n";
        exit(3);
    }
    return stk.top();
}

int main()
{
    string expr;
    getline(cin, expr);

    if (expr.empty())
    {
        cerr << "Syntax error: empty expression\n";
        return 1;
    }

    vector<Token> tokens = tokenize(expr);

    if (tokens.empty())
    {
        cerr << "Syntax error: no tokens\n";
        return 1;
    }
    if (tokens.front().first == "OP" || tokens.back().first == "OP")
    {
        cerr << "Syntax error: expression starts or ends with operator\n";
        return 1;
    }

    validateTokens(tokens);

    vector<Token> postfix = toPostfix(tokens);

    // print postfix
    for (int i = 0; i < (int)postfix.size(); i++)
    {
        if (i)
            cout << " ";
        cout << postfix[i].second;
    }
    cout << "\n";

    // prompt user for variable values
    vector<string> vars = collectVars(tokens);
    map<string, double> varValues;
    for (const string &var : vars)
    {
        cerr << "Enter value for " << var << ": ";
        double val;
        if (!(cin >> val))
        {
            cerr << "Runtime error: invalid input\n";
            return 2;
        }
        varValues[var] = val;
    }

    double result = evaluate(postfix, varValues);

    // print result (as int if whole number)
    if (result == (long long)result)
        cout << (long long)result << "\n";
    else
        cout << result << "\n";

    return 0;
}