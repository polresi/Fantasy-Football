#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

using namespace std;


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

using PlayerList = vector<Player>;
PlayerList player_list; // global variable to store all the players


struct Query
{
    int N1;
    int N2;
    int N3;
    int max_cost;
    int max_price_per_player;
};


Query query; // global variable to store the query given

class Solution {

private:
    PlayerList player_list;
    int cost;
    int points;
    map<string, int> num_pos = {
        {"por", 0},
        {"def", 0},
        {"mig", 0},
        {"dav", 0}
    };

public:
    // Constructor
    Solution(PlayerList player_list, int cost, int points) 
        : player_list(player_list), cost(cost), points(points) {}


    void add_player(Player& player) {
        player_list.push_back(player);
        
        cost += player.price;
        points += player.points;
        num_pos[player.pos]++;
    }
    
    void pop_last_player() {
        Player player = player_list.back();
        player_list.pop_back();

        cost -= player.price;
        points -= player.points;
        num_pos[player.pos]--;
    }

    void complete_team() {// add the remaining player with 0 price from the FakeTeam
        for (auto& [pos, num_missing] : get_missing_players()) { 
            for (int i = 0; i < num_missing; i++) {
                Player player = {"Fake_"+pos+char(i), pos, 0, 0};
                add_player(player);
            }
        }
    }

    PlayerList& get_players() {
        complete_team();
        return player_list;
    }

    bool is_valid() {
        if (num_pos["por"] > 1 or num_pos["def"] > query.N1 or num_pos["mig"] > query.N2 or num_pos["dav"] > query.N3) {
            return false;
        }

        // check that there are no repeated players
        for (int i = 0; i < player_list.size() - 1; i++) {
            if (player_list[i] == player_list.back()) {
                return false;
            }
        }
        return true;
    }

    int get_cost() const {
        return cost;
    }
    
    int get_points() const {
        return points;
    }

    int get_size() const {
        return player_list.size();
    }


    map<string, int> get_missing_players() {
        return {{"por", 1 - num_pos["por"]},
                {"def", query.N1 - num_pos["def"]},
                {"mig", query.N2 - num_pos["mig"]},
                {"dav", query.N3 - num_pos["dav"]}};
    }
};

Solution final_solution = {PlayerList{}, 0, -1}; // global variable to store the best solution found so far

Query read_query(const string& input_query) {
    ifstream file(input_query);
    Query query;

    if (file.is_open()) {
        if (file >> query.N1 >> query.N2 >> query.N3) {
            if (file >> query.max_cost >> query.max_price_per_player) {
                file.close();
                return query;
            } else {
                cout << "Error en la lectura de la segona linia." << endl;
            }
        } else {
            cout << "Error en la lectura de la primera linia." << endl;
        }
        file.close();
    } else {
        cout << "No s'ha pogut obrir l'arxiu: " << input_query << endl;
    }

    return query; // Returns an empty query in case of an error.
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
void exhaustive_search(string output, Solution& partial_solution, int k = 0) {

    if (partial_solution.get_size() > 11) return;

    if (partial_solution.get_points() > final_solution.get_points()) { // candidate solution is better than solution
        final_solution = partial_solution;
        
        ofstream output_file(output);
        if (output_file.is_open()) {
            output_file << "La millor solució fins ara:" << endl;
            bool first = true;
            for (Player p : final_solution.get_players()) {
                if (first) {
                    first = false;
                    output_file << p.name;
                } else {
                    output_file << ", " << p.name;
                }
            }
            output_file << endl << "Punts: " << final_solution.get_points() << endl;
            output_file << "Cost: " << final_solution.get_cost() << endl << endl;
            output_file.close();
        } else {
            cout << "No s'ha pogut obrir l'arxiu: " << endl;
        }
    }

    if (partial_solution.get_cost() > query.max_cost) { // this solution will have a higher cost than the maximum
        return;
    }

    // iterate over all possible players from the last player you have added to the solution (to avoid repeated partial solutions)
    for (int i = k; i < player_list.size(); i++) {
        partial_solution.add_player(player_list[i]);
        if (partial_solution.is_valid()) {
            exhaustive_search(output, partial_solution, k+1);
        }
        partial_solution.pop_last_player();
    }
}

/*
 * Obtains the best solution using exhaustive search.
 * Modifies the global variable solution, and stores the best partial solution found there
 */
void exhaustive_search(string output) {
    Solution initial_solution = {PlayerList{}, 0, 0};
    exhaustive_search(output, initial_solution);
}

int main(int argc, char *argv[]) {
    if (argc != 4) { // If the number of the files is not correct
        cout << "Us incorrecte. S'han de proporcionar 3 arguments: data_base, query_file, output_file" << endl;
        return 1; 
    }

    string input_database = argv[1]; // players' database
    string input_query = argv[2]; // query input file
    string output = argv[3]; // output file

    query = read_query(input_query); // llegim la consulta    
    player_list = get_players_list(); // store all the players' info

    exhaustive_search(output); // stores the best solution in the global variable solution
}