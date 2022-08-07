/*
 *  Copyright (C) 2010-2022 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef SERIAL_DEVICES_HPP_
#define SERIAL_DEVICES_HPP_

#include <vector>
#include "common.h"
#include "save_slot.h"

// ----------------------------------------------------------------------------------------

class serialDevice {
	protected:
		BYTE state;
		BYTE clock;
		BYTE data;
		BYTE output;

	public:
		serialDevice();
		virtual ~serialDevice();

	public:
		virtual void reset();
		virtual BYTE getData() const;
		virtual void setPins(BYTE select, BYTE newClock, BYTE newData);
		virtual BYTE saveMapper(BYTE mode, BYTE slot, FILE *fp);
};

// ----------------------------------------------------------------------------------------

class serialROM: public serialDevice {
	protected:
		int bitPosition;
		BYTE command;
		const BYTE *rom;

	public:
		serialROM(const BYTE *rom);
		~serialROM();

	public:
		void setPins(BYTE select, BYTE newClock, BYTE newData) override;
		BYTE saveMapper(BYTE mode, BYTE slot, FILE *fp) override;
};

// ----------------------------------------------------------------------------------------

class inverterROM: public serialDevice {
	protected:
		BYTE command;
		BYTE result;

	public:
		inverterROM();
		~inverterROM();

	public:
		void setPins(BYTE select, BYTE newClock, BYTE newData) override;
		BYTE saveMapper(BYTE mode, BYTE slot, FILE *fp) override;
};

// ----------------------------------------------------------------------------------------

typedef struct _attachedSerialDevice {
	serialDevice *device;
	BYTE select;
	BYTE clock;
	BYTE data;
} _attachedSerialDevice;

class GPIO_OneBus {
	protected:
		BYTE mask;
		BYTE latch;
		BYTE state;
		std::vector<_attachedSerialDevice> serialDevices;

	public:
		GPIO_OneBus();
		~GPIO_OneBus();

	public:
		void reset(void);
		BYTE read(BYTE address);
		void write(BYTE address, BYTE value);
		void attachSerialDevice(serialDevice *s, BYTE select, BYTE clock, BYTE data);
		BYTE saveMapper(BYTE mode, BYTE slot, FILE *fp);
};

#endif /* SERIAL_DEVICES_HPP_ */
