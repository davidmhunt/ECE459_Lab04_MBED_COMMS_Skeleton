//define which node this is within the MRF24J40.h file

#include <mbed.h>
#include "rtos.h"    
#include "SerialStream.h"        
#include "basic_rf.h"
#include <string.h>

//TODO: In the MRF24J40.h file, do the following:
// 1. #define NODE1,NODE2, or NODE3 (only one per MBED)
// 2. set the RADIO_CHANNEL to your lab group on canvas

BufferedSerial serial(USBTX, USBRX, 115200);
SerialStream<BufferedSerial> pc(serial);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);


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
// Counter for number of relayed packets for node 2
static uint16_t txRelayedPackets = 0;
// Counters for the number of packets to send
static int packetsToSend = 1000;
static int sentPackets = 0;

// Count number of detected packets and toggle a pin for each packet
void mrfIsrCallback()
{       
//    nrk_led_toggle(ORANGE_LED);
    mrfRxISRcallbackCounter++;
    rx_buf_empty = 0;
}

int main(void)
{   
    led4 = 0;
    rfRxInfo.pPayload = rx_buf;
    rfRxInfo.max_length = RF_MAX_PAYLOAD_SIZE;
    rf_init(&rfRxInfo, RADIO_CHANNEL, 0xFFFF, 0); //RX struct, channel, panId, myAddr
    //rf_addr_decode_set_my_mac(0x0003); // function to set myAddr if not performed during init

    rf_auto_ack_enable(); // enable if IEEE 802.15.4 ack packets are requested
    // rf_auto_ack_disable();
    
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
    
//    nrk_init();
    nrk_create_taskset();
    while(1);
    return 0;
}

void rx_task ()
{
    rf_addr_decode_set_my_mac(MAC_ADDR);
    
    while (!rx_buf_empty)
    {
        // Vuk: Rx content test
        if (strncmp(rx_buf, "This is a test, LPC1768-nanoRK-MRF24J40MA", 41) == 0)
        {
//                nrk_led_toggle(BLUE_LED);
            rxContentSuccess++;
            led4 = !led4;
#ifdef NODE2          
    //comment out for part 2      
            sprintf (tx_buf, "This is a test, LPC1768-nanoRK-MRF24J40MA");
            rfTxInfo.pPayload=tx_buf;
            rfTxInfo.length=strlen(tx_buf);
            rfTxInfo.destAddr = 0x000C; // destination node address
            rf_rx_off();
            bool success = rf_tx_packet(&rfTxInfo);
            if (success){
                txRelayedPackets ++;
            }
            rf_rx_on();
#endif
        }
        rx_buf_empty = 1;
        ThisThread::yield();   
    }
}

void monitor_task ()
{
        pc.printf("Decoded pkts: %d Perfect pkts: %d, Relayed pkts: %d\n",
         mrfRxISRcallbackCounter,
         rxContentSuccess,
         txRelayedPackets);
        ThisThread::yield();
}

void tx_task ()
{
    rf_addr_decode_set_my_mac(MAC_ADDR);
    sprintf (tx_buf, "This is a test, LPC1768-nanoRK-MRF24J40MA");
    rfTxInfo.pPayload=tx_buf;
    rfTxInfo.length=strlen(tx_buf);
    rfTxInfo.destAddr = 0x000B; // destination node address
     
    if(packetsToSend!=0)
    {
        rf_rx_off();
        bool success = rf_tx_packet(&rfTxInfo);
        if (! success) pc.printf("f: %d\n",sentPackets);
        rf_rx_on();
                
        packetsToSend--;
        sentPackets++;
        led4 = !led4;
        ThisThread::yield();
    }else{
        led1 = 0;
        ThisThread::yield();
    }
}

void nrk_create_taskset ()
{
    // Separately activate Tx task on one node, and Rx task on the other
    // or both if congestion operation is desirable

#ifdef NODE1
  // EventQueue for NODE1 (transmitter)
  node1_thread.start(callback(&node1_queue,&EventQueue::dispatch_forever));
  node1_queue.call_every(100ms,&tx_task);
#endif
#ifdef NODE2
  //nrk_activate_task (&RX_TASK);
  node2_thread.start(callback(&node2_queue,&EventQueue::dispatch_forever));
  node2_queue.call_every(50ms,&rx_task);
  monitor_thread.start(callback(&monitor_queue,&EventQueue::dispatch_forever));
  monitor_queue.call_every(500ms,&monitor_task);
#endif
#ifdef NODE3
//  nrk_activate_task (&RX_TASK);
  node3_thread.start(callback(&node3_queue,&EventQueue::dispatch_forever));
  node3_queue.call_every(50ms,&rx_task);
  monitor_thread.start(callback(&monitor_queue,&EventQueue::dispatch_forever));
  monitor_queue.call_every(500ms,&monitor_task);
#endif
}