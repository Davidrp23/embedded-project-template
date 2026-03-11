# Printing messages to console and logging

## Overview

One of the very first things to learn in a new software development environment is the ability to print messages like the popular “Hello World!” on a console. In the previous lesson, we have briefly seen `printk()` to print simple messages on the console. In this lesson we will learn more about logging using both the simple method of `printk()` and a sophisticated method using the advanced logging module.

In the exercises section of this lesson, we will first practice using the user-friendly `printk()` function to print string literals and formatted strings to a console. In Exercise 2, we will cover enabling/configuring the feature-rich logger module to print string literals, formatted strings, and hex dump variables. Lastly, in Exercise 3, we will examine the logger module features in more depth.

---

## printk() function

For printing basic messages on a console, we can use the `printk()` method. The syntax `printk()` is similar to the standard `printf()` in C; you can provide either a string literal or a format string followed by one or more variables to be printed. However, `printk()` is a less advanced function that only supports a subset of the features that `printf()` does, making it optimized for embedded development.

A basic set of specifiers are supported:

*   Signed decimal: `%d`, `%i` and its subcategories
*   Unsigned decimal: `%u` and its subcategories
*   Unsigned hexadecimal: `%x` (`%X` is treated as `%x`)
*   Pointer: `%p`
*   String: `%s`
*   Character: `%c`
*   Percent: `%%`
*   New line: `\n`
*   Carriage return: `\r`

Field width (with or without leading zeroes) is supported. Length attributes `h`, `hh`, `l`, `ll` and `z` are supported. However, integral values with `lld` and `lli` are only printed if they fit in a `long`, otherwise `ERR` is printed. Full 64-bit values may be printed with `llx`. Flags and precision attributes (float and double) are not supported by default, but can be enabled manually, which we will cover in Lesson 6.

For example, the following line will print the string `Button 1 was pressed!` on the console (including a new line and carriage return).
```c
printk("Button 1 was pressed!\n\r");
```
While this line will print the formatted string `The value of x is 44` on the console (including a new line and carriage return).
```c
int x = 44;
printk("The value of x is %d\n\r",x);
```
Using `printk()` is straightforward, all you have to do is :

1.  **Include the console drivers.**
    This is done by enabling the configuration option `CONFIG_CONSOLE` in the application configuration file (`prj.conf`). This step is not necessary if it is already set in the board configuration file.

2.  **Select the console.**
    There are a few options available, such as the UART console (`CONFIG_UART_CONSOLE`) and RTT console (`CONFIG_RTT_CONSOLE`).

    > **Definition**
    > *   **UART console:** Uses Universal Asynchronous Receiver/Transmitter (UART) hardware for serial communication between the device and a computer.
    > *   **RTT console:** RTT (Real Time Transfer) is a proprietary technology developed by SEGGER Microcontroller for bidirectional communication that supports J-Link devices and ARM-based microcontrollers. An RTT console allows you to view debug messages and logging information from your device.

    In this lesson, we will focus on the UART console, which can easily be captured using a serial terminal program like the built-in serial terminal in VS Code. The default console set in the board configuration file is the UART console. This step is not necessary if it is already set in the board configuration file.

3.  **Include the header file** `<zephyr/sys/printk.h>` in your application source code.

In exercise 1, we will practice using the `printk()` function.

> **Important**
> The output of the `printk()` is **not deferred**, meaning the output is sent immediately to the console without any mutual exclusion or buffering. This is also known as synchronous logging, in-place logging, or blocking logging. Logs are sent immediately as they are issued, and `printk()` will not return until all bytes of the message are sent. This limits the use of this function in time-critical applications.

---

## Logger module

The **logger module** is the recommended method for sending messages to a console. Unlike the `printk()` function, which will not return until all bytes of the message are sent, the logger module supports both in-place and deferred logging among many other advanced features such as:

*   Multiple backends
*   Compile time filtering on module level
*   Run time filtering independent for each backend
*   Timestamping with user-provided function
*   Dedicated API for dumping data
*   Coloring of logs
*   `printk()` support – `printk` messages can be redirected to the logger

You can read the full list of features in the [Logging documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/services/logging/index.html).

As you will see, the logger module is highly configurable at compile time and at run time. By using proper configuration options, logs can be gradually removed from compilation to reduce image size and execution time when logs are not needed. During compilation, logs can be filtered out based on module and severity level.

> **More on this**
> When the logging module is used, it will create a low priority thread (`log_process_thread_func`) by default. The task of this thread is to take the deferred (“queued”) logs and push them out to a console.

### Severity levels

Logs can also be compiled in, but filtered at run time using a dedicated API. The run time filtering is independent for each backend and each module. There are four severity levels available in the system, see the table below.

| Number          | Severity Level | Details                              | Macro         |
| :-------------- | :------------- | :----------------------------------- | :------------ |
| 1 (most severe) | Error          | Severe error conditions              | `LOG_LEVEL_ERR` |
| 2               | Warning        | Conditions that should be taken care of | `LOG_LEVEL_WRN` |
| 3               | Info           | Informational messages               | `LOG_LEVEL_INF` |
| 4 (least severe)| Debug          | Debugging messages                   | `LOG_LEVEL_DBG` |

The following set of macros are available for each level:

*   **`LOG_X`** for standard printf-like messages, where `X` can be `DBG`, `INF`, `WRN`, or `ERR`.

For example, the following line:
```c
LOG_INF("nRF Connect SDK Fundamentals");
```
would give the output:
```terminal
[00:00:00.382,965] <inf> Less4_Exer2: nRF Connect SDK Fundamentals
```
*   `[00:00:00.382,965]` is a timestamp associated with the generation of the message. It uses the format `hh:mm:ss.ms,us`. The logger gets the timestamp by internally calling the kernel function `k_cycle_get_32()`. This routine returns the current time since boot up (uptime), as measured by the system’s hardware clock. You could change this to return an actual time and date if an external Real-time clock is present on the system.
*   `<inf>` indicates the severity level, i.e information.
*   `Less4_Exer2` is the name of the module generating the log message.
*   `nRF Connect SDK Fundamentals` is the actual message.

As another example, the following lines:
```c
    LOG_INF("Exercise %d",2);
    LOG_DBG("A log message in debug level");
    LOG_WRN("A log message in warning level!");
    LOG_ERR("A log message in Error level!");
```
would print the following on the console (assuming the log level allows all these messages):
```terminal
[timestamp] <inf> module_name: Exercise 2
[timestamp] <dbg> module_name: A log message in debug level
[timestamp] <wrn> module_name: A log message in warning level!   // Output likely yellow
[timestamp] <err> module_name: A log message in Error level!     // Output likely red
```
The first logging command `LOG_INF("Exercise %d",2)` is used to generate a formatted string with an integer place holder `%d`, similar to what have we seen with `printk()`. Instead of the constant `2`, you could place any integer variable to print its value.

The remaining three logging lines will print messages in three different severity levels. Note that messages with Warning severity level will be displayed in yellow and messages with Error severity level will be displayed in red. This is because the coloring of error and warning logs feature (`CONFIG_LOG_BACKEND_SHOW_COLOR`) is enabled by default as we will see in Exercises 2 and 3.

### Dumping data

We also have the **`LOG_HEXDUMP_X`** macros for dumping data where `X` can be `DBG`, `INF`, `WRN`, or `ERR`.

The `LOG_HEXDUMP_X` macro takes three parameters: a pointer to the data to be printed, the size in bytes of the data to be printed, and a string to describe the data.

For example, the following lines:
```c
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03,
                      0x04, 0x05, 0x06, 0x07,
                      'H', 'e', 'l', 'l','o'};
    LOG_HEXDUMP_INF(data, sizeof(data),"Sample Data!");
```
will print the following on the console:
```terminal
[00:00:00.257,385] <inf> Less4_Exer2: Sample Data!
                  00 01 02 03 04 05 06 07  48 65 6c 6c 6f | ........ Hello
```

### Configuration categories

There are two configuration categories for the logger module: **configurations per module** and **global configuration**. When logging is enabled globally, it works for all modules. However, modules can disable logging locally. Every module can specify its own logging level (`LOG_LEVEL_[level]`) or use `LOG_LEVEL_NONE`, which will disable the logging for that module.

The module logging level will be honored unless a global override is set. A global override can only increase the logging level. It cannot be used to lower module logging levels that were previously set higher. It is also possible to globally limit logs by providing a maximum severity level present in the system (`CONFIG_LOG_MAX_LEVEL`). For instance, if the maximum level in the system is set to `INFO`, messages less severe than the info level (i.e `DEBUG`) will be excluded.

Each module that is using the logger must specify a unique name and register itself to the logger. We will cover how to do this in Exercise 2. If the module consists of more than one file, registration is performed in one file, but each file must declare the module name.

### Summary

As we have seen, the logger API has two types of messages: standard and hexdump. When the logger API is called, a message is created and added to a dedicated, configurable buffer containing all log messages. Each message contains a source ID, timestamp, and severity level. A standard message contains a pointer to the string and any arguments. A hexdump message contains copied data. We will cover how to use the logger module in Exercise 2 and Exercise 3.

The logger module is designed to be thread-safe and minimizes the time needed to log the message. Time-consuming operations like string formatting or obtaining access to the transport (i.e UART, RTT or whatever backend you are using) are not performed immediately when the logger API is called (in deferred mode).

---

## Exercise 1: Printing to the console

*(Instrucciones correspondientes a nRF Connect SDK v3.0.0 – v3.2.0)*

In this exercise, we will practice using the user-friendly `printk()` function to print strings to the console.

We will base this exercise on Lesson 2 Exercise 2 and modify it so that when button 1 (button 0 on nRF54 Series DKs) is pressed, the factorials of the numbers from 1 to 10 are calculated and printed on the console as shown below. We are using the UART console, which is the default set in the board configuration file.

Expected output on button press:
```terminal
*** Booting nRF Connect SDK ... ***
*** Using Zephyr OS ... ***
nRF Connect SDK Fundamentals - Lesson 4 - Exercise 1
Calculating the factorials of numbers from 1 to 10:
The factorial of  1 = 1
The factorial of  2 = 2
The factorial of  3 = 6
The factorial of  4 = 24
The factorial of  5 = 120
The factorial of  6 = 720
The factorial of  7 = 5040
The factorial of  8 = 40320
The factorial of  9 = 362880
The factorial of 10 = 3628800
_______________________________________________________
```

### Exercise steps

1.  In the GitHub repository for this course, use the base code for this exercise, found in `l4/l4_e1` (select your version subfolder).
2.  Use the **Open an existing application** option in nRF Connect for VS Code to open the base code for this exercise.
3.  Make sure that your development kit is powered on and connected to your computer.
4.  Configure a terminal emulator on your machine (as covered in Lesson 3 Exercise 1).
5.  Open the `main.c` file inside `l4/l4_e1/src`.
6.  Under `STEP 6` comment, include the header file for the `printk()` function:
    ```c
    #include <zephyr/sys/printk.h>
    ```
    > **Note**: Console drivers (`CONFIG_CONSOLE`, `CONFIG_UART_CONSOLE`) are typically enabled by default in board configuration files, so no changes are needed in `prj.conf` for this.

7.  Let’s print out our first message. Search for `STEP 7` comment in `main()` and add the following line:
    ```c
    printk("nRF Connect SDK Fundamentals - Lesson 4 - Exercise 1\n");
    ```
    Build and flash the application. You should see this message printed on the console upon boot.

8.  Now let’s replace the button callback function (`button_pressed`) with code that calculates and prints factorials:
    8.1 Define a macro for the maximum number. Add near the top of the file:
    ```c
    #define MAX_NUMBER_FACT 10
    ```
    8.2 Replace the existing `button_pressed` function (the ISR) with the following code:
    ```c
    void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
    {
        int i;
        long int factorial = 1;

        printk("Calculating the factorials of numbers from 1 to %d:\n", MAX_NUMBER_FACT);
        for (i = 1; i <= MAX_NUMBER_FACT; i++) {
            factorial = factorial * i;
            printk("The factorial of %2d = %ld\n", i, factorial);
        }
        printk("_______________________________________________________\n");
        /* Important note!
         * Code in ISR runs at a high priority, therefore, it should be written with timing in mind.
         * Too lengthy or too complex tasks should not be performed by an ISR, they should be deferred
         * to a thread. (Printing here is for demonstration; in real apps, defer complex work)
         */
    }
    ```
    This code uses `printk` three times: once for a title, once inside the loop to print each factorial using format specifiers (`%2d`, `%ld`), and once to print a separator line.

9.  Add a build configuration, build the exercise, and flash it to the board.
10. Press button 1 (or button 0 on nRF54 DKs). Observe the factorial output printed on the console as shown in the expected output section above.

The solution for this exercise can be found in the GitHub repository, `l4/l4_e1_sol` (select your version subfolder).

---

## Exercise 2: Using the logger module

*(Instrucciones correspondientes a nRF Connect SDK v3.0.0 – v3.2.0)*

In this exercise, we will simply redo Exercise 1 of this lesson, but this time we will be using the feature-rich **logger module**. We will enable the logger, and utilize it to print logs of different severities and to hexdump variables.

Expected output on boot:
```terminal
[timestamp] <inf> Less4_Exer2: nRF Connect SDK Fundamentals
[timestamp] <inf> Less4_Exer2: Exercise 2
[timestamp] <dbg> Less4_Exer2: A log message in debug level
[timestamp] <wrn> Less4_Exer2: A log message in warning level!     // Yellow text
[timestamp] <err> Less4_Exer2: A log message in Error level!       // Red text
[timestamp] <inf> Less4_Exer2: Sample Data!
                  00 01 02 03 04 05 06 07  48 65 6c 6c 6f | ........ Hello
```
Expected output on button press:
```terminal
[timestamp] <inf> Less4_Exer2: Calculating the factorials of numbers 1 to 10:
[timestamp] <inf> Less4_Exer2: The factorial of  1 = 1
[timestamp] <inf> Less4_Exer2: The factorial of  2 = 2
...
[timestamp] <inf> Less4_Exer2: The factorial of 10 = 3628800
```

The timestamping (`CONFIG_LOG_BACKEND_FORMAT_TIMESTAMP`) and coloring (`CONFIG_LOG_BACKEND_SHOW_COLOR`) features are enabled by default when `CONFIG_LOG=y`.

Before starting, ensure that your development kit is powered on and connected and that your terminal emulator is configured properly.

### Exercise steps

1.  In the GitHub repository for this course, open the base code for this exercise, found in `l4/l4_e2` (select your version subfolder). Open it as an existing application in VS Code.
2.  **Enable the logger module.** Add the following configuration line to `prj.conf`:
    ```kconfig
    # STEP 2: Enable Logger Module
    CONFIG_LOG=y
    ```
    Save the `prj.conf` file (`Ctrl+S` or enable Auto Save). This includes the logger module source code in the build.
3.  Open `main.c` inside the `l4_e2/src` directory.
4.  **Include the header file** of the logger module. Search for `STEP 4` comment and add:
    ```c
    #include <zephyr/logging/log.h>
    ```
5.  **Register your code** with the logger module. Use the `LOG_MODULE_REGISTER()` macro. Add this *after* the includes, before any functions:
    ```c
    // STEP 5: Register Logger Module
    LOG_MODULE_REGISTER(Less4_Exer2, LOG_LEVEL_DBG);
    ```
    *   `Less4_Exer2`: The mandatory module name (not a string).
    *   `LOG_LEVEL_DBG`: Optional maximum log level for this module. `DBG` means all levels (Debug, Info, Warning, Error) from this module will be processed. If omitted, the global default (`CONFIG_LOG_DEFAULT_LEVEL`, usually `INF`) is used.

6.  **Print logging information.** Add the following code snippet inside `main()` after the GPIO/interrupt setup, before the `while(1)` loop:
    ```c
    // STEP 6: Add log messages
    int exercise_num = 2;
    uint8_t data[] = {0x00, 0x01, 0x02, 0x03,
                      0x04, 0x05, 0x06, 0x07,
                      'H', 'e', 'l', 'l','o'};
    // Printf-like messages
    LOG_INF("nRF Connect SDK Fundamentals");
    LOG_INF("Exercise %d", exercise_num);
    LOG_DBG("A log message in debug level");
    LOG_WRN("A log message in warning level!");
    LOG_ERR("A log message in Error level!");
    // Hexdump some data
    LOG_HEXDUMP_INF(data, sizeof(data),"Sample Data!");
    ```
7.  **Change the callback function `button_pressed()`** to use the logger API instead of `printk()`. Replace the existing `button_pressed` function with the following:
    ```c
    void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
    {
        int i;
        long int factorial = 1;

        LOG_INF("Calculating the factorials of numbers 1 to %d:", MAX_NUMBER_FACT);
        for (i = 1; i <= MAX_NUMBER_FACT; i++) {
            factorial = factorial * i;
            LOG_INF("The factorial of %2d = %ld", i, factorial);
        }
        /* Important note!
         * Code in ISR runs at a high priority, therefore,
         * it should be written with timing in mind.
         * Too lengthy or too complex tasks should not be performed by an ISR,
         * they should be deferred to a thread.
         */
    }
    ```
    The key difference is using `LOG_INF` instead of `printk`. `LOG_INF` (in deferred mode) is generally safer and less blocking for use within an ISR.

8.  Build the exercise.
9.  Flash it to the board.
10. Using a serial terminal, observe the initial log output (including colored messages and hexdump) as shown in the expected output section.
11. Press button 1 (or button 0). Observe the factorial output printed using the logger.

The solution for this exercise can be found in the GitHub repository, in `l4/l4_e2_sol` (select your version subfolder).

---

## Exercise 3: Exploring the logger module features

*(Instrucciones correspondientes a nRF Connect SDK v3.0.0 – v3.2.0)*

In this exercise, we will practice configuring a software module using Kconfig configuration options. We will use the logger module as a case study and experiment with some of its features.

When the logger module is enabled (`CONFIG_LOG=y`), the following configurations are typically enabled **by default**:

| Option                         | Description                                                                            |
| :----------------------------- | :------------------------------------------------------------------------------------- |
| `CONFIG_LOG_MODE_DEFERRED`       | Deferred mode: Logs buffered, processed later by logger thread (less application impact). |
| `CONFIG_LOG_PROCESS_THREAD`      | Creates a dedicated logger thread for processing deferred logs.                           |
| `CONFIG_LOG_BACKEND_UART`        | Sends logs to the UART console backend.                                               |
| `CONFIG_LOG_BACKEND_SHOW_COLOR`  | Prints errors in red and warnings in yellow (terminal support required).              |
| `CONFIG_LOG_BACKEND_FORMAT_TIMESTAMP` | Formats timestamp as hh:mm:ss.ms,us.                                               |
| `CONFIG_LOG_MODE_OVERFLOW`       | If log buffer full, drops the oldest message to make space for new ones.              |

Let’s verify this by invoking **nRF Kconfig GUI** from the Actions menu in VS Code.

> **Important Reminder:** For SDK v2.8.0+, if using Sysbuild, select the application image in the **APPLICATIONS** view *before* opening the Kconfig GUI from the **ACTIONS** view to see the application's Kconfig settings.

The logger module settings are found under the menu `Sub Systems and OS Services -> Logging`.

*(Description of original image showing default logger configs in Kconfig GUI)*

These defaults are set by Kconfig definition files within the SDK. We can override them using our application's `prj.conf` file.

### Exercise steps

1.  In the GitHub repository for this course, open the base code for this exercise, found in `l4/l4_e3` (select your version subfolder). This code is identical to the solution of Exercise 2. Open it as an existing application in VS Code.
2.  **Disable colored output.** Add the following line to `prj.conf` to override the default:
    ```kconfig
    # STEP 2: Disable colored logs
    CONFIG_LOG_BACKEND_SHOW_COLOR=n
    ```
    Save `prj.conf`.
3.  Build the exercise. You can optionally confirm the change in the Kconfig GUI (search for `LOG_BACKEND_SHOW_COLOR`; it should be unchecked).
4.  Flash the application to the board. Observe the initial log output; the warning and error messages should no longer be colored.
    ```terminal
    [timestamp] <inf> Less4_Exer2: nRF Connect SDK Fundamentals
    [timestamp] <inf> Less4_Exer2: Exercise 2
    [timestamp] <dbg> Less4_Exer2: A log message in debug level
    [timestamp] <wrn> Less4_Exer2: A log message in warning level!   // No longer yellow
    [timestamp] <err> Less4_Exer2: A log message in Error level!     // No longer red
    ...
    ```
5.  **Try minimal logging mode.** Add the following configuration line to `prj.conf`:
    ```kconfig
    # STEP 5: Enable minimal logging mode
    CONFIG_LOG_MODE_MINIMAL=y
    ```
    Save `prj.conf`. `CONFIG_LOG_MODE_MINIMAL` enables a very basic, synchronous logging implementation, similar to `printk()`, with minimal memory overhead. It disables most other logger features like deferred processing, timestamps, colors, and module prefixes.

6.  **Perform a Pristine Build** (recommended when changing fundamental modes like this) and flash the application.
7.  Observe the significantly different log output in the terminal:
    ```terminal
    I: nRF Connect SDK Fundamentals
    I: Exercise 2
    D: A log message in debug level
    W: A log message in warning level!
    E: A log message in Error level!
    I: Sample Data!
                      00 01 02 03 04 05 06 07  48 65 6c 6c 6f | ........ Hello
    ```
    Log levels are indicated by single letters, and timestamps/module names are gone.

The solution for this exercise can be found in the GitHub repository, in `l4/l4_e3_sol` (select your version subfolder).

---