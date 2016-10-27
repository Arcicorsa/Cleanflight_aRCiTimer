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

// aRCiTimer
#define TRANSPONDER_BITS_PER_BYTE_ARCITIMER 8 // start + 8 data + stop (pokud uberes tak na vystupu nebude cely bajt)
#define TRANSPONDER_DATA_LENGTH_ARCITIMER 9
#define TRANSPONDER_TOGGLES_PER_BIT_ARCITIMER 4 //11 tiknut� nosne na bit (kolik period bude v 1)
#define TRANSPONDER_GAP_TOGGLES_ARCITIMER 0 
#define TRANSPONDER_TOGGLES_ARCITIMER (TRANSPONDER_TOGGLES_PER_BIT_ARCITIMER + TRANSPONDER_GAP_TOGGLES_ARCITIMER)

#define TRANSPONDER_DMA_BUFFER_SIZE_ARCITIMER 155 * TRANSPONDER_TOGGLES_PER_BIT_ARCITIMER

// I-LAP
#define TRANSPONDER_BITS_PER_BYTE 10 // start + 8 data + stop
#define TRANSPONDER_DATA_LENGTH 6
#define TRANSPONDER_TOGGLES_PER_BIT 11
#define TRANSPONDER_GAP_TOGGLES 1
#define TRANSPONDER_TOGGLES (TRANSPONDER_TOGGLES_PER_BIT + TRANSPONDER_GAP_TOGGLES)

#define TRANSPONDER_DMA_BUFFER_SIZE ((TRANSPONDER_TOGGLES_PER_BIT + 1) * TRANSPONDER_BITS_PER_BYTE * TRANSPONDER_DATA_LENGTH)



// BOTH
#define BIT_TOGGLE_1 78 // (156 / 2) (delka mezery)
#define BIT_TOGGLE_0 0               

void transponderIrInit(void);
void transponderIrDisable(void);

void transponderIrHardwareInit(void);
void transponderIrDMAEnable(void);

void transponderIrWaitForTransmitComplete(void);

void transponderIrUpdateData(const uint8_t* transponderData);
void transponderIrTransmit(void);

bool isTransponderIrReady(void);


extern uint8_t transponderIrDMABuffer1[TRANSPONDER_DMA_BUFFER_SIZE_ARCITIMER];
extern uint8_t transponderIrDMABuffer[TRANSPONDER_DMA_BUFFER_SIZE];
extern volatile uint8_t transponderIrDataTransferInProgress;
extern uint8_t transponder_tipe;
