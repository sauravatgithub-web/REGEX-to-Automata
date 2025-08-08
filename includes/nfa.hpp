# pragma once

#include <vector>
#include <string>
#include <stack>
#include <utility>
#include "fsm_elements.hpp"
#include "automata.hpp"
#include "definitions.hpp"

class NFA : public Automata {
public:
    State final_state;

    void create_nfa(std::string regex);
    void print();
};

void NFA::create_nfa(std::string regex) {
    int index = 0;
    std::stack<std::pair<State, State>> statePairs;

    for(char ch : regex) {
        if(isalnum(ch)) {
            State s1(++index), s2(++index);
            Transitions t(ch, s1, s2);
            states.push_back(s1);
            states.push_back(s2);
            alphabets.insert(ch);
            transitions.push_back(t);
            statePairs.push({s1, s2});
        }
        else {
            switch(ch) {
                case '.' : {
                    std::pair<State, State> p2 = statePairs.top();
                    statePairs.pop();
                    std::pair<State, State> p1 = statePairs.top();
                    statePairs.pop();
                    Transitions t(EPSILON, p1.second, p2.first);
                    transitions.push_back(t);
                    statePairs.push({p1.first, p2.second});
                    break;
                }
                case '|' : {
                    std::pair<State, State> p2 = statePairs.top();
                    statePairs.pop();
                    std::pair<State, State> p1 = statePairs.top();
                    statePairs.pop();
                    State s1(++index), s2(++index);
                    states.push_back(s1);
                    states.push_back(s2);
                    Transitions t1(EPSILON, s1, p1.first);
                    Transitions t2(EPSILON, s1, p2.first);
                    Transitions t3(EPSILON, p1.second, s2);
                    Transitions t4(EPSILON, p2.second, s2);
                    transitions.push_back(t1);
                    transitions.push_back(t2);
                    transitions.push_back(t3);
                    transitions.push_back(t4);
                    statePairs.push({s1, s2});
                    break;
                }
                case '*' : {
                    std::pair<State, State> p = statePairs.top();
                    statePairs.pop();
                    State s1(++index), s2(++index);
                    states.push_back(s1);
                    states.push_back(s2);
                    Transitions t1(EPSILON, s1, s2);
                    Transitions t2(EPSILON, s1, p.first);
                    Transitions t3(EPSILON, p.second, s2);
                    Transitions t4(EPSILON, p.second, p.first);
                    transitions.push_back(t1);
                    transitions.push_back(t2);
                    transitions.push_back(t3);
                    transitions.push_back(t4);
                    statePairs.push({s1, s2});
                    break;
                }
                default : break;
            }
        }
    }

    if(statePairs.size() > 1) {
        std::pair<State, State> p2 = statePairs.top();
        statePairs.pop();
        std::pair<State, State> p1 = statePairs.top();
        statePairs.pop();
        Transitions t(EPSILON, p1.second, p2.first);
        transitions.push_back(t);
        statePairs.push({p1.first, p2.second});
    }

    initial_state = statePairs.top().first;
    final_state = statePairs.top().second;
}

void NFA::print() {
    Automata::print(0, final_state, {});
    Automata::exportToDot(NFA_DOT_FILE_PATH, 0, final_state, {});
    system("dot -Tpng nfa.dot -o nfa.png");
}