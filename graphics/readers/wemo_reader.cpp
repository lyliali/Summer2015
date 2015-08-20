/*
A driver for the wemo monitoring system.
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/
#include "ereader.h"
#include "wemo_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <cstring>

wemo_reader::wemo_reader(){
  //Catching empty constructors
  perror("Error - You must construct wemo_reader with an ip address\n");
}

wemo_reader::wemo_reader(string ip){
  wemo_reader::ip_addr = "http://" + ip + ":";
  wemo_reader::init();
}


wemo_reader::~wemo_reader(){

}

string wemo_reader::getType(){
  return "wemo";
}

int wemo_reader::getNumDevices(){
  return 1;
}

void wemo_reader::init(){
  //Constructing the headers and post body defaults for wemo connections
  wemo_reader::headers_chunk = NULL;
  
  wemo_reader::headers_chunk = 
    curl_slist_append(wemo_reader::headers_chunk, "Accept:");
  
  wemo_reader::headers_chunk = 
    curl_slist_append(wemo_reader::headers_chunk,
		      "Content-Type: text/xml; charset=\"utf-8\"");
  
  wemo_reader::headers_chunk = 
    curl_slist_append(wemo_reader::headers_chunk,
	   "SOAPACTION: \"urn:Belkin:service:insight:1#GetInsightParams\"");

  wemo_reader::post_body = "<?xml version='1.0' encoding='utf-8'?>"
                           "<s:Envelope xmlns:s="
                           "'http://schemas.xmlsoap.org/soap/envelope/'"
                           "s:encodingStyle="
                           "'http://schemas.xmlsoap.org/soap/encoding/'>"
                           "<s:Body><u:GetInsightParams "
                           "xmlns:u='urn:Belkin:service:insight:1'>"
                           "</u:GetInsightParams></s:Body></s:Envelope>";
  
  wemo_reader::control =   "/upnp/control/insight1";

  curl_global_init(CURL_GLOBAL_ALL);
  wemo_reader::curl = curl_easy_init();
  if (!curl){
    perror("Could not create a curl object.\n");
    curl_global_cleanup();
    exit(1);
  }

  // Testing on a range of known ports to attempt a connection
  string res;
  for (int i = 49150; i < 49160; i++){
    ostringstream ss;
    ss << i;
    res = wemo_reader::query(ss.str());
    if (! res.empty()) {
      wemo_reader::port = ss.str();
      break;
    }
  }
  if (wemo_reader::port.empty()){
    perror("Could not find a port to connect to. Test your connection");
    exit(1);
  }
}

//Helper function to find the nth delim in a string s.
size_t wemo_reader::find_nth(string s, string delim, int n){
  size_t ans = s.find(delim);
  if (n == 0 || ans == string::npos){
    return s.find(delim);
  }
  return ans + delim.length() +wemo_reader::find_nth(s.substr(ans+delim.length()), delim, --n);
}

//Returns the result of a query to the associated device.
string wemo_reader::query(string port_num){
  string url;
  string buffer = "";

  curl_easy_setopt(curl,CURLOPT_HTTPHEADER,wemo_reader::headers_chunk);
  url = ip_addr + port_num + wemo_reader::control;
  
  curl_easy_setopt(curl,CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl,CURLOPT_POSTFIELDS, wemo_reader::post_body.c_str());
  curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, wemo_reader::readFunc);
  curl_easy_setopt(curl,CURLOPT_WRITEDATA, &buffer);
  curl_easy_perform(curl);
  
  return buffer;
}

//Parses query response and returns it in reading format.
vector <reading> wemo_reader::getNewReadings(){
  reading cur;
  if (! wemo_reader::ip_addr.empty()){
    cur.deviceID = "wemo";
    string response = wemo_reader::query(wemo_reader::port);
    size_t i = wemo_reader::find_nth(response,"|",6) + 1;
    size_t e = wemo_reader::find_nth(response,"|",7);
    response = response.substr(i,e-i);
    cur.data = atof(response.c_str()) / 1000;
  }
  else {
    perror("Can not read without a valid ip address\n");
  }
  vector<reading> readings;
  readings.push_back(cur);
  return readings;
}

//CURL code to deal with responses. 
size_t wemo_reader::readFunc(char *ptr, size_t size, size_t nmeb, string *buf){
  if (buf == NULL){
    printf("%s\n", "buf was NULL");
    return 0;
  }
  buf->append(ptr, size*nmeb);
  return size*nmeb;
}

void wemo_reader::shutdown(){
  if (wemo_reader::curl){
    curl_easy_cleanup(wemo_reader::curl);
    curl_global_cleanup();
    curl_slist_free_all(wemo_reader::headers_chunk);
  }
  wemo_reader::ip_addr = "";
}
