/**********************************************
 *
 * Copyright (c) 2016-2020 IoTerop.
 * All rights reserved.
 *
 * This program and the accompanying materials
 * are made available under the terms of
 * IoTerop’s IOWA License (LICENSE.TXT) which
 * accompany this distribution.
 *
 **********************************************/

/**************************************************
 *
 * This is the implementation of a simple custom
 * LwM2M Object.
 *
 **************************************************/

// Platform specific headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Header file containing the definition of the sample custom Object
#include "sample_streamable_client.h"

iowa_status_t sample_object_dataCallback(iowa_dm_operation_t operation,
                                         iowa_lwm2m_data_t *dataP,
                                         size_t numData,
                                         void *userData,
                                         iowa_context_t iowaH)
{
    sample_object_values_t *objectValuesP;
    size_t i;

    // Retrieve our Object values
    objectValuesP = (sample_object_values_t *)userData;

    for (i = 0; i < numData; i++)
    {
        switch (dataP[i].resourceID)
        {
        case 5500:
            if (operation == IOWA_DM_READ)
            {
                dataP[i].value.asBoolean = objectValuesP->booleanValue;
            }
            break;

        case 5503:
           if (operation == IOWA_DM_READ)
            {
                dataP[i].value.asInteger = objectValuesP->integerValue;
            }
            else if (operation == IOWA_DM_WRITE)
            {
                objectValuesP->integerValue = dataP[i].value.asInteger;
            }
            break;

        case 5750:
        {
            if (operation == IOWA_DM_READ)
            {
                FILE *fileP;
                uint8_t *bufferP;
                size_t readLength;
                iowa_status_t result;
                bool more;
                uint32_t blockNumber;
                uint16_t blockSize;
                size_t fileIndex;

                fileP = objectValuesP->fileP;

                // Determine if this is the initial read or a request for the next piece
                // Note that the received more value has no meaning for a Read.
                result = iowa_data_get_block_info(dataP + i, &blockNumber, &more, &blockSize);
                if (result == IOWA_COAP_404_NOT_FOUND)
                {
                    // This is an initial read
                    blockNumber = 0;
                    blockSize = IOWA_DATA_BLOCK_SIZE_256;
                    fseek(fileP, 0, SEEK_SET);
                }
                else if (result != IOWA_COAP_NO_ERROR)
                {
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }

                // *blockSize* bytes are to be read.
                // Note that the piece size may be different from the one we chose in the initial read.
                bufferP = malloc(blockSize);
                if (bufferP == NULL)
                {
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }

                // Compute the index of the data to read
                fileIndex = blockSize * blockNumber;
                fseek(fileP, fileIndex, SEEK_SET);

                // Read the data from the file at the index.
                readLength = fread(bufferP, 1, blockSize, fileP);
                if (readLength == 0)
                {
                    return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                }

                if (feof(fileP) == 0)
                {
                    // There are more data to read
                    more = true;
                }
                else
                {
                    more = false;
                }

                if (result == IOWA_COAP_404_NOT_FOUND
                    && more == false)
                {
                    // This an initial READ and there are less than 512 bytes of data
                    dataP[i].value.asBuffer.length = readLength;
                    dataP[i].value.asBuffer.buffer = bufferP;
                }
                else
                {
                    // Set the piece info
                    result = iowa_data_set_block_info(dataP + i, blockNumber, more, readLength);
                    if (result != IOWA_COAP_NO_ERROR)
                    {
                        free(bufferP);
                        return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
                    }
                    dataP[i].value.asBlock.buffer = bufferP;
                }
                break;
            }
            else if(operation == IOWA_DM_FREE)
            {
                // Free the allocated memory during read operation
                if (dataP[i].type == IOWA_LWM2M_TYPE_STRING)
                {
                    free(dataP[i].value.asBuffer.buffer);
                }
                else if (dataP[i].type == IOWA_LWM2M_TYPE_STRING_BLOCK)
                {
                    free(dataP[i].value.asBlock.buffer);
                }
            }
        }

        default:
            // Should not happen
            return IOWA_COAP_404_NOT_FOUND;
        }
    }

    return IOWA_COAP_NO_ERROR;
}
