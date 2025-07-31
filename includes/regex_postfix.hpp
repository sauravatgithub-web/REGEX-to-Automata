# pragma once

#include <iostream>
#include <string>
#include <stack>

std::string addConcatOperator(std::string regex) {
    std::string result;
    char prev = '\0';

    for(char ch : regex) {
        if(prev) {
            if((isalnum(prev) || prev == ')' || prev == '*') && (isalnum(ch) || ch == '(')) {
                result += '.';
            }
        }
        result += ch;
        prev = ch;
    }

    return result;
}

int precedence(char op) {
    switch(op) {
        case '*' : return 3;
        case '.' : return 2;
        case '|' : return 1;
        default  : return 0;
    }
}

std::string toPostfix(std::string regex) {
    std::string expr = addConcatOperator(regex);
    std::string output = "";
    std::stack<char> operators;

    for(char ch : expr) {
        if(isalnum(ch)) output.push_back(ch);
        else if(ch == '(') operators.push(ch);
        else if(ch == ')') {
            while(!operators.empty() && operators.top() != '(') {
                output.push_back(operators.top());
                operators.pop();
            }
            if(!operators.empty()) operators.pop();
        }
        else {
            while(!operators.empty() && operators.top() != '(' && precedence(operators.top()) >= precedence(ch)) {
                output.push_back(operators.top());
                operators.pop();
            }
            operators.push(ch);
        }
    }

    while(!operators.empty()) {
        output.push_back(operators.top());
        operators.pop();
    }

    return output;
}