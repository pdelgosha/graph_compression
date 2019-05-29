#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "random_graph.h"

using namespace std;

void print_usage(){
  cout << " Usage: " << endl;
  cout << " rnd_graph -t [random graph type] -n [number of vertices] [[options with -p / -d]] -e [number of edge marks] -v [number of vertex marks] -o [output file]" << endl;
  cout << endl;
  cout << " OPTIONS " << endl;
  cout << " -n : number of vertices (positive integer) " << endl;
  cout << " -e : size of edge mark set " << endl;
  cout << " -v : size of vertex mark set " << endl;
  cout << " -o : output file to store graph " << endl;
  cout << " -t : type of random graph: " << endl;
  cout << "           \"er\" for Erdos Renyi " << endl;
  cout << "           \"reg\" for near regular graph " << endl;
  cout << "           \"poi\" for Poisson graph " << endl;
  cout << endl;
  cout << " TYPE SPECIFIC OPTIONS " << endl;
  cout << endl;
  cout << " Erdos Renyi Graph " << endl;
  cout << " ----------------- " << endl;
  cout << " -p : edge probability " << endl;
  cout << endl;
  cout << " Near Regular Graph " << endl;
  cout << " ------------------ " << endl;
  cout << " -d : half degree, generates a random graph which is nearly regular, and the degree of each vertex is close to 2 * half_deg. " << endl;
  cout << endl;
  cout << " Poisson Graph " << endl;
  cout << " ------------- " << endl;
  cout << " -d : mean degree, each vertex chooses Poisson neighbors with this mean " << endl;
  cout << endl;

}



int main(int argc, char ** argv){
  int n, edge_mark, ver_mark;
  double p, deg;
  string type, outfile;
  char opt;

  while ((opt = getopt(argc, argv, "t:n:p:d:e:v:o:h")) != EOF){
    switch(opt){
    case 't':
      type = string(optarg);
      break;
    case 'n':
      n = atoi(optarg);
      break;
    case 'p':
      p = atof(optarg);
      break;
    case 'd':
      deg = atof(optarg);
      break;
    case 'e':
      edge_mark = atoi(optarg);
      break;
    case 'v':
      ver_mark = atoi(optarg);
      break;
    case 'o':
      outfile = string(optarg);
      break;
    case 'h':
      print_usage();
      break;
    case '?':
      cerr << "Error: option -" << char(optopt) << " requires an argument" << endl;
      print_usage();
      return 1;
    }
  }
  ofstream oup(outfile.c_str());
  if (!oup.good()){
    cerr << "Error: invalid output file <" << outfile << "> given " << endl;
    return 1;
  }

  marked_graph G;
  if (type == "er"){
    G = marked_ER(n, p, ver_mark, edge_mark);
    oup << G;
  }else if(type == "reg"){
    G = near_regular_graph(n, deg, ver_mark, edge_mark);
    oup << G; 
  }else if(type == "poi"){
    G = poisson_graph(n, deg, ver_mark, edge_mark);
    oup << G;
  }else{
    cerr << " ERROR: unknown type " << type << ", type must be either <er> or <reg> or <poi> " << endl;
    return 1;
  }
  return 0;
}
