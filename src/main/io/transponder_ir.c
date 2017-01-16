/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include <platform.h>

#include "build/build_config.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/transponder_ir.h"
#include "drivers/system.h"

#include "drivers/usb_io.h"

#include "io/transponder_ir.h"
#include "fc/config.h"

static bool transponderInitialised = false;
static bool transponderRepeat = false;

// aRCiTimer transponder codes:
	//
	// ID1 0x1F, 0xFC, 0x8F, 0x3, 0xF0, 0x1, 0xF8, 0x1F, 0x0           // E00370FC0FFE07E0FF
	// ID2 0xFF, 0x83, 0xFF, 0xC1, 0x7, 0xFF, 0x3, 0xF0, 0x1           // 007C003EF800FC0FFE
	// ID3 0x7, 0x7E, 0xE0, 0x7, 0x7E, 0xE0, 0x0, 0x38, 0x0            // F8811FF8811FFFC7FF
	// ID4 0xFF, 0x83, 0xFF, 0xC1, 0x7, 0xE0, 0x7F, 0xF0, 0x1          // 007C003EF81F800FFE
	// ID5 0xF, 0xF0, 0x0, 0xFF, 0x0, 0xF, 0xF0, 0xF, 0x0              // F00FFF00FFF00FF0FF
	// ID6 0xFF, 0x83, 0xF, 0x3E, 0xF8, 0xE0, 0x83, 0xFF, 0xF          // 007CF0C1071F7C00F0
	// ID7 0x1F, 0xFC, 0xF, 0xC0, 0xFF, 0x0, 0xFC, 0xF, 0x3E           // E003F03F00FF03F0C1
	// ID8 0xFF, 0x3, 0xF0, 0x1, 0xF8, 0xE0, 0xC1, 0xFF, 0x1           // 00FC0FFE071F3E00FE
	// ID9 0x1F, 0x7C, 0x40, 0xF, 0xF0, 0x61, 0xC7, 0x3F, 0x0          // E083BFF00F9E38C0FF


PG_REGISTER_WITH_RESET_TEMPLATE(transponderConfig_t, transponderConfig, PG_TRANSPONDER_CONFIG, 0);
PG_RESET_TEMPLATE(transponderConfig_t, transponderConfig,
    .data =  { 0x1F, 0xFC, 0x8F, 0x3, 0xF0, 0x1, 0xF8, 0x1F, 0x0 }, // Note, this is NOT a valid transponder code, it's just for testing production hardware
);

PG_REGISTER_WITH_RESET_TEMPLATE(transponderType_t, transponderType, PG_TRANSPONDERTYPE, 0);
PG_RESET_TEMPLATE(transponderType_t, transponderType, 
	.Type = { 0x0 },  
);

// timers
static uint32_t nextUpdateAt = 0;

#define JITTER_DURATION_COUNT (sizeof(jitterDurations) / sizeof(uint8_t))
static uint8_t jitterDurations[] = {0,9,4,8,3,9,6,7,1,6,9,7,8,2,6};

void updateTransponder(const uint8_t* transponderType) 
{
    static uint32_t jitterIndex = 0;

    if (!(transponderInitialised && transponderRepeat && isTransponderIrReady())) {
        return;
    }

    uint32_t now = micros();

    bool updateNow = (int32_t)(now - nextUpdateAt) >= 0L;
    if (!updateNow) {
        return;
    }

    // TODO use a random number genenerator for random jitter?  The idea here is to avoid multiple transmitters transmitting at the same time.
    uint32_t jitter = (1000 * jitterDurations[jitterIndex++]);
    if (jitterIndex >= JITTER_DURATION_COUNT) {
        jitterIndex = 0;
    }

    nextUpdateAt = now + 4500 + jitter;

#ifdef REDUCE_TRANSPONDER_CURRENT_DRAW_WHEN_USB_CABLE_PRESENT
    // reduce current draw when USB cable is plugged in by decreasing the transponder transmit rate.
    if (usbCableIsInserted()) {
        nextUpdateAt = now + (1000 * 1000) / 10; // 10 hz.
    }
#endif

	transponderIrTransmit(transponderType); 
}

void transponderInit(uint8_t* transponderData, uint8_t* transponderType) 
{
    transponderInitialised = false;
    transponderIrInit(transponderType); 
    transponderIrUpdateData(transponderData, transponderType); 
}

void transponderEnable(void)
{
    transponderInitialised = true;
}

void transponderDisable(void)
{
    transponderInitialised = false;
}

void transponderStopRepeating(void)
{
    transponderRepeat = false;
}

void transponderStartRepeating(void)
{
    transponderRepeat = true;
}

void transponderUpdateData(uint8_t* transponderData, uint8_t* transponderType) 
{
    transponderIrUpdateData(transponderData, transponderType); 
}

void transponderTransmitOnce(const uint8_t* transponderType)
{

    if (!transponderInitialised) {
        return;
    }
	transponderIrTransmit(transponderType); 
}
