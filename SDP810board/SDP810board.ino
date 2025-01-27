/*
*	Title: SDP810board.ino
*	Author: Alberto Rived
*	Date: 20-1-2025
*	Version: 1.3
*	Brief: ESP32 firmware for the SDP810 board
*/

#include "SensirionI2CSdp.h"
#include <ESP32Servo.h> 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "myScreens.h"

// GPIO
#define B_MOVE        35
#define B_OK          32
#define B_CANCEL      33
#define B_THRESHOLD   500
#define POT_PIN       34
#define SERVO_PIN     19
#define TCA_RST       18

// Serial FTDI
#define RXD2 16
#define TXD2 17

// OLED Display
#define SCREEN_WIDTH  128 
#define SCREEN_HEIGHT 64 
#define OLED_MOSI     14
#define OLED_CLK      12
#define OLED_DC       2
#define OLED_CS       15
#define OLED_RESET    4

// State machine
enum State {
  IDLE_A,
  IDLE_B,
  MEASURE_A,
  MEASURE_B,
  MODE_A,
  MODE_B,
  MODE_C,
  MEASURING_SOLO,
  MEASURING_MULTI,
  POT_SOLO,
  POT_MULTI_A,
  POT_MULTI_B
};
State currentState = IDLE_A;

// Data recorded
struct pressureData_t{
  int8_t angle;                  // Angle of the NACA
  float measures[8];            // Pressure on each sensor
};

// Buttons
bool b_ok = false;
bool b_move = false;
bool b_cancel = false;

unsigned long b_ok_time = 0;
unsigned long b_move_time = 0;
unsigned long b_cancel_time = 0;
unsigned long button_time_o = 0; 
unsigned long button_time_m = 0; 
unsigned long button_time_c = 0;  

uint8_t mode = 0;

// ADC things
int16_t ADC_Max = 3458;  //4096
int16_t ADC_min = 473;   //0  
int16_t val;
int8_t angle = 0;

// dynamic array
int8_t *array = nullptr;
uint8_t nSamples = 1;
uint8_t nSamplesNew = 1;

// Create objects
// OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
  OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
// Pressure
SensirionI2CSdp sdp;
// Servo
Servo myservo;                  

// Interrupts for buttons
void IRAM_ATTR isr_ok() {
  button_time_o = millis();
  if (button_time_o - b_ok_time > B_THRESHOLD && (digitalRead(B_OK) == LOW)){
    b_ok = true;
    b_ok_time = button_time_o;
  }
}

void IRAM_ATTR isr_move() {
  button_time_m = millis();
  if (button_time_m - b_move_time > B_THRESHOLD && (digitalRead(B_MOVE) == LOW)){
    b_move = true;
    b_move_time = button_time_m;
  }
}

void IRAM_ATTR isr_cancel() {
  button_time_c = millis();
  if (button_time_c - b_cancel_time > B_THRESHOLD && (digitalRead(B_CANCEL) == LOW)){
    b_cancel = true;
    b_cancel_time = button_time_c;
  }
}

/*****************************************************************
*                         SETUP FUNCTIONS
******************************************************************/

// Function to select I2C BUS
void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
}

// Function to set up the sensors
void setupSDP810(){
  
  uint16_t error;
  char errorMessage[256];

  uint32_t productNumber;
  uint8_t serialNumber[8];
  uint8_t serialNumberSize = 8;

  sdp.stopContinuousMeasurement();

  error = sdp.readProductIdentifier(productNumber, serialNumber,
                                    serialNumberSize);
  if (error) {
      Serial.print("Error trying to execute readProductIdentifier(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  } else {
      Serial.print("ProductNumber:");
      Serial.print(productNumber);
      Serial.print("\t");
      Serial.print("SerialNumber:");
      Serial.print("0x");
      for (size_t i = 0; i < serialNumberSize; i++) {
          Serial.print(serialNumber[i], HEX);
      }
      Serial.println();
  }

  error = sdp.startContinuousMeasurementWithDiffPressureTCompAndAveraging();

  if (error) {
      Serial.print(
          "Error trying to execute "
          "startContinuousMeasurementWithDiffPressureTCompAndAveraging(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
  }
}

// Function to set up the servo
void setupServo(){
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
  myservo.setPeriodHertz(50);           // 50hz servo
  myservo.attach(SERVO_PIN, 1000, 2000); // high time in us
  delay(200);
  myservo.write(90);
  delay(2000);
}

// Update array
void updateArray() {
  if (array != nullptr) delete[] array;
  array = new int8_t[nSamples];
}


/*****************************************************************
*                         SETUP
******************************************************************/
void setup() {
  // UART
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  // SPI
  if(!display.begin(SSD1306_SWITCHCAPVCC)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }
  display.display();

  // I2C
  Wire.begin();

  sdp.begin(Wire, SDP8XX_I2C_ADDRESS_0);
  // Initialize each sensor
  for(uint8_t i = 0; i < 8; i++){
    TCA9548A(i);
    setupSDP810();
  }

  // Servo
  setupServo();

  // Initialize array
  updateArray(); 

  // Pins and interrupts
	pinMode(B_MOVE, INPUT_PULLUP);
  pinMode(B_OK, INPUT_PULLUP);
  pinMode(B_CANCEL, INPUT_PULLUP);
	attachInterrupt(B_MOVE, isr_move, FALLING);
  attachInterrupt(B_OK, isr_ok, FALLING);
  attachInterrupt(B_CANCEL, isr_cancel, FALLING);
}


/*****************************************************************
*                   MAIN LOOP (STATE MACHINE)
******************************************************************/
void loop(){
  switch(currentState){
    case IDLE_A:
      showIDLE_A();
      if      (b_move)  currentState = IDLE_B;
      else if (b_ok)    currentState = MEASURE_A;       
      break;

    case IDLE_B:
      showIDLE_B();
      if      (b_move)  currentState = IDLE_A;
      else if (b_ok)    currentState = MODE_A;
      break;

    case MEASURE_A:
      showMEASURE_A();
      if      (b_move)    currentState = MEASURE_B;
      else if (b_ok)      currentState = POT_SOLO; 
      else if (b_cancel)  currentState = IDLE_A;
      break;
    
    case MEASURE_B:
      showMEASURE_B();
      if      (b_move)    currentState = MEASURE_A;
      else if (b_ok)      currentState = POT_MULTI_A; 
      else if (b_cancel)  currentState = IDLE_A;
      break;

    case MODE_A:
      showMODE_A();
      if      (b_move)    currentState = MODE_C;
      else if (b_cancel)  currentState = IDLE_B;
      else if (b_ok){
        showMODE_A_SELECT();
        delay(1500);
        mode = 1;
      } 
      break;
    
    case MODE_C:
      showMODE_C();
      if      (b_move)    currentState = MODE_A;
      else if (b_cancel)  currentState = IDLE_B;
      else if (b_ok){
        showMODE_C_SELECT();
        delay(1500);
        mode = 3;
      }     
      break;

    case MEASURING_SOLO:
      showMEASURING();
      readPressure();        ///////////////////////////////////////////////////////////////////////////////////
      currentState = MEASURE_A;
      
      //aqui medir y eso
      if(b_cancel){
        showCANCEL();
        delay(1500);
        currentState = MEASURE_A;
        break;
      }
      showDATA_ACQ();
      delay(1500);
      break;

    case MEASURING_MULTI:
      //aqui medir y eso
      showMEASURING();
      for(uint8_t i=0; i<nSamples; i++){
        angle = array[i];
		myservo.write(array[i]);
        delay(500);
        readPressure();
        
        if(b_cancel){
          showCANCEL();
          delay(1500);
          currentState = MEASURE_B;
          break;
        }
      }        
      
      currentState = MEASURE_B;

      showDATA_ACQ();
      delay(1500);
      break;

    case POT_SOLO:
      val = analogRead(POT_PIN);
      
      val = map(val, ADC_min, ADC_Max, 65, 113);
      //Serial.println(val);
      angle = map(val, 0, 180, 45, -45);
      myservo.write(val);
      delay(1000);
      showPOT(angle);

      if      (b_ok)      currentState = MEASURING_SOLO;
      else if (b_cancel)  currentState = MEASURE_A;
      break;

    case POT_MULTI_A:
      val = analogRead(POT_PIN);
      val = map(val, ADC_min, ADC_Max, 0, 180);
      nSamplesNew = map(val, 0, 180, 1, 21);
      showPOT_MULTI_A(nSamplesNew);

      if      (b_ok)      currentState = POT_MULTI_B;             
      else if (b_cancel)  currentState = MEASURE_B;
    break;

    case POT_MULTI_B:
      bool arrayValid = true;
      if (nSamplesNew != nSamples){
        nSamples = nSamplesNew;
        updateArray();
      } 
      uint8_t i = 0;
      while( i < nSamples){
        val = analogRead(POT_PIN);
        val = map(val, ADC_min, ADC_Max, 65, 113);
        angle = map(val, 0, 180, 45, -45);
        showPOT_MULTI_B(angle, i+1);
        
        if (b_ok){
          b_ok = false;
          delay(300);
          array[i] = val;
          i++;
          continue;
        }  
        else if (b_move){
          b_move = false;
          delay(300);
          if (i > 0) i--;
          continue;
        }           
        else if (b_cancel){
          delete[] array;
          showCANCEL();
          delay(1500);
          arrayValid = false;
          break;
        }  
      }
      if (arrayValid) currentState = MEASURING_MULTI;
      else currentState = MEASURE_B;
      break; 
  }

  b_ok = false;
  b_move = false;
  b_cancel = false;
  delay(250);
}


/*****************************************************************
*                         FUNCTIONS
******************************************************************/

// Read all pressures and send
uint8_t readPressure(){

  pressureData_t measures;
  measures.angle = angle;
  
  uint16_t error;
  char errorMessage[256];

  // Read Measurement
  float differentialPressure;
  float temperature;

  for(uint8_t j = 0; j < 8; j++){
    TCA9548A(j);
    error = sdp.readMeasurement(differentialPressure, temperature);

    if (error) {
      Serial.printf("Error trying to execute readMeasurement() on %d: ", j);
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
      differentialPressure = -999;
    } //else {
      if(j == 0) measures.measures[2] = differentialPressure; 
      if(j == 1) measures.measures[3] = differentialPressure;
      if(j == 2) measures.measures[6] = differentialPressure;
      if(j == 3) measures.measures[7] = differentialPressure;
      if(j == 4) measures.measures[4] = differentialPressure;
      if(j == 5) measures.measures[5] = differentialPressure;
      if(j == 6) measures.measures[0] = differentialPressure;
      if(j == 7) measures.measures[1] = differentialPressure;     
    //}
  }
  
  /*Serial.printf("Medidas a %f: \t", measures.angle);
  for(uint8_t k=0; k<8;k++){
    Serial.printf("%d: %f\t", k, measures.measures[k]);
  }
  Serial.println();*/

  Serial2.print(measures.angle);
  for (int i = 0; i < 8; i++) {
    Serial2.print(";");
    Serial2.print(measures.measures[i]);  // Enviar el valor del float
  }
  Serial2.println(); 

  return 0;
}

// Display functions
void showIDLE_A(){
  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_menu_idle_a , 128, 64, WHITE);
  display.display();
}

void showIDLE_B(){
  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_menu_idle_b , 128, 64, WHITE);
  display.display();
}

void showMEASURE_A(){
  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_measure_a , 128, 64, WHITE);
  display.display();
}

void showMEASURE_B(){
  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_measure_b , 128, 64, WHITE);
  display.display();
}

void showMODE_A(){
  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_menu_mode_a , 128, 64, WHITE);
  display.display();
}

void showMODE_C(){
  display.clearDisplay();
  display.drawBitmap(0, 0,  epd_bitmap_menu_mode_c , 128, 64, WHITE);
  display.display();
}

void showMEASURING(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,20);
  display.println(F("MEASURING"));
  display.setTextSize(1);
  display.setCursor(0,57);
  display.println(F("CANCEL = STOP MEASURE"));
  display.display();
}

void showDATA_ACQ(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,15);
  display.println(F("DATA"));
  display.println(F("ACQUIRED"));
  display.display();
}

void showCANCEL(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,15);
  display.println(F("MEASURING"));
  display.println(F("CANCELLED"));
  display.display();
}

void showPOT(int8_t angle){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10,2);
  display.println(F("ANGLE"));
  display.setTextSize(3);
  display.setCursor(50,30);
  display.println(angle);
  display.display();
}

void showPOT_MULTI_A(uint8_t number){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,2);
  display.println(F("Samples:"));
  display.setTextSize(3);
  display.setCursor(50,30);
  display.println(number);
  display.display();
}

void showPOT_MULTI_B(int8_t angle, int8_t number){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1,2);
  display.print(F("ANGLE: "));
  display.println(number);
  display.setTextSize(3);
  display.setCursor(50,30);
  display.println(angle);
  display.display();
}

void showMODE_A_SELECT(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,15);
  display.println(F("MODE:"));
  display.println(F("STD ALONE"));
  display.display();
}

void showMODE_C_SELECT(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,15);
  display.println(F("MODE:"));
  display.println(F("USB"));
  display.display();
}
