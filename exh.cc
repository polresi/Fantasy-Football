#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;


// Global variables
const vector<string> positions = {"por", "def", "mig", "dav"}; // List of all the positions: ["por", "def", "mig", "dav"] 
const map<string, string> pos_to_CAPS = {{"por","POR"}, {"def","DEF"}, {"mig","MIG"}, {"dav","DAV"}}; // Map to convert the position to capital letters

string output_file;
chrono::time_point <chrono::high_resolution_clock> start_time;


class Player
{
public:
    static inline double alpha;
    string name;
    string pos;
    int price;
    int points;

    Player(const string& name, const string& pos, int price, int points)
        : name(name), pos(pos), price(price), points(points) {}

    bool operator== (const Player& other) const {
        return name == other.name and pos == other.pos and price == other.price and points == other.points;
    }

    bool operator> (const Player& other) const {
        return get_value() > other.get_value();
    }
    
private:
    
    double get_value() const {
        if (price == 0) return 0;
        return pow(points, alpha + 1) / price;
    }
};


struct Query
{
    int N1;
    int N2;
    int N3;
    int max_cost;
    int max_price_per_player;
    map<string, uint> max_num_players = {{"por", 1}, {"def", N1}, {"mig", N2}, {"dav", N3}};
};

Query query; // global variable to store the query given


using PlayerList = vector<Player>; // vector of players
using PlayerMap = map<string, PlayerList>; // map of players by position
PlayerMap players_map; // Global variable to store all the players


class Solution
{
private:
    map<string, PlayerList> players;
    int cost;
    int points;

public:

    Solution() : cost(0), points(0) {
        for (auto pos : positions) {
            players[pos] = PlayerList();
        }
    }

    int get_cost() const { return cost; }
    
    int get_points() const { return points; }

    size_t get_size() const {
        size_t size = 0;
        for (auto pos : positions) {
            size += players.at(pos).size();
        }
        return size;
    }

    // Adds a player to the solution if it is possible to do so.
    // @returns true if the player was added, false otherwise. 
    bool add_if_possible(const Player& player) {
        if (can_be_added(player)) {
            add_player(player);
            return true;
        }
        return false;
    }

    // Removes the last player added to the solution
    // @param pos: the position of the player to be removed
    void pop_last_player_at(string pos) {
        Player last_player = players[pos].back();
        players[pos].pop_back();

        cost -= last_player.price;
        points -= last_player.points;
    }

    // Returns the next position to be added to the solution, using the order: por, def, mig, dav
    string get_pos_to_add() const {
        for (auto pos : positions) {
            if (players.at(pos).size() < query.max_num_players[pos]) return pos;
        }
        
        assert(false); // the solution is not complete before calling this function
    }

    // Writes the solution in the output file
    void write() const { 
        ofstream output(output_file);

        auto end_time = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        output << fixed;
        output.precision(1);
        output << duration/1000.0 << endl;

        for (auto pos : positions) {
            output << pos_to_CAPS.at(pos) << ": ";
            write_players(pos, output);
        }

        output << "Punts: " << points << endl;
        output << "Preu: " << cost << endl;
        output.close();
    }

private:

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

Solution best_solution; // global variable to store the best solution found so far


Query read_query(const string& query_file) {
    ifstream file(query_file);
    int N1, N2, N3, max_cost, max_price_per_player;
    file >> N1 >> N2 >> N3 >> max_cost >> max_price_per_player;
    return {N1, N2, N3, max_cost, max_price_per_player};
}


/*
 * Reads the players database in data_base.txt and returns a map of all the players separated by position
 * and sorted by a heuristic determining the best players to be considered first.
 */
PlayerMap read_players_map()
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
        if (points == 0 and club != "FakeTeam") continue; // we don't store players that have 0 points, except from the last ones

        Player player = {name, position, price, points};
        players_map[player.pos].push_back(player);
    }
    in.close();

    // sort each of the lists of players by a heuristic determining the best players to be considered first
    for (auto pos : positions) {
        sort(players_map[pos].begin(), players_map[pos].end(), greater<Player>());
    }

  return players_map;
}

/*
 * Recursive function that obtains the best solution using exhaustive search.
 * Modifies the global variable solution, and stores the best partial solution found there
 * @param prev_pos the position of the last player added to the solution
 * @param last_index the index in players_map[prev_pos] of the last player added
 */
void exhaustive_search(Solution& solution, string prev_pos = "", uint last_index = 0) {
    
    if (solution.get_size() == 11) {
        
        if (solution.get_points() > best_solution.get_points()) {
            best_solution = solution;
            best_solution.write();
        }
        return;
    }

    if (solution.get_cost() > query.max_cost) return;

    // search for the next position needed to complete the solution
    string pos = solution.get_pos_to_add();
    if (prev_pos != pos) last_index = 0;
    
    // iterate over all players from the last player added to the solution to avoid repeated partial solutions
    for (uint i = last_index; i < players_map[pos].size(); i++) {
        Player player = players_map[pos][i];
        
        if (solution.add_if_possible(player)) {
            exhaustive_search(solution, pos, i+1);
            solution.pop_last_player_at(player.pos);
        }
    }
}

/*
 * Obtains the best solution using exhaustive search.
 * Modifies the global variable solution, and stores the best partial solution found there
 */
void exhaustive_search() {
    Solution initial_solution;
    exhaustive_search(initial_solution);
}


int main(int argc, char *argv[]) {

    start_time = chrono::high_resolution_clock::now(); // start the timer

    const string input_database = argv[1];
    const string query_file = argv[2];
    output_file = argv[3];

    query = read_query(query_file);
    Player::alpha = pow(query.max_cost / 1e7, 0.3); // sets the static variable alpha of the class Player

    players_map = read_players_map();
    
    exhaustive_search(); // stores the best solution in the global variable solution
}