#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "parser.cc"

using namespace std;


using Solution = vector<Player>; 
Solution solution = vector<Player>(11);

struct Query
{
    int N1;
    int N2;
    int N3;
    int max_cost;
    int max_cost_per_player;
};

Query read_query(const string& input_query) {
    ifstream file(input_query);
    Query query;

    if (file.is_open()) {
        if (file >> query.N1 >> query.N2 >> query.N3) {
            if (file >> query.max_cost >> query.max_cost_per_player) {
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




void exhaustive_search(const Query& query) {
    
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

    cout << query.max_cost << endl;
}
