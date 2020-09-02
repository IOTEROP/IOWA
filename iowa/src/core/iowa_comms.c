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
* Copyright (c) 2016-2019 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_core_internals.h"
#include "iowa_prv_lwm2m_internals.h"

/*************************************************************************************
** Internal functions
*************************************************************************************/

#ifdef LWM2M_CLIENT_MODE

void coreServerEventCallback(iowa_context_t contextP,
                             lwm2m_server_t *serverP,
                             iowa_event_type_t eventType)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with new event: %s", CORE_STR_EVENT_TYPE(eventType));

    if (contextP->eventCb != NULL)
    {
        iowa_event_t event;
        memset(&event, 0, sizeof(iowa_event_t));

        event.serverShortId = serverP->shortId;
        event.eventType = eventType;

        // Retrieve the server lifetime
        switch (eventType)
        {
        case IOWA_EVENT_REG_UNREGISTERED:
            break;

        default:
            event.details.registration.lifetime = serverP->lifetime;
        }

        CRIT_SECTION_LEAVE(contextP);
        contextP->eventCb(&event, contextP->userData, contextP);
        CRIT_SECTION_ENTER(contextP);
    }
}

void coreObservationEventCallback(iowa_context_t contextP,
                                  lwm2m_observed_t *targetP,
                                  iowa_event_type_t eventType)
{
    // WARNING: This function is called in a critical section
    IOWA_LOG_ARG_TRACE(IOWA_PART_LWM2M, "Entering with new event: %s", CORE_STR_EVENT_TYPE(eventType));

    if (contextP->eventCb != NULL)
    {
        iowa_event_t event;
        lwm2m_server_t *serverP;

        memset(&event, 0, sizeof(iowa_event_t));

        event.details.observation.maxPeriod = UINT32_MAX;
        event.details.observation.maxEvalPeriod = UINT32_MAX;
        event.eventType = eventType;

        for (serverP = contextP->lwm2mContextP->serverList; serverP != NULL; serverP = serverP->next)
        {
            lwm2m_observed_t *observedP;

            observedP = serverP->runtime.observedList;
            while (observedP != NULL)
            {
                if (observedP == targetP)
                {
                    event.serverShortId = serverP->shortId;
                    observedP = NULL;
                    serverP = NULL;
                }
                else
                {
                    observedP = observedP->next;
                }
            }
            if (serverP == NULL)
            {
                break;
            }
        }

        {
            size_t ind;
            ind = 0;

            if (targetP->timeAttrP != NULL
                && targetP->timeAttrP->flags != 0)
            {
                if ((targetP->timeAttrP->flags & LWM2M_ATTR_FLAG_MIN_PERIOD) != 0)
                {
                    event.details.observation.minPeriod = targetP->timeAttrP->minPeriod;
                }
                else
                {
                    event.details.observation.minPeriod = 0;
                }
                if ((targetP->timeAttrP->flags & LWM2M_ATTR_FLAG_MAX_PERIOD) != 0)
                {
                    event.details.observation.maxPeriod = targetP->timeAttrP->maxPeriod;
                }
                else
                {
                    event.details.observation.maxPeriod = UINT32_MAX;
                }
            }
            if (LWM2M_URI_IS_SET_INSTANCE(&(targetP->uriInfoP[ind].uri)))
            {
                event.details.observation.sensorId = OBJECT_INSTANCE_ID_TO_SENSOR(targetP->uriInfoP[ind].uri.objectId, targetP->uriInfoP[ind].uri.instanceId);
                CRIT_SECTION_LEAVE(contextP);
                contextP->eventCb(&event, contextP->userData, contextP);
                CRIT_SECTION_ENTER(contextP);
            }
            else
            {
                lwm2m_object_t *objectP;

                for (objectP = contextP->lwm2mContextP->objectList; objectP != NULL; objectP = objectP->next)
                {
                    if (objectP->objID == targetP->uriInfoP[ind].uri.objectId)
                    {
                        uint16_t instIndex;

                        for (instIndex = 0; instIndex < objectP->instanceCount; instIndex++)
                        {
                            event.details.observation.sensorId = OBJECT_INSTANCE_ID_TO_SENSOR(objectP->objID, objectP->instanceArray[instIndex].id);

                            CRIT_SECTION_LEAVE(contextP);
                            contextP->eventCb(&event, contextP->userData, contextP);
                            CRIT_SECTION_ENTER(contextP);
                        }
                        break;
                    }
                }
            }
        }
    }
}

#endif
