#include <tuple>
#include <atomic>
#include <thread>
#include <vector>
#include <optional>
#include <iostream>
#include <boost/asio.hpp>

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

class gf {
private:
    size_t n, dif;
    ndim form;
    vector<v> graph;
    vector<int> size;
    vector<bool> cycle, is_dfs;
    vector<vector<op>> out_comp;
    array<array<array<int, 7>, 7>, 6> c{};
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
            if (i == begin || (!cl[i] && dfs_cyclic(graph, cl, i, begin)))
                return true;
        return false;
    }

    void find_cycle() {
        vector<bool> cl(n);
        for (int i = 0; i != n; ++i, cl.assign(n, false))
            cycle[i] = dfs_cyclic(graph, cl, i, i);
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
        do {
            form[find_index(strategy, players)] = find_outcome(strategy, cl, begin);
            cl.assign(n, false);
        } while (incr_g(strategy));
    }

    void dfs_comp_greater(array<array<int, 7>, 7> &comp, int cur, int priv) {
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

    void apply_cond_cur(array<array<int, 7>, 7> &comp, int a, int b) {
        comp[a][b] = -1;
        comp[b][a] = 1;
        dfs_comp_greater(comp, b, a);
    }

    void make_cond(array<array<int, 7>, 7> &comp, array<array<array<int, 7>, 7>, 6> &count,
                   vector<bool> &out_check, int player, int a, int b) {
        array<array<int, 7>, 7> copy(comp);
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

    bool is_ne_rec(array<array<array<int, 7>, 7>, 6> comp, array<array<array<int, 7>, 7>, 6> count,
                   int player, int a, int b, vector<bool> out_check) {
        make_cond(comp[player], count, out_check, player, a, b);
        while (incr_c(player, a, b)) {
            if ((count[player][a][b] != 0 || count[player][b][a] != 0) && comp[player][a][b] == 0) {
                if (!is_ne_rec(comp, count, player, a, b, out_check)) {
                    return false;
                }
                make_cond(comp[player], count, out_check, player, b, a);
            }
        }
        for (bool v : out_check)
            if (!v)
                return true;
        c = comp;
        return false;
    }

    void resize_comp(vector<vector<vector<int>>> &comp) {
        for (auto &matrix : comp) {
            matrix.resize(n + 1);
            for (auto &line : matrix) {
                line.resize(n + 1);
            }
        }
    }

    bool is_ne() {
        int player = 0, a = 0, b = 1;
        vector<bool> out_check(form.get_size());
        array<array<array<int, 7>, 7>, 6> comp{}, count{};
        for (int i = 0; i != dif; ++i)
            for (int j = 0; j != n + 1; ++j)
                for (int k = 0; k != n + 1; ++k)
                    count[i][j][k] = static_cast<int>(comp_out[i][j][k].size());
        for (int i = 0; i != dif; ++i)
            for (int j = 1; j != n + 1; ++j)
                make_cond(comp[i], count, out_check, player, 0, j);
        do {
            if ((count[player][a][b] != 0 || count[player][b][a] != 0) && comp[player][a][b] == 0) {
                if (!is_ne_rec(comp, count, player, a, b, out_check)) {
                    return false;
                }
                make_cond(comp[player], count, out_check, player, b, a);
            }
        } while (incr_c(player, a, b));
        for (bool v : out_check)
            if (!v)
                return true;
        c = comp;
        return false;
    }

    void write_data(vector<int> &players, int begin) {
        stringstream ss;
        for (int i = 0; i != dif; ++i) {
            for (int j = 0; j != n + 1; ++j) {
                for (int k = 0; k != n + 1; ++k) {
                    ss << c[i][j][k] << ' ';
                }
                ss << '\n';
            }
            ss << '\n';
        }
        ss << begin << "\n\n";
        for (int i = 0; i != n; ++i)
            ss << players[i] << ' ';
        ss << "\n\n";
        for (int i = 0; i != n; ++i) {
            for (int j : graph[i].out) {
                ss << j << ' ';
            }
            ss << '\n';
        }
        ss << '\n';
        cout << ss.str();
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

    void dfs_rec(vector<bool> &cl, int index) {
        cl[index] = true;
        for (int i : graph[index].out)
            if (!cl[i])
                dfs_rec(cl, i);
    }

    bool dfs(int begin) {
        vector<bool> cl(n);
        dfs_rec(cl, begin);
        for (int i = 0; i != n; ++i)
            if (!cl[i])
                return false;
        return true;
    }

public:
    explicit gf(vector<v> g) : n(g.size()), graph(move(g)), cycle(n), is_dfs(n), dif(0) {
        vector<int> players(n);
        find_cycle();
        for (int i = 0; i != n; ++i)
            is_dfs[i] = dfs(i);
        while (incr(players)) {
            if (check_players(players)) {
                for (int i = 0; i != n; ++i) {
                    if (!cycle[i] && is_dfs[i]) {
                        graph[i].has_t = false;
                        size.assign(dif, 1);
                        for (int j = 0; j != n; ++j) {
                            size[players[j]] *= graph[j].degree + (graph[j].has_t ? 1 : 0);
                        }
                        find_form(players, i);
                        resize();
                        make_data();
                        if (!is_ne()) {
                            write_data(players, i);
                        }
                        graph[i].has_t = true;
                    }
                }
            }
        }
    }
};

class g {
private:
    size_t n, num, all;
    boost::asio::thread_pool pool;
    vector<bool> perms_good;
    vector<vector<int>> perms, perms_back;
    vector<pair<vector<v>, vector<vector<int>>>> graphs_deg;

    bool next_monotonic(vector<int> &v) {
        ++v.back();
        int i;
        for (i = static_cast<int>(n - 1); i != 0 && v[i] == n; --i, ++v[i]) {}
        for (int j = i + 1; j != n; v[j] = v[i], ++j) {}
        return v.front() != n;
    }

    bool next_graph(vector<int> &v) {
        while (next_monotonic(v) && v.front() == 0) {}
        return v.front() != n;
    }

    void gen_subsets(vector<vector<int>> &ans, vector<vector<int>> &data, int len, int index,
                     vector<int> current) {
        for (int i = 0; i != data[index].size() && current.size() != len; ++i) {
            current.push_back(data[index][i]);
            if (current.size() != len) {
                for (int j = index + 1; j != data.size(); ++j) {
                    gen_subsets(ans, data, len, j, current);
                }
            }
        }
        if (current.size() == len)
            ans.push_back(move(current));
    }

    void dfs(vector<v> &graph, vector<bool> &cl, int index) {
        cl[index] = true;
        for (int i : graph[index].out)
            if (!cl[i])
                dfs(graph, cl, i);
        for (int i : graph[index].in)
            if (!cl[i])
                dfs(graph, cl, i);
    }

    bool dfs_cyclic(vector<v> &graph, vector<bool> &cl, int index) {
        cl[index] = true;
        for (int i : graph[index].out)
            if (cl[i] || dfs_cyclic(graph, cl, i))
                return true;
        return false;
    }

    bool check_connected(vector<v> &graph) {
        vector<bool> cl(n);
        dfs(graph, cl, 0);
        for (int i = 0; i != n; ++i)
            if (!cl[i])
                return false;
        return true;
    }

    bool check_cyclic(vector<v> &graph) {
        vector<bool> cl(n);
        for (int i = 0; i != n; ++i, cl.assign(n, false))
            if (dfs_cyclic(graph, cl, i))
                return true;
        return false;
    }

    bool check_graph(vector<v> &graph) {
        return check_connected(graph) && check_cyclic(graph);
    }

    void make_copy(vector<vector<int>> &copy, vector<v> &graph) {
        for (int i = 0; i != n; ++i) {
            copy[i].assign(n, 0);
            for (int j : graph[i].out) {
                copy[i][j] = 1;
            }
        }
    }

    void make_copy(vector<vector<int>> &copy, vector<v> &graph, int p) {
        for (int i = 0; i != n; ++i) {
            copy[i].assign(n, 0);
            for (int j : graph[perms[p][i]].out) {
                copy[i][perms_back[p][j]] = 1;
            }
        }
    }

    bool check_iso_cur(vector<v> &g1, pair<vector<v>, vector<vector<int>>> &g2) {
        bool check;
        vector<vector<int>> g1_copy(n);
        for (int i = 0; i != perms.size(); ++i) {
            if (perms_good[i]) {
                check = true;
                for (int j = 0; j != n; ++j) {
                    if (g1[perms[i][j]].in.size() != g2.first[j].in.size()) {
                        check = false;
                        break;
                    }
                }
                if (check) {
                    make_copy(g1_copy, g1, i);
                    if (g1_copy == g2.second) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool check_iso(vector<v> &g1) {
        vector<bool> in_degs1(n), in_degs2(n);
        for (int i = 0; i != n; ++i)
            in_degs1[g1[i].in.size()] = true;
        for (auto &g2 : graphs_deg) {
            for (int i = 0; i != n; ++i) {
                in_degs2[g2.first[i].in.size()] = true;
            }
            if (in_degs1 == in_degs2 && check_iso_cur(g1, g2)) {
                return true;
            }
            in_degs2.assign(n, false);
        }
        return false;
    }

    void find_gf(vector<v> &graph) {
        ++all;
        if (check_graph(graph) && !check_iso(graph)) {
            ++num;
            vector<vector<int>> copy(n);
            make_copy(copy, graph);
            graphs_deg.emplace_back(graph, move(copy));
            boost::asio::post(pool, [m = num, graph] {
                gf find_game(graph);
                if (m % 1000 == 0) {
                    stringstream ss;
                    ss << "Done: " << m << "\n\n";
                    cout << ss.str();
                }
            });
            if (num % 10000 == 0) {
                stringstream ss;
                ss << "Good: " << num << '\n' << "All: " << all << "\n\n";
                cout << ss.str();
            }
        }
    }

    vector<vector<int>> find_uniq(vector<v> &vertex, int pos) {
        bool check;
        vector<v> uniq;
        for (int i = 0; i != n; ++i) {
            if (i == pos)
                continue;
            check = true;
            for (int j = static_cast<int>(uniq.size()) - 1;
                 j != -1 && uniq[j].degree == vertex[i].degree; --j) {
                if (vertex[i] == uniq[j]) {
                    check = false;
                    break;
                }
            }
            if (check)
                uniq.push_back(vertex[i]);
        }
        vector<vector<int>> ans(uniq.size());
        for (int i = 0, j = 0; j != uniq.size(); ++j) {
            for (; i != n && vertex[i] == uniq[j]; ++i) {
                if (i != pos) {
                    ans[j].push_back(i);
                }
            }
            if (i == pos)
                ++i;
        }
        return ans;
    }

    void make_graph(vector<v> vertex, int pos) {
        if (pos != 0)
            for (int i : vertex[pos - 1].out)
                vertex[i].in.push_back(pos - 1);
        if (pos == n) {
            find_gf(vertex);
        } else {
            vector<vector<int>> uniq(find_uniq(vertex, pos));
            vector<vector<int>> perms;
            for (int i = 0; i != uniq.size(); ++i)
                gen_subsets(perms, uniq, vertex[pos].degree, i, vector<int>());
            for (auto &perm : perms) {
                vertex[pos].out = move(perm);
                make_graph(vertex, pos + 1);
            }
        }
    }

    void find_perms(vector<int> &deg) {
        bool check;
        perms_good.assign(perms.size(), false);
        for (int i = 0; i != perms.size(); ++i) {
            check = true;
            for (int j = 0; j != n; ++j) {
                if (deg[j] != deg[perms[i][j]]) {
                    check = false;
                    break;
                }
            }
            if (check) {
                perms_good[i] = true;
            }
        }
    }

    void all_graph(vector<int> &deg) {
        graphs_deg.resize(0);
        find_perms(deg);
        vector<v> vertex(n);
        for (int i = 0; i != n; ++i)
            vertex[i].degree = deg[i];
        make_graph(move(vertex), 0);
    }

    void all_perms_rec(vector<int> order, vector<bool> in, int deph) {
        if (deph == n) {
            perms.push_back(order);
        } else {
            for (int i = 0; i != n; ++i) {
                if (!in[i]) {
                    order[deph] = i;
                    in[i] = true;
                    all_perms_rec(order, in, deph + 1);
                    in[i] = false;
                }
            }
        }
    }

    void gen_perms() {
        all_perms_rec(vector<int>(n), vector<bool>(n), 0);
        perms_back.resize(perms.size());
        for (int i = 0; i != perms.size(); ++i) {
            perms_back[i].resize(n);
            for (int j = 0; j != n; ++j) {
                perms_back[i][perms[i][j]] = j;
            }
        }
    }

public:
    explicit g(size_t n) : n(n), num(0), all(0), pool(thread::hardware_concurrency()) {
        gen_perms();
    }

    void find_game() {
        vector<int> deg(n);
        while (next_graph(deg))
            all_graph(deg);
        pool.join();
        stringstream ss;
        ss << "Done " << n << " vertexes, total graphs " << num << '\n';
        cout << ss.str();
    }
};

int main() {
    g game(6);
    game.find_game();
}
