#include <tuple>
#include <vector>
#include <optional>
#include <iostream>

using namespace std;

struct op {
    int player, a, b;

    op() = default;

    op(int player, int a, int b) : player(player), a(a), b(b) {}
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
    int total_size;
    vector<int> data, size, offset;

public:

    ndim() : total_size(0) {}

    explicit ndim(vector<int> &s) : size(s), offset(s.size()) {
        total_size = 1;
        for (int i = static_cast<int>(size.size() - 1); i >= 0; --i) {
            offset[i] = total_size;
            total_size *= size[i];
        }
        data.resize(static_cast<size_t>(total_size));
    }

    size_t get_size() {
        return static_cast<size_t>(total_size);
    }

    int find_index(vector<int> &num) {
        int index = 0, t = total_size;
        for (int i = 0; i != size.size(); ++i) {
            t /= size[i];
            index += t * num[i];
        }
        return index;
    }

    int &operator[](vector<int> &num) {
        return data[find_index(num)];
    }

    int &operator[](vector<int> &&num) {
        return data[find_index(num)];
    }

    int find_cases(vector<int> &index, vector<vector<int>> &ans) {
        int t = total_size, id = find_index(index), a = data[id];
        for (int i = 0, it; i != size.size(); ++i) {
            it = id - offset[i] * index[i];
            for (int j = 0; j != size[i]; ++j, it += offset[i]) {
                if (data[it] != a) {
                    ans[i][data[it] + 1] = 1;
                }
            }
        }
        return a + 1;
    }
};

template<size_t n>
class gf {
private:
    size_t dif, num = 0;
    ndim form;
    vector<v> graph;
    vector<int> size;
    vector<bool> cycle;
    vector<vector<op>> out_comp;
    int c[n][n + 1][n + 1];
    vector<vector<vector<vector<int>>>> comp_out;

    bool incr(vector<int> &v) {
        ++v.back();
        for (int i = static_cast<int>(n); i != 1; --i)
            if (v[i - 1] == n)
                v[i - 1] = 0, ++v[i - 2];
        for (int i = 0; i != n - 2; ++i)
            if (v[i] != i)
                return true;
        return v[n - 2] != n - 1 || v[n - 1] != 0;
    }

    bool incr_s(vector<int> &v) {
        ++v.back();
        for (int i = static_cast<int>(dif); i != 1; --i)
            if (v[i - 1] == size[i - 1])
                v[i - 1] = 0, ++v[i - 2];
        return v.front() != size.front();
    }

    bool incr_g(vector<int> &v) {
        ++v.back();
        for (int i = static_cast<int>(n); i != 1; --i)
            if (v[i - 1] == graph[i - 1].degree + (graph[i - 1].has_t ? 1 : 0))
                v[i - 1] = 0, ++v[i - 2];
        return v.front() != graph.front().degree + (graph.front().has_t ? 1 : 0);
    }

    bool incr_c(int &player, int &a, int &b) {
        ++b;
        if (b == n + 1) {
            ++a, b = a + 1;
            if (a == n) {
                ++player, a = 0, b = 1;
                if (player == dif) {
                    return false;
                }
            }
        }
        return true;
    }

    bool dfs_cyclic(vector<v> &graph, vector<bool> &cl, int index, int begin) {
        cl[index] = true;
        for (int i : graph[index].out)
            if ((cl[i] == true && i == begin) || (!cl[i] && dfs_cyclic(graph, cl, i, begin)))
                return true;
        return false;
    }

    void find_cycle() {
        int begin, end;
        vector<bool> cl(n);
        for (int i = 0; i != n; ++i, cl.assign(n, false))
            if (dfs_cyclic(graph, cl, i, i))
                cycle[i] = true;
    }

    bool check_players(vector<int> &players) {
        dif = 0;
        bool overlap = false;
        vector<int> count(n), on_cycle(n);
        for (int i = 0; i != n; ++i) {
            if (count[players[i]] == 0) {
                if (players[i] != dif) {
                    return false;
                }
                ++dif;
            }
            if (cycle[i]) {
                if (on_cycle[players[i]] == -1) {
                    overlap = true;
                } else {
                    on_cycle[players[i]] = 1;
                }
            } else {
                if (on_cycle[players[i]] == 1) {
                    overlap = true;
                } else {
                    on_cycle[players[i]] = -1;
                }
            }
            ++count[players[i]];
        }
        if (dif < 3 || !overlap)
            return false;
        for (int i = 0; i != n; ++i)
            for (int j : graph[i].out)
                if (players[i] == players[j] && cycle[i] == cycle[j])
                    return false;
        return true;
    }

    int find_outcome(vector<int> &strategy, vector<bool> &cl, int begin) {
        if (graph[begin].degree == strategy[begin])
            return begin;
        if (cl[graph[begin].out[strategy[begin]]])
            return -1;
        cl[begin] = true;
        return find_outcome(strategy, cl, graph[begin].out[strategy[begin]]);
    }

    vector<int> find_index(vector<int> &strategy, vector<int> &players) {
        vector<int> ans(size.size()), s(size);
        for (int i = 0; i != dif; ++i) {
            for (int j = 0; j != n; ++j) {
                if (players[j] == i) {
                    s[i] /= graph[j].degree + (graph[j].has_t ? 1 : 0);
                    ans[i] += s[i] * strategy[j];
                }
            }
        }
        return ans;
    }

    void find_form(vector<int> &players, int begin) {
        form = ndim(size);
        vector<int> strategy(n);
        vector<bool> cl(n);
        form[find_index(strategy, players)] = find_outcome(strategy, cl, begin);
        while (incr_g(strategy)) {
            cl.assign(n, false);
            form[find_index(strategy, players)] = find_outcome(strategy, cl, begin);
        }
    }

    void dfs_comp_greater(int comp[n + 1][n + 1], int cur, int priv) {
        for (int i = 0; i != n + 1; ++i) {
            if (comp[priv][i] == 1) {
                comp[cur][i] = 1;
                comp[i][cur] = -1;
            }
        }
        for (int i = 0; i != n + 1; ++i)
            if (comp[i][cur] == 1)
                dfs_comp_greater(comp, i, cur);
    }

    void apply_cond_cur(int comp[n + 1][n + 1], int a, int b) {
        comp[a][b] = -1;
        comp[b][a] = 1;
        dfs_comp_greater(comp, b, a);
    }

    void make_cond(int comp[n + 1][n + 1], int count[][n + 1][n + 1], bool out_check[], int player,
                   int a, int b) {
        int copy[n + 1][n + 1];
        std::copy(&comp[0][0], &comp[0][0] + (n + 1) * (n + 1), &copy[0][0]);
        apply_cond_cur(comp, a, b);
        for (int i = 0; i != n + 1; ++i) {
            for (int j = i + 1; j != n + 1; ++j) {
                if (copy[i][j] == 0 && comp[i][j] != 0) {
                    if (comp[i][j] == 1)
                        swap(i, j);
                    for (int outcome : comp_out[player][i][j]) {
                        if (!out_check[outcome]) {
                            for (auto &p : out_comp[outcome]) {
                                --count[p.player][p.a][p.b];
                            }
                            out_check[outcome] = true;
                        }
                    }
                    if (j < i)
                        swap(i, j);
                }
            }
        }
    }

    bool
    is_ne_rec(int comp_arg[][n + 1][n + 1], int count_arg[][n + 1][n + 1], int player, int a, int b,
              bool out_check_arg[]) {
        ++num;
        bool out_check[form.get_size()];
        int comp[dif][n + 1][n + 1], count[dif][n + 1][n + 1];
        copy(out_check_arg, out_check_arg + form.get_size(), out_check);
        copy(&comp_arg[0][0][0], &comp_arg[0][0][0] + dif * (n + 1) * (n + 1), &comp[0][0][0]);
        copy(&count_arg[0][0][0], &count_arg[0][0][0] + dif * (n + 1) * (n + 1), &count[0][0][0]);
        make_cond(comp[player], count, out_check, player, a, b);
        while (incr_c(player, a, b)) {
            if ((count[player][a][b] != 0 || count[player][b][a] != 0) && comp[player][a][b] == 0) {
                if (!is_ne_rec(comp, count, player, a, b, out_check)) {
                    return false;
                }
                make_cond(comp[player], count, out_check, player, b, a);
            }
        }
        for (int i = 0; i != form.get_size(); ++i)
            if (!out_check[i])
                return true;
        copy(&comp[0][0][0], &comp[0][0][0] + dif * (n + 1) * (n + 1), &c[0][0][0]);
        return true;
    }

    void resize_comp(vector<vector<vector<int>>> &comp) {
        for (auto &matrix : comp) {
            matrix.resize(n + 1);
            for (auto &line : matrix) {
                line.resize(n + 1);
            }
        }
    }

    void init(int comp[][n + 1][n + 1], int count[][n + 1][n + 1], bool out_check[]) {
        for (int i = 0; i != dif; ++i) {
            for (int j = 0; j != n + 1; ++j) {
                for (int k = 0; k != n + 1; ++k) {
                    comp[i][j][k] = 0;
                    count[i][j][k] = static_cast<int>(comp_out[i][j][k].size());
                }
            }
        }
        for (int i = 0; i != form.get_size(); ++i)
            out_check[i] = false;
    }

    bool is_ne() {
        bool out_check[form.get_size()];
        int comp[dif][n + 1][n + 1], count[dif][n + 1][n + 1];
        int player = 0, a = 0, b = 1;
        init(comp, count, out_check);
        do {
            if ((count[player][a][b] != 0 || count[player][b][a] != 0) && comp[player][a][b] == 0) {
                if (!is_ne_rec(comp, count, player, a, b, out_check)) {
                    return false;
                }
                make_cond(comp[player], count, out_check, player, b, a);
            }
        } while (incr_c(player, a, b));
        for (int i = 0; i != form.get_size(); ++i)
            if (!out_check[i])
                return true;
        copy(&comp[0][0][0], &comp[0][0][0] + dif * (n + 1) * (n + 1), &c[0][0][0]);
        return true;
    }

    void write_data(vector<int> &players, int begin) {
        for (int i = 0; i != dif; ++i) {
            for (int j = 0; j != n + 1; ++j) {
                for (int k = 0; k != n + 1; ++k) {
                    cout << c[i][j][k] << ' ';
                }
                cout << '\n';
            }
            cout << '\n';
        }
        cout << '\n' << begin << "\n\n";
        for (int i = 0; i != n; ++i)
            cout << players[i] << ' ';
        cout << "\n\n";
        for (int i = 0; i != n; ++i) {
            for (int j : graph[i].out) {
                cout << j << ' ';
            }
            cout << '\n';
        }
        cout << '\n';
    }

    void resize() {
        out_comp.resize(form.get_size());
        for (auto &v : out_comp)
            v.resize(0);
        comp_out.resize(0);
        comp_out.resize(dif);
        for (auto &matrix : comp_out) {
            matrix.resize(n + 1);
            for (auto &line : matrix) {
                line.resize(n + 1);
            }
        }
    }

    void make_data() {
        int id, outcome;
        vector<vector<int>> ans(dif);
        for (auto &line : ans)
            line.resize(n + 1);
        vector<int> index(dif);
        do {
            id = form.find_index(index);
            outcome = form.find_cases(index, ans);
            for (int i = 0; i != dif; ++i) {
                for (int j = 0; j != n + 1; ++j) {
                    if (ans[i][j]) {
                        out_comp[id].emplace_back(i, outcome, j);
                        comp_out[i][outcome][j].push_back(id);
                    }
                }
            }
            for (auto &line : ans)
                line.assign(n + 1, 0);
        } while (incr_s(index));
    }

public:
    gf(vector<v> &g) : graph(move(g)) {
        dif = 4;
        vector<int> players = {2, 2, 1, 3, 1, 0, 1, 0};
        size.assign(dif, 1);
        for (int j = 0; j != n; ++j)
            size[players[j]] *= graph[j].degree + (graph[j].has_t ? 1 : 0);
        find_form(players, 5);
        resize();
        make_data();
        if (!is_ne()) {
            write_data(players, 5);
        }
    }

    ~gf() {
        cout << num;
    }
};

int main() {
    vector<v> graph(8);
    graph[0].out = {1};
    graph[1].out = {2};
    graph[2].out = {3};
    graph[3].out = {4};
    graph[4].out = {0};
    graph[5].out = {0, 6};
    graph[6].out = {1, 7};
    graph[7].out = {2, 3, 4};
    for (int i = 0; i != 8; ++i)
        graph[i].degree = graph[i].out.size();
    graph[5].has_t = graph[6].has_t = graph[7].has_t = false;
    gf<8> game(graph);
}
