#include <tuple>
#include <vector>
#include <fstream>
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

class players_gen {
private:
    uint64_t n, n1;
    vector<v> graph;
    vector<vector<int>> players;

    bool incr() {
        ++p_cur.back();
        for (int i = static_cast<int>(n); i != 1; --i) {
            if (p_cur[i - 1] == n) {
                p_cur[i - 1] = 0, ++p_cur[i - 2];
            } else {
                break;
            }
        }
        for (int i = 0; i != n - 2; ++i) {
            if (p_cur[i] != i) {
                return true;
            }
        }
        return p_cur[n - 2] != n - 1 || p_cur[n - 1] != 0;
    }

    void check_p_order() {
        int min_p = -1, max_p = 0;
        for (int i = 0; i != n; ++i) {
            max_p = max(max_p, p_cur[i]);
            if (min_p + 1 < max_p) {
                p_cur[i] = 0;
                ++p_cur[i - 1];
                min_p = -1;
                max_p = 0;
                i = -1;
            } else {
                min_p = max_p;
            }
        }
    }

    bool next_players() {
        if (!incr()) {
            return false;
        } else {
            check_p_order();
            return true;
        }
    }

    vector<int> norm_players(int size) {
        int dif = 0;
        bool check = true;
        vector<int> players_norm(p_cur.end() - size, p_cur.end());
        vector<int> mapping(p_cur.size(), -1);
        while (check) {
            check = false;
            for (int i = 0; i != players_norm.size(); ++i) {
                if (mapping[players_norm[i]] == -1) {
                    mapping[players_norm[i]] = dif;
                    check = true;
                    ++dif;
                }
            }
        }
        for (int i = 0; i != players_norm.size(); ++i) {
            players_norm[i] = mapping[players_norm[i]];
        }
        return players_norm;
    }

    bool check_template() {
        auto p_cur_norm = norm_players(players.front().size());
        for (auto &p : players) {
            if (p == p_cur_norm) {
                return true;
            }
        }
        return false;
    }

    bool check_players() {
        int dif = 0;
        int a_dif = 0;
        bool overlap = false;
        vector<int> count(n), on_cycle(n), a_cycle(n);
        for (int i = 0; i != n; ++i) {
            if (count[p_cur[i]] == 0) {
                ++dif;
            }
            ++count[p_cur[i]];
            if (i >= n1) {
                if (on_cycle[p_cur[i]] == -1) {
                    overlap = true;
                } else {
                    on_cycle[p_cur[i]] = 1;
                }
            } else {
                if (a_cycle[p_cur[i]] == 0) {
                    ++a_dif;
                }
                ++a_cycle[p_cur[i]];
                if (on_cycle[p_cur[i]] == 1) {
                    overlap = true;
                } else {
                    on_cycle[p_cur[i]] = -1;
                }
            }
        }
        if (dif < 3 || a_dif < 2 || !overlap) {
            return false;
        }
        for (int i = 0; i != n; ++i) {
            for (int j : graph[i].out) {
                if (p_cur[i] == p_cur[j] && (((i >= n1) && (j >= n1)) || ((i < n1) && (j < n1)))) {
                    return false;
                }
            }
        }
        return check_template();
    }

public:
    vector<int> p_cur;

    players_gen(vector<v> graph_p, int n1, vector<vector<int>> players_p) : n(graph_p.size()), n1(n1),
                                                                            graph(move(graph_p)), p_cur(n),
                                                                            players(move(players_p)) {}

    bool init() {
        while (!check_players()) {
            if (!next_players()) {
                return false;
            }
        }
        return true;
    }

    bool next() {
        do {
            if (!next_players()) {
                return false;
            }
        } while (!check_players());
        return true;
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
    int n1, size;
    vector<v> graph;
    vector<vector<int>> players;
    fstream orders("orders.txt", ios::in);
    while (orders >> size) {
        players.emplace_back(size);
        for (int i = 0; i != size; ++i) {
            orders >> players.back()[i];
        }
    }
    while (true) {
        cin >> n1;
        read_graph(graph);
        players_gen p_gen(graph, n1, players);
        if (p_gen.init()) {
            do {
                for (int i = 0; i != n1; ++i) {
                    write_graph(graph);
                    write_vector(p_gen.p_cur);
                    cout << i << endl;
                }
            } while (p_gen.next());
        }
    }
}
