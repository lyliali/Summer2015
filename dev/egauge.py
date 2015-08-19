#Beatifully simply interface for communication with egauge devices.
#Uses the old formatting style of /cgi-bin/egauge?noteam, could become depreciated in the future.

import requests
import time
import xml.etree.ElementTree as ET

class EgaugeReader:
    def __init__(self, website,ignores):
        self.addr = website + "cgi-bin/egauge?noteam"
        self.ig = ignores

    def power(self):
        r = requests.get(self.addr)
        x = {meter.get('title') : max(0,float(meter.find('power').text)) for meter in ET.fromstring(r.text).findall('meter')}
        for key in self.ig:
            del x[key]
        return x

