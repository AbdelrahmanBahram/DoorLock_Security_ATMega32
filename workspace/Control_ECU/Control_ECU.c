/*
 * main.c
 *
 *  Author: Abdelrahman Bahram
 */
// Control_ECU.c

#include <avr/io.h>
#include "uart.h"
#include "external_eeprom.h"
#include "motor.h"
#include "buzzer.h"
#include "pir.h"
#include "std_types.h"
#include "twi.h"
#include "timer.h"
#include <avr/interrupt.h>
#include <util/delay.h>

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define PASSWORD_SIZE 5
#define EEPROM_ADDR 0x0311
#define COMM_START_BYTE 0x15
#define OPEN_DOOR_CMD '+'
#define CHANGE_PASS_CMD '-'
#define WAIT_CMD 0x20
#define MAX_ATTEMPTS 3
#define RETRY_CMD 0x11

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

uint8 storedPassword[PASSWORD_SIZE];
uint8 inputPassword[PASSWORD_SIZE];
uint8 failedAttempts = 0;

/*******************************************************************************
 *                            Functions Prototypes                             *
 *******************************************************************************/

void systemInitialize();
void openDoor();
void closeDoor();
void receiveAndCheckPasswords();
void receiveAndCheckPasswordForCommand(uint8 cmd);
void storePasswordToEEPROM(uint8 *password);
void retrievePasswordFromEEPROM(uint8 *password);
void manageFailedAttempts();

volatile uint16 timerTick = 0;  // Global tick counter

void Timer1_callback(void) {
    timerTick++;  // Increment tick every compare match
}

void initTimer1(void) {
    Timer_ConfigType timerConfig = {0, 7812, TIMER_1, TIMER_CLOCK_1024, TIMER_COMPARE_MODE};  // 1-second interval with 8MHz clock
    Timer_init(&timerConfig);
    Timer_setCallBack(Timer1_callback, TIMER_1);
}

void stopTimer1(void) {
    Timer_deInit(TIMER_1);
}

int main(void) {
    systemInitialize();
    sei();
    receiveAndCheckPasswords();

    while (1) {
        uint8 command = UART_recieveByte();
        if (command == OPEN_DOOR_CMD || command == CHANGE_PASS_CMD) {
            receiveAndCheckPasswordForCommand(command);
        }
        else if (command == RETRY_CMD) {
            receiveAndCheckPasswords();
        }
    }
}

void systemInitialize() {
    UART_ConfigType uartConfig = {8, 0, 1, 9600};
    TWI_ConfigType twiConfig = {0x01, 12};
    UART_init(&uartConfig);
    TWI_init(&twiConfig);
    DcMotor_Init();
    Buzzer_init();
    PIR_init();
}

void openDoor() {
    initTimer1();
    timerTick = 0;
    DcMotor_Rotate(CW, 100);
    while (timerTick < 15);  // Keep the motor running for 15 seconds
    DcMotor_Rotate(STOP, 100);
    timerTick = 0;
    stopTimer1();

    while (PIR_getState()) {
        UART_sendByte(WAIT_CMD);
    }
    UART_sendByte(0);
    closeDoor();
}

void closeDoor() {
    initTimer1();
    timerTick = 0;
    DcMotor_Rotate(ACW, 100);
    while (timerTick < 15);  // Keep the motor running for 15 seconds to lock
    DcMotor_Rotate(STOP, 100);
    stopTimer1();
}

void receiveAndCheckPasswords() {
    uint8 password1[PASSWORD_SIZE], password2[PASSWORD_SIZE];

    while (UART_recieveByte() != COMM_START_BYTE);
    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        password1[i] = UART_recieveByte();
    }

    while (UART_recieveByte() != COMM_START_BYTE);
    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        password2[i] = UART_recieveByte();
    }

    if (memcmp(password1, password2, PASSWORD_SIZE) == 0) {
        storePasswordToEEPROM(password1);
        UART_sendByte(1);
    } else {
        UART_sendByte(0);
    }
}

void receiveAndCheckPasswordForCommand(uint8 cmd) {
    retrievePasswordFromEEPROM(storedPassword);

    while (UART_recieveByte() != COMM_START_BYTE);
    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        inputPassword[i] = UART_recieveByte();
    }

    if (memcmp(storedPassword, inputPassword, PASSWORD_SIZE) == 0) {
        UART_sendByte(1);
        failedAttempts = 0;
        if (cmd == OPEN_DOOR_CMD) openDoor();
        else if (cmd == CHANGE_PASS_CMD) receiveAndCheckPasswords();
    } else {
        UART_sendByte(0);
        failedAttempts++;
        manageFailedAttempts();
    }
}

void storePasswordToEEPROM(uint8 *password) {
    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        EEPROM_writeByte(EEPROM_ADDR + i, password[i]);
        _delay_ms(10);
    }
}

void retrievePasswordFromEEPROM(uint8 *password) {
    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        EEPROM_readByte(EEPROM_ADDR + i, &password[i]);
        _delay_ms(10);
    }
}

void manageFailedAttempts() {
    if (failedAttempts >= MAX_ATTEMPTS) {
        initTimer1();
        timerTick = 0;
        Buzzer_on();
        while (timerTick < 60);  // Keep buzzer on for 1 minute (60 seconds)
        Buzzer_off();
        failedAttempts = 0;
        stopTimer1();
    }
}
