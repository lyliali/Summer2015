//A simple interface for communication with the monitors
#ifndef ereader_h
#define ereader_h

#include <string>
#include <vector>

using namespace std;

//An individual reading from one of the associated devices
struct reading {
  string deviceID; //Name of the device
  double data;     //Current power
};

//An example of an ereader is the egauge device with many associated readers
class ereader {

  public:
    virtual void init() = 0;
    virtual string getType() = 0;
    virtual int getNumDevices() = 0;
    virtual vector<reading> getNewReadings() = 0;
    virtual void shutdown() = 0;
};
#endif
