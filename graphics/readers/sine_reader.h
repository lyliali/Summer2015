/*
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/  
#include "ereader.h"

class sine_reader : public ereader {
  private:
    double t;

  public:
    sine_reader();
    ~sine_reader();
    string getType();
    void init();
    int getNumDevices();
    vector<reading> getNewReadings();
    void shutdown();
};
