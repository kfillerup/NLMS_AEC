/*****************************************************
 * This UartEvent example shows the "RX Buffer Size"
 * event where it will fire the RX event handler when
 * the buffer is full. You can have any size but it
 * cannot be greater than the internal buffer size - 1.
 *
 * By using the loopback feature we can test the
 * sending and receiving without having to connect
 * up anything. If you want to disable this feature
 * comment it out.
 *****************************************************/

// Microphone to Speaker output
#define NUMEL(x) (sizeof(x)/sizeof((x)[0])) // retuns/calculates the size of an array

#include "NLMS2.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <UartEvent.h>

//*****functions**********
void getBuffer1();
void playData();
void displayData();
void NLMS_AEC1();
int64_t dotp();

// GUItool: begin automatically generated code
AudioInputI2S            Mic;           //microphone
AudioPlayQueue           MicMem;         //queue for microphone
AudioRecordQueue         Memory;
AudioOutputI2S           headphones;           //output
AudioConnection          patchCord1(Mic, 0, Memory, 0);
AudioConnection          patchCord2(MicMem, 0, headphones,0);
AudioConnection          patchCord3(MicMem, 0, headphones,1);
AudioControlSGTL5000     sgtl5000_1;     //xy=339.6666564941406,285.6666488647461

const int numBlocks = 8; //min block size is 2!!!!!!
const int lFilt = numBlocks-1;
const int gg = lFilt*128;
int16_t *pAddr; //pointer to 128 block of getBuffer
int16_t lblock = numBlocks*128; //length of arrays
int16_t ablock[128*numBlocks];  // set an array block
int16_t error[128]; 
int16_t *Perror;
int16_t *pablock; // pointer to an array block
int16_t *pp; // pointer to getBuffer
int n;
unsigned long time1; // time variable

//************** UART SETTINGS *********************************
Uart1Event Event1;
volatile bool print_flag = false;// flag to indicate rx buffer size has been met

const uint16_t BUFSIZE = Event1.rxBufferSize;// size of internal buffer
uint8_t buffer[BUFSIZE]; // user variable to hold incoming data
uint8_t TRANSMIT[BUFSIZE-1];

//***************NMLS const and settings*************************
const int16_t numTaps = gg;
int64_t mu = 1;
int32_t mu0 = 0;
int8_t psi = 1;
int32_t w[numTaps];
int32_t *pw; // pointer to w 
int16_t yhat = 0;
//int64_t xtdl = 1;
int16_t y12 = gg/2;
int i = 0;

//define input/source
const int micInput = AUDIO_INPUT_MIC;


void setup() {

    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(0);
    while (!Serial);
    delay(100);
    
  //--------------------------Uart1Event Configuration--------------------------------
  //Event1.loopBack = true;                   // internal loopback set / "default = false"
  //Event1.txEventHandler = tx1Event;         // event handler Serial1 TX
    Event1.rxEventHandler = rx1Event;         // event handler Serial1 RX
    Event1.rxBufferSizeTrigger = BUFSIZE-1; // set trigger for (buffer size) - 1
    Event1.begin(9600);                       // start serial port
  //----------------------------------------------------------------------------------
    
    Serial.print("RX Trigger will fire when this many bytes are avaliable -> ");
    Serial.println(BUFSIZE - 1); // print the size of the internal RX buffer

    delay(1000);

  mu = 34359738368*mu;  // scale mu 
  memset(w,0,numTaps);
  memset(ablock,100,128*numBlocks*2);
  memset(error,0,128*2);
  //initialize pointers for NLMS
  pw = w;
  Perror = error;
  AudioMemory(8); // allocate # of audio memory (each block has 128 sample of data)
  sgtl5000_1.enable(); // enalbe 
  sgtl5000_1.volume(0.5); // output volume
  sgtl5000_1.inputSelect(micInput); //input selection (mic or line in)
  sgtl5000_1.micGain(30); // mic gain
  //set array values to zero
  pablock = ablock; // assign pointer to array
  delay(1000);


}

//***************************MAIN LOOP********************************************************************
void loop() {
//Serial.println(micros()); // print time 
getBuffer1(numBlocks); //returns pointer to the buffered dat

Event1.write((byte*)error, 256);
Event1.flush();

//print sample to serial plotter 
//displayData(pablock); // print data to serial plotter
//NLMS_AEC(pablock,pablock);
//NLMS_AEC(pablock,pablock,Perror,pw,gg,mu,4,pxtdl);
//for(int i = gg; i<gg+128;i+=1){
//  Serial.println(ablock[i]);
//}
}

//--------------------------------------Serial1 Events--------------------------------
//void tx1Event(void) {
//    // TX Event function will fire when it is finished sending a packet
//}

void rx1Event(void) {
    Serial.println("RX Interupt");
    // RX Event function prints the buffer when it is full
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    while (Event1.available())
    Event1.readBytes(buffer,256);
    Event1.flush();
    print_flag = true;
}

//*********************************************************************************************************
// print data to serial plotter
void displayData(int16_t * data){
  for(int i = 0; i<128*numBlocks;i++){
  Serial.println(*(data+i));
  }
}
// play data to speaker
void playData(void){
    pAddr = MicMem.getBuffer(); //get pointer to 128 array playBuffer 
    // take one block from buffer array (ablock) and copy it to playBuffer pointer
    // play that block
    memcpy((byte*)pAddr,(byte*)error,256);
    MicMem.playBuffer();
}

// buffer an array with samples from Microphone
// function receives and array and integer indicating number of blocks
void getBuffer1(int x){
  Memory.begin(); // begin putting sample data into memory
  int l = 0;
  memcpy((byte*)pablock,(byte*)pablock+256,256*(x-1));
  // put 128 blocks of sample into an array size of 128*(number of blocks)
    while(l == 0){ // wait until block samples are available
      if(Memory.available() ==1) {// if availabe copy 128 block to array and get out of while loop      
        //read one 128 block of samples and copy it into array
        memcpy((byte*)pablock+(256*(x-1)),Memory.readBuffer(),256);
        Memory.freeBuffer(); // free buffer memory
        l = 1; // set n to 1 to get out of while loop
      }//end if 
    }//end while 
    l = 0;
   Memory.clear(); // clear all audio memory 
   Memory.end();
   NLMS_AEC(pablock,(int16_t*)buffer,Perror,pw,gg,mu,5,1);
   //NLMS_AEC(pablock,pablock,Perror,pw,gg,mu,5,1);
   playData(); // play 128 block from buffered array
//   return ablock; // regurn pointer to array
}

