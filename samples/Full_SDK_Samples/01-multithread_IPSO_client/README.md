# Multithreaded IPSO Client

This is the IPSO Client using a separate thread to update the sensor value.

The following API will be explained:

- `iowa_stop()`

Running IOWA in a multithreaded environment requires you to do the followings beforehand:

* set the compilation flag **IOWA_THREAD_SUPPORT**
* implement the system abstraction function `iowa_system_connection_interrupt_select()`
* implement the system abstraction functions `iowa_system_mutex_lock()` and `iowa_system_mutex_unlock()`

## Preparation

To use this sample, you need the [IOWA Full SDK](https://ioterop.com/iowa/).

You can either copy the IOWA Full SDK to the **iowa** folder at the root of the repo, or you can edit the root **CMakeLists.txt** to indicate the path to the IOWA Full SDK by modifying the line 14:

```
set_property(GLOBAL PROPERTY iowa_sdk_folder "PATH_TO_THE_IOWA_SDK")
```

## Usage

The usage is exactly the same as the IPSO Client from the Evaluation SDK samples.

## Breakdown

### Client Pseudo Code

This is the pseudo code of Client main function:

```c
main()
{
    // Initialization
    iowa_init();

    // LwM2M Client configuration
    iowa_client_configure(CLIENT_NAME);

    // IPSO Temperature Object enabling
    iowa_client_IPSO_add_sensor(IOWA_IPSO_TEMPERATURE);

    // LwM2M Server declaration
    iowa_client_add_server(SERVER_SHORT_ID, SERVER_URI, SERVER_LIFETIME);

    // Measure task start
    start thread measure_routine()

    // "Main loop"
    iowa_step(-1);

    // Cleanup
    iowa_client_remove_server(SERVER_SHORT_ID);
    iowa_client_IPSO_remove_sensor();
    iowa_close();
}
```

And the pseudo code of the measure routine:

```c
measure_routine()
{
    for (120s)
    {
        sleep(3s);
        // Temperature value update
        iowa_client_IPSO_update_value()
    }

    // IOWA stop
    iowa_stop();
}
```

### Main thread

#### Initialization

When using IOWA in a multithreaded application, we need a mutex to ensure the IOWA APIs are reentrant. IOWA will use the platform abstraction functions `iowa_system_mutex_lock()` and `iowa_system_mutex_unlock()`. These functions like most platform abstraction functions have an "user data" parameter. This "user data" is the parameter passed to `iowa_init()`.

```c
#ifdef _WIN32
    DWORD  threadId;
    HANDLE thread;
    HANDLE mutex;
#else
    pthread_t thread;
    pthread_attr_t attr;
    pthread_mutex_t mutex;
#endif

    // Create a mutex for the iowa_system_mutex_* functions
#ifdef _WIN32
    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL)
#else
    if (pthread_mutex_init(&mutex, NULL) != 0)
#endif
    {
        fprintf(stderr, "Mutex creation failed.\r\n");
        return 1;
    }

    // Initialize the IOWA stack using the mutex as the system abstraction functions user data.
    iowaH = iowa_init(&mutex);
```

First a mutex is created. (The creation depends on the build environment: Windows or Linux.)

Then the memory address of this mutex is passed as parameter to `iowa_init()`.

#### LwM2M Client Configuration

 This step is the same as in the Baseline Client sample.

#### IPSO Temperature Object Enabling

 This step is the same as in the IPSO Client sample.

#### LwM2M Server Declaration

 This step is the same as in the Baseline Client sample.

#### "Main Loop"

We let IOWA run forever. The measure routine will take care of stopping it.

```c
result = iowa_step(iowaH, -1);
```

The call to `iowa_step()` is the same as in the Simple Client sample. Using a time value of "-1" indicates to IOWA that the iowa_step() should return only in case of error or when `iowa_stop()` is called.

#### Cleanup

This step is the same as in the IPSO Client sample.

### Measure Thread

>  For code readability reasons, the shared variables between the two threads (namely the IOWA context and the sensor identifier) are simply declared as global variables. A real-world application should protect these data against concurrent access.

#### Temperature Value Update

Like in the IPSO Client sample, we inform IOWA that the value measured by the sensor has changed by calling `iowa_client_IPSO_update_value()`.

```c
int i;
iowa_status_t result;

i = 0;
do
{
#ifdef _WIN32
    Sleep(3000);
#else
    usleep(3000000);
#endif

    result = iowa_client_IPSO_update_value(iowaH, sensorId, 20 + i%4);

    i++;
} while (i < 40 && result == IOWA_COAP_NO_ERROR);
```

Since `iowa_step()` is already running in the main thread, between sensor value updates, we just sleep for three seconds.

#### IOWA Stop

Once two minutes have elapsed, we stop IOWA, making the `iowa_step()` exit in the main thread.

```c
iowa_stop(iowaH);
```

The only argument is as usual the IOWA context created in the Initialization step.

------------------------------------------------------------

![IoTerop Logo](../../.images/IoTerop_logo.jpg)