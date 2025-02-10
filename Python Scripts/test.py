import asyncio
import os,sys
from bleak import BleakClient, BleakScanner, discover

HID_UUID = '0000252a-0000-1000-8000-00805f9b34fb'
B_SIG_DATA_UUID = '0000260b-0000-1000-8000-00805f9b34fb'

class DeviceBle():

    def __init__(self):
        self.client = None
        self.uuid_HID_service = HID_UUID
        self.uuid_b_sig_data_characteristic = B_SIG_DATA_UUID

    async def discover(self):
        devices = await BleakScanner.discover(5.0,return_adv=True)
        for device in devices:
            advertisement_data = devices[device][1]
            if(advertisement_data.local_name == "ESP32C6T"):
                if(advertisement_data.rssi > -100):
                    self.device = devices[device]
                    return device
                
    async def connect(self):
        address = await self.discover()
        if address is not None:
            try:
                print("Found device at address: %s" % (address))
                print("Connecting...")
                self.client = BleakClient(address)
                await self.client.connect()
                print("Connected.")
            except:
                raise Exception("Failed to connect")
        else:
            raise Exception("Did not find device")
        
    async def disconnect(self):
        try:
            print("Disconnecting...")
            await self.client.disconnect()
            print("Disconnected.")
        except:
            raise Exception("FAILED TO DISCONNECT. HANGING POSSIBLE.")
        
    async def readData(self):
        try:
            data = await self.client.start_notify(self.uuid_b_sig_data_characteristic)
            print(f"Data read from characteristic {self.uuid_b_sig_data_characteristic}: {data}")
        except Exception as e:
            print(f"Error reading data: {e}")
        
    async def notification_handler(self,sender,data):
        print(f"\r{data}")
        sys.stdout.flush()

    async def start_notifications(self):
        try:
            await self.client.start_notify(self.uuid_b_sig_data_characteristic,self.notification_handler)
            print(f"Started notification on {self.uuid_b_sig_data_characteristic}")
            await asyncio.sleep(30)
            await self.client.stop_notify(self.uuid_b_sig_data_characteristic)
            print(f"Stopped notifications on {self.uuid_b_sig_data_characteristic}")

        except Exception as e:
            print(f"Notification Failed: {e}")

async def main():
    device = DeviceBle()
    try:
        await device.connect()
        await device.start_notifications()
        await device.disconnect()
    except Exception as e:
        print(e)

asyncio.run(main())
