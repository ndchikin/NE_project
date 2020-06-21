#include <tuple>
#include <vector>
#include <iostream>
#include <algorithm>


using namespace std;


struct op {
    uint8_t player, a, b;


    op() = default;


    op(uint8_t player, uint8_t a, uint8_t b) : player(player), a(a), b(b) {}
};

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

class ndim {
private:
    uint64_t total_size;
    vector<uint64_t> size, offset;

public:
    vector<int8_t> data;


    ndim() : total_size(0) {}


    explicit ndim(vector<uint64_t> &s) : size(s), offset(s.size()) {
        total_size = 1;
        for (int i = static_cast<int>(size.size() - 1); i >= 0; --i) {
            offset[i] = total_size;
            total_size *= size[i];
        }
        data.resize(total_size);
    }


    uint64_t get_size() {
        return total_size;
    }


    uint64_t find_index(vector<uint64_t> &num) {
        uint64_t index = 0, t = total_size;
        for (int i = 0; i != size.size(); ++i) {
            t /= size[i];
            index += t * num[i];
        }
        return index;
    }


    int8_t &operator[](vector<uint64_t> &num) {
        return data[find_index(num)];
    }


    int8_t &operator[](vector<uint64_t> &&num) {
        return data[find_index(num)];
    }


    void
    find_cases(vector<ndim> &forms, vector<uint64_t> &index, vector<uint8_t> &outcomes, vector<vector<op>> &out_comp) {
        uint64_t id = find_index(index);
        int8_t a = data[id];
        for (uint64_t i = 0, it; i != size.size(); ++i) {
            it = id - offset[i] * index[i];
            for (int j = 0; j != size[i]; ++j, it += offset[i]) {
                for (int k = 0; k != forms.size(); ++k) {
                    if (forms[k].data[it] != a) {
                        outcomes[forms[k].data[it] + 1] = 1;
                    }
                }
            }
            for (int j = 0; j != outcomes.size(); ++j) {
                if (outcomes[j]) {
                    out_comp[id].emplace_back(i, a + 1, j);
                    outcomes[j] = 0;
                }
            }
        }
    }
};

class gf {
private:

    uint64_t n, dif;
    vector<v> graph;
    vector<int> players;
    vector<uint64_t> size;
    vector<vector<vector<op>>> out_comp;


    bool incr_s(vector<uint64_t> &v) {
        ++v.back();
        for (int i = static_cast<int>(dif); i != 1; --i) {
            if (v[i - 1] == size[i - 1]) {
                v[i - 1] = 0, ++v[i - 2];
            }
        }
        return v.front() != size.front();
    }


    bool incr_g(vector<int> &v) {
        ++v.back();
        for (int i = static_cast<int>(n); i != 1; --i) {
            if (v[i - 1] == graph[i - 1].degree + (graph[i - 1].has_t ? 1 : 0)) {
                v[i - 1] = 0, ++v[i - 2];
            }
        }
        return v.front() != graph.front().degree + (graph.front().has_t ? 1 : 0);
    }


    int8_t find_outcome(vector<int> &strategy, vector<int> &cl, int begin) {
        if (graph[begin].degree == strategy[begin]) {
            return static_cast<int8_t>(begin);
        } else if (cl[graph[begin].out[strategy[begin]]]) {
            return -1;
        } else {
            cl[begin] = 1;
            return find_outcome(strategy, cl, graph[begin].out[strategy[begin]]);
        }
    }


    vector<uint64_t> find_index(vector<int> &strategy) {
        vector<uint64_t> ans(size.size()), s(size);
        for (int j = 0; j != n; ++j) {
            s[players[j]] /= graph[j].degree + (graph[j].has_t ? 1 : 0);
            ans[players[j]] += s[players[j]] * strategy[j];
        }
        return ans;
    }


    int toVar(int player, int a, int b) {
        return static_cast<int>((player * (n + 1) * n) / 2 + (a * (2 * n - a - 1)) / 2 + b);
    }


    int toLit(int player, int a, int b) {
        if (a < b) {
            return toVar(player, a, b);
        } else {
            return -toVar(player, b, a);
        }
    }

public:
    gf(vector<v> g, vector<int> pl) : n(g.size()), dif(0), graph(move(g)), players(move(pl)) {
        vector<int> count(n);
        for (int i = 0; i != n; ++i) {
            if (!count[players[i]]) {
                ++dif;
                count[players[i]] = 1;
            }
        }
    }

    void make_cnf() {
        int beg;
        vector<int> strategy, cl(n);
        vector<uint8_t> outcomes(n + 1);
        vector<uint64_t> index;
        vector<ndim> forms(n);
        out_comp.resize(n);
        size.assign(dif, 1);
        for (int i = 0; i != n; ++i) {
            size[players[i]] *= graph[i].degree + (graph[i].has_t ? 1 : 0);
        }
        for (int i = 0; i != n; ++i) {
            forms[i] = ndim(size);
            strategy.assign(n, 0);
            do {
                forms[i][find_index(strategy)] = find_outcome(strategy, cl, i);
                cl.assign(n, 0);
            } while (incr_g(strategy));
        }
        for (int i = 0; i != n; ++i) {
            index.assign(dif, 0);
            out_comp[i].resize(forms[i].get_size());
            do {
                forms[i].find_cases(forms, index, outcomes, out_comp[i]);
            } while (incr_s(index));
        }
    }

    void write_cnf() {
        vector<vector<int>> result;
        for (int i = 0; i != dif; ++i) {
            for (int j = 0; j != n + 1; ++j) {
                for (int k = j + 1; k != n + 1; ++k) {
                    for (int l = k + 1; l != n + 1; ++l) {
                        result.push_back({toLit(i, k, j), toLit(i, l, k), toLit(i, j, l)});
                        result.push_back({toLit(i, j, k), toLit(i, k, l), toLit(i, l, j)});
                    }
                }
            }
        }
        for (int i = 0; i != n; ++i) {
            for (uint64_t j = 0; j != out_comp[i].size(); ++j) {
                if (!out_comp[i][j].empty()) {
                    result.emplace_back();
                    for (uint64_t k = 0; k != out_comp[i][j].size(); ++k) {
                        result.back().push_back(
                                toLit(out_comp[i][j][k].player, out_comp[i][j][k].a, out_comp[i][j][k].b));
                    }
                    sort(result.back().begin(), result.back().end());
                }
            }
        }
        sort(result.begin(), result.end());
        result.erase(unique(result.begin(), result.end()), result.end());
        cout << result.size() << endl;
        for (auto &clause : result) {
            for (int lit : clause) {
                cout << lit << ' ';
            }
            cout << endl;
        }
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
    vector<v> graph;
    vector<int> players;
    while (true) {
        read_graph(graph);
        read_vector(players);
        gf PPH(graph, players);
        PPH.make_cnf();
        write_graph(graph);
        write_vector(players);
        PPH.write_cnf();
    }
}
