from PyQt6.QtWidgets import QWidget, QApplication, QGridLayout
from PyQt6.QtCore import QTimer
import pyqtgraph as pg
import numpy as np

class LiveFFTWindow(QWidget):
    def __init__(self, fft_buffer_getter, sample_rate_getter, window_seconds=1):
        super().__init__()
        self.setWindowTitle("Live FFT Viewer")
        self.resize(1400, 800)

        self.fft_buffer_getter = fft_buffer_getter
        self.sample_rate_getter = sample_rate_getter
        self.sample_rate = self.sample_rate_getter()
        self.window_seconds = window_seconds

        self.sample_count = self.sample_rate * self.window_seconds  # Make sure sample_rate is set!
        self.time_step = 1 / self.sample_rate
        self.freq_axis = np.fft.rfftfreq(self.sample_count, d=self.time_step)

        self.layout = QGridLayout()
        self.setLayout(self.layout)

        self.buffer_length = 0
        self.plots = []
        self.curves = []
        self.buffers = [[] for _ in range(28)]

        for i in range(28):
            plot = pg.PlotWidget()
            plot.setYRange(0, 26600)
            plot.setXRange(20, 250)
            plot.setTitle(f"Ch {i+1}")
            curve = plot.plot(pen='g')
            self.layout.addWidget(plot, (27-i) // 7, i % 7)
            self.plots.append(plot)
            self.curves.append(curve)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_fft)
        self.timer.start(250)  # ~2Hz

    def update_fft(self):
        if self.sample_rate == 0:
            return

        fft_buffer = self.fft_buffer_getter()

        for i in range(28):
            if len(fft_buffer[i]) < self.sample_count:
                continue

            data = fft_buffer[i][-self.sample_count:]
            
            if not data:
                continue

            data = np.array(data)

            fft_result = np.fft.rfft(data) / float(self.sample_count)
            fft_mag = 2 * np.abs(fft_result)

            fft_mag[0] = 0
            fft_mag[self.freq_axis < 20] = 0

            self.curves[i].setData(self.freq_axis, fft_mag)
