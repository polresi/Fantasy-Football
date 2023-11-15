#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "parser.cc"

using namespace std;


struct Solution
{
    PlayerList player_list;
    int cost;
    int points;
};

Solution solution = {PlayerList(11), 0, 0};


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



void exhaustive_search(const Query& query, const PlayerPositionLists& player_position_lists, Solution partial_solution){ // ¿¿¿ partial_solution ha de ser una referencia ???
    if (partial_solution.player_list.size() == 11) { // found a candidate solution
        if (partial_solution.points > solution.points) { // candidate solution is better than solution
            solution = partial_solution;
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
