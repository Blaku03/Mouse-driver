# Simple Character Device Driver

## Interacting with the Driver

### 1. Compile the Driver and Load it into the Kernel
Use the provided `Makefile` to compile and load the driver:
```bash
make all
sudo insmod simple_char.ko
```

### 2. Check dmesg for Device Registration

After loading the module, check the kernel log (dmesg) to verify that the device was registered.
```bash
Device registered with major number X
```
Here, X is the dynamically assigned major number for device.

### 3. Create a Device Node

Once you have the major number (X), create the device node:
```bash
sudo mknod /dev/simple_char c X 0
sudo chmod 666 /dev/simple_char
```

### 4. Interact with the Device

Use cat or similar tools to read data from the device:
```bash
cat /dev/simple_char
```

This will output:
```bash
Hello from the kernel!
```

### 5. Unload the Driver

To unload the module and clean up:
```bash
sudo rmmod simple_char
sudo rm /dev/simple_char
```

To clean up the build artifacts:
```bash
make clean
```