import customtkinter as ctk
import socket, struct, threading, csv, sys, subprocess, ast
import numpy as np
import matplotlib.pyplot as plt
import select
from scipy.interpolate import interp1d
from queue import Queue

#Set theme and colour options
ctk.set_appearance_mode("light")
ctk.set_default_color_theme("green")  # Themes: "blue" (standard), "green", "dark-blue"

sample_rate = 0
sample_range = 0
configuration_arr = [0,0,0]
# sample rate , range , operation mode 

# --- GUI App Class ---
class MYOLIGHTInterface(ctk.CTk):
    def __init__(self):
        super().__init__()
        self.title("MYOLIGHT")
        self.geometry("400x600")
        self.resizable(False, False)

        #frame for buttons
        self.button_frame = ctk.CTkFrame(self,fg_color="transparent")
        self.button_frame.grid(row=0,column=0,sticky="nw")

        #BUTTONS AND SUCH
        # ---- Logo ----
        self.label = ctk.CTkLabel(
            self.button_frame,  # Now inside button frame
            text="MYOLIGHT v0.5",
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

        self.control_frame = ctk.CTkFrame(self, fg_color="transparent")
        self.control_frame.grid(row=0,column=1,sticky="ne")

        self.sample_label = ctk.CTkLabel(
            self.control_frame,
            text="Sample Rate (Hz)", 
            font=("Monaco", 12)
        )
        self.sample_label.pack(pady=5)

        self.sample_combo = ctk.CTkComboBox(
            self.control_frame,
            values=["200","500","1000","2000"],
            command=lambda choice: self.update_config(0,choice),
            font=("Monaco", 12),  
            dropdown_font=("Monaco", 12)  
        )
        self.sample_combo.pack(pady=2)

        self.range_label = ctk.CTkLabel(
            self.control_frame,
            text="Range (+-V)", 
            font=("Monaco", 12)
        )
        self.range_label.pack(pady=2)

        self.range_combo = ctk.CTkComboBox(
            self.control_frame,
            values=["2","4","8"],
            command=lambda choice: self.update_config(1,choice),
            font=("Monaco", 12),  
            dropdown_font=("Monaco", 12)  
        )
        self.range_combo.pack(pady=2)

        self.opmode_label = ctk.CTkLabel(
            self.control_frame,
            text="Operation Mode", 
            font=("Monaco", 12)
        )
        self.opmode_label.pack(pady=2)

        self.opmode_combo = ctk.CTkComboBox(
            self.control_frame,
            values=["Default","Single Row (1)","Single Row (2)","Single Row (3)","Single Row (4)"],
            command=lambda choice: self.update_config(2,choice),
            font=("Monaco", 12),  
            dropdown_font=("Monaco", 12)  
        )
        self.opmode_combo.pack(pady=2)

        self.config_button = ctk.CTkButton(
            self.control_frame,
            text="Send Config",
            font=("Monaco", 12),
            corner_radius=0,
            width=160,
            command=self.send_config,
            state="disabled"
        )
        self.config_button.pack(pady=2)

        #status updater
        self.status_label = ctk.CTkLabel(
            self, 
            text="Status: Idle", 
            font=("Monaco", 12))
        self.status_label.grid(row=1,column=0,columnspan=2)

        #console window
        self.console_textbox = ctk.CTkTextbox(
            self,
            width=400,
            height=330,
            font=("Monaco", 12))
        self.console_textbox.grid(row=2,column=0,columnspan=2,sticky="s")
        self.console_textbox.configure(state="disabled") #yall dont interact with this textbox

        self.console_textbox.tag_config("INFO", foreground="#00AA00")   # Green
        self.console_textbox.tag_config("ERROR", foreground="#FF3333")  # Red
        self.console_textbox.tag_config("ECHO", foreground="#00AAAA")   # Cyan
        self.console_textbox.tag_config("WARN", foreground="#FF6600")   # Orange
        self.console_textbox.tag_config("CONN", foreground="#5555FF")   # Blue
        self.console_textbox.tag_config("PRCS", foreground="#f828ff")   # Yellow

        #sysout to this ctk window instead of console
        sys.stdout = self

        #threading
        self.stop_active_data_thread_event = threading.Event()
        # self.stop_data_event = threading.Event()
        # self.collection_thread = None
        self.status_queue = Queue()
        self.after(100, self.process_status_queue)

        self.socket_connection = None
        self.active_data_thread = None
        # self.socket_debug_connection = None

    def write(self,message):
        #print statements to this textbox
        self.console_textbox.configure(state="normal")  # Enable editing

        if message.startswith("[") and "]" in message:
            tag_end = message.find("]") + 1
            tag_text = message[:tag_end]
            rest_text = message[tag_end:]

            tag_name = tag_text.strip("[]")  # e.g., "ERROR"

            # Insert tag part with color
            self.console_textbox.insert("end", tag_text, tag_name)
            self.console_textbox.insert("end", rest_text)
        else:
        # No tag found, print normally
            self.console_textbox.insert("end", message)

        self.console_textbox.configure(state="disabled")  # Disable editing
        self.console_textbox.see("end")  # Auto-scroll to the bottom

    def flush(self):
        #sysout redirection or smth idk
        pass

    def start_connection(self):
        # if "Connected" in connection_result:
        self.status_queue.put("Status: Connecting to MCU...")
        print("[CONN] Connecting to MCU...")
        try:
            self.socket_connection = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket_connection.settimeout(5) 
            self.socket_connection.connect(('192.168.4.1', 666))
            print("[INFO] Data Port Connected (666)")
            self.status_queue.put("Status: Connected to MCU (Data: 666)")

            self.connect_button.configure(state="disabled")
            self.disconnect_button.configure(state="normal")
            self.start_button.configure(state="normal")
            self.stop_button.configure(state="normal")
            self.analyse_button.configure(state="disabled")
            self.config_button.configure(state="normal")

            self.stop_active_data_thread_event.clear()
            self.active_data_thread = threading.Thread(target=self.run_data_thread,daemon=True)
            self.active_data_thread.start()

        except Exception as e:
            self.status_queue.put(f"[ERROR] start_connection: {e}")

    def stop_connection(self):
        try:
            self.stop_active_data_thread_event.set()

            # if self.collection_thread and self.collection_thread.is_alive():
            #     self.stop_data_collection()

            if self.socket_connection:
                self.socket_connection.close()
                self.socket_connection = None

            self.status_queue.put("[CONN] Disconnected from MCU")
            print("[CONN] Disconnected from MCU")
            self.connect_button.configure(state="normal")
            self.disconnect_button.configure(state="disabled")
            self.start_button.configure(state="disabled")
            self.stop_button.configure(state="disabled")
            self.analyse_button.configure(state="normal")
            self.config_button.configure(state="disabled")

        except Exception as e:
            self.status_queue.put(f"[ERROR] stop_connection: {e}")
            print(f"[ERROR] stop_connection: {e}")

    def process_status_queue(self):
        try:
            while True:
                new_text = self.status_queue.get_nowait()
                self.status_label.configure(text=new_text)
        except Exception:
            pass
        self.after(100, self.process_status_queue)

    def start_data_collection(self):
        self.send_command("START")
        self.status_queue.put("Status: Collecting Data...")
        self.start_button.configure(state="disabled")
        self.stop_button.configure(state="normal")
        self.analyse_button.configure(state="disabled")
        self.config_button.configure(state="disabled")

    def stop_data_collection(self):
        self.send_command("STOP")
        self.status_queue.put("Status: Stopping...")
        self.connect_button.configure(state="disabled")
        self.disconnect_button.configure(state="normal")
        self.start_button.configure(state="normal")
        self.stop_button.configure(state="disabled")
        self.analyse_button.configure(state="normal")
        self.config_button.configure(state="normal")
        
    def destroy(self):
        plt.close('all')
        self.stop_data_collection()
        self.stop_connection()
        super().destroy()

    # def check_collection_thread(self):
    #     if self.collection_thread.is_alive():
    #         self.after(100, self.check_collection_thread)
    #     else:
    #         self.status_queue.put("Status: Stopped")
    #         self.start_button.configure(state="normal")
    #         self.stop_button.configure(state="disabled")

    def start_analysis(self):
        #main thread
        prune_data()
        print("[INFO] Pruning Complete")
        self.status_queue.put("Status: Analysing Data...")
        analyse_data()
        self.status_queue.put("Status: Analysis Complete")

    def send_command(self, command: str):
        if self.socket_connection:
            try:
                message = command.strip() + "\n"
                self.socket_connection.sendall(message.encode('utf-8'))
                print(f"[INFO] Sent command: {command}")
            except Exception as e:
                print(f"Error sending command: {e}")

    def send_config(self):
        if self.socket_connection:
            try:
                self.send_command("CONFIG")
                message=",".join(map(str, configuration_arr))
                self.send_command(message)
            except Exception as e:
                print(f"Error sending config: {e}")
    
    def read_incoming(self, sock, timeout=0.1):
        try:
             # Only read if data is available
            ready_to_read, _, _ = select.select([sock], [], [], timeout)
            if not ready_to_read:
                return None, None  # No data — avoid errors
        
            header = sock.recv(3)
            if len(header) < 3:
                return None, None

            msg_type = header[0]
            length = struct.unpack('<H', header[1:])[0]

            payload = b''
            while len(payload) < length:
                chunk = sock.recv(length - len(payload))
                if not chunk:
                    break
                payload += chunk

            return msg_type, payload
        except Exception as e:
            print(f"[ERROR] read_incoming: {e}")
            return None,None
    
    def update_config(self,index,choice):
        global sample_range, sample_rate

        mapping = [
        {"200": 0, "500": 1, "1000": 2, "2000": 3},  # Sample rate
        {"2": 0, "4": 1, "8": 2},  # Range
        {"Default": 0, "Single Row (1)": 1, "Single Row (2)": 2, "Single Row (3)": 3, "Single Row (4)": 4}  # Op Mode
        ]

        if choice in mapping[index]:
            configuration_arr[index] = mapping[index][choice]
            print(f"[INFO] Updated config[{index}] -> {configuration_arr[index]}")

            if index == 0:
                sample_rate = int(choice)
                print(f"[INFO] Updated sample_rate -> {sample_rate}")
            elif index == 1:
                sample_range = int(choice)
                print(f"[INFO] Updated sample_range -> {sample_range}")

    def run_data_thread(self):
        CSV_FILE = "data_log.csv"
        try:
            with open(CSV_FILE,mode='w',newline='') as file:
                writer = csv.writer(file)
                writer.writerow(['CH1', 'CH2', 'CH3', 'CH4', 'PacketNumber'])
                while not self.stop_active_data_thread_event.is_set():
                    try:
                        if(self.socket_connection):
                            msg_type,payload = self.read_incoming(self.socket_connection)
                            if msg_type == 1:
                                print("[ECHO]",payload.decode('utf-8'))
                            elif msg_type == 2:
                                self.parse_data(payload,writer)
                            elif msg_type is not None:
                                print(f"[WARN] Unknown message type: {msg_type}")
                    except Exception as e:
                        print(f"[ERROR] Active Data Thread: {e}")
        except Exception as e:
            print(f"[ERROR] CSV Writer: {e}")

    def parse_data(self,payload,csvwriter): #this function needs to be rewritten alongside the active data thread
        try:
            if len(payload) == 66:
                CH1 = struct.unpack('<8h', payload[0:16])
                CH2 = struct.unpack('<8h', payload[16:32])
                CH3 = struct.unpack('<8h', payload[32:48])
                CH4 = struct.unpack('<8h', payload[48:64])
                packet_number = struct.unpack('<H', payload[64:66])[0]
                csvwriter.writerow([CH1, CH2, CH3, CH4, packet_number])
        except Exception as e:
            self.status_queue.put(f"[ERROR] parse_data: {e}")

#Data Pruning
def prune_data():
    CSV_FILE = 'data_log.csv'
    OUTPUT_FILE = 'data_log_pruned.csv'
    fields = []
    channels = [[] for _ in range(32)]
    packet_numbers = []
    diff_threshold = 30000

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

    print("[INFO] CSV Load Good")
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
            if (ch1[6] == -21846 or ch2[6] == -21846 or ch3[6] == -21846 or ch4[6] == -21846):
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
    print(f"[PRCS] ------ {invalid_data} invalid data points ({percentage_corrupt:.2f}%)")
    print(f"[INFO] Pruned data written to {OUTPUT_FILE}.")

def analyse_data():
    CSV_FILE = "data_log_pruned.csv"
    time_step = 1/sample_rate

    channels = []
    packet_numbers = []

    with open(CSV_FILE, mode='r',newline='') as file:
        csvreader = csv.reader(file)
        next(csvreader)
        # Define the new index positions
        index_map = [1, 3, 5, 7, 0, 2, 4, 6]

        for row in csvreader:
            ch1_str, ch2_str, ch3_str, ch4_str, pn_str = row 
            packet_numbers.append(int(pn_str))
            ch1 = list(ast.literal_eval(ch1_str))
            ch2 = list(ast.literal_eval(ch2_str))
            ch3 = list(ast.literal_eval(ch3_str))
            ch4 = list(ast.literal_eval(ch4_str))
            ch1[:8] = [ch1[i] for i in index_map]
            ch2[:8] = [ch2[i] for i in index_map]
            ch3[:8] = [ch3[i] for i in index_map]
            ch4[:8] = [ch4[i] for i in index_map]
            ch1.pop(7)
            ch2.pop(7)
            ch3.pop(7)
            ch4.pop(7)
            full_row = ch1+ch2+ch3+ch4
            channels.append(full_row)

    channels = np.array(channels,dtype=float)
    packet_numbers = np.array(packet_numbers)

    print(f"[INFO] Loaded data shape: {channels.shape}")  # should be (N, 32)

    channels *= (sample_range/2**15) #normalisation and scaling to VREF

    min_packet = np.min(packet_numbers)
    max_packet = np.max(packet_numbers)
    full_time_axis = np.arange(min_packet,max_packet+1)*time_step

    #data interpolation in case data loss or corruption
    interpolated_channels = []
    for channel in channels.T:
        seconds_time_axis = packet_numbers*time_step
        interpolation_func = interp1d(seconds_time_axis,channel,kind="linear",fill_value="linear")
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
    fft_results = [np.fft.fft(channel)/float(target_length)for channel in z_pad_channels]
    frequency_axis = np.fft.fftfreq(target_length, d=time_step)

    #positive frequencies
    half_length = target_length//2
    frequency_axis = frequency_axis[:half_length]
    fft_truncated = [2*np.abs(fft_result[:half_length]) for fft_result in fft_results]

    #limit to 250 hz bin
    valid_bins = np.where(frequency_axis <= 250)
    frequency_axis = frequency_axis[valid_bins]
    fft_truncated = [fft_result[valid_bins] for fft_result in fft_truncated]
    
    for fft in fft_truncated:
        fft[0] = 0 #remove DC component
        fft[np.where(frequency_axis < 20)] = 0

    #plt
    fig, axes = plt.subplots(4, 7, figsize=(22, 11))  # 4 rows, 8 columns
    fig.suptitle("FFT of All 28 Channels", fontsize=10)
    
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
    print("[PRCS] Analysis Complete")

# main
if __name__ == "__main__":
    app = MYOLIGHTInterface()
    app.mainloop()
