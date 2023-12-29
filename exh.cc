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
map<string, int> max_points_pos = {{"por", 0}, {"def", 0}, {"mig", 0}, {"dav", 0}}; // max points of all players in each position

string output_file;
chrono::time_point<chrono::high_resolution_clock> start_time;


class Player
{
public:
    static inline double alpha;
    string name, pos;
    int price, points;

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
    uint N1, N2, N3;
    int max_cost, max_price_per_player;
    map<string, uint> max_num_players = {{"por", 1}, {"def", N1}, {"mig", N2}, {"dav", N3}};
};

Query query;

using PlayerList = vector<Player>; // vector of players
using PlayerMap = map<string, PlayerList>; // map of players by position
PlayerMap players_map; // Global variable to store all the players


class Solution
{
private:
    map<string, PlayerList> players;
    int cost, points;

public:

    Solution() : cost(0), points(0) {
        for (auto pos : positions) {
            players[pos] = PlayerList();
        }
    }

    int get_cost() const { return cost; }
    
    int get_points() const { return points; }

    size_t size() const {
        size_t size = 0;
        for (auto pos : positions) {
            size += players.at(pos).size();
        }
        return size;
    }

    // The only way a player can't be added is if its price exceeds the maximum cost
    bool can_be_added(const Player& player) const {
        return cost + player.price <= query.max_cost;
    }

    // Adds a player to the solution
    void add_player(const Player& player) {
        players[player.pos].push_back(player);
        
        cost += player.price;
        points += player.points;
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

    // Returns the maximum possible points of the solution, using the current points of the solution 
    // and the maximum points of players in each position
    int get_max_possible_points() const {
        int max_possible_points = points;
        for (auto pos : positions) {
            max_possible_points += max_points_pos.at(pos) * (query.max_num_players.at(pos) - players.at(pos).size());
        }
        return max_possible_points;
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
    uint N1, N2, N3;
    int max_cost, max_price_per_player;
    file >> N1 >> N2 >> N3 >> max_cost >> max_price_per_player;
    return {N1, N2, N3, max_cost, max_price_per_player};
}


/*
 * Reads the players database in data_base.txt and returns a map of all the players separated by position
 * and sorted by a heuristic determining the best players to be considered first.
 */
void read_players_map()
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
        if (points == 0) continue;
        
        Player player = {name, position, price, points};    
        players_map[player.pos].push_back(player);

        max_points_pos[position] = max(max_points_pos[position], points);
    }

    in.close();

    // remove players that are worse in points and price than other players in the same position given the maximum number of players in each position  
    for (auto pos : positions) {
        for (uint i = 0; i < players_map[pos].size(); i++) {
            Player player = players_map[pos][i];

            uint count = count_if(players_map[pos].begin(), players_map[pos].end(), [player](const Player& other) {
                return other.price <= player.price and other.points >= player.points;
            });
            if (count > query.max_num_players[pos]) {
                players_map[pos].erase(players_map[pos].begin() + i);
                i--;
            }

        }
    }
    
    // sort each of the lists of players by a heuristic determining the best players to be considered first
    for (auto pos : positions) {
        sort(players_map[pos].begin(), players_map[pos].end(), greater<Player>());
    }

    // add fake players to each position given the maximum number of players in each position
    for (auto pos : positions) {
        for (uint i = 1; i <= query.max_num_players[pos]; i++) {
            Player fake_player = {"Fake_" + pos + to_string(i), pos, 0, 0};
            players_map[pos].push_back(fake_player);
        }
    }

}

/*
 * Recursive function that obtains the best solution using exhaustive search.
 * Modifies the global variable solution, and stores the best partial solution found there
 * @param prev_pos the position of the last player added to the solution
 * @param last_index the index in players_map[prev_pos] of the last player added
 */
void exhaustive_search(Solution& solution, string prev_pos = "", uint last_index = 0) {
    
    if (solution.size() == 11) {
        
        if (solution.get_points() > best_solution.get_points()) {
            best_solution = solution;
            best_solution.write();
        }
        return;
    }

    if (solution.get_cost() > query.max_cost) return;

    if (solution.get_max_possible_points() < best_solution.get_points()) return;

    // search for the next position needed to complete the solution
    string pos = solution.get_pos_to_add();
    if (prev_pos != pos) last_index = 0;
    
    // iterate over all players from the last player added to the solution to avoid repeated partial solutions
    for (uint i = last_index; i < players_map[pos].size(); i++) {
        Player player = players_map[pos][i];
        
        if (solution.can_be_added(player)) {
            solution.add_player(player);

            exhaustive_search(solution, pos, i+1);
            solution.pop_last_player_at(player.pos);
        }
        // !!! could maybe be improved by adding a break here, for example if the players are sorted by price ASC
        // and one player isn't added, then the next players won't be added either
        // but this could hurt performance because the first solution would be really bad
        // Maybe we could get the greedy solution, and start comparing to it, but are we allowed???
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

    start_time = chrono::high_resolution_clock::now();

    const string input_database = argv[1];
    const string query_file = argv[2];
    output_file = argv[3];

    query = read_query(query_file);
    Player::alpha = pow(query.max_cost / 1e7, 0.3); // sets the static variable alpha of the class Player

    read_players_map();
    
    exhaustive_search(); // stores the best solution in the global variable solution
}