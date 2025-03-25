from PyQt6.QtWidgets import QWidget, QApplication, QGridLayout
from PyQt6.QtCore import QTimer
import pyqtgraph as pg
import numpy as np
from scipy.signal import lfilter, iirnotch,filtfilt

class LiveFFTWindow(QWidget):
    def __init__(self, fft_buffer_getter, sample_rate_getter, window_seconds=1):
        super().__init__()
        self.setWindowTitle("Live FFT Viewer")
        self.resize(1400, 800)

        self.fft_buffer_getter = fft_buffer_getter
        self.sample_rate_getter = sample_rate_getter
        self.sample_rate = self.sample_rate_getter()
        self.window_seconds = window_seconds

        self.sample_count = int(self.sample_rate * self.window_seconds)  # Make sure sample_rate is set!
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
            plot.setYRange(0, 500)
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

            data = list(fft_buffer[i])[-self.sample_count:]
            
            if not data:
                continue

            data = np.array(data)

            data = comb_notch_filter(data, self.sample_rate)

            fft_result = np.fft.rfft(data) / float(self.sample_count)
            fft_mag = 2 * np.abs(fft_result)
            fft_mag[0] = 0
            fft_mag[self.freq_axis < 20] = 0
            fft_mag = fft_mag[::2]

            self.curves[i].setData(self.freq_axis[::2], fft_mag)

def comb_notch_filter(data, fs, f0=50, num_harmonics=5):
    N = int(fs / f0)  # spacing in samples
    b = np.ones(1)  # start with no filter
    
    for k in range(1, num_harmonics + 1):
        delay = k * N
        # Construct simple notch: current sample - delayed sample
        h = np.zeros(delay + 1)
        h[0] = 1
        h[-1] = -1
        b = np.convolve(b, h)

    # Normalize to avoid gain explosion
    b /= np.sum(np.abs(b))

    # Apply filter
    filtered_data = lfilter(b, [1], data)
    return filtered_data


def apply_notch_filters(data, fs, freqs=[50, 100, 150, 200, 250], quality=30):
        
        for f0 in freqs:
            b, a = iirnotch(f0, quality, fs)
            data = filtfilt(b, a, data)
        return data
        