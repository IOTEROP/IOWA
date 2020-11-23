# IOWA Multithread Environment

## Presentation

The thread safety is enabled through the flag **IOWA_THREAD_SUPPORT**. Without this flag, IOWA APIs cannot be called from different threads.
**IOWA_THREAD_SUPPORT** enables this feature by introducing a global mutex for the IOWA functions.

Three platform functions must be additionally implemented to support the thread safety:

* [`iowa_system_connection_interrupt_select()`](AbstractionLayer.md#iowa_system_connection_interrupt_select)
* [`iowa_system_mutex_lock()`](AbstractionLayer.md#iowa_system_mutex_lock)
* [`iowa_system_mutex_unlock()`](AbstractionLayer.md#iowa_system_mutex_unlock)

## Code Example

A global architecture is introduced. This architecture is not intended to be exactly followed. More than one architecture is achievable and depends primarily on the use case.

Let's say here, IOWA is running on a RTOS system.
This RTOS system will create a main thread. In addition to this thread, for our use case, two other threads will be created:

* The first will be used to initialize IOWA and run the [`iowa_step()`](CommonAPI.md#iowa_step) function.
* The second will be used to update the value of an IPSO object.

About these threads:

* IOWA thread:
    * Memory size (stack allocation): 512 bytes (recommended)
    * Priority: High
* Update thread:
    * Memory size (stack allocation): 256 bytes (recommended)
    * Priority: Low

Next is a sample pseudo-code implementation of these tasks:

```c
#include "iowa_client.h"

typedef struct
{
    iowa_context_t iowaH;
    iowa_sensor_t  sensorId;
    int            pipeArray[2];
} user_struct_t;

void iowa_task(void *userData)
{
    user_struct_t *dataP;
    iowa_device_info_t devInfo;

    /******************
    * Initialization
    */

    dataP = (user_struct_t *)userData;

    dataP->iowaH = iowa_init(dataP);

    devInfo.manufacturer = "IOTEROP";
    devInfo.deviceType = "Example device";
    devInfo.modelNumber = "1";
    devInfo.serialNumber = NULL;
    devInfo.hardwareVersion = NULL;
    devInfo.softwareVersion = NULL;
    devInfo.optFlags = 0;
    iowa_client_configure(dataP->iowaH, "IOWA_Sample_MT_Client", devInfo, NULL);

    iowa_client_IPSO_add_sensor(dataP->iowaH,
                                IOWA_IPSO_VOLTAGE, 12.0,
                                "V", "Test DC", 0.0, 0.0,
                                &(dataP->sensorId));

    iowa_client_add_server(dataP->iowaH,
                           1234,
                           "coap://localhost:5683",
                           0,
                           0,
                           IOWA_SEC_NONE);

    /******************
    * "Main loop"
    */

    iowa_step(dataP->iowaH, -1); // Run indefinitely

    /******************
    * Close
    */

    iowa_client_IPSO_remove_sensor(dataP->iowaH, dataP->sensorId);
    iowa_client_remove_server(dataP->iowaH, 1234);
    iowa_close(dataP->iowaH);
}

void update_task(void *userData)
{
    user_struct_t *dataP;

    WAIT_IOWA_INIT(); // Can be done through a semaphore

    dataP = (user_struct_t *)userData;

    /******************
    * "Main loop"
    */

    while (1)
    {
        float sensorValue;

        sensorValue = READ_VOLTAGE();
        iowa_client_IPSO_update_value(dataP->iowaH,
                                      dataP->sensorId,
                                      sensorValue);

        SLEEP(1);
    }
}

int main(int argc,
         char *argv[])
{
    user_struct_t data;

    RTOS_INIT();

    CREATE_THREAD(iowa_task, 512, &data, 2);
    CREATE_THREAD(update_task, 256, &data, 1);

    /******************
    * "Main loop"
    */

    while (1)
    {
        // Do nothing
    }

    return 0;
}
```

If for any reason, the [`iowa_step()`](CommonAPI.md#iowa_step) function has to be stopped, the [`iowa_stop()`](CommonAPI.md#iowa_stop) API can be called to force the step to finish by calling the [`iowa_system_connection_interrupt_select()`](AbstractionLayer.md#iowa_system_connection_interrupt_select) platform function.

If [`iowa_step()`](CommonAPI.md#iowa_step) function is called with a negative value, [`iowa_system_connection_select()`](AbstractionLayer.md#iowa_system_connection_select) will be called with a long timeout. To prevent the [`iowa_step()`](CommonAPI.md#iowa_step) function from waiting until the timeout is reached, the [`iowa_system_connection_select()`](AbstractionLayer.md#iowa_system_connection_select) function must react and exit when the [`iowa_system_connection_interrupt_select()`](AbstractionLayer.md#iowa_system_connection_interrupt_select) is called.

A way to implement this behaviour is to create a local pipe:

* One side, named `read`, of the pipe will be used by [`iowa_system_connection_select()`](AbstractionLayer.md#iowa_system_connection_select) to listen on an interrupt select event.
* The other side, named `write`, of the pipe will be used by [`iowa_system_connection_interrupt_select()`](AbstractionLayer.md#iowa_system_connection_interrupt_select) to send an interrupt select event.

Below an example of these platform functions with pseudo-code:

```c
#define READ_PIPE  0
#define WRITE_PIPE 1

int iowa_system_connection_select(void **connArray,
                                  size_t connCount,
                                  int32_t timeout,
                                  void *userData)
{
    fd_set readfds;
    int result;
    user_struct_t *dataP;

    dataP = (user_struct_t *)userData;

    SET(dataP->pipeArray[READ_PIPE], &readfds);
    for (i = 0; i < connCount; i++)
    {
        SET(get_connection_sock(connArray[i]), &readfds);
    }

    result = SELECT(timeout);
    if (result > 0)
    {
        for (i = 0; i < connCount; i++)
        {
            if (!IS_SET(get_connection_sock(connArray[i]), &readfds))
            {
                connArray[i] = NULL;
            }
        }

        if (IS_SET(dataP->pipeArray[READ_PIPE], &readfds))
        {
            // Remove data written by iowa_system_connection_interrupt_select()
            READ(dataP->pipeArray[READ_PIPE]);
        }
    }

    return result;
}

void iowa_system_connection_interrupt_select(void *userData)
{
    user_struct_t *dataP;

    dataP = (user_struct_t *)userData;

    WRITE(dataP->pipeArray[WRITE_PIPE], "NOISE");
}

void iowa_system_mutex_lock(void *userData)
{
    MUTEX_LOCK();
}

void iowa_system_mutex_unlock(void *userData)
{
    MUTEX_UNLOCK();
}
```
