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

using PlayerList = vector<Player>;
using PlayerPositionLists = unordered_map<string, PlayerList>;

/*
 * Reads the players database in data_base.txt and returns a vector of all the players (name, position, price and points)
 */
PlayerPositionLists get_players_list(Query query) 
{
  string databaseFile = "data_base.txt";
  ifstream in(databaseFile);

  PlayerPositionLists player_position_lists = {
    {"por", vector<Player>()},
    {"def", vector<Player>()},
    {"mig", vector<Player>()},
    {"dav", vector<Player>()}
  };
  
  while (not in.eof()) {
    string name, position, club;
    int points, cost;
    getline(in, name, ';');
    if (name == "") break;
    
    getline(in, position, ';');
    in >> cost;
    char aux; in >> aux;

    getline(in, club, ';');
    in >> points;
    string aux2;
    getline(in,aux2);

    
    if (cost > query.max_cost_per_player) continue; // filtrem quan el cost del jugador és major al cost màxim per jugador
    if (points == 0 and club != "FakeTeam") continue; // we don't store players that have 0 points, except from the last ones

    Player player = {name, position, cost, points};
    player_position_lists[position].push_back(player);
  }
  in.close();

  return player_position_lists;
}


// int main(int argc, char** argv) {
//   if (argc != 2) {
//     cout << "Syntax: " << argv[0] << " data_base.txt" << endl;
//     exit(1);
//   }

//   ifstream in(argv[1]);
//   while (not in.eof()) {
//     string nom, posicio, club;
//     int punts, preu;
//     getline(in,nom,';');    if (nom == "") break;
//     getline(in,posicio,';');
//     in >> preu;
//     char aux; in >> aux;
//     getline(in,club,';');
//     in >> punts;
//     string aux2;
//     getline(in,aux2);
//     cout << "Nom: " << nom << endl;
//     cout << "Posició: " << posicio << endl;
//     cout << "Preu: " << preu << endl;
//     cout << "Club: " << club << endl;
//     cout << "Punts: " << punts << endl;
//     cout << endl;
//   }
//   in.close();
// }
