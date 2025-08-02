# pragma once

class State {
public:
    int val;
    
    State() { val = 0; }
    State(int v) { val = v; }

    bool operator==(const State& other) const {
        return val == other.val;
    }    

    bool operator!=(const State& other) const {
        return val != other.val;
    }    

    bool operator<(const State& other) const {
        return val < other.val;
    }
};

std::ostream& operator<<(std::ostream& os, const State& state) {
    if(state.val != -1) os << "s" << state.val;
    else os << "NULL";
    return os;
}


class Transitions {
public:
    char input;
    State initial_state;
    State final_state;

    Transitions(char symbol, State s1, State s2) {
        input = symbol, initial_state = s1, final_state = s2;
    }
};