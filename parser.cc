#include <iostream>
#include <vector>
#include <fstream>
#include <cassert>
using namespace std;

string outputFile;
struct Player
{
    string name;
    string pos;
    int price;
    int points;
};


/*
 * Reads the players database in data_base.txt and returns a vector of all the players (name, position, price and points)
 */
vector<Player> get_players_list() 
{
  string databaseFile = "data_base.txt";
  ifstream in(databaseFile);
  
  vector<Player> player_list;

  while (not in.eof()) {
    string nom, posicio, club;
    int punts, preu;
    getline(in,nom,';');    if (nom == "") break;
    getline(in,posicio,';');
    in >> preu;
    char aux; in >> aux;
    getline(in,club,';');
    in >> punts;
    string aux2;
    getline(in,aux2);
    
    if (preu > ...) continue; //filtrar quan el preu és massa elevat !!!
    if (punts = 0 and club != "FakeTeam") continue; // we don't store players that have 0 points, except from the last ones
    
    Player player = {nom, posicio, preu, punts};
    player_list.push_back(player);
  }
  in.close();

  return player_list;
}


int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Syntax: " << argv[0] << " data_base.txt" << endl;
    exit(1);
  }

  ifstream in(argv[1]);
  while (not in.eof()) {
    string nom, posicio, club;
    int punts, preu;
    getline(in,nom,';');    if (nom == "") break;
    getline(in,posicio,';');
    in >> preu;
    char aux; in >> aux;
    getline(in,club,';');
    in >> punts;
    string aux2;
    getline(in,aux2);
    cout << "Nom: " << nom << endl;
    cout << "Posició: " << posicio << endl;
    cout << "Preu: " << preu << endl;
    cout << "Club: " << club << endl;
    cout << "Punts: " << punts << endl;
    cout << endl;
  }
  in.close();
}
