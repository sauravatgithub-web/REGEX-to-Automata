# pragma once

#include <iostream>
#include <string>
#include <set>
#include <map>
#include <utility>
#include "regex_postfix.hpp"
#include "fsm_elements.hpp"
#include "nfa.hpp"
#include "dfa.hpp"

std::map<std::pair<State, char>, std::set<State>> createTransitionMap(NFA);
void fill_epsilon_inputs(NFA, std::map<std::pair<State, char>, std::set<State>>&);
void fill_non_epsilon_inputs(NFA, std::map<std::pair<State, char>, std::set<State>>&);


void helper(std::string expression, int input) {
    std::string postfix = toPostfix(expression);

    NFA nfa = NFA();
    nfa.create_nfa(postfix);

    std::map<std::pair<State, char>, std::set<State>> stateTransitions = createTransitionMap(nfa);
    
    std::cout << std::endl << std::endl;
    
    DFA dfa = DFA();
    dfa.create_dfa(nfa, stateTransitions, stateTransitions[{nfa.initial_state, EPSILON}].count(nfa.final_state));

    minimized_DFA mini_dfa = dfa.state_reduction();
    
    if(input == 1 || input == 4) nfa.print();
    if(input == 2 || input == 4) dfa.print();
    if(input == 3 || input == 4) mini_dfa.print();
}

std::map<std::pair<State, char>, std::set<State>> createTransitionMap(NFA nfa) {
    std::map<std::pair<State, char>, std::set<State>> stateTransitions;

    fill_epsilon_inputs(nfa, stateTransitions);
    fill_non_epsilon_inputs(nfa, stateTransitions);

    return stateTransitions;
}

void fill_epsilon_inputs(NFA nfa, std::map<std::pair<State, char>, std::set<State>>& stateTransitions) {
    for(Transitions t : nfa.transitions) {
        stateTransitions[{t.initial_state, t.input}].insert(t.final_state);
    }
    for(State s : nfa.states) stateTransitions[{s, EPSILON}].insert(s);
    
    for(auto& transits : stateTransitions) {
        if(transits.first.second == EPSILON) {
            for(State state : transits.second) {
                for(State nextStates : stateTransitions[{state, EPSILON}]) {
                    transits.second.insert(nextStates);
                }
            }
        }
    }
}

void fill_non_epsilon_inputs(NFA nfa, std::map<std::pair<State, char>, std::set<State>>& stateTransitions) {
    for(char input : nfa.alphabets) {
        for(State s : nfa.states) {            
            for(State state : stateTransitions[{s, EPSILON}]) {
                for(State nextState : stateTransitions[{state, input}]) {
                    for(State actualState : stateTransitions[{nextState, EPSILON}]) {
                        stateTransitions[{s, input}].insert(actualState);
                    }
                }
            }
        }
    }
}