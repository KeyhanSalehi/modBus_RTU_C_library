/**
 ******************************************************************************
 * @file           : modBusRTU.c
 * @author         : keyhanSalehi
 * @brief          : Custom modBus RTU library for STM32.
 ******************************************************************************
 *
 * This file provides the implementation of modBus RTU functions,
 * including CRC calculation, UART communication, and timer-based timing.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

/* 1. System Header Files */
#include "stm32f1xx_hal.h"
/* 2. Project Header Files */

/* 3. Module Header File */
#include <modBusRTU.h>

/* Defines & Macros ----------------------------------------------------------*/

/*!
 * @defgroup
 * @brief Constants and macros available globally.
 */

/* Typedefs ------------------------------------------------------------------*/

/*!
 * @typedef
 * @brief Typedefs for global use.
 */

/*!
 * @typedef @struct _modeBusPacket
 * @brief modBus packet structure
 */
typedef struct _modeBusPacket{
	uint8_t slaveId;
	uint8_t functionCode;
	uint8_t data[MODBUS_RTU_MAX_DATA_SIZE + 2]; // + 2 =  for CRC
} modBusPacket_t;

/* Variables -----------------------------------------------------------------*/

/* 1. Global Variables */

/* 2. Static Variables */

/*! @var @brief Global TX/RX variables for modBus RTU communication */
static modBusPacket_t txPacket = { 0 };
static modBusPacket_t rxPacket = { 0 };

/* Function Declarations -----------------------------------------------------*/

/* 1. Local Prototype Functions */

/*! @fn @private */
static uint16_t ModbusRTU_CalculateCRC(uint8_t *data, size_t length);

/* 2. Global Function Declarations */

/*!
 * @fn    void modbusRTUInit(ModbusRTU_HandleT *modbus, UART_HandleTypeDef *huart, TIM_HandleTypeDef *htim, uint8_t slaveId)
 * @brief Initialize the modBus RTU instance.
 *
 * @param modBus Pointer to the ModbusRTU instance.
 * @param huart Pointer to the UART handle.
 * @param htim Pointer to the timer handle.
 * @param slaveId The modBus slave ID.
 * @param baudRate The UART baud rate.
 */
void modbusRTUInit(ModbusRTU_HandleT *modbus, UART_HandleTypeDef *huart,
		TIM_HandleTypeDef *htim, uint8_t slaveId) {
	modbus->huart = huart;
	modbus->htim = htim;
	modbus->slaveId = slaveId;
	/* Start the timer in interrupt mode */
	HAL_TIM_Base_Start_IT(modbus->htim);
}

/*!
 * @fn    ModbusRTU_ErrorT modbusRTUSendData(ModbusRTU_HandleT *modbus,uint8_t functionCode, uint8_t *data, size_t dataSize)
 * @brief Send a modBus RTU request.
 *
 * @param modBus Pointer to the ModbusRTU instance.
 * @param functionCode The modBus function code (e.g., 0x03, 0x06).
 * @param address The register address.
 * @param data Data to send to the device.
 * @param dataSize Size of the data.
 * @return result @ref ModbusRTU_ErrorT in modBus_RTU.h header file
 */
ModbusRTU_ErrorT modbusRTUSendData(ModbusRTU_HandleT *modbus,
		uint8_t functionCode, uint8_t *data, size_t dataSize) {

	/* local variable */
	uint16_t calCrc = 0;
	ModbusRTU_ErrorT result = MODBUS_RTU_SUCCESS;

	/* Validate data size */
	if (dataSize > MODBUS_RTU_MAX_DATA_SIZE) {
		result = MODBUS_RTU_ERROR_INVALID_FRAME;
	} else {

		/* Construct the modBus RTU frame */
		txPacket.slaveId = modbus->slaveId;
		txPacket.functionCode = functionCode;

		memcpy(txPacket.data, data, dataSize);

		/* Calculate CRC */
		calCrc = ModbusRTU_CalculateCRC((uint8_t*) &txPacket, dataSize + 2); /* +2 = 1(slaveId) + 1(functionCode)) */
		txPacket.data[dataSize] = calCrc & 0xFF; /* CRC low byte */
		txPacket.data[dataSize + 1] = (calCrc >> 8) & 0xFF; /* CRC high byte */

		/* Send the frame over UART */
		if (HAL_OK
				!= HAL_UART_Transmit(modbus->huart, (uint8_t*) &txPacket,
						dataSize + 4, MODBUS_RTU_TRANSMIT_TIMEOUT)) { /* +4 = 1(slaveId) + 1(functionCode) + 2(CRC) */
			result = MODBUS_RTU_ERROR_TX_FAILED;
		}
	}

	return result;
}

/*!
 * @fn 	  ModbusRTU_ErrorT modbusRTUReciveData(ModbusRTU_HandleT *modbus, size_t dataSize)
 * @brief Receive a modBus RTU response.
 *
 * @param modBus Pointer to the ModbusRTU instance.
 * @param dataSize Size of the data buffer (with out SlaveId and function code and CRC).
 * @return result @ref ModbusRTU_ErrorT in modBus_RTU.h header file
 */
ModbusRTU_ErrorT modbusRTUReciveData(ModbusRTU_HandleT *modbus, size_t dataSize) {

	/* local variable */
	ModbusRTU_ErrorT result = MODBUS_RTU_SUCCESS;

	/* Validate data size */
	if (dataSize > MODBUS_RTU_MAX_DATA_SIZE) {
		result = MODBUS_RTU_ERROR_INVALID_FRAME;
	} else {
		/* start communicate for received data */
		HAL_UART_Receive_IT(modbus->huart, (uint8_t*) &rxPacket, dataSize + 4); /* +4 = 1(slaveId) + 1(functionCode) + 2(CRC) */
	}

	return result;
}

/*!
 * @fn    ModbusRTU_ErrorT modbusRTUCheckRxState(ModbusRTU_HandleT *modbus, uint8_t *data,size_t dataSize)
 * @brief check state and validation of received data.
 *
 * @param modBus Pointer to the ModbusRTU instance.
 * @param data Pointer to store the received data.
 * @param dataSize Size of the data buffer.
 * @return result @ref ModbusRTU_ErrorT in modBus_RTU.h header file
 */
ModbusRTU_ErrorT modbusRTUCheckRxState(ModbusRTU_HandleT *modbus, uint8_t *data,
		size_t dataSize) {

	/* local variable */
	uint16_t calCrc = 0, receivedCRC = 0;
	ModbusRTU_ErrorT result = MODBUS_RTU_SUCCESS;

	if (true == modbus->isRxDataReceived) { /* if data received successfully */
		/* check received data */
		/* Validate the slave ID */
		if (rxPacket.slaveId != modbus->slaveId) {
			result = MODBUS_RTU_ERROR_INVALID_SLAVE_ID; /* Invalid slave ID */
		} else {
			/* Calculate the CRC for the received packet (excluding the last 2 bytes which are the CRC) */
			calCrc = ModbusRTU_CalculateCRC((uint8_t*) &rxPacket, dataSize + 2); /* + 2 = 1(slaveId) + 1(functionCode) */

			/* Extract the received CRC from the packet */
			receivedCRC = (rxPacket.data[dataSize] << 8)
					| rxPacket.data[dataSize + 1];

			/* Compare the calculated CRC with the received CRC */
			if (calCrc != receivedCRC) {
				result = MODBUS_RTU_ERROR_CRC; /* CRC mismatch */
			} else {
				/* Unpack the received data*/
				memcpy(data, rxPacket.data, dataSize);
			}
		}

		/* reset flag */
		modbus->isRxDataReceived = false;

	} else if (false) { /* TODO timeOut */
		result = MODBUS_RTU_ERROR_RX_TIMEOUT;
		/* reset flag */
		modbus->isRxDataReceived = false;
	} else {
		result = MODBUS_RTU_RX_BUSY;
	}

	return result;
}

/* 3. Local Function Declarations */

/*!
 * @fn    static uint16_t ModbusRTU_CalculateCRC(uint8_t *data, size_t length)
 * @brief Calculate the modBus RTU CRC-16.
 *
 * @param data Pointer to the data buffer.
 * @param length Length of the data.
 * @return The calculated CRC-16 value.
 *
 *
 * @note : TODO : you can use your hardware calculator CRC in your MCU.
 */
static uint16_t ModbusRTU_CalculateCRC(uint8_t *data, size_t length) {
	uint16_t crc = 0xFFFF; /* Initial CRC value */

	for (size_t i = 0; i < length; i++) {
		crc ^= data[i];
		for (uint8_t j = 0; j < 8; j++) {
			if (crc & 0x0001) {
				crc >>= 1;
				crc ^= 0xA001; /* Polynomial for modBus RTU */
			} else {
				crc >>= 1;
			}
		}
	}

	return crc;
}

/************************ (C) COPYRIGHT [KeyhanSalehi] *****END OF FILE****/
