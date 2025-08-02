#include <iostream>
#include "includes/helper.hpp"

using namespace std;

int main() {
    string expression;
    cout << "Enter your regular expression (regex) : ";
    cin >> expression;

    cout << "\nIf you only need NFA enter 1, if you need only DFA enter 2, if you need only reduced DFA enter 3, if you want all enter 4" << endl;
    int input;

    auto takeInput = [&]() {
        cout << "\nEnter your number: ";
        cin >> input;
    };

    takeInput();

    while(input < 1 || input > 4) {
        cout << "Oops, you entered a wrong value. Please enter a correct one." << endl;
        takeInput();
    }

    helper(expression, input);

    return 0;
}