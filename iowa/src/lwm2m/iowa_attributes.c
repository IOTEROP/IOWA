/**********************************************
*
*  _________ _________ ___________ _________
* |         |         |   |   |   |         |
* |_________|         |   |   |   |    _    |
* |         |    |    |   |   |   |         |
* |         |    |    |           |         |
* |         |    |    |           |    |    |
* |         |         |           |    |    |
* |_________|_________|___________|____|____|
*
* Copyright (c) 2018-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_lwm2m_internals.h"

#ifdef LWM2M_CLIENT_MODE
/*************************************************************************************
** Private functions
*************************************************************************************/

static void prv_attributesSetFromInheritance(attributes_t *destAttrP,
                                             attributes_t *sourceAttrP)
{
    if ((destAttrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) == 0
        && (sourceAttrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0)
    {
        destAttrP->minPeriod = sourceAttrP->minPeriod;
    }
    if ((destAttrP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD) == 0
        && (sourceAttrP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0)
    {
        destAttrP->maxPeriod = sourceAttrP->maxPeriod;
    }
    if ((destAttrP->flags & LWM2M_ATTR_FLAG_GREATER_THAN) == 0
        && (sourceAttrP->flags & LWM2M_ATTR_FLAG_GREATER_THAN) != 0)
    {
        destAttrP->greaterThan = sourceAttrP->greaterThan;
    }
    if ((destAttrP->flags & LWM2M_ATTR_FLAG_LESS_THAN) == 0
        && (sourceAttrP->flags & LWM2M_ATTR_FLAG_LESS_THAN) != 0)
    {
        destAttrP->lessThan = sourceAttrP->lessThan;
    }
    if ((destAttrP->flags & LWM2M_ATTR_FLAG_STEP) == 0
        && (sourceAttrP->flags & LWM2M_ATTR_FLAG_STEP) != 0)
    {
        destAttrP->step = sourceAttrP->step;
    }
    destAttrP->flags |= sourceAttrP->flags;
}

static int prv_attributesRetrieve(iowa_coap_option_t *optionP,
                                  attributes_t *attrP)
{
    uint8_t toClear;
    int64_t intValue;
    double floatValue;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Reading attributes.");

    toClear = 0;
    memset(attrP, 0, sizeof(attributes_t));

    while (optionP != NULL && optionP->number == IOWA_COAP_OPTION_URI_QUERY)
    {
        if (optionP->length > ATTR_MIN_PERIOD_LEN
            && strncmp((char *)optionP->value.asBuffer, ATTR_MIN_PERIOD_STR, ATTR_MIN_PERIOD_LEN) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_MIN_PERIOD))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            if (1 != dataUtilsBufferToInt(optionP->value.asBuffer + ATTR_MIN_PERIOD_LEN, (size_t)(optionP->length - ATTR_MIN_PERIOD_LEN), &intValue))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Convert buffer to int failed.");
                return -1;
            }
            if (intValue < 0 || intValue > UINT32_MAX)
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "intValue is not between 0 - UINT32_MAX.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Set pmin.");
            attrP->flags |= LWM2M_ATTR_FLAG_MIN_PERIOD;
            attrP->minPeriod = (uint32_t)intValue;
        }
        else if (optionP->length == ATTR_MIN_PERIOD_LEN - 1
                 && strncmp((char *)optionP->value.asBuffer, ATTR_MIN_PERIOD_STR, ATTR_MIN_PERIOD_LEN - 1) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_MIN_PERIOD))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Clear pmin.");
            toClear |= LWM2M_ATTR_FLAG_MIN_PERIOD;
        }
        else if (optionP->length > ATTR_MAX_PERIOD_LEN
                 && strncmp((char *)optionP->value.asBuffer, ATTR_MAX_PERIOD_STR, ATTR_MAX_PERIOD_LEN) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_MAX_PERIOD))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            if (1 != dataUtilsBufferToInt(optionP->value.asBuffer + ATTR_MAX_PERIOD_LEN, (size_t)(optionP->length - ATTR_MAX_PERIOD_LEN), &intValue))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Convert buffer to int failed.");
                return -1;
            }
            if (intValue < 0 || intValue > UINT32_MAX)
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "intValue is not between 0 - UINT32_MAX.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Set pmax.");
            attrP->flags |= LWM2M_ATTR_FLAG_MAX_PERIOD;
            attrP->maxPeriod = (uint32_t)intValue;
        }
        else if (optionP->length == ATTR_MAX_PERIOD_LEN - 1
                 && strncmp((char *)optionP->value.asBuffer, ATTR_MAX_PERIOD_STR, ATTR_MAX_PERIOD_LEN - 1) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_MAX_PERIOD))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Clear pmax.");
            toClear |= LWM2M_ATTR_FLAG_MAX_PERIOD;
        }
        else if (optionP->length > ATTR_GREATER_THAN_LEN
                 && strncmp((char *)optionP->value.asBuffer, ATTR_GREATER_THAN_STR, ATTR_GREATER_THAN_LEN) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_GREATER_THAN))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            if (1 != dataUtilsBufferToFloat(optionP->value.asBuffer + ATTR_GREATER_THAN_LEN, (size_t)(optionP->length - ATTR_GREATER_THAN_LEN), &floatValue))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Convert buffer to float failed.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Set gt.");
            attrP->flags |= LWM2M_ATTR_FLAG_GREATER_THAN;
            attrP->greaterThan = floatValue;
        }
        else if (optionP->length == ATTR_GREATER_THAN_LEN - 1
                 && strncmp((char *)optionP->value.asBuffer, ATTR_GREATER_THAN_STR, ATTR_GREATER_THAN_LEN - 1) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_GREATER_THAN))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Clear gt.");
            toClear |= LWM2M_ATTR_FLAG_GREATER_THAN;
        }
        else if (optionP->length > ATTR_LESS_THAN_LEN
                 && strncmp((char *)optionP->value.asBuffer, ATTR_LESS_THAN_STR, ATTR_LESS_THAN_LEN) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_LESS_THAN))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            if (1 != dataUtilsBufferToFloat(optionP->value.asBuffer + ATTR_LESS_THAN_LEN, (size_t)(optionP->length - ATTR_LESS_THAN_LEN), &floatValue))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Convert buffer to float failed.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Set lt.");
            attrP->flags |= LWM2M_ATTR_FLAG_LESS_THAN;
            attrP->lessThan = floatValue;
        }
        else if (optionP->length == ATTR_LESS_THAN_LEN - 1
                 && strncmp((char *)optionP->value.asBuffer, ATTR_LESS_THAN_STR, ATTR_LESS_THAN_LEN - 1) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_LESS_THAN))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Clear lt.");
            toClear |= LWM2M_ATTR_FLAG_LESS_THAN;
        }
        else if (optionP->length > ATTR_STEP_LEN
                 && strncmp((char *)optionP->value.asBuffer, ATTR_STEP_STR, ATTR_STEP_LEN) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_STEP))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            if (1 != dataUtilsBufferToFloat(optionP->value.asBuffer + ATTR_STEP_LEN, (size_t)(optionP->length - ATTR_STEP_LEN), &floatValue))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Convert buffer to float failed.");
                return -1;
            }
            if (floatValue < 0)
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "floatValue is not superior to 0.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Set st.");
            attrP->flags |= LWM2M_ATTR_FLAG_STEP;
            attrP->step = floatValue;
        }
        else if (optionP->length == ATTR_STEP_LEN - 1
                 && strncmp((char *)optionP->value.asBuffer, ATTR_STEP_STR, ATTR_STEP_LEN - 1) == 0)
        {
            if (0 != ((attrP->flags | toClear) & LWM2M_ATTR_FLAG_STEP))
            {
                IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Both clear and set is forbidden.");
                return -1;
            }
            IOWA_LOG_INFO(IOWA_PART_LWM2M, "Clear st.");
            toClear |= LWM2M_ATTR_FLAG_STEP;
        }
        else
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Invalid attribute.");
            return -1;
        }

        optionP = optionP->next;
    }

    return toClear;
}

/*************************************************************************************
** Internal functions
*************************************************************************************/

iowa_status_t attributesWrite(iowa_context_t contextP,
                              iowa_lwm2m_uri_t *uriP,
                              lwm2m_server_t *serverP,
                              iowa_coap_option_t *optionP)
{
    int retrieveResult;
    uint8_t toClear;
    attributes_t readAttr;
    attributes_t *attributesP;
    iowa_status_t result;

    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Writing attributes.");

    result = object_checkReadable(contextP, serverP->shortId, uriP);
    if (IOWA_COAP_205_CONTENT != result)
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Object check is not readable.");
        return result;
    }

    retrieveResult = prv_attributesRetrieve(optionP, &readAttr);
    if (retrieveResult < 0)
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Failed to retrieve attributes.");
        return IOWA_COAP_400_BAD_REQUEST;
    }
    toClear = (uint8_t)retrieveResult;

    attributesP = serverP->runtime.attributesList;
    while (attributesP != NULL)
    {
        if (LWM2M_URI_ARE_EQUAL(&attributesP->uri, uriP))
        {
            // Found existing attributes
            break;
        }

        attributesP = attributesP->nextP;
    }
    if (attributesP != NULL)
    {

        // Clear attributes
        attributesP->flags &= (uint8_t)(~toClear);

        if (attributesP->flags == 0
            && readAttr.flags == 0)
        {
            // Remove the attributes if it's empty
            serverP->runtime.attributesList = (attributes_t *)IOWA_UTILS_LIST_REMOVE(serverP->runtime.attributesList, attributesP);

            iowa_system_free(attributesP);
            return IOWA_COAP_NO_ERROR;
        }

        if ((attributesP->flags & LWM2M_ATTR_FLAG_LESS_THAN) != 0
            && (readAttr.flags & LWM2M_ATTR_FLAG_LESS_THAN) == 0)
        {
            readAttr.lessThan = attributesP->lessThan;
            readAttr.flags |= LWM2M_ATTR_FLAG_LESS_THAN;
        }
        if ((attributesP->flags & LWM2M_ATTR_FLAG_GREATER_THAN) != 0
            && (readAttr.flags & LWM2M_ATTR_FLAG_GREATER_THAN) == 0)
        {
            readAttr.greaterThan = attributesP->greaterThan;
            readAttr.flags |= LWM2M_ATTR_FLAG_GREATER_THAN;
        }
        if ((attributesP->flags & LWM2M_ATTR_FLAG_STEP) != 0
            && (readAttr.flags & LWM2M_ATTR_FLAG_STEP) == 0)
        {
            readAttr.step = attributesP->step;
            readAttr.flags |= LWM2M_ATTR_FLAG_STEP;
        }
        if ((attributesP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0
            && (readAttr.flags & LWM2M_ATTR_FLAG_MIN_PERIOD) == 0)
        {
            readAttr.minPeriod = attributesP->minPeriod;
            readAttr.flags |= LWM2M_ATTR_FLAG_MIN_PERIOD;
        }
    }
    // Check Attributes Rules
    if ((readAttr.flags & ATTR_FLAG_NUMERIC) != 0)
    {
        iowa_lwm2m_data_type_t type;
        if (!LWM2M_URI_IS_SET_RESOURCE(uriP))
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "'lt', 'gt' and 'st' must be set at resource level.");
            return IOWA_COAP_405_METHOD_NOT_ALLOWED;
        }

        // Check resource type
        type = object_getResourceType(uriP->objectId, uriP->resourceId, contextP);
        if (type != IOWA_LWM2M_TYPE_INTEGER
            && type != IOWA_LWM2M_TYPE_FLOAT
            && type != IOWA_LWM2M_TYPE_TIME)
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "'lt', 'gt' and 'st' must be set at numeric resource.");
            return IOWA_COAP_405_METHOD_NOT_ALLOWED;
        }

        // Check rule: 'lt' value + 2*'st' value < 'gt' value
        if ((readAttr.flags & LWM2M_ATTR_FLAG_LESS_THAN) != 0
            && (readAttr.flags & LWM2M_ATTR_FLAG_GREATER_THAN) != 0
            && readAttr.lessThan + (2 * readAttr.step) >= readAttr.greaterThan)
        {
            IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Invalid observe parameters. Must respect: 'lt' value + 2*'st' value < 'gt' value.");
            return IOWA_COAP_400_BAD_REQUEST;
        }
    }

    if ((readAttr.flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0
        && (readAttr.flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0
        && (readAttr.minPeriod > readAttr.maxPeriod))
    {
        IOWA_LOG_WARNING(IOWA_PART_LWM2M, "Invalid observe parameters. Must respect: 'pmax' value > 'pmin' value.");
        return IOWA_COAP_400_BAD_REQUEST;
    }

    if (attributesP != NULL)
    {
        // Set attributes
        attributesP->flags |= readAttr.flags;
        if ((readAttr.flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0)
        {
            attributesP->minPeriod = readAttr.minPeriod;
        }
        if ((readAttr.flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0)
        {
            attributesP->maxPeriod = readAttr.maxPeriod;
        }
        if ((readAttr.flags & LWM2M_ATTR_FLAG_GREATER_THAN) != 0)
        {
            attributesP->greaterThan = readAttr.greaterThan;
        }
        if ((readAttr.flags & LWM2M_ATTR_FLAG_LESS_THAN) != 0)
        {
            attributesP->lessThan = readAttr.lessThan;
        }
        if ((readAttr.flags & LWM2M_ATTR_FLAG_STEP) != 0)
        {
            attributesP->step = readAttr.step;
        }
    }
    else
    {
        attributes_t *newAttributesP;

        newAttributesP = (attributes_t *)iowa_system_malloc(sizeof(attributes_t));
#ifndef IOWA_CONFIG_SKIP_SYSTEM_FUNCTION_CHECK
        if (newAttributesP == NULL)
        {
            IOWA_LOG_ERROR_MALLOC(sizeof(attributes_t));
            return IOWA_COAP_500_INTERNAL_SERVER_ERROR;
        }
#endif
        // Copy the flags
        memcpy(newAttributesP, &readAttr, sizeof(attributes_t));

        newAttributesP->nextP = NULL;
        memcpy(&newAttributesP->uri, uriP, sizeof(iowa_lwm2m_uri_t));
        uriP->resInstanceId = IOWA_LWM2M_ID_ALL;

        // Add the attributes
        serverP->runtime.attributesList = (attributes_t *)IOWA_UTILS_LIST_ADD(serverP->runtime.attributesList, newAttributesP);
    }

    return IOWA_COAP_NO_ERROR;
}

bool attributesGet(lwm2m_server_t *serverP,
                   iowa_lwm2m_uri_t *uriP,
                   attributes_t *attrP,
                   bool useInheritance,
                   bool getDefault)
{
    attributes_t *attributesP;
    attributes_t *objAttrP;
    attributes_t *instAttrP;
    attributes_t *resAttrP;

#ifndef IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS
    (void)getDefault;
#endif
    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Getting attributes.");

    // Retrieve the attributes for object, instance and resource levels
    attributesP = serverP->runtime.attributesList;
    objAttrP = NULL;
    instAttrP = NULL;
    resAttrP = NULL;
    while (attributesP != NULL)
    {
        if (useInheritance == false)
        {
            if (LWM2M_URI_ARE_EQUAL(uriP, &attributesP->uri))
            {
                break;
            }
        }
        else
        {
            if (uriP->objectId == attributesP->uri.objectId)
            {
                if (LWM2M_URI_IS_SET_INSTANCE(&attributesP->uri)
                    && uriP->instanceId == attributesP->uri.instanceId)
                {
                    if (LWM2M_URI_IS_SET_RESOURCE(&attributesP->uri)
                        && uriP->resourceId == attributesP->uri.resourceId)
                    {
                        {
                            resAttrP = attributesP;
                        }
                    }
                    else if (!LWM2M_URI_IS_SET_RESOURCE(&attributesP->uri))
                    {
                        instAttrP = attributesP;
                    }
                }
                else if (!LWM2M_URI_IS_SET_INSTANCE(&attributesP->uri))
                {
                    objAttrP = attributesP;
                }
            }
        }
        attributesP = attributesP->nextP;
    }

    // Set the attributes
    memset(attrP, 0, sizeof(attributes_t));
    if (useInheritance == true)
    {
        if (resAttrP != NULL)
        {
            prv_attributesSetFromInheritance(attrP, resAttrP);
        }
        if (instAttrP != NULL)
        {
            prv_attributesSetFromInheritance(attrP, instAttrP);
        }
        if (objAttrP != NULL)
        {
            prv_attributesSetFromInheritance(attrP, objAttrP);
        }
    }
    else if (attributesP != NULL)
    {
        prv_attributesSetFromInheritance(attrP, attributesP);
    }

    // Check attributes
    if ((attrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0
        && (attrP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0
        && attrP->maxPeriod < attrP->minPeriod)
    {
        // Ignore Maximum Period if smaller than Minimum Period
        attrP->flags &= (uint8_t)(~LWM2M_ATTR_FLAG_MAX_PERIOD);
    }

#ifdef IOWA_SERVER_SUPPORT_RSC_DEFAULT_PERIODS
    if (getDefault)
    {
        if ((attrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) == 0
            && serverP->defaultPmin != 0)
        {
            attrP->flags |= LWM2M_ATTR_FLAG_MIN_PERIOD;
            attrP->minPeriod = serverP->defaultPmin;
        }
        if ((attrP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD) == 0
            && serverP->defaultPmax != PMAX_UNSET_VALUE)
        {
            attrP->flags |= LWM2M_ATTR_FLAG_MAX_PERIOD;
            attrP->maxPeriod = serverP->defaultPmax;
        }
    }
#endif

    return attrP->flags != 0;
}

void attributesRemoveFromServer(lwm2m_server_t *serverP)
{
    IOWA_LOG_TRACE(IOWA_PART_LWM2M, "Clearing attributes list.");

    IOWA_UTILS_LIST_FREE(serverP->runtime.attributesList, iowa_system_free);
    serverP->runtime.attributesList = NULL;
}

#endif // LWM2M_CLIENT_MODE
