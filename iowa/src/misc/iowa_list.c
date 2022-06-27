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
* Copyright (c) 2017-2020 IoTerop.
* All rights reserved.
*
* This program and the accompanying materials
* are made available under the terms of
* IoTerop’s IOWA License (LICENSE.TXT) which
* accompany this distribution.
*
**********************************************/

#include "iowa_prv_misc.h"

/*************************************************************************************
** Internal functions
*************************************************************************************/

bool listFindCallbackBy16bitsId(void *nodeP,
                                void *criteriaP)
{
    return ((list_16_bits_id_t *)nodeP)->id == *((uint16_t *)criteriaP);
}

uint16_t listNew16bitsId(list_16_bits_id_t *headP)
{
    uint16_t newId;
    list_16_bits_id_t *nodeP;

    newId = 0;
    nodeP = headP;

    while (nodeP != NULL)
    {
        if (newId == nodeP->id)
        {
            newId++;
            nodeP = headP;
        }
        else
        {
            nodeP = nodeP->nextP;
        }
    }

    return newId;
}

bool listFindCallbackBy32bitsId(void *nodeP,
                                void *criteriaP)
{
    return ((list_32_bits_id_t *)nodeP)->id == *((uint32_t *)criteriaP);
}

uint32_t listNew32bitsId(list_32_bits_id_t *headP)
{
    uint32_t newId;
    list_32_bits_id_t *nodeP;

    newId = 0;
    nodeP = headP;

    while (nodeP != NULL)
    {
        if (newId == nodeP->id)
        {
            newId++;
            nodeP = headP;
        }
        else
        {
            nodeP = nodeP->nextP;
        }
    }

    return newId;
}

/*************************************************************************************
** Public functions
*************************************************************************************/

iowa_list_t * iowa_utils_list_add(iowa_list_t *headP,
                                  iowa_list_t *nodeP)
{
    if (nodeP == NULL)
    {
        IOWA_LOG_WARNING(IOWA_PART_SYSTEM, "Cannot add to the list. Node is nil.");
        return headP;
    }

    // Link the head as the next node of 'nodeP'
    nodeP->nextP = headP;

    // 'nodeP' is the new head of the list
    return nodeP;
}

iowa_list_t * iowa_utils_list_remove(iowa_list_t *headP,
                                     iowa_list_t *nodeP)
{
    iowa_list_t *parent;

    if (headP == NULL
        || nodeP == NULL)
    {
        IOWA_LOG_WARNING(IOWA_PART_SYSTEM, "Head and/or node is nil.");
        return headP;
    }

    if (nodeP == headP)
    {
        // Node to remove is the first one, return the next node of the list
        return headP->nextP;
    }

    // Look for the parent i.e. the node just before the one to remove
    parent = headP;
    while (parent->nextP != NULL
           && parent->nextP != nodeP)
    {
        parent = parent->nextP;
    }

    if (parent->nextP != NULL)
    {
        // Node was found, unlink it
        parent->nextP = nodeP->nextP;
    }

    // The head of the list stays the same
    return headP;
}

void iowa_utils_list_free(iowa_list_t *headP,
                          iowa_list_node_free_callback_t freeCb)
{
    assert(freeCb != NULL);

    while (headP != NULL)
    {
        iowa_list_t *nextNodeP;

        // Keep the next node
        nextNodeP = headP->nextP;

        // Free the first node
        freeCb(headP);

        // Continue with the rest of the list
        headP = nextNodeP;
    }
}

iowa_list_t * iowa_utils_list_find(iowa_list_t *headP,
                                   iowa_list_node_find_callback_t findCb,
                                   void *criteriaP)
{
    iowa_list_t *nodeP;

    assert(findCb != NULL);

    for (nodeP = headP; nodeP != NULL; nodeP = nodeP->nextP)
    {
        if (findCb(nodeP, criteriaP) == true)
        {
            break;
        }
    }

    return nodeP;
}

iowa_list_t * iowa_utils_list_find_and_remove(iowa_list_t *headP,
                                              iowa_list_node_find_callback_t findCb,
                                              void *criteriaP,
                                              iowa_list_t **nodeP)
{
    iowa_list_t *previousNodeP;
    iowa_list_t *currentNodeP;

    assert(findCb != NULL);

    if (headP == NULL)
    {
        IOWA_LOG_WARNING(IOWA_PART_SYSTEM, "No callback and/or head is nil.");

        if (nodeP != NULL)
        {
            *nodeP = NULL;
        }

        return headP;
    }

    previousNodeP = NULL;
    for (currentNodeP = headP; currentNodeP != NULL; currentNodeP = currentNodeP->nextP)
    {
        if (findCb(currentNodeP, criteriaP) == true)
        {
            break;
        }

        previousNodeP = currentNodeP;
    }

    if (nodeP != NULL)
    {
        // Keep the node found
        *nodeP = currentNodeP;
    }

    if (previousNodeP != NULL)
    {
        if (currentNodeP != NULL)
        {
            // Node was found, unlink it
            previousNodeP->nextP = currentNodeP->nextP;
        }
        else
        {
            // Node has not been found
            previousNodeP->nextP = NULL;
        }
    }
    else
    {
        // Node found was the head of the queue
        headP = headP->nextP;
    }

    return headP;
}
