/*
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/

#include "ereader.h"
#include <curl/curl.h>

class wemo_reader : public ereader {
  private:
    string ip_addr;
    CURL *curl;
    struct curl_slist *headers_chunk;
    string post_body;
    string control;
    string port;
    static size_t readFunc(char *ptr, size_t size, size_t nmeb, string *buf);
    size_t find_nth(string s,string delim, int n);

  public:
    wemo_reader();
    wemo_reader(string ip);
    ~wemo_reader();
    string query(string port_num);
    string getType();
    void init();
    int getNumDevices();
    vector<reading> getNewReadings();
    void shutdown();
};
