#ifndef ereader_h
#define ereader_h

#include <string>
#include <vector>

using namespace std;

struct reading {
  string deviceID;
  double data;
};

class ereader {

  public:
    virtual void init() = 0;
    virtual string getType() = 0;
    virtual int getNumDevices() = 0;
    virtual vector<reading> getNewReadings() = 0;
    virtual void shutdown() = 0;
};
#endif
