/*
  Test program for Adafruit Feather shields.
  Uses mostly the Adafruit libraries.
  See www.adafruit.com

  See aslo Radiohead libraries.
  Currently only a recieve data node, for connecting to a Linux host or othewise.
  
*/

#include <Wire.h>
#include "RTClib.h"

/* Set up the LoRa radio */ 
#include <SPI.h>
#include <RH_RF95.h>
#include <RHDatagram.h>  // Use the second Radiohead if you want to use a Manager.
 
/* Interface for Feather m0 w/wing */
#define RFM95_RST     11   // "A"
#define RFM95_CS      10   // "B"
#define RFM95_INT     6    // "D"
 

#define RF95_FREQ 868.0 // Change to 868.0 or other frequency, must match RX's freq!
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
uint8_t len = sizeof(buf);

// Singleton instance of the radio driver and also the radio manager.
RH_RF95 rf95(RFM95_CS, RFM95_INT);
RHDatagram datagram (RHGenericDriver &rf95, uint8_t thisAddress=0);   // Use if you want to define nodes.



#define VBATPIN A7
float measuredvbat = analogRead(VBATPIN);

// Instance of the precision real time clock (not M0 internal RTC)
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
DateTime now;


/*********************************/
/*           SETUP               */
/*********************************/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // Now the RTC
  Serial.begin(9600);
  delay(3000); // wait for console opening
  Serial.println("Feather LoRa TX and RTC Test!");

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    //rtc.adjust(DateTime(2017, 9, 27, 23, 03, 0));
  }
  
/* Set thus line to specifically set the time. */
  // rtc.adjust(DateTime(2017, 9, 27, 23, 7, 0));


// manual reset of LoRa
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);
 
  while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    while (1);
  }
  Serial.println("LoRa radio init OK!");
 
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
 
int16_t packetnum = 0;  // packet counter, we increment per xmission


 }  // End of setup


/*********************************/
/*           MAIN                */
/*********************************/

// the loop function runs over and over again forever
void loop() {



 
/* Wait for a message over Lora */

//Serial.println("Waiting for data..."); delay(10);
  if (rf95.waitAvailableTimeout(1000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {      
      Serial.println();
      now = rtc.now();
      showTime(now);      // Simple function to display datetime.
      
      Serial.println();
       
      Serial.print("Data received: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf95.lastRssi(), DEC);
      // do a double blink acknowledge
      blink();

/* Show the battery voltage */
      measuredvbat *= 2;    // we divided by 2, so multiply back
      measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
      measuredvbat /= 1024; // convert to voltage
      Serial.print("VBat: " ); Serial.println(measuredvbat);
    }
    else
    {
      Serial.println("Receive failed");
    }
  }
  else
  {
    Serial.print(".");                  // Just show activity
  }

    
    delay(1000);


}  // End of main



/*********************************/
/*           FUNCTIONS           */
/*********************************/

void blink () {
   // do a double blink acknowledge  
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(100);                       // wait for a second
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);                       // wait for a second
}

void showTime( DateTime now) {
   
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(' ');
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.year(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

  
/*    
    Serial.print(" since midnight 1/1/1970 = ");
    Serial.print(now.unixtime());
    Serial.print("s = ");
    Serial.print(now.unixtime() / 86400L);
    Serial.println("d");
   
    // calculate a date which is 7 days and 30 seconds into the future
    DateTime future (now + TimeSpan(7,12,30,6));
    
    Serial.print(" now + 7d + 30s: ");
    Serial.print(future.year(), DEC);
    Serial.print('/');
    Serial.print(future.month(), DEC);
    Serial.print('/');
    Serial.print(future.day(), DEC);
    Serial.print(' ');
    Serial.print(future.hour(), DEC);
    Serial.print(':');
    Serial.print(future.minute(), DEC);
    Serial.print(':');
    Serial.print(future.second(), DEC);
 */
}




