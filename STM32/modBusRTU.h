/**
 ******************************************************************************
 * @file           : modBusRTU.h
 * @author         : keyhanSalehi
 * @brief          : header of Custom modBus RTU library for STM32.
 ******************************************************************************
 *
 * This file provides the implementation of modBus RTU functions,
 * including CRC calculation, UART communication, and timer-based timing.
 *
 ******************************************************************************
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* 1. System Header Files */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f1xx_hal.h"
/* 2. Project Header Files */

/* Defines & Macros ----------------------------------------------------------*/

/*!
 * @defgroup
 * @brief Constants and macros available globally.
 */
/*! @def Maximum data size (excluding address, function code, and CRC) */
#define MODBUS_RTU_MAX_DATA_SIZE 250
/*! @def  Maximum frame size (including address, function code, data, and CRC) */
#define MODBUS_RTU_MAX_FRAME_SIZE 256
/*! @defgroup Timeout in milliseconds */
#define MODBUS_RTU_TRANSMIT_TIMEOUT -1  /* Transmit timeOut */
#define MODBUS_RTU_RECEIVED_TIMEOUT 100 /* Received timeOut */

/*! @defgroup Function codes */
/* single bit access */
#define MODBUS_FUNC_READ_COILS 0x01
#define MODBUS_FUNC_READ_DISCRETE_INPUTS 0x02
#define MODBUS_FUNC_WRITE_SINGLE_COIL 0x05
#define MODBUS_FUNC_WRITE_MULTY_COIL 0x0F
/* 16 bit access */
#define MODBUS_FUNC_READ_HOLDING_REGISTERS 0x03
#define MODBUS_FUNC_READ_INPUT_REGISTERS 0x04
#define MODBUS_FUNC_WRITE_SINGLE_REGISTER  0x06
#define MODBUS_FUNC_WRITE_MULTY_REGISTER  0x10
#define MODBUS_FUNC_MASK_WRITE_REGISTER  0x16
#define MODBUS_FUNC_READ_WRITE_MULTY_REGISTER  0x17
#define MODBUS_FUNC_READ_FIFO_QUEUE  0x18
/* file recode access */
#define MODBUS_FUNC_READ_FILE_RECORD  0x14
#define MODBUS_FUNC_WRITE_FILE_RECORD  0x15
/* Diagnostic */
#define MODBUS_FUNC_READ_EXEPTION_STATUS  0x07
#define MODBUS_FUNC_READ_DIAGNOSTIC  0x08
#define MODBUS_FUNC_GET_COM_EVENT_COUNTER  0x0B
#define MODBUS_FUNC_GET_COM_EVENT_LOG  0x0C
#define MODBUS_FUNC_REPORT_SERVER_ID  0x11

/* Typedefs ------------------------------------------------------------------*/
/*!
 * @typedef
 * @brief Typedefs for global use.
 */

/*!
 * @typedef @enum  _modBusRtuErrors
 * @brief modBus Error enumeration handler.
 */
typedef enum _modBusRtuErrors{
	MODBUS_RTU_SUCCESS, /*!< MODBUS_RTU_SUCCESS */
	MODBUS_RTU_ERROR_CRC, /*!< MODBUS_RTU_ERROR_CRC */
	MODBUS_RTU_ERROR_RX_TIMEOUT, /*!< MODBUS_RTU_ERROR_TIMEOUT (Received timeOut) */
	MODBUS_RTU_ERROR_TX_FAILED, /*!< MODBUS_RTU_ERROR_TX_FAILED (UART send data failed)*/
	MODBUS_RTU_ERROR_INVALID_SLAVE_ID,/*!< MODBUS_RTU_ERROR_INVALID_SLAVE_ID */
	MODBUS_RTU_ERROR_INVALID_FRAME, /*!< MODBUS_RTU_ERROR_INVALID_FRAME */
	MODBUS_RTU_RX_BUSY /*!< MODBUS_RTU_RX_BUSY */
} ModbusRTU_ErrorT;

/*!
 * @typedef @struct  _modbusClassHandller
 * @brief modBus RTU handle structure.
 */
typedef struct _modbusClassHandller{
	UART_HandleTypeDef *huart; /*! UART handle */
	TIM_HandleTypeDef *htim; /*! Timer handle */
	uint8_t slaveId; /*! modBus slave ID */
	bool isRxDataReceived;
} ModbusRTU_HandleT;

/* Exported Variables --------------------------------------------------------*/

/* 1. Global Variables */

/* Exported Functions --------------------------------------------------------*/

/* 1. Global Function Declarations */


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
		TIM_HandleTypeDef *htim, uint8_t slaveId);

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
		uint8_t functionCode, uint8_t *data, size_t dataSize);

/*!
 * @fn 	  ModbusRTU_ErrorT modbusRTUReciveData(ModbusRTU_HandleT *modbus, size_t dataSize)
 * @brief Receive a modBus RTU response.
 *
 * @param modBus Pointer to the ModbusRTU instance.
 * @param dataSize Size of the data buffer (with out SlaveId and function code and CRC).
 * @return result @ref ModbusRTU_ErrorT in modBus_RTU.h header file
 */
ModbusRTU_ErrorT modbusRTUReciveData(ModbusRTU_HandleT *modbus, size_t dataSize);

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
		size_t dataSize);

#ifdef __cplusplus
}
#endif

#endif // MODBUS_RTU_H
