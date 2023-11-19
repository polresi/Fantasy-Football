#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cassert>


using namespace std;


// Global variables
const vector<string> positions = {"por", "def", "mig", "dav"}; // all the possible positions
map<string, string> pos_to_CAPS = {{"por","POR"}, {"def","DEF"}, {"mig","MIG"}, {"dav","DAV"}};

string output_filename;
chrono::time_point <chrono::high_resolution_clock> start;


struct Player
{
    string name;
    string pos;
    int price;
    int points;

    bool operator==(const Player& other) const {
        // Compare the members that define equality for Player objects
        return name == other.name && pos == other.pos && price == other.price && points == other.points;
    }

};

using PlayerList = vector<Player>; // vector of players
PlayerList player_list; // Global variable to store all the players


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


class Solution {

private:
    map<string, PlayerList> players;
    int cost;
    int points;
    string last_pos_added; // last position added to the solution

public:

    // Default constructor
    Solution() : cost(0), points(0), last_pos_added("") {
        for (auto pos : positions) {
            players[pos] = PlayerList();
        }
    }

    // Constructor
    Solution(map<string, PlayerList> players, int cost, int points, string last_pos_added) 
        : players(players), cost(cost), points(points), last_pos_added(last_pos_added) {}


    void add_player(Player& player) {
        players[player.pos].push_back(player);
        last_pos_added = player.pos;
        
        cost += player.price;
        points += player.points;
    }
    
    void pop_last_player(string pos) {
        Player player = players[pos].back();
        players[pos].pop_back();

        cost -= player.price;
        points -= player.points;
    }

    bool can_be_added(const Player& player) {
        if (players[player.pos].size() + 1 > query.max_num_players[player.pos]) return false;
        if (cost + player.price > query.max_cost) return false;

        for (Player p : players[player.pos]) {
            if (p == player) return false;
        }
        return true;
    }

    int get_cost() const {
        return cost;
    }
    
    int get_points() const { // returns the number of points of the solution
        return points;
    }

    uint get_size() { // returns the number of players in the solution
        int size = 0;
        for (auto pos : positions) {
            size += players[pos].size();
        }
        return size;
    }

    /*
     * Obtains the best player that can be added to the solution, and adds it to the solution.
     * It uses a heuristic that depends on the max_cost of the query
     * Returns false if no player can be added.
     */
    bool add_best_player() {
        player_list.erase(remove_if(player_list.begin(), player_list.end(), [this](Player p){
            return not can_be_added(p); // filters out the players that can't be added
        }), player_list.end());

        if (player_list.empty()) return false;

        // alpha is a parameter that depends on the maximum cost of the team of medium test and query 
        const double alpha = 1.5 * pow(query.max_cost / 75e6, 2);
        
        Player best_player;
        if (get_size() < 10) { // we're not adding the last player
            best_player = *max_element(player_list.begin(), player_list.end(), [alpha](const Player& a, const Player& b) {
                return pow(a.points, alpha + 1) / a.price < pow(b.points, alpha + 1) / b.price; // sort by points^alpha * density,
            });                                                                                 // where density = points/price
        } else { // we're adding the last player
            best_player = *max_element(player_list.begin(), player_list.end(), [alpha](const Player& a, const Player& b) {
                return a.points < b.points; // get the player with most points
            });
        }

        add_player(best_player);
        return true;
    }

    
    // Writes the solution in the output file
    void write() { 
        ofstream output(output_filename);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        output << fixed;
        output.precision(1);
        output << duration/1000.0 << endl;

        for (auto pos : positions) {
            output << pos_to_CAPS[pos] << ": ";
            write_players(pos, output);
        }
        
        output << "Punts: " << points << endl;
        output << "Preu: " << cost;
        output.close();
    }

private:

    // Writes the players of a given position in the output files
    void write_players(string pos, ofstream& output) {
        PlayerList all_players = players[pos];
        // add fake players to complete the team
        PlayerList fake_players = get_fake_players(pos);
        all_players.insert(all_players.end(), fake_players.begin(), fake_players.end());
        bool first = true;
        for (Player p : all_players) {
            if (first) {
                first = false;
                output << p.name;
            } else {
                output << ";" << p.name;
            }
        }
        output << endl;
    }

    PlayerList get_fake_players(string pos) {
        PlayerList p_list;
        uint num_players = query.max_num_players[pos] - players[pos].size();
        for (uint i = 0; i < num_players; i++) {
            Player player = {"Fake_"+pos+char('1'+i), pos, 0, 0};
            p_list.push_back(player);
        }
        return p_list;
    }

};

Solution best_solution; // global variable to store the best solution found so far


Query read_query(const string& input_query) {
    ifstream file(input_query);
    int N1, N2, N3, max_cost, max_price_per_player;
    file >> N1 >> N2 >> N3 >> max_cost >> max_price_per_player;
    return {N1, N2, N3, max_cost, max_price_per_player};
}


/*
 * Reads the players database in data_base.txt and returns a vector of all the players (name, position, price and points)
 */
PlayerList read_players_list()
{
    string databaseFile = "data_base.txt";
    ifstream in(databaseFile);

    PlayerList player_list(0);
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
        if (points == 0) continue; // we don't store players that have 0 points

        Player player = {name, position, price, points};
        player_list.push_back(player);
    }
    in.close();
  return player_list;
}


void greedy(Solution solution) {
    if (solution.add_best_player()){ // modifies solution, returns false when no player can be added
        greedy(solution);
    } else {
        solution.write();
    }
}


/*
 * Function that obtains a good solution using a greedy algorithm.
 * It sorts the players by density (points/price) and adds them to the solution if they are valid.
 */
void greedy() {
    Solution solution;
    greedy(solution);    
}


int main(int argc, char *argv[]) {
    if (argc != 4) { // If the number of the files is not correct
        cout << "Wrong usage, 3 arguments must be passed: data_base query_file output_file" << endl;
        return 1; 
    }
    
    start = chrono::high_resolution_clock::now(); // start the timer

    string input_database = argv[1]; // players' database
    string input_query = argv[2]; // query input file
    output_filename = argv[3]; // output file

    query = read_query(input_query); // llegim la consulta    
    player_list = read_players_list(); // store all the players' info

    greedy(); //greedy algorithm
}
