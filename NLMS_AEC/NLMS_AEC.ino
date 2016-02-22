
/* NLMS Acousitc Echo Cancelation
 * Copyright (c) 2016, Sergey Makitrin, Kyler Fillerup
 * 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
// Microphone to Speaker output
#define NUMEL(x) (sizeof(x)/sizeof((x)[0])) // retuns/calculates the size of an array

#include "NLMS.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

//*****functions**********
void getBuffer1();
void playData();
void displayData();
void NLMS_AEC(int16_t *x)

// GUItool: begin automatically generated code
AudioInputI2S            Mic;           //microphone
AudioPlayQueue           MicMem;         //queue for microphone
AudioRecordQueue         Memory;
AudioOutputI2S           headphones;           //output
AudioConnection          patchCord1(Mic, 0, Memory, 0);
AudioConnection          patchCord2(MicMem, 0, headphones,0);
AudioConnection          patchCord3(MicMem, 0, headphones,1);
AudioControlSGTL5000     sgtl5000_1;     //xy=339.6666564941406,285.6666488647461

const int numBlocks = 2; //min block size is 2!!!!!!
const int lFilt = numBlocks-1;
const int gg = lFilt*128;
int16_t *pAddr; //pointer to 128 block of getBuffer
int16_t lblock = numBlocks*128; //length of arrays
int16_t ablock[128*numBlocks] __attribute__ ((aligned (4)));  // set an array block
int16_t error[128] __attribute__ ((aligned (4)));
int16_t *Perror;
int16_t *pablock; // pointer to an array block
int16_t *pp; // pointer to getBuffer
int n;
unsigned long time1; // time variable


//***************NMLS const and settings*************************
const int16_t numTaps = gg;
int16_t w[numTaps] __attribute__ ((aligned (4)));


//define input/source
const int micInput = AUDIO_INPUT_MIC;

void setup() {
  memset(w,0,numTaps);
  memset(ablock,100,128*numBlocks*2);
  memset(error,0,128);
  //initialize pointers for NLMS
  Perror = error;
  Serial.begin(9600); // initiate baud rate for serial comm.
  AudioMemory(8); // allocate # of audio memory (each block has 128 sample of data)
  sgtl5000_1.enable(); // enalbe 
  sgtl5000_1.volume(0.5); // output volume
  sgtl5000_1.inputSelect(micInput); //input selection (mic or line in)
  sgtl5000_1.micGain(26); // mic gain
  //set array values to zero
  pablock = ablock; // assign pointer to array
  delay(1000);
}


//***************************MAIN LOOP********************************************************************
void loop() {
//Serial.println(millis()); // print time 
getBuffer1(numBlocks); //returns pointer to the buffered data
//print sample to serial plotter 
//displayData(pablock); // print data to serial plotter
//for(int i = 0; i<128;i++){
//  Serial.println(w[i]);
//}
}
//*********************************************************************************************************
// print data to serial plotter
void displayData(int16_t * data){
  for(int i = 0; i<128*numBlocks;i++){
  Serial.println(*(data+i));
}
}
// play data to speaker
void playData(int16_t *data, int i){
    pAddr = MicMem.getBuffer(); //get pointer to 128 array playBuffer 
    // take one block from buffer array (ablock) and copy it to playBuffer pointer
    // play that block
    memcpy((byte*)pAddr,(byte*)data+(256*i),256);
    MicMem.playBuffer();
}

// buffer an array with samples from Microphone
// function receives and array and integer indicating number of blocks
void getBuffer1(int x) {
  Memory.begin(); // begin putting sample data into memory
  int l = 0;
  memcpy((byte*)pablock,(byte*)pablock+256,256*(x-1));
  // put 128 blocks of sample into an array size of 128*(number of blocks)
    while(l == 0){ // wait until block samples are available
      if(Memory.available() ==1) {// if availabe copy 128 block to array and get out of while loop      
        //read one 128 block of samples and copy it into array
        memcpy((byte*)pablock+(256*(x-1)),Memory.readBuffer(),256);
        Memory.freeBuffer(); // free buffer memory
//        NLMS_AEC(pablock);
//        playData(Perror,0); // play 128 block from buffered array
        l = 1; // set n to 1 to get out of while loop
      }//end if 
    }//end while 
    NLMS_AEC(pablock);
    playData(Perror,0); // play 128 block from buffered array
    l = 0;
   Memory.clear(); // clear all audio memory 
   Memory.end();
//   return ablock; // regurn pointer to array
}

// NMLS algorithm
// inputs are Mic Signal, far end signal and index of n-1 block wher n is the current block itteration
// previous block data is used to modify current block data samples
void NLMS_AEC(int16_t *x)
{
  int16_t yhat = 0;
  int64_t xtdl = 0;
  int16_t mu0;
  int16_t mu = 13;
  int8_t psi = 1;

  for(int h = 0; h < 128; h+=1) {
    for(int j = gg; j > 0; j-=1) {
      yhat += (x[j+h]*w[gg-j])/32768;
      xtdl += x[j+h]*x[j+h];
    }
    error[h] = x[gg+h]-yhat;
    xtdl = xtdl + psi;
    mu0 = (67108864*mu)/xtdl;
 
    //update filter taps
    for(int j = 0; j<gg;j+=1) {
      w[j] = w[j] + (x[gg-j+h]*mu0*error[h])/131072;
    }//end for
  }//end for outer
}// end NLMS_AEC
