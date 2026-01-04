#include <iostream>
#include <stack>
#include <string>

bool isBalanced(const std::string& expr) {
    std::stack<char> stk;
    for (char ch : expr) {
        if (ch == '(') {
            stk.push(ch);
        } else if (ch == ')') {
            if (stk.empty()) {
                return false;  // More closing than opening
            }
            stk.pop();
        }
        // Ignore other characters, as the focus is on ( and )
    }
    return stk.empty();  // True if all opened are closed
}

int main() {
    std::string input;
    std::cout << "Enter an expression: ";
    std::getline(std::cin, input);

    if (isBalanced(input)) {
        std::cout << "The parentheses are balanced." << std::endl;
    } else {
        std::cout << "The parentheses are not balanced." << std::endl;
    }

    // Example tests (for demonstration)
    std::cout << "Test '()': " << (isBalanced("()") ? "Balanced" : "Unbalanced") << std::endl;
    std::cout << "Test '(())': " << (isBalanced("(())") ? "Balanced" : "Unbalanced") << std::endl;
    std::cout << "Test '(()': " << (isBalanced("(()") ? "Balanced" : "Unbalanced") << std::endl;
    std::cout << "Test ')(': " << (isBalanced(")(") ? "Balanced" : "Unbalanced") << std::endl;
    std::cout << "Test 'a+(b-c)': " << (isBalanced("a+(b-c)") ? "Balanced" : "Unbalanced") << std::endl;

    return 0;
}