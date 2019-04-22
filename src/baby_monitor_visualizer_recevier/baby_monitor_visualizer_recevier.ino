 /*********************************************************************************************
 * About visualizer-receiver:
 * The main purpose is to display alarm whenever threshold was crossed. 
 * The seconday purpose is to visualize on screen the last SCREEN_WIDTH samples received
 * 
 * Data received is the average of some collected samples on listener, with 
 * at least one sample over the 'quiet thershold' (the minimal amplitude that is 
 * considered 'not-quiet')
 * 
 * Visualizer shows all received samples on screen, and user is able to determine from what
 * noise level and up to display the visual-alarm
 *********************************************************************************************/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// radio data
RF24 radio(9,10); // CE, CSN
const byte address[6] = "00023";

// OLED data
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);
const int SCREEN_WIDTH = 128;
const int SCREEN_HEIGHT = 32;

// monitor related data
float wave_samples[SCREEN_WIDTH];
unsigned long last_data_time = millis();
int noise_threshold = 20;

const int ALARM_LEDS = 5;
unsigned long show_alarm_until = 0;

int last_potentiometer_value = 0;
unsigned long next_potentiometer_read = 0;


const bool OUTPUT_TO_SERIAL = true;

void setup() {
  pinMode(ALARM_LEDS, OUTPUT);
  digitalWrite(ALARM_LEDS, LOW);
  
  if (OUTPUT_TO_SERIAL) { 
    Serial.begin(9600); 
    for (int i=0; i<100; i++) { Serial.println(10); }
  }
  setup_radio();
  setup_oled();
}

void loop() {
  Serial.println(get_radio_sample());
  delay(10);
}

void no_loop() {
  digitalWrite(ALARM_LEDS, (show_alarm_until > millis()) ? HIGH : LOW);

  int encoder_value = readPotentiometer();
  if (noise_threshold != encoder_value) {
    noise_threshold = encoder_value;
    display_samples();
  }
  
  float sample_from_radio = get_radio_sample();

  if (sample_from_radio > noise_threshold) {
    show_alarm_until = millis() + 1000;
  }
  
  if (sample_from_radio >= 0) {
    add_sample(sample_from_radio);
    display_samples();
    if (OUTPUT_TO_SERIAL) { Serial.println(sample_from_radio); }
  }
  else if (millis() - last_data_time > 60 * 1000) {
    // if last transmission was more than a minute ago -
    // it means we have a communication problem
    if (OUTPUT_TO_SERIAL) { Serial.println("No comm in the last 60 seconds"); }
    delay(100);
  }
}

float get_radio_sample() {
  float sample = -1; // fallback for no data available
  if (radio.available()) {    
    radio.read(&sample, sizeof(sample));
  }
  return sample;
}

void add_sample(float samp) {
  for (int i=127; i>0; i--) {
    wave_samples[i] = wave_samples[i-1];
  }
  wave_samples[0] = samp;
}

void display_samples() {
  display.clearDisplay();
  
  // minimum is always 0, find maximum and scale is relative to SCREEN_HEIGHT
  float samp_max = 0;
  for (int i=0; i<SCREEN_WIDTH; i++) {
    samp_max = max(wave_samples[i], samp_max);
  }

  float scale = (float)SCREEN_HEIGHT / samp_max;
  
  for (int i=0; i<SCREEN_WIDTH; i++) {
    int val = (int)(wave_samples[i]*scale);
    display.drawFastVLine(i, 0, val, WHITE);
  }

  //noise_threshold
  display.drawFastHLine(0, noise_threshold*scale, SCREEN_WIDTH, WHITE);
  
  display.display();
}

void setup_oled() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
}

void setup_radio() {
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();
}

int readPotentiometer() {
  if (millis() < next_potentiometer_read) {
    return last_potentiometer_value;
  }
  
  int val = 999;
  for (int i=0; i<97; i++) {
    val = min(val, analogRead(A2));
  }

  val = 10.0 * (7.0 - log(val + 1));

  next_potentiometer_read = millis() + 300;
  last_potentiometer_value = val;
  return val;
}

