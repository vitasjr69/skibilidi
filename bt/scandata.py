import asyncio
from bleak import BleakClient

# Replace with your device's address or UUID
DEVICE_ADDRESS = "B9F3A018-D9E4-36E7-1567-C3117206BF5D"
class DebugData():
    def __init__(self, direction, voltage, distanceRight, distanceLeft):
        self.direction = direction
        self.voltage = voltage
        self.distanceRight = distanceRight
        self.distanceLeft = distanceLeft


    def __str__(self):
        return f"DebugData(direction={self.direction}, voltage={self.voltage}, distanceRight={self.distanceRight}, distanceLeft={self.distanceLeft})"


recieved_data = []

# This is a typical characteristic UUID for HM-10 (UART service)
# Replace with the actual characteristic UUID if it's different
CHARACTERISTIC_UUID = "1238"  # UART RX

data_stream = ""

async def notification_handler(sender: int, data: bytearray):
   
   
    """Callback function that will handle incoming notifications"""
    #print("data comming!")
    #print(f"Received data from {sender}: {data.decode()}")

    global data_stream
    data_stream = data_stream + data.decode()
    if data_stream.endswith("\n"):
        data_parts = data_stream.strip().split(",")
        if len(data_parts) == 4:
            try:
                direction = int(data_parts[0])
                voltage = float(data_parts[1])
                distanceRight = float(data_parts[2])
                distanceLeft = float(data_parts[3])
                debug_data = DebugData(direction, voltage, distanceRight, distanceLeft)
                recieved_data.append(debug_data)
                print(debug_data)
            except ValueError as e:
                print(f"Error parsing measurments: {e}")
        else:
            print(f"Unexpected data format: {data_stream}")
        
        
        data_stream = ""



import aioconsole

async def main():
    print("Scanning for devices...")
    async with BleakClient(DEVICE_ADDRESS) as client:
        if not client.is_connected:
            print(f"Failed to connect to {DEVICE_ADDRESS}")
            return

        print(f"Connected to {DEVICE_ADDRESS}")
        await client.start_notify(CHARACTERISTIC_UUID, notification_handler)
        print("Listening for data...")

        while True:
            user_input = await aioconsole.ainput()

            if user_input == "STOP":
                await client.stop_notify(CHARACTERISTIC_UUID)
                print("Stopped listening to the device.")
                break
            else:
                try:
                    # Convert string to bytes and send via BLE
                    await client.write_gatt_char(CHARACTERISTIC_UUID, user_input.encode())
                    print(f"Sent: {user_input}")
                except Exception as e:
                    print(f"Failed to send data: {e}")


print("Stopped listening to the device.")


# Run the asyncio event loop
asyncio.run(main())
