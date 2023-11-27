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

    bool is_valid() const {
        
        if (cost > query.max_cost) return false;

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
        players[p.pos].erase(remove(players[p.pos].begin(), players[p.pos].end(), p), players[p.pos].end());
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
        sort(players_map[pos].begin(), players_map[pos].end(), [](const Player& p1, const Player& p2) {
            return pow(p1.points, 3) / p1.price > pow(p2.points, 3) / p2.price; 
        });
    }

    return players_map;
}


int fitness(const Solution& solution){ // returns the fitness of a solution
    if (not solution.is_valid()) return 0; // penalize the solutions that exceed the maximum cost
    return solution.get_points();
}


pair<Solution, Solution> select_parents(const Population& population) {
    
    int index1 = rand() % population.size();
    int index2 = rand() % population.size();

    Solution solution1 = population[index1];
    Solution solution2 = population[index2];

    return {solution1, solution2};
}


void mutate(Solution& solution) {
    for (auto pos : positions) {
        for (Player p : solution[pos]) {
            if ((rand() / static_cast<double>(RAND_MAX)) < mutation_rate) {
                solution.remove_player(p);
                solution.add_player(players_map[pos][rand() % players_map[pos].size()]); // try not uniform distr
            }
        }
    }
    return;
}


Population recombine_and_mutate(const Solution& parent1, const Solution& parent2) {
    Population combined_solutions;
    
    for (int i = 0; i < num_combined; ++i) {

        Solution new_solution = parent1;
        for (auto pos : positions){
            for (int j = 0; j < new_solution[pos].size(); ++j) {
                if (rand() % 2 == 0) {
                    new_solution.remove_player(new_solution[pos][j]); // solution[pos] == solution.player[pos]
                    new_solution.add_player(parent2.at(pos)[j]);
                }
            }
        }
        mutate(new_solution);
        combined_solutions.push_back(new_solution);
    }

    return combined_solutions;
}

Population select_individuals(Population solutions) {
    Population selected_solutions;

    sort (solutions.begin(), solutions.end(), [](const Solution& s1, const Solution& s2) {
        return fitness(s1) > fitness(s2);
    });

    return Population(solutions.begin(), solutions.begin() + min(num_selected, solutions.size()));

    // for (long unsigned int i = 0; i < min(num_selected, solutions.size()); i++) {
    //     selected_solutions.push_back(solutions[i]);
    // }
    // return selected_solutions;
}

Population generate_initial_population() { // Use greedy algorithm to generate an initial solution
    Solution solution;
    for (auto pos : positions) {
        for (Player p : players_map[pos]) {
            if (solution.can_be_added(p)) solution.add_player(p);
        }
    }
    return {solution};
}
/*
int no_improvement_count = 0;
Solution prev_best;

bool not_finished(){
    int threshold = 100; // Set your threshold
    return no_improvement_count < threshold;
}
*/

void metaheuristica(int num_selected) {

    Population population = generate_initial_population();
    
    uint gen = 0;
    while (true){
        if (++gen%500 == 0) cout << gen << endl;
        auto [parent1, parent2] = select_parents(population);
        Population population = recombine_and_mutate(parent1, parent2);
        population = select_individuals(population);
        
        Solution candidate = population[0];
        if (candidate.get_points() > best_solution.get_points() and candidate.is_valid()) {
            best_solution = candidate;
            best_solution.write();
            cout << "New best solution found: " << best_solution.get_points() << ", " << best_solution.get_cost() <<  endl;
        }
    }
}


int main(int argc, char *argv[]) {
    if (argc != 4) { // If the number of the files is not correct
        cout << "Us incorrecte. S'han de proporcionar 3 arguments: data_base, query_file, output_file" << endl;
        return 1; 
    }
    
    start = chrono::high_resolution_clock::now(); // start the timer

    string input_database = argv[1]; // players' database
    string input_query = argv[2]; // query input file
    output_filename = argv[3]; // output file

    query = read_query(input_query); // llegim la consulta    
    players_map = read_players_map(); // store all the players' info
    
    metaheuristica(num_selected); 
}