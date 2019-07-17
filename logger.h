#ifndef __LOGGER__
#define __LOGGER__

#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <map>

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
  static bool stat; //!< if true, statistics will be displayed, e.g. number of star vertices / edges or the number of partition graphs etc
  static ostream* verbose_stream; //!< the stream for printing entries at the time of arrival, default is cout
  static bool report; //!< if true, at the end of the program, a hierarchical report is printed 
  static ostream* report_stream; //!< the stream to report the final report. Default is cout
  static ostream* stat_stream; //!< the stream to report the statistics. Default is cout
  static vector<log_entry> logs;
  static int current_depth;
  static void add_entry(string name, string description);
  static void start();
  static void stop();
  static void log();
  static void item_start(string name);
  static void item_stop(string name);
  static map<string, float> item_duration; //<! maps the title of each item to its duration
  static map<string, high_resolution_clock::time_point> item_last_start; //!< the last time each item was started
};


void __attribute__ ((constructor)) prog_start();
void __attribute__ ((destructor))  prog_finish();

#endif
