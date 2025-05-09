import asyncio
from bleak import BleakScanner
# B9F3A018-D9E4-36E7-1567-C3117206BF5D HMSoft
async def scanDevices():
    # Create a BleakScanner instance
    scanner = BleakScanner()

    # Start scanning for 5 seconds
    print("Starting scan for 5 seconds...")
    await scanner.start()
    await asyncio.sleep(5)  # Wait for 5 seconds
    await scanner.stop()  # Stop scanning

    # Get discovered devices and advertisement data
    discovered_devices = scanner.discovered_devices_and_advertisement_data

    if not discovered_devices:
        print("No devices found.")
    else:
        for device_address, (device, adv_data) in discovered_devices.items():
            # Print basic device information
            device_name = adv_data.local_name if adv_data.local_name else "Unnamed Device"
            print(f"Device address: {device_address}, Device name: {device_name}")
            
            

    print("Scan complete.")

# Run the async function
asyncio.run(scanDevices())
