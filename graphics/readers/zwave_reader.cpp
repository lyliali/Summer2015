#include "ereader.h"
#include "zwave_reader.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

static vector<reading> currentReadings;
static bool running;

zwave_reader::zwave_reader(){
  zwave_reader::child = 0;
  zwave_reader::init();
  thread update_thread(&zwave_reader::getNext,this);
  running = true;
  update_thread.detach();
  sleep(10); //Gotta wait till the reader is fully online.
}

zwave_reader::~zwave_reader(){
  zwave_reader::shutdown();
}

string zwave_reader::getType(){
  return "zwave";
}

int zwave_reader::getNumDevices(){
  return currentReadings.size();
}

void zwave_reader::init(){
  if (zwave_reader::child){
    printf("%d\n",zwave_reader::child);
    perror("Already initialized!\n");
    return;
  }

  pid_t pid;
  int p_stdin[2];
  int p_stdout[2];
  int READ = 0;
  int WRITE = 1;

  if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0){
    perror("Failed to open pipes\n");
    exit(1);
  }
  
  pid = fork();
  if (pid < 0){
    perror("Failed to fork?\n");
    exit(1);
  }
  else if (pid == 0){
    close(p_stdin[WRITE]);
    close(p_stdin[READ]);
    close(p_stdout[READ]);
    dup2(p_stdout[WRITE],WRITE);
    execlp( (string(getpwuid(getuid())->pw_dir) + string("/Desktop/graphics/readers/zwmeter")).c_str(), "./zwmeter", "/dev/ttyUSB0", "300", NULL);
    perror("Couldn't start reader\n");
    exit(1);
  }
  
  close(p_stdin[WRITE]);
  zwave_reader::fp = fdopen(p_stdout[READ],"r");
  zwave_reader::child = pid;
}

string zwave_reader::getNext(){
  char buff[1024];
  while (running && (fgets(buff, 1024, zwave_reader::fp) != NULL)){
    string read = string(buff);
    if (! read.compare(0,3,"* [")){
      string device_type = zwave_reader::getType().append(read.substr(6,1));
      int i = 0;
      for (i = 0; i < currentReadings.size(); i++){
	if (currentReadings[i].deviceID == device_type){
	  currentReadings[i].data =     
	    atof(read.substr(9,read.length()).c_str());
	  i = -1;
	  break;
	}
      }
      //If we haven't found the device in currentReadings
      if (i != -1){
	reading * given_reading = new reading();
	given_reading->deviceID = device_type;
	given_reading->data = atof(read.substr(9,read.length()).c_str());
	currentReadings.push_back(*given_reading);
      }
    }
  }
  return "";
}

vector<reading> zwave_reader::getNewReadings(){
  return currentReadings;
}

void zwave_reader::shutdown(){
  running = false;
  if (zwave_reader::child){
    kill(zwave_reader::child, SIGTERM);
    printf("%s\n","zwave successfully shutdown");
  }
}

