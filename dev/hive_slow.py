'''
Towards the end of the year we hoped to produce a python version of
the animation for ease of iteration on the animation. 

Unfortunately this is extremely slow to render, it is extremely bugged...

C++ was the right choice for the raspberry pi.

However this is a good place to look to see how everything works!
(c) Devin Gardella 2015 (dpg3@williams.edu)
'''

from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
import sys 
import math
import queue
import time
import random
import itertools
import egauge
import wemo
import test_readers

DISPLAY = (1000, 600)
ASPECT = DISPLAY[0]/DISPLAY[1]
INIT_POS = (100,100)

HIVE_ROWS = 7
HIVE_COLS = 9
INIT_SIZE = .01

MIN_OPACITY = .6
MAX_OPACITY = .9

POWER_PER_CELL = 10 #Watts

OPACITY_VARIENCE = .01

DELAY = 1

off_cells = queue.Queue()
devices = [] # A list of devices that we will display

#http://tools.medialab.sciences-po.fr/iwanthue/index.php
palette = [[255/255.0,217/255.0,123/255.0],
           [229/255.0,80/255.0,41/255.0],
           [166/255.0,0/255.0,39/255.0],
           [102/255.0,0/255.0,51/255.0],
           [25/255.0,28/255.0,38/255.0],
           [203/255.0,133/255.0,187/255.0],
           [91/255.0,79/255.0,127/255.0],
           [121/255.0,86/255.0,56/255.0],
           [90/255.0,170/255.0,131/255.0],
           [107/255.0,155/255.0,188/255.0],
           [200/255.0,79/255.0,202/255.0],
           [164/255.0,153/255.0,55/255.0],
           [220/255.0,136/255.0,45/255.0],
           [213/255.0,203/255.0,194/255.0],
           [193/255.0,224/255.0,157/255.0]];


def size_function(watthours):
  #We want a maximum size of around 1
  return min(1, ((1-INIT_SIZE) * (math.sqrt(watthours)/1000.0)  + INIT_SIZE))

class Cell:
  def __init__(self, pos, color, deviceNum):
    self.pos = pos
    self.color = color
    self.opacity = MIN_OPACITY;
    self.zDepth = 0
    self.increasing = 1

  def draw(self,watthours):
    self.opacity += (self.increasing) * OPACITY_VARIENCE
    if (self.opacity >= MAX_OPACITY):
      self.increasing = -1
    if (self.opacity <= MIN_OPACITY):
      self.increasing = 1
    
    make_hexagon(size_function(watthours), self.pos, self.zDepth, self.color, self.opacity)


def make_hexagon(size, pos, zDepth, color, opacity):
  angle = 0.0
  angleInc =  1.04719755 #pi / 3
  glColor4f(color[0],color[1],color[2],opacity)

  #Drawing hexagon
  glBegin(GL_POLYGON)
  for i in range(6):
    angle += angleInc
    glVertex3d(size * math.cos(angle) + pos[0], size * math.sin(angle) + pos[1],zDepth)
  glEnd()

  #Drawing outline
  glColor4f(0.0,0.0,0.0,opacity)
  glBegin(GL_LINE_STRIP)  
  for i in range(7):
    angle += angleInc
    glVertex3d(size * math.cos(angle) + pos[0], size * math.sin(angle) + pos[1],zDepth)
  glEnd()

def populate_hive(rows, cols):  
  #Why I love python
  hive_coords = [( (.5 + c + (r % 2) /2) * ASPECT/cols, (.5 + r) * 1/rows) for r in range(rows) for c in range(cols - r % 2)]
  random.shuffle(hive_coords)
  for pos in hive_coords:
    off_cells.put(Cell(pos, (1.0,1.0,1.0), -1))

def init():
  #glClearColor(1.0,1.0,1.0,0.0)
  #glutSwapBuffers()
  glOrtho(0, ASPECT, 1, 0, 0, 1.0)
  populate_hive(HIVE_ROWS, HIVE_COLS)

def render():
  glClearColor(1.0,1.0,1.0,0.0)
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)
  
  for i in range(len(devices)):
    #Get last power reading
    last_readings = devices[i][0].power()

    #For each reading, update
    for name,reading in last_readings.items():
      devices[i][1][name][2] += reading
      cells_needed = int(reading / POWER_PER_CELL) 
      cells_current = len(devices[i][1][name][1])
      if (cells_needed > cells_current):
        #Need to add cells
        while not off_cells.empty() and cells_needed:
          new_cell = off_cells.get()
          new_cell.color = devices[i][1][name][0]
          devices[i][1][name][1].append(new_cell)
          cells_needed -= 1
      if (cells_needed < cells_current):
        #Need to remove cells      
        off_cells.put(devices[i][1][name][1].pop())
        cells_current -= 1

      for cell in devices[i][1][name][1]:
        cell.draw(devices[i][1][name][2])
  
  glutSwapBuffers()
  time.sleep(DELAY)
  

def on_idle():
  #I honestly don't know what this does but without it, things die...
  glutPostRedisplay()
  
def on_click(button, state, x, y):
  if button == GLUT_RIGHT_BUTTON and state:
    for i in range(len(devices)):
      for name in devices[i][1].keys():
        devices[i][1][name][2] = 0
        #Release all cells
        for j in range(len(devices[i][1][name][1])):
          off_cells.put(devices[i][1][name][1].pop())

def close_window():
  glutLeaveMainLoop()
    

#Yuck... Ignore this unless it insults you... Then use some nice xml library!
def devices_from_file(conf_file):
  with open(conf_file,"r") as fp:
    readers = False
    line = fp.readline()
    while line:
      if not readers:
        if "<readers>" in line:
          readers = True
      else:
        if "sine_reader" in line:
          var = 5
          mid = 15
          while line and "/sine_reader" not in line:
            if "var" in line:
              var = float(line[line.index(">") + 1:line.index("</")])
            if "mid" in line:
              mid = float(line[line.index(">") + 1:line.index("</")])
            line = fp.readline()
          add_device(test_readers.SineReader(mid,var))
        if "wemo" in line:
          ignores = []
          ip = ""
          while line and "</wemo" not in line:
            if "ip" in line:
              ip = line[line.index(">") + 1:line.index("</")].strip()
            if "ignore" in line:
              ignores.append(line[line.index(">") + 1:line.index("</")].strip())
            line = fp.readline()
          if ip:
            add_device(wemo.WemoReader(ip,ignores))
          else:
            print("Error in reading the ip_address for the wemo device. Include in one line: <ip>123.456.789</ip>")
        if "egauge" in line:
          ignores = []
          ip = ""
          while line and "</egauge" not in line:
            if "ip" in line:
              ip = line[line.index(">") + 1:line.index("</")].strip()
            if "ignore" in line:
              ignores.append(line[line.index(">") + 1:line.index("</")].strip())
            line = fp.readline()
          if ip:
            add_device(egauge.EgaugeReader(ip,ignores))
          else:
            print("Error in reading the website for the egauge device. Include in one line: <ip> http://egauge20975.egaug.es/ </ip>")
      line = fp.readline()
        

def add_device(dev):
  init_reading = dev.power()
  devices.append( [dev, {name: [palette[len(devices)],[],0] for name,reading in init_reading.items()}])

if __name__ == '__main__':
  glutInit()
  glutInitWindowSize(DISPLAY[0],DISPLAY[1])
  glutInitWindowPosition(INIT_POS[0],INIT_POS[1])
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutCreateWindow("Hive Animation")
  glutCloseFunc(close_window)
  glutMouseFunc(on_click)
  glutDisplayFunc(render)
  glutIdleFunc(on_idle)
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
  glEnable( GL_BLEND )
  glEnable( GL_LINE_SMOOTH )
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
  init()

  devices_from_file("hive.conf")

  glutMainLoop()
