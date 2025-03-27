pyinstaller --onefile --noconsole ^
  --add-data "myolight_pyqt_fft.py;." ^
  --hidden-import=numba ^
  --hidden-import=numba.core.typing ^
  --hidden-import=numba.core ^
  --hidden-import=llvmlite.binding ^
  myolight.py

pause
