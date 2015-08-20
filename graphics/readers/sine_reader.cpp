/* 
An example of a ereader, written for testing and debugging purposes.
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/

#include "ereader.h"
#include "sine_reader.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

sine_reader::sine_reader(){
  sine_reader::init();
}

sine_reader::~sine_reader(){

}

string sine_reader::getType(){
  return "sine";
}

int sine_reader::getNumDevices(){
  return 1;
}

void sine_reader::init(){
  sine_reader::t = 0.0;
  return;
}

vector<reading> sine_reader::getNewReadings(){
  reading cur;
  cur.deviceID = "sine-1";
  cur.data = sin(sine_reader::t) * 10 + 20;
  t += .1;
  vector<reading> readings;
  readings.push_back(cur);
  return readings;
}

void sine_reader::shutdown(){
 
}
