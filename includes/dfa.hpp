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

    bool is_null_state = false;

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

    if(is_null_state) {
        for(char input : alphabets) transitions.push_back(Transitions(input, NULL_STATE, NULL_STATE));
    }
}

minimized_DFA DFA::state_reduction() {
    // Step 1: Build transition map for fast lookup
    std::map<std::pair<State, char>, State> state_transitions;
    for(const auto& t : transitions) {
        state_transitions[{t.initial_state, t.input}] = t.final_state;
    }

    // Step 2: Initialize partition — final vs non-final
    std::set<std::set<State>> state_holder;
    std::set<State> final_set(final_states.begin(), final_states.end());
    std::set<State> non_finals;

    for(const auto& s : states) {
        if(final_set.find(s) == final_set.end()) {
            non_finals.insert(s);
        }
    }

    if(!final_set.empty()) state_holder.insert(final_set);
    if(!non_finals.empty()) state_holder.insert(non_finals);

    // Step 3: Iteratively refine the partition
    while(true) {
        // Normalize state_holder into a vector of sorted vectors
        std::vector<std::vector<State>> normalized;
        for(const auto& group : state_holder) {
            std::vector<State> sorted_group(group.begin(), group.end());
            std::sort(sorted_group.begin(), sorted_group.end());
            normalized.push_back(sorted_group);
        }
        std::sort(normalized.begin(), normalized.end());

        // Build state_to_partition using stable partition IDs
        std::map<State, int> state_to_partition;
        for(int i = 0; i < normalized.size(); ++i) {
            for(const State& s : normalized[i]) {
                state_to_partition[s] = i;
            }
        }

        // Refine the partitions
        std::set<std::set<State>> new_state_holder;
        for(const auto& group : state_holder) {
            std::map<std::vector<int>, std::set<State>> signature_map;
            for(const State& s : group) {
                std::vector<int> signature;
                for(const char& c : alphabets) {
                    auto it = state_transitions.find({s, c});
                    if(it != state_transitions.end()) {
                        signature.push_back(state_to_partition[it->second]);
                    } 
                    else {
                        signature.push_back(-1); // missing transition
                    }
                }
                signature_map[signature].insert(s);
            }

            for(auto& it : signature_map) {
                new_state_holder.insert(it.second);
            }
        }

        if(new_state_holder == state_holder) break;
        state_holder = new_state_holder;
    }

    // Step 4: Build the minimized DFA
    minimized_DFA mini_dfa;
    mini_dfa.alphabets = alphabets;

    std::map<State, State> old_to_new_state;
    std::vector<std::set<State>> ordered_holder(state_holder.begin(), state_holder.end());

    // Normalize partition order
    std::sort(ordered_holder.begin(), ordered_holder.end(), [](const std::set<State>& a, const std::set<State>& b) {
        return *a.begin() < *b.begin();
    });

    int state_id = 0;
    for(const auto& group : ordered_holder) {
        State rep(state_id++);
        for(const auto& old_state : group) {
            old_to_new_state[old_state] = rep;
        }
        mini_dfa.states.push_back(rep);
    }

    // Set initial state
    mini_dfa.initial_state = old_to_new_state[initial_state];

    // Set final states
    std::set<State> added_finals;
    for(const auto& s : final_states) {
        State new_s = old_to_new_state[s];
        if(!added_finals.count(new_s)) {
            mini_dfa.final_states.push_back(new_s);
            added_finals.insert(new_s);
        }
    }

    // Set transitions
    std::set<std::tuple<State, char, State>> seen_transitions;
    for(const auto& it : state_transitions) {
        auto key = it.first;
        auto dest = it.second;
        const State& src = key.first;
        const char& input = key.second;
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