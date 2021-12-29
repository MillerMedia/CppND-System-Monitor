#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>

#include "process.h"
#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// DONE: Return this process's ID
int Process::Pid() { return pid_; }

// DONE: Return this process's CPU utilization
float Process::CpuUtilization() {
    string line, token;
    float starttime;
    float cpu_usage;

    // Get all relevant values from the stat file
    std::ifstream file(LinuxParser::kProcDirectory + to_string(pid_) + LinuxParser::kStatFilename);
    starttime = LinuxParser::UpTime(pid_);

    // Calculation reference: https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
    float active_jiffies = LinuxParser::ActiveJiffies(pid_);
    float Hertz = sysconf(_SC_CLK_TCK);
    float seconds = LinuxParser::UpTime() - (starttime / Hertz);
    cpu_usage = 100 * (active_jiffies / seconds);

    /*std::cout << "Total time: " << total_time << std::endl;
    std::cout << "Hertz: " << Hertz << std::endl;
    std::cout << "Seconds: " << seconds << std::endl;
    std::cout << "CPU Usage: " << cpu_usage << std::endl;*/

    return cpu_usage;
    

    return 0;
}

// DONE: Return the command that generated this process
string Process::Command() { return LinuxParser::Command(pid_); }

// DONE: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(pid_); }

// DONE: Return the user (name) that generated this process
string Process::User() { return LinuxParser::User(pid_); }

// DONE: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(pid_); }

// DONE: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
    return a.cpu_ < this->cpu_ ? true : false;
}