# baby-cry-notify
Arduino project to visually indicate baby-crying over wireless

This project consists of 2 modules that are functionally tightly-couled:
1. Recorder-transmitter module, made from 
  * arduino
  * nrfl24l01 (used to transmit noise-level)
  * ADMP401 MEMS microphone breakout
  * ON/OFF switch
  * Battery-pack (4 x AA)
  
2. Receiver-visualizer module, made from
  * arduino
  * OLED display (to indicate noise-levels vs. threshold)
  * nrfl24l01 (used to receive noise-level)
  * Volume-threshold potentiometer
  * LEDs strip to visually-indicate over-thershold voice-level
  * ON/OFF switch
  * Battery-pack (4 x AA)
  * 5V audio-jack output (to vibration-module)
  * 3055VL mosfet (to drive LEDs and 5V output)
  
