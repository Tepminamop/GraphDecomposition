#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <ctime>
#include <time.h>
#include <set>
#include <sstream>

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

void input_graph_from_file(vector<vector<int>>& input_data,
    const int& count_vertices,
    int& input_count_edges, const string& filename) {
    string str;
    std::fstream in;
    in.open(filename);
    if (in) {
        while (!in.eof()) {
            while (getline(in, str, '\n') && !in.eof()) {
                vector<int> result;
                stringstream ss(str);
                string item;
                while (getline(ss, item, ' ')) {
                    result.push_back(stoi(item));
                }
                for (int i = 1; i < result.size(); i++) {
                    input_data[result[0]].push_back(result[i]);
                    input_count_edges = max(input_count_edges, result[i]);
                }
                result.clear();
            }
        }
        in.close();
    }
    else(cout << "Error with kuk_top.nls file!");
}

int main(int argc, char const* argv[]) {
    //TODO: 
    //1) in core.cpp in naive give sides to cells in order to make all connected cells live together (soph = false)
    //2) in core.cpp in fm, fm once, init_gains set gains of connected nodes to int_min in order to not move them
    //3) in io.cpp change input and output so that connected cells wont filp (never, they must stay in their sides)
    //4) in io.cpp ?maybe add vector of connected cells in output file to make control easier
    //5) also need to change init.cpp

    log_release_mode();
    /*set<int> verts;
    for (int i = 0; i < 5900; i++) {
        verts.insert(i);
    }*/

    //input data, making incidence list in order to do fm

    cout << "Input number of vertices: " << '\n';
    int input_count_vertices;
    cin >> input_count_vertices;

    cout << "Input balance factor: " << '\n';
    double balance_factor;
    cin >> balance_factor;

    cout << "Input filename: " << '\n';
    string filename;
    //cin >> filename;
    filename = "kuk_top.nls";
    //filename = "TRINITY_1.nls";
    const int COUNT_VERTICES = input_count_vertices;
    vector<vector<int>> input_data(COUNT_VERTICES);
    int input_count_edges = -1;
    input_graph_from_file(input_data, COUNT_VERTICES, input_count_edges, filename);
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


    unsigned int size_true_1 = 0;
    unsigned int size_false_1 = 0;
    unsigned int size_true_2 = 0;
    unsigned int size_false_2 = 0;
    unsigned int size_true_3 = 0;
    unsigned int size_false_3 = 0;
    unsigned int size_true_4 = 0;
    unsigned int size_false_4 = 0;
    unsigned int size_true_5 = 0;
    unsigned int size_false_5 = 0;

    unsigned int size_true_6 = 0;
    unsigned int size_false_6 = 0;
    unsigned int size_true_7 = 0;
    unsigned int size_false_7 = 0;


    //FIRST
    FloorPlan fp_1;
    string input_first = "incidence.txt";
    //string output = "out_1.txt";
    fp_1.input(input_first);

    const unsigned nsize_1 = fp_1.nmap().size();
    const unsigned csize_1 = fp_1.cmap().size();
    const double bal_1 = fp_1.balance();

    const unsigned tolerate_1 = static_cast<unsigned>(bal_1 * csize_1);
    fp_1.tolerate(tolerate_1);

    constexpr bool soph = true;
    fp_1.init_side<soph>();

    debug_printf(
        "balance = %lf, "
        "net_size = %u, "
        "cell_size = %u\n",
        bal_1, nsize_1, csize_1);

    fp_1.fm();
    //fp.output(output, 0, 1, 0, 0);
    set<unsigned int> first_connected_vertices_true;
    set<unsigned int> first_connected_vertices_false;
    fp_1.output_to_decomposite("true_1.txt", "false_1.txt", 0, 1, 0, 0, first_connected_vertices_true, first_connected_vertices_false, csize_1);
    fp_1.get_true_false_count(size_true_1, size_false_1);


    //SECOND
    FloorPlan fp_2;
    string input_second = "true_1.txt";
    //string output = "out_1.txt";
    fp_2.input(input_second);

    const unsigned nsize_2 = fp_2.nmap().size();
    const unsigned csize_2 = fp_2.cmap().size() + first_connected_vertices_true.size();//?????????????????????
    const double bal_2 = fp_2.balance();

    const unsigned tolerate_2 = static_cast<unsigned>(bal_2 * csize_2);
    fp_2.tolerate(tolerate_2);

    fp_2.init_side<soph>();

    debug_printf(
        "balance = %lf, "
        "net_size = %u, "
        "cell_size = %u\n",
        bal_2, nsize_2, csize_2);

    fp_2.fm();
    //fp.output(output, 0, 1, 0, 0);
    set<unsigned int> second_connected_vertices_true;
    set<unsigned int> second_connected_vertices_false;
    fp_2.output_to_decomposite("true_2.txt", "false_2.txt", 0, 1, 0, 0, second_connected_vertices_true, second_connected_vertices_false, csize_2);
    fp_2.get_true_false_count(size_true_2, size_false_2);


    //THIRD
    FloorPlan fp_3;
    string input_third = "false_1.txt";
    //string output = "out_1.txt";
    fp_3.input(input_third);

    const unsigned nsize_3 = fp_3.nmap().size();
    const unsigned csize_3 = fp_3.cmap().size() + first_connected_vertices_false.size();//?????????????????????????
    const double bal_3 = fp_3.balance();

    const unsigned tolerate_3 = static_cast<unsigned>(bal_3 * csize_3);
    fp_3.tolerate(tolerate_3);

    fp_3.init_side<soph>();

    debug_printf(
        "balance = %lf, "
        "net_size = %u, "
        "cell_size = %u\n",
        bal_3, nsize_3, csize_3);

    fp_3.fm();
    //fp.output(output, 0, 1, 0, 0);
    set<unsigned int> third_connected_vertices_true;
    set<unsigned int> third_connected_vertices_false;
    fp_3.output_to_decomposite("true_3.txt", "false_3.txt", 0, 1, 0, 0, third_connected_vertices_true, third_connected_vertices_false, csize_3);
    fp_3.get_true_false_count(size_true_3, size_false_3);


    //FOURTH
    //FloorPlan fp_4;
    //string input_fourth = "true_3.txt";
    ////string output = "out_1.txt";
    //fp_4.input(input_fourth);

    //const unsigned nsize_4 = fp_4.nmap().size();
    //const unsigned csize_4 = fp_4.cmap().size() + third_connected_vertices_true.size();//?????????????????????????
    //const double bal_4 = fp_4.balance();

    //const unsigned tolerate_4 = static_cast<unsigned>(bal_4 * csize_4);
    //fp_4.tolerate(tolerate_4);

    //fp_4.init_side<soph>();

    //debug_printf(
    //    "balance = %lf, "
    //    "net_size = %u, "
    //    "cell_size = %u\n",
    //    bal_4, nsize_4, csize_4);

    //fp_4.fm();
    ////fp.output(output, 0, 1, 0, 0);
    //set<unsigned int> fourth_connected_vertices_true;
    //set<unsigned int> fourth_connected_vertices_false;
    //fp_4.output_to_decomposite("true_4.txt", "false_4.txt", 0, 1, 0, 0, fourth_connected_vertices_true, fourth_connected_vertices_false, csize_4);
    //fp_4.get_true_false_count(size_true_4, size_false_4);


    //FIFTH
    FloorPlan fp_5;
    string input_fifth = "false_2.txt";
    //string output = "out_1.txt";
    fp_5.input(input_fifth);

    const unsigned nsize_5 = fp_5.nmap().size();
    const unsigned csize_5 = fp_5.cmap().size() + second_connected_vertices_false.size() + second_connected_vertices_false.size();//?????????????????????????
    const double bal_5 = fp_5.balance();

    const unsigned tolerate_5 = static_cast<unsigned>(bal_5 * csize_5);
    fp_5.tolerate(tolerate_5);

    fp_5.init_side<soph>();

    debug_printf(
        "balance = %lf, "
        "net_size = %u, "
        "cell_size = %u\n",
        bal_5, nsize_5, csize_5);

    fp_5.fm();
    //fp.output(output, 0, 1, 0, 0);
    set<unsigned int> fifth_connected_vertices_true;
    set<unsigned int> fifth_connected_vertices_false;
    fp_5.output_to_decomposite("true_5.txt", "false_5.txt", 0, 1, 0, 0, fifth_connected_vertices_true, fifth_connected_vertices_false, csize_5);//different connected vertices
    fp_5.get_true_false_count(size_true_5, size_false_5);


    ////SIXTH
    //FloorPlan fp_6;
    //string input_sixth = "true_2.txt";
    ////string output = "out_1.txt";
    //fp_6.input(input_sixth);

    //const unsigned nsize_6 = fp_6.nmap().size();
    //const unsigned csize_6 = fp_6.cmap().size() + second_connected_vertices_true.size();//?????????????????????????
    //const double bal_6 = fp_6.balance();

    //const unsigned tolerate_6 = static_cast<unsigned>(bal_6 * csize_6);
    //fp_6.tolerate(tolerate_6);

    //fp_6.init_side<soph>();

    //debug_printf(
    //    "balance = %lf, "
    //    "net_size = %u, "
    //    "cell_size = %u\n",
    //    bal_6, nsize_6, csize_6);

    //fp_6.fm();
    ////fp.output(output, 0, 1, 0, 0);
    //set<unsigned int> sixth_connected_vertices_true;
    //set<unsigned int> sixth_connected_vertices_false;
    //fp_6.output_to_decomposite("true_6.txt", "false_6.txt", 0, 1, 0, 0, sixth_connected_vertices_true, sixth_connected_vertices_false, csize_6);
    //fp_6.get_true_false_count(size_true_6, size_false_6);


    ////SEVENTH
    //FloorPlan fp_7;
    //string input_seventh = "false_3.txt";
    ////string output = "out_1.txt";
    //fp_7.input(input_seventh);

    //const unsigned nsize_7 = fp_7.nmap().size();
    //const unsigned csize_7 = fp_7.cmap().size() + third_connected_vertices_false.size();//?????????????????????????
    //const double bal_7 = fp_7.balance();

    //const unsigned tolerate_7 = static_cast<unsigned>(bal_7 * csize_7);
    //fp_7.tolerate(tolerate_7);

    //fp_7.init_side<soph>();

    //debug_printf(
    //    "balance = %lf, "
    //    "net_size = %u, "
    //    "cell_size = %u\n",
    //    bal_7, nsize_7, csize_7);

    //fp_7.fm();
    ////fp.output(output, 0, 1, 0, 0);
    //set<unsigned int> seventh_connected_vertices_true;
    //set<unsigned int> seventh_connected_vertices_false;
    //fp_7.output_to_decomposite("true_7.txt", "false_7.txt", 0, 1, 0, 0, seventh_connected_vertices_true, seventh_connected_vertices_false, csize_7);
    //fp_7.get_true_false_count(size_true_7, size_false_7);

    cout << "Subgraphs size: " << '\n';
    cout << size_true_1 << " " << size_false_1 << '\n';
    cout << size_true_2 << " " << size_false_2 << '\n';
    cout << size_true_3 << " " << size_false_3 << '\n';

    cout << size_true_4 << " " << size_false_4 << '\n';
    cout << size_true_5 << " " << size_false_5 << '\n';
    cout << size_true_6 << " " << size_false_6 << '\n';
    cout << size_true_7 << " " << size_false_7 << '\n';

    cout << "Connected vertices size: " << '\n';
    cout << 


    debug_printf("Program exit.\n");

    return 0;
}
