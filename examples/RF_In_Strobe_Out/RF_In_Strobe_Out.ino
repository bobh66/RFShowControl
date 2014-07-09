/*
* RF_In_Strobe_Out.ino
*
* Required dependencies : ShiftPWM Library https://github.com/elcojacobs/ShiftPWM
*                         FastLED Library https://github.com/FastLED/FastLED
*
*  Created on: Mar  2013
*  Updated 7/8/2014
*      Author: Greg Scull, komby@komby.com
*
*/


#include <Arduino.h>
#include <RFPixelControl.h>
#include <IPixelControl.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <EEPROM.h>
#include "printf.h"



/********************* START OF REQUIRED CONFIGURATION ***********************/
// NRF_TYPE Description: http://learn.komby.com/wiki/58/configuration-settings#NRF_TYPE
// Valid Values: RF1, KOMBLINKIN
#define NRF_TYPE                        KOMBLINKIN

// OVER_THE_AIR_CONFIG_ENABLE Description: http://learn.komby.com/wiki/58/configuration-settings#OVER_THE_AIR_CONFIG_ENABLE
// Valid Values: OTA_ENABLED, OTA_DISABLED
#define OVER_THE_AIR_CONFIG_ENABLE      0

// RECEIVER_UNIQUE_ID Description: http://learn.komby.com/wiki/58/configuration-settings#RECEIVER_UNIQUE_ID
// Valid Values: 1-255
#define RECEIVER_UNIQUE_ID              33
/********************** END OF REQUIRED CONFIGURATION ************************/

/****************** START OF NON-OTA CONFIGURATION SECTION *******************/
// LISTEN_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#LISTEN_CHANNEL
// Valid Values: 1-83
#define LISTEN_CHANNEL                  10

// DATA_RATE Description: http://learn.komby.com/wiki/58/configuration-settings#DATA_RATE
// Valid Values: RF24_250KBPS, RF24_1MBPS
#define DATA_RATE                       RF24_250KBPS

// HARDCODED_START_CHANNEL Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_START_CHANNEL
// Valid Values: 1-512
#define HARDCODED_START_CHANNEL         1

// HARDCODED_NUM_CHANNELS Description: http://learn.komby.com/wiki/58/configuration-settings#HARDCODED_NUM_CHANNELS
// Valid Values: 1
// strobe control only uses one channel
#define HARDCODED_NUM_CHANNELS          1
/******************* END OF NON-OTA CONFIGURATION SECTION ********************/

/************** START OF ADVANCED SETTINGS SECTION (OPTIONAL) ****************/



// DEBUG Description: http://learn.komby.com/wiki/58/configuration-settings#DEBUG
//#define DEBUG                           1

// STROBE_MODE Description: http://learn.komby.com/wiki/58/configuration-settings#STROBE_MODE
//ValidValues: 1 or 0  (on or off)
#define STROBE_MODE 1

// STROBE_DURATION Description: http://learn.komby.com/wiki/58/configuration-settings#STROBE_DURATION
//ValidValues: 1 or 0  (on or off)
#define STROBE_DURATION 75

#define PIXEL_TYPE STROBE
/********************* END OF ADVANCED SETTINGS SECTION **********************/
//You do not need to change settings below this line



const int ShiftPWM_latchPin=9;
#define SHIFTPWM_NOSPI
const int ShiftPWM_dataPin = 10;
const int ShiftPWM_clockPin = 6;


const bool ShiftPWM_invertOutputs = false;
const bool ShiftPWM_balanceLoad = false;

//Include this after all configuration variables are set
#include <ShiftPWM.h>
#include <lib8tion.h>
#include <RFPixelControlConfig.h>
#define RECEIVER_NODE 1

// Here you set the number of brightness levels, the update frequency and the number of shift registers.
// These values affect the load of ShiftPWM.
// Choose them wisely and use the PrintInterruptLoad() function to verify your load.
// There is a calculator on my website to estimate the load.

unsigned char maxBrightness = 255;
unsigned char pwmFrequency = 110;
int numRegisters = 2;
int numLeds;



bool readytoupdate=false;

byte * buffer;


//Uncomment for serial
#define DEBUG 0


//Arduino setup function.
void setup() {
	Serial.begin(115200);
	buffer[0]=255;
	
	// Sets the number of 8-bit registers that are used.
	ShiftPWM.SetAmountOfRegisters(numRegisters);
	ShiftPWM.SetPinGrouping(1);
	
	
	
	radio.EnableOverTheAirConfiguration(OVER_THE_AIR_CONFIG_ENABLE);
	if(!OVER_THE_AIR_CONFIG_ENABLE)
	{
		int logicalControllerSequenceNum = 0;
		radio.AddLogicalController(logicalControllerSequenceNum, HARDCODED_START_CHANNEL, HARDCODED_NUM_CHANNELS,0);
	}
	printf_begin();
	
	delay(200);

	radio.Initialize( radio.RECEIVER, pipes, LISTEN_CHANNEL,DATA_RATE ,RECEIVER_UNIQUE_ID);
	delay (2000);
	
	numLeds = radio.GetNumberOfChannels(0);
	delay (2000);


	radio.printDetails();
	//initialize data buffer
	buffer= radio.GetControllerDataBase(0);
	delay (2000);
	
	

	ShiftPWM.Start(pwmFrequency,maxBrightness);
	
	
	ShiftPWM.SetAll(0);

	for (int i =0; i < numLeds;i++)
	{
		for (int j=0;j<255;j++){
			ShiftPWM.SetOne(i, j);
			delay(2);
		}
		delay(10);
		for (int j=255;j>=0;j--){
			ShiftPWM.SetOne(i, j);
			delay(2);
		}
	}

	
	ShiftPWM.SetAll(0);
	delay(400);
	

	cli();
	delay(400);
	
}

// Do work loop
void loop(void){

	if (radio.Listen()){
		
		sei();
		if(STROBE_MODE == 1)
		strobeMode(millis(), buffer[0]);
		else
		updateLEDsWithRFData();
		
		cli();

	}
	
}


//Not working at the moment
void updateLEDsWithRFData(void){
	
	ShiftPWM.SetAll(0);
	for (int i =0; i < numLeds;i++)
	{
		ShiftPWM.SetOne(i, buffer[i]);
	}

	delay(30);
}



//handle the strobing randomly
void strobeMode( int seed, int d ) {
	
	ShiftPWM.SetAll(0);
	if (d>0){
		
		for(int i=0;i<4;i++)
		{
			ShiftPWM.SetOne(random16(seed)%16, 255);
			delay(1);

		}
		//delay so you can see the strobe
		delay(STROBE_DURATION);
	}
	
}
