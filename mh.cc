#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <vector>

using namespace std;


// Global variables
const vector<string> positions = {"por", "def", "mig", "dav"}; // all the possible positions
map<string, string> pos_to_CAPS = {{"por","POR"}, {"def","DEF"}, {"mig","MIG"}, {"dav","DAV"}}; // map to convert the positions to uppercase
map<string, int> max_points_pos = {{"por", 0}, {"def", 0}, {"mig", 0}, {"dav", 0}}; // max points of all players in each position

string output_filename;
chrono::time_point <chrono::high_resolution_clock> start;

// Parameters of the metaheuristic algorithm
const long unsigned int population_size = 2000; // number of solutions selected in each iteration
const uint num_combined = 500; //  number of solutions combined and mutated in each iteration
const double mutation_rate = 0.15; // probability of mutation of each player in a mutated solution
const uint max_no_improvement = 2000; // maximum number of iterations without improvement allowed


// Random number generator
random_device rd;
mt19937 gen(rd());

// Returns a random integer between 0 and n-1
uint rand_uint(uint n) {
    uniform_int_distribution<uint> distribution(0, n-1);
    return distribution(gen);
}

// Returns a random floar bewteen 0 and 1
double rand_0to1() {
    uniform_real_distribution<double> distribution(0, 1);
    return distribution(gen);
}


class Player
{
public:
    static inline double alpha;
    string name, pos;
    int price, points;

    Player () {}

    Player(const string& name, const string& pos, int price, int points)
        : name(name), pos(pos), price(price), points(points) {}

    bool operator== (const Player& other) const {
        return name == other.name and pos == other.pos and price == other.price and points == other.points;
    }

    bool operator< (const Player& other) const {
        return get_value() < other.get_value();
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
    uint N1, N2, N3;
    int max_cost, max_price_per_player;
    map<string, uint> max_num_players = {{"por", 1}, {"def", N1}, {"mig", N2}, {"dav", N3}};
};

Query query; // global variable to store the query given


class Solution {

private:
    map<string, PlayerList> players;
    int cost, points;
    bool valid;
    bool valid_needs_update = true;

public:

    // Default constructor
    Solution() : cost(0), points(0) {
        for (auto pos : positions) {
            players[pos] = PlayerList();
        }
    }

    // Constructor
    Solution(map<string, PlayerList> players, int cost, int points) 
        : players(players), cost(cost), points(points) {}

    int get_cost() const { return cost; }
    
    int get_points() const { return points; }

    size_t get_size() const { // returns the number of players in the solution
        size_t size = 0;
        for (auto pos : positions) {
            size += at(pos).size();
        }
        return size;
    }

    void add_player(const Player& player) {
        players[player.pos].push_back(player);
        
        cost += player.price;
        points += player.points;

        valid_needs_update = true;
    }
    
    bool can_be_added(const Player& player) const {
        if (at(player.pos).size() + 1 > query.max_num_players[player.pos]) return false;
        if (cost + player.price > query.max_cost) return false;

        for (Player p : at(player.pos)) {
            if (p == player) return false;
        }
        return true;
    }

    void remove_player(const Player& p) {
        cost -= p.price;
        points -= p.points;

        auto it = find(players[p.pos].begin(), players[p.pos].end(), p);
        players[p.pos].erase(it);

        valid_needs_update = true;
    }

    bool is_valid() {
        if (valid_needs_update) {
            update_valid();
            valid_needs_update = false;
        }
        return valid;
    }

    const PlayerList& at(string pos) const {
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

    // Updates the valid attribute of the solution
    void update_valid() {
        if (cost > query.max_cost) {
            valid = false;
            return;
        }
        for (auto pos : positions) {
            // check that there are no repeated players in the same position
            for (uint i = 0; i < at(pos).size(); i++) {
                if (find(at(pos).begin() + i + 1, at(pos).end(), at(pos)[i]) != at(pos).end()) {
                    valid = false;
                    return;
                }
            }
        }
        valid = true;
    }

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
    uint N1, N2, N3;
    int max_cost, max_price_per_player;
    file >> N1 >> N2 >> N3 >> max_cost >> max_price_per_player;
    return {N1, N2, N3, max_cost, max_price_per_player};
}


/*
 * Reads the players database in data_base.txt and returns a map of all the players separated by position
 * and sorted by a heuristic determining the best players to be considered first.
 */
void read_players_map()
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
        if (points == 0) continue;
        
        Player player = {name, position, price, points};    
        players_map[player.pos].push_back(player);

        max_points_pos[position] = max(max_points_pos[position], points);
    }

    in.close();

    // remove players that are worse in points and price than other players in the same position given the maximum number of players in each position  
    for (auto pos : positions) {
        for (uint i = 0; i < players_map[pos].size(); i++) {
            Player player = players_map[pos][i];

            uint count = count_if(players_map[pos].begin(), players_map[pos].end(), [player](const Player& other) {
                return other.price <= player.price and other.points >= player.points;
            });
            if (count > query.max_num_players[pos]) {
                players_map[pos].erase(players_map[pos].begin() + i);
                i--;
            }

        }
    }

    // add fake players to each position given the maximum number of players in each position
    for (auto pos : positions) {
        for (uint i = 1; i <= query.max_num_players[pos]; i++) {
            Player fake_player = {"Fake_" + pos + to_string(i), pos, 0, 0};
            players_map[pos].push_back(fake_player);
        }
    }

}


int fitness(Solution& solution){ // returns the fitness of a solution
    if (not solution.is_valid()) return 0; // penalize the solutions that exceed the maximum cost
    return solution.get_points();
}


pair<Solution, Solution> select_parents(const Population& population) {

    vector<Solution> parents (2);
    sample(population.begin(), population.end(), parents.begin(), 2, gen);
    return {parents[0], parents[1]};
}


void mutate(Solution& solution) {
    for (auto pos : positions) {
        for (Player p : solution.at(pos)) {
            if (rand_0to1() < mutation_rate) {
                solution.remove_player(p);
                solution.add_player(players_map[pos][rand_uint(players_map[pos].size())]);
            }
        }
    }
    return;
}


void recombine_and_mutate(const Solution& parent1, const Solution& parent2, Population& population) {  
    for (uint i = 0; i < num_combined; ++i) {
    
        Solution new_solution = parent1;
        for (auto pos : positions){
            for (unsigned int j = 0; j < new_solution.at(pos).size(); ++j) {
                if (rand_uint(2) == 0) {
                    new_solution.remove_player(new_solution.at(pos)[j]);
                    new_solution.add_player(parent2.at(pos)[j]);
                }
            }
        }
        mutate(new_solution);
        population.push_back(new_solution);
    }
}


void select_individuals(Population& population) {

    sort(population.begin(), population.end(), [](Solution& s1, Solution& s2) {
        return fitness(s1) > fitness(s2);
    });

    population = Population(population.begin(), population.begin() + min(population_size, population.size()));
}


Population generate_initial_population() { // Use greedy algorithm to generate an initial solution

    // Population initial_population = {get_greedy_solution()};
    Population initial_population;

    for (uint i = 0; i < population_size; ++i) {
        Solution new_solution;
        for (auto pos : positions) {
            for (uint j = 0; j < query.max_num_players[pos]; j++) {
                new_solution.add_player(players_map[pos][rand_uint(players_map[pos].size())]);
            }
        }
        initial_population.push_back(new_solution);
    }
    return initial_population;
    
}


void metaheuristica(int population_size) {

    Population population = generate_initial_population();
    uint no_improvement_count = 0;
    uint generation = 0;
    while (no_improvement_count++ < max_no_improvement) {
        if (++generation % 500 == 0) cout << "gen " << generation << endl;
        auto [parent1, parent2] = select_parents(population);
        recombine_and_mutate(parent1, parent2, population);
        select_individuals(population);
        
        Solution candidate = population[0];
        if (candidate.get_points() > best_solution.get_points() and candidate.is_valid()) {
            cout << "New best solution " << candidate.get_points() << ", gen:" << generation << endl;
            best_solution = candidate;            
            best_solution.write();
            no_improvement_count = 0;
        }
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

    read_players_map();
    
    metaheuristica(population_size);
}