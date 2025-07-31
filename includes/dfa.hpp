# pragma once

#include <vector>
#include <map>
#include <set>
#include <utility>
#include "nfa.hpp"
#include "automata.hpp"
#include "definitions.hpp"

class DFA : public Automata {
public:
    std::vector<State> final_states;

    void create_dfa(NFA nfa, std::map<std::pair<State, char>, std::set<State>>& stateTransitions, bool ep);
    void print();
};

void DFA::create_dfa(NFA nfa, std::map<std::pair<State, char>, std::set<State>>& stateTransitions, bool ep) {
    alphabets = nfa.alphabets;

    std::vector<std::vector<State>> temp_states;
    std::map<std::vector<State>, int> stateMap;
    int counter = 0;
    
    temp_states.push_back({nfa.initial_state});
    stateMap[{nfa.initial_state}] = ++counter;
    int index = 0;

    while(index < int(temp_states.size())) {
        if(temp_states[index].empty()) {
            index++;
            continue;
        }

        for(char input : nfa.alphabets) {
            std::set<State> newStates;

            for(State s : temp_states[index]) {
                if(stateTransitions.find({s, input}) != stateTransitions.end()) {
                    for(State s : stateTransitions[{s, input}]) {
                        newStates.insert(s);
                    }
                }
            }

            std::vector<State> newStateElement(newStates.begin(), newStates.end());
            
            if(stateMap.find(newStateElement) == stateMap.end()) {
                if(newStateElement.empty()) stateMap[newStateElement] = NULL_STATE;
                else stateMap[newStateElement] = ++counter;
                temp_states.push_back(newStateElement);
            }

            State s1(stateMap[temp_states[index]]), s2(stateMap[newStateElement]);
            Transitions t(input, s1, s2);
            transitions.push_back(t);

        }

        index++;
    }

    initial_state = State(1);
    if(ep) final_states.push_back(initial_state);

    for(auto newStates : stateMap) {
        State s = State(newStates.second);
        states.push_back(s);
        
        bool find = false;
        for(State state : newStates.first) {
            if(state.val == nfa.final_state.val) {
                find = true;
                break;
            }
        }
        if(find) final_states.push_back(s);
    }
}

void DFA::print() {
    Automata::print(1, State(), final_states);
    Automata::exportToDot(DFA_DOT_FILE_PATH, 1, State(), final_states);
    system("dot -Tpng dfa.dot -o dfa.png");
}