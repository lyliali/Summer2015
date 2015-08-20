/*
(c) Devin Gardella 2015 (dpg3@williams.edu)
*/
#include "GL/freeglut.h"
#include "GL/gl.h"
#include "readers/ereader.h"
#include <vector>
#include <fstream>
#include <unistd.h>

struct Vector2{
  float x,y;
};

struct Vector3{
  float x,y,z;
};

void makeHexagon(float size, Vector2 pos, float zDepth,Vector3 color, float opacity = 1.0);
void renderFunction();
void close_window();
void addDevice(ifstream *myfile, string line, string type);


class Cell {
 public:
  Cell(Vector2 pos, Vector3 color, size_t type);
  Vector2 pos;
  Vector3 color;
  bool    on;
  float   opacity;
  size_t type;
  void    draw(float watt_hours);
  bool increasing;
  float zDepth;
};

struct meter {
  Vector3 color;
  vector<Cell*> meter_cells;
  double accum_watts;
};

struct e_device {
  ereader * reader;
  vector<meter *> *meters;
  vector<string> *ignore;
};


