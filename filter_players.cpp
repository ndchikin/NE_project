#include <vector>
#include <algorithm>
#include <serialization.h>

using namespace std;

class players_filter {
private:
    uint64_t n, n1;
    vector<v> graph;

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
        return true;
    }

public:
    vector<int> p_cur;

    players_filter(vector<v> graph_p, int n1) : n(graph_p.size()), n1(n1), graph(move(graph_p)), p_cur(n) {}

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

int main() {
    vector<v> g1, g2, graph;
    read_graph(g1);
    read_graph(g2);
    graph.resize(g1.size() + g2.size());
    for (size_t i = 0; i != g1.size(); ++i) {
        graph[i] = g1[i];
    }
    for (size_t i = 0; i != g2.size(); ++i) {
        graph[g1.size() + i] = g2[i];
        for (int &j : graph[g1.size() + i].out) {
            j += g1.size();
        }
    }
    players_filter p_filter(graph, g1.size());
    write_graph(g1);
    write_graph(g2);
    if (p_filter.init()) {
        do {
            write_vector(p_filter.p_cur);
        } while (p_filter.next());
    }
    return 0;
}
