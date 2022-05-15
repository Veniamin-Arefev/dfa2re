#include "api.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>


class MyNode {
public:
    std::set<int> in_states;
    std::map<int, std::string> out_states;
};


std::string dfa2re(DFA &d) {
    Alphabet alphabet = d.get_alphabet();

    std::vector<std::string> states;
    std::map<std::string, int> states_to_int;

    for (auto &state: d.get_states()) {
        states.push_back(state);
    }
    states.push_back("real start state");
    states.push_back("real final state");

    for (int i = 0; i < states.size(); i++) {
        states_to_int[states[i]] = i;
    }

    std::vector<MyNode> my_nodes(states.size());
    for (auto &cur_state: d.get_states()) {
        for (auto &alpha: alphabet) {
            if (d.has_trans(cur_state, alpha)) {
                std::string to_state = d.get_trans(cur_state, alpha);
                my_nodes[states_to_int[to_state]].in_states.insert(states_to_int[cur_state]);
                if (my_nodes[states_to_int[cur_state]].out_states[states_to_int[to_state]].empty()) {
                    my_nodes[states_to_int[cur_state]].out_states[states_to_int[to_state]] = alpha;
                } else {
                    my_nodes[states_to_int[cur_state]].out_states[states_to_int[to_state]] +=
                            "|" + std::string(1, alpha);
                }

            }
        }
    }

    my_nodes[states_to_int[d.get_initial_state()]].in_states.insert(my_nodes.size() - 2);
    my_nodes[my_nodes.size() - 2].out_states[states_to_int[d.get_initial_state()]] = "";

    for (auto &state: d.get_final_states()) {
        my_nodes[states_to_int[state]].out_states[my_nodes.size() - 1] = "";
        my_nodes[my_nodes.size() - 1].in_states.insert(states_to_int[state]);
    }

    std::vector<std::pair<int, int> > priorities(states.size() - 2);
    for (int i = 0; i < states.size() - 2; i++) {
        priorities[i] = std::make_pair(i, my_nodes[i].in_states.size() + my_nodes[i].out_states.size());
    }

    std::sort(priorities.begin(), priorities.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });

    std::vector<int> queue_to_delete;
    for (auto &[index, priority]: priorities) {
        queue_to_delete.push_back(index);
    }

    for (auto &state_to_delete_index: queue_to_delete) {
        for (auto &q_state_index: my_nodes[state_to_delete_index].in_states) {
            if (q_state_index == state_to_delete_index) {
                continue;
            }
            MyNode new_q_state = my_nodes[q_state_index];
            new_q_state.out_states.erase(state_to_delete_index);
            for (auto &[p_state_index, p_transition]: my_nodes[state_to_delete_index].out_states) {
                std::string new_transition;
                if (my_nodes[p_state_index].in_states.count(q_state_index) == 1) { // add to start R |
                    if (my_nodes[q_state_index].out_states[p_state_index].size() > 1) {
                        new_transition += "(" + my_nodes[q_state_index].out_states[p_state_index] + ")|";
                    } else {
                        new_transition += my_nodes[q_state_index].out_states[p_state_index] + "|";
                    }
                }
                // Q S* P
                if (my_nodes[q_state_index].out_states[state_to_delete_index].size() > 1) { // Q
                    new_transition += "(" + my_nodes[q_state_index].out_states[state_to_delete_index] + ")";
                } else {
                    new_transition += my_nodes[q_state_index].out_states[state_to_delete_index];
                }
                if (my_nodes[state_to_delete_index].in_states.count(state_to_delete_index) == 1) { // S*
                    new_transition += "(" + my_nodes[state_to_delete_index].out_states[state_to_delete_index] + ")*";
                }
                if (p_transition.size() > 1) { // P
                    new_transition += "(" + p_transition + ")";
                } else {
                    new_transition += p_transition;
                }

                if (q_state_index == p_state_index) {
                    new_q_state.in_states.insert(q_state_index);
                } else {
                    my_nodes[p_state_index].in_states.insert(q_state_index);
                }
                new_q_state.out_states[p_state_index] = new_transition;
            }
            my_nodes[q_state_index] = new_q_state;
        }

        for (auto &[p_state_index, p_transition]: my_nodes[state_to_delete_index].out_states) {
            my_nodes[p_state_index].in_states.erase(state_to_delete_index);
        }
//        my_nodes[state_to_delete_index].in_states
//        my_nodes[state_to_delete_index].out_states

    }

    std::string reg = my_nodes[my_nodes.size() - 2].out_states[my_nodes.size() - 1];

    return reg;
}
