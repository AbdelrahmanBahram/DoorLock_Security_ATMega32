/*
 * buzzer.c
 *
 *
 *  Author: Abdelrahman Bahram
 */ 
#include "buzzer.h"

void Buzzer_init(void) {
	// Set the buzzer pin as an output
	BUZZER_DDR |= (1 << BUZZER_PIN);
	// Turn off the buzzer initially
	BUZZER_PORT &= ~(1 << BUZZER_PIN);
}

void Buzzer_on(void) {
	// Activate the buzzer
	BUZZER_PORT |= (1 << BUZZER_PIN);
}

void Buzzer_off(void) {
	// Deactivate the buzzer
	BUZZER_PORT &= ~(1 << BUZZER_PIN);
}
