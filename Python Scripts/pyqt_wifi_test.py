import socket, struct, time
from collections import deque
import pyqtgraph as pg
from pyqtgraph.Qt import QtCore

HOST = '192.168.4.1'
PORT = 666
BUFFER_SIZE = 16
HISTORY_SIZE = 500
VOLTAGE_RANGE = 5  # ±2.5V = 5V range
ADC_LEVELS = 32768   # 16-bit

class SensorPlotter:
    def __init__(self):
        self.data_buffer = deque(maxlen=HISTORY_SIZE)
        self.time_buffer = deque(maxlen=HISTORY_SIZE)
        self.start_time = time.time()
        
        self.app = pg.mkQApp()
        self.win = pg.GraphicsLayoutWidget()
        self.plot = self.win.addPlot()
        self.plot.setLabel('left', 'Voltage', 'V')
        self.plot.setLabel('bottom', 'Time', 's')
        self.curve = self.plot.plot(pen='y')
        
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((HOST, PORT))
        
        self.read_timer = QtCore.QTimer()
        self.read_timer.timeout.connect(self.read_data)
        self.read_timer.start(0)
        
        self.win.show()
    
    def to_voltage(self, adc_value):
        return (adc_value / ADC_LEVELS) * VOLTAGE_RANGE - (VOLTAGE_RANGE/2)
    
    def read_data(self):
        if self.socket.recv(BUFFER_SIZE, socket.MSG_PEEK):
            data = self.socket.recv(BUFFER_SIZE)
            if len(data) == BUFFER_SIZE:
                values = struct.unpack('<8h', data)[0]
                current_time = time.time() - self.start_time
                self.time_buffer.append(current_time)
                self.data_buffer.append(self.to_voltage(values + 32768/2))  # Shift from signed to unsigned
                self.curve.setData(self.time_buffer, self.data_buffer)
                
if __name__ == "__main__":
    plotter = SensorPlotter()
    plotter.app.exec()