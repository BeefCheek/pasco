#include <Arduino.h>
#include <Wire.h>
#include <AT42QT2120.h>
#include "USB.h"
#include <SimpleKalmanFilter.h>
#include <driver/i2s.h>
#include <M16.h>
#include <Osc.h>

#include "AudioFileSourcePROGMEM.h"
#include "AudioGeneratorMOD.h"
#include "AudioOutputI2S.h"
#include "enigma.h"

AudioGeneratorMOD *mod;
AudioFileSourcePROGMEM *file;
AudioOutputI2S *out;
// #define MUTE 
#define LED 48

// ATQ touch sensor
#define ATQ_CHANGE 12
#define ATQ_RST 13
#define ATQ_SDA 15
#define ATQ_SCL 14

volatile bool touchedDetected = false;

const int RESET_DELAY = 2000;
const int CALIBRATION_LOOP_DELAY = 50;

AT42QT2120 touch_sensor(Wire,ATQ_CHANGE);
SimpleKalmanFilter* kalman = (SimpleKalmanFilter*)malloc(sizeof(SimpleKalmanFilter) * 12);

struct oversample{
  uint8_t pulse;
  uint8_t scale;
};
struct oversample oversamples[12];

uint8_t overSampleConfIndex = 2;

int16_t values[12] = {0};
int16_t baseValues[12] = {0};
int16_t averagedValues[12] = {0};
int16_t maxValues[12] = {0};

uint32_t lastCalibrationTimes[12] = {0};
uint32_t autoCalibrationInterval = 10000;
int16_t calibrationValues[12] = {0};
bool canCalibrate[12] = {false};
// pdm mic

#define I2S_SAMPLE_RATE   (16000) // Sample rate
#define I2S_BUFFER_SIZE   (512) // Buffer size


// M16
int16_t waveTable[TABLE_SIZE]; // empty wavetable
Osc aOsc1(waveTable);
int16_t vol = 1000; // 0 - 1024, 10 bit
unsigned long msNow = millis();
unsigned long pitchTime = msNow;

void scanI2c() {

  for (uint8_t addr = 0; addr<=127; addr++) {
    Serial.print("scanning "); Serial.println(addr);
    Wire.beginTransmission(addr);
    if (!Wire.endTransmission()) {
      Serial.print("Found I2C 0x");  Serial.println(addr,HEX);
    }
  }
  
  Serial.println("\ndone");
}

void sensorTouchedCB() {
  touchedDetected = true;
  //Serial.println("touched");
}

void setupM16() {
  Osc::sinGen(waveTable); // fill the wavetable
  aOsc1.setPitch(69);
  seti2sPins(16, 17, 18, -1); // bck, ws, data_out, data_in // change ESP32 defaults
  audioStart();
}
void setupSensor() {
  
  oversamples[0].pulse = 0x0;
  oversamples[0].scale = 0x00;

  oversamples[1].pulse = 0x2;
  oversamples[1].scale = 0x01;

  oversamples[2].pulse = 0x4;
  oversamples[2].scale = 0x02;

  oversamples[3].pulse = 0x6;
  oversamples[3].scale = 0x03;

  oversamples[4].pulse = 0x8;
  oversamples[4].scale = 0x04;
  
  oversamples[5].pulse = 0x0A;
  oversamples[5].scale = 0x05;

  oversamples[6].pulse = 0x0C;
  oversamples[6].scale = 0x06;

  oversamples[7].pulse = 0x0E;
  oversamples[7].scale = 0x07;

  touch_sensor.reset();
  delay(RESET_DELAY);
  
  AT42QT2120::KeyPulseScale key_pulse_scale;

  key_pulse_scale.pulse = oversamples[overSampleConfIndex].pulse;
  key_pulse_scale.scale = oversamples[overSampleConfIndex].scale;

  for (int i = 0; i<12; i++) {
    touch_sensor.setKeyPulseScale(i, key_pulse_scale);
  }
   touch_sensor.setChargeDuration(1);
  touch_sensor.triggerCalibration();
  delay(CALIBRATION_LOOP_DELAY);

  while (touch_sensor.calibrating()){
    Serial.println("calibrating...");
    delay(CALIBRATION_LOOP_DELAY);
  }

  touch_sensor.attachChangeCallback(sensorTouchedCB);
}

void setupMic() {
  // Configure I2S
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // high interrupt priority
        .dma_buf_count = 2,
        .dma_buf_len = I2S_BUFFER_SIZE,
        .use_apll = false,
    };

    // Install I2S driver
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

    // Set up the I2S pin configuration
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_PIN_NO_CHANGE, // Bit Clock Pin
        .ws_io_num = 10,  // Word Select Pin
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = 11, // Data Input Pin
    };

    i2s_set_pin(I2S_NUM_0, &pin_config);
}

// M16
/* The audioUpdate function is required in all M16 programs 
* to specify the audio sample values to be played.
* Always finish with i2s_write_samples()
*/
void audioUpdate() {
  int16_t leftVal = (aOsc1.next() * vol)>>10;
  int16_t rightVal = leftVal;
  i2s_write_samples(leftVal, rightVal);
}

void setup() {
  pinMode(LED, OUTPUT);

  pinMode(ATQ_RST, OUTPUT);
  digitalWrite(ATQ_RST, HIGH);

  Serial.begin(115200);
/*

audioLogger = &Serial;
  file = new AudioFileSourcePROGMEM( enigma_mod, sizeof(enigma_mod) );

   out = new AudioOutputI2S(0,0, 8, 0 );
  out->SetPinout(16, 17, 18);
  
  mod = new AudioGeneratorMOD();
  mod->SetBufferSize(3*1024);
  mod->SetSampleRate(44100);
  mod->SetStereoSeparation(32);
  mod->begin(file, out);


*/
  


  //setupM16();
   
  bool b = Wire.begin(ATQ_SDA, ATQ_SCL, 400000UL);

  delay(5000);
  if (!b) {
    Serial.println("Error initializing Wire");
  } else {
    Serial.println("Wire initialized");
  } 
  Wire.setTimeout(10);
 // scanI2c();
  
 
   setupSensor();

  for (int i = 0; i<12; i++) {
    kalman[i] = SimpleKalmanFilter(20.0, 20.0, 0.1);
  }
  for (int x = 0; x<100; x++) {
    for (int i = 0; i<12; i++) {
      values[i] = touch_sensor.getKeySignal(i);
      calibrationValues[i] = kalman[i].updateEstimate(values[i]);
    }
    delay(10);
  }
  
 
  
  // setupMic();

}

void loop() {
  uint32_t now = millis();
  //digitalWrite(LED, !digitalRead(LED));
  digitalWrite(LED, millis()%1000 < 500);

  // if (mod->isRunning()) {
  //   if (!mod->loop()) mod->stop();
  // } else {
  //   Serial.printf("MOD done\n");
  //   delay(1000);
  // }

  msNow = millis();

  // if (msNow - pitchTime > 10 || msNow - pitchTime < 0) {
  //   pitchTime = msNow;
  //   int pitch = random(24) + 58;
  //  // Serial.println(pitch);
  //   aOsc1.setPitch(pitch);
  // }



  for (int i = 0; i<12; i++) {
    values[i] = touch_sensor.getKeySignal(i);
    averagedValues[i] =  kalman[i].updateEstimate(values[i]);
    int16_t value = averagedValues[i] - calibrationValues[i];
    if (value <= 0) { // touch down
      calibrationValues[i] = averagedValues[i];
    }
    // if (i==0) {
    //    out->SetGain(value/300);
    // }
    Serial.print(value);
    Serial.print('\t');
  }

  for (int i = 0; i<12; i++) {
    if (now - lastCalibrationTimes[i] >= autoCalibrationInterval) {
      lastCalibrationTimes[i] = now;
      calibrationValues[i] = averagedValues[i];
    }
  }

 
  Serial.println();
  delay(10);
  

// Microphone reading
/* 

int16_t buffer[I2S_BUFFER_SIZE];
size_t bytes_read;

// Read data from the microphone
i2s_read(I2S_NUM_0, &buffer, sizeof(buffer), &bytes_read, portMAX_DELAY);

// Process the audio data
for (int i = 0; i < bytes_read / sizeof(int16_t); i++) {
    Serial.println(buffer[i]);
}
*/


 
}
