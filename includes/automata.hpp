# pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "fsm_elements.hpp"
#include "definitions.hpp"

class Automata {
public:
    std::vector<State> states;
    std::vector<char> alphabets;
    std::vector<Transitions> transitions;
    State initial_state;

    void print(int, State, std::vector<State>);
    void exportToDot(const std::string&, int, State, const std::vector<State>&);
};

void Automata::print(int automation, State final_state, std::vector<State> final_states) {
    if(automation == 0) {
        std::cout << "\n================= NFA Description =================" << std::endl;
    }
    else {
        std::cout << "\n================= DFA Description =================" << std::endl;
    }

    std::cout << "States: { ";
    for (const State& s : states) std::cout << s << " ";
    std::cout << "}" << std::endl;

    std::cout << "Alphabets: { ";
    for (char ch : alphabets) std::cout << "'" << ch << "' ";
    std::cout << "}" << std::endl;

    std::cout << "Start State: " << initial_state << std::endl;

    if(automation == 0) {
        std::cout << "Final State: " << final_state << std::endl;
    }
    else {
        std::cout << "Final States: { ";
        for (const State& s : final_states) std::cout << s << " ";
        std::cout << "}" << std::endl;
    }

    std::cout << "Transitions:" << std::endl;
    for (const auto& t : transitions) {
        std::cout << "  " << t.initial_state << " --" << t.input << "--> " << t.final_state << std::endl;
    }

    std::cout << "===================================================\n" << std::endl;
}

void Automata::exportToDot(const std::string& filename, int automation, State final_state, const std::vector<State>& final_states) {
    std::ofstream out(filename);
    if(!out) {
        std::cerr << "Error: Cannot open file: " << filename << std::endl;
        return;
    }

    out << "digraph Automaton {\n";
    out << "    rankdir=LR;\n";
    out << "    node [shape=circle];\n\n";

    out << "    __start__ [shape=point];\n";
    out << "    __start__ -> " << (initial_state.val == -1 ? "s_dead" : "s" + std::to_string(initial_state.val)) << ";\n";

    if(automation == 0) {
        out << "    " << (final_state.val == -1 ? "s_dead" : "s" + std::to_string(final_state.val)) << " [shape=doublecircle];\n";
    } 
    else {
        for (const State& s : final_states) {
            out << "    " << (s.val == -1 ? "s_dead" : "s" + std::to_string(s.val)) << " [shape=doublecircle];\n";
        }
    }

    bool has_dead_state = (initial_state.val == -1);
    
    for(const auto& t : transitions) {
        std::string label = (t.input == EPSILON) ? "ε" : std::string(1, t.input);
        std::string from = (t.initial_state.val == -1) ? "s_dead" : "s" + std::to_string(t.initial_state.val);
        std::string to = (t.final_state.val == -1) ? "s_dead" : "s" + std::to_string(t.final_state.val);

        out << "    " << from << " -> " << to << " [label=\"" << label << "\"];\n";

        if (t.initial_state.val == -1 || t.final_state.val == -1) {
            has_dead_state = true;
        }
    }

    if(has_dead_state) {
        out << "    s_dead [label=\"dead\"];\n";
    }

    out << "}\n";
    out.close();

    std::cout << "Check diagram folder for images." << std::endl;

    auto generateDiagram = [&](const std::string& imageFilename) {
        std::string dotPath = filename;
        std::string imagePath = imageFilename;

        std::string command = "dot -Tpng " + dotPath + " -o " + imagePath;
        system(command.c_str());
    };

    if(automation == 0) generateDiagram(NFA_IMAGE_FILE_PATH);
    else generateDiagram(DFA_IMAGE_FILE_PATH);
}