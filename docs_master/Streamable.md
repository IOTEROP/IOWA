# Custom Object Streaming APIs

Let's consider a proprietary LwM2M Object presenting Device generated logs with the following resources:

| Resource Name  | Resource ID | Access Type | Mandatory | Type    | Description                           |
| -------------- | ----------- | ----------- | --------- | ------- | ------------------------------------- |
| Logs Available | 0           | R           | Mandatory | Boolean | *True* means some logs are available. |
| Logs           | 1           | R           | Mandatory | String  | The logs.                             |

The logs can be very large and thus needs to be retrieved by blocks.

## Object Declaration

When declaring the Object, inform IOWA that the resource with ID 1 can be retrieved by blocks:

```c
iowa_lwm2m_resource_desc_t resources[2] =
  { {0, IOWA_LWM2M_TYPE_BOOLEAN, IOWA_DM_READ, IOWA_RESOURCE_FLAG_MANDATORY},
    {1, IOWA_LWM2M_TYPE_STRING, IOWA_DM_READ, IOWA_RESOURCE_FLAG_MANDATORY | IOWA_RESOURCE_FLAG_STREAMABLE} };
```

## Callback Implementation

In *iowa.h*, the `iowa_lwm2m_data_t` structure is expanded to to be able contain a block of a string, opaque, or CoRE Link value:

```c
typedef uint8_t iowa_lwm2m_data_type_t;

#define IOWA_LWM2M_TYPE_UNDEFINED         0
#define IOWA_LWM2M_TYPE_STRING            1
#define IOWA_LWM2M_TYPE_OPAQUE            2
#define IOWA_LWM2M_TYPE_INTEGER           3
#define IOWA_LWM2M_TYPE_FLOAT             4
#define IOWA_LWM2M_TYPE_BOOLEAN           5
#define IOWA_LWM2M_TYPE_CORE_LINK         6
#define IOWA_LWM2M_TYPE_OBJECT_LINK       7
#define IOWA_LWM2M_TYPE_TIME              8
#define IOWA_LWM2M_TYPE_UNSIGNED_INTEGER  9
#define IOWA_LWM2M_TYPE_STRING_BLOCK      101
#define IOWA_LWM2M_TYPE_OPAQUE_BLOCK      102
#define IOWA_LWM2M_TYPE_CORE_LINK_BLOCK   106

typedef struct
{
    uint16_t objectID;
    uint16_t instanceID;
    uint16_t resourceID;
    uint16_t resInstanceID;
    iowa_lwm2m_data_type_t type;
    union
    {
        bool    asBoolean;
        int64_t asInteger;
        double  asFloat;
        struct
        {
            size_t   length;
            uint8_t *buffer;
        } asBuffer;
        struct
        {
            uint32_t details;
            uint8_t *buffer;
        } asBlock;
        iowa_lwm2m_object_link_t asObjLink;
    } value;
    int32_t timestamp;
} iowa_lwm2m_data_t;

```

`iowa_lwm2m_data_t::value::asBlock::buffer` contains the block of data.

`iowa_lwm2m_data_t::value::asBlock::details` contains the information of the block. It is to be treated as an opaque type and must be accessed only with the helper functions below:

```c
// Get block information from an iowa_lwm2m_data_t.
// Returned value: IOWA_COAP_NO_ERROR in case of success or IOWA_COAP_404_NOT_FOUND if there are no block information to retrieve.
// Parameters:
// - dataP: the iowa_lwm2m_data_t to retrieve the block info from.
// - numberP: OUT. the block number.
// - moreP: OUT. true if there are more blocks coming.
// - sizeP: OUT. the size of the block.
iowa_status_t iowa_data_get_block_info(iowa_lwm2m_data_t *dataP,
                                       uint16_t *numberP,
                                       bool *moreP,
                                       uint16_t *sizeP);

// Set block information of an iowa_lwm2m_data_t.
// Returned value: IOWA_COAP_NO_ERROR in case of success or an error status.
// Parameters:
// - dataP: the iowa_lwm2m_data_t to set the info to.
// - number: the block number.
// - more: true if there are more blocks coming.
// - size: the size of the block. It must be inferior to 1024.
iowa_status_t iowa_data_set_block_info(iowa_lwm2m_data_t *dataP,
                                       uint16_t number,
                                       bool more,
                                       uint16_t size);

```

When calling `iowa_data_set_block_info`, if *more* is set to true, due to the CoAP Block format used on the wire, *size* can only have one of the following values:

```c
#define IOWA_DATA_BLOCK_SIZE_16    16
#define IOWA_DATA_BLOCK_SIZE_32    32
#define IOWA_DATA_BLOCK_SIZE_64    64
#define IOWA_DATA_BLOCK_SIZE_128   128
#define IOWA_DATA_BLOCK_SIZE_256   256
#define IOWA_DATA_BLOCK_SIZE_512   512
#define IOWA_DATA_BLOCK_SIZE_1024  1024
```

On a READ on resource 1, the Object callback can return the data by block by doing:

```c
iowa_status_t objectCallback(iowa_dm_operation_t operation,
                             iowa_lwm2m_data_t *dataP,
                             size_t numData,
                             void *userData,
                             iowa_context_t contextP)
{
    size_t i;
    my_application_data_t *appDataP;
    size_t readLength;
    uint8_t *buffer;

    appDataP = (my_application_data_t *)userData;

    switch (operation)
    {
    case IOWA_DM_READ:
        for (i = 0 ; i < numData ; i++)
        {
            switch (dataP[i].resourceID)
            {
            case 0:
                dataP[i].value.asBoolean = appDataP->hasLogs;
                break;

            case 1:
                // Here we read the data from a file by chunks of 512 bytes
                buffer = iowa_system_malloc(IOWA_DATA_BLOCK_SIZE_512);

                readLength = fread(buffer, 1,
                                   IOWA_DATA_BLOCK_SIZE_512, appDataP->logFile);
                if (feof(appDataP->logFile) == 0)
                {
                    // There are more data to read, we use block
                    dataP[i].value.asBlock.buffer = buffer;
                    iowa_data_set_block_info(dataP + i, 0, true, readLength);
                }
                else
                {
                    // There are less than 512 bytes of data
                    dataP[i].value.asBuffer.buffer = buffer;
                    dataP[i].value.asBuffer.length = readLength;
                }
                break;

            default:
                // Should not happen
                break;
            }
        }
        break;

    case IOWA_DM_FREE:
        for (i = 0 ; i < numData ; i++)
        {
            if (dataP[i].resourceID == 1)
            {
                if (dataP[i].type == IOWA_LWM2M_TYPE_STRING_BLOCK)
                {
                    iowa_system_free(dataP[i].value.asBlock.buffer);
                }
                else
                {
                    iowa_system_free(dataP[i].value.asBuffer.buffer);
                }
            }
        }
        break;

(...)
```

The code above works only for the first block.

When IOWA receives from the Object callback a `iowa_lwm2m_data_t` with the type flag **IOWA_LWM2M_TYPE_\*\*\*_BLOCK** set, if the *more* info is set to true, it calls again the callback to retrieve the next block.

For the callback to determine which block is requested, IOWA sets the `iowa_lwm2m_data_t::value::asBlock::details`. A correct Object callback is:

```c
iowa_status_t objectCallback(iowa_dm_operation_t operation,
                             iowa_lwm2m_data_t *dataP,
                             size_t numData,
                             void *userData,
                             iowa_context_t contextP)
{
    size_t i;
    my_application_data_t *appDataP;
    size_t readLength;
    uint8_t *buffer;
    iowa_status_t result;
    bool more;
    uint32_t blockNumber;
    uint16_t blockSize;

    appDataP = (my_application_data_t *)userData;

    switch (operation)
    {
    case IOWA_DM_READ:
        for (i = 0 ; i < numData ; i++)
        {
            switch (dataP[i].resourceID)
            {
            case 0:
                dataP[i].value.asBoolean = appDataP->hasLogs;
                break;

            case 1:
                // Determine if this is the initial read or a request for a next block
                // Note that the received more value has no meaning for a Read.

                if (IOWA_COAP_NO_ERROR == iowa_data_get_block_info(dataP + i,
                                                                   &blockNumber,
                                                                   &more,
                                                                   &blockSize))
                {
                    size_t index;

                    // Note that the received more value has no meaning for a Read.

                    // *blockSize* bytes are to be read.
                    // Note that the block size may be different from the one we chose in the initial read.
                    buffer = iowa_system_malloc(blockSize);

                    // Compute the index of the data to read
                    index = blockSize * blockNumber;

                    // Read the data from the file at the index.
                    fseek(appDataP->logFile, index, SEEK_SET);
                    readLength = fread(buffer, 1,
                                       blockSize, appDataP->logFile);

                    if (feof(appDataP->logFile) == 0)
                    {
                        // There are more data to read
                        more = true;
                    }
                    else
                    {
                        more = false;
                    }

                    // Set the info
                    dataP[i].value.asBlock.buffer = buffer;
                    iowa_data_set_block_info(dataP + i,
                                             blockNumber,
                                             more,
                                             readLength);
                }
                else
                {
                    // This is an initial read, do as in the first example
                    // Here we read the data from a file by chunks of 512 bytes
                    buffer = iowa_system_malloc(IOWA_DATA_BLOCK_SIZE_512);

                    readLength = fread(buffer, 1,
                                       IOWA_DATA_BLOCK_SIZE_512, appDataP->logFile);
                    if (feof(appDataP->logFile) == 0)
                    {
                        // There are more data to read, we use block
                        dataP[i].value.asBlock.buffer = buffer;
                        iowa_data_set_block_info(dataP + i, 0, true, readLength);
                    }
                    else
                    {
                        // There are less than 512 bytes of data
                        dataP[i].value.asBuffer.buffer = buffer;
                        dataP[i].value.asBuffer.length = readLength;
                    }
                }
                break;

            default:
                // Should not happen
                break;
            }
        }
        break;

    case IOWA_DM_FREE:
        for (i = 0 ; i < numData ; i++)
        {
            if (dataP[i].resourceID == 1)
            {
                if (dataP[i].type == IOWA_LWM2M_TYPE_STRING_BLOCK)
                {
                    iowa_system_free(dataP[i].value.asBlock.buffer);
                }
                else
                {
                    iowa_system_free(dataP[i].value.asBuffer.buffer);
                }
            }
        }
        break;

        (...)
```

Note that depending on the LwM2M Server behavior or the transport MTU, the requested block size may vary.

Note also that the blocks may be read out of order.

### Limitations

As the resource value is retrieved by blocks, it is not possible to encode it in a multiple-resource format like SenML. Thus the Text, Opaque or CoRE-Link data formats will be used.

If the LwM2M Server performs a Read on an Object or Object Instance containing a resource returned by blocks, the response will not include the streamable resource.

To retrieve the value of the resource, the Server must perform a Read on the resource URI.

For the same reasons, it is not possible to add timestamps to a streamable resource.

A streamable resource cannot be asynchronous.
