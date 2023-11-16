#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <chrono>


using namespace std;


// Global variables
const vector<string> positions = {"por", "def", "mig", "dav"}; // all the possible positions
map<string, string> pos_to_CAPS = {{"por","POR"}, {"def","DEF"}, {"mig","MIG"}, {"dav","DAV"}};

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


class Query
{
public:
    int N1;
    int N2;
    int N3;
    int max_cost;
    int max_price_per_player;
    map<string, uint> max_players_per_position;
    
    // Default constructor
    Query() {}
    
    // Constructor
    Query(int N1, int N2, int N3, int max_cost, int max_price_per_player)
        : N1(N1), N2(N2), N3(N3), max_cost(max_cost), max_price_per_player(max_price_per_player) 
    {
        max_players_per_position = {{"por", 1}, {"def", N1}, {"mig", N2}, {"dav", N3}};
    }
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

    bool is_valid() {
        for (auto pos : positions) {
            if (players[pos].size() > query.max_players_per_position[pos]) {
                return false;
            }
        }
        // check that there are no repeated players        
        for (uint i = 0; i < players[last_pos_added].size() - 1; i++) {
            if (players[last_pos_added][i] == players[last_pos_added].back()) {
                return false;
            }
        }
        return true;
    }

    int get_cost() const {
        return cost;
    }
    
    int get_points() const { // returns the number of points of the solution
        return points;
    }

    int get_size() { // returns the number of players in the solution
        int size = 0;
        for (auto pos : positions) {
            size += players[pos].size();
        }
        return size;
    }


    PlayerList get_fake_players(string pos) {
        PlayerList p_list;
        uint num_players = query.max_players_per_position[pos] - players[pos].size();
        for (uint i = 0; i < num_players; i++) {
            Player player = {"Fake_"+pos+char('1'+i), pos, 0, 0};
            p_list.push_back(player);
        }
        return p_list;
    }
    
    // Writes the solution in the output file
    void write(string filename) { 
        ofstream output_file(filename);
        if (output_file.is_open()) {
            auto end = chrono::high_resolution_clock::now(); // stop the timer
            auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            output_file << fixed;
            output_file.precision(1);
            output_file << duration/1000.0 << endl;

            for (auto pos : positions) {    
                // print the time it took to obtain the solution
                output_file << pos_to_CAPS[pos] << ": ";
                PlayerList all_players = players[pos];
                // add fake players to complete the team

                auto fake_players = get_fake_players(pos);
                all_players.insert(all_players.end(), fake_players.begin(), fake_players.end());
                // all_players.insert(all_players.end(), get_fake_players(pos).begin(), get_fake_players(pos).end()); 
                bool first = true;
                for (Player p : all_players) {
                    if (first) {
                        first = false;
                        output_file << p.name;
                    } else {
                        output_file << ";" << p.name;
                    }
                }
                output_file << endl;
            }
            output_file << "Punts: " << points << endl;
            output_file << "Preu: " << cost << endl << endl;
            output_file.close();
        } else {
            cout << "No s'ha pogut obrir l'arxiu: " << endl;
        }
    }

};

Solution final_solution; // global variable to store the best solution found so far


Query read_query(const string& input_query) {
    ifstream file(input_query);
    int N1, N2, N3, max_cost, max_price_per_player;
    file >> N1 >> N2 >> N3 >> max_cost >> max_price_per_player;
    return {N1, N2, N3, max_cost, max_price_per_player};
}

/*
 * Reads the players database in data_base.txt and returns a vector of all the players (name, position, price and points)
 */
PlayerList get_players_list()
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
        if (points == 0) continue; // we don't store players that have 0 points, except from the last ones

        Player player = {name, position, price, points};
        player_list.push_back(player);
    }
    in.close();

  return player_list;
}

/*
 * Recursive function that obtains the best solution using exhaustive search.
 * Modifies the global variable solution, and stores the best partial solution found there
 */
void exhaustive_search(string output_file, Solution& partial_solution, int k = 0) {
    
    if (partial_solution.get_size() > 11) return;

    if (partial_solution.get_cost() > query.max_cost) return; // this solution will have a higher cost than the maximum


    // candidate solution is better than solution
    if (partial_solution.get_points() > final_solution.get_points()) {
        final_solution = partial_solution;
        final_solution.write(output_file);
    }
    // iterate over all possible players from the last player you have added to the solution (to avoid repeated partial solutions)
    for (uint i = k; i < player_list.size(); i++) {
        partial_solution.add_player(player_list[i]);
        string pos = player_list[i].pos;
        if (partial_solution.is_valid()) {
            exhaustive_search(output_file, partial_solution, k+1);
        }
        partial_solution.pop_last_player(pos);
    }
}

/*
 * Obtains the best solution using exhaustive search.
 * Modifies the global variable solution, and stores the best partial solution found there
 */
void exhaustive_search(string output_file) {
    Solution initial_solution;
    exhaustive_search(output_file, initial_solution);
}


int main(int argc, char *argv[]) {
    if (argc != 4) { // If the number of the files is not correct
        cout << "Us incorrecte. S'han de proporcionar 3 arguments: data_base, query_file, output_file" << endl;
        return 1; 
    }
    
    start = chrono::high_resolution_clock::now(); // start the timer

    string input_database = argv[1]; // players' database
    string input_query = argv[2]; // query input file
    string output_file = argv[3]; // output file

    query = read_query(input_query); // llegim la consulta    
    player_list = get_players_list(); // store all the players' info

    exhaustive_search(output_file); // stores the best solution in the global variable solution
}