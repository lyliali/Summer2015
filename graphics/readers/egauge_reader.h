/*
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/
#include "ereader.h"
#include <curl/curl.h>

class egauge_reader : public ereader {
  private:
    string ip;
    CURL *curl;
    size_t numDevices;
    static size_t readFunc(char *ptr, size_t size, size_t nmeb, string *buf);

  public:
    egauge_reader();
    egauge_reader(string ip);
    ~egauge_reader();
    string query();
    string getType();
    void init();
    int getNumDevices();
    vector<reading> getNewReadings();
    void shutdown();
};
