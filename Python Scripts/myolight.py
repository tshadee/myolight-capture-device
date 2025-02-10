import customtkinter as ctk
import socket, struct, threading, csv, os, sys, subprocess, ast
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import interp1d
from queue import Queue
from PIL import Image

#Set theme and colour options
ctk.set_appearance_mode("light")
ctk.set_default_color_theme("green")  # Themes: "blue" (standard), "green", "dark-blue"

# --- GUI App Class ---
class MYOLIGHTInterface(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("MYOLIGHT")
        self.geometry("250x500")
        self.resizable(False, False)

        #frame for buttons
        self.button_frame = ctk.CTkFrame(self,fg_color="transparent")
        self.button_frame.pack(pady=10,padx=10,anchor="n")

        #BUTTONS AND SUCH
        # ---- Logo ----
        self.label = ctk.CTkLabel(
            self.button_frame,  # Now inside button frame
            text="MYOLIGHT v0.1",
            font=("Monaco", 16)
        )
        self.label.pack(pady=5)

        self.connect_button = ctk.CTkButton(
            self.button_frame,
            text="Search and Connect",
            font=("Monaco", 12),
            command=self.start_connection,
            state="normal",
            corner_radius=0,
            width=160
        )
        self.connect_button.pack(pady=2)

        self.disconnect_button = ctk.CTkButton(
            self.button_frame,
            text="Disconnect",
            font=("Monaco", 12),
            command=self.stop_connection,
            state="disabled",
            corner_radius=0,
            width=160
        )
        self.disconnect_button.pack(pady=2)

        self.start_button = ctk.CTkButton(
            self.button_frame,
            text="Start Data Collection",
            font=("Monaco", 12),
            command=self.start_data_collection,
            state="disabled",
            corner_radius=0,
            width=160
        )
        self.start_button.pack(pady=2)

        self.stop_button = ctk.CTkButton(
            self.button_frame,
            text="Stop Data Collection",
            font=("Monaco", 12),
            command=self.stop_data_collection,
            state="disabled",
            corner_radius=0,
            width=160
        )
        self.stop_button.pack(pady=2)

        #prune data
        self.prune_button = ctk.CTkButton(
            self.button_frame, 
            text="Prune Data", 
            font=("Monaco", 12),
            command=self.start_pruning,
            state="normal",
            corner_radius=0,
            width=160)
        self.prune_button.pack(pady=2)

        #analyse data
        self.analyse_button = ctk.CTkButton(
            self.button_frame, 
            text="Analyse Data", 
            font=("Monaco", 12),
            command=self.start_analysis,
            state="normal",
            corner_radius=0,
            width=160)
        self.analyse_button.pack(pady=2)

        #status updater
        self.status_label = ctk.CTkLabel(
            self, 
            text="Status: Idle", 
            font=("Monaco", 12))
        self.status_label.pack(pady=10)

        #console window
        self.console_textbox = ctk.CTkTextbox(
            self,
            width=250,
            height=200,
            font=("Monaco", 12))
        self.console_textbox.pack(pady=10,expand=True,anchor="s")
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
        # self.status_queue.put("Status: Scanning for MCU...")
        # print("Status: Scanning for MCU...")

        # wifi_networks = self.scan_wifi_network()
        # print(wifi_networks)

        # self.status_queue.put("Status: Connecting to Wi-Fi...")
        # print("Status: Connecting to Wi-Fi...")

        # ssid = "ESP32C6T-softAP"
        # password = "qw12er34"
        # connection_result = self.connect_to_wifi(ssid,password)
        # print(connection_result)

        # if "Connected" in connection_result:
        self.status_queue.put("Status: Connecting to MCU...")
        print("Status: Connecting to MCU...")
        try:
            self.socket_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket_connection.settimeout(5) 
            self.socket_connection.connect(('192.168.4.1', 666))
            self.status_queue.put("Status: Connected to MCU at IP: 192.168.4.1 , Port: 666")
            print("Status: Connected to MCU at IP: 192.168.4.1 , Port: 666")
            self.connect_button.configure(state="disabled")
            self.disconnect_button.configure(state="normal")
            self.start_button.configure(state="normal")
            self.stop_button.configure(state="normal")
            self.prune_button.configure(state="disabled")
            self.analyse_button.configure(state="disabled")
        except Exception as e:
            self.status_queue.put(f"Error: {e}")
        # else:
        #     self.status_queue.put("Status: Failed to connect to Wi-Fi.")
        #     print("Status: Failed to connect to Wi-Fi.")

    def scan_wifi_network(self):
        try:
            result = subprocess.run(["netsh", "wlan", "show", "networks"], capture_output=True, text=True)
        except Exception as e:
            return f"Error: {e}"
        
    def connect_to_wifi(self,ssid,password):
        try:
            # Create a XML configuration file for the Wi-Fi profile
            config = f"""<?xml version=\"1.0\"?>
                        <WLANProfile xmlns="http://www.microsoft.com/networking/WLAN/profile/v1">
                        <name>{ssid}</name>
                        <SSIDConfig>
                        <SSID>
                        <name>{ssid}</name>
                        </SSID>
                        </SSIDConfig>
                        <connectionType>ESS</connectionType>
                        <connectionMode>auto</connectionMode>
                        <MSM>
                        <security>
                        <authEncryption>
                        <authentication>WPA2PSK</authentication>
                        <encryption>AES</encryption>
                        <useOneX>false</useOneX>
                        </authEncryption>
                        <sharedKey>
                        <keyType>passPhrase</keyType>
                        <protected>false</protected>
                        <keyMaterial>{password}</keyMaterial>
                        </sharedKey>
                        </security>
                        </MSM>
                        </WLANProfile>"""
            
            with open(f"{ssid}.xml", "w") as file:
                file.write(config)
            
            # Add the profile to the system
            subprocess.run(["netsh", "wlan", "add", "profile", f"filename={ssid}.xml"], check=True)
            
            # Connect to the Wi-Fi network
            subprocess.run(["netsh", "wlan", "connect", f"name={ssid}"], check=True)
            
            return f"Connected to {ssid}"
        except subprocess.CalledProcessError as e:
            return f"Error: {e}"

    def stop_connection(self):
        self.stop_data_collection()
        try:
            if self.socket_connection:
                self.socket_connection.close()
                self.socket_connection = None
                self.status_queue.put("Status: Disconnected from MCU.")
                print("Status: Disconnected from MCU.")
                self.socket_connection = None
                self.connect_button.configure(state="normal")
                self.disconnect_button.configure(state="disabled")
                self.start_button.configure(state="disabled")
                self.stop_button.configure(state="disabled")
                self.prune_button.configure(state="normal")
                self.analyse_button.configure(state="normal")
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
        # if self.collection_thread and self.collection_thread.is_alive():
        #     self.collection_thread.join()  # Wait for the thread to finish
        self.after(100, self.check_collection_thread)
        self.connect_button.configure(state="disabled")
        self.disconnect_button.configure(state="normal")
        self.start_button.configure(state="normal")
        self.stop_button.configure(state="disabled")
        self.prune_button.configure(state="normal")
        self.analyse_button.configure(state="normal")
        
    def destroy(self):
        self.stop_data_collection()
        self.stop_connection()
        super().destroy()

    def check_collection_thread(self):
        if self.collection_thread.is_alive():
            self.after(100, self.check_collection_thread)
        else:
            self.status_queue.put("Status: Stopped")
            self.start_button.configure(state="normal")
            self.stop_button.configure(state="disabled")

    def start_pruning(self):
        #main thread
        self.status_queue.put("Status: Pruning Data...")
        prune_data()
        self.status_queue.put("Status: Pruning Complete")
        print("[DONE] Pruning Complete")

    def start_analysis(self):
        #main thread
        self.status_queue.put("Status: Analysing Data...")
        analyse_data()
        self.status_queue.put("Status: Analysis Complete")

    def collect_data(self):
        BUFFER_SIZE = 66
        CSV_FILE = "data_log.csv"

        if self.socket_connection:
            try:
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

#Data Pruning
def prune_data():
    CSV_FILE = 'data_log.csv'
    OUTPUT_FILE = 'data_log_pruned.csv'
    fields = []
    channels = [[] for _ in range(32)]
    packet_numbers = []
    diff_threshold = 15000

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
            if (ch1[6] == -21846):
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

    if num_rows == 0:
        percentage_corrupt = 0;
    else:
        percentage_corrupt = invalid_data*100/num_rows
    print(f"------ {invalid_data} invalid data points ({percentage_corrupt:.2f}%)")
    print(f"[DONE] Pruned data written to {OUTPUT_FILE}.")

def analyse_data():
    #TODO: change this into something user can input
    CSV_FILE = "data_log_pruned.csv"
    VREF = 2.50 #change this to userinput
    sample_rate = 500 #hz
    time_step = 1/sample_rate

    channels = []
    packet_numbers = []

    with open(CSV_FILE, mode='r',newline='') as file:
        csvreader = csv.reader(file)
        next(csvreader)

        for row in csvreader:
            ch1_str, ch2_str, ch3_str, ch4_str, pn_str = row 
            packet_numbers.append(int(pn_str))
            ch1 = list(ast.literal_eval(ch1_str))
            ch2 = list(ast.literal_eval(ch2_str))
            ch3 = list(ast.literal_eval(ch3_str))
            ch4 = list(ast.literal_eval(ch4_str))
            ch1.pop(6)
            ch2.pop(6)
            ch3.pop(6)
            ch4.pop(6)
            full_row = ch1+ch2+ch3+ch4
            channels.append(full_row)

    channels = np.array(channels,dtype=float)
    packet_numbers = np.array(packet_numbers)

    print(f"[INFO] Loaded data shape: {channels.shape}")  # should be (N, 32)

    channels *= (VREF/32768.0) #normalisation and scaling to VREF

    min_packet = np.min(packet_numbers)
    max_packet = np.max(packet_numbers)
    full_time_axis = np.arange(min_packet,max_packet+1)*time_step

    #data interpolation in case data loss or corruption
    interpolated_channels = []
    for channel in channels.T:
        seconds_time_axis = packet_numbers*time_step
        interpolation_func = interp1d(seconds_time_axis,channel,kind="linear",fill_value="extrapolate")
        interpolated_channel = interpolation_func(full_time_axis)
        interpolated_channels.append(interpolated_channel)
    
    #back to numpy
    interpolated_channels = np.array(interpolated_channels)

    #zero pad limit
    num_samples = len(interpolated_channels[0])
    if (num_samples < 500):
        target_length = 500
    else:
        target_length = num_samples

    #zero padding
    z_pad_channels = [np.pad(channel,(0,target_length-num_samples),'constant') for channel in interpolated_channels]

    #fft compute
    fft_results = [np.fft.fft(channel)/target_length for channel in z_pad_channels]
    frequency_axis = np.fft.fftfreq(target_length, d=time_step)[:target_length//2]
    fft_truncated = [np.abs(fft_result[:target_length//2]) for fft_result in fft_results]

    #plt
    fig, axes = plt.subplots(4, 7, figsize=(22, 11))  # 4 rows, 8 columns
    fig.suptitle("FFT of All 28 Channels", fontsize=12)
    
    for i, fft_result in enumerate(fft_truncated):
        row = (27-i) // 7  # ch1-8 at the bottom instead of top
        col = i % 7   # column index
        ax = axes[row, col]
        
        ax.plot(frequency_axis,fft_result)
        ax.set_title(f"Channel {i+1}")
        ax.set_xlabel("Frequency")
        ax.set_ylabel("Amplitude")
    
    #adjust for overlap
    plt.tight_layout()
    plt.show()
    print("[DONE] Analysis Complete")

# main
if __name__ == "__main__":
    app = MYOLIGHTInterface()
    app.mainloop()
