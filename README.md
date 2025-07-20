# MODBUS RTU Library for Embedded Systems

![MODBUS RTU Protocol](https://img.shields.io/badge/Protocol-MODBUS%20RTU-blue)
![License](https://img.shields.io/badge/License-MIT-green)

A lightweight, hardware-agnostic implementation of MODBUS RTU protocol in C for embedded systems. Designed for resource-constrained microcontrollers.

## Supported Platforms
- **STM32** (F1, F4, H7 series tested)
- **ESP32** (with UART peripheral)
- **NXP** (LPC, MKV)
- Any microcontroller with UART capability

## Features
- 🚀 **Dual Mode**: Supports both Master and Slave operation
- ⚡ **Efficient**: Low memory footprint (<2KB RAM)
- 🛡️ **Robust**: Automatic CRC validation and timeout handling
- 🔧 **Configurable**: Adjustable frame sizes and timeouts
- 🧩 **Modular**: Easy porting layer for different MCUs

## Function Codes Supported
| Code | Description                  |
|------|------------------------------|
| 0x01 | Read Coils                   |
| 0x02 | Read Discrete Inputs         |
| 0x03 | Read Holding Registers       |
| 0x04 | Read Input Registers         |
| 0x05 | Write Single Coil            |
| 0x06 | Write Single Register        |
| 0x0F | Write Multiple Coils         |
| 0x10 | Write Multiple Registers     |
| 0x16 | Mask Write Register          |
| 0x17 | Read/Write Multiple Registers|

## Getting Started

### 1. Hardware Setup
Connect your UART peripheral with:
- Proper baud rate (1200-115200)
- RS485 transceiver (if needed)
- Correct parity settings (typically Even/NONE)

### 2. Library Integration
```c
#include "modbus_rtu.h"

// 1. Implement hardware-specific functions:
void HAL_UART_MspInit(UART_HandleTypeDef* huart) {
    // Your UART initialization
}

// 2. Configure your MODBUS instance
ModbusRTU_HandleT hmodbus;
modbusRTUInit(&hmodbus, &huart1, &htim2, DEVICE_ADDRESS);
