#include "logger.h"

vector<log_entry> logger::logs;
int logger::current_depth = 1; // use depth 0 only for start and finish entries
bool logger::verbose = true;
ostream& logger::verbose_stream = cout;
bool logger::report = true;
ostream& logger::report_stream = cout; 

map<string, float> logger::item_duration;
map<string, high_resolution_clock::time_point> logger::item_last_start;


log_entry::log_entry(string name_, string description_, int depth_){
  name = name_;
  description = description_;
  depth = depth_;
  t = high_resolution_clock::now();
  sys_t = system_clock::now();
}


void logger::add_entry(string name, string description)
{
  //cerr << " adding entry current_depth " << current_depth << endl;
  log_entry new_entry(name, description, current_depth);
  logger::logs.push_back(new_entry);
  if (logger::verbose){
    string s = "";
    time_t tt = system_clock::to_time_t(new_entry.sys_t);
    char buffer[80];
    strftime(buffer, 80, "%F %r", localtime(&tt));
    for (int i=1;i<current_depth;i++)
      s += "|---";
    string buffer_str(buffer);
    s +=  name + " (" + description + ") ";
    s += buffer_str;
    verbose_stream << s << endl;
  }
}


void logger::log(){
  //cerr << " log started " << endl;
  int max_depth = 0;
  for (int i=0;i<logs.size(); i++){
    if (logs[i].depth > max_depth)
      max_depth = logs[i].depth;
  }

  vector<int> parent(logs.size()); // for a log L, parent[L] is the max index i < L such that depth[i] < depth[L], this shows in what block we are in
  vector<int> next(logs.size()); // for a log L, next[L] is the min index i > L such that depth[i] >= depth[L], this is the index right after L in its block, or if L is the last entry in its block, it is the start of the next block. The diff between L and next[L] shows the duration of running L
  for (int i=1;i<(logs.size()-1);i++){
    // finding parent[i]
    //cerr << " finding parent " << i << endl;
    for (int j=(i-1); j>=0 ;j--){
      if (logs[j].depth < logs[i].depth){
        parent[i] = j;
        break;
      }
    }
    //cerr << " parent[" << i << "] = " << parent[i] << endl;
    //cerr << " finding next " << i << endl;
    for (int j=(i+1); j<logs.size();j++){
      if (logs[j].depth <= logs[i].depth){
        next[i] = j;
        break;
      }
    }
    //cerr << " next[" << i << "] = " << next[i] << endl;
  }
  next[0] = logs.size()-1; // next of start entry is the finish entry

  vector<float> dur(logs.size()); // dur[L] is the duration that entry L takes, meaning the difference between L and next[L]
  vector<float> block_dur(logs.size()); // the duration of the whole block for each entry, which is the difference between
  vector<float> block_percent(logs.size()); // the percentage of time each entry takes inside a block
  duration<float> diff;
  string s;
  //cerr << " logs.size() " << logs.size() << endl;
  report_stream << endl;
  for (int i=1;i<(logs.size()-1); i++){
    //cerr << " i " << i << endl;
    // finding duration[i]
    diff = logs[next[i]].t - logs[i].t;
    dur[i] = diff.count();
    //cerr << " dur[i] " << dur[i] << endl;
    // finding block_duration 
    diff = logs[next[parent[i]]].t - logs[parent[i]].t;
    block_dur[i] = diff.count();
    //cerr << " block_dur[i] " << block_dur[i] << endl;
    block_percent[i] = dur[i] / block_dur[i] * 100;
    //cerr << " block_percent[i] " << block_percent[i] << endl;
    s = "";
    //cerr << " logs[i].depth " << logs[i].depth << endl;
    for (int j=1;j<logs[i].depth; j++)
      s += "|---";
    s +=  logs[i].name + " (" + logs[i].description + "): " + to_string(dur[i]) + "s " + "[" + to_string(block_percent[i]) + "\%]" + " ";
    report_stream << s << endl;
  }

  report_stream << endl << " itemized log " << endl;
  for (map<string, float>::iterator it = item_duration.begin(); it!= item_duration.end(); it++)
    report_stream << it->first << " : " << it->second << endl;
}

//void __attribute__ ((constructor)) prog_start(){
void logger::start(){
  log_entry new_log("Start", "", 0);
  //cerr << " started " << endl;
  logs.push_back(new_log);
  //cerr << logger::logs.size() << endl;
}

//void __attribute__ ((destructor))  prog_finish(){
void logger::stop(){
  log_entry new_log("Finish", "", 0);
  //cerr << " finished " << endl;
  logs.push_back(new_log);
  //cerr << logs.size() << endl;
  if (report)
    log();

}


void logger::item_start(string name){
  item_last_start[name] = high_resolution_clock::now();
}

void logger::item_stop(string name){
  high_resolution_clock::time_point t = high_resolution_clock::now();
  duration<float> diff = t - item_last_start[name];
  item_duration[name] += diff.count();
}
