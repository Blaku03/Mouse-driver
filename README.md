# Accelerated Mouse Driver

This repository contains a C++ program that intercepts events from an existing mouse device, applies acceleration to its movements, and forwards the modified events to a virtual mouse device using the Linux `uinput` subsystem.

## Features
- **Mouse acceleration**: Dynamically adjusts mouse movement using a logarithmic function.
- **Virtual mouse device**: Creates a virtual mouse device with enhanced acceleration.
- **Event forwarding**: Handles relative movement (`REL_X`, `REL_Y`), wheel events, and button clicks.

## Prerequisites
- Linux system with `uinput` enabled.
- Development tools: `g++`, `make`, and `libc` headers.
- Root permissions to access `/dev/uinput` and the original mouse device (e.g., `/dev/input/event13`).
