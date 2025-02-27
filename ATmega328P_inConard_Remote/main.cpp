/*
 * ATmega328P_inConard_Remote.cpp
 *
 * Created: 4/17/2019 4:38:13 PM
 * Author : orencollaco
 */ 

#include "ProjectDef.h"
#include <avr/io.h>
#include "AVR.h"
#include "Essential.h"
#include "NRF24L01.h"
#include "Timer.h"
#include "Switch.h"
//extern SwitchClass Switch;
void printSomething(uint8_t);
void runSetup();
void pullSlaveSelectLow(uint8_t SwitchID);
void pullSlaveSelectHigh(uint8_t SwitchID);
void portStateChange(uint8_t PortNo);
void switchPressed(uint8_t Switch_ID);
void timerDone(uint8_t Timer_ID);
volatile uint8_t SPIdata, SwitchID;
volatile bool SwitchPressedFlag, TimeOut;
uint8_t Socket, State;
TimerClass Timer1, Timer2;
SwitchClass S1, S2, S3, S4, S5, S6;


int main(void)
{
	runSetup();
	_delay_ms(500);
	Notify(PSTR("Powering on..."));
	sei();
	TimeOut = false;
	NRF24L01 Radio(1,1,0);
	Radio.setTransmitAddress(0xEABABABAC1);
	Radio.setReceiveAddress(0xEABABABAC1, 0);
	Timer1.begin();
	Timer1.initializeTimer();
	Timer2.initializeTimer();
	Timer1.setCallBackTime(100, 0, timerDone);
	S1.begin();
	S1.initializeSwitch(PORT_B, 0, &S1); //1
	S2.initializeSwitch(PORT_B, 6, &S2); //0
	S3.initializeSwitch(PORT_B, 1, &S3); //6
	S4.initializeSwitch(PORT_B, 7, &S4);
	S5.initializeSwitch(PORT_D, 6, &S5);
	S6.initializeSwitch(PORT_D, 7, &S6);
	S1.shortPress(switchPressed);
	S1.enableSamePtrMode(true);
	Notify(PSTR("Done"));
	allowSleep(true);
	if(Radio.isRT_Max_Set())
	Radio.clearRT_Max();
	if(Radio.isTXFull())
	Radio.flushTX();
	if(Radio.isTX_DS_Set())
	Radio.clearTX_DS();
	while (1)
	{	
		if(SwitchPressedFlag){
			SwitchPressedFlag = false;
			#ifdef STATS
			printStringCRNL("Switch Pressed: ");
			printNumber(SwitchID);
			#endif
			if(SwitchID < 4){
				Radio.fastTransferPayload(SwitchID + 1);
			}
			else
			{
				if(SwitchID == 0x05)
				Radio.fastTransferPayload(0xD1);
				else
				Radio.fastTransferPayload(0xD0);
			}
			TimeOut = false;
			Timer2.setCallBackTime(50, 0, timerDone);
			while(Radio.isTX_DS_Set() != 0x20 && !TimeOut){
				
			}
			if(Radio.isTX_DS_Set() == 0x20){
				LED = 1;
				Radio.clearTX_DS();
				Timer1.setCallBackTime(50, 0, timerDone);
				Timer2.resetCallbackTimer();
				#ifdef STATS
				printStringCRNL("Payload transmitted.");
				#endif
				//Radio.flushTX();
			}
			else{
				TimeOut = false;
				//Radio.flushTX();
				#ifdef STATS
				printStringCRNL("Timed out");
				#endif
			}
			if(Radio.isRT_Max_Set())
			Radio.clearRT_Max();
			if(Radio.isTXFull())
			Radio.flushTX();
			if(Radio.isTX_DS_Set())
			Radio.clearTX_DS();
			#ifdef STATS
			Radio.printInfo();
			#endif
		}
		if(sleepAllowed()){
			#ifdef STATS
			printStringCRNL("Sleeping...");
			#endif
			set_sleep_mode(SLEEP_MODE_PWR_DOWN);
			sleep_enable();
			sei();
			getReadyToSleep();
			CE = 0;
			sleep_mode();
			CE = 1;
			_delay_ms(20);
		}
	}
}

void switchPressed(uint8_t Switch_ID){
	SwitchPressedFlag = true;
	SwitchID = Switch_ID;
	allowSleep(false);
}

void timerDone(uint8_t Timer_ID){
	#ifdef DEBUG_MAIN
	printStringCRNL("Timer done.");
	#endif
	 TimeOut = true;
	 LED = 0;
	 allowSleep(1);
}

void portStateChange(uint8_t PortNo){
	printStringCRNL("Pin state changed");
	if(PortNo == PORT_C){
		setPinState(PORT_C, 2, getPinState(PORT_C, 0));
	}
}

void runSetup(){
	
	//USART_Init(MYUBRR);
	Init_CTC_T1(2,1000);
	SPI_MasterInit();
	setPinDirection(PORT_D, 2, 1);
	//enableSPIInterrupt(true);
	//sei();
}


