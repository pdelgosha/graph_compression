#ifndef __LOGGER__
#define __LOGGER__

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>

using namespace std;
using namespace std::chrono;

class log_entry{
 public:
  string name;
  string description;
  int depth;
  high_resolution_clock::time_point t;
  system_clock::time_point sys_t;
  log_entry(string name, string description, int depth);
};

class logger{
 public:
  static bool verbose; //!< if true, every entry is printed at the time of its report, default is false
  static ostream& verbose_stream; //!< the stream for printing entries at the time of arrival, default is cout
  static bool report; //!< if true, at the end of the program, a hierarchical report is printed 
  static ostream& report_stream; //!< the stream to report the final report. Default is cout 
  static vector<log_entry> logs;
  static int current_depth;
  static void add_entry(string name, string description);
  static void start();
  static void stop();
  static void log();
};


void __attribute__ ((constructor)) prog_start();
void __attribute__ ((destructor))  prog_finish();

#endif
