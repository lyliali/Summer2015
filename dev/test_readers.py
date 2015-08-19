import math

class SineReader:
    def __init__(self, midpoint, varience):
        self.v = varience
        self.m = midpoint
        self.t = 0

    def power(self):
        self.t += .1
        return {"sine" : math.sin(self.t) * self.v + self.m}

