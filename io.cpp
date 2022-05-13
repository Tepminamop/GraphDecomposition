#include "fm.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>

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
        if (rev_cnames.find(name) ==rev_cnames.end()) {
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
    unsigned count_related_vertices_true = 0;
    unsigned count_related_vertices_false = 0;
    for (unsigned idx = 0; idx < _nmap.size(); ++idx) {
        const Net* net = _nmap[idx];
        if (net->true_count() && net->false_count()) {
            ++cut_size;
            count_related_vertices_true += net->true_count();
            count_related_vertices_false += net->false_count();
        }
    }

    ss << "Cutsize: " << cut_size << "\n";
    ss << "Count related vertices from " << set1 << " subset: " << count_related_vertices_true << '\n';
    ss << "Count related vertices from " << set2 << " subset: " << count_related_vertices_false << '\n';
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
}
