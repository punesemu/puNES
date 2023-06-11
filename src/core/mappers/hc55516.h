// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_HC55516_H
#define MAME_SOUND_HC55516_H
#pragma once

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <assert.h>

class hc55516_device {
public:
	hc55516_device(uint32_t clock);
	virtual ~hc55516_device();

	/* sets the digit (0 or 1) */
	void digit_w(int digit);

	/* sets the clock state (0 or 1, clocked on the rising edge) */
	void clock_w(int state);

	/* returns whether the clock is currently LO or HI */
	int clock_state_r();

	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

	// sound stream update overrides
	virtual void sound_stream_update(int16_t *output, int samples);

	void start_common(uint8_t _shiftreg_mask, int _active_clock_hi);

	// internal state
	uint32_t  m_clock;
	int     m_active_clock_hi;
	uint8_t   m_shiftreg_mask;

	uint8_t   m_last_clock_state;
	uint8_t   m_digit;
	uint8_t   m_new_digit;
	uint8_t   m_shiftreg;

	int16_t   m_curr_sample;
	int16_t   m_next_sample;

	uint32_t  m_update_count;

	double  m_filter;
	double  m_integrator;

	double  m_charge;
	double  m_decay;
	double  m_leak;

	inline int is_external_oscillator();
	inline int is_active_clock_transition(int clock_state);
	inline int current_clock_state();
	void process_digit();
};

#endif // MAME_SOUND_HC55516_H
