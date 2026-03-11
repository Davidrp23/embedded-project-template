# Serial communication (UART)

## Overview

Universal Asynchronous Receiver/Transmitter (UART) is a popular serial communication protocol. It is used to communicate with a variety of sensors, electronic components, and is also commonly used as the back-end for consoles through USB-to-UART convertors. In this lesson, we will learn how to use the UART driver in an interrupt-driven fashion so that when new data arrives the application is interrupted and a callback function (ISR) is called.

In the exercise section of this lesson, we will control the LEDs on the board by sending commands over UART.

### UART Protocol

UART is a peer-to-peer (P2P) hardware communication protocol where one end can be an MCU (microcontroller) and the other end can be another MCU, a sensor or a PC (through a USB-to-UART converter).

> **Note**: All Nordic development kits have a USB-to-UART converter onboard to allow serial communication to a PC.

Data transfer is done serially. It starts with a starting bit, usually by driving logic low for one clock cycle. In the next n clock cycles, n bits are sent sequentially from the transmitter (n is usually 8). Optionally, 1 parity bit can be added to improve transfer reliability. In the end, the data wire is usually pulled up high to indicate the end of transfer.

> **Definition: Parity bit**
> A parity bit describes the evenness or oddness of the data and is a way for the receiver to tell if the data has changed during transmission.

*(Diagram description from original text: UART timing diagram)*

UART has one connection pin for transmitted data, usually called `TX`, and another for received data, called `RX`. These connections are cross-coupled between a transmitter and a receiver. So the `TX` on one device is connected to the `RX` on the remote device and vice versa. `GND` stands for ground.

*(Diagram description from original text: Basic UART connection diagram)*

The `RX` pin senses when a start bit has been initiated and automatically clocks in to store the new word. The data rate that the receiver and transmitter will operate at must be selected in advance; this is known as the **baud rate**. Common baud rate values for UART are 115200 or 9600 bits/s (bauds).

It is also possible to use hardware flow control with UART using two extra lines called `RTS` (request to send) and `CTS` (clear to send).

*(Diagram description from original text: UART connection with hardware flow control diagram)*

These wires are cross-coupled between the two devices. If hardware flow control is enabled, each end will use its `RTS` to indicate that it is ready to receive new data and read its `CTS` to see if it is allowed to send data to the other end.

---

## UART Driver

*(Information relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

In Zephyr, there are three different ways to access the UART peripheral, all with different API functions: polling, interrupt-driven and asynchronous. In this lesson, we will be covering the **asynchronous API**.

*   The **Asynchronous API** is the most efficient way to use UART. It allows you to read and write data in the background using EasyDMA. In addition, the asynchronous mode supports features that allow us to enable receive timeouts and control the amount of data received before an interrupt is triggered. The asynchronous API is quite powerful and covers most use-cases.
*   **Polling** is the most basic method. The reading function, `uart_poll_in()`, is non-blocking and returns a character or -1. The writing function, `uart_poll_out()`, is blocking. (Not covered in this course).
*   With the **Interrupt-driven API** (raw interrupts), the UART driver ISR manages data, while the application continues other tasks. Kernel Data Passing features (e.g., FIFO) can communicate between the application and driver. (Not covered in this course).

> **Important**: The interrupt-driven API and the asynchronous API should NOT be used at the same time for the same hardware peripheral, since both APIs require hardware interrupts to function properly. Using callbacks for both APIs would result in interference between each other. `CONFIG_UART_EXCLUSIVE_API_CALLBACKS` is enabled by default so that only the callbacks associated with one API is active at a time.

You can learn more about the other APIs [here](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/hardware/peripherals/uart.html#api-reference).

### Enabling driver

1.  Enable the serial driver (UART driver) by adding these two lines in `prj.conf`:
    ```kconfig
    CONFIG_SERIAL=y
    CONFIG_UART_ASYNC_API=y
    ```
    The first line is usually enabled by default via the board's configuration. The second line is important to enable the asynchronous API.

2.  Include the header file of the UART driver in your source code:
    ```c
    #include <zephyr/drivers/uart.h>
    ```
3.  Instantiate the peripheral as a device pointer using `DEVICE_DT_GET()`. The UART driver does not have an API-specific structure like `gpio_dt_spec`.
    ```c
    // Assuming uart0 is the node label for the desired UART peripheral
    const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));

    if (!device_is_ready(uart)) {
        printk("UART device not found!");
        return; // Or handle error appropriately
    }
    ```
    The pointer `uart` of type `struct device` is used when interacting with the UART API. `uart0` is the devicetree node label representing the hardware controller.

    *(Description of image showing how to find UART node in VS Code DeviceTree view)*

    Note that the default speed (baud rate) in the devicetree is often 115200.

### UART Configurations

1.  UART configurations like baudrate and parity can be configured statically (at build time via devicetree) or dynamically (at run time), as `CONFIG_UART_USE_RUNTIME_CONFIGURE` is enabled by default.

    To change configurations dynamically, create a variable of type `uart_config`:
    *(Description of image: uart_config struct definition)*
    ```c
    const struct uart_config uart_cfg = {
            .baudrate = 115200,
            .parity = UART_CFG_PARITY_NONE,
            .stop_bits = UART_CFG_STOP_BITS_1,
            .data_bits = UART_CFG_DATA_BITS_8,
            .flow_ctrl = UART_CFG_FLOW_CTRL_NONE
        };
    ```
    The header file `uart.h` has enumerations for all available options.

    Then, call `uart_configure()` with the device pointer and the config struct:
    *(Description of image: uart_configure() function signature)*
    ```c
    int err = uart_configure(uart, &uart_cfg);
    if (err == -ENOSYS) {
        // Runtime configuration not supported, device may already be configured
        // Or handle other errors
        return -ENOSYS;
    } else if (err) {
        // Handle other configuration errors
        return err;
    }
    ```

2.  **Define the application callback function for UART.**
    > **Note**: A callback function (ISR) runs asynchronously in response to interrupts, typically at a higher priority than threads. Keep ISRs short and defer complex processing to threads (covered in Lesson 7) to maintain system responsiveness.

    Choose the UART events relevant to your application. Available events include:

    | Event                  | Description                                                                          |
    | :--------------------- | :----------------------------------------------------------------------------------- |
    | `UART_TX_DONE`         | The whole TX buffer was transmitted.                                                 |
    | `UART_TX_ABORTED`      | Transmitting aborted due to timeout or `uart_tx_abort()` call.                        |
    | `UART_RX_RDY`          | Data received and either RX timeout occurred (if enabled) or RX buffer is full.     |
    | `UART_RX_BUF_REQUEST`  | Driver requests the next buffer for continuous reception.                            |
    | `UART_RX_BUF_RELEASED` | A previously provided RX buffer is no longer used by the driver.                    |
    | `UART_RX_DISABLED`     | Receiver has been stopped/disabled and can be re-enabled using `uart_rx_enable()`.    |
    | `UART_RX_STOPPED`      | RX has stopped due to an external event (e.g., line break or error condition). After this event, `UART_RX_BUF_RELEASED` and `UART_RX_DISABLED` events will follow. |

    The callback function should have the following signature:
    ```c
    static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
    {
        switch (evt->type) {

        case UART_TX_DONE:
            // Handle TX completion
            printk("UART_TX_DONE event\n");
            break;

        case UART_TX_ABORTED:
            // Handle TX abort
            printk("UART_TX_ABORTED event\n");
            break;

        case UART_RX_RDY:
            // Handle received data ready
            printk("UART_RX_RDY event: %d bytes received\n", evt->data.rx.len);
            // Access data via evt->data.rx.buf + evt->data.rx.offset
            break;

        case UART_RX_BUF_REQUEST:
            // Handle buffer request for continuous reception
            printk("UART_RX_BUF_REQUEST event\n");
            break;

        case UART_RX_BUF_RELEASED:
            // Handle buffer release
            printk("UART_RX_BUF_RELEASED event for buffer at %p\n", evt->data.rx_buf.buf);
            break;

        case UART_RX_DISABLED:
            // Handle receiver disabled (e.g., re-enable for continuous reception)
            printk("UART_RX_DISABLED event\n");
            // Example: uart_rx_enable(dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
            break;

        case UART_RX_STOPPED:
            // Handle receiver stopped due to external event
            printk("UART_RX_STOPPED event, reason: %d\n", evt->data.rx_stop.reason);
            break;

        default:
            break;
        }
    }
    ```
    Only include the `case` statements relevant to your application.

    The `uart_event` struct contains the event type and a union holding event-specific data:
    *(Description of image: uart_event struct definition)*

    The `rx` member (for `UART_RX_RDY`) is of type `uart_event_rx`:
    *(Description of image: uart_event_rx struct definition)*

3.  **Register the callback function** using `uart_callback_set()`:
    *(Description of image: uart_callback_set() function signature)*
    ```c
    err = uart_callback_set(uart, uart_cb, NULL); // Pass NULL for user_data for now
    if (err) {
        // Handle callback registration error
        return err;
    }
    ```

### Receiving

Steps to receive data using the asynchronous API:

1.  **Declare a receive buffer.** Choose the size and type based on application needs. For simple examples, a `uint8_t` buffer might suffice. More complex applications might use FIFOs or circular buffers (covered later).
    ```c
    #define RECEIVE_BUFF_SIZE 10
    static uint8_t rx_buf[RECEIVE_BUFF_SIZE]; // A buffer to store incoming UART data
    ```
2.  **Start receiving** by calling `uart_rx_enable()`. Pass the buffer address, size, and an inactivity timeout (in microseconds). The timeout triggers `UART_RX_RDY` if some data arrives but the buffer doesn't fill completely within that time. Use `SYS_FOREVER_US` to disable the timeout (event only triggers when buffer is full).
    ```c
    #define RECEIVE_TIMEOUT 100 // microseconds

    err = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
    if (err) {
        // Handle error enabling RX
        return err;
    }
    ```
    *(Description of image: uart_rx_enable() function signature)*
    This function returns immediately; data reception happens in the background via ISR/DMA.

    > **Note**: RX timeout is counted from the last byte received. If no data was received, there won't be any timeout event.

3.  **Access received data** within the callback function when a `UART_RX_RDY` event occurs:
    *   **Data Length:** `evt->data.rx.len`
    *   **Offset in Buffer:** `evt->data.rx.offset` (where the new data starts)
    *   **Actual Data:** Access bytes from `evt->data.rx.buf[evt->data.rx.offset]` up to `evt->data.rx.buf[evt->data.rx.offset + evt->data.rx.len - 1]`

4.  **Handle continuous reception (optional).** By default, `uart_rx_enable` sets up a single buffer reception. Once the buffer is full (or timeout occurs), reception stops. To receive continuously, you must re-enable reception, typically within the `UART_RX_DISABLED` event handler in your callback:
    ```c
    // Inside the callback's switch statement:
    case UART_RX_DISABLED:
        uart_rx_enable(dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
        break;
    ```
    > **Note**: The asynchronous API supports chained buffer reception for seamless continuous receiving. Handle the `UART_RX_BUF_REQUEST` event by calling `uart_rx_buf_rsp()` to provide the *next* buffer. The driver automatically switches when the current buffer is full.

### Transmitting

Steps to transmit data using the asynchronous API:

1.  **Define a transmission buffer** holding the data to send.
    ```c
    static uint8_t tx_buf[] = {"nRF Connect SDK Fundamentals Course \n\r"};
    ```
2.  **Call `uart_tx()`** to initiate the transmission.
    *(Description of image: uart_tx() function signature)*
    The timeout parameter (last argument) is typically only effective if hardware flow control is enabled. Use `SYS_FOREVER_US` to disable the timeout.
    ```c
    err = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
    if (err) {
        // Handle error initiating TX
        return err;
    }
    ```
    This function returns immediately; data transmission happens in the background.

3.  **(Optional) Handle transmission completion.** If needed, take action when the entire buffer has been sent by handling the `UART_TX_DONE` event in the callback function.
    ```c
    // Inside the callback's switch statement:
    case UART_TX_DONE:
        printk("Transmission finished!\n");
        // Optionally free buffer, start next transmission, etc.
        break;
    ```

---

## Exercise 1: Controlling LEDs through UART

*(Instructions for nRF Connect SDK v3.0.0 - v3.2.0)*

In this exercise, you will learn how to control the LEDs on your development kit through UART using the asynchronous API. We will utilize the default UART controller connected to the interface MCU for USB-to-UART communication.

We will set up UART receive with a 10-byte buffer and a 100-microsecond inactivity timeout. A `UART_RX_RDY` event will trigger either when 10 bytes arrive or when at least one byte arrives followed by 100 microseconds of inactivity.

> **Important**: This exercise is not supported with the Thingy:53.

### Exercise steps

1.  In the GitHub repository for this course, open the base code for this exercise, found in `l5/l5_e1` (select your version subfolder). Open it as an existing application in VS Code. This code is based on the blinky sample.
2.  Enable the serial driver and asynchronous API in `prj.conf`:
    ```kconfig
    # STEP 2: Enable UART Async API
    CONFIG_SERIAL=y
    CONFIG_UART_ASYNC_API=y
    ```
    Save the file. (`CONFIG_SERIAL=y` might already be implicitly enabled by the board config).
3.  Include the UART driver header file in `src/main.c`:
    ```c
    // STEP 3: Include UART driver header
    #include <zephyr/drivers/uart.h>
    ```
4.  Get the device pointer for the UART hardware and verify it's ready. Add inside `main()`:
    4.1 Get the device pointer (use `uart0` for nRF5340 DK and most other DKs):
    ```c
    // STEP 4.1: Get UART device
    const struct device *uart = DEVICE_DT_GET(DT_NODELABEL(uart0));
    ```
    4.2 Verify readiness:
    ```c
    // STEP 4.2: Check UART readiness
    if (!device_is_ready(uart)) {
        printk("UART device not ready\r\n");
        return 1; // Indicate error
    }
    ```
5.  Get the device pointers for the LEDs (0-2) and verify readiness. Add inside `main()`:
    5.1 Get LED specs (adjust for boards with fewer LEDs like nRF7002 DK):
    ```c
    // STEP 5.1: Get LED devices
    #if defined(CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP) // nRF7002 DK has 2 LEDs
        static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
        static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
    #else // Other DKs usually have at least 3 LEDs
        static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
        static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(DT_ALIAS(led1), gpios);
        static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(DT_ALIAS(led2), gpios);
    #endif
    ```
    5.2 Verify GPIO port readiness (only need to check one as they share the port):
    ```c
    // STEP 5.2: Check LED GPIO readiness (only need to check one)
    if (!gpio_is_ready_dt(&led0)) {
        printk("GPIO device is not ready\r\n");
        return 1; // Indicate error
    }
    ```
6.  Configure the LED GPIO pins as outputs. Add inside `main()`:
    ```c
    // STEP 6: Configure LEDs
    int ret;
    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) { return 1; }
    ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) { return 1; }
    #if !defined(CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP)
        ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
        if (ret < 0) { return 1; }
    #endif
    // Optional: Turn LEDs ON initially
    gpio_pin_set_dt(&led0, 1);
    gpio_pin_set_dt(&led1, 1);
    #if !defined(CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP)
        gpio_pin_set_dt(&led2, 1);
    #endif
    ```
7.  Define the application callback function for UART. Add this function definition *before* `main()`:
    ```c
    // STEP 7: Define UART callback function
    static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
    {
        switch (evt->type) {
        case UART_RX_RDY:
            // Check if we received one byte ('1', '2', or '3')
            if ((evt->data.rx.len) == 1) {
                if (evt->data.rx.buf[evt->data.rx.offset] == '1') {
                    gpio_pin_toggle_dt(&led0);
                } else if (evt->data.rx.buf[evt->data.rx.offset] == '2') {
                    gpio_pin_toggle_dt(&led1);
                } else if (evt->data.rx.buf[evt->data.rx.offset] == '3') {
                    #if !defined(CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP)
                        gpio_pin_toggle_dt(&led2);
                    #endif
                }
            }
            break;
        case UART_RX_DISABLED:
            // Re-enable reception for continuous operation
            uart_rx_enable(dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
            break;
        default:
            break;
        }
    }
    ```
    > **Note**: The check `(evt->data.rx.len) == 1` assumes a terminal in character mode (like nRF Terminal, PuTTY default) where pressing '1' sends just '1'. Line mode terminals might send extra characters like '\r\n', requiring adjustment to the length check and parsing logic.

8.  Register the UART callback function. Add inside `main()` after configuring LEDs:
    ```c
    // STEP 8: Register UART callback
    ret = uart_callback_set(uart, uart_cb, NULL);
    if (ret) {
        printk("Failed to set UART callback");
        return 1;
    }
    ```
9.  Transmit a welcome message over UART. Add inside `main()`:
    9.1 Define the transmission buffer (adjust message based on board):
    ```c
    // STEP 9.1: Define TX buffer
    #if defined(CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP)
        static uint8_t tx_buf[] = {"\n\rPress 1 or 2 to toggle LEDS 1 or 2 respectively\n\r"};
    #else
        static uint8_t tx_buf[] = {"\n\rPress 1, 2 or 3 to toggle LEDS 1, 2 or 3 respectively\n\r"};
    #endif
    ```
    9.2 Send the data using `uart_tx()`:
    ```c
    // STEP 9.2: Transmit welcome message
    ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
    if (ret) {
        printk("Failed to send UART message");
        return 1;
    }
    ```
10. Start receiving data. Add inside `main()`:
    10.1 Define receive buffer size and timeout:
    ```c
    // STEP 10.1.1: Define RX buffer size
    #define RECEIVE_BUFF_SIZE 10
    // STEP 10.2: Define RX timeout
    #define RECEIVE_TIMEOUT 100 // us
    ```
    10.1.2 Define the receive buffer (needs to be accessible by callback, so make static or global):
    ```c
    // STEP 10.1.2: Define RX buffer (static)
    static uint8_t rx_buf[RECEIVE_BUFF_SIZE];
    ```
    10.3 Start receiving using `uart_rx_enable()`:
    ```c
    // STEP 10.3: Enable UART RX
    ret = uart_rx_enable(uart, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
    if (ret) {
        printk("Failed to enable UART RX");
        return 1;
    }
    ```
11. Build the application and flash it to your development kit.
12. **Testing:**
    *   Observe that the LEDs turn on initially.
    *   Open a serial terminal (e.g., nRF Terminal, PuTTY) connected to your board's COM port at 115200 baud (8N1, no flow control).
    *   You should see the welcome message printed.
    *   Type a number (`1`, `2`, or `3`) in the terminal. The corresponding LED on the DK should toggle.

    > **Note**: If using Thingy:91, LED1=Red, LED2=Green, LED3=Blue.

The solution for this exercise can be found in the GitHub repository, `l5/l5_e1_sol` (select your version subfolder).
