#include "fm.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <set>

using namespace std;

void FloorPlan::input(const string fname) {
    unordered_map<string, unordered_set<string>> nmap_set, cmap_set;

    auto file = ifstream(fname);

    file >> _balance;

    string buffer;
    while (file >> buffer) {
        if (buffer != "NET") {
            break;
        }

        string nname;
        file >> nname;

        nmap_set[nname] = unordered_set<string>();

        unordered_set<string>& current_net = nmap_set[nname];

        string cname;
        while (file >> cname) {
            if (cname == ";") {
                break;
            }

            if (cmap_set.find(cname) == cmap_set.end()) {
                cmap_set[cname] = unordered_set<string>();
            }

            current_net.insert(cname);

            unordered_set<string>& current_cell = cmap_set[cname];
            current_cell.insert(nname);
        }
    }

    unordered_map<string, unsigned> rev_nnames, rev_cnames;

    assert(_nmap.size() == 0 && _cmap.size() == 0);
    assert(_nnames.size() == 0 && _cnames.size() == 0);

    unsigned nsize = nmap_set.size();
    unsigned csize = cmap_set.size();

    _nmap.reserve(nsize);
    _cmap.reserve(csize);

    _nnames.reserve(nsize);
    _cnames.reserve(csize);

    for (unsigned idx = 0; idx < nsize; ++idx) {
        _nmap.push_back(new Net());
    }
    for (unsigned idx = 0; idx < csize; ++idx) {
        _cmap.push_back(new Cell());
    }

    for (auto iter : nmap_set) {
        const string name = iter.first;
        if (rev_nnames.find(name) == rev_nnames.end()) {
            rev_nnames[name] = _nnames.size();
            _nnames.push_back(name);
        }
    }

    for (auto iter : cmap_set) {
        const string name = iter.first;
        if (rev_cnames.find(name) == rev_cnames.end()) {
            rev_cnames[name] = _cnames.size();
            _cnames.push_back(name);
        }
    }

    for (auto iter : nmap_set) {
        const string nname = iter.first;
        const auto& cell_list = iter.second;
        const unsigned net_id = rev_nnames[nname];

        Net* net = _nmap[net_id];

        assert(net_id < _nmap.size());

        for (string cname : cell_list) {
            const unsigned cell_id = rev_cnames[cname];

            assert(cell_id < _cmap.size());

            net->push_cell(cell_id);

            Cell* cell = _cmap[cell_id];
            cell->push_net(net_id);
        }
    }

    // Asserts
    assert(_nnames.size() == rev_nnames.size());
    for (unsigned idx = 0; idx < _nnames.size(); ++idx) {
        const string name = _nnames[idx];
        assert(rev_nnames[name] == idx);
    }
    assert(_cnames.size() == rev_cnames.size());
    for (unsigned idx = 0; idx < _cnames.size(); ++idx) {
        const string name = _cnames[idx];
        assert(rev_cnames[name] == idx);
    }
}

void FloorPlan::output(const string fname, const int set1, const int set2, const int subset1, const int subset2) {
    stringstream ss;
    auto file = ofstream(fname);

    unsigned cut_size = 0;
    for (unsigned idx = 0; idx < _nmap.size(); ++idx) {
        const Net* net = _nmap[idx];
        if (net->true_count() && net->false_count()) {
            ++cut_size;
        }
    }

    ss << "Cutsize: " << cut_size << "\n";
    file << ss.str();

    stringstream true_ss, false_ss;
    unsigned true_count = 0, false_count = 0;

    true_ss << "\n";
    false_ss << "\n";

    for (unsigned idx = 0; idx < _cmap.size(); ++idx) {
        const string name = _cnames[idx];
        const Cell* cell = _cmap[idx];
        if (cell->side()) {
            ++true_count;
            true_ss << set1 << " " << subset1 << " " << name << '\n';
        } else {
            ++false_count;
            false_ss << set2 << " " << subset2 << " " << name << '\n';
        }
    }

    true_ss << '\n';
    false_ss << '\n';

    assert(true_count + false_count == _cmap.size());

    file << "Count " << set1 << ": " << true_count << true_ss.str();
    file << "Count " << set2 << ": " << false_count << false_ss.str();

    file.close();
}

void FloorPlan::output_to_decomposite(const string fname1, const string fname2, const int set1, const int set2, const int subset1, const int subset2,
    set<unsigned int>& connected_vertices_true, set<unsigned int>& connected_vertices_false, const unsigned int size, const unsigned int conn_size_true, const unsigned int conn_size_false) {
    stringstream ss;
    auto file_true = ofstream(fname1);
    auto file_false = ofstream(fname2);
    set<unsigned int> connected_vertices;

    //checking nets which we cut
    for (unsigned idx = 0; idx < _nmap.size(); ++idx) {
        const Net* net = _nmap[idx];
        if (net->true_count() && net->false_count()) {
            for (auto cell : net->cells()) {
                connected_vertices.insert(cell);//if there are same cells add only one (set?)
            }
        }
    }

    //checking vertices that connects two subgraphs
    for (auto iterator = connected_vertices.begin(); iterator != connected_vertices.end(); ++iterator) {
        Cell* cell = _cmap[*iterator];
        if (cell->side()) {
            connected_vertices_true.insert(*iterator);
        }
        else {
            connected_vertices_false.insert(*iterator);

        }
    }

    cout << connected_vertices_true.size() << '\n';
    cout << connected_vertices_false.size() << '\n';
    int SIZE_TRUE = size - (int)connected_vertices_true.size() - conn_size_true;
    int SIZE_FALSE = size - (int)connected_vertices_false.size();
    double balance_factor_true = (double)connected_vertices_true.size() / ((double)(SIZE_TRUE));
    double balance_factor_false = ((double)connected_vertices_false.size() + (double)conn_size_false) / ((double)(SIZE_FALSE));
    file_true << balance_factor_true << '\n'; //balance factor
    file_false << balance_factor_false << '\n';
    unsigned int counter_true = 0;
    unsigned int counter_false = 0;
    unsigned int cell_counter = 0;
    vector<bool> check_vertices(size + 1, false);
    for (unsigned idx = 0; idx < _nmap.size(); ++idx) {
        const Net* net = _nmap[idx];
        if (net->true_count() && net->false_count()) {
            continue;
        }
        else if (net->true_count()) {
            file_true << "NET n" << counter_true << " ";
            counter_true++;
            for (auto cell : net->cells()) {
                if (connected_vertices_true.find(cell) == connected_vertices_true.end()) {
                    file_true << cell << " ";
                    check_vertices[cell] = true;
                }
            }

            file_true << ";\n";
        }
        else {
            file_false << "NET n" << counter_false << " ";
            counter_false++;
            for (auto cell : net->cells()) {
                if (connected_vertices_false.find(cell) == connected_vertices_false.end()) {
                    file_false << cell << " ";
                    check_vertices[cell] = true;
                }
            }

            file_false << ";\n";
        }
    }

    file_true.close();
    file_false.close();

    for (int i = 0; i < size + 1; i++) {
        if (check_vertices[i]) {
            cell_counter++;
        }
    }

    //assert(cell_counter + connected_vertices_true.size() + connected_vertices_false.size() == size);
}

void FloorPlan::get_true_false_count(unsigned int& true_count, unsigned int& false_count) {
    for (unsigned idx = 0; idx < _cmap.size(); ++idx) {
        const Cell* cell = _cmap[idx];
        if (cell->side()) {
            true_count++;
        }
        else {
            false_count++;
        }
    }
}

void FloorPlan::output_to_decomposite_without_connection_control(const string fname1, const string fname2, const int set1, const int set2, const int subset1, const int subset2,
    set<unsigned int>& connected_vertices_true, set<unsigned int>& connected_vertices_false, set<unsigned int>& check_vertices, bool check_side) {
    stringstream ss_true, ss_false;
    auto file_true = ofstream(fname1);
    auto file_false = ofstream(fname2);
    set<unsigned int> connected_vertices;

    //balance factor
    double balance_factor_true = 0.01;
    double balance_factor_false = 0.01;

    file_true << balance_factor_true << '\n';
    file_false << balance_factor_false << '\n';

    unsigned int counter_true = 0;
    unsigned int counter_false = 0;

    //check cell connection
    if (check_vertices.size() > 0) {
        for (unsigned int idx = 0; idx < _cmap.size(); ++idx) {
            Cell* cell = _cmap[idx];
            int cell_name = stoi(_cnames[idx]);
            if (cell->side() == check_side) {
                if (check_vertices.find(cell_name) != check_vertices.end()) {
                    cell->set_side(!check_side);
                }
            }
        }
    }
    
    //output to file
    for (unsigned idx = 0; idx < _nmap.size(); ++idx) {
        const Net* net = _nmap[idx];
        if (net->true_count() && net->false_count()) {
            //if there are same cells add only one (set?), NEED TO CHECK THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            ss_true << "NET n" << counter_true << " ";
            counter_true++;

            ss_false << "NET n" << counter_false << " ";
            counter_false++;

            for (auto cell_int : net->cells()) {
                //cout << cell_int << " " << _cnames[cell_int] << '\n'; //EROR HERE NEED TO OUTPUT _CELLNAMES!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                Cell* cell = _cmap[cell_int];
                int cell_name = stoi(_cnames[cell_int]);
                if (cell->side()) {
                    connected_vertices_true.insert(cell_name);
                    ss_true << cell_name << " ";
                }
                else {
                    /*if (check_vertices.find(cell_int) != check_vertices.end()) {
                        connected_vertices_true.insert(cell_int);

                        ss_true << cell_int << " ";
                    }
                    else {
                        connected_vertices_false.insert(cell_int);

                        ss_false << cell_int << " ";
                    }*/
                    connected_vertices_false.insert(cell_name);
                    ss_false << cell_name << " ";
                }
            }

            ss_true << ";\n";
            ss_false << ";\n";

            file_true << ss_true.str();
            file_false << ss_false.str();

            ss_true.clear();
            ss_false.clear();
        }
        else if (net->true_count()) {
            file_true << "NET n" << counter_true << " ";
            counter_true++;
            for (auto cell : net->cells()) {
                int cell_name = stoi(_cnames[cell]);
                if (connected_vertices_true.find(cell_name) == connected_vertices_true.end()) {
                    file_true << cell_name << " ";
                }
            }

            file_true << ";\n";
        }
        else {
            file_false << "NET n" << counter_false << " ";
            counter_false++;
            for (auto cell : net->cells()) {
                int cell_name = stoi(_cnames[cell]);
                if (connected_vertices_false.find(cell_name) == connected_vertices_false.end()) {
                    file_false << cell_name << " ";
                }
            }

            file_false << ";\n";
        }
    }

    cout << "\nConnected vertices true size: " << connected_vertices_true.size() << '\n';
    cout << "Connected vertices false size: " << connected_vertices_false.size() << '\n';

    file_true.close();
    file_false.close();
}
