# pragma once

#include <vector>
#include <map>
#include <set>
#include <utility>
#include <algorithm>
#include "nfa.hpp"
#include "automata.hpp"
#include "definitions.hpp"

class minimized_DFA : public Automata {
public:
    std::vector<State> final_states;

    void print();
};

class DFA : public Automata {
public:
    std::vector<State> final_states;
    
    void create_dfa(NFA nfa, std::map<std::pair<State, char>, std::set<State>>& stateTransitions, bool ep);
    minimized_DFA state_reduction();
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

minimized_DFA DFA::state_reduction() {
    // Step 1: Build transition map for fast lookup
    std::map<std::pair<State, char>, State> state_transitions;
    for(const auto& t : transitions) {
        state_transitions[{t.initial_state, t.input}] = t.final_state;
    }

    // Step 2: Initialize partition — final vs non-final
    std::vector<std::vector<State>> state_holder;
    std::set<State> final_set(final_states.begin(), final_states.end());
    
    std::vector<State> non_finals;
    for(const auto& s : states) {
        if(final_set.find(s) == final_set.end()) {
            non_finals.push_back(s);
        }
    }

    if(!final_states.empty()) state_holder.push_back(final_states);
    if(!non_finals.empty())  state_holder.push_back(non_finals);

    // Step 3: Iteratively refine the partition
    while(true) {
        std::map<State, int> state_to_partition;
        for(int i = 0; i < state_holder.size(); ++i) {
            for(const auto& s : state_holder[i]) {
                state_to_partition[s] = i;
            }
        }

        std::map<std::vector<int>, std::vector<State>> signature_map;

        for(const auto& group : state_holder) {
            for(const auto& s : group) {
                std::vector<int> signature;
                for(const auto& c : alphabets) {
                    auto it = state_transitions.find({s, c});
                    if(it != state_transitions.end()) {
                        signature.push_back(state_to_partition[it->second]);
                    } 
                    else {
                        signature.push_back(-1); // special code for missing transition
                    }
                }
                signature_map[signature].push_back(s);
            }
        }

        std::vector<std::vector<State>> new_state_holder;
        for(auto& [_, group] : signature_map) {
            new_state_holder.push_back(group);
        }

        if(new_state_holder == state_holder) break;
        state_holder = new_state_holder;
    }

    // Step 4: Build the minimized DFA
    minimized_DFA mini_dfa;
    mini_dfa.alphabets = alphabets;

    std::map<State, State> old_to_new_state;
    std::map<int, State> partition_id_to_state;

    int state_id = 0;
    for(int i = 0; i < state_holder.size(); ++i) {
        State rep(state_id++);
        for(const auto& old_state : state_holder[i]) {
            old_to_new_state[old_state] = rep;
        }
        partition_id_to_state[i] = rep;
        mini_dfa.states.push_back(rep);
    }

    // Set minimized initial state
    mini_dfa.initial_state = old_to_new_state[initial_state];

    // Set minimized final states
    std::set<State> added_finals;
    for(const auto& s : final_states) {
        State new_s = old_to_new_state[s];
        if(!added_finals.count(new_s)) {
            mini_dfa.final_states.push_back(new_s);
            added_finals.insert(new_s);
        }
    }

    // Build transitions
    // std::pair<State, char> key;
    // State dest;
    std::set<std::tuple<State, char, State>> seen_transitions;
    for (const auto& [key, dest] : state_transitions) {
        const State& src = key.first;
        char input = key.second;
        State new_src = old_to_new_state[src];
        State new_dest = old_to_new_state[dest];

        if(!seen_transitions.count({new_src, input, new_dest})) {
            mini_dfa.transitions.emplace_back(input, new_src, new_dest);
            seen_transitions.insert({new_src, input, new_dest});
        }
    }

    return mini_dfa;
}


void DFA::print() {
    Automata::print(1, State(), final_states);
    Automata::exportToDot(DFA_DOT_FILE_PATH, 1, State(), final_states);
    system("dot -Tpng dfa.dot -o dfa.png");
}

void minimized_DFA::print() {
    Automata::print(1, State(), final_states);
    Automata::exportToDot(MINI_DFA_DOT_FILE_PATH, 1, State(), final_states);
    system("dot -Tpng mini_dfa.dot -o mini_dfa.png");
}