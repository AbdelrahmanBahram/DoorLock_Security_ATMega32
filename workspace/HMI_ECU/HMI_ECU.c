/*
 * main.c
 *
 *  Author: Abdelrahman Bahram
 */

// HMI_ECU.c
#include <avr/io.h>
#include "lcd.h"
#include "keypad.h"
#include "common_macros.h"
#include "uart.h"
#include "std_types.h"
#include "timer.h"
#include <avr/interrupt.h>
#include <util/delay.h>

/*******************************************************************************
 *                                Definitions                                  *
 *******************************************************************************/
#define PASSWORD_SIZE 5
#define MAX_ATTEMPTS 3
#define INIT_COMM 0x15
#define RESPONSE_ACK 0x16
#define OPEN_DOOR_CMD '+'
#define CHANGE_PASS_CMD '-'
#define WAIT_CMD 0x20
#define RETRY_CMD 0x11
#define CONFIRM_KEY 13

/*******************************************************************************
 *                           Global Variables                                  *
 *******************************************************************************/

// Global variables
uint8 pass1[PASSWORD_SIZE];
uint8 pass2[PASSWORD_SIZE];
uint8 enteredPass[PASSWORD_SIZE];
uint8 failedAttempts = 0;
uint8 passInitialized = 0;

/*******************************************************************************
 *                            Functions Prototypes                             *
 *******************************************************************************/

// Function Prototypes
void systemInit();
void setPassword();
void showMainMenu();
void processCommand(uint8 cmd);
void inputPassword(uint8 *passBuffer, const char* prompt);
void transmitPasswordToControl(uint8 pass[]);
void handleLockout();
void showWaitingMessage();
void collectPasswords(uint8 *passBuffer1,uint8 *passBuffer2);
volatile uint16 tickCount = 0;  // Global tick counter

void Timer1_callback(void) {
    tickCount++;  // Increment tick every compare match
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
    systemInit();
    sei();
    LCD_displayString("Smart Door");
    LCD_displayStringRowColumn(1, 0, "locking system");
    _delay_ms(1000);
    LCD_clearScreen();
    setPassword();

    while (1) {
        if (passInitialized) {
            showMainMenu();
            uint8 option = KEYPAD_getPressedKey();
            _delay_ms(500);  // Debounce delay

            if (option == OPEN_DOOR_CMD || option == CHANGE_PASS_CMD) {
                processCommand(option);
            } else {
                LCD_clearScreen();
                LCD_displayStringRowColumn(1, 0, "Invalid Option");
                _delay_ms(1000);
            }
        } else {
            setPassword();
        }
    }
}

void systemInit() {
    LCD_init();
    UART_ConfigType uartConfig = {8, 0, 1, 9600};
    UART_init(&uartConfig);
}

void setPassword() {
    while (1) {
        collectPasswords(pass1, pass2);

        // Send passwords to Control_ECU
        UART_sendByte(INIT_COMM);
        for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
            UART_sendByte(pass1[i]);
        }
        UART_sendByte(INIT_COMM);
        for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
            UART_sendByte(pass2[i]);
        }

        uint8 matchStatus = UART_recieveByte();
        if (matchStatus == 1) {
            LCD_clearScreen();
            LCD_displayString("Password Set!");
            _delay_ms(1000);
            passInitialized = 1;
            break;
        } else if (matchStatus == 0) {
            LCD_clearScreen();
            LCD_displayString("Mismatch!");
            LCD_displayStringRowColumn(1, 0, "Try Again");
            _delay_ms(1000);
            passInitialized = 0;
            UART_sendByte(RETRY_CMD);
        }
    }
}

void showMainMenu() {
    LCD_clearScreen();
    LCD_displayString("(+) Open Door");
    LCD_displayStringRowColumn(1, 0, "(-) Change Pass");
}

void processCommand(uint8 cmd) {
    failedAttempts = 0;
    while (failedAttempts < MAX_ATTEMPTS) {
        inputPassword(enteredPass, "Enter Password:");
        UART_sendByte(cmd);  // Send the selected command
        _delay_ms(100);  // Wait briefly for synchronization
        transmitPasswordToControl(enteredPass);

        uint8 response = UART_recieveByte();
        if (response == 1) {
            LCD_clearScreen();
            if (cmd == OPEN_DOOR_CMD) {
                LCD_displayString("Door is");
                LCD_moveCursor(1, 0);
                LCD_displayString("Unlocking...");
                initTimer1();
                tickCount = 0;
                while (tickCount < 15);  // Keep the motor running for 15 seconds
                tickCount = 0;
                stopTimer1();
                showWaitingMessage();
            } else if (cmd == CHANGE_PASS_CMD) {
                setPassword();  // Re-create password
            }
            return;
        } else {
            LCD_clearScreen();
            LCD_displayString("Incorrect Pass!");
            _delay_ms(500);
            failedAttempts++;
        }
    }
    handleLockout();
}

void handleLockout() {
    if (failedAttempts >= MAX_ATTEMPTS) {
        initTimer1();
        tickCount = 0;
        LCD_clearScreen();
        LCD_displayString("System Locked!");
        while (tickCount < 60);  // Lock the system for 60 seconds (1 minute)
        failedAttempts = 0;
        stopTimer1();
    }
}

void collectPasswords(uint8 *passBuffer1, uint8 *passBuffer2) {
    uint8 key1, key2;
    LCD_clearScreen();
    LCD_displayString("Plz enter pass:");
    LCD_moveCursor(1, 0);

    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        *(passBuffer1 + i) = KEYPAD_getPressedKey();
        LCD_displayCharacter('*');
        _delay_ms(300);
    }

    // Wait for Enter key
    while (key1 != CONFIRM_KEY) {
        key1 = KEYPAD_getPressedKey();
        _delay_ms(500);
    }

    LCD_clearScreen();
    LCD_displayString("Plz re-enter");
    LCD_moveCursor(1, 0);
    LCD_displayString("same pass:");
    LCD_moveCursor(1, 10);
    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        *(passBuffer2 + i) = KEYPAD_getPressedKey();
        LCD_displayCharacter('*');
        _delay_ms(300);
    }

    // Wait for Enter key
    while (key2 != CONFIRM_KEY) {
        key2 = KEYPAD_getPressedKey();
        _delay_ms(500);
    }
}

void inputPassword(uint8 *passBuffer, const char *prompt) {
    uint8 key;
    LCD_clearScreen();
    LCD_displayString(prompt);
    LCD_moveCursor(1, 0);

    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        *(passBuffer + i) = KEYPAD_getPressedKey();
        LCD_displayCharacter('*');
        _delay_ms(300);
    }

    // Wait for Enter key
    while (key != CONFIRM_KEY) {
        key = KEYPAD_getPressedKey();
        _delay_ms(500);
    }
}

void transmitPasswordToControl(uint8 pass[]) {
    UART_sendByte(INIT_COMM);
    for (uint8 i = 0; i < PASSWORD_SIZE; i++) {
        UART_sendByte(pass[i]);
    }
}

void showWaitingMessage() {
    LCD_clearScreen();
    LCD_displayString("Wait for people");
    LCD_displayStringRowColumn(1, 0, "to enter");
    _delay_ms(1000);
    while (1) {
        uint8 pirStatus = UART_recieveByte();
        if (pirStatus == 0) {
            LCD_clearScreen();
            LCD_displayString("Door is");
            LCD_displayStringRowColumn(1, 0, "locking...");
            initTimer1();
            tickCount = 0;
            while (tickCount < 15);  // Keep the motor running for 15 seconds
            tickCount = 0;
            stopTimer1();
            break;
        }
    }
}
