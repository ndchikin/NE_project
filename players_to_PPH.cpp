#include <set>
#include <vector>
#include <serialization.h>

using namespace std;

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
    size_t size;
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
