/*
Updated version of the hive.

Power is mapped to the number of hexagons on the screen, 
energy to the progress they have made towards you (in the z-dimension).

Tested very poorly with users. 

Because of the prespective given to the user, it is hard to tell which hexagons
are more numerous and the reset towards the back is still jarring...

Takes awhile to figure out what is going on (is not immediate).

Left in for legacy.
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/

#include "hive_animation3.h"
#include "readers/sine_reader.cpp"
#include "readers/wemo_reader.cpp"
#include "readers/egauge_reader.cpp"
#include "readers/zwave_reader.cpp"
#include "stdio.h"
#include "math.h"
#include <queue>
#include <algorithm>
#include <ctime>
#include <cstdlib>
#include <vector>

using namespace std;

float DELAY = .1; //How long to wait between queries in seconds
float MAX_OPACITY = .9;
float MIN_OPACITY = .7;

int MAX_WATTAGE; //Set in main according to devices attached
int MAX_WATTHOURS;
float WATT_HOUR_RESET = 1000;

int SCREEN_WIDTH = 1100;
int SCREEN_HEIGHT = 600;
int HIVE_DIM = 10;
float SPACING = 1;
float INIT_SIZE = 1;
float INIT_DEPTH = -0;

int NUM_CELLS; //Set after hive is populated

double ASPECT = SCREEN_WIDTH / (double) SCREEN_HEIGHT;

const size_t NUM_COLORS = 15;
//http://tools.medialab.sciences-po.fr/iwanthue/index.php
Vector3 palette[NUM_COLORS] =
  {{255/255.0,217/255.0,123/255.0},
   {229/255.0,80/255.0,41/255.0},
   {166/255.0,0/255.0,39/255.0},
   {25/255.0,28/255.0,38/255.0},
   {102/255.0,0/255.0,51/255.0},
   {203/255.0,133/255.0,187/255.0},
   {91/255.0,79/255.0,127/255.0},
   {121/255.0,86/255.0,56/255.0},
   {90/255.0,170/255.0,131/255.0},
   {107/255.0,155/255.0,188/255.0},
   {200/255.0,79/255.0,202/255.0},
   {164/255.0,153/255.0,55/255.0},
   {220/255.0,136/255.0,45/255.0},
   {213/255.0,203/255.0,194/255.0},
   {193/255.0,224/255.0,157/255.0}};

queue<Cell *> off_cells;
vector<e_device *> devices;

Cell::Cell(Vector2 pos, Vector3 color, size_t type){
  Cell::pos = pos;
  Cell::color = color;
  Cell::type = type;
  Cell::on = false;
  Cell::opacity = MIN_OPACITY;
  Cell::increasing = true;
  Cell::zDepth = 0;
  Cell::d = (-0.5)+ (rand() % 200 / 400.0);
}

void Cell::draw(float watt_hours) {
  //Set opacity
  if (increasing) Cell::opacity += .02;
  if (!increasing) Cell::opacity -= .02;
  if (Cell::opacity >= MAX_OPACITY) increasing = false;
  if (Cell::opacity <= MIN_OPACITY) increasing = true;

  makeHexagon(INIT_SIZE, Cell::pos, Cell::zDepth,Cell::color, Cell::opacity);
}

struct compDepth
{
  bool operator()(Cell* c1, Cell* c2){
    return (c1->zDepth > c2->zDepth);
  }
};

void makeHexagon(float size, Vector2 pos, float zDepth,Vector3 color, float opacity){
  float angle = 0.0;
  float angleInc =  1.04719755; //    pi/3
  glColor4f(color.x,color.y,color.z,opacity);
  glBegin(GL_POLYGON);
  for (int i=0; i <= 6; i++){
    angle += angleInc;
    glVertex3d(size * cos(angle) + pos.x, size * sin(angle) + pos.y,zDepth);
  }
  glEnd();
  glColor4f(0.0,0.0,0.0,opacity);
  glBegin(GL_LINE_STRIP);
  for (int i=0; i <= 6; i++){
    angle += angleInc;
    glVertex3d(size * cos(angle) + pos.x, size * sin(angle) + pos.y,zDepth);
  }
  glEnd();

}

void populate_hive(float shift_val){
  float hd = 0.5 * HIVE_DIM;
  std::vector<Cell *> cells;
  float shift_row = shift_val / 2.0;
  float x = -hd * ASPECT;
  float y = -hd;
  while ( x <= hd * ASPECT){
    if (y > hd){
      y = -hd;
      x += shift_val;
      if (shift_row){
	y += shift_row;
	shift_row = 0;
      }
      else{
	shift_row = shift_val / 2.0;
      }
    }
    Vector2 pos = {x,y};
    int r = rand() % NUM_COLORS;
    cells.push_back(new Cell(pos,palette[r],-1));
    y += shift_val;
  }
  //The last cell isn't correct. 
  //Should change the logic, but this works for now.
  cells.pop_back();

  NUM_CELLS = cells.size();
  random_shuffle(cells.begin(), cells.end());
  for (int i = 0; i < NUM_CELLS; i++){
    off_cells.push(cells[i]);
  }
  
}

void init(){
  glOrtho(-0.5 * ASPECT * HIVE_DIM , 0.5 * ASPECT * HIVE_DIM,
	  -0.5 * HIVE_DIM, 0.5 * HIVE_DIM, 
	  -1.0, 1.0);
}

void renderFunction(){ 
  //glClearColor(0.95,0.95,0.95,0.0);
  priority_queue<Cell*, vector<Cell*>, compDepth> pq;
  glClearColor(0.9,0.9,0.9,0.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  int meter_count = 0;
  for (int d = 0; d < devices.size(); d++){
    e_device *cur_device = devices[d];
    vector<reading> reads = cur_device->reader->getNewReadings();
    for (int m = 0; m < cur_device->meters->size(); m++){
      if (find(cur_device->ignore->begin(),cur_device->ignore->end(), reads[m].deviceID) 
	  == cur_device->ignore->end()){
	meter_count += 1;
	float cur_wattage = reads[m].data;
	float cell_ratio = (NUM_CELLS * 1.0/MAX_WATTAGE);
	
	//Managing number of cells
	int cells_needed = (int) (cur_wattage * cell_ratio) 
	  - cur_device->meters->at(m)->meter_cells.size(); 
	
	if (cells_needed < 0){
	  //Remove some cells
	  for (int c = 0; c < -1.0 * cells_needed; c++){
	    if (! cur_device->meters->at(m)->meter_cells.size()){
	      break;
	    }
	    Cell * cur_cell = cur_device->meters->at(m)->meter_cells.back();
	    cur_device->meters->at(m)->meter_cells.pop_back();
	    cur_cell->type = -1;
	    off_cells.push(cur_cell);
	  }
	}
	else if (cells_needed > 0){
	  //Add some cells
	  for (int c = 0; c < cells_needed; c++){
	    if (! off_cells.size()){
	      break;
	    }
	    Cell * cur_cell = off_cells.front();
	    off_cells.pop();
	    cur_cell->type = d;
	    cur_cell->opacity = MIN_OPACITY + (rand() % 10 / 100.0);
	    cur_cell->increasing = true;
	    cur_cell->color = palette[meter_count -1];
	    cur_device->meters->at(m)->meter_cells.push_back(cur_cell);
	  }
	}
      
      
	//Now I need to manage watthours... which we will do informally.
	cur_device->meters->at(m)->accum_watts += DELAY * cur_wattage;
	float watt_hours = cur_device->meters->at(m)->accum_watts;
	float percent = (watt_hours / WATT_HOUR_RESET) - (long)(watt_hours / WATT_HOUR_RESET);
	

	//Getting position of each cell in display
	for (int c = 0; c < cur_device->meters->at(m)->meter_cells.size(); c++){
	  Cell * cur_cell = cur_device->meters->at(m)->meter_cells[c];
	  
	  cur_cell->zDepth = cur_cell->d +(-0.7 * cur_cell->d) * percent;
	  pq.push(cur_cell);
	}
      }
    } 
  }

  while(!pq.empty()){
    Cell * cur_cell = pq.top();
    pq.pop();
    cur_cell->draw(0);
  }
  
  glutSwapBuffers();
  usleep(1000000 * DELAY);
}


void close_window(){
  /* Freeing stuff - Could also use this as a chance to log outputs. */
  for (int d = 0; d < devices.size(); d++){
    e_device *cur_device = devices[d];
    for (int m = 0; m < cur_device->meters->size(); m++){
      meter *e_meter = cur_device->meters->at(m);
      for (int c = 0; c < e_meter->meter_cells.size(); c++){
	free(e_meter->meter_cells.back());
	e_meter->meter_cells.pop_back();
      }
      free(e_meter);
    }
    free(cur_device->ignore);
    free(cur_device->reader);
  } 
  for (int c = 0; c < off_cells.size(); c++){
    free(off_cells.front());
    off_cells.pop();
  }
  exit(0);
}

void on_click(int button, int state, int x, int y){
  glClearColor(1.0,1.0,1.0,0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glutSwapBuffers();
  for (int d = 0; d < devices.size(); d++){
    e_device *cur_device = devices[d];
    for (int m = 0; m < cur_device->meters->size(); m++){
      meter *e_meter = cur_device->meters->at(m);
      e_meter->accum_watts = 0;
      for (int c = 0; c < e_meter->meter_cells.size(); c++){
	Cell * cur_cell = e_meter->meter_cells.back();
	e_meter->meter_cells.pop_back();
	cur_cell->type = -1;
	off_cells.push(cur_cell);
      }
    }
  }
}

void on_idle(){
  glutPostRedisplay();
}

void addDevice(ifstream *myfile, string line, string type){
  vector<string> *ignores = new vector<string>;
  string ip;
  string watts;

  if (type != "egauge" && type != "wemo" && type != "sine" && type != "zwave"){
    return ;
  }

  while(getline(*myfile,line)){
    if (line.find("</"+ type)!= string::npos){
      break;
    }
    if (line.find("<ip") != string::npos){
      ip = line.substr(line.find(">") + 1,
		       line.find("</") -line.find(">") - 1);
      printf("Using ip : %s\n", ip.c_str());
    }
    if (line.find("<watts") != string::npos){
      watts = line.substr(line.find(">") + 1,
		       line.find("</") -line.find(">") - 1);
      printf("Found watts: %d!\n",stoi(watts));
    }
    if (line.find("<ignore") != string::npos){
      printf("Ignoring: %s\n", line.substr(line.find(">") + 1,
					   line.find("</") -line.find(">") - 1).c_str());
      ignores->push_back(line.substr(line.find(">") + 1,
			line.find("</") -line.find(">") - 1));
      
    }
  }
  if (ip == "" && (type == "egauge" || type == "wemo")){
    perror(("No IP provided for " 
	    + type + ", check configuration file.").c_str());
    return;
  }
  e_device *found = new e_device();
  found->ignore = ignores;

  if (type == "egauge"){
    found->reader = new egauge_reader(ip);
  }
  else if (type == "wemo"){
    found->reader = new wemo_reader(ip);
  }
  else if (type == "sine"){
    if (watts.empty()){
      found->reader = new sine_reader();
    }
    else{
      found->reader = new sine_reader(stoi(watts));
    }
  }
  else if (type == "zwave"){
    found->reader = new zwave_reader();
  }
  
  vector<meter *> *d_meters = new vector <meter *>;
  found->meters = d_meters;
  devices.push_back(found);
}

int main(int argc, char**argv){
  srand(unsigned(time(0)));

  string line;
  ifstream myfile ("hive.conf");
  if (myfile.is_open()){
    while (getline(myfile,line)){
      if (line == "<readers>"){
	while(getline(myfile,line)){
	  if (line == "</readers>"){
	    break;
	  }
	  if (line.find("#") != string::npos){}
	  else if (line.find("egauge") != string::npos){
	    addDevice(&myfile, line, "egauge");
	  }
	  else if (line.find("wemo") != string::npos){
	    addDevice(&myfile, line, "wemo");
	  }
	  else if (line.find("sine") != string::npos){
	    addDevice(&myfile, line, "sine");
	  }
	  else if (line.find("zwave") != string::npos){
	    addDevice(&myfile, line, "zwave");
	  }
	}
      }
    }
    myfile.close();
  }
  
  int meter_count = 0;
  for (int ed = 0; ed < devices.size(); ed++){
    e_device *current = devices[ed];
    int m = current->reader->getNumDevices();
    vector<reading> reads = current->reader->getNewReadings();
    for (int i = 0; i < m; i++){
      printf("%s\n",reads[i].deviceID.c_str());
      if (find(current->ignore->begin(),
	       current->ignore->end(), reads[i].deviceID) 
	  == current->ignore->end()){
	if (meter_count == NUM_COLORS){
	  perror("Too many meters detected. Please define which to display in configuration file, using ignores.");
	  exit(1);
	}
	meter * e_meter = new meter();
	vector<Cell *> cells;
	e_meter->meter_cells = cells;
	e_meter->color = palette[meter_count];
	e_meter->accum_watts = 0.0;
	current->meters->push_back(e_meter);
	meter_count++;
      }
    }
  }
  printf("%i\n",meter_count);

  MAX_WATTAGE = meter_count * 80;
  MAX_WATTHOURS = MAX_WATTAGE * 400;
  
  populate_hive(SPACING);

  glutInit(&argc,argv);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
  glutInitWindowSize(SCREEN_WIDTH,SCREEN_HEIGHT);
  glutInitWindowPosition(100,100);
  int x = glutCreateWindow("Hive_Animation");
  glutSetWindow(x);
  glutCloseFunc(close_window);
  glutMouseFunc(on_click);
  glutDisplayFunc(renderFunction);
  glutIdleFunc(on_idle);
  glEnable( GL_BLEND );
  glEnable( GL_LINE_SMOOTH );
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  init();
  gluPerspective(150.0,1,0,-100.0);
  glutMainLoop();

  return 0;
}
