#include <tuple>
#include <thread>
#include <vector>
#include <fstream>
#include <iostream>
#include <boost/asio.hpp>

using namespace std;

int toVar(int player, int a, int b, int n) {
    return static_cast<int>(player * (n + 1) * n / 2 + a * (2 * n - a - 1) / 2 + b);
}

int toLit(int player, int a, int b, int n) {
    if (a < b) {
        return toVar(player, a, b, n);
    } else {
        return -toVar(player, b, a, n);
    }
}

void addTr(int player, int a, int b, int c, vector<vector<int>> &CNF, int n) {
    CNF.push_back({toLit(player, b, a, n), toLit(player, c, b, n), toLit(player, a, c, n)});
    CNF.push_back({toLit(player, a, b, n), toLit(player, b, c, n), toLit(player, c, a, n)});
}

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
    size_t dif;
    vector<ndim> form;
    vector<v> graph;
    vector<int> size, good_perm;
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

    bool dfs_cyclic(vector<v> &g, vector<bool> &cl, int index, int begin) {
        cl[index] = true;
        for (int i : g[index].out)
            if (i == begin || (!cl[i] && dfs_cyclic(g, cl, i, begin)))
                return true;
        return false;
    }

    bool check_players(vector<int> &players) {
        dif = 0;
        int a_dif = 0;
        bool overlap = false;
        vector<int> count(n);
        for (int i = 0; i != n; ++i) {
            if (count[players[i]] == 0) {
                if (players[i] != dif) {
                    return false;
                }
                ++dif;
            }
            ++count[players[i]];
        }
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
        form[begin] = ndim(size);
        vector<int> strategy(n);
        vector<bool> cl(n);
        do {
            form[begin][find_index(strategy, players)] = find_outcome(strategy, cl, begin);
            cl.assign(n, false);
        } while (incr_g(strategy));
    }

    void write_CNF(int p_num, vector<int> &players) {
        cout << p_num << '\n';
        int var = dif * (n + 1) * n;
        fstream f("/home/nikolay/Desktop/PPH/" + to_string(n) + "/" + to_string(dif) + "/" + to_string(p_num) + ".txt",
                  ios::out);
        vector<vector<int>> CNF;
        for (auto &c : out_comp) {
            if (!c.empty()) {
                CNF.emplace_back();
                for (auto &p : c) {
                    CNF.back().push_back(toLit(p.player, p.a, p.b, n));
                }
            }
        }
        var /= 2;
        f << "p cnf " << var << ' ' << CNF.size();
        for (auto &clause : CNF) {
            f << '\n';
            for (int Lit : clause) {
                f << Lit << ' ';
            }
            f << 0;
        }
        f << "\nc";
        for (int i = 0; i != n; ++i) {
            f << ' ' << players[i];
        }
        f.close();
    }

    void resize() {
        out_comp.resize(form[0].get_size());
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
            for (int k = 0; k != n; ++k) {
                id = form[k].find_index(index);
                outcome = form[k].find_cases(index, ans);
                for (uint8_t i = 0; i != dif; ++i) {
                    for (uint8_t j = 0; j != n + 1; ++j) {
                        if (ans[i][j]) {
                            out_comp[id].emplace_back(i, outcome, j);
                        }
                    }
                }
                for (auto &line : ans)
                    line.assign(n + 1, 0);
            }
        } while (incr_s(index));
    }

    void dfs_rec(vector<bool> &cl, int index) {
        cl[index] = true;
        for (int i : graph[index].out)
            if (!cl[i])
                dfs_rec(cl, i);
    }

    void make_copy(vector<vector<int>> &copy, vector<v> &g) {
        for (int i = 0; i != n; ++i) {
            copy[i].assign(n, 0);
            for (int j : g[i].out) {
                copy[i][j] = 1;
            }
        }
    }

    void make_copy(vector<vector<int>> &copy, vector<v> &g, int p) {
        for (int i = 0; i != n; ++i) {
            copy[i].assign(n, 0);
            for (int j : g[perms[p][i]].out) {
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
    gf(vector<v> g, vector<vector<int>> &p, vector<vector<int>> &p_b) : dif(0), form(n), graph(move(g)), perms(p),
                                                                        perms_back(p_b) {
        cout << "1";
        int p_count = 0;
        if (n % 2 == 0) {
            dif = 2;
            vector<int> players(n);
            for (int k = 0; k < n; ++k) {
                players[k] = k % 2;
            }
            size.assign(dif, 1);
            for (int j = 0; j != n; ++j) {
                size[players[j]] *= graph[j].degree + (graph[j].has_t ? 1 : 0);
            }
            for (int i = 0; i != n; ++i) {
                find_form(players, i);
            }
            resize();
            make_data();
            cout << p_count << '\n';
            write_CNF(p_count, players);
            ++p_count;
        }
        if (n % 3 == 0) {
            dif = 3;
            vector<int> players(n);
            for (int k = 0; k < n; ++k) {
                players[k] = k % 3;
            }
            size.assign(dif, 1);
            for (int j = 0; j != n; ++j) {
                size[players[j]] *= graph[j].degree + (graph[j].has_t ? 1 : 0);
            }
            for (int i = 0; i != n; ++i) {
                find_form(players, i);
            }
            resize();
            make_data();
            cout << p_count << '\n';
            write_CNF(p_count, players);
        }
        cout << "2";
    }
};

template<size_t n>
class g {
private:
    size_t num, all;
    atomic<size_t> d_count;
    vector<int> perms_good;
    vector<vector<int>> perms, perms_back;

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

    void all_perms_rec(vector<int> order, vector<bool> in, int deph) {
        if (deph == n) {
            if (perms.size() % 10000 == 0)
                cout << perms.size() << '\n';
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
    g() : num(0), all(0), d_count(0) {
        gen_perms();
    }

    void find_game() {
        cout << "Begin: " << n << '\n';
        vector<int> deg(n, 2);
        find_perms(deg);
        vector<v> graph(n);
        for (int i = 0; i != n; ++i) {
            graph[i].degree = 2;
            graph[i].out.push_back((i + 1) % n);
            graph[i].in.push_back((i + 1) % n);
            graph[(i + 1) % n].out.push_back(i);
            graph[(i + 1) % n].in.push_back(i);
        }
        gf<n> find_game(graph, perms, perms_back);
        cout << "End: " << n << '\n';
    }
};

int main() {
//    for (int i = 3; i < 21; ++i) {
//        for (int j = 1; j < i + 1; ++j) {
//            vector<vector<int>> CNF;
//            fstream f("/home/nikolay/Desktop/PPH/" + to_string(i) + "/" + to_string(j) + "/" + "ter.txt",
//                      ios::out);
//            for (int k = 0; k < j; ++k) {
//                for (int l = 0; l < i + 1; ++l) {
//                    for (int m = l + 1; m < i + 1; ++m) {
//                        for (int n = m + 1; n < i + 1; ++n) {
//                            addTr(k, l, m, n, CNF, i);
//                        }
//                    }
//                }
//            }
//            for (auto &cl : CNF) {
//                for (int Lit : cl) {
//                    f << Lit << ' ';
//                }
//                f << 0 << '\n';
//            }
//            f.close();
//        }
//    }
    g<11>().find_game();
}
