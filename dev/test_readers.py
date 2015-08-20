'''
A basic reader which is used to test the animation.
(c) Devin Gardella 2015 (dpg3@williams.edu)
'''

import math

class SineReader:
    def __init__(self, midpoint, varience):
        self.v = varience
        self.m = midpoint
        self.t = 0

    def power(self):
        self.t += .1
        return {"sine" : math.sin(self.t) * self.v + self.m}

