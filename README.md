# Simple Character Device Driver

## Interacting with the Driver

### 1. Compile the Driver and Load it into the Kernel
Use the provided `Makefile` to compile and load the driver:
```bash
make all
sudo insmod mouse_tracker.ko
```

### 2. Check dmesg for Device Registration

After loading the module, check the kernel log (dmesg) to verify that the device was registered.
```bash
Device registered with major number X
```
Here, X is the dynamically assigned major number for device.

### 3. Interact with the Device

Use cat or similar tools to read data from the device:
```bash
cat /dev/mouse_tracker
```

This will output:
```bash
Hello from the kernel!
```

### 4. Unload the Driver

To unload the module and clean up:
```bash
sudo rmmod mouse_tracker
sudo rm /dev/mouse_tracker
```

To clean up the build artifacts:
```bash
make clean
```