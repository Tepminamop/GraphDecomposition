#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <set>

#include "buckets.h"
#include "cells.h"
#include "nets.h"

class FloorPlan {
   public:
    FloorPlan();
    FloorPlan(std::vector<Net*>& nmap, std::vector<Cell*>& cmap);
    FloorPlan(std::vector<Net*>&& nmap, std::vector<Cell*>&& cmap);
    ~FloorPlan();

    void fm();

    void nmap(std::vector<Net*>&& nmap);
    std::vector<Net*>& nmap();
    const std::vector<Net*>& nmap() const;

    void cmap(std::vector<Cell*>&& cmap);
    std::vector<Cell*>& cmap();
    const std::vector<Cell*>& cmap() const;

    void input(const std::string fname);
    void output(const std::string name, const int set1, const int set2, const int subset1, const int subset2);
    void output_to_decomposite(const std::string fname1, const std::string fname2, const int set1, const int set2, const int subset1, const int subset2, 
        std::set<unsigned int>& connected_vertices_true, std::set<unsigned int>& connected_vertices_false, const unsigned int size, const unsigned int conn_size_true, const unsigned int conn_size_false);

    void output_to_decomposite_without_connection_control(const std::string fname1, const std::string fname2, const int set1, const int set2, const int subset1, const int subset2,
        std::set<unsigned int>& connected_vertices_true, std::set<unsigned int>& connected_vertices_false, std::set<unsigned int>& check_vertices, bool check_side);

    double balance() const;

    template <bool soph>
    void init_side();

    void tolerate(unsigned amount);

    void set_connected_vertices(std::set<int>& connected_vertices) {
        this->_connected_vertices = connected_vertices;
    }

    void get_true_false_count(unsigned int& true_count, unsigned int& false_count);

   private:
    std::set<int> _connected_vertices;
    std::vector<Net*> _nmap;
    std::vector<Cell*> _cmap;
    std::vector<std::string> _nnames;
    std::vector<std::string> _cnames;
    Bucket _bucket;
    double _balance;
    unsigned _tcount;
    unsigned _tolerate;

    unsigned naiv_init_side();
    unsigned soph_init_side();

    void init_gains();
    void check_gains();
    void init_bucket();

    template <bool check>
    void cal_gains();

    int fm_once(std::function<bool(const unsigned)> condition);

    int flip(Bucket& nbucket,
             const std::unordered_set<unsigned>& seen,
             unsigned cname);
};

template <bool soph>
void FloorPlan::init_side() {
    if (soph) {
        _tcount = soph_init_side();
    } else {
        _tcount = naiv_init_side();
    }
}
