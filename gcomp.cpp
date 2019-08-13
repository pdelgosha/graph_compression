/*!
  \file gcomp.cpp
  \brief To compress / decompress simple marked graphs.

  This code has two functionalities: 1) gets a simple marked graph and compresses it, 2) gets a simple marked graph in its compressed form and decompresses it.

  In order to compress a graph, the hyperparameters h and delta should be given using -h and -d, respectively. The input graph should be given using -i option, followed by the name of the file containing the graph. Also, the compressed graph will be stored in the file specified by -o option. A graph must be specified using its edge list in the following format: first, the number of vertices comes, then the mark of vertices in order, then each line contains the information on an edge, which is of the form i j x y, meaning there is an edge between vertices i and j, with mark x and y towards i and j, respectively. 

  In order to decompress, the compressed file should be given after -i, the file to store the decompressed graph should be given using -o, and an argument -u for uncompress should be given. 
 */

#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "marked_graph_compression.h"

using namespace std;


int main(int argc, char ** argv){
  int h, delta;
  string infile, outfile;
  bool uncompress = false; // becomes true if -u option is given (to decompress)
  bool quiet = true; // becomes false if -v option is given (verbose)
  bool stat = false; // if true, statistics on the properties of the compressed graph, e.g. number of star vertices / edges or the number of partition graphs will be given
  char opt;

  string report_file, stat_file;
  ofstream report_stream, stat_stream; 

  while ((opt = getopt(argc, argv, "h:d:i:o:uvsV:S:")) != EOF){
    switch(opt){
    case 'h':
      h = atoi(optarg);
      break;
    case 'd':
      delta = atoi(optarg);
      break;
    case 'i':
      infile = string(optarg);
      break;
    case 'o':
      outfile = string(optarg);
      break;
    case 'u':
      uncompress = true; // in the decompression phase 
      break;
    case 'v':
      quiet = false;
      break;
    case 'V':
      report_file = string(optarg);
      if (report_file != ""){
        report_stream.open(report_file);
        logger::report_stream = &report_stream;
      }
      break;
    case 's':
      stat = true;
      break;
    case 'S':
      stat_file = string(optarg);
      if (stat_file != ""){
        stat_stream.open(stat_file);
        logger::stat_stream = &stat_stream;
      }
      break;
    case '?':
      cerr << "Error: option -" << char(optopt) << " requires an argument" << endl;
      return 1;
    }
  }
  if (uncompress == false and h <= 0){
    cerr << "Error: parameter h must be a positive integer. Instead, the value " << h << " was given." << endl;
    return 1;
  }
  if (uncompress == false and delta <= 0){
    cerr << "Error: parameter d (delta) must be a positive integer. Instead, the value " << delta << " was given." << endl;
    return 1;
  }

  ifstream inp(infile.c_str());
  ofstream oup(outfile.c_str());

  if (!inp.good()){
    cerr << "Error: invalid input file <" << infile << "> given " << endl;
    return 1;
  }

  if (!oup.good()){
    cerr << "Error: invalid output file <" << outfile << "> given " << endl;
    return 1;
  }


  if (quiet == true){
    // do not log 
    logger::verbose = false; // no run time log 
    logger::report = false; // no final report
  }

  if (stat == true){
    logger::stat = true;
  } 

  //cout << " h = " << h << " delta = " << delta << " infile = " << infile << " outfile = " << outfile << endl;

  logger::start();
  if (uncompress == false){
    // goal is compression
    logger::current_depth++;
    logger::add_entry("Read Graph", "");
    marked_graph_encoder E(h, delta);
    marked_graph G; // the input graph to be compressed
    inp >> G;
    logger::current_depth--;
    logger::current_depth++;
    logger::add_entry("Encode", "");
    marked_graph_compressed C = E.encode(G);
    logger::current_depth--;
    FILE* f;
    f = fopen(outfile.c_str(), "wb+");
    logger::current_depth++;
    logger::add_entry("Write to binary", "");
    C.binary_write(f);
    fclose(f);
    logger::current_depth--;
  }else{
    // goal is to decompess
    FILE* f;
    f = fopen(infile.c_str(), "rb+");
    marked_graph_compressed C;
    logger::current_depth++;
    logger::add_entry("Read from binary", "");
    C.binary_read(f);
    fclose(f);
    logger::current_depth--;

    logger::current_depth++;
    logger::add_entry("Decode", "");
    marked_graph_decoder D;
    marked_graph G = D.decode(C);
    logger::current_depth--;

    logger::current_depth++;
    logger::add_entry("Write decoded graph to output file","");
    oup << G;
    logger::current_depth--;
  }
  logger::stop();
  return 0;
}
