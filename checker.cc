#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cassert>
using namespace std;

void check(bool cond, string msg) {
  if (not cond) {
    cout << "ERROR: " << msg << endl;
    exit(1);
  }
}


class Player {
public:
  int    id;
  string name;
  string position;
  int    price;
  string club;
  int    points;

  Player(int ident, const string& n, const string& pos, int pr, const string& c, int p):
    id(ident), name(n), position(pos), price(pr), club(c), points(p){}

  friend ostream & operator << (ostream &out, const Player& p) {
    out << p.name << "(id " << p.id << ") price = " << p.price << " points = " << p.points;
    return out;
  }
};

class Tactic {
public:
  uint goal;
  uint def;
  uint mid;
  uint str;
  Tactic() {}
  Tactic(uint g, uint d, uint m, uint s):goal(g), def(d), mid(m), str(s) {}
};

vector<Player> id2player;

Tactic tactic;
int maxTotalPrice;
int maxIndivPrice;

string removeBlanks(const string& s){
  uint i = 0;
  while (i < s.length() and (s[i] == ' ' or s[i] == '\t')) ++i;
  // i is now the first non-blank
  
  uint j = s.length() - 1 ;
  while (j >= 0 and (s[j] == ' ' or s[j] == '\t')) --j;
  // j is now the last non-blank

  return s.substr(i,j-i+1);
}

bool playerPresent (const string& name, const string& pos) {
  for (auto& p:id2player) {
    if (p.name == name and p.position == pos) return true;
  }
  return false;
}

int price (const string& s) {
  for (auto& p:id2player) 
    if (p.name == s) return p.price;
  return 0;
}

int points (const string& s) {
  for (auto& p:id2player) 
    if (p.name == s) return p.points;
  return 0;
}

int main(int argc, char** argv) {
  if (argc != 4) {
    cout << "Syntax: " << argv[0] << " data_base.txt query.txt solution.txt" << endl;
    exit(1);
  }

  // Read data base
  ifstream in(argv[1]);
  int nextId = 0;
  while (not in.eof()) {
    string name, club, position;
    int p;
    getline(in,name,';');    if (name == "") break;
    getline(in,position,';');
    int price; in >> price;
    char aux; in >> aux;
    getline(in,club,';');
    in >> p;
    string aux2;
    getline(in,aux2);
    id2player.push_back(Player(nextId++,name,position,price,club,p));
  }
  in.close();

  // Read query
  ifstream in2(argv[2]);
  uint nDef, nMig, nDav;
  in2 >> nDef >> nMig >> nDav >> maxTotalPrice >> maxIndivPrice;
  tactic = Tactic(1,nDef,nMig,nDav);
  in2.close();

  // Read solution
  ifstream in3(argv[3]);
  double time; in3 >> time;
  // Read one goalkeeper
  vector<string> goa, def, mig, dav ;
  string aux; in3 >> aux; check(aux == "POR:", "Esperava token \"POR:\" i s'ha trobat \"" + aux + "\"");
  string nom;
  getline(in3,nom); nom = removeBlanks(nom); goa.push_back(nom);

  in3 >> aux; check(aux == "DEF:", "Esperava token \"DEF:\" i s'ha trobat \"" + aux + "\"");
  for (uint i = 0; i < nDef - 1; ++i) {
    getline(in3,nom,';'); nom = removeBlanks(nom); def.push_back(nom);
  }
  getline(in3,nom); nom = removeBlanks(nom); def.push_back(nom);

  in3 >> aux; check(aux == "MIG:", "Esperava token \"MIG:\" i s'ha trobat \"" + aux + "\"");
  for (uint i = 0; i < nMig - 1; ++i) {
    getline(in3,nom,';'); nom = removeBlanks(nom); mig.push_back(nom);
  }
  getline(in3,nom); nom = removeBlanks(nom); mig.push_back(nom);

  in3 >> aux; check(aux == "DAV:", "Esperava token \"DAV:\" i s'ha trobat \"" + aux + "\"");
  for (uint i = 0; i < nDav - 1; ++i) {
    getline(in3,nom,';'); nom = removeBlanks(nom); dav.push_back(nom);
  }
  getline(in3,nom); nom = removeBlanks(nom); dav.push_back(nom);

  in3 >> aux; check(aux == "Punts:", "Esperava token \"Punts:\" i s'ha trobat \"" + aux + "\"");
  int punts; in3 >> punts;
  in3 >> aux; check(aux == "Preu:", "Esperava token \"Preu:\" i s'ha trobat \"" + aux + "\"");
  int preu; in3 >> preu;
  in3.close();

  check(goa.size() == 1, "L'alineació hauria de tenir 1 porter");
  check(def.size() == nDef, "L'alineació hauria de tenir " + to_string(nDef) + " defenses");
  check(mig.size() == nMig, "L'alineació hauria de tenir " + to_string(nMig) + " migcampistes");
  check(dav.size() == nDav, "L'alineació hauria de tenir " + to_string(nDav) + " davanters");

  for (auto& s:goa) check(playerPresent(s,"por"),"El jugador " + s + " no es troba a la base de dades com a porter");
  for (auto& s:def) check(playerPresent(s,"def"),"El jugador " + s + " no es troba a la base de dades com a defensa");
  for (auto& s:mig) check(playerPresent(s,"mig"),"El jugador " + s + " no es troba a la base de dades com a migcampista");
  for (auto& s:dav) check(playerPresent(s,"dav"),"El jugador " + s + " no es troba a la base de dades com a davanter");

  int realPoints = 0;
  int realPrice = 0;

  for (auto& s:goa) {realPrice += price(s); realPoints += points(s); check(price(s) <= maxIndivPrice, "El jugador " + string(s) + " te preu " + to_string(price(s)) + " que es major que el maxim " + to_string(maxIndivPrice));}
  for (auto& s:def) {realPrice += price(s); realPoints += points(s);check(price(s) <= maxIndivPrice, "El jugador " + string(s) + " te preu " + to_string(price(s)) + " que es major que el maxim " + to_string(maxIndivPrice)); }
  for (auto& s:mig) {realPrice += price(s); realPoints += points(s);check(price(s) <= maxIndivPrice, "El jugador " + string(s) + " te preu " + to_string(price(s)) + " que es major que el maxim " + to_string(maxIndivPrice));}
  for (auto& s:dav) {realPrice += price(s); realPoints += points(s);check(price(s) <= maxIndivPrice, "El jugador " + string(s) + " te preu " + to_string(price(s)) + " que es major que el maxim " + to_string(maxIndivPrice));}

  check(realPoints == punts, "L'arxiu de solució reporta un total de punts de " + to_string(punts) + " pero els punts reals de l'alineació són " + to_string(realPoints));
  check(realPrice == preu, "L'arxiu de solució reporta un preu de " + to_string(preu) + " pero el preu real de l'alineació és de " + to_string(realPrice));
  check(realPrice <= maxTotalPrice, "L'arxiu de solució te un equip amb un preu de " + to_string(realPrice) + " que es major que el permes " + to_string(maxTotalPrice));

  
  cout << "OK" << endl;
  
}
