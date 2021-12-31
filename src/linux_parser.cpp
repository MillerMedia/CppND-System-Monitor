#include <dirent.h>
#include <unistd.h>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <math.h>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;
using std::stol;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, version;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// DONE: Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  // Parse proc/meminfo file
  // Reference: https://stackoverflow.com/a/350046
  string token;
  unsigned long memtotal;
  unsigned long memfree;
  std::ifstream file(kProcDirectory + kMeminfoFilename);
  
  if(file){
    while(file >> token) {
        unsigned long mem;
        if(token == filterMemTotalString) {
          if(file >> mem){
            memtotal = mem;
          }
        }
        if(token == filterMemFreeString) {
          if(file >> mem){
            memfree = mem;
          }
        }
        // Ignore the rest of the line
        file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    ///std::cout << "MemTotal: " << memtotal << "\nMemFree: " << memfree << std::endl;
    //std::cout << "Calculation: " << float(memtotal - memfree)/memtotal << std::endl;
    return float(memtotal - memfree)/memtotal;
  }

  // There was a problem; return 0 as a default
  return 0.0;
}

// DONE: Read and return the system uptime
long LinuxParser::UpTime() {
  long uptime, idletime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime >> idletime;
  }
  
  return uptime;
}

// DONE: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

// DONE: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  string line, token;
  long utime, stime, cutime, cstime;
  
  // Get all relevant values from the stat file
  std::ifstream file(LinuxParser::kProcDirectory + to_string(pid) + LinuxParser::kStatFilename);
  
  if (file.is_open()) {
    while (std::getline(file, line)) {
      std::istringstream linestream(line);
        
      for (int i = 1; i < 23; ++i){
        // Reference: https://stackoverflow.com/a/2381188/975592
        linestream >> token;
        switch(i){
          case 14:
            utime = stol(token);
            break;
          case 15:
            stime = stol(token);
            break;
          case 16:
            cutime = stol(token);
            break;
          case 17:
            cstime = stol(token);
            break;
          default:
            break;
        }
      }
    }
  }

    // Calculation reference: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599            
    long total_time = utime + stime + cutime + cstime;
    long Hertz = sysconf(_SC_CLK_TCK);
  
    // Divided by 'Hertz' per the man page: https://man7.org/linux/man-pages/man5/proc.5.html
    return total_time / Hertz;
}

// DONE: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> CpuUtil = CpuUtilization();
  
  // Getting 'non-idle' values from reference: https://stackoverflow.com/a/23376195/975592
  // NonIdle = user + nice + system + irq + softirq + steal;
  // Converting from string to long; i.e. stol()
  return stol(CpuUtil[CPUStates::kUser_]) + stol(CpuUtil[CPUStates::kNice_]) + stol(CpuUtil[CPUStates::kSystem_]) + stol(CpuUtil[CPUStates::kIRQ_]) + stol(CpuUtil[CPUStates::kSoftIRQ_]) + stol(CpuUtil[CPUStates::kSteal_]);
}

// DONE: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> CpuUtil = CpuUtilization();
  
  // Getting 'idle' values from reference: https://stackoverflow.com/a/23376195/975592
  // Idle = idle + iowait;
  // Converting from string to long; i.e. stol()
  return stol(CpuUtil[CPUStates::kIdle_]) + stol(CpuUtil[CPUStates::kIOwait_]);
}

// DONE: Read and return CPU utilization
// Returned in 'jiffies'
vector<string> LinuxParser::CpuUtilization() {
    string line;
    string key, value;
    vector<string> CpuUsage;
    std::ifstream filestream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);
    if (filestream.is_open()) {
      while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        while (linestream >> key) {
          if (key == filterCpu) {
            while (linestream >> value){
            	CpuUsage.emplace_back(value);
            }
            
            return CpuUsage;
          }
        }
      }
    }

    return {};

 }

// DONE: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string token;
  int processes;
  std::ifstream file(kProcDirectory + kStatFilename);
  
  if(file){
    while(file >> token) {
        if(token == filterProcesses) {
          if(file >> processes){
            return processes;
          }
        }
    }
  }

  return 0;
}

// DONE: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string token;
  int procs_running;
  std::ifstream file(kProcDirectory + kStatFilename);
  
  if(file){
    while(file >> token) {
        if(token == filterRunningProcesses) {
          if(file >> procs_running){
            return procs_running;
          }
        }
    }
  }

  return 0;
}

// DONE: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string command;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);

  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> command;
    return command;
  }

  return string();
}

// DONE: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string token;
  double ram_kb;
  string ram_mb;

  std::ifstream file(kProcDirectory + to_string(pid) + kStatusFilename);
  
  if(file){
    while(file >> token) {
      if(token == filterProcMem) {
        if(file >> ram_kb){
          ram_mb = to_string(ram_kb*0.001);

          // Remove trailing zeroes
          // Reference: https://stackoverflow.com/a/13709929/975592
          return ram_mb.erase(ram_mb.find_last_not_of('0') + 1, std::string::npos);
        }
      }
    }
  }

  return string();
}

// DONE: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string token;
  string uid;
  std::ifstream file(kProcDirectory + to_string(pid) + kStatusFilename);
  
  if(file){
    while(file >> token) {
      if(token == filterUID) {
        if(file >> uid){
          return uid;
        }
      }
    }
  }

  return string();
}

// DONE: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);  
  string line;
  string username, x, user_id;
  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> username >> x >> user_id) {
        if (user_id == uid) {
          return username;
        }
      }
    }
  }

  return string();
}

// DONE: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, token;
  std::ifstream file(kProcDirectory + to_string(pid) + kStatFilename);
  
  if (file.is_open()) {
    while (std::getline(file, line)) {
      std::istringstream linestream(line);
      
      // Get 22nd item
      // Reference: https://stackoverflow.com/a/2381188/975592
      for (int i = 0; i < 22; ++i)
        linestream >> token;

      float starttime = std::stol(token);
      float Hertz = sysconf(_SC_CLK_TCK);
      float seconds = LinuxParser::UpTime() - (starttime / Hertz);
      return seconds;
    }
  }

  return 0;
}
