/*
 *  Rainguage-v2 project
 *  
 *  Description:
 *     
 *     This program uses interrupt to detect when a raingauge tipping bucket is tipping.
 *
 *     Detection is done by the help of an Low Power Consumption Hall Effect Sensor : DRV5032FADBZR
 *
 *     Then it wakes up the XBee and send a message
 *
 *     The message is received on another XBee plugged in a Raspberry.
 *
 *     A Nodered flow then received the message and handles so it is consumed by HomeAssistant
 *     
 *     This is an ultra low power battery operated design. It consumes only XXuA when sleeping.
 *     
 *     More info here : https://github.com/fguiet/raingauge-v2
 *     
 *  Board used : Using board 'ATtiny85' 
 *               Using Arduino As ISP to program the ATtiny85
 *     
 *  Date : 2023/12/23
 *  
 *  Version : 1.0
 *  
 */

#include <Arduino.h>  // required for main()

// Note :
//   - See https://github.com/ArminJo/ATtinySerialOut/blob/master/README.md#compile-options--macros-for-this-library
//   - By default the ATtinySerialOut is using the PIN_PB2 (physical Pin 7)
//   - By default The rates are 38400 baud at 1 MHz

//Change default (PINB2) TX Pin to PINB1
#define TX_PIN PINB1

//Uncomment this line below if you want to use 38400 bauds instead of 115200 bauds
#define TINY_SERIAL_DO_NOT_USE_115200BAUD

#include "ATtinySerialOut.hpp"  // Available as Arduino library "ATtinySerialOut"

//From https://github.com/ortegafernando/Low-Power/tree/master
#include "LowPower.h"

unsigned int eightSecondsPassedCounter = 0;

//450 * 8 = 3600 secons
const int ONE_HOUR = 450;
const String SENSOR_ID = "1";
const String FIRMWARE_VERSION = "1.0.0";
const byte ADC_PIN = PINB3;                             //Physical PIN 2
const byte WAKE_XBEE_PIN = PINB0;                       //ie PIN_PB0 - Physical PIN 6
const byte INTERRUPT_NUMBER = 0;                        // This is the interrupt number linked to PB2 (See schematic) - INT0
const byte PHYSICAL_PIN_LINK_TO_INTERRUPT_PIN = PINB2;  //Physical PIN 6 - PIN_PB1 link to PCINT1 (see schematic)
const byte BATTERY_READING = 5;
volatile bool isTippingOccured = false;

void setup() {
  // Must be called once if pin is not set to output otherwise
  initTXPin();

  //Set Interrupt pin to HIGH
  pinMode(PHYSICAL_PIN_LINK_TO_INTERRUPT_PIN, INPUT_PULLUP);

  //Setting the XBee Sleeping Pin to HIGH = Go to sleep my friend
  pinMode(WAKE_XBEE_PIN, OUTPUT);
  digitalWrite(WAKE_XBEE_PIN, HIGH);
}

void loop() {

  //Reinit variable
  isTippingOccured = false;

  //Attach interrupt
  attachInterrupt(INTERRUPT_NUMBER, fakeFunction, LOW);

  //Using for method is not working...
  //for (unsigned long i= 0; i < ONE_HOUR_SECONDS; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);    
    eightSecondsPassedCounter++;
  //}

  //Detach interrupt
  detachInterrupt(INTERRUPT_NUMBER);

  if (eightSecondsPassedCounter >= ONE_HOUR || isTippingOccured) {
    advertiseTipOccured();

    //Reset counter
    eightSecondsPassedCounter = 0;
  } 
}

void fakeFunction() {
  isTippingOccured = true;
}

// Needed to compute ZigBee API 2 mode frame length
void computeMSB_LSB(uint16_t number, uint8_t &msb, uint8_t &lsb) {
    msb = (number >> 8) & 0xFF; // Shift right by 8 bits to get MSB
    lsb = number & 0xFF;        // Mask to get LSB
}

// See https://forum.arduino.cc/t/how-to-calculate-checksum-for-16bit-address-xbee-tx-frame/439001/4
uint8_t calculateChecksum(const uint8_t* data, size_t length) {  
  uint8_t checksum = 0;
  for (size_t i = 0; i < length; ++i) {
    //Serial.println(data[i],HEX);
    checksum += data[i];
  }  
  checksum &= 0xFF;        // low order 8 bits
  checksum = 0xFF - checksum; // subtract from 0xFF

  return  checksum;
}

//Create the ZigBee Mode API 2 - frame
//
//Using 0x00 - 64-bit Transmit Request (Deprecated one, should use 0x10 but I am using old XBee S1 (802.15.4 protocol stack)
//See https://www.digi.com/resources/documentation/Digidocs/90001477/reference/r_frame_0x00.htm?tocpath=API%20frames%7C_____3
void createAndSendZigBeeAPIFrame(String message) {
  
  //Gets an array of String : message
  byte messageArray[message.length()+1];
  message.getBytes(messageArray, message.length()+1);

  //Get messageArraySize
  int messageArraySize = sizeof(messageArray) / sizeof(messageArray[0]);
  
  //Declare array with needed space for message + need byte of frame
  uint8_t frame[14+sizeof(messageArray)]; 
  
  //Empty frame
  for (int i=0; i < sizeof(frame) / sizeof(frame[0]) -1; i++) {    
    frame[i] = 0;    
  }

  //Initialize Frame
  frame[0] = 0x7E; // Start delimiter
  //
  frame[1] = 0x00; // Length MSB (to be updated later)
  frame[2] = 0x00; // Length LSB (to be updated later)
  //
  frame[3] = 0x00; // Frame type ie 0x00 - Tx (Transmit) - Request: 64-bits address
  // 
  frame[4] = 0x42; // Frame ID 0x42 why not ;-)
  //
  // Desination address : here my coordinator 
  frame[5] = 0x00;
  frame[6] = 0x13;
  frame[7] = 0xA2;
  frame[8] = 0x00;
  frame[9] = 0x40;
  frame[10] = 0xB2;
  frame[11] = 0x45;
  frame[12] = 0xBF;
  //
  frame[13] = 0x00; // Options
  //
  // DATA      
  for (int i=0; i < messageArraySize-1; i++) { 
    //Serial.println("TOTO:"+String(messageArray[i], HEX));
    frame[14+i] = messageArray[i];    
  }
  //Checksum = all data except start + length (on non escaped data)
  //Start compute at frame+3 (ie frame type) to 11 (frame type to options + message size)
  frame[14+messageArraySize-1] = calculateChecksum(frame+3, 11+messageArraySize-1);

  //In API 2 mode, the length field does not include any escape character in the frame and the checksum is calculated with non-escaped data.  
  uint8_t msb, lsb;
  //Serial.println("Message Size = " + String(11+messageArraySize-1));
  computeMSB_LSB(11+messageArraySize-1, msb, lsb);  

  //Serial.print("MSB: 0x");
  //Serial.println(msb, HEX);

  //Serial.print("LSB: 0x");
  //Serial.println(lsb, HEX);

  //Set frame length
  frame[1] = msb;
  frame[2] = lsb;

  int j=1;
  //Compute number final arraylenght necessary with escaped characters needed
  for (int i=1; i < 14+messageArraySize; i++) {
    
    if (frame[i] == 0x7E 
     || frame[i] == 0x7D 
     || frame[i] == 0x13
     || frame[i] == 0x11)
      j++;
    
    j++;
  }

  //
  // DEBUG
  //
  /*Serial.println("DEBUT FRAME");
  for (int i=0; i < sizeof(frame); i++) {    
    Serial.print("0x" + String(frame[i], HEX) + " ");
  }
  Serial.println("FIN FRAME");
  return ;*/

  //Escape frame (all byte exept Start delimiter)
  uint8_t escapedFrame[j];

  //Start delimiter
  escapedFrame[0] = frame[0];
  j = 1;

  for (int i=1; i < 14+messageArraySize; i++) {
    
    if (frame[i] == 0x7E) {
      escapedFrame[j] = 0x7D;
      j++;
      escapedFrame[j] = 0x5E;      
      j++;
      continue;
    }

    if (frame[i] == 0x7D) {
      escapedFrame[j] = 0x7D;
      j++;
      escapedFrame[j] = 0x5D;      
      j++;
      continue;
    }

    if (frame[i] == 0x13) {
      escapedFrame[j] = 0x7D;
      j++;
      escapedFrame[j] = 0x33;      
      j++;
      continue;
    }

    if (frame[i] == 0x11) {
      escapedFrame[j] = 0x7D;
      j++;
      escapedFrame[j] = 0x31;      
      j++;
      continue;
    }
    
    //other case
    escapedFrame[j] = frame[i];
    j++;

  }
  //
  // DEBUG
  //
  /*Serial.println("DEBUT FRAME");
  for (int i=0; i < sizeof(escapedFrame); i++) {    
    Serial.print("0x" + String(escapedFrame[i], HEX) + " ");
  }
  Serial.println("FIN FRAME");*/

  //Send frame
  //Serial.write(escapedFrame);
  
  //char tt []= {0x7E, 0x00, 0x7D, 0x33, 0x00, 0x42, 0x00, 0x7D, 0x33, 0xA2, 0x00, 0x40, 0xB2, 0x45, 0xBF, 0x00, 0x66, 0x72, 0x65, 0x64, 0x65, 0x72, 0x69, 0x63, 0xCE};

  //WORKING
  //char tt [] = {0x7e, 0x0, 0x2a, 0x0, 0x42, 0x0, 0x7d, 0x33, 0xa2, 0x0, 0x40, 0xb2, 0x45, 0xbf, 0x0, 0x7b, 0x22, 0x74, 0x69, 0x70, 0x22, 0x3a, 0x22, 0x74, 0x72, 0x75, 0x65, 0x22, 0x2c, 0x22, 0x62, 0x61, 0x74, 0x74, 0x65, 0x72, 0x79, 0x22, 0x3a, 0x22, 0x32, 0x2e, 0x34, 0x32, 0x22, 0x7d, 0x5d, 0x9c};
  //for (int i=0;i<sizeof(tt);i++) {
  //  Serial.print(tt[i]);    
  //}
  //uint8_t tt []= {0x7E, 0x00, 0x7D, 0x33, 0x00, 0x42, 0x00, 0x7D, 0x33, 0xA2, 0x00, 0x40, 0xB2, 0x45, 0xBF, 0x00, 0x66, 0x72, 0x65, 0x64, 0x65, 0x72, 0x69, 0x63, 0xCE};
  //char* ptr = escapedFrame;
  //Serial.print(tt);

  for (int i=0;i<sizeof(escapedFrame);i++) {
    Serial.print((char)escapedFrame[i]);    
  }

  //Serial.flush();
}

void advertiseTipOccured() {

  String tippingOccured = "false";

  //Wakeup XBee
  digitalWrite(WAKE_XBEE_PIN, LOW);

  //Wait for XBee to wake up don't be so impatient
  delay(50);

  float battery_voltage = averageVoltage();

  if (isTippingOccured)
    tippingOccured = "true";

  //Send message  
  String message = "{\"sensorid\":\""+SENSOR_ID+"\",\"tip\":\""+tippingOccured+"\",\"battery\":\""+ String(battery_voltage,2)+"\"}";
  //String message = "frederic";

  //Send ZigBee message using API Mode 2 
  createAndSendZigBeeAPIFrame(message);
  
  //Wait for XBee to send the message and avoid to send multiple messages
  delay(1000);

  //Go to sleep XBee
  digitalWrite(WAKE_XBEE_PIN, HIGH);
}

// Compute an average voltage
float averageVoltage() {

  // set the ADC reference to 1.1V
  analogReference(INTERNAL);    
  
  int total = 0;
     
  for(byte i=0;i<BATTERY_READING;i++) {
    total = total + analogRead(ADC_PIN);
  }
     
  int averageSensor = total / BATTERY_READING; 
  
  //ADC reading = 964 for 4.2v
  float voltage = (averageSensor * 4.20) / 964;
  
  return voltage;             
}