import customtkinter as ctk
import os
from CTkColorPicker import *

os.environ['TCL_LIBRARY'] = r'C:\Users\pante\AppData\Local\Programs\Python\Python313\tcl\tcl8.6'
os.environ['TK_LIBRARY'] = r'C:\Users\pante\AppData\Local\Programs\Python\Python313\tcl\tk8.6'

#Set theme and colour options
ctk.set_appearance_mode("light")
ctk.set_default_color_theme("green")  # Themes: "blue" (standard), "green", "dark-blue"


root = ctk.CTk()
root.title('CTKTEST') 
root.geometry('900x600')

def glebasubmit():
    my_label.configure(text=f'Gleba Credentials : : Hello {my_entry.get()}')
    my_entry.delete(0, "end")
    pass

def glebacheck():
    my_label.configure(text="You checked the GLEBA!")
    pass

my_label = ctk.CTkLabel(root, text="Gleba Credentials", font=("Monaco",12))
my_label.pack(pady=5)


my_entry = ctk.CTkEntry(root, 
                        placeholder_text="Enter The GLEBA Name",
                        font=("Monaco",12),
                        height=30,
                        width=300,
                        corner_radius=0
                        )
my_entry.pack(pady=5)

#can use Bookerly, Bookerly Display, Helvetica, CMU Serif, Comic Sans, Monaco
my_button = ctk.CTkButton(root, 
                          text="GLEBA SUBMIT", 
                          command=glebasubmit,
                          height=70,
                          width=140,
                          font=("Monaco",20), 
                          text_color="white",
                          fg_color="#76A165",
                          hover_color="#3E5435",
                          corner_radius=0
                          )
my_button.pack(pady=30)

check_var = ctk.IntVar(value=0)

my_check = ctk.CTkCheckBox(root,
                           text="Would you like to GLEBA?",
                           font=("Monaco",12),
                           variable=check_var,
                           onvalue=1,
                           offvalue=0,
                           fg_color="#76A165",
                           hover_color="#3E5435",
                           corner_radius=0,
                           command=glebacheck,
                           hover=False
                           )

my_check.pack(pady=20)

root.mainloop()