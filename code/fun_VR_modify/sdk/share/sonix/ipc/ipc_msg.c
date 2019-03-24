#include <string.h>
#include "ipc_msg.h"

/**
 * M3_1 write queue
 * This is the queue which will be written by M3_1 and be read by M3_0
 * \note
 * This variable is put in 0x20000000 section which will
 * be linked to shared memory region by linker script.
 */
static struct ipc_queue queue_m3_1 __attribute((at(0x20000800)));

/**
 * M3_0 write queue
 * This is the queue which will be written by M3_0 and be read by M3_1
 * \note
 * This variable is put in 0x20000800 section which will
 * be linked to shared memory region by linker script.
 */
static struct ipc_queue queue_m3_0 __attribute((at(0x20000C00)));

/**
 * IRQ priority of IPC interrupt
 * Currently this has the highest priority
 */
#define IPC_IRQ_Priority    5
#define CORE_M3_0
//#define OS_FREE_RTOS

#ifdef CORE_M3_0
/**
 * M3_0 read queue pointer
 * M3_0 reads from this message queue based on the
 * tail index.
 */
static struct ipc_queue *qrd = &queue_m3_1;
/**
 * M3_0 write queue pointer
 * M3_0 writes from this message queue based on the
 * head index.
 */
static struct ipc_queue *qwr = &queue_m3_0;
#define IPC_IRQHandler Core1_Issue_IRQHandler
#define IPC_IRQn       DSP1_Issue_IRQn
#define IPC_IRQn_TRIGGER DSP0_Issue_IRQn
#elif defined(CORE_M3_1)
static struct ipc_queue *qrd = &queue_m3_0;
static struct ipc_queue *qwr = &queue_m3_1;
#define IPC_IRQHandler Core0_Issue_IRQHandler
#define IPC_IRQn       DSP0_Issue_IRQn
#define IPC_IRQn_TRIGGER DSP1_Issue_IRQn
#else
#error "For SNC7320, CORE_M3_1 or CORE_M3_0 must be defined!"
#endif

/* FreeRTOS functions */
#ifdef OS_FREE_RTOS
#include "FreeRTOS.h"
#include "semphr.h"

static xSemaphoreHandle event_tx, event_rx;
static void os_event_handler(void)
{
    portBASE_TYPE wake1 = pdFALSE, wake2 = pdFALSE;

    if (event_rx && !QUEUE_IS_EMPTY(qrd))
        xSemaphoreGiveFromISR(event_rx, &wake1);
    
    if (event_tx && !QUEUE_IS_FULL(qwr))
        xSemaphoreGiveFromISR(event_tx, &wake2);
    
    portEND_SWITCHING_ISR(wake1 || wake2);
}

static void ipc_misc_init(void)
{
    vSemaphoreCreateBinary (event_tx);
    vSemaphoreCreateBinary (event_rx);
    
    if (!event_tx || !event_rx)
        while(1); /* BUG: unable to create semaphores */
}

#define ipc_wait_event(evt, sem) while ((evt)) xSemaphoreTake(sem, 100)
#define ipc_wait_event_tout(evt,tout,sem) \
    do { \
        while((evt)) \
            if ((evt) && xSemaphoreTake(sem, tout) != pdTRUE) \
                {tout = 0; break;} \
    }while (0)

#else
/***************************************************************************
 *                           OS less adapter functions                     *
 ***************************************************************************/
#define CPU_FREQ     162000000UL
    
static void os_event_handler(void)
{
    /* Nothing to do here */
}

/* Wait for the event */
#define ipc_wait_event(evt,sem) while((evt))

#define ipc_wait_event_tout(evt,tout,sem) \
    do {\
        uint32_t cnt = (tout * (CPU_FREQ / 1000)); \
        while ((evt) && cnt > 0){ \
            cnt--; \
        } \
        if (evt) tout = 0; \
    }while(0)

static void ipc_misc_init(void)
{
}
/************** End of OS less adapter functions *********************/
#endif

/**
 * Initiate interrupt on other processor
 * Upon calling this function generates and interrupt on the other
 * core. Ex. if called from M3_1 core it generates interrupt on M3_0 core
 * and vice versa.
 */
static void ipc_send_signal(void)
{
    __DSB();
    __sev();
}

/**
 * Sends notification interrupt
 *
 * When called from M3_1 sends notification interrupt to M3_0 and vice-versa.
 */
void IPC_msgNotify(void)
{
    ipc_send_signal();
//	NVIC->STIR = DSP0_Issue_IRQn;
    NVIC_SetPendingIRQ(IPC_IRQn_TRIGGER);
}

/**
 * \brief Function to initialize the IPC message queue
 *
 * This function intializes the interprocessor communication message queue
 * \param data Pointer to the array of 32bit unsigned integers to be used as queue
 * \param size size of the \a data element
 * \param count Maximum number of items in Queue
 */
void IPC_initMsgQueue(void *data, int size, int count)
{
    /* Sanity Check */
    if (!size || !count || !data)
        while(1);

    /* Check if size is a power of 2 */
    if (count & (count - 1))
        while(1); /* BUG: Size must always be power of 2 */
    
    if(size*count > (1024 - 64))
        while(1); /* BUG: queue size > 1024 bytes*/
    
    ipc_misc_init();
    qwr->count = count;
    qwr->size = size;
    qwr->data = (uint8_t *)qwr + sizeof(struct ipc_queue);
    qwr->head = 0;
    qwr->tail = 0;
    NVIC_SetPriority(IPC_IRQn, IPC_IRQ_Priority);
    NVIC_EnableIRQ(IPC_IRQn);
}

/**
 * \brief Function to push message into IPC queue
 *
 * This function will push an 32-bit unsigned data as a message
 * to the other processor. i.e., if called from M3_1 it pushes the
 * message to M3_0 and vice-versa.
 * \param data 32-bit unsigned integer data to be pushed
 * \param tout Timeout in milliseconds
 * \return QUEUE_INSERT on success, QUEUE_ERROR or QUEUE_FULL on failure
 *         QUEUE_TIMEOUT when there is a timeout
 */
int IPC_pushMsgTout(const void *data, int tout)
{
    /* Check if write queue is initialized */
    if (!qwr->size)
        return QUEUE_ERROR;

    if (tout == 0) {
        /* Check if queue is full */
        if (QUEUE_IS_FULL(qwr))
            return QUEUE_FULL;
    } else if (tout < 0) {
        /* Wait for read queue to have some data */
        ipc_wait_event (QUEUE_IS_FULL(qwr), event_tx);
    } else {
        /* Wait for read queue to have some data */
        ipc_wait_event_tout (QUEUE_IS_FULL(qwr), tout, event_tx);
        if (tout == 0)
            return QUEUE_TIMEOUT;
    }

/*	qwr->data[qwr->head & (qwr->size - 1)] = data; */
    memcpy(qwr->data + ((qwr->head & (qwr->count - 1)) * qwr->size), data, qwr->size);
    qwr->head ++;
    ipc_send_signal();

    return QUEUE_INSERT;
}

int IPC_popMsgTout(void *data, int tout)
{
#ifdef EVENT_ON_RX
    int raise_event = QUEUE_IS_FULL(qrd);
#endif

    if (!qrd->size)
        return QUEUE_ERROR;

    if (tout == 0) {
        /* Check if read queue is empty */
        if (QUEUE_IS_EMPTY(qrd))
        return QUEUE_EMPTY;
    }	else if (tout < 0) {
        /* Wait for read queue to have some data */
        ipc_wait_event (QUEUE_IS_EMPTY(qrd), event_rx);
    } else {
        /* Wait for event or timeout */
        ipc_wait_event_tout(QUEUE_IS_EMPTY(qrd), tout, event_rx);
        if (tout == 0)
            return QUEUE_TIMEOUT;
    }

    /* Pop the queue Item */
/*	*data = qrd->data[qrd->tail & (qrd->size - 1)]; */
    memcpy(data, qrd->data + ((qrd->tail & (qrd->count - 1)) * qrd->size), qrd->size);
    qrd->tail ++;

#ifdef EVENT_ON_RX
    if (raise_event)
        ipc_send_signal();
#endif
    return QUEUE_VALID;
}

int IPC_msgPending(void)
{
    return QUEUE_DATA_COUNT(qrd);
}

/* IPC Event handler, can be overridden by user */
void ipc_event_handler(void) __attribute__((weak));
void ipc_event_handler(void)
{
}

/* Queue event handler */
void IPC_IRQHandler(void)
{
    /* Clear the interrupt */
    NVIC_ClearPendingIRQ(IPC_IRQn); 
    /* Invoke OS Specific handler */
    os_event_handler();

    /* Invoke handler */
    ipc_event_handler();
}

const char *IPC_strerror(int errnum)
{
    static const char *ipc_errstr[] = {
        "Queue Insert OK",
        "Queue Pop OK/Valid",
        "Queue is Full",
        "Queue is Empty",
        "Queue Error/Not initialized",
        "Queue operation timed out",
    };
    if (errnum < 0) errnum = 1 - errnum;
    return ipc_errstr[errnum];
}

static const uint8_t    core1_img[] __attribute__((at(CONFIG_CORE1_LD_ADDR))) =
{
    /* Based on the core_0.uvproj file. */
    #include "..\core_1\tool\core_1_img.dat"
};

#ifdef CORE_M3_0 /* Start M3_1 from M3_0 core */
int IPC_Manual_Boot_Core1(uint32_t m3_1_image_addr)
{
    #define ROM1_COMMAND_PROGRAM_JUMP   17  /* ROM defined command */
    #define DISABLE_CLK_M1()            { *(uint32_t*)0x45000100 &= ~(1<<15); }
    #define ENABLE_CLK_M1()                     \
    {                                           \
        *(uint32_t*)0x4500010C |= (1<<15);      \
        *(uint32_t*)0x45000100 |= (1<<15);      \
    }
    
    /* Make sure the alignment is OK */
    if (m3_1_image_addr & 0x7)
    {
        return -1;
    }
    
    DISABLE_CLK_M1();

    /* Prepair the execution region for core 1 */
    memcpy( (void*) CONFIG_CORE1_ER_ADDR,
            (void*) CONFIG_CORE1_LD_ADDR,
            CONFIG_CORE1_ER_SIZE);

    /* Initial command to the core 1 */
    *(uint32_t*)0x20000008 = ROM1_COMMAND_PROGRAM_JUMP;
    *(uint32_t*)0x2000000C = CONFIG_CORE1_ER_ADDR;
    *(uint32_t*)0x20000000 = 1;
    
    ENABLE_CLK_M1();
            
    return 0;
}

void IPC_Reset_Core1()
{
    DISABLE_CLK_M1();
    
    /* Clear Core1 IPC Buffer */
    memset(0x20000000 , 0, 0x10);
    
    ENABLE_CLK_M1();
}
#endif
