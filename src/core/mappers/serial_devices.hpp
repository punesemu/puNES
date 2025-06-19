/*
 *  Copyright (C) 2010-2026 Fabio Cavallo (aka FHorse)
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
		BYTE state{};
		BYTE clock{};
		BYTE data{};
		BYTE output{};

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

class I2CDevice: public serialDevice {
	protected:
		BYTE deviceType;
		BYTE deviceAddr;
		bool readMode;

	public:
		I2CDevice(BYTE _deviceType, BYTE _deviceAddr);
		virtual ~I2CDevice();

	public:
		virtual void reset();
		virtual void setPins(BYTE select, BYTE newClock, BYTE newData);
		virtual void receiveBit();
		virtual BYTE saveMapper (BYTE mode, BYTE slot, FILE *fp);
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

// ----------------------------------------------------------------------------------------

class EEPROM_I2C: public I2CDevice {
	protected:
		WORD address;
		WORD addressMask;
		BYTE *rom;
		BYTE bit;
		BYTE latch;

	public:
		EEPROM_I2C(WORD _addressMask, BYTE _deviceAddr, BYTE *rom);
		~EEPROM_I2C();

	public:
		virtual void reset();
		virtual void receiveBit();
		virtual BYTE saveMapper(BYTE mode, BYTE slot, FILE *fp);
};

class EEPROM_24C01: public EEPROM_I2C {
	public:
		EEPROM_24C01(BYTE _deviceAddr, BYTE *rom);
		~EEPROM_24C01();

	public:
		void       receiveBit();
};

class EEPROM_24C02: public EEPROM_I2C {
	public:
		EEPROM_24C02(BYTE _deviceAddr, BYTE *rom);
		~EEPROM_24C02();
};

class EEPROM_24C04: public EEPROM_I2C {
	public:
		EEPROM_24C04(BYTE _deviceAddr, BYTE *rom);
		~EEPROM_24C04();

	public:
		void       receiveBit();
};

class EEPROM_24C08: public EEPROM_I2C {
	public:
		EEPROM_24C08(BYTE _deviceAddr, BYTE *rom);
		~EEPROM_24C08();

		public:
		void       receiveBit();
};

class EEPROM_24C16: public EEPROM_I2C {
	public:
		EEPROM_24C16(BYTE _deviceAddr, BYTE *rom);
		~EEPROM_24C16();

	public:
		void       receiveBit();
};

#endif /* SERIAL_DEVICES_HPP_ */
