/*****************************************************
 * This UartEvent example shows the "Read Bytes Until"
 * event where it will fire the RX event handler when
 * a termination character is detected or the buffer
 * is full.
 * Open a serial monitor and type into it and end your
 * command with a line feed.
 *
 * By using the loopback feature we can test
 * the sending and receiving without having to connect
 * up anything. If you want to disable this feature
 * comment it out.
 *****************************************************/
#include <UartEvent.h>

Uart1Event Event1;
volatile bool print_flag = false;// flag to indicate rx buffer full

const uint16_t BUFSIZE = Event1.rxBufferSize;// size of internal buffer
char buffer[BUFSIZE + 1]; // user variable to hold incoming data

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(0);
    while (!Serial);
    delay(100);
    //--------------------------Uart1Event Configuration--------------------------------
    Event1.loopBack = true;              // internal loopback set / "default = false"
    Event1.txEventHandler = tx1Event;    // event handler Serial1 TX
    Event1.rxEventHandler = rx1Event;    // event handler Serial1 RX
    Event1.rxTermCharacterTrigger = '\n';// this termination character will fire the RX event handler
    Event1.begin(9600);                  // start serial port
    //----------------------------------------------------------------------------------
    Serial.println("RX event will fire when termination character is detected");
}

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
        Event1.print(c);
    }
    
    if (print_flag) {
        Serial.print(buffer);
        print_flag = false;
    }
}

//--------------------------------------Serial1 Events--------------------------------
void tx1Event(void) {
    // TX Event function will fire when it is finished sending a packet
}

void rx1Event(void) {
    // RX Event function prints the buffer when it is full
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    int x = 0;
    while (Event1.available()) buffer[x++] = Event1.read();
    buffer[x] = 0;
    print_flag = true;
}