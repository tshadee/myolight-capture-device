import tkinter as tk
import datetime
import os

os.environ['TCL_LIBRARY'] = r'C:\Users\pante\AppData\Local\Programs\Python\Python313\tcl\tcl8.6'
os.environ['TK_LIBRARY'] = r'C:\Users\pante\AppData\Local\Programs\Python\Python313\tcl\tk8.6'

now = datetime.datetime.now()

#initialise tkinter main window
root = tk.Tk() 
root.title("Scan for nearby bluetooth devices")

#define width and height of window
window_w = 800
window_h = 800

#finds the width and height of the screen
screen_w = root.winfo_screenwidth()
screen_h = root.winfo_screenheight()

#find center point
center_x = int(screen_w/2 - window_w/2)
center_y = int(screen_h/2 - window_h/2)

#define main window geometry
root.geometry(f'{window_w}x{window_h}+{center_x}+{center_y}')

root.mainloop()
