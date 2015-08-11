#include "ereader.h"
#include "egauge_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <cstring>

egauge_reader::egauge_reader(){
  perror("Error - You must construct egauge_reader with an ip address\n");
}

egauge_reader::egauge_reader(string ip){
  egauge_reader::ip = ip;
  egauge_reader::init();
}


egauge_reader::~egauge_reader(){
  egauge_reader::shutdown();
}

string egauge_reader::getType(){
  return "egauge";
}

int egauge_reader::getNumDevices(){
  return egauge_reader::numDevices;
}

void egauge_reader::init(){

  curl_global_init(CURL_GLOBAL_ALL);
  egauge_reader::curl = curl_easy_init();
  if (!curl){
    perror("Could not create a curl object.\n");
    curl_global_cleanup();
    exit(1);
  }
  
  string count_test = egauge_reader::query();
  egauge_reader::numDevices = 0;
  size_t nPos = count_test.find("/power",0);
  while (nPos != string::npos){
    egauge_reader::numDevices++;
    nPos = count_test.find("/power",nPos+1);
  }
}

string egauge_reader::query(){
  string url;
  string buffer = "";

  url = egauge_reader::ip + "/cgi-bin/egauge?noteam";
  curl_easy_setopt(curl,CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, egauge_reader::readFunc);
  curl_easy_setopt(curl,CURLOPT_WRITEDATA, &buffer);
  curl_easy_perform(curl);
  
  return buffer;
}

//Could the order of devices change? That would be annoying.
vector<reading> egauge_reader::getNewReadings(){
  vector<reading> readings;

  string full = egauge_reader::query();
  size_t nPos = full.find("<meter title=\"",0);
  while (nPos != string::npos){
    reading cur;
    size_t mPos = full.find("\">",nPos+1);
    cur.deviceID = full.substr(nPos+14,mPos - nPos - 14);
    nPos = full.find("<power>",mPos+1);
    mPos = full.find("</power>",nPos+1);
    cur.data = -1.0 * min(0.0,atof(full.substr(nPos+7,mPos - nPos - 7).c_str()));    
    readings.push_back(cur);
    nPos = full.find("<meter title=\"",mPos+1);
  }
  
  return readings;
}

//NOTE: This is only called if something has been read!
size_t egauge_reader::readFunc(char *ptr, size_t size, size_t nmeb, string *buf){
  if (buf == NULL){
    printf("%s\n", "buf was NULL");
    return 0;
  }
  buf->append(ptr, size*nmeb);
  return size*nmeb;
}

void egauge_reader::shutdown(){
  if (egauge_reader::curl){
    curl_easy_cleanup(egauge_reader::curl);
    curl_global_cleanup();
  }
  egauge_reader::ip = "";
}

/*
int main(int argc, char**argv){
  egauge_reader* one = new egauge_reader("http://egauge1474.egauge.cs.umass.edu");
  vector<reading> roundone = one->getNewReadings();
  for (int i = 0; i < roundone.size(); i++){
    reading cur = roundone[i];
    printf("Device Id: %s, Value: %f\n",cur.deviceID.c_str(), cur.data);
  }
  return 0;
}
*/

