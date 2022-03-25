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
            while (inputfile >> li)
            {
                if (li == "PID") {
                    inputfile >> li;
                    
                    cout << "PID is: " << li.substr(0,li.length()-1) << endl;
                    pid = stoi(li.substr(0,li.length()-1));
                }
                cout << li << endl;
            }
        }
    return 0;
}

// stringName.substr(0, stringName.find(' ')) -- First entry
// stringName.substr(stringName.find(' ')+1, stringName.find(' ')) -- Second (can shorten string to first stringName.find(' ')) to use this in a loop
// 
// 
// 
// 
// 
// 
// 
