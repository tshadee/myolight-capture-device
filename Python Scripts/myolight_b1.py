import customtkinter as ctk
import socket
import struct
import threading
import csv
import numpy as np
import ast
import os
import sys
from queue import Queue
from PIL import Image

os.environ['TCL_LIBRARY'] = r'C:\Users\pante\AppData\Local\Programs\Python\Python313\tcl\tcl8.6'
os.environ['TK_LIBRARY'] = r'C:\Users\pante\AppData\Local\Programs\Python\Python313\tcl\tk8.6'

#Set theme and colour options
ctk.set_appearance_mode("light")
ctk.set_default_color_theme("green")  # Themes: "blue" (standard), "green", "dark-blue"

# --- GUI App Class ---
class DataLoggerApp(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("MYOLIGHT")
        self.geometry("600x600")

        # #backinggroundimage
        # background_image = Image.open("background_image.png")
        # ctk_background = ctk.CTkImage(dark_image = background_image,light_image=background_image,size=(600,600))
        
        # self.background_label = ctk.CTkLabel(self,image=ctk_background,text="")

        # self.background_label.place(x=0,y=0,relwidth=1,relheight=1)

        #buttons and such

        #logo
        self.label = ctk.CTkLabel(
            self, 
            text="MYOLIGHT v0.1", 
            font=("Monaco", 16))
        self.label.pack(pady=10)

        #connection establish
        self.connect_button = ctk.CTkButton(
            self,
            text="Search and Connect", 
            font=("Monaco", 12), 
            command=self.start_connection,
            state="normal")
        self.connect_button.pack(pady=10)

        #disconnect
        self.disconnect_button = ctk.CTkButton(
            self,
            text="Disconnect", 
            font=("Monaco", 12), 
            command=self.stop_connection,
            state="disabled")
        self.disconnect_button.pack(pady=10)

        #data collection start
        self.start_button = ctk.CTkButton(
            self, 
            text="Start Data Collection", 
            font=("Monaco", 12),
            command=self.start_data_collection,
            state="disabled")
        self.start_button.pack(pady=10)

        #data collection stop
        self.stop_button = ctk.CTkButton(
            self, 
            text="Stop Data Collection", 
            font=("Monaco", 12),
            command=self.stop_data_collection, 
            state="disabled")
        self.stop_button.pack(pady=10)

        #prune data
        self.prune_button = ctk.CTkButton(
            self, 
            text="Prune Data", 
            font=("Monaco", 12),
            command=self.start_pruning,
            state="normal")
        self.prune_button.pack(pady=10)

        #status updater
        self.status_label = ctk.CTkLabel(
            self, 
            text="Status: Idle", 
            font=("Monaco", 12))
        self.status_label.pack(pady=10)

        #console window
        self.console_textbox = ctk.CTkTextbox(self,width=500,height=100,font=("Monaco", 12))
        self.console_textbox.pack(pady=10,expand=True)
        self.console_textbox.configure(state="disabled") #yall dont interact with this textbox

        #sysout to this ctk window instead of console
        sys.stdout = self

        #threading
        self.stop_event = threading.Event()
        self.collection_thread = None
        self.status_queue = Queue()
        self.after(100, self.process_status_queue)

        self.socket_connection = None

    def write(self,message):
        #print statements to this textbox
        self.console_textbox.configure(state="normal")  # Enable editing
        self.console_textbox.insert("end", message)  # Insert the message
        self.console_textbox.configure(state="disabled")  # Disable editing
        self.console_textbox.see("end")  # Auto-scroll to the bottom

    def flush(self):
        #sysout redirection or smth idk
        pass

    def start_connection(self):
        self.status_queue.put("Status: Connecting to MCU...")
        print("Status: Connecting to MCU...")
        try:
            self.socket_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket_connection.connect(('192.168.4.1', 666))
            self.status_queue.put("Status: Connected to MCU at IP: 192.168.4.1 , Port: 666")
            print("Status: Connected to MCU at IP: 192.168.4.1 , Port: 666")
            self.connect_button.configure(state="disabled")
            self.disconnect_button.configure(state="normal")
            self.start_button.configure(state="normal")
            self.stop_button.configure(state="normal")
            self.prune_button.configure(state="disabled")
        except Exception as e:
            self.status_queue.put(f"Error: {e}")

    def stop_connection(self):
         if self.socket_connection:
            try:
                self.socket_connection.close()
                self.status_queue.put("Status: Disconnected from MCU.")
                print("Status: Disconnected from MCU.")
                self.socket_connection = None
                self.connect_button.configure(state="normal")
                self.disconnect_button.configure(state="disabled")
                self.start_button.configure(state="disabled")
                self.stop_button.configure(state="disabled")
                self.prune_button.configure(state="normal")
            except Exception as e:
                self.status_queue.put(f"Error: {e}")
                print(f"Error: {e}")

    def process_status_queue(self):
        try:
            while True:
                new_text = self.status_queue.get_nowait()
                self.status_label.configure(text=new_text)
        except Exception:
            pass
        self.after(100, self.process_status_queue)

    def start_data_collection(self):
        self.status_queue.put("Status: Collecting Data...")
        self.start_button.configure(state="disabled")
        self.stop_button.configure(state="normal")
        self.stop_event.clear()

        self.collection_thread = threading.Thread(target=self.collect_data, daemon=True)
        self.collection_thread.start()

    def stop_data_collection(self):
        self.status_queue.put("Status: Stopping...")
        self.stop_event.set()
        self.after(100, self.check_collection_thread)
        self.connect_button.configure(state="disabled")
        self.disconnect_button.configure(state="normal")
        self.start_button.configure(state="normal")
        self.stop_button.configure(state="disabled")
        self.prune_button.configure(state="normal")
        

    def check_collection_thread(self):
        if self.collection_thread.is_alive():
            self.after(100, self.check_collection_thread)
        else:
            self.status_queue.put("Status: Stopped")
            self.start_button.configure(state="normal")
            self.stop_button.configure(state="disabled")

    def start_pruning(self):
        #Runs data pruning in the main thread after collection is stopped.
        self.status_queue.put("Status: Pruning Data...")
        prune_data()
        self.status_queue.put("Status: Pruning Complete")

    def collect_data(self):
        #Handles WiFi data collection and writes to CSV file
        # HOST = '192.168.4.1'  
        # PORT = 666 
        BUFFER_SIZE = 66
        CSV_FILE = "data_log.csv"

        if self.socket_connection:
            # s.settimeout(0.1)
            try:
                # # self.status_label.configure(text=f"Connecting to {HOST}:{PORT}...", font=("Monaco", 12))
                # self.status_queue.put(f"Connecting to {HOST}:{PORT}...")
                # print(f"Connecting to {HOST}:{PORT}...")
                # s.connect((HOST, PORT))
                # # self.status_label.configure(text="Connected. Collecting Data...", font=("Monaco", 12))
                # self.status_queue.put("Connected. Collecting Data...")
                # print("Connected. Collecting Data...")

                with open(CSV_FILE, mode='w', newline='') as file:
                    writer = csv.writer(file)
                    writer.writerow(['CH1', 'CH2', 'CH3', 'CH4', 'PacketNumber'])

                    while not self.stop_event.is_set():
                        data = b''
                        while len(data) < BUFFER_SIZE:
                            try:
                                packet = self.socket_connection.recv(BUFFER_SIZE - len(data))
                                if not packet:
                                    break
                                data += packet
                            except socket.timeout:
                                if self.stop_event.is_set():
                                    break
                                continue
                            if self.stop_event.is_set():
                                break
                        if self.stop_event.is_set():
                            break
                        if not data:
                            break

                        if len(data) == BUFFER_SIZE:
                            CH1 = struct.unpack('<8h', data[0:16])
                            CH2 = struct.unpack('<8h', data[16:32])
                            CH3 = struct.unpack('<8h', data[32:48])
                            CH4 = struct.unpack('<8h', data[48:64])
                            packet_number = struct.unpack('<H', data[64:66])[0]

                            writer.writerow([CH1, CH2, CH3, CH4, packet_number])

            except Exception as e:
                self.status_queue.put(f"Error: {e}")
            # finally:
            #     pass

#Data Pruning
def prune_data():
    CSV_FILE = 'data_log.csv'
    OUTPUT_FILE = 'data_log_pruned.csv'
    fields = []
    channels = [[] for _ in range(32)]
    packet_numbers = []
    diff_threshold = 500

    with open(CSV_FILE, mode='r', newline='') as file:
        csvreader = csv.reader(file)
        fields = next(csvreader)

        for row in csvreader:
            ch1_str, ch2_str, ch3_str, ch4_str, pn_str = row
            packet_numbers.append(int(pn_str))
            for ch_idx, ch_str in enumerate([ch1_str, ch2_str, ch3_str, ch4_str]):
                ch_data = ast.literal_eval(ch_str)
                for elem_idx, value in enumerate(ch_data):
                    channel_index = ch_idx * 8 + elem_idx
                    channels[channel_index].append(value)

    print("[DONE] CSV Load Good")
    channels = [np.array(ch) for ch in channels]
    packet_numbers = np.array(packet_numbers)
    output_array = []

    num_rows = len(channels[0])
    valid_mask = np.ones(num_rows, dtype=bool)

    for ch in channels:
        diffs = np.diff(ch)
        invalid_packet = np.where(np.abs(diffs) > diff_threshold)[0] + 1
        valid_mask[invalid_packet] = False

    invalid_data = 0

    for i in range(num_rows):
        if valid_mask[i]:
            ch1 = tuple(int(channels[j][i]) for j in range(0, 8))
            ch2 = tuple(int(channels[j][i]) for j in range(8, 16))
            ch3 = tuple(int(channels[j][i]) for j in range(16, 24))
            ch4 = tuple(int(channels[j][i]) for j in range(24, 32))
            if (ch1[6] == -21846) and (ch2[6] == -21846) and (ch3[6] == -21846) and (ch4[6] == -21846):
                row = [ch1, ch2, ch3, ch4, packet_numbers[i]]
                output_array.append(row)
            else:
                invalid_data += 1
        else:
            invalid_data += 1

    with open(OUTPUT_FILE, mode='w', newline='') as outputfile:
        csvwriter = csv.writer(outputfile)
        csvwriter.writerow(fields)
        csvwriter.writerows(output_array)

    percentage_corrupt = invalid_data*100/num_rows
    print(f"------ {invalid_data} invalid data points ({percentage_corrupt:.2f}%)")
    print(f"[DONE] Pruned data written to {OUTPUT_FILE}.")

# --- Run GUI ---
if __name__ == "__main__":
    app = DataLoggerApp()
    app.mainloop()
