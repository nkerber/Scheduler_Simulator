#include <queue>
#include <thread>
#include <unistd.h>
#include <mutex>
#include <iostream>
#include <fstream>

using namespace std;

int main() {

    fstream inputfile;

    int arrivalTime;
    int pid;
    vector<int> cpuTimes;
    vector<int> ioTimes;

    inputfile.open("random_pids.csv");
        if (inputfile.is_open()) {
            string li;
            while (inputfile >> li) {
                //cout << "currently analyzing the word: |" << li << "|" << endl;
                if (li == "PID") {

                    if (!cpuTimes.empty()) {
                        PT.add(new PCBFile(pid, arrivalTime, &cpuTimes, &ioTimes));
                        cpuTimes.clear();
                        ioTimes.clear();
                    }

                    inputfile >> li;
                    
                    cout << "PID is: " << li.substr(0,li.length()-1) << endl;
                    pid = stoi(li.substr(0,li.length()-1));
                } else if (li == "Arrival") {
                    inputfile >> li;
                    cout << "Arrival is: " << li.substr(0,li.length()-1) << endl;
                    pid = stoi(li.substr(0,li.length()-1));
                } else if (li == "CPU") {
                    inputfile >> li;
                    //cout << "currently analyzing the word: |" << li << "|" << endl;
                    //cout << "Found new CPU burst: " << li << endl;
                    if (li.find(',') && li.length() != 1) {
                        cpuTimes.push_back(stoi(li.substr(0,li.length()-1)));
                    } else cpuTimes.push_back(stoi(li));
                } else if (li == "IO") {
                    inputfile >> li;
                    //cout << "Found new IO burst: " << li << endl;
                    if (li.find(',') && li.length() != 1) {
                        ioTimes.push_back(stoi(li.substr(0,li.length()-1)));
                    } else ioTimes.push_back(stoi(li));
                } else {
                    //cout << "Fatal error in file import" << endl;
                    throw -1;
                }
            }
            if (!cpuTimes.empty()) {
                PT.add(new PCBFile(pid, arrivalTime, &cpuTimes, &ioTimes));
            }
        }
    return 0;
}
