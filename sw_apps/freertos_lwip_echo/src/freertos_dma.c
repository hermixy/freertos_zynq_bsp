#include "freertos_dma.h"

/* PRINT_ will print the received data */
#define PRINT_DBG_DMA 0
#define DBG_DMA_TX    1

/* must use system GIC controller */
extern XScuGic xInterruptController;

/* prepare a PING-PONG buffer */
const u8 *rx_a_ptr         =(u8 *)RX_BUFFER_BASE;
const u8 *rx_b_ptr         =(u8 *)(RX_BUFFER_BASE + RX_MAX_BLOCK);
static u8 *rxbd            =(u8 *)RX_BUFFER_BASE;

/* should we invalidate these memory */
char *tcp_tx_ptr           =(char *)TCP_TX_BASE;
char *tcp_rx_ptr           =(char *)TCP_RX_BASE;

/* Semaphore used to guard the TX buffer. */
static xSemaphoreHandle xmTxBufferMutex = NULL;

/*---------------------------------------------------------------------------*/

int AXIDMA_Init(XAxiDma *AxiDmaInstPtr, u32 DeviceId)
{
int xStatus;
XAxiDma_Config *xConfig;

    /*
     * Create the mutex used to ensure
     * mutual exclusive access to the Tx buffer.
     */
    xmTxBufferMutex = xSemaphoreCreateMutex();
    configASSERT( xmTxBufferMutex );

    /* lookup the DMA device */
    xConfig = XAxiDma_LookupConfig(DeviceId);
    if (!xConfig) {
        xil_printf("No config found for %d\r\n", DMA_DEV_ID);
        return XST_FAILURE;
    }

    /* Initialize DMA engine */
    xStatus = XAxiDma_CfgInitialize(AxiDmaInstPtr, xConfig);
    if (xStatus != XST_SUCCESS) {
        xil_printf("Initialization failed %d\r\n", xStatus);
        return XST_FAILURE;
    }

    /* enable simple DMA mode, we do not work in SG mode. */
    if(XAxiDma_HasSg(AxiDmaInstPtr)) {
        xil_printf("Device configured as SG mode\r\n");
        return XST_FAILURE;
    }

    /* Set up Interrupt system  */
    xStatus = SetupIntrSystem(&xInterruptController,
                             AxiDmaInstPtr,
                             TX_INTR_ID,
                             RX_INTR_ID);
    if (xStatus != XST_SUCCESS) {
        xil_printf("Failed INTR setup\r\n");
        return XST_FAILURE;
    }

    /*
     * Disable interrupts.
     */
    XAxiDma_IntrDisable(AxiDmaInstPtr,
                        XAXIDMA_IRQ_ALL_MASK,
                        XAXIDMA_DEVICE_TO_DMA);
    XAxiDma_IntrDisable(AxiDmaInstPtr,
                        XAXIDMA_IRQ_ALL_MASK,
                        XAXIDMA_DMA_TO_DEVICE);

    /* Enable all interrupts */
    XAxiDma_IntrEnable(AxiDmaInstPtr,
                       XAXIDMA_IRQ_ALL_MASK,
                       XAXIDMA_DMA_TO_DEVICE);

    XAxiDma_IntrEnable(AxiDmaInstPtr,
                       XAXIDMA_IRQ_ALL_MASK,
                       XAXIDMA_DEVICE_TO_DMA);

    /* Initialize flags before start transfer test  */
    TxDone = 0;
    RxDone = 0;
    Error  = 0;

    return XST_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * Reset should never fail for transmit channel
 */
int AXIDMA_Reset(XAxiDma *AxiDmaInstPtr)
{

    /*
     * Disable interrupts, we use polling mode
     */
    XAxiDma_IntrDisable( AxiDmaInstPtr,
                         XAXIDMA_IRQ_ALL_MASK,
                         XAXIDMA_DEVICE_TO_DMA );
    XAxiDma_IntrDisable( AxiDmaInstPtr,
                         XAXIDMA_IRQ_ALL_MASK,
                         XAXIDMA_DMA_TO_DEVICE );

    /*
     * After the reset:
     *  - All interrupts are disabled.
     *  - Engine is halted
     */
    XAxiDma_Reset(AxiDmaInstPtr);

    int TimeOut = RESET_TIMEOUT_COUNTER;

    while (TimeOut) {
        if (XAxiDma_ResetIsDone(AxiDmaInstPtr)) {
            break;
        }

        TimeOut -= 1;
    }

    /* Enable all interrupts */
    XAxiDma_IntrEnable( AxiDmaInstPtr,
                        XAXIDMA_IRQ_ALL_MASK,
                        XAXIDMA_DMA_TO_DEVICE );

    XAxiDma_IntrEnable( AxiDmaInstPtr,
                        XAXIDMA_IRQ_ALL_MASK,
                        XAXIDMA_DEVICE_TO_DMA );

    /* Initialize flags before start transfer test  */
    TxDone = 0;
    RxDone = 0;
    Error  = 0;

    return 0;
}
/*---------------------------------------------------------------------------*/

/*
 * low level DMA read
 */
int AXIDMA_Read(XAxiDma *AxiDmaInstPtr, u8 *RxBufferPtr, unsigned int nBytes)
{

    /*
     * set TLast Timer = N WORD (i.e., Byte >> 2)
     */
    u32 *dma_cnt_ptr = (void *)DMA_MCSR;
    *(dma_cnt_ptr + 0x1) = (nBytes >> 2);

    /* wait for sometime before AXI-bus stable */
    int i;
    for(i=0;i<100;i++){;}

    int xStatus;
    xStatus = XAxiDma_SimpleTransfer( AxiDmaInstPtr,
                                      (u32)RxBufferPtr,
                                      nBytes,
                                      XAXIDMA_DEVICE_TO_DMA );

    if (xStatus != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /* we do not wait here and left all for INTR */
    return XST_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/* @important deprecated */
int AXIDMA_Recv(XAxiDma *AxiDmaInstPtr)
{

    /*
     * set TLast Timer = N WORD (i.e., Byte >> 2)
     */
    u32 *dma_cnt_ptr = (void *)DMA_MCSR;
    *(dma_cnt_ptr + 0x1) = (MAX_PKT_LEN >> 2);

    /* wait for sometime before AXI-bus stable */
    int i;
    for(i=0;i<100;i++){;}

    int xStatus;
    u8 *RxBufferPtr = setRxBasePtr();

    xStatus = XAxiDma_SimpleTransfer( AxiDmaInstPtr,
                                      (u32)RxBufferPtr,
                                      MAX_PKT_LEN,
                                      XAXIDMA_DEVICE_TO_DMA );

    if (xStatus != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /*
     * Polling template.
     *
     * while (XAxiDma_Busy(AxiDmaInstPtr,XAXIDMA_DEVICE_TO_DMA)){;}
     */

    /*
     * Interrupt template.
     *
     * while(!RxDone && !Error){;}
     *
     * if (Error)
     *     return XST_FAILURE;
     *
     * if (RxDone)
     *     RxDone = 0;
     */

    /* check data valid (Xil_DCacheInvalidateRange must be called) */
    //if (CheckData() != XST_SUCCESS)
    //    return XST_FAILURE;

    return XST_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/*
 * low level DMA write
 */
int AXIDMA_Write(XAxiDma *AxiDmaInstPtr, u8 *TxBufferPtr, unsigned int nBytes)
{
    int xStatus;

    /*
     * Flush the SrcBuffer before the DMA transfer,
     * in case the Data Cache is enabled.
     */
    Xil_DCacheFlushRange((u32)TxBufferPtr, nBytes);

    xStatus = XAxiDma_SimpleTransfer( AxiDmaInstPtr,
                                      (u32)TxBufferPtr,
                                      nBytes,
                                      XAXIDMA_DMA_TO_DEVICE );

    if (xStatus != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /*
     * Interrupt, we wait for transmission terminated.
     */
    while( !TxDone && !Error){;}

    if (Error)
        return XST_FAILURE;

    if (TxDone)
        TxDone = 0;

    return XST_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/* @important, deprecated */
int AXIDMA_Send(XAxiDma *AxiDmaInstPtr)
{
    int xStatus;

    /* transfer in terms of BYTE */
    u8 *TxBufferPtr;

    /* build TX Buffer */
    TxBufferPtr = (u8 *)TX_BUFFER_BASE ;

    /* initialize with fixed pattern (time consuming) */
    /*
    int Index;
    u8 Value = TEST_START_VALUE;
    for(Index = 0; Index < MAX_PKT_LEN; Index ++) {
        TxBufferPtr[Index] = Value;
        Value = (Value + 1) & 0xFF;
    }
    */

    /*
     * Flush the SrcBuffer before the DMA transfer,
     * in case the Data Cache is enabled.
     */
    Xil_DCacheFlushRange((u32)TxBufferPtr, MAX_PKT_LEN);

    xStatus = XAxiDma_SimpleTransfer( AxiDmaInstPtr,
                                      (u32)TxBufferPtr,
                                      MAX_PKT_LEN,
                                      XAXIDMA_DMA_TO_DEVICE );

    if (xStatus != XST_SUCCESS) {
        return XST_FAILURE;
    }

    /*
     * polling
     *
     * while (XAxiDma_Busy(AxiDmaInstPtr,XAXIDMA_DMA_TO_DEVICE)){;}
     */

    /*
     * Interrupt.
     */
    while( !TxDone && !Error){;}

    if (Error)
        return XST_FAILURE;

    if (TxDone)
        TxDone = 0;

    return XST_SUCCESS;
}
/*---------------------------------------------------------------------------*/

/* check whether the received data are valid */
int CheckData(void)
{
    u8 *RxPacket = getRxBasePtr();

    /* Invalidate the DestBuffer before receiving the data,
     * in case the Data Cache is enabled.
     */
    Xil_DCacheInvalidateRange((u32)RxPacket, MAX_PKT_LEN);

    /* we only print the readed output */
#if(PRINT_DBG_DMA)
    int Index = 0;
    for(Index = 0; Index < MAX_PKT_LEN; Index++) {
        xil_printf("%x ", RxPacket[Index]);
    }
    xil_printf("\r\n");
#endif

    return XST_SUCCESS;
}

/*****************************************************************************/
/*
*
* This is the DMA TX Interrupt handler function.
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* is present, then sets the TxDone.flag
*
* @param    Callback is a pointer to TX channel of the DMA engine.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void TxIntrHandler(void *Callback)
{

    u32 IrqStatus;
    int TimeOut;
    XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

    /* Read pending interrupts */
    IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DMA_TO_DEVICE);

    /* Acknowledge pending interrupts */
    XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DMA_TO_DEVICE);

    /*
     * If no interrupt is asserted, we do not do anything
     */
    if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
        return;
    }

    /*
     * If error interrupt is asserted, raise error flag, reset the
     * hardware to recover from the error, and return with no further
     * processing.
     */
    if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {
        Error = 1;

        /*
         * Reset should never fail for transmit channel
         */
        XAxiDma_Reset(AxiDmaInst);

        TimeOut = RESET_TIMEOUT_COUNTER;

        while (TimeOut) {
            if (XAxiDma_ResetIsDone(AxiDmaInst)) {
                break;
            }

            TimeOut -= 1;
        }

        return;
    }

    /*
     * If Completion interrupt is asserted, then set the TxDone flag
     */
    if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {
        TxDone = 1;
    }

}

/*****************************************************************************/
/*
*
* This is the DMA RX interrupt handler function
*
* It gets the interrupt status from the hardware, acknowledges it, and if any
* error happens, it resets the hardware. Otherwise, if a completion interrupt
* is present, then it sets the RxDone flag.
*
* @param    Callback is a pointer to RX channel of the DMA engine.
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void RxIntrHandler(void *Callback)
{
    u32 IrqStatus;
    int TimeOut;
    XAxiDma *AxiDmaInst = (XAxiDma *)Callback;

    /* Read pending interrupts */
    IrqStatus = XAxiDma_IntrGetIrq(AxiDmaInst, XAXIDMA_DEVICE_TO_DMA);

    /* Acknowledge pending interrupts */
    XAxiDma_IntrAckIrq(AxiDmaInst, IrqStatus, XAXIDMA_DEVICE_TO_DMA);

    /*
     * If no interrupt is asserted, we do not do anything
     */
    if (!(IrqStatus & XAXIDMA_IRQ_ALL_MASK)) {
        return;
    }

    /*
     * If error interrupt is asserted, raise error flag, reset the
     * hardware to recover from the error, and return with no further
     * processing.
     */
    if ((IrqStatus & XAXIDMA_IRQ_ERROR_MASK)) {
        Error = 1;

        /* Reset could fail and hang
         * NEED a way to handle this or do not call it??
         */
        XAxiDma_Reset(AxiDmaInst);

        TimeOut = RESET_TIMEOUT_COUNTER;

        while (TimeOut) {
            if(XAxiDma_ResetIsDone(AxiDmaInst)) {
                break;
            }

            TimeOut -= 1;
        }

        return;
    }

    /*
     * If completion interrupt is asserted, then set RxDone flag
     */
    if ((IrqStatus & XAXIDMA_IRQ_IOC_MASK)) {
        RxDone = 1;

        /* connect with outside world */
#if(USE_EXTERNAL_INTR_HANDLER)
        extern void stop_ADC(void);
        stop_ADC();
#endif

    }

}

/*****************************************************************************/
/*
* This function setups the interrupt system so interrupts can occur for the
* DMA, it assumes INTC component exists in the hardware system.
*
* @param    IntcInstancePtr is a pointer to the instance of the INTC.
* @param    AxiDmaPtr is a pointer to the instance of the DMA engine
* @param    TxIntrId is the TX channel Interrupt ID.
* @param    RxIntrId is the RX channel Interrupt ID.
*
* @return
*       - XST_SUCCESS if successful,
*       - XST_FAILURE.if not succesful
*
* @note     None.
*
******************************************************************************/
int SetupIntrSystem(INTC * IntcInstancePtr,
                    XAxiDma * AxiDmaPtr,
                    u16 TxIntrId,
                    u16 RxIntrId)
{
    int Status;

    /* edge sensitive (rising) */
    XScuGic_SetPriorityTriggerType(IntcInstancePtr, TxIntrId, 0xA0, 0x3);
    XScuGic_SetPriorityTriggerType(IntcInstancePtr, RxIntrId, 0xA0, 0x3);

    /*
     * Connect the device driver handler that will be called when an
     * interrupt for the device occurs, the handler defined above performs
     * the specific interrupt processing for the device.
     */

    /* connect to TX Intr pin */
    Status = XScuGic_Connect( IntcInstancePtr,
                              TxIntrId,
                              (Xil_InterruptHandler)TxIntrHandler,
                              AxiDmaPtr );

    if (Status != XST_SUCCESS) {
        return Status;
    }

    /* connect to RX INTR pin */
    Status = XScuGic_Connect( IntcInstancePtr,
                              RxIntrId,
                              (Xil_InterruptHandler)RxIntrHandler,
                              AxiDmaPtr );

    if (Status != XST_SUCCESS) {
        return Status;
    }

    /* enable both interrupt */
    XScuGic_Enable(IntcInstancePtr, TxIntrId);
    XScuGic_Enable(IntcInstancePtr, RxIntrId);

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function disables the interrupts for DMA engine.
*
* @param    IntcInstancePtr is the pointer to the INTC component instance
* @param    TxIntrId is interrupt ID associated w/ DMA TX channel
* @param    RxIntrId is interrupt ID associated w/ DMA RX channel
*
* @return   None.
*
* @note     None.
*
******************************************************************************/
void DisableIntrSystem(INTC * IntcInstancePtr,
                       u16 TxIntrId,
                       u16 RxIntrId)
{
    XScuGic_Disconnect(IntcInstancePtr, TxIntrId);
    XScuGic_Disconnect(IntcInstancePtr, RxIntrId);
}

/*---------------------------------------------------------------------------*/
/*
 * toggle PING-PONG for MFEIT
 * you may add a semaphore to control the toggling of
 * the RX_Base_ptr, in case TCP-IP is reading a buffer.
 */
static unsigned int dma_nOverride = 0;
u8 *setRxBasePtr()
{
    if (rxbd == rx_a_ptr)
        rxbd = (u8 *)rx_b_ptr;
    else
        rxbd = (u8 *)rx_a_ptr;

    /* Finished with the TX buffer. Produce new. */
    if (xSemaphoreTake( xmTxBufferMutex,0 ) != pdPASS)
        ;
    else {
        /* if the producer takes the mutex, it means that the
         * consumer failed to process the message, an override
         * may happens due to TCP/IP traffic */
        dma_nOverride ++;
#if(DBG_DMA_TX)
        xil_printf("(%8d) Producer override Consumer\n", dma_nOverride);
#endif
    }

    /* regardless of whether DMA succeed or fail,
     * always produce a new RX BD */
    xSemaphoreGive( xmTxBufferMutex );

    return rxbd;
}
/*---------------------------------------------------------------------------*/

/* get a valid ptr for writing sTCB */
u8 *getRxBasePtr()
{
    Xil_DCacheInvalidateRange((u32)rxbd, RX_REG_OFFSET);
    return rxbd;
}
/*---------------------------------------------------------------------------*/

/* get RX valid base_ptr for Transmission */
u8 *getValidRxBasePtr()
{

    /* it will wait here if no valid ptr has been generated */
    while (xSemaphoreTake( xmTxBufferMutex,
                           MFEIT_MAX_TIME_TO_WAIT_FOR_TX_BUFFER_MS ) != pdPASS ) {
        ;
    }

    if (rxbd == rx_a_ptr)
        return (u8 *)rx_b_ptr;
    else
        return (u8 *)rx_a_ptr;

}
/*---------------------------------------------------------------------------*/

/*
 * get RX BD. bdNum should be [0, ..., max_switch - 1]
 */
u8 *getRxBD(unsigned int bdNum, unsigned int nBytes)
{

    u8 *this_rxbd = rxbd + RX_REG_OFFSET + bdNum * nBytes;

    /*
     * Invalidate the DestBuffer before receiving the data,
     * in case the Data Cache is enabled.
     */
    Xil_DCacheInvalidateRange((u32)this_rxbd, nBytes);

    return this_rxbd;
}
/*---------------------------------------------------------------------------*/

