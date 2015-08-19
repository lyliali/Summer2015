import subprocess
import threading

class EgaugeReader:
    def __init__(self, delay, ignores):
        self.delay = str(delay * 100)
        self.ig = ignores
        self.current_readings = {}

        self.read_thread = threading.Thread(target=self.read)
        self.read_thread.daemon = True  # thread dies when main thread (only non-daemon thread) exits.
        self.read_thread.start()
        

    def read(self):
        proc = subprocess.Popen(['./zwmeter','dev/ttyUSB0', self.delay],stdout=subprocess.PIPE)
        while True:
            line = proc.stdout.readline()
            if line != '':
                pass
            else:
                break

    def power(self):
        return current_readings


