#include <set>
#include <tuple>
#include <vector>
#include <iostream>
#include <algorithm>

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

void write_vector(const vector<int> &v) {
    cout << v.size() << endl;
    for (int i = 0; i != v.size(); ++i) {
        cout << v[i] << ' ';
    }
    cout << endl;
}

vector<int> norm_players(vector<v> &graph, vector<int> &players) {
    int dif = 0;
    bool check = true;
    vector<int> players_norm(players.end() - graph.size(), players.end());
    vector<int> mapping(players.size(), -1);
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

int main() {
    int size;
    vector<v> g1, g2;
    vector<int> players;
    set<vector<int>> uniq_players;
    read_graph(g1);
    read_graph(g2);
    while (cin >> size) {
        players.resize(size);
        for (int i = 0; i != size; ++i) {
            cin >> players[i];
        }
        uniq_players.insert(norm_players(g2, players));
    }
    for (const vector<int> &p : uniq_players) {
        write_graph(g2);
        write_vector(p);
    }
}
