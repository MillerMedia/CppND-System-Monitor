#include "processor.h"
#include "linux_parser.h"
#include <string>
#include <sstream>
#include <vector>

using std::string;

// DONE: Return the aggregate CPU utilization
float Processor::Utilization() {
  std::vector<string> CpuUsages = LinuxParser::CpuUtilization();

  // Reference: https://stackoverflow.com/a/23376195/975592
  float Idle = LinuxParser::IdleJiffies();
  //float NonIdle = LinuxParser::ActiveJiffies();
  float Total = LinuxParser::Jiffies();

  return (Total - Idle)/Total;
}