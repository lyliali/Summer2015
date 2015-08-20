'''
A simple library for communication with the wemo insight switch.
(c) Devin Gardella 2015 (dpg3@williams.edu)
'''
import requests
import time
import sys

class WemoReader:
    def __init__(self, ip_addr, ignores):
        self.ip = ip_addr
        POSS_PORTS = ["49152","49153","49154"]
        #Try each port that I've seen the Wemo decide to use.
        for p in POSS_PORTS:
            try:
                self.port = p
                self.get_info()
                break
            except Exception as err:
                #Seems silly, but it works
                if not "ConnectionRefusedError" in str(err):
                    raise

        if not self.port:
            print("Could not find port to connect to on ip: " + self.ip)
            sys.exit()
        self.ig = ignores

    def _action(self,command,service,control,param_dict):
        header = {'Content-Type': 'text/xml; charset="utf-8"',
                   'SOAPACTION': '"' + service + '#' + command + '"'}
        body="<?xml version='1.0' encoding='utf-8'?>" + \
             "<s:Envelope xmlns:s='http://schemas.xmlsoap.org/soap/envelope/" + \
             "'s:encodingStyle='http://schemas.xmlsoap.org/soap/encoding/'>" + \
             "<s:Body>" + \
             "<u:" + command + " xmlns:u='" + service + "'>"
         
        for key,val in param_dict.items():
            body += "<" + key + ">"+ val + "</" + key + ">"
        
        body += "</u:" + command + ">" + \
                "</s:Body>" + \
                "</s:Envelope>"

        controlUrl= "http://" + self.ip + ":" + self.port + control
        response = requests.post(controlUrl, body, headers=header)
        return response

    #Gets the current state of the Wemo device. 
    #(Be aware there is a delay in the data)
    def get_info(self):
        state_definition = {"0": "off", "1" : "on", 
                            "8" : "wemo on, connected device off"}
        # state: 0 if off, 1 if on, 8 if on but load is off
        params = ["device_state", "last_change","on_for",
                  "on_today","on_total","???",
                  "???", "current_w","today_w",
                  "total_w", "power_threshold"]
        
        wemo_res = self._action("GetInsightParams","urn:Belkin:service:insight:1","/upnp/control/insight1" ,{}).text
        data_line = wemo_res.split("\n")[2]
        data = data_line[data_line.index(">") + 1: data_line.index("</")].split("|")
        res_dict = {key:val for key,val in zip(params,data) if key != "???"}
        res_dict["device_state"] = state_definition[res_dict["device_state"]]
        return res_dict


    def turn_off(self):
        self._action("SetBinaryState","urn:Belkin:service:basicevent:1","/upnp/control/basicevent1", {"BinaryState":"0"})

    def turn_on(self):
        self._action("SetBinaryState","urn:Belkin:service:basicevent:1","/upnp/control/basicevent1", {"BinaryState":"1"})
        
    def power(self):
        if "wemo" in self.ig:
            return {}
        return {"wemo" : float(self.get_info()['current_w'])}

    def stream_data(self,delay):
        t = 0
        while (True):
            data = self.get_info()
            print(data)
            time.sleep(delay)
            t += 1

