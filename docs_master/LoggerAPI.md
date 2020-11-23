\clearpage

## Logger Component

The functions explained below are defined inside the file *include/iowa_logger.h*.

### Presentation

To use your logger layer, you have to set the define [`IOWA_LOGGER_USER`][IOWA_LOGGER_USER] and implement the following functions.

```c
void iowa_log(uint8_t part,
              uint8_t level,
              const char *functionName,
              unsigned int line,
              const char *message);

void iowa_log_arg(uint8_t part,
                  uint8_t level,
                  const char *functionName,
                  unsigned int line,
                  const char *message, ...);

void iowa_log_buffer(uint8_t part,
                     uint8_t level,
                     const char *functionName,
                     unsigned int line,
                     const char *message,
                     const uint8_t *buffer,
                     size_t bufferLength);

void iowa_log_arg_buffer(uint8_t part,
                         uint8_t level,
                         const char *functionName,
                         unsigned int line,
                         const char *message,
                         const uint8_t *buffer,
                         size_t bufferLength,
                         ...);
```

\clearpage

### Functions

#### iowa_log

#** Prototype **

```c
void iowa_log(uint8_t part,
              uint8_t level,
              const char *functionName,
              unsigned int line,
              const char *message);
```

#** Description **

`iowa_log()` writes a log message to the output.

#** Arguments **

*part*
: Log part.

*level*
: Log level.

*functionName*
: Name of the function from where the Log has been called.

*line*
: Line from where the Log has been called.

*message*
: String to display.

#** Return Value **

None.

#** Header File **

iowa_logger.h

\clearpage

#### iowa_log_arg

#** Prototype **

```c
void iowa_log_arg(uint8_t part,
                  uint8_t level,
                  const char *functionName,
                  unsigned int line,
                  const char *message, ...);
```

#** Description **

`iowa_log_arg()` writes a log message to the output with specifier arguments.

#** Arguments **

*part*
: Log part.

*level*
: Log level.

*functionName*
: Name of the function from where the Log has been called.

*line*
: Line from where the Log has been called.

*message*
: String to display.

*...*
: Format specifiers which are replaced by the values specified in additional arguments.

#** Return Value **

None.

#** Header File **

iowa_logger.h

\clearpage

#### iowa_log_buffer

#** Prototype **

```c
void iowa_log_buffer(uint8_t part,
                     uint8_t level,
                     const char *functionName,
                     unsigned int line,
                     const char *message,
                     const uint8_t *buffer,
                     size_t bufferLength);
```

#** Description **

`iowa_log_buffer()` writes a buffer with a log message to the output.

#** Arguments **

*part*
: Log part.

*level*
: Log level.

*functionName*
: Name of the function from where the Log has been called.

*line*
: Line from where the Log has been called.

*message*
: String to display.

*buffer*
: Buffer.

*bufferLength*
: Buffer size.

#** Return Value **

None.

#** Header File **

iowa_logger.h

\clearpage

#### iowa_log_arg_buffer

#** Prototype **

```c
void iowa_log_arg_buffer(uint8_t part,
                         uint8_t level,
                         const char *functionName,
                         unsigned int line,
                         const char *message,
                         const uint8_t *buffer,
                         size_t bufferLength,
                         ...);
```

#** Description **

`iowa_log_arg_buffer()` writes a buffer with a log message to the output with specifier arguments.

#** Arguments **

*part*
: Log part.

*level*
: Log level.

*functionName*
: Name of the function from where the Log has been called.

*line*
: Line from where the Log has been called.

*message*
: String to display.

*buffer*
: Buffer.

*bufferLength*
: Buffer size.

*...*
: Format specifiers which are replaced by the values specified in additional arguments.

#** Return Value **

None.

#** Header File **

iowa_logger.h
