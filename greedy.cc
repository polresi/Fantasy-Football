#include <algorithm>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;


// Global variables
const vector<string> positions = {"por", "def", "mig", "dav"}; // all the possible positions

string output_file;
chrono::time_point <chrono::high_resolution_clock> start_time;


class Player
{
public:
    static inline double alpha;
    string name, pos;
    int price, points;

    Player () {}

    Player(const string& name, const string& pos, int price, int points)
        : name(name), pos(pos), price(price), points(points) {}

    bool operator== (const Player& other) const {
        return name == other.name and pos == other.pos and price == other.price and points == other.points;
    }

    bool operator< (const Player& other) const {
        return get_value() < other.get_value();
    }

private:
    double get_value() const {
        if (price == 0) return 0;
        return pow(points, alpha + 1) / price;
    }

};

using PlayerList = vector<Player>; // vector of players
PlayerList player_list; // Global variable to store all the players


struct Query
{
    uint N1, N2, N3;
    int max_cost, max_price_per_player;
    map<string, uint> max_num_players = {{"por", 1}, {"def", N1}, {"mig", N2}, {"dav", N3}};
};

Query query; // global variable to store the query given


class Solution {

private:
    map<string, PlayerList> players;
    int cost, points;

public:

    Solution() : cost(0), points(0) {
        for (auto pos : positions) {
            players[pos] = PlayerList();
        }
    }


    // Returns true if some player can be added to the solution
    bool can_some_be_added() const {

        // filter out the players that can't (and will never be able to) be added
        player_list.erase(remove_if(player_list.begin(), player_list.end(), [this](Player p){
            return not can_be_added(p);
        }), player_list.end());

        if (player_list.empty()) return false;
        
        return true;
    }

    // Adds the best player to the solution, according to a heuristic
    void add_best_player() {
        Player best_player;
        
        if (size() == 10) { // last player to be added
            best_player = *max_element(player_list.begin(), player_list.end(), [](const Player& p1, const Player& p2) {
                return p1.points < p2.points; // get the player with most points
            });
        }
        else {
            best_player = *max_element(player_list.begin(), player_list.end()); // get the best player according to Player comparison operator
        }

        add_player(best_player);
    }
    
    // Writes the solution in the output file
    void write() const { 
        ofstream output(output_file);

        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        output << fixed;
        output.precision(1);
        output << duration/1000.0 << endl;

        const map<string, string> pos_to_UPPER = {{"por","POR"}, {"def","DEF"}, {"mig","MIG"}, {"dav","DAV"}};
        for (auto pos : positions) {
            output << pos_to_UPPER.at(pos) << ": ";
            write_players(pos, output);
        }

        output << "Punts: " << points << endl;
        output << "Preu: " << cost << endl;
        output.close();
    }

private:
    size_t size() const {
        size_t size = 0;
        for (auto pos : positions) {
            size += players.at(pos).size();
        }
        return size;
    }

    void add_player(const Player& player) {
        players[player.pos].push_back(player);
        
        cost += player.price;
        points += player.points;
    }

    bool can_be_added(const Player& player) const {
        if (players.at(player.pos).size() + 1 > query.max_num_players[player.pos]) return false;
        
        if (cost + player.price > query.max_cost) return false;

        for (Player p : players.at(player.pos)) {
            if (p == player) return false;
        }
        return true;
    }

    // Writes the players of a given position in the output files
    // @param pos: the position of the players to be written
    void write_players(string pos, ofstream& output) const {
        bool first = true;
        for (Player p : players.at(pos)) {
            if (first) first = false;
            else output << ";";
            output << p.name;
        }
        output << endl;
    }

};


Query read_query(const string& query_file) {
    ifstream file(query_file);
    uint N1, N2, N3;
    int max_cost, max_price_per_player;
    file >> N1 >> N2 >> N3 >> max_cost >> max_price_per_player;
    return {N1, N2, N3, max_cost, max_price_per_player};
}


/*
 * Reads the players database in data_base.txt and returns a vector of all the players (name, position, price and points)
 */
void read_players_list()
{
    string databaseFile = "data_base.txt";
    ifstream in(databaseFile);

    while (not in.eof()) {
        string name, position, club;
        int points, price;
        getline(in, name, ';');
        if (name == "") break;
        
        getline(in, position, ';');
        in >> price;
        char aux; in >> aux;

        getline(in, club, ';');
        in >> points;
        string aux2;
        getline(in,aux2);
        
        if (price > query.max_price_per_player) continue; // filter out the players with higher price than the maximum
        if (points == 0 and club != "FakeTeam") continue; // we don't store players that have 0 points (neither real player nor fake ones)

        Player player = {name, position, price, points};
        player_list.push_back(player);
    }

    in.close();
}


/*
 * Function that obtains a good solution using a greedy algorithm.
 * For each player to be added, it get the best player according to a heuristic.
 */
void greedy() {
    Solution solution;
    while(solution.can_some_be_added()) {
        solution.add_best_player();
    }

    solution.write();
}


int main(int argc, char *argv[]) {

    start_time = chrono::high_resolution_clock::now();

    const string input_database = argv[1];
    const string query_file = argv[2];
    output_file = argv[3];

    query = read_query(query_file);
    Player::alpha = pow(query.max_cost / 1e7, 0.3); // heuristic parameter for the greedy algorithm

    read_players_list();

    greedy();
}
