# Custom Object

IOWA abstracts lot of objects for the LwM2M Client through dedicated API. But additional object can be provided to the stack by using the following functions:

- [`iowa_client_add_custom_object`](ClientAPI.md#iowa_client_add_custom_object)
- [`iowa_client_remove_custom_object`](ClientAPI.md#iowa_client_remove_custom_object)
- [`iowa_client_object_resource_changed`](ClientAPI.md#iowa_client_object_resource_changed)
- [`iowa_client_object_instance_changed`](ClientAPI.md#iowa_client_object_instance_changed)

Below some examples described how to use these APIs to implement your objects.

## Controlling a Power Switch

Let’s consider that the device containing the IOWA Client is connected to a power switch. You want to be able to monitor and control this power switch from a LwM2M Server.

The first step is to determine the best layout of the LwM2M Object describing the power switch. Fortunately, the IPSO Alliance defined the Power Control Object (ID: 3312) which suits our needs. By re-using a standardized Object, we do not need to provide the LwM2M Servers with the layout of our Object. A server already knows the Object 3312 which contains the following resources:

| Resource Name           | Resource ID | Access Type | Mandatory | Type    | Description                                                                                    |
| ----------------------- | ----------- | ----------- | --------- | ------- | ---------------------------------------------------------------------------------------------- |
| On/Off                  | 5850        | R, W        | Mandatory | Boolean | On/off control, 0=OFF, 1=ON.                                                                   |
| Dimmer                  | 5851        | R, W        | Optional  | Integer | Proportional control, integer value between 0 and 100 as a percentage.                         |
| On Time                 | 5852        | R, W        | Optional  | Integer | The time in seconds that the power relay has been on. Writing a value of 0 resets the counter. |
| Cumulative active power | 5805        | R           | Optional  | Float   | The cumulative active power since the last cumulative energy reset or device start.            |
| Power factor            | 5820        | R           | Optional  | Float   | The power factor of the load.                                                                  |
| Application Type        | 5750        | R, W        | Optional  | String  | The application type of the sensor or actuator as a string, for instance, "Air Pressure".      |

The On/Off resource is mandatory to implement and is used to both control and monitor our power switch.

The Dimmer resource is not relevant to our use case. As it is optional, we do not implement it.

The On Time is optional but interesting for this example unlike the Cumulative active power, Power factor and Application Type resources.

To implement this Object in our device, in our main(), after initializing the IOWA context (see above), we declare an array describing the two resources:

```c
iowa_lwm2m_resource_desc_t resources[2] =
  { {5850, IOWA_LWM2M_TYPE_BOOLEAN, IOWA_DM_READ | IOWA_DM_WRITE, IOWA_RESOURCE_FLAG_MANDATORY},
    {5852, IOWA_LWM2M_TYPE_INTEGER, IOWA_DM_READ | IOWA_DM_WRITE, IOWA_RESOURCE_FLAG_OPTIONAL} };
```

Note that the header file "iowa_IPSO_ID.h" in the "include" folder contains defines for the ID, type and operations of the Reusable Resources defined by the IPSO Alliance. Using these macros, we can write the equivalent code as:

```c
iowa_lwm2m_resource_desc_t resources[2] =
  { {IPSO_RSC_ID_ON_OFF, IPSO_RSC_TYPE_ON_OFF, IPSO_RSC_OP_ON_OFF, IOWA_RESOURCE_FLAG_MANDATORY},
    {IPSO_RSC_ID_ON_TIME, IPSO_RSC_TYPE_ON_TIME, IPSO_RSC_OP_ON_TIME, IOWA_RESOURCE_FLAG_OPTIONAL} };
```

Instance-wise, we only have one power switch to control and it makes no sense that the LwM2M Server could create new instances or delete some and the object does not used resources that can be multiple. The *instanceCallback* and the *resInstanceCallback* can be omitted and we declare a single instance with ID 0:

```c
uint16_t singleInstanceID[1] = { 0 };

result = iowa_client_add_custom_object(contextP,
                                       3312,
                                       1, singleInstanceID,
                                       2, resources,
                                       powerSwitchCallback, NULL, NULL,
                                       NULL);
```

Now the Power Control Object is presented to the LwM2M Servers by the IOWA stack.

When a Server performs a command on one of its resources (or the whole Object), the `powerSwitchCallback` function is called.

Let’s assume our device provides two functions `switch_on()` and `switch_off()` to control the power switch and three interruptions to handle:

- `SWITCH_ON` when the power switch is set to On,
- `SWITCH_OFF` when the power switch is set to Off
- `TIMER` when a second had elapsed.

We first define two global variables to store the power switch state:

```c
bool switchState;
unsigned int onTime;
```

In the interrupt handler, we modify these variables to be in sync with the actual power switch state (pseudo-code):

```c
switch (interrupt)
{
(...)
case SWITCH_ON:
    switchState = true;
    break;

case SWITCH_OFF:
    switchState = false;
    break;

case TIMER:
    if (switchState == true)
    {
        onTime++;
    }
    break;
(...)
}
```

Now the `powerSwitchCallback` function can be implemented:

```c
iowa_status_t powerSwitchCallback(iowa_dm_operation_t operation,
                                  iowa_lwm2m_data_t *dataP,
                                  size_t numData,
                                  void *userData,
                                  iowa_context_t iowaH)
{
    size_t i;

    switch (operation)
    {
    case IOWA_DM_READ:
        for (i = 0; i < numData; i++)
        {
            switch (dataP[i].resourceID)
            {
            case 5850:
                dataP[i].value.asBoolean = switchState;
                break;

            case 5852:
                dataP[i].value.asInteger = onTime;
                break;

            default:
                // Already handled by IOWA stack
                break;
            }
        }
        break;

    case IOWA_DM_WRITE:
        for (i = 0; i < numData; i++)
        {
            switch (dataP[i].resourceID)
            {
            case 5850:
                if (dataP[i].value.asBoolean == true)
                {
                    switch_on();
                }
                else
                {
                    switch_off();
                }
                break;

            case 5852:
                if (dataP[i].value.asInteger == 0)
                {
                    onTime = 0;
                }
                else
                {
                    return IOWA_COAP_406_NOT_ACCEPTABLE;
                }
                break;

            default:
                // Already handled by IOWA stack
                break;
            }
        }
        break;

    default:
        // Already handled by IOWA stack
        break;
    }

    return IOWA_COAP_NO_ERROR;
}
```

Here we assume that the `switch_on()` and `switch_off()` functions will trigger an interrupt and the `switchState` variable is updated by the interrupt handler.

A LwM2M Server can write only `0` as the new value of the On Time resource hence the check.

If you test this code, you will notice that you can control the power switch remotely and check its state. However the LwM2M Server will not receive notifications when observing the Object resources.

To correct this, we need to inform the IOWA stack of the changes not triggered by a LwM2M Server. This is done by calling the API [`iowa_client_object_resource_changed()`](ClientAPI.md#iowa_client_object_resource_changed) whenever a change occurs.

A correct interrupt handler is then:

```c
switch (interrupt)
{
(...)
case SWITCH_ON:
    if (switchState == false)
    {
        iowa_client_object_resource_changed(contextP, 3312, 0, 5850);
    }
    switchState = true;
    break;

case SWITCH_OFF:
    if (switchState == true)
    {
        iowa_client_object_resource_changed(contextP, 3312, 0, 5850);
    }
    switchState = false;
    break;

case TIMER:
    if (switchState == true)
    {
        onTime++;
        iowa_client_notification_lock(contextP, true);
        iowa_client_object_resource_changed(contextP, 3312, 0, 5852);
        iowa_client_notification_lock(contextP, false);
    }
    break;
(...)
}
```

Note that the calls to [`iowa_client_notification_lock()`](ClientAPI.md#iowa_client_notification_lock) are not necessary but are here as an example.

/clearpage

## Monitoring the temperature during a period

Let’s consider that the device containing the IOWA Client is connected to a temperature sensor. You want to be able to monitor this sensor from a LwM2M Server. Monitored value will be reported with the time at which the value was taken.

The following example can be implemented with [`iowa_client_IPSO_add_sensor`](ClientAPI.md#iowa_client_IPSO_add_sensor) and [`iowa_client_IPSO_update_values`](ClientAPI.md#iowa_client_IPSO_update_values). But we will instead implement our own Custom Object to understand how monitored values with timestamp can be sent to a LwM2M Server.

The first step is to determine the best layout of the LwM2M Object describing the temperature sensor. Fortunately, the IPSO Alliance defined the Temperature Object (ID: 3303) which suits our needs. By re-using a standardized Object, we do not need to provide the LwM2M Servers with the layout of our Object. A server already knows the Object 3303 which contains the following resources:

| Resource Name                     | Resource ID | Access Type | Mandatory | Type    | Description                                                                                    |
| --------------------------------- | ----------- | ----------- | --------- | ------- | ---------------------------------------------------------------------------------------------- |
| Sensor Value                      | 5700        | R           | Mandatory | Float   | Last or Current Measured Value from the Sensor.                                                |
| Min Measured Value                | 5601        | R           | Optional  | Float   | The minimum value measured by the sensor since power ON or reset.                              |
| Max Measured Value                | 5602        | R           | Optional  | Float   | The maximum value measured by the sensor since power ON or reset.                              |
| Min Range Value                   | 5603        | R           | Optional  | Float   | The minimum value that can be measured by the sensor.                                          |
| Max Range Value                   | 5604        | R           | Optional  | Float   | The maximum value that can be measured by the sensor.                                          |
| Sensor Units                      | 5701        | R           | Optional  | String  | Measurement Units Definition e.g. "Cel" for Temperature in Celsius.                            |
| Reset Min and Max Measured Values | 5605        | E           | Optional  |         | Reset the Min and Max Measured Values to Current Value.                                        |

The Sensor Value resource is mandatory to implement and is used to monitor the temperature value.

All the others resources are not relevant to our use case. As they are optional, we do not implement them.

To implement this Object in our device, in our main(), after initializing the IOWA context (see above), we declare an array describing resource:

```c
iowa_lwm2m_resource_desc_t resources[1] =
  { {5700, IOWA_LWM2M_TYPE_FLOAT, IOWA_DM_READ , IOWA_RESOURCE_FLAG_MANDATORY} };
```

Note that the header file "iowa_IPSO_ID.h" in the "include" folder contains defines for the ID, type and operations of the Reusable Resources defined by the IPSO Alliance. Using these macros, we can write the equivalent code as:

```c
iowa_lwm2m_resource_desc_t resources[1] =
  { {IPSO_RSC_ID_SENSOR_VALUE, IPSO_RSC_TYPE_SENSOR_VALUE, IPSO_RSC_OP_SENSOR_VALUE, IOWA_RESOURCE_FLAG_MANDATORY} };
```

Instance-wise, we only have one temperature sensor to monitor and it makes no sense that the LwM2M Server could create new instances or delete some and the object does not used resources that can be multiple. The *instanceCallback* and the *resInstanceCallback* can be omitted and we declare a single instance with ID 0:

```c
uint16_t singleInstanceID[1] = { 0 };

result = iowa_client_add_custom_object(contextP,
                                       3303,
                                       1, singleInstanceID,
                                       1, resources,
                                       tempSensorCallback, NULL, NULL,
                                       NULL);
```

Now the Temperature Object is presented to the LwM2M Servers by the IOWA stack.

When a Server performs a command on one of its resources (or the whole Object), the `tempSensorCallback` function is called.

We want to inform to the LwM2M Server the last values taken with the timestamp associated to each value.

Let’s assume our device provides two functions `get_time()` and `get_temperature()` to monitor the temperature value and one interruption to handle:

- `NEW_TEMP` when a new temperature value is available.

We first define a new structure and two global variables to store the temperature values. The temperature values will be stored inside a circular buffer:

```c
#define NB_VALUE 10

typedef struct
{
    int32_t timestamp;
    float   value;
} temp_value_t;

temp_value_t valuesList[NB_VALUE] = {0};
size_t currentIndex = 0;
```

In the interrupt handler, we modify these variables to store the temperature values (pseudo-code):

```c
switch (interrupt)
{
(...)
case NEW_TEMP:
{
    valuesList[currentIndex].timestamp = get_time();
    valuesList[currentIndex].value = get_temperature();

    currentIndex++;
    if (currentIndex == NB_VALUE)
    {
        currentIndex = 0;
    }
    break;
}
(...)
}
```

Now the `tempSensorCallback` function can be implemented:

```c
iowa_status_t tempSensorCallback(iowa_dm_operation_t operation,
                                 iowa_lwm2m_data_t *dataP,
                                 size_t numData,
                                 void *userData,
                                 iowa_context_t iowaH)
{
    iowa_status_t result;
    size_t i;

    result = IOWA_COAP_NO_ERROR;

    switch (operation)
    {
    case IOWA_DM_READ:
        for (i = 0; i < numData; i++)
        {
            switch (dataP[i].resourceID)
            {
            case 5700:
            {
                // On a Read operation, provides all the timestamp values
                size_t j;

                if (valuesList[0].timestamp == 0)
                {
                    // No value added yet
                    break;
                }

                if (dataP[i].timestamp != 0)
                {
                    // Loop on next element
                    break;
                }

                // Find the latest timestamp not currently added
                j = currentIndex - 1;
                if (i > 0)
                {
                    while (j != currentIndex)
                    {
                        if (j > 0)
                        {
                            if (valuesList[j-1].timestamp == 0)
                            {
                                break;
                            }
                        }
                        else if(valuesList[NB_VALUE-1].timestamp == 0)
                        {
                            break;
                        }

                        if (valuesList[j].timestamp < dataP[i-1].timestamp)
                        {
                            break;
                        }

                        if (j == 0)
                        {
                            j = NB_VALUE - 1;
                        }
                        else
                        {
                            j--;
                        }
                    }
                }

                dataP[i].value.asFloat = valuesList[j].value;
                dataP[i].timestamp = valuesList[j].timestamp;

                if (j > 0)
                {
                    if (valuesList[j-1].timestamp != 0)
                    {
                        // Changed result to be called again in the callback
                        result = IOWA_COAP_231_CONTINUE;
                    }
                }
                else if (valuesList[NB_VALUE-1].timestamp != 0)
                {
                    // Changed result to be called again in the callback
                    result = IOWA_COAP_231_CONTINUE;
                }

                break;
            }

            default:
                // Already handled by IOWA stack
                break;
            }
        }
        break;

    default:
        // Already handled by IOWA stack
        break;
    }

    return result;
}
```

Pay attention that the timestamp must be absolute and not relative to the current time, means negative values are not accepted. If the timestamp is zero, it's ignored.

If you test this code, you will notice that you can retrieve the current temperature value. However the LwM2M Server will not receive notifications when observing the Object resources.

To correct this, we need to inform the IOWA stack of the changes not triggered by a LwM2M Server. This is done by calling the API [`iowa_client_object_resource_changed()`](ClientAPI.md#iowa_client_object_resource_changed) whenever a change occurs.

A correct interrupt handler is then:

```c
switch (interrupt)
{
(...)
case NEW_TEMP:
{
    valuesList[currentIndex].timestamp = get_time();
    valuesList[currentIndex].value = get_temperature();

    currentIndex++;
    if (currentIndex == NB_VALUE)
    {
        currentIndex = 0;
    }

    iowa_client_object_resource_changed(contextP, 3303, 0, 5700);
    break;
}
(...)
}
```

The diagram of the above example can be:

![Timestamped Notifications](images/timestamped_notifications.png)

- The Server sends first an Observation on /3303/0/5700.
- At this time the LwM2M Client has only one value to send back, and so the return value of the callback is 0.00 (No Error).
- Then the LwM2M Client retrieves the temperature values and call [`iowa_client_object_resource_changed`](ClientAPI.md#iowa_client_object_resource_changed) for each new value. The Object callback has the possibility to return multiple values by returning the 2.31 (Continue) code.

On first round, the application has to provide the first value, the callback is called with:

- *numData*: 1
- *dataP[0]*:
    - *dataP[0].objectID*: 3303, *dataP[0].instanceID*: 0, *dataP[0].resourceID*: 5700, *dataP[0].resInstanceID*: 65535 (IOWA_LWM2M_ID_ALL)
    - *dataP[0].type*: IOWA_LWM2M_TYPE_FLOAT, *dataP[0].value.asFloat*: 0.0
    - *dataP[0].timestamp*: 0

On second round, the application has to provide the second value, the callback is called with:

- *numData*: 2
- *dataP[0]*:
    - *dataP[0].objectID*: 3303, *dataP[0].instanceID*: 0, *dataP[0].resourceID*: 5700, *dataP[0].resInstanceID*: 65535 (IOWA_LWM2M_ID_ALL)
    - *dataP[0].type*: IOWA_LWM2M_TYPE_FLOAT, *dataP[0].value.asFloat*: **FIRST_VALUE**
    - *dataP[0].timestamp*: **FIRST_TIMESTAMP**
- *dataP[1]*:
    - *dataP[1].objectID*: 3303, *dataP[1].instanceID*: 0, *dataP[1].resourceID*: 5700, *dataP[1].resInstanceID*: 65535 (IOWA_LWM2M_ID_ALL)
    - *dataP[1].type*: IOWA_LWM2M_TYPE_FLOAT, *dataP[1].value.asFloat*: 0.0
    - *dataP[1].timestamp*: 0

On third round, the application has to provide the third value, the callback is called with:

- *numData*: 3
- *dataP[0]*:
    - *dataP[0].objectID*: 3303, *dataP[0].instanceID*: 0, *dataP[0].resourceID*: 5700, *dataP[0].resInstanceID*: 65535 (IOWA_LWM2M_ID_ALL)
    - *dataP[0].type*: IOWA_LWM2M_TYPE_FLOAT, *dataP[0].value.asFloat*: **FIRST_VALUE**
    - *dataP[0].timestamp*: **FIRST_TIMESTAMP**
- *dataP[1]*:
    - *dataP[1].objectID*: 3303, *dataP[1].instanceID*: 0, *dataP[1].resourceID*: 5700, *dataP[1].resInstanceID*: 65535 (IOWA_LWM2M_ID_ALL)
    - *dataP[1].type*: IOWA_LWM2M_TYPE_FLOAT, *dataP[1].value.asFloat*: **SECOND_VALUE**
    - *dataP[1].timestamp*: **SECOND_TIMESTAMP**
- *dataP[2]*:
    - *dataP[2].objectID*: 3303, *dataP[2].instanceID*: 0, *dataP[2].resourceID*: 5700, *dataP[2].resInstanceID*: 65535 (IOWA_LWM2M_ID_ALL)
    - *dataP[2].type*: IOWA_LWM2M_TYPE_FLOAT, *dataP[2].value.asFloat*: 0.0
    - *dataP[2].timestamp*: 0

And so on, until the last value is provided and the return value of the callback is 0.00 (No Error).