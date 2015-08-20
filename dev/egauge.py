'''
Beautifully simply interface for communication with egauge devices.
Uses the old formatting style of /cgi-bin/egauge?noteam, which could become depreciated in the future.
(c) Devin Gardella 2015 (dpg3@williams.edu)
'''
import requests
import time
import xml.etree.ElementTree as ET

class EgaugeReader:
    def __init__(self, website,ignores):
        self.addr = website + "cgi-bin/egauge?noteam"
        self.ig = ignores

    def power(self):
        r = requests.get(self.addr)
        return {meter.get('title') : max(0,float(meter.find('power').text)) 
        		for meter in ET.fromstring(r.text).findall('meter')  if meter.get('title') not in self.ig}