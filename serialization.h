#ifndef NAUTY_SERIALIZATION_H
#define NAUTY_SERIALIZATION_H

#include <tuple>
#include <vector>
#include <iostream>

using namespace std;

struct v {
    int degree;
    bool has_t = true;
    vector<int> in;
    vector<int> out;

    bool operator==(v &other) {
        return tie(degree, in, out) == tie(other.degree, other.in, other.out);
    }

    bool operator!=(v &other) {
        return tie(degree, in, out) != tie(other.degree, other.in, other.out);
    }
};

void read_size(size_t &num) {
    if (!(cin >> num)) {
        exit(0);
    }
}


void read_vector(vector<int> &v) {
    size_t size;
    read_size(size);
    v.resize(size);
    for (size_t i = 0; i != size; ++i) {
        cin >> v[i];
    }
}

void read_graph(vector<v> &graph) {
    size_t size;
    read_size(size);
    graph.resize(size);
    for (size_t i = 0, num; i != size; ++i) {
        read_vector(graph[i].out);
        graph[i].degree = graph[i].out.size();
    }
}

void write_size(const size_t num) {
    cout << num << endl;
}

void write_vector(const vector<int> &v) {
    write_size(v.size());
    for (size_t i = 0; i != v.size(); ++i) {
        cout << v[i] << ' ';
    }
    cout << endl;
}

void write_graph(const vector<v> &graph) {
    write_size(graph.size());
    for (size_t i = 0; i != graph.size(); ++i) {
        write_vector(graph[i].out);
    }
}

#endif //NAUTY_SERIALIZATION_H
