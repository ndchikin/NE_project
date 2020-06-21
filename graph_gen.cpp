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

class connect {
private:
    uint64_t n1, n2;
    vector<v> g1, g2;
    vector<int> c_count;
    vector<vector<int>> c_nums;

    bool next_monotonic(vector<int> &v) {
        ++v.back();
        int i = static_cast<int>(v.size() - 1);
        for (; i != 0 && v[i] == n2; --i, ++v[i]) {}
        for (int j = i + 1; j != v.size(); v[j] = v[i], ++j) {}
        return v.front() != n2;
    }

    bool not_all_dif(vector<int> &v) {
        vector<bool> count(n2);
        for (int i : v) {
            if (count[i]) {
                return true;
            } else {
                count[i] = true;
            }
        }
        return false;
    }

    bool next_con_nums(vector<int> &v) {
        while (next_monotonic(v) && not_all_dif(v)) {}
        return v.front() != n2;
    }

public:
    vector<v> g_cur;

    connect(vector<v> g1_p, vector<v> g2_p, vector<int> c_count_p) : n1(g1_p.size()), n2(g2_p.size()),
                                                                     c_count(move(c_count_p)), g1(move(g1_p)),
                                                                     g2(move(g2_p)),
                                                                     c_nums(g1.size()),
                                                                     g_cur(g1.size() + g2.size()) {
        for (int i = 0; i != n1; ++i) {
            g_cur[i] = g1[i];
            g_cur[i].degree += c_count[i];
            c_nums[i].resize(c_count[i]);
            if (not_all_dif(c_nums[i])) {
                next_con_nums(c_nums[i]);
            }
        }
        for (int i = 0; i != n2; ++i) {
            g_cur[n1 + i] = g2[i];
            for (int &j : g_cur[n1 + i].out) {
                j += n1;
            }
        }
        for (int i = 0; i != n1; ++i) {
            for (int j = 0; j != c_count[i]; ++j) {
                g_cur[i].out.push_back(n1 + c_nums[i][j]);
            }
        }
    }

    bool next() {
        for (int i = 0; i != n1; ++i) {
            for (int j = 0; j != c_count[i]; ++j) {
                g_cur[i].out.pop_back();
            }
        }
        for (int i = 0; i != n1; ++i) {
            if (!next_con_nums(c_nums[i])) {
                c_nums[i].assign(c_count[i], 0);
                if (not_all_dif(c_nums[i])) {
                    next_con_nums(c_nums[i]);
                }
            } else {
                for (int j = 0; j != n1; ++j) {
                    for (int k = 0; k != c_count[j]; ++k) {
                        g_cur[j].out.push_back(n1 + c_nums[j][k]);
                    }
                }
                if (c_nums[n1 - 1][0] != 0) { //only for simetric cycles
                    return false;
                }
                return true;
            }
        }
        return false;
    }
};

void read_graph(vector<v> &graph) {
    int size;
    if (!(cin >> size)) {
        exit(0);
    }
    graph.resize(size);
    for (int i = 0, num; i != size; ++i) {
        cin >> num;
        graph[i].degree = num;
        graph[i].out.resize(num);
        for (int j = 0; j != num; ++j) {
            cin >> graph[i].out[j];
        }
    }
}

void read_vector(vector<int> &v) {
    int size;
    if (!(cin >> size)) {
        exit(0);
    }
    v.resize(size);
    for (int i = 0; i != size; ++i) {
        cin >> v[i];
    }
}

void write_graph(vector<v> &graph) {
    cout << graph.size() << endl;
    for (int i = 0; i != graph.size(); ++i) {
        cout << graph[i].out.size();
        for (int j = 0; j != graph[i].out.size(); ++j) {
            cout << ' ' << graph[i].out[j];
        }
        cout << endl;
    }
}

void write_vector(vector<int> &v) {
    cout << v.size() << endl;
    for (int i = 0; i != v.size(); ++i) {
        cout << v[i] << ' ';
    }
    cout << endl;
}

int main() {
    vector<v> g1, g2;
    vector<int> c_count;
    read_graph(g1);
    read_graph(g2);
    read_vector(c_count);
    connect g_con(g1, g2, c_count);
    do {
        cout << g1.size() << endl;
        write_graph(g_con.g_cur);
    } while (g_con.next());
    return 0;
}
