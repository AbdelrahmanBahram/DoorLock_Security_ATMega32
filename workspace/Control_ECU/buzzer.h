/*
 * buzzer.h
 *
 * Author: Abdelrahman Bahram
 */ 


#ifndef BUZZER_H_
#define BUZZER_H_

#include <avr/io.h>

// Macro Definitions for Buzzer
#define BUZZER_PORT PORTC
#define BUZZER_PIN PC7
#define BUZZER_DDR DDRC


// Function Prototypes
void Buzzer_init(void);
void Buzzer_on(void);
void Buzzer_off(void);




#endif /* BUZZER_H_ */
