#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

struct Player
{
    string name;
    string pos;
    int price;
    string team;
    int points;
};

struct Query
{
    int n1;
    int n2;
    int n3;
    int T;
    int J;
};

Query read_query(const string& input_query) {
    ifstream file(input_query);
    Query query;

    if (file.is_open()) {
        if (file >> query.n1 >> query.n2 >> query.n3) {
            if (file >> query.T >> query.J) {
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


vector<Player> read_database(const string& input_database) {

}


int main(int argc, char *argv[]) {
    if (argc != 4) { // si el nombre de fitxers no el correcte.
        cout << "Us incorrecte. S'han de proporcionar 3 arguments." << endl;
        return 1; 
    }

    string input_database = argv[1]; // lectura dels jugadors
    string input_query = argv[2]; // lectura de la consulta
    string output = argv[3]; // arxiu de sortida

    vector<>
    Query query = read_query(input_query); // llegim la consulta

    cout << query.T << endl;
}
