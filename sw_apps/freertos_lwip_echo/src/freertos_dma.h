#ifndef FREERTOS_DMA_H_
#define FREERTOS_DMA_H_

/* FreeRTOS */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Xilinx */
#include "xaxidma.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xparameters.h"
#include "xdebug.h"
#include "platform.h"

#ifndef _DEBUG_SIMPLE
#define _DEBUG_SIMPLE
extern void xil_printf(const char *format, ...);
#endif

/******************** Constant Definitions **********************************/
#define MAX_PKT_LEN         4096*4

/* You must have this address to generate PROPER TLast */
#define DMA_MCSR            XPAR_MFEIT_DATA_0_S00_AXI_BASEADDR

/* DMA INTR */
#define RX_INTR_ID          XPAR_FABRIC_AXI_DMA_0_S2MM_INTROUT_INTR
#define TX_INTR_ID          XPAR_FABRIC_AXI_DMA_0_MM2S_INTROUT_INTR
void TxIntrHandler(void *Callback);
void RxIntrHandler(void *Callback);

/*
 * Flags interrupt handlers use to notify the application context the events.
 */
volatile int TxDone;
volatile int RxDone;
volatile int Error;

/*
 * using XScuGic
 */
#define INTC            XScuGic
#define INTC_HANDLER    XScuGic_InterruptHandler

/* should we call some MFEIT routines in ISR */
#define USE_EXTERNAL_INTR_HANDLER 1

/*
 * XScuGic must be instance in main
 */
int SetupIntrSystem(   INTC * IntcInstancePtr,
                       XAxiDma * AxiDmaPtr,
                       u16 TxIntrId,
                       u16 RxIntrId);
void DisableIntrSystem(INTC * IntcInstancePtr,
                       u16 TxIntrId,
                       u16 RxIntrId);

/*
 * Timeout loop counter for reset.
 */
#define RESET_TIMEOUT_COUNTER   10000

/*
 * Device hardware build related constants.
 */
#define DMA_DEV_ID          XPAR_AXIDMA_0_DEVICE_ID

// set to your MM2S/S2MM address
#define DMA_BASE_ADDR       0x10000000
#define MEM_BASE_ADDR       (DMA_BASE_ADDR + 0x00000000)

// must be 32bit aligned
#define TX_BUFFER_BASE      (MEM_BASE_ADDR + 0x01000000)
#define RX_BUFFER_BASE      (MEM_BASE_ADDR + 0x02000000)
#define RX_BUFFER_HIGH      (MEM_BASE_ADDR + 0x08000000)

// TCP TX buffers
#define TCP_TX_BASE         RX_BUFFER_HIGH
#define TCP_TX_HIGH         (TCP_TX_BASE   + 0x01000000)
#define TCP_RX_BASE         TCP_TX_HIGH
#define TCP_RX_HIGH         (TCP_RX_BASE   + 0x01000000)

// Truly transfer size = RX_REG_OFFSET + 16*MAX_PKT_LEN
#define RX_MAX_BLOCK        0x00060000  // 6*65536 Bytes
#define RX_REG_OFFSET       0x00001000  // 4096 Bytes

/*
 * The maximum time to block waiting to obtain the xTxBufferMutex
 * to become available.
 */
#define MFEIT_MAX_TIME_TO_WAIT_FOR_TX_BUFFER_MS ( 20 / portTICK_RATE_MS )

/*
 * implement a simple PING-PONG FIFO.
 */
u8 *setRxBasePtr( void );    // set A-port
u8 *getRxBasePtr( void );    // get A-port
u8 *getValidRxBasePtr( void ); // get B-port
u8 *getRxBD(unsigned int bdNum, unsigned int nBytes);

/*--------------- function Definitions ---------------*/
int AXIDMA_Init(XAxiDma *AxiDmaInstPtr, u32 DeviceId);
int AXIDMA_Reset(XAxiDma *AxiDmaInstPtr);
int AXIDMA_Recv(XAxiDma *AxiDmaInstPtr);
int AXIDMA_Send(XAxiDma *AxiDmaInstPtr);

/* low level read/write on DMA */
int AXIDMA_Read(XAxiDma *AxiDmaInstPtr, u8 *RxBufferPtr, unsigned int nBytes);
int AXIDMA_Write(XAxiDma *AxiDmaInstPtr, u8 *TxBufferPtr, unsigned int nBytes);

/* Auxiliary functions */
int CheckData(void);

#endif
