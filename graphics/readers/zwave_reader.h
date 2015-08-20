/*
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/  
#include "ereader.h"

class zwave_reader : public ereader {
  private:
    FILE* fp;
    pid_t child;
    int numDev;
  
  public:
    zwave_reader();
    ~zwave_reader();
    string getType();
    void init();
    int getNumDevices();
    string getNext();
    vector<reading> getNewReadings();
    void shutdown();
};
