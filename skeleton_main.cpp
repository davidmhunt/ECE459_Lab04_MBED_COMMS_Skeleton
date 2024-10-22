#include <mbed.h>
#include "rtos.h"    
#include "SerialStream.h"        
#include "basic_rf.h"

BufferedSerial serial(USBTX, USBRX, 115200);
SerialStream<BufferedSerial> pc(serial);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

Thread node1_thread;
Thread node2_thread;
Thread node3_thread;
Thread monitor_thread;
EventQueue node1_queue;
EventQueue node2_queue;
EventQueue node3_queue;
EventQueue monitor_queue;

void rx_task (void);
void tx_task (void);
void monitor_task (void);
void nrk_create_taskset ();

char tx_buf[RF_MAX_PAYLOAD_SIZE];
char rx_buf[RF_MAX_PAYLOAD_SIZE];
RF_RX_INFO rfRxInfo;
RF_TX_INFO rfTxInfo;
uint8_t rx_buf_empty = 1;
// Counter for number of detected packets that have valid data
static uint16_t rxContentSuccess = 0;
// Counter for number of detected packets from the preamble perspective, regardless of contents
static uint16_t mrfRxISRcallbackCounter = 0;


// Count number of detected packets and toggle a pin for each packet
void mrfIsrCallback()
{       
    // TODO: optionally, toggle a pin for each recieved packet
    mrfRxISRcallbackCounter++;
    rx_buf_empty = 0;
}

int main(void)
{   
    // To switch between the 3 versions of this code, refer to MRF24J40.h and define NODE1, NODE2, *or* NODE3 (not multiple)

#ifdef NODE1
    led1=1;
    pc.printf("I am NODE1\n");
#endif
#ifdef NODE2 
    led2=1;
    pc.printf("I am NODE2\n");
#endif
#ifdef NODE3 
    led3=1;
    pc.printf("I am NODE3\n");
#endif
        
    // ------------------------------ PLACE CODE HERE ----------------------------------
    // TODO:
    //  (1) set up the reciever buffer and metadata (i.e., max length) in rfRxInfo
    //  (2) initialize radio module
    //  (3) enable acknowledgement packets (if needed)

    
    
    nrk_create_taskset();
    while(1);
    return 0;
}

void rx_task ()
{
    // ------------------------------ PLACE CODE HERE ----------------------------------
    // TODO:
    //  (1) set MAC address 
    //  (2) wait until RX packet is received
    //  (3) upon packet received, increment rxContentSuccess. Optionally, blink an LED to confirm receipt.
                
#ifdef NODE2                
    // CODE FOR 3-NODE CONFIGURATION. If node2, receive THEN send packet. IGNORE FOR 2-NODE CONFIGURATION.
#endif
    {   
    }
}

void monitor_task ()
{
    while(1)
    {
        pc.printf("Decoded pkts: %d Perfect pkts: %d\n", mrfRxISRcallbackCounter, rxContentSuccess);
        ThisThread::yield();
    }
}

void tx_task ()
{
    // ------------------------------ PLACE CODE HERE ----------------------------------
    // TODO:
    //  (1) set MAC address 
    //  (2) load payload and metadata (i.e., size, destination) into rfTxInfo
    //  (3) initiate packet transmission. remember to surround the function with packet decoding off then on.
    //  (4) Optionally, blink an LED to confirm transmission.

}

void nrk_create_taskset ()
{
    // Separately activate Tx task on one node, and Rx task on the other
    // or both if congestion operation is desirable

#ifdef NODE1
  // EventQueue for NODE1 (transmitter)
  node1_thread.start(callback(&node1_queue,&EventQueue::dispatch_forever));
  node1_queue.call_every(100,&tx_task);
#endif
#ifdef NODE2
  // EventQueue for NODE2 (receiver (+ transmitter for 3-node configuration))
  node2_thread.start(callback(&node2_queue,&EventQueue::dispatch_forever));
  node2_queue.call_every(50,&rx_task);
  monitor_thread.start(callback(&monitor_queue,&EventQueue::dispatch_forever));
  monitor_queue.call_every(50,&monitor_task);
#endif
#ifdef NODE3
  // EventQueue for NODE3 (receiver)
  node3_thread.start(callback(&node3_queue,&EventQueue::dispatch_forever));
  node3_queue.call_every(50,&rx_task);
  monitor_thread.start(callback(&monitor_queue,&EventQueue::dispatch_forever));
  monitor_queue.call_every(50,&monitor_task);
#endif
}