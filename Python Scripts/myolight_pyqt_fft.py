from PyQt6.QtWidgets import QWidget, QApplication, QGridLayout
from PyQt6.QtCore import QTimer
import pyqtgraph as pg
import numpy as np
from scipy.signal import lfilter
from scipy.signal import firwin
from numba import njit

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
        self.low_pass = low_pass_filter(self.sample_rate)
        self.high_pass = high_pass_filter(self.sample_rate)

        self.layout = QGridLayout()
        self.setLayout(self.layout)

        self.buffer_length = 0
        self.plots = []
        self.curves = []
        self.buffers = [[] for _ in range(28)]

        for i in range(28):
            plot = pg.PlotWidget()
            plot.setYRange(0, 1000)
            plot.setXRange(20, 250)
            plot.setTitle(f"Ch {i+1}")
            curve = plot.plot(pen='g')
            self.layout.addWidget(plot, (27-i) // 7, i % 7)
            self.plots.append(plot)
            self.curves.append(curve)

        self.timer = QTimer()
        self.timer.timeout.connect(self.update_fft)
        self.timer.start(200)  # ~5Hz

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
            data = apply_highpass(data,self.high_pass)
            data = apply_lowpass(data,self.low_pass)
            data = comb_notch_filter_numba(data, self.sample_rate)

            fft_result = np.fft.rfft(data) / float(self.sample_count)
            fft_mag = 2 * np.abs(fft_result)
            fft_mag[0] = 0
            fft_mag[self.freq_axis < 20] = 0
            fft_mag = fft_mag[::3]

            self.curves[i].setData(self.freq_axis[::3], fft_mag)


def high_pass_filter(fs,cutoff=15,numtaps=51):
    return firwin(numtaps,cutoff,pass_zero=0,fs=fs)

def apply_highpass(data,b):
    return lfilter(b,[1.0],data)

def low_pass_filter(fs,cutoff=400,numtaps=51):
    return firwin(numtaps,cutoff,fs=fs)

def apply_lowpass(data,b):
    return lfilter(b,[1.0],data)

@njit
def comb_notch_filter_numba(data, fs, f0=50, num_harmonics=5):
    N = int(fs / f0)
    max_delay = num_harmonics * N
    b = np.zeros(max_delay + 1)
    b[0] = 1.0

    for k in range(1, num_harmonics + 1):
        b[k * N] -= 1.0

    b_sum = np.sum(np.abs(b))
    if b_sum != 0:
        b /= b_sum

    # Manual convolution (faster for short filters with Numba)
    output = np.zeros_like(data)
    for n in range(len(data)):
        for k in range(len(b)):
            if n - k >= 0:
                output[n] += b[k] * data[n - k]
    return output

        