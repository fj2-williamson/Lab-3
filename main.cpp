//=====[Libraries]=============================================================

#include "mbed.h"
#include "arm_book_lib.h"

//=====[Declaration and initialization of public global objects]===============

DigitalIn enterButton(BUTTON1);
DigitalIn gasDetector(D2);
DigitalIn overTempDetector(D3);
DigitalIn aButton(D4);
DigitalIn bButton(D5);
DigitalIn cButton(D6);
DigitalIn dButton(D7);

DigitalOut alarmLed(LED1);
DigitalOut TempLed(LED1);
DigitalOut incorrectCodeLed(LED3);
DigitalOut systemBlockedLed(LED2);

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);

#define cycle 2000ms

//=====[Declaration and initialization of public global variables]=============

bool alarmState = OFF;
bool TempAlarm = OFF;
bool monitoring = OFF;
int numberOfIncorrectCodes = 0;

//=====[Declarations (prototypes) of public functions]=========================

void inputsInit();
void outputsInit();

void alarmActivationUpdate();
void alarmDeactivationUpdate();

void uartTask();
void availableCommands();

//=====[Main function, the program entry point after power on or reset]========

int main()
{
    inputsInit();
    outputsInit();
    while (true) {
        alarmActivationUpdate();
        alarmDeactivationUpdate();
        uartTask();
    }
}

//=====[Implementations of public functions]===================================

void inputsInit()
{
    gasDetector.mode(PullDown);
    overTempDetector.mode(PullDown);
    aButton.mode(PullDown);
    bButton.mode(PullDown);
    cButton.mode(PullDown);
    dButton.mode(PullDown);
}

void outputsInit()
{
    alarmLed = OFF;
    TempLed = OFF;
    incorrectCodeLed = OFF;
    systemBlockedLed = OFF;
}

void alarmActivationUpdate()
{
    if ( gasDetector) {
        alarmState = ON;
    }
    if (overTempDetector) {
        TempAlarm = ON;
    }
    alarmLed = alarmState;
    TempLed = TempAlarm;


}

void alarmDeactivationUpdate()
{
    if ( numberOfIncorrectCodes < 5 ) {
        if ( aButton && bButton && cButton && dButton && !enterButton ) {
            incorrectCodeLed = OFF;
        }
        if ( enterButton && !incorrectCodeLed && alarmState ) {
            if ( aButton && bButton && !cButton && !dButton ) {
                alarmState = OFF;
                numberOfIncorrectCodes = 0;
            } else {
                incorrectCodeLed = ON;
                numberOfIncorrectCodes = numberOfIncorrectCodes + 1;
            }
        }
    } else {
        systemBlockedLed = ON;
    }
}

void uartTask()
{
    char receivedChar = '\0';
    
        if (monitoring){
            uartUsb.write("---- SYSTEM STATUS ---- \r\n", 26);
            if (alarmState){
                uartUsb.write("GAS ALARM: ACTIVE\r\n", 19);
            }else{
                uartUsb.write("GAS ALARM: CLEAR\r\n", 18);
            }
            if (TempAlarm){
                uartUsb.write("TEMP ALARM: ACTIVE\r\n", 20);
            }else{
                uartUsb.write("TEMP ALARM: CLEAR\r\n", 19);
            }
            uartUsb.write("-----------------------\r\n", 25);
            ThisThread::sleep_for(cycle); }

    if( uartUsb.readable() ) {
        uartUsb.read( &receivedChar, 1 );
        switch (receivedChar){

    case '1':
            alarmState = !alarmState;
            alarmLed = alarmState;
            if (alarmState){
                uartUsb.write( "WARNING: GAS DETECTED\r\n", 23);
            }
        break;

     case '4':
            TempAlarm = !TempAlarm;
            TempLed = TempAlarm;
            if (TempAlarm){
                uartUsb.write( "WARNING: TEMPERATURE TOO HIGH\r\n", 31);
            }
            break;
        
    case '2':
            if (alarmState){
                uartUsb.write( "GAS ALARM ACTIVE\r\n", 18);
        } else{
                uartUsb.write( "GAS ALARM CLEAR\r\n", 17);
        }
        break;
    
    case '3':
            if (TempAlarm){
                uartUsb.write( "TEMP ALARM ACTIVE\r\n", 19);
        } else{
                uartUsb.write( "TEMP ALARM CLEAR\r\n", 18);
        }
        break;
    
    case '5':
        if (alarmState){
           uartUsb.write( "GAS ALARM ON\r\n", 14);
        }else{
           uartUsb.write( "GAS ALARM OFF\r\n", 15); 
        }
        if (TempAlarm){
            uartUsb.write( "TEMP ALARM ON\r\n", 15);
        }else{
            uartUsb.write( "TEMP ALARM OFF\r\n", 16);
        }
        alarmState = OFF;
        TempAlarm = OFF;
        uartUsb.write( "ALL ALARMS RESET\r\n", 18);
        break;

        case '6':
        monitoring = !monitoring;
        break;
    
    default:
            availableCommands();
            break;
        
        }
    }
}

void availableCommands()
{
    uartUsb.write( "Available commands:\r\n", 21 );
    uartUsb.write( "Press '1' to change the gas alarm state\r\n\r\n", 43 );
    uartUsb.write( "Press '2' to get the gas alarm state\r\n\r\n", 40 );
    uartUsb.write( "Press '3' to get the temp alarm state\r\n\r\n", 41);
    uartUsb.write( "Press '4' to change the temp alarm state\r\n\r\n", 44 );
    uartUsb.write( "Press '5' to reset all alarms\r\n\r\n", 33 );
}
