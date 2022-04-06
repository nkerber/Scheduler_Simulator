#ifndef PROCESSTABLE
#define PROCESSTABLE

#define PTSIZE 10000

#include "./pcb.cpp"

class processtable{
    private:
        PCBFile* processes[PTSIZE];
        PCBFile* oldProcesses[2*PTSIZE];
        int activeCount;

    public: 
        processtable(){
            activeCount = 0;

            for(int i = 0; i < PTSIZE; i++){
                processes[i] = nullptr;
            }
        }

        ~processtable(){
            for(int i = 0; i < PTSIZE; i++){
                if(processes[i] != nullptr){
                    delete processes[i];
                    processes[i] = nullptr;
                }
                if(oldProcesses[i] != nullptr){
                    delete oldProcesses[i];
                    oldProcesses[i] = nullptr;
                }
            }
        }

        bool add(PCBFile* in){
            int t = hasSpace();
            if(t >= 0){
                processes[t] = in;
                ++activeCount;
                return true;
            }

            return false;
        }

        bool remove(int i){
            if(processes[i] != nullptr){
                transferProcess(processes[i]);
                processes[i] = nullptr;
                --activeCount;
                return true;
            }

            return false;
        }

        PCBFile* getOld(int i){
            return oldProcesses[i];
        }

        PCBFile* operator[](const int i){
            return processes[i];
        }

        int size(){
            return activeCount;
        }

        int oldSize(){
            for(int i = 0; i < 2*PTSIZE; i++){
                if(oldProcesses[i] == nullptr)
                    return i;
            }
            return 0;
        }

    private:

        void transferProcess(PCBFile* in){
            for(int i = 0; i < 2*PTSIZE; i++){
                if(oldProcesses[i] == nullptr){
                    oldProcesses[i] = in;
                    return;
                }
            }
        }

        int hasSpace(){
            for(int i = 0; i < PTSIZE; i++){
                if(processes[i] == nullptr){
                    return i;
                }
            }

            return -1;
        }
};

#endif