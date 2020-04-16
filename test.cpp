#include <tuple>
#include <thread>
#include <vector>
#include <fstream>
#include <iostream>
#include "glucose.hpp"
#include <boost/asio.hpp>

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
    uint32_t total_size;
    vector<int> size, offset;
    vector<int8_t> data;

public:

    ndim() : total_size(0) {}

    explicit ndim(vector<int> &s) : size(s), offset(s.size()) {
        total_size = 1;
        for (auto i = static_cast<int8_t>(size.size() - 1); i >= 0; --i) {
            offset[i] = total_size;
            total_size *= size[i];
        }
        data.resize(static_cast<size_t>(total_size));
    }

    uint32_t get_size() {
        return total_size;
    }

    uint32_t find_index(vector<int> &num) {
        uint32_t index = 0, t = total_size;
        for (int i = 0; i != size.size(); ++i) {
            t /= size[i];
            index += t * num[i];
        }
        return index;
    }

    int8_t &operator[](vector<int> &num) {
        return data[find_index(num)];
    }

    int8_t &operator[](vector<int> &&num) {
        return data[find_index(num)];
    }

    uint8_t find_cases(vector<int> &index, vector<vector<int>> &ans) {
        uint32_t id = find_index(index);
        int8_t a = data[id];
        for (int i = 0, it; i != size.size(); ++i) {
            it = id - offset[i] * index[i];
            for (int j = 0; j != size[i]; ++j, it += offset[i]) {
                if (data[it] != a) {
                    ans[i][data[it] + 1] = 1;
                }
            }
        }
        return static_cast<uint8_t>(a + 1);
    }
};

template<size_t n>
class gf {
private:
    bool is_iso = false;
    size_t dif, num;
    ndim form;
    vector<v> graph;
    vector<int> size, good_perm;
    vector<bool> cycle, is_dfs;
    vector<vector<op>> out_comp;
    vector<vector<int>> &perms, &perms_back;
    vector<pair<vector<int>, pair<int, vector<int>>>> good_player;

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
        int a_dif = 0;
        bool overlap = false;
        vector<int> count(n), on_cycle(n), a_cycle(n);
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
                if (a_cycle[players[i]] == 0)
                    ++a_dif;
                ++a_cycle[players[i]];
                if (on_cycle[players[i]] == 1) {
                    overlap = true;
                } else {
                    on_cycle[players[i]] = -1;
                }
            }
            ++count[players[i]];
        }
        if (dif < 3 || a_dif < 2 || !overlap)
            return false;
        for (int i = 0; i != n; ++i)
            for (int j : graph[i].out)
                if (players[i] == players[j] && cycle[i] == cycle[j])
                    return false;
        return true;
    }

    int8_t find_outcome(vector<int> &strategy, vector<bool> &cl, int begin) {
        if (graph[begin].degree == strategy[begin])
            return static_cast<int8_t>(begin);
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

    int toVar(int player, int a, int b) {
        return static_cast<int>(player * (n + 1) * n / 2 + a * (2 * n - a - 1) / 2 + b);
    }

    Glucose::Lit toLit(int player, int a, int b) {
        if (a < b) {
            return Glucose::mkLit(toVar(player, a, b));
        } else {
            return ~Glucose::mkLit(toVar(player, b, a));
        }
    }

    void addTr(int player, int a, int b, int c, Glucose::Solver &solver) {
        solver.addClause(toLit(player, b, a), toLit(player, c, b), toLit(player, a, c));
        solver.addClause(toLit(player, a, b), toLit(player, b, c), toLit(player, c, a));
    }

    bool is_ne() {
        Glucose::Solver solver;
        Glucose::vec<Glucose::Lit> clause;
        for (int i = 0; i != dif; ++i)
            for (int j = 0; j != n + 1; ++j)
                for (int k = j + 1; k != n + 1; ++k)
                    solver.newVar();
        for (int i = 0; i != dif; ++i)
            for (int j = 1; j != n + 1; ++j)
                solver.addClause(toLit(i, 0, j));
        for (int i = 0; i != dif; ++i)
            for (int j = 0; j != n + 1; ++j)
                for (int k = j + 1; k != n + 1; ++k)
                    for (int l = k + 1; l != n + 1; ++l)
                        addTr(i, j, k, l, solver);
        for (auto &c : out_comp) {
            if (!c.empty()) {
                for (auto &p : c) {
                    clause.push(toLit(p.player, p.a, p.b));
                }
                solver.addClause(clause);
                clause.clear();
            }
        }
        return solver.solve();
    }

    void resize() {
        out_comp.resize(form.get_size());
        for (auto &v : out_comp)
            v.resize(0);
    }

    void make_data() {
        uint32_t id;
        uint8_t outcome;
        vector<vector<int>> ans(dif);
        for (auto &line : ans)
            line.resize(n + 1);
        vector<int> index(dif);
        do {
            id = form.find_index(index);
            outcome = form.find_cases(index, ans);
            for (uint8_t i = 0; i != dif; ++i) {
                for (uint8_t j = 0; j != n + 1; ++j) {
                    if (ans[i][j]) {
                        out_comp[id].emplace_back(i, outcome, j);
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

    void find_automorf() {
        bool check;
        vector<vector<int>> copy(n), p_copy(n);
        make_copy(copy, graph);
        for (int i = 1; i != perms.size(); ++i) {
            check = true;
            for (int j = 0; j != n; ++j) {
                if (graph[j].out.size() != graph[perms[i][j]].out.size() ||
                    graph[j].in.size() != graph[perms[i][j]].in.size()) {
                    check = false;
                    break;
                }
            }
            if (check) {
                make_copy(p_copy, graph, i);
                if (p_copy == copy) {
                    is_iso = true;
                    good_perm.push_back(i);
                }
            }
        }
    }

    bool check_players_iso(vector<int> &players) {
        bool check;
        if (!is_iso)
            return false;
        vector<int> count(n), rename(n);
        pair<int, vector<int>> print;
        print.second.resize(n);
        for (int i = 0; i != n; ++i) {
            if (count[players[i]] == 0)
                ++print.first;
            ++count[players[i]];
        }
        for (int i = 0; i != n; ++i) {
            ++print.second[count[i]];
        }
        for (auto &p : good_player) {
            if (print == p.second) {
                for (int i : good_perm) {
                    check = true;
                    rename.assign(n, -1);
                    for (int j = 0; j != n; ++j) {
                        if (rename[players[perms[i][j]]] == -1) {
                            rename[players[perms[i][j]]] = p.first[j];
                        } else if (rename[players[perms[i][j]]] != p.first[j]) {
                            check = false;
                            break;
                        }
                    }
                    if (check) {
                        return true;
                    }
                }
            }
        }
        good_player.emplace_back(players, print);
        return false;
    }

public:
    gf(vector<v> g, size_t num, vector<vector<int>> &p,
       vector<vector<int>> &p_b) : dif(0), num(num), graph(move(g)), cycle(n), is_dfs(n),
                                   perms(p), perms_back(p_b) {
        int a_count = 0;
        bool check = true;
        vector<int> players(n);
        find_automorf();
        find_cycle();
        for (int i = 0; i != n; ++i)
            if (!cycle[i])
                ++a_count;
        if (a_count < 2)
            check = false;
        for (int i = 0; i != n; ++i)
            is_dfs[i] = dfs(i);
        while (incr(players) && check) {
            if (check_players(players) && !check_players_iso(players)) {
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
                        if (is_ne()) {
                            cout << "NE FOUND ---------------------------------\n";
                            check = false;
                            break;
                        }
                        graph[i].has_t = true;
                    }
                }
            }
        }
    }
};

template<size_t n>
class g {
private:
    size_t num, all;
    atomic<size_t> d_count;
    boost::asio::thread_pool pool;
    vector<int> perms_good;
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

    bool check_connected(vector<v> &graph) {
        vector<bool> cl(n);
        dfs(graph, cl, 0);
        for (int i = 0; i != n; ++i)
            if (!cl[i])
                return false;
        return true;
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
        for (int i : perms_good) {
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
        return false;
    }

    bool check_iso(vector<v> &g1) {
        vector<int> in_degs1(n), in_degs2(n);
        for (int i = 0; i != n; ++i)
            ++in_degs1[g1[i].in.size()];
        for (auto &g2 : graphs_deg) {
            for (int i = 0; i != n; ++i) {
                ++in_degs2[g2.first[i].in.size()];
            }
            if (in_degs1 == in_degs2 && check_iso_cur(g1, g2)) {
                return true;
            }
            in_degs2.assign(n, 0);
        }
        return false;
    }

    void find_gf(vector<v> &graph) {
        ++all;
        if (check_connected(graph) && !check_iso(graph)) {
            vector<vector<int>> copy(n);
            make_copy(copy, graph);
            graphs_deg.emplace_back(graph, move(copy));
            boost::asio::post(pool, [m = num, graph, this] {
                gf<n> find_game(graph, m, perms, perms_back);
                ++d_count;
                if (d_count % 1000 == 0) {
                    stringstream ss;
                    ss << "Done: " << d_count << '\n';
                    cout << ss.str();
                }
            });
            ++num;
            if (num % 10000 == 0) {
                stringstream ss;
                ss << "Good: " << num << '\n' << "All: " << all << '\n';
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
        perms_good.clear();
        for (int i = 1; i != perms.size(); ++i) {
            check = true;
            for (int j = 0; j != n; ++j) {
                if (deg[j] != deg[perms[i][j]]) {
                    check = false;
                    break;
                }
            }
            if (check) {
                perms_good.push_back(i);
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
    g() : num(0), all(0), d_count(0), pool(thread::hardware_concurrency()) {
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
    g<2> game2;
    game2.find_game();
    g<3> game3;
    game3.find_game();
    g<4> game4;
    game4.find_game();
    g<5> game5;
    game5.find_game();
    g<6> game6;
    game6.find_game();
}

