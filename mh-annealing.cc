#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <random>
#include <cassert>

using namespace std;

// Global variables
const vector<string> positions = {"por", "def", "mig", "dav"}; // all the possible positions
map<string, string> pos_to_CAPS = {{"por","POR"}, {"def","DEF"}, {"mig","MIG"}, {"dav","DAV"}};

string output_filename;
chrono::time_point <chrono::high_resolution_clock> start;

const long unsigned int num_selected = 100; // parameter: number of solutions selected in each iteration
const int num_combined = 50; // parameter: number of solutions combined in each iteration
const double mutation_rate = 0.1;
const uint T0 = 1000000; // parameter: initial temperature
const double alpha = 0.99; // parameter: cooling rate
uint no_improvement_count = 0;


class Player
{
public:
    static inline double alpha;
    string name;
    string pos;
    int price;
    int points;

    Player () {}

    Player(const string& name, const string& pos, int price, int points)
        : name(name), pos(pos), price(price), points(points) {}

    bool operator== (const Player& other) const {
        return name == other.name and pos == other.pos and price == other.price and points == other.points;
    }

    bool operator> (const Player& other) const {
        return get_value() > other.get_value();
    }

    double get_value() const {
        if (price == 0) return 0;
        return pow(points, alpha + 1) / price;
    }
};


using PlayerList = vector<Player>; // vector of players
using PlayerMap = map<string, PlayerList>; // map of players by position
PlayerMap players_map; // Global variable to store all the players


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

public:

    // Default constructor
    Solution() : cost(0), points(0){
        for (auto pos : positions) {
            players[pos] = PlayerList();
        }
    }

    // Constructor
    Solution(map<string, PlayerList> players, int cost, int points, string last_pos_added) 
        : players(players), cost(cost), points(points) {}


    int get_cost() const { return cost; }
    
    int get_points() const { return points; }

    uint get_size() const { // returns the number of players in the solution
        int size = 0;
        for (auto pos : positions) {
            size += players.at(pos).size();
        }
        return size;
    }

    void add_player(const Player& player) {
        players[player.pos].push_back(player);
        
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

    bool is_valid() const {
        if (cost > query.max_cost) return false;
        if (get_size() != 11) return false;
        // check that there are no repeated players
        for (auto pos : positions) {        
            for (uint i = 0; i < players.at(pos).size() - 1; i++) {
                for (uint j = i+1; j < players.at(pos).size(); j++) {
                    if (players.at(pos)[i] == players.at(pos)[j]) return false;
                }
            }
        }
        return true;
    }

    void remove_player(const Player& p) {
        cost -= p.price;
        points -= p.points;
        players[p.pos].erase(find(players[p.pos].begin(), players[p.pos].end(), p));
    }

    void remove_player_at(string pos, uint idx){
        Player p = players[pos][idx];
        remove_player(p);
    }

    // Returns a random position and index of a player in the solution
    pair<string, uint> get_random_pos_and_idx() {
        uint index = rand() % get_size();
        uint count = 0;
        string position;
        for (auto pos : positions) {
            if (count + players[pos].size() > index) return {pos, index - count};
            count += players[pos].size();
        }
        assert(false);
    }

    // Adds a random player to the solution, while remaining valid
    void add_random_player(string pos) {
        PlayerList player_list = players_map[pos];
        random_shuffle(player_list.begin(), player_list.end());
        for (Player p : player_list) {
            if (can_be_added(p)) { // add if possible ???
                add_player(p);
                return;
            }
        }
        assert(false);
    }    

    void substitute_random_player(){
        auto [pos, idx] = get_random_pos_and_idx();
        remove_player_at(pos, idx);
        add_random_player(pos);
    }


    PlayerList operator[](string pos) {
        return players[pos];
    }

    PlayerList at(string pos) const {
        return players.at(pos);
    }

    
    // Writes the solution in the output file
    void write() {
        ofstream output(output_filename);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        output << fixed;
        output.precision(1);
        output << duration/1000.0 << endl;

        for (auto pos : positions) {
            output << pos_to_CAPS[pos] << ": ";
            write_players(pos, output);
        }
        
        output << "Punts: " << points << endl;
        output << "Preu: " << cost << endl;
        output.close();
    }

private:

    // Writes the players of a given position in the output files
    void write_players(string pos, ofstream& output) {
        bool first = true;
        for (Player p : players[pos]) {
            if (first) {
                first = false;
                output << p.name;
            } else {
                output << ";" << p.name;
            }
        }
        output << endl;
    }

};

Solution best_solution; // global variable to store the best solution found so far
using Population = vector<Solution>;


Query read_query(const string& input_query) {
    ifstream file(input_query);
    int N1, N2, N3, max_cost, max_price_per_player;
    file >> N1 >> N2 >> N3 >> max_cost >> max_price_per_player;
    return {N1, N2, N3, max_cost, max_price_per_player};
}


PlayerMap read_players_map()
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
        if (points == 0 and club != "FakeTeam") continue; // we don't store players that have 0 points, except from the last ones

        Player player = {name, position, price, points};
        players_map[player.pos].push_back(player);
    }
    in.close();

    // sort each of the lists of players by a heuristic determining the best players to be considered first
    for (auto pos : positions) {
        sort(players_map[pos].begin(), players_map[pos].end(), greater<Player>());
    }

    return players_map;
}

double rand(uint x) { return rand() / static_cast<double>(RAND_MAX) * x; }

double probabiliy(double delta, uint T) { return exp(-delta/T); }

double fitness(Solution s) { return s.is_valid() * s.get_points(); }


Solution generate_initial_solution() {
    /*
    Solution s;
    for (auto pos : positions) {
        for (uint i = 0; i < query.max_num_players[pos]; i++) {
            s.add_random_player(pos);
        }
    }
    return s;
    */
    Solution solution;
    vector<PlayerList::iterator> iters = {players_map["por"].begin(), players_map["def"].begin(), 
                                          players_map["mig"].begin(),players_map["dav"].begin()};

    vector<double> best_values = {players_map["por"][0].get_value(), players_map["def"][0].get_value(),
                                  players_map["mig"][0].get_value(), players_map["dav"][0].get_value()};

    while (solution.get_size() < 11) {
        int index = max_element(best_values.begin(), best_values.end()) - best_values.begin();
        
        if (solution.can_be_added(*(iters[index])))
            solution.add_player(*(iters[index]));

        iters[index]++;
        
        if (iters[index] == players_map[positions[index]].end())
            best_values[index] = -1;
        else
            best_values[index] = (*iters[index]).get_value();
    }
    return solution;

}

Solution pick_random_neighbour(const Solution& s) {
    Solution r = s;
    r.substitute_random_player();
    return r;
}


void annealing() {
    Solution s = generate_initial_solution();
    uint T = T0;
    uint k = 0;
    no_improvement_count = 0;
    while (no_improvement_count++ < 100000){
        if (k % 1000 == 0) cout << "Generation: " << k << endl;

        Solution r = pick_random_neighbour(s);
        if (fitness(r) > fitness(s)){
            s = r;
            if (fitness(s) > fitness(best_solution)) {
                best_solution = s;
                best_solution.write();
                no_improvement_count = 0;
            }
        }

        else if (probabiliy(fitness(r) - fitness(s), T) > rand(1)) s = r;
        k++;
        T *= alpha;
    }
}






int main(int argc, char *argv[]) {
    if (argc != 4) { // If the number of the files is not correct
        cout << "Us incorrecte. S'han de proporcionar 3 arguments: data_base, query_file, output_file" << endl;
        return 1; 
    }

    start = chrono::high_resolution_clock::now(); // start the timer

    string input_database = argv[1];
    string input_query = argv[2];
    output_filename = argv[3];

    query = read_query(input_query);
    Player::alpha = pow(query.max_cost / 1e7, 0.3);

    players_map = read_players_map();
    
    annealing();
}