#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <random>


using namespace std;


// Global variables
const vector<string> positions = {"por", "def", "mig", "dav"}; // all the possible positions
map<string, string> pos_to_CAPS = {{"por","POR"}, {"def","DEF"}, {"mig","MIG"}, {"dav","DAV"}};

string output_filename;
chrono::time_point <chrono::high_resolution_clock> start;


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
PlayerList player_list; // Global variable to store all the players


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
    string last_pos_added; // last position added to the solution

public:

    // Default constructor
    Solution() : cost(0), points(0), last_pos_added("") {
        for (auto pos : positions) {
            players[pos] = PlayerList();
        }
    }

    // Constructor
    Solution(map<string, PlayerList> players, int cost, int points, string last_pos_added) 
        : players(players), cost(cost), points(points), last_pos_added(last_pos_added) {}


    void add_player(Player& player) {
        players[player.pos].push_back(player);
        last_pos_added = player.pos;
        
        cost += player.price;
        points += player.points;
    }
    
    void pop_last_player(string pos) {
        Player player = players[pos].back();
        players[pos].pop_back();

        cost -= player.price;
        points -= player.points;
    }

    bool can_be_added(Player& player) {
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

    bool is_valid() {
        for (auto pos : positions) {
            if (players[pos].size() > query.max_num_players[pos]) {
                return false;
            }
        }
        // check that there are no repeated players        
        for (uint i = 0; i < players[last_pos_added].size() - 1; i++) {
            if (players[last_pos_added][i] == players[last_pos_added].back()) {
                return false;
            }
        }
        return true;
    }


    void remove_player(const Player& p) {
        players[p.pos].erase(remove(players[p.pos].begin(), players[p.pos].end(), p), players[p.pos].end());
    }

    PlayerList operator[](string pos) {
        return players[pos];
    }

    
    // Writes the solution in the output file
    void write() { 
        ofstream output(output_filename);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        output << fixed;
        output.precision(1);
        output << duration/1000.0 << endl;

        for (auto pos : positions) {
            output << pos_to_CAPS[pos] << ": ";
            write_players(pos, output);
        }
        
        output << "Punts: " << points << endl;
        output << "Preu: " << cost;
        output.close();
    }

private:

    // Writes the players of a given position in the output files
    void write_players(string pos, ofstream& output) {
        PlayerList all_players = players[pos];
        // add fake players to complete the team
        PlayerList fake_players = get_fake_players(pos);
        all_players.insert(all_players.end(), fake_players.begin(), fake_players.end());
        bool first = true;
        for (Player p : all_players) {
            if (first) {
                first = false;
                output << p.name;
            } else {
                output << ";" << p.name;
            }
        }
        output << endl;
    }

    PlayerList get_fake_players(string pos) {
        PlayerList p_list;
        uint num_players = query.max_num_players[pos] - players[pos].size();
        for (uint i = 0; i < num_players; i++) {
            Player player = {"Fake_"+pos+char('1'+i), pos, 0, 0};
            p_list.push_back(player);
        }
        return p_list;
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


/*
 * Reads the players database in data_base.txt and returns a vector of all the players (name, position, price and points)
 */
PlayerList read_players_list()
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


int fitness(Solution solution){ // returns the fitness of a solution
    if (not solution.is_valid()) return 0; // penalize the solutions that exceed the maximum cost
    return solution.get_points();
}

pair<Solution, Solution> select_parents(Population population) {
    // Select two random solutions from the population
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, population.size() - 1);
    
    int index1 = dist(gen);
    int index2 = dist(gen);

    Solution solution1 = population[index1];
    Solution solution2 = population[index2];
    return {solution1, solution2};
}

Population recombine(Solution solution1, Solution solution2) {
    Population combined_solutions;
    Solution new_solution = solution1;
    
    for (auto pos : positions){

        for (uint i = 0; i < new_solution[pos].size(); ++i) {
            if (rand() % 2 == 0) {
                new_solution.remove_player(new_solution[pos][i]); // solution[pos] == solution.player[pos]
                new_solution.add_player(solution2[pos][i]);
            }
        }
        combined_solutions.push_back(new_solution);
    }
    return combined_solutions;
}

void mutate(Solution solution) {
    int mutation_rate = 0.1 * solution.get_size(); 
    for (auto pos : positions) {
        for (Player p : solution[pos]) {
            if ((rand() / static_cast<double>(RAND_MAX)) < mutation_rate){
                solution.remove_player(p);
            }
        }
    }
    return;
}

Population recombine_and_mutate(Solution solution1, Solution solution2) {
    Population combined_solutions = recombine(solution1, solution2);
    for (Solution solution : combined_solutions) {
        mutate(solution);
    }
    return combined_solutions;
}

Population select_individuals(Population solutions, ulong num_selected) {
    Population selected_solutions;

    sort (solutions.begin(), solutions.end(), [](const Solution& s1, const Solution& s2) {
        return fitness(s1) > fitness(s2);
    });

    for (int i = 0; i < min(num_selected, solutions.size()); i++) {
        selected_solutions.push_back(solutions[i]);
    }
    return selected_solutions;
    // while (num_selected < 2) {
    //     int index = rand() % solutions.size();
    //     Solution solution = solutions[index];
    //     if (rand() / static_cast<double>(RAND_MAX) < fitness(solution)) {
    //         selected_solutions.push_back(solution);
    //         num_selected++;
    //     }
    // }
    // return selected_solutions;
}

Population generate_initial_population() { // Use greedy algorithm to generate an initial solution
    Solution solution;
    const double alpha = 1.6 * pow(query.max_cost / 75000000, 2); // the greedy algorithm works best with this parameter

    sort(player_list.begin(), player_list.end(), [alpha](const Player& p1, const Player& p2) {
        return pow(p1.points, alpha + 1) / p1.price > pow(p2.points, alpha + 1) / p2.price;
    });

    for (Player p : player_list) {
        if (solution.can_be_added(p)) {
            solution.add_player(p);
        }
    }
    return {solution};
}

void metaheuristica(int num_selected) {

    Population population = generate_initial_population();
    uint gen = 0;
    while (true){
        cout << gen++ << endl;
        auto [parent1, parent2] = select_parents(population);
        Population population = recombine_and_mutate(parent1, parent2);
        population = select_individuals(population, num_selected);
        
        for (auto solution : population) {
            if (solution.get_points() > best_solution.get_points()) {
                best_solution = solution;
                best_solution.write();
            }
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
    player_list = read_players_list(); // store all the players' info

    ulong num_selected = 100; // parameter: number of solutions selected in each iteration
    metaheuristica(num_selected); 
}