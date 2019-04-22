/************************************************************************************************* 
 *  About monitor:
 *  The main purpose is to leave the decision about indication to the receiver component
 *  The secondary purpose is to refrain from unneccessary transmissions
 *  Hence, a reasonable threshold is set for transmission, and above it everything is 
 *  sen as-is
 *  
 *  Audio collection method:
 *  (Max-Min) over [sample_time] are collected as [amp], and aggregated over [collect_time]
 *  to an array [amp_samp]
 *  
 *  If any of the [amp] values exceeds the [min_amp_threshold], a transmission is invoked
 *  with the sum of the [amp_samp] values
 *  
 *  Time of last transmission is saved to [last_transmission_time] and if more than 10 sec
 *  passed from last transmission, a 'zero-amp' transmission is invoked as a 'keep-alive'
 *  signal
 ************************************************************************************************/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// pin data
const int MY_LED = 3; // debug only
const int MIC_PIN = A1;

// mic data
const int sampleTime = 50;
int micOut;
int min_amp_threshold = 10;
unsigned long last_transmission_time = 0;
unsigned long keepalive_interval = 2000;

// radio data
RF24 radio(9,10); // CE, CSN
const byte address[6] = "00023";

const bool OUTPUT_TO_SERIAL = false;

void setup() {
  if (OUTPUT_TO_SERIAL) { Serial.begin(9600); }
  setup_mic();
  setup_radio();
}

void loop() {
  float sample = (findPTPAmp()+findPTPAmp()+findPTPAmp()+findPTPAmp()) / 4;
  transmit(sample);
}

void transmit(float mic_data) {
  if (OUTPUT_TO_SERIAL) { Serial.println(mic_data); }
  radio.write(&mic_data, sizeof(mic_data));
  last_transmission_time = millis();
}

void setup_mic() {
  pinMode(MIC_PIN, INPUT);
  last_transmission_time = millis();
}

void setup_radio() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

float findPTPAmp(){
   unsigned long startTime = millis();  // Start of sample window
   unsigned int PTPAmp = 0; 
   unsigned int maxAmp = 0;
   unsigned int minAmp = 1023;

   while(millis() - startTime < sampleTime) 
   {
      micOut = analogRead(MIC_PIN);
      if( micOut < 1023)
      {
        if (micOut > maxAmp) { maxAmp = micOut; }
        else if (micOut < minAmp) { minAmp = micOut; }
      }
   }
  PTPAmp = maxAmp - minAmp;
  return PTPAmp / 10.0;
}
