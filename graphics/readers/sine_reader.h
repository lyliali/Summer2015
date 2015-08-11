// An example of a ereader, written or testing and debugging purposes.                                                   
// Devin Gardella - Summer 2015      
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
