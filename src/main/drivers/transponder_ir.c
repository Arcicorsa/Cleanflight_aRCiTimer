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
#include <stdint.h>
#include <string.h>

#include <platform.h>

#include "build/build_config.h"

#include "drivers/dma.h"
#include "drivers/nvic.h"
#include "drivers/transponder_ir.h"

/*
 * Implementation note:
 * Using around over 700 bytes for a transponder DMA buffer is a little excessive, likely an alternative implementation that uses a fast
 * ISR to generate the output signal dynamically based on state would be more memory efficient and would likely be more appropriate for
 * other targets.  However this approach requires very little CPU time and is just fire-and-forget.
 *
 * On an STM32F303CC 720 bytes is currently fine and that is the target for which this code was designed for.
 */
uint8_t transponderIrDMABuffer1[TRANSPONDER_DMA_BUFFER_SIZE_ARCITIMER];
uint8_t transponderIrDMABuffer[TRANSPONDER_DMA_BUFFER_SIZE];

volatile uint8_t transponderIrDataTransferInProgress = 0;

static dmaCallbackHandler_t transponderDMACallbacRec;


void transponderDMAHandler(dmaChannel_t* descriptor, dmaCallbackHandler_t* handler)
{
    UNUSED(handler);

    if (DMA_GET_FLAG_STATUS(descriptor, DMA_IT_TCIF)) {
        transponderIrDataTransferInProgress = 0;
        DMA_Cmd(descriptor->channel, DISABLE);
        DMA_CLEAR_FLAG(descriptor, DMA_IT_TCIF);
    }
}

void transponderIrInit(const uint8_t* transponderType)
{
	uint8_t transponderTypeLocal = *transponderType;
	
	if (transponderTypeLocal == 0x0) {
		memset(&transponderIrDMABuffer1, 0, TRANSPONDER_DMA_BUFFER_SIZE_ARCITIMER);
	}
	else if(transponderTypeLocal == 0x1) {
		memset(&transponderIrDMABuffer, 0, TRANSPONDER_DMA_BUFFER_SIZE);

	}
	
    dmaHandlerInit(&transponderDMACallbacRec, transponderDMAHandler);
    dmaSetHandler(TRANSPONDER_DMA_HANDLER_IDENTIFER, &transponderDMACallbacRec, NVIC_PRIO_TRANSPONDER_DMA);
	transponderIrHardwareInit(transponderType); 
}

bool isTransponderIrReady(void)
{
    return !transponderIrDataTransferInProgress;
}

static uint16_t dmaBufferOffset;


void updateTransponderDMABuffer(const uint8_t* transponderData, const uint8_t* transponderType)
{
	uint8_t transponderTypeLocal = *transponderType;
	
	uint8_t byteIndex;
    uint8_t bitIndex;
    uint8_t toggleIndex;

	if (transponderTypeLocal == 0x0) {
		

		for (byteIndex = 0; byteIndex < TRANSPONDER_DATA_LENGTH_ARCITIMER; byteIndex++) {

			uint8_t byteToSend = *transponderData;
			transponderData++;
			for (bitIndex = 0; bitIndex < TRANSPONDER_BITS_PER_BYTE_ARCITIMER; bitIndex++)
			{
				bool doToggles = byteToSend & (1 << (bitIndex));

				for (toggleIndex = 0; toggleIndex < TRANSPONDER_TOGGLES_PER_BIT_ARCITIMER; toggleIndex++)
				{
					transponderIrDMABuffer1[dmaBufferOffset] = doToggles ? BIT_TOGGLE_1 : BIT_TOGGLE_0;
					dmaBufferOffset++;
				}
			}
		}
	}
	else if(transponderTypeLocal == 0x1) {
		

		for (byteIndex = 0; byteIndex < TRANSPONDER_DATA_LENGTH; byteIndex++) {

			uint8_t byteToSend = *transponderData;
			transponderData++;
			for (bitIndex = 0; bitIndex < TRANSPONDER_BITS_PER_BYTE; bitIndex++)
			{
				bool doToggles = false;
				if (bitIndex == 0) {
					doToggles = true;
				}
				else if (bitIndex == TRANSPONDER_BITS_PER_BYTE - 1) {
					doToggles = false;
				}
				else {
					doToggles = byteToSend & (1 << (bitIndex - 1));
				}

				for (toggleIndex = 0; toggleIndex < TRANSPONDER_TOGGLES_PER_BIT; toggleIndex++)
				{
					if (doToggles) {
						transponderIrDMABuffer[dmaBufferOffset] = BIT_TOGGLE_1;
					}
					else {
						transponderIrDMABuffer[dmaBufferOffset] = BIT_TOGGLE_0;
					}
					dmaBufferOffset++;
				}
				transponderIrDMABuffer[dmaBufferOffset] = BIT_TOGGLE_0;
				dmaBufferOffset++;
			}
		}
	}
}

void transponderIrWaitForTransmitComplete(void)
{
    static uint32_t waitCounter = 0;

    while(transponderIrDataTransferInProgress) {
        waitCounter++;
    }
}

void transponderIrUpdateData(const uint8_t* transponderData, const uint8_t* transponderType)
{
    transponderIrWaitForTransmitComplete();

    updateTransponderDMABuffer(transponderData, transponderType);
}


void transponderIrTransmit(const uint8_t* transponderType) 
{
    transponderIrWaitForTransmitComplete();

    dmaBufferOffset = 0;

    transponderIrDataTransferInProgress = 1;
	transponderIrDMAEnable(transponderType); 
}
