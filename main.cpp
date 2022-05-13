#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <ctime>
#include <time.h>

#include "cells.h"
#include "fm.h"
#include "nets.h"
#include "utils.h"

using namespace std;

void input_filenames(string& input, string& output) {
    cout << "Enter input filename: ";
    cin >> input;
    cout << '\n';
    cout << "Enter output filename: ";
    cin >> output;
    cout << '\n';
}

void input_graph(vector<vector<int>>& input_data,
           const int& count_vertices,
           int& input_count_edges) {
    for (int i = 0; i < count_vertices; i++) {
        int vertice;
        cin >> vertice;

        while (cin.peek() != '\n') {
            int input;
            cin >> input;
            input_data[vertice].push_back(input);
            input_count_edges = max(input_count_edges, input);
        }
    }
}

void make_incidence_list(vector<vector<int>>& incidence_list,
                         const vector<vector<int>>& input_data) {
    for (int i = 0; i < (int)input_data.size(); i++) {
        for (int j = 0; j < (int)input_data[i].size(); j++) {
            incidence_list[input_data[i][j]].push_back(i);
        }
    }
}

void make_adjacent_list(const vector<vector<int>>& incidence_list,
                        vector<vector<int>>& adjacent_list) {
    for (int i = 0; i < (int)incidence_list.size(); i++) {
        for (int j = 0; j < (int)incidence_list[i].size(); j++) {
            for (int k = 0; k < (int)incidence_list[i].size(); k++) {
                if (k == j)
                    continue;
                adjacent_list[incidence_list[i][j]].push_back(
                    incidence_list[i][k]);
            }
        }
    }
}

void print_answer(const vector<pair<int, int>>& TYPE_ID_SET, const int& n) {
    for (int i = 0; i < n; i++) {
        cout << TYPE_ID_SET[i].first << " " << TYPE_ID_SET[i].second << '\n';
    }
}

void split_parts_into_id_subsets(const vector<int>& parts_of_type_set,
                                 const vector<int>& count,
                                 vector<int>& count_type_subset,
                                 map<int, int>& parts_to_subset) {
    for (int i = 0; i < (int)parts_of_type_set.size(); i++) {
        if (parts_to_subset.find(parts_of_type_set[i] + 1) !=
            parts_to_subset.end()) {
            count_type_subset[parts_to_subset[parts_of_type_set[i] + 1]] +=
                count[parts_of_type_set[i]];
            parts_to_subset[parts_of_type_set[i]] =
                parts_to_subset[parts_of_type_set[i] + 1];
            continue;
        } else if (parts_to_subset.find(parts_of_type_set[i] - 1) !=
                   parts_to_subset.end()) {
            count_type_subset[parts_to_subset[parts_of_type_set[i] - 1]] +=
                count[parts_of_type_set[i]];
            parts_to_subset[parts_of_type_set[i]] =
                parts_to_subset[parts_of_type_set[i] - 1];
            continue;
        }

        int minimum_subset = 0;
        for (int i = 1; i < 4; i++) {
            if (count_type_subset[i] < count_type_subset[minimum_subset]) {
                minimum_subset = i;
            }
        }

        count_type_subset[minimum_subset] += count[parts_of_type_set[i]];
        parts_to_subset[parts_of_type_set[i]] = minimum_subset;
    }
}

int main(int argc, char const* argv[]) {
    //TODO: 
    //1) in core.cpp in naive give sides to cells in order to make all connected cells live together (soph = false)
    //2) in core.cpp in fm, fm once, init_gains set gains of connected nodes to int_min in order to not move them
    //3) in io.cpp change input and output so that connected cells wont filp (never, they must stay in their sides)
    //4) in io.cpp ?maybe add vector of connected cells in output file to make control easier
    //5) also need to change init.cpp

    log_release_mode();

    //input data, making incidence list in order to do fm
    cout << "Input number of vertices: " << '\n';
    int input_count_vertices;
    cin >> input_count_vertices;

    cout << "Input balance factor: " << '\n';
    double balance_factor;
    cin >> balance_factor;

    cout << "Input: " << '\n';
    const int COUNT_VERTICES = input_count_vertices;
    vector<vector<int>> input_data(COUNT_VERTICES);
    int input_count_edges = -1;
    input_graph(input_data, COUNT_VERTICES, input_count_edges);
    const int COUNT_EDGES = input_count_edges + 1;

    vector<vector<int>> incidence_list(COUNT_EDGES);
    make_incidence_list(incidence_list, input_data);

    vector<vector<int>> adjacent_list(COUNT_VERTICES);
    make_adjacent_list(incidence_list, adjacent_list);

    ofstream myfile;
    myfile.open("incidence.txt");
    myfile << balance_factor << '\n';
    for (int i = 0; i < (int)incidence_list.size(); i++) {
        myfile << "NET n" << i << " ";
        for (int j = 0; j < (int)incidence_list[i].size(); j++) {
            myfile << /*"c" <<*/ incidence_list[i][j] << " ";
        }
        myfile << " ;" << '\n';
    }
    myfile.close();

    FloorPlan fp;
    //input = "input_1.dat";
    string input = "incidence.txt";
    string output = "out_1.txt";
    //string input, output;
    //input_filenames(input, output);
    fp.input(input);

    const unsigned nsize = fp.nmap().size();
    const unsigned csize = fp.cmap().size();
    const double bal = fp.balance();

    const unsigned tolerate = static_cast<unsigned>(bal * csize);
    fp.tolerate(tolerate);

    constexpr bool soph = false;
    fp.init_side<soph>();

    debug_printf(
        "balance = %lf, "
        "net_size = %u, "
        "cell_size = %u\n",
        bal, nsize, csize);

    fp.fm();
    fp.output(output, 0, 1, 0, 0);

    debug_printf("Program exit.\n");

    return 0;
}
