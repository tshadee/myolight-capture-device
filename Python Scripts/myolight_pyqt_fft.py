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
        self.window_seconds = window_seconds

        self.layout = QGridLayout()
        self.setLayout(self.layout)

        self.buffer_length = 0
        self.plots = []
        self.curves = []
        self.buffers = [[] for _ in range(28)]

        for i in range(28):
            plot = pg.PlotWidget()
            plot.setYRange(0, 32768)
            plot.setXRange(20, 250)
            plot.setTitle(f"Ch {i+1}")
            curve = plot.plot(pen='g')
            self.layout.addWidget(plot, (27-i) // 7, i % 7)
            self.plots.append(plot)
            self.curves.append(curve)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_fft)
        self.timer.start(500)  # ~2Hz

    def update_fft(self):
        sample_rate = self.sample_rate_getter()
        if sample_rate == 0:
            return

        fft_buffer = self.fft_buffer_getter()
        sample_count = sample_rate * self.window_seconds
        time_step = 1 / sample_rate

        for i in range(28):
            data = fft_buffer[i][-sample_count:] if len(fft_buffer[i]) >= sample_count else fft_buffer[i]
            
            if not data:
                continue
            data = np.array(data)
            if len(data) < sample_count:
                data = np.pad(data, (0, sample_count - len(data)))

            fft_result = np.fft.fft(data) / float(sample_count)
            freq_axis = np.fft.fftfreq(sample_count, d=time_step)
            fft_mag = 2 * np.abs(fft_result[:sample_count // 2])
            freq_axis = freq_axis[:sample_count // 2]
            fft_mag[0] = 0
            fft_mag[freq_axis < 20] = 0

            self.curves[i].setData(freq_axis, fft_mag)
            self.plots[i].setYRange(0, np.max(fft_mag) * 1.2 if np.max(fft_mag) > 0 else 1)
