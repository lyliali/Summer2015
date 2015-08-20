/*
The interface we designed to communicate with electronic monitors.
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/
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

class ereader {

  public:
    virtual void init() = 0; //Initializes and tests connections with the devices
    virtual string getType() = 0; //Returns the name of the ereader
    virtual int getNumDevices() = 0;
    virtual vector<reading> getNewReadings() = 0;
    virtual void shutdown() = 0;
};
#endif
