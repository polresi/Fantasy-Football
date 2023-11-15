#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>

using namespace std;

struct Player
{
    string name;
    string pos;
    int price;
    int points;
};

struct Query
{
    int N1;
    int N2;
    int N3;
    int max_cost;
    int max_price_per_player;
};

using PlayerList = vector<Player>;
using PlayerPositionLists = unordered_map<string, PlayerList>;


class Solution {
private:
    PlayerList player_list;
    int cost;
    int points;

public:
    // Constructor
    Solution(PlayerList player_list, int cost, int points) 
        : player_list(player_list), cost(cost), points(points) {}


    void add_player(Player& player) {
        player_list.push_back(player);
        cost += player.price;
        points += player.points;
    }
    
    void remove_last_player() {
        Player player = player_list.back();
        player_list.pop_back();
        cost -= player.price;
        points -= player.points;
    }

    const PlayerList& get_players() const {
        return player_list;
    }

    int get_cost() const {
        return cost;
    }
    
    int get_points() const {
        return points;
    }
};


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
        cout << "Unable to open the file: " << input_query << endl;
    }

    return query; // Returns an empty query in case of an error.
}

/*
 * Reads the players database in data_base.txt and returns a vector of all the players (name, position, price and points)
 */
PlayerPositionLists get_players_list(const Query& query)
{
    string databaseFile = "data_base.txt";
    ifstream in(databaseFile);

    PlayerPositionLists player_position_lists = { // dictionary 
        {"por", PlayerList()},
        {"def", PlayerList()},
        {"mig", PlayerList()},
        {"dav", PlayerList()}
    };
  
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
        player_position_lists[position].push_back(player);
    }
    in.close();

  return player_position_lists;
}


Solution final_solution = {PlayerList{}, 0, -1}; // global variable to store the best solution found so far

void exhaustive_search(const Query& query, const PlayerPositionLists& player_position_lists, Solution partial_solution){ // ¿¿¿ partial_solution ha de ser una referencia ???
    if (partial_solution.player_list.size() == 11) { // found a candidate solution
        if (partial_solution.get_points() > final_solution.get_points()) { // candidate solution is better than solution
            final_solution = final_solution;
        }
    }
    
    if (partial_solution.cost > query.max_cost) { // this solution will never be better than the one already found
        return;        
    }

    // iterate over all possible por, def, mig, dav, which fullfull the restrictions N1, N2, N3
    
}


/*
 * Obtains the best solution using exhaustive search.
 * Modifies the global variable solution, and stores the best partial solution found there
 */
void exhaustive_search(const Query& query, const PlayerPositionLists& player_position_lists) {
    Solution initial_solution = {PlayerList{}, 0, 0};
    exhaustive_search(query, player_position_lists, initial_solution);
}


int main(int argc, char *argv[]) {
    if (argc != 4) { // si el nombre de fitxers no el correcte.
        cout << "Us incorrecte. S'han de proporcionar 3 arguments." << endl;
        return 1; 
    }

    string input_database = argv[1]; // players' database
    string input_query = argv[2]; // query input file
    string output = argv[3]; // output file

    Query query = read_query(input_query); // llegim la consulta
    PlayerPositionLists player_position_lists = get_players_list(query); // store all the players' info

    // print all the players that fulfill the query:
    for (auto& position_list : player_position_lists) {
        cout << position_list.first << ": ";
        for (auto& player : position_list.second) {
            cout << player.name << ", ";
        }
        cout << endl;
    }

    // compute the best solution
    exhaustive_search(query, player_position_lists);
    
    // print the solution
    for (auto& player : solution.player_list) {
        cout << player.name << ", ";
    }
    cout << endl << "Points: " << solution.points << ", Cost: " << solution.cost << endl;

}
