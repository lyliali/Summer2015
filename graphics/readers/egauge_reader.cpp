/*
A driver for the egauge monitoring system.
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/
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
  //Initialize a curl object for easy packet sending.
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

  //This might not always work! This isn't the most current version of the data format according to egauge.
  //But, it is the most convenient for us in the moment.
  url = egauge_reader::ip + "/cgi-bin/egauge?noteam";
  curl_easy_setopt(curl,CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, egauge_reader::readFunc);
  curl_easy_setopt(curl,CURLOPT_WRITEDATA, &buffer);
  curl_easy_perform(curl);
  
  return buffer;
}

//If the order of the devices returned change in the response of the egauge monitor, 
//this will cause an issue with the current code! (haven't seen that in practice)
vector<reading> egauge_reader::getNewReadings(){
  vector<reading> readings;

  string full = egauge_reader::query();
  //Find the first meter
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

//NOTE: This is only called if something has been read by CURL (I would not mess with it).
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

