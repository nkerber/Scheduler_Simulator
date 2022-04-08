#ifndef globals
#define globals

#define RUNTIME 5
#define RANDPROCESSNUM 1000
#define queueCount 5
#define NUMCORES 64
#define USEDATAFILE false
#define FCFS true
#define TIMERMAX 100000
#define TIMEQUANTUM 1

// Access enum using PSTATE::[NEW/READY/RUNNING/BLOCKED/EXIT]
enum PSTATE{NEW, READY, RUNNING, BLOCKED, EXIT};

#endif