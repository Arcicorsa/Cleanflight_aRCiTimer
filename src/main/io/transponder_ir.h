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

#pragma once

typedef struct transponderConfig_s {
    uint8_t data[9];
} transponderConfig_t;

typedef struct transponderType_s {
	uint8_t Type[1];
} transponderType_t;

PG_DECLARE(transponderConfig_t, transponderConfig);
PG_DECLARE(transponderType_t, transponderType);

void transponderInit(uint8_t* transponderCode, uint8_t* transponderType); 

void transponderEnable(void);
void transponderDisable(void);
void updateTransponder(const uint8_t* transponderCode);
void transponderUpdateData(uint8_t* transponderData, uint8_t* transponderType);
//void transponderUpdateType(uint8_t* transponderTipe);
void transponderTransmitOnce(const uint8_t* transponderType);
void transponderStartRepeating(void);
void transponderStopRepeating(void);
