import socket
import struct
import csv
import numpy as np
import ast
import customtkinter as ctk

def collect_data():
    CSV_FILE = "data_log.csv"
    HOST = '192.168.4.1'  # Default IP address in SoftAP mode
    PORT = 666            # Port used in ESP32C6T-WIFI
    BUFFER_SIZE = 66      #big buffer

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        print(f"Connecting to {HOST}:{PORT}...")
        s.connect((HOST, PORT))
        print("Connected.")

        # Open CSV file for appending data
        with open(CSV_FILE, mode='w', newline='') as file:
            writer = csv.writer(file)
            # Write header if the file is empty
            if file.tell() == 0:
                writer.writerow(['CH1', 'CH2', 'CH3', 'CH4', 'PacketNumber'])

            try:
                while True:
                    data = b''
                    while(len(data) < BUFFER_SIZE):
                        packet = s.recv(BUFFER_SIZE - len(data))
                        if not packet:
                            break
                        data += packet
                    if not data:
                        break

                    # Ensure 66 bytes
                    if len(data) == BUFFER_SIZE:
                        # Unpack the data: 4 ADC readings (16 bytes each) + 1 packet number (2 bytes)
                        CH1 = struct.unpack('<8h', data[0:16])   # First 16 bytes as 8 int16_t values
                        CH2 = struct.unpack('<8h', data[16:32])  # Next 16 bytes
                        CH3 = struct.unpack('<8h', data[32:48])  # Next 16 bytes
                        CH4 = struct.unpack('<8h', data[48:64])  # Next 16 bytes
                        packet_number = struct.unpack('<H', data[64:66])[0]  # Last 2 bytes as uint16_t

                        writer.writerow([CH1, CH2, CH3, CH4, packet_number])
                        
            except KeyboardInterrupt:
                print("\nKeyboard Interrupt.")
            finally:
                s.close()
                print("Connection closed.\n\n")

def prune_data():
    CSV_FILE = "data_log.csv"
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
            #process tuple data into separate arrays
            for ch_idx, ch_str in enumerate([ch1_str, ch2_str, ch3_str, ch4_str]):
                ch_data = ast.literal_eval(ch_str)
                for elem_idx, value in enumerate(ch_data):
                    channel_index = ch_idx*8 + elem_idx
                    channels[channel_index].append(value)

        print("Total no. of rows: %d" %(csvreader.line_num), end="\n")
        print("[DONE] Data parsing\n")
        

    channels = [np.array(ch) for ch in channels]
    packet_numbers = np.array(packet_numbers)
    output_array = []

    num_rows = len(channels[0])
    valid_mask = np.ones(num_rows,dtype=bool)

    for ch in channels:
        diffs = np.diff(ch)
        invalid_packet = np.where(np.abs(diffs) > diff_threshold)[0] + 1
        valid_mask[invalid_packet] = False
    print("[DONE] Data Validity Mask\n")

    invalid_data = 0

    #output data into an array here discarding invalid according to validity mask
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

    percentage_corrupt = invalid_data*100/num_rows
    print("[DONE] Output Data Pruned")
    print(f"------ {invalid_data} invalid data points ({percentage_corrupt:.2f}%)\n")

    with open(OUTPUT_FILE, mode = 'w', newline = '') as outputfile:
        csvwriter = csv.writer(outputfile)
        csvwriter.writerow(fields)
        csvwriter.writerows(output_array)


    print(f"[DONE] Pruned data written to {OUTPUT_FILE}.\n\n")

def main():
    try:
        print("[S] Collecting Data. Press Ctrl+C to stop")
        collect_data()
    except Exception:
        print("[E] Fault in data collection")
    
    print("\n[D] Data collection complete.")

    try:
        print("[S] Start Data Pruning")
        prune_data()
    except Exception:
        print("[E] Fault in data pruning")

    print("\n[D] Done")

if __name__ == "__main__":
    main()
