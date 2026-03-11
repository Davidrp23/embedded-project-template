# Debugging and troubleshooting

*(Updated for nRF Connect SDK v3.0.0 - v3.2.0)*

## Overview

Being able to quickly and efficiently debug an application when met with an issue is a useful skill for any developer. In this lesson, we will cover some debugging techniques that can be useful when developing applications with the nRF Connect SDK. First, we will go over more debugging functionality available in nRF Connect for VS Code and go through build errors and fatal errors, and how to debug them using tools like core dump and addr2line. Then we will cover how to troubleshoot the devicetree, and touch upon some physical debugging techniques.

In the exercise portion of this lesson, you will practice using advanced debugging features in nRF Connect for VS Code, and write and read directly to the SoC/SiP peripherals. Then, you will create an application that triggers a fault error, and practice using core dump and addr2line to debug.

> **Note on Documentation URLs (v3.0.0+):** Nordic Semiconductor documentation has moved from `developer.nordicsemi.com` to `docs.nordicsemi.com`. The new URL structure is:
> - Latest: `https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/...`
> - Version-specific: `https://docs.nordicsemi.com/bundle/ncs-X.X.X/page/nrf/...`

---

## Debugging in nRF Connect for VS Code

Debugging, in the context of embedded development, is the process of identifying, analyzing and resolving issues related to software and hardware. Debugging is an iterative process where the developer analyzes the code or hardware and makes small adjustments before they test their application again. Common problems you might face during development are syntax errors, logical errors, memory leaks, timing issues, and hardware-related issues.

The nRF Connect SDK has multiple tools that can help when debugging your application. In this chapter, we will go through some of the most useful debugging tools and how to use them.

### Preparing for debugging

To be able to debug your nRF Connect SDK application, you must configure it correctly before building.

In the build configuration menu, under **Optimization level**, select the option **Optimize for debugging (-Og)**. This option enables additional extension-specific debugging features after the application is built.

*(Description of image: Setting optimization level in VS Code build configuration)*

When this optimization is set, the following Kconfigs are enabled in the build:

*   `CONFIG_DEBUG_OPTIMIZATIONS`: Limits compiler optimizations to those that do not significantly impact debugging.
*   `CONFIG_DEBUG_THREAD_INFO`: Adds additional information to thread objects so the debugger can discover threads and their states.

Other Kconfigs that can be useful when debugging:

*   `CONFIG_I2C_DUMP_MESSAGES`: Enables logging of all I2C messages.
*   `CONFIG_ASSERT`: Enables the `__ASSERT()` macro in kernel code. If an assertion fails, the default `assert_post_action()` triggers a fatal error.

It’s worth noting that enabling optimization for debugging will affect code size. If this is problematic, consider enabling only specific debug features needed (e.g., `CONFIG_THREAD_NAME`, `CONFIG_DEBUG`). Additional debug Kconfigs exist for specific network stacks.

### Pristine Build

Outputs are created at CMake configuration time and are not always regenerated when inputs change. If facing build issues, especially after configuration changes, deleting the build folder and performing a **pristine build** is often a good first step.

> **Note (nRF Connect SDK v3.0.0+):** The `west flash` command now uses **nRF Util** instead of `nrfjprog`. The nRF Command Line Tools are deprecated in favor of the nRF Util utility. Ensure nRF Util is installed if the flash action in VS Code does not work.

### Breakpoints

A **breakpoint** is a place in the code where execution will pause, allowing you to examine program state, variable values, and the call stack.

*(Description of images: Adding a breakpoint and execution halted at a breakpoint)*

VS Code supports advanced breakpoints:
*   **Conditional Breakpoints:** Trigger only when a specific expression evaluates to true.
*   **Logpoints:** Print a specified message to the debug console instead of halting execution.

Right-click on the gutter (next to line numbers) or an existing breakpoint to add/edit these types.

*(Description of images: Editing/adding advanced breakpoint types)*

### Monitor mode debugging

The debugger can run in two modes:

*   **Halt mode:** The default mode. Halts the entire CPU when a debug event (breakpoint hit, step command) occurs.
*   **Monitor mode:** Allows the CPU to continue running crucial functions (like timing-critical protocols) while handling debug requests through dedicated debug interrupts. Useful for debugging applications like Bluetooth LE or PWM where halting would disrupt operation. Logpoints are particularly effective in monitor mode.

To enable monitor mode:
1.  Add Kconfigs to `prj.conf`:
    ```kconfig
    CONFIG_CORTEX_M_DEBUG_MONITOR_HOOK=y
    CONFIG_SEGGER_DEBUGMON=y
    ```
2.  During a debug session, enter the following command in the VS Code Debug Console:
    ```
    -exec monitor exec SetMonModeDebug=1
    ```
    *(Description of image: Entering the command in Debug Console)*

### Variables view

Shows variables in the current scope (local, global, static). Values update only when execution is halted.

*(Description of image: Variables view in VS Code debugger)*

### Watch View

Displays the values of variables you explicitly choose to monitor. Values update when execution is halted.

To watch a variable:
1.  Highlight the variable in the code.
2.  Right-click -> Add to Watch.
3.  Run/step the debugger to see the value update in the Watch view.

*(Description of image: Watch view in VS Code debugger)*

### Peripherals View

Displays a memory map of device peripherals based on the SVD file. Shows register values and fields, allowing monitoring and direct modification (use with caution!) of hardware state during a debug session.

*(Description of image: Peripherals view showing GPIO registers)*

### Call stack

Lists active function calls for the currently halted thread. Useful for understanding how execution reached the current point. Also shows other threads and their states.

*(Description of image: Call stack view in VS Code debugger)*

### Thread Viewer

Provides detailed information about all Zephyr threads in the application: Name, State (Running, Ready, Pending, Suspended, etc.), Priority, Stack Usage (current and high water mark), Entry function, and Options. State is updated only when the debugger is halted.

*(Description of image: Thread Viewer in VS Code debugger)*

Complete documentation for the Thread Viewer [here](https://docs.nordicsemi.com/bundle/nrf-connect-vscode/page/reference/ui_debug.html#thread-viewer).

### Memory Explorer

Shows the raw memory content of the device. Allows inspection of different memory regions. You can go to specific addresses or symbols (global). Useful for examining data buffers, detecting memory corruption, or checking memory layout.

*(Description of image: Memory Explorer view)*

More information [here](https://docs.nordicsemi.com/bundle/nrf-connect-vscode/page/reference/ui_debug.html#memory-explorer).

---

## Build errors and fatal errors

*(Information relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

Issues can arise during building, testing, or runtime.

### Build errors

#### Build log

Found in the build output terminal in VS Code (`View` -> `Terminal` or `Ctrl + \`). The complete log is split across multiple files within the `build/CMakeFiles/` directory (`CMakeOutput.log`, `CMakeError.log`).

When facing build errors, start with the *first* error message in the log and work downwards. Fixing one error might resolve others or reveal new ones.

#### Troubleshooting procedure

1.  **Look for error/warning messages:** Scan the log for lines starting with "error" (critical) or "warning".
2.  **Read the message carefully:** Understand what went wrong and where (file/line number).
3.  **Identify the cause:** Common causes include:
    *   **Code issues:** Syntax errors (missing `;`, `}`), undefined variables. Check `errno.h` for runtime error codes.
    *   **Configuration problems:** Incorrect settings in `prj.conf`, devicetree (`.overlay`), Kconfig files.
    *   **Missing dependencies:** Libraries or components not found.
    *   **Incompatible components:** Conflicts between configurations or versions.
    *   **Sysbuild issues:** If using sysbuild, check configuration for *all* images (app core, net core, bootloader, etc.).
4.  **Fix the error:** Use resources like DevZone, SDK/Zephyr documentation, Stack Overflow.
5.  **Rebuild and repeat:** Building and debugging is iterative.

### Fatal errors

A **fatal error** occurs when the Zephyr kernel enters an unrecoverable state. These can be hard to debug, especially if they occur intermittently during runtime.

#### Addr2line

`addr2line` is a command-line tool (part of GCC, included in the SDK toolchain) that translates a memory address from an executable file into the corresponding source file name and line number.

When a fatal error occurs, the log often prints a "Faulting instruction address (r15/pc)". Use this address with `addr2line`.

Syntax:
```bash
addr2line [options] -e <executable_file> [addr...]
```
*   **`<executable_file>`:** The `zephyr.elf` file found in your build output directory (e.g., `build/zephyr/zephyr.elf`). `.elf` files contain symbol and debug information needed for translation, unlike `.hex` files.
*   **`[addr...]`:** One or more faulting addresses to translate.

Example:
```bash
# Find your toolchain's addr2line path first
# Example command (paths will vary):
/path/to/toolchain/arm-zephyr-eabi/bin/arm-zephyr-eabi-addr2line -e build/zephyr/zephyr.elf 0x000045a2
```
Output might be:
```
C:/ncs/v3.0.0/modules/hal/nordic/nrfx/drivers/src/nrfx_gpiote.c:668
```
Useful flags:
*   `-a`: Show address before translation.
*   `-f`: Show function name containing the location.
*   `-p`: Prettier, human-readable output.

(Practice in Exercise 2).

#### Core dump

**Core dump** captures the program's state (CPU registers, memory contents, stack) when a crash occurs. This data can be analyzed later, even without a live debugger connection.

To use core dumps:
1.  **Prevent immediate restart (optional but helpful):** Configure boot delay to 0 in `prj.conf` to see logs before potential reset loops.
    ```kconfig
    # Optional: Prevent quick resets on crash for viewing logs
    CONFIG_BOOT_BANNER=y
    CONFIG_BOOT_DELAY=y
    CONFIG_BOOT_DELAY_S=0
    CONFIG_BOOT_DELAY_MS=0
    CONFIG_BOOT_DELAY_SLEEPY_TICKS=n # Ensure non-sleepy delay
    CONFIG_BOOT_DELAY_SLEEPY_TIME=0
    ```
2.  **Enable Core Dump Backend:** Choose where to store/output the dump in `prj.conf`.
    *   **Logging Backend:** Prints dump data to the configured log output (e.g., UART).
        ```kconfig
        CONFIG_DEBUG_COREDUMP=y
        CONFIG_DEBUG_COREDUMP_BACKEND_LOGGING=y
        ```
    *   **Flash Partition Backend:** Stores dump data to a dedicated flash partition. Requires devicetree definition.
        ```kconfig
        # In prj.conf
        CONFIG_DEBUG_COREDUMP=y
        CONFIG_DEBUG_COREDUMP_BACKEND_FLASH_PARTITION=y
        # CONFIG_COREDUMP_FLASH_OFFSET=0x... # Offset is now read from partition

        # In devicetree overlay (*.overlay)
        &flash0 { /* Or appropriate flash controller */
                partitions {
                        /* Adjust partition address and size based on device/needs */
                        coredump_partition: partition@300000 {
                                label = "coredump-partition";
                                reg = <0x300000 DT_SIZE_K(4)>; /* Example: 4KB at 0x300000 */
                        };
                };
        };
        ```

3.  **Analyze the Dump:**
    *   **If using Logging Backend:**
        *   Capture the full log output containing the core dump data (lines between `#CD:BEGIN#` and `#CD:END#`).
        *   Save this text data to a file (e.g., `dump.log`).
        *   Use the Python script `zephyr/scripts/coredump/coredump_serial_log_parser.py` to convert `dump.log` to a binary format (`dump.bin`).
            ```bash
            python <sdk_path>/zephyr/scripts/coredump/coredump_serial_log_parser.py dump.log dump.bin
            ```
    *   **If using Flash Backend:** Use tools like `west` or `nrfjprog` to read the data from the defined flash partition and save it to a binary file (`dump.bin`).
    *   **Start GDB Server:** Use the Python script `zephyr/scripts/coredump/coredump_gdbserver.py` with the binary dump file and the `zephyr.elf` file.
        ```bash
        python <sdk_path>/zephyr/scripts/coredump/coredump_gdbserver.py build/zephyr/zephyr.elf dump.bin
        ```
        The server will wait for a GDB connection (default port 1234).
    *   **Start GDB Client:** Open a new terminal and run GDB for your target, pointing to the ELF file.
        ```bash
        # Find your toolchain's GDB path
        /path/to/toolchain/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb build/zephyr/zephyr.elf
        ```
    *   **Connect GDB Client to Server:** Inside the GDB client prompt, connect to the server:
        ```gdb
        target remote localhost:1234
        ```
    *   **Debug:** You can now use GDB commands like `bt` (backtrace), `info registers`, `p <variable>`, `x <address>` to examine the state at the time of the crash.

(Practice in Exercise 2).

---

## Troubleshooting the devicetree

Common issues related to devicetree configuration.

> **Recall**: Devicetree (`.dts`, `.dtsi`, `.overlay` files) describes hardware structure and initial configuration. Covered in Fundamentals Lesson 2.

Common errors and checks:

*   **`devicetree.h` not included:** While often included indirectly, sometimes you need `#include <zephyr/device.h>` or a driver header that includes it if using devicetree macros directly.
*   **`Undefined reference to __device_dts_ord_XX`:** Build error indicating a problem resolving a devicetree node, often due to incorrect configuration or missing enablement. The number `XX` corresponds to the node's ordinal ID.
*   **Verify which device failed:** Look up the ordinal ID (`XX`) in the generated header `build/zephyr/include/generated/devicetree_generated.h` to find the problematic node path.
*   **Is the device enabled?**
    *   Check the **compiled devicetree** (`build/zephyr/zephyr.dts`) to ensure the node's `status` property is `"okay"`. If not, enable it in your `.overlay` file.
    *   Ensure the corresponding **driver Kconfig** is enabled in `prj.conf` (e.g., `CONFIG_I2C=y` for I2C peripherals).
*   **Missing bindings:** If the build fails finding a binding, check if the node's `compatible` property matches an existing YAML binding file in `<sdk>/zephyr/dts/bindings` or `<sdk>/nrf/dts/bindings`. You might need to create a custom binding.
*   **Naming conventions:**
    *   In C code macros (`DT_...`), use node labels or aliases directly. When using paths (`DT_PATH`), replace special characters like `@` and `-` with underscores (`_`).
        ```c
        /* Correct C macro usage */
        #define MY_I2C_NODE DT_NODELABEL(i2c0) // Using label
        #define MY_OTHER_NODE DT_ALIAS(uart1) // Using alias
        #define MY_CLOCK_FREQ DT_PROP(DT_PATH(soc, i2c_12340000), clock_frequency) // Using path
        ```
    *   In `.overlay` files (which are DTS fragments), use the *exact* node names and property names as defined in the base `.dts`/`.dtsi` files (including `@`, `-`).
        ```devicetree
        /* Correct overlay usage */
        &i2c_12340000 { /* Node name might be i2c@12340000 in base DTS */
             clock-frequency = <100000>;
        };
        &uart1 { /* Referencing node via label/alias */
            current-speed = <9600>;
        };
        ```
*   **Validate properties:** If reading a property fails, ensure all *required* properties for the node's `compatible` are set in the devicetree (base + overlays). Check the binding YAML file for requirements.
*   **Check node existence:** Use preprocessor checks in C code to guard against missing nodes:
    ```c
    #if DT_NODE_HAS_STATUS(DT_ALIAS(my_optional_sensor), okay)
        // Code using the optional sensor
    #else
        #warning "Optional sensor not enabled"
    #endif
    ```

---

## Physical debugging

When the application builds and runs, but hardware doesn't behave as expected (e.g., communication errors, wrong sensor readings, incorrect RF performance).

Common tools:

*   **Digital Logic Analyzer (DLA):** Analyzes digital signals (UART, I2C, SPI, GPIOs). Displays logic states over time. Essential for debugging communication protocols.
*   **Power Profiler Kit II (PPK2):** Measures current consumption accurately. Helps identify unexpected power draw, verify sleep modes, and correlate power with events. Can also act as a simple 8-channel logic analyzer.
*   **Multimeter:** Basic checks for voltage levels, continuity, resistance. Useful for verifying power rails or checking for damaged components.
*   **Oscilloscope:** Visualizes voltage waveforms over time. Essential for analyzing signal integrity, noise, timing issues on analog or digital lines.

**Basic Troubleshooting Flowchart:**

1.  **Power Check:** Is the device powered correctly? Correct voltage? Stable supply? (Use Multimeter/PPK2/Oscilloscope).
2.  **Connectivity Check:** Correct COM port selected? Wires connected properly? Good solder joints? (Visual inspection, Multimeter continuity).
3.  **Simple Code Markers:** Use LEDs or GPIO toggles (measured with DLA/PPK2/Oscilloscope) to verify code execution flow if print logs are unavailable.
    ```c
    // Example: Toggle LED on error
    err = bt_le_scan_start(&scan_param, NULL);
    if (err) {
        gpio_pin_toggle_dt(&error_led); // Toggle specific LED on failure
        return;
    }
    ```
4.  **Power Profiling:** Is current consumption as expected? Unexpected high current might indicate peripherals not sleeping, short circuits, etc. (Use PPK2).
    > **Note**: For accurate low-power measurements, power the DK via the "External Supply" header, not USB, and ensure the debugger is logically detached (no active debug session).
5.  **Clock Check:** If using a custom board or specific low-power features, ensure the Low Frequency Clock (LFCLK) source is correctly configured (e.g., external crystal vs internal RC oscillator). DKs often use external LFCLK enabled by default. For internal RC on custom boards, enable relevant Kconfigs:
    ```kconfig
    # Example for using internal RC LFCLK
    CONFIG_CLOCK_CONTROL_NRF_K32SRC_RC=y
    CONFIG_CLOCK_CONTROL_NRF_K32SRC_CALIBRATION=y # Recommended for RC accuracy
    # Adjust ppm tolerance if needed
    # CONFIG_CLOCK_CONTROL_NRF_K32SRC_ACCURACY_500PPM=y
    ```

---

## Exercise 1 – Advanced debugging in nRF Connect for VS Code

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

This exercise explores interacting with peripherals directly via the debugger and using advanced breakpoint features and monitor mode debugging.

### Exercise steps

#### Part 1 – Peripheral interaction

Open the exercise base code: `l2/l2_e1` (select version) using "Copy a sample".

1.  **Add Build Configuration:** Create a build configuration for your board. Crucially, select **Optimize for debugging (-Og)** in the Optimization level dropdown. Build the configuration.
2.  **Start Debug Session:** Click the Debug action in the VS Code nRF Connect view. Execution pauses at `main()`.
3.  **Control LED via Peripherals View:**
    3.1. Identify the GPIO pin for LED1 on your DK (e.g., check silk screen, documentation). For nRF52840 DK, it's `P0.13`.
    3.2. In the **Run and Debug** view -> **PERIPHERALS** panel, find the corresponding GPIO peripheral (e.g., `GPIO P0`).
    3.3. Configure the pin direction: Expand `GPIO P0` -> `DIR`. Find the pin number (e.g., `PIN13`). Right-click -> Set Value -> Select `Output`.
    3.4. Toggle the pin output: Expand `GPIO P0` -> `OUT`. Find the pin number (e.g., `PIN13`). Right-click -> Set Value -> Enter `1` (or `0`). Observe the physical LED toggling.

#### Part 2 – Advanced debugging techniques

4.  **Add basic logic:** In `src/main.c`, inside `main()` before the `while(1)` loop (if present) or just inside `main`, add:
    ```c
    #include <zephyr/logging/log.h> // Make sure logging is included/enabled
    LOG_MODULE_REGISTER(Lesson2_Exercise1); // Register module if not done

    int8_t test_var = 124; // Initialize close to overflow
    LOG_INF("Starting Exercise 1!");
    for (int i = 0; i < 10; i++)
    {
        test_var = test_var + 1;
        LOG_INF("test_var = %d", test_var);
    }
    ```
5.  **Build and Flash.** Connect the serial terminal. Observe the output:
    ```terminal
    *** Booting nRF Connect SDK ***
    Starting Exercise 1!
    [timestamp] <inf> Lesson2_Exercise1: test_var = 125
    [timestamp] <inf> Lesson2_Exercise1: test_var = 126
    [timestamp] <inf> Lesson2_Exercise1: test_var = 127
    [timestamp] <inf> Lesson2_Exercise1: test_var = -128 // Overflow!
    [timestamp] <inf> Lesson2_Exercise1: test_var = -127
    ...
    ```
6.  **Debug the overflow:**
    6.1. **Add Conditional Breakpoint:** Right-click the gutter next to the line `test_var = test_var + 1;`. Select `Add Conditional Breakpoint...`. In the expression field, enter `test_var == 127`. Press Enter.
    6.2. Add a regular breakpoint at the start of `main()`.
    6.3. Start Debug Session. Execution stops at `main()`.
    6.4. **Use Memory Explorer:** Go to Run and Debug view -> MEMORY EXPLORER -> Data tab. Click the "Go to Symbol" icon (looks like `@`). Enter `test_var`. Observe the memory location and surrounding area.
    6.5. **Continue (F5).** Execution stops at the conditional breakpoint, *just before* the overflow occurs (when `test_var` is 127). Examine `test_var` in Variables/Watch/Memory Explorer.
    6.6. **Step Over (F10).** Execute the `test_var = test_var + 1;` line. Observe `test_var` change to -128 in the debugger views due to signed 8-bit integer overflow.
       > **Note**: Integer overflow is undefined behavior in C. The wrap-around seen here is common but not guaranteed. Code should prevent overflow using checks (e.g., `if (test_var < INT8_MAX)`).

7.  **(Optional - Monitor Mode & Logpoints - nRF52 only in original text):**
    7.1 Add Kconfigs to `prj.conf`:
       ```kconfig
       CONFIG_CORTEX_M_DEBUG_MONITOR_HOOK=y
       CONFIG_SEGGER_DEBUGMON=y
       CONFIG_DK_LIBRARY=y # Enable DK Buttons & LEDs library
       ```
    7.2 Include DK library header in `main.c`: `#include <dk_buttons_and_leds.h>`
    7.3 Add button handling code (init function and callback):
       ```c
       // Callback for button presses
       static void button_changed(uint32_t button_state, uint32_t has_changed)
       {
           if (has_changed & DK_BTN1_MSK) { // Check if Button 1 changed
               if (button_state & DK_BTN1_MSK) { // Check if Button 1 is pressed
                    dk_set_led_on(DK_LED1); // Turn on LED 1
               } else {
                    dk_set_led_off(DK_LED1); // Turn off LED 1 (optional)
               }
           }
       }
       // Initialization function for buttons
       static int init_button(void)
       {
           int err = dk_buttons_init(button_changed);
           if (err) {
               printk("Cannot init buttons (err: %d)\n", err);
           }
           return err;
       }
       // Call these from main():
       // err = dk_leds_init(); /* handle error */
       // err = init_button(); /* handle error */
       ```
    7.4 Build with debug optimization.
    7.5 **Add Logpoint:** Find the definition of `dk_set_led()` (Ctrl+Click `dk_set_led_on`, then Ctrl+Click `dk_set_led`). Right-click the gutter inside `dk_set_led` -> Add Logpoint... -> Enter message: `DK Set LED: led_idx={led_idx}`.
    7.6 Start Debug Session.
    7.7 **Enable Monitor Mode:** In the Debug Console, type `-exec monitor exec SetMonModeDebug=1` and press Enter.
    7.8 **Resume (F5).** Press Button 1 on the DK. Observe the logpoint message appearing in the **Debug Console** without halting execution.

---

## Exercise 2 – Debugging with core dump and addr2line

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

This exercise demonstrates capturing and analyzing a core dump after intentionally crashing the application, using the logging backend and the `addr2line` tool.

> **Important**: Requires Python 3. Might need `pip install pyelftools`.

### Exercise steps

Open the exercise base code: `l2/l2_e2` (select version) using "Copy a sample".

#### Debugging with core dump

1.  **Enable Core Dump (Logging Backend):**
    1.1 Add to `prj.conf`:
       ```kconfig
       # STEP 1.1: Enable Coredump via Logging
       CONFIG_DEBUG_COREDUMP=y
       CONFIG_DEBUG_COREDUMP_BACKEND_LOGGING=y
       # Optional: Prevent rapid resets to see logs
       CONFIG_BOOT_DELAY=y
       CONFIG_BOOT_DELAY_MS=1000
       ```
    1.2 Include header in `main.c`:
       ```c
       // STEP 1.2: Include coredump header
       #include <zephyr/debug/coredump.h>
       ```
2.  **Add Crash Functionality:**
    2.1 Define `crash_function` (before `main`) to cause a fault:
       ```c
       // STEP 2.1: Define function to cause crash
       void crash_function(uint32_t *addr)
       {
           LOG_INF("Button pressed at %" PRIu32, k_cycle_get_32());
           LOG_INF("Coredump: %s", CONFIG_BOARD);

           #if !defined(CONFIG_CPU_CORTEX_M)
               /* For non-Cortex-M (or non-TrustZone): Dereference NULL */
               *addr = 0;
           #else
               ARG_UNUSED(addr);
               /* For Cortex-M (safer for TrustZone): Undefined instruction fault */
               __asm__ volatile("udf #0" :::);
           #endif
       }
       ```
    2.2 Call `crash_function(0);` inside `button_handler()` (or the button callback you are using) when Button 1 is pressed.
3.  **Build and Flash.**
    > **Important**: There was a note about a potential bug using coredump with debug optimization. Build *without* `-Og` if issues arise, although addr2line works best with debug symbols. Try with optimization first.
4.  Connect serial terminal. Press Button 1. Observe crash log and core dump output:
    ```terminal
    [timestamp] <inf> Lesson2_Exercise2: Button 1 pressed
    [timestamp] <inf> Lesson2_Exercise2: Button pressed at ...
    [timestamp] <inf> Lesson2_Exercise2: Coredump: nrf52840dk_nrf52840
    [timestamp] <err> os: ***** USAGE FAULT *****
    [timestamp] <err> os:   Attempt to execute undefined instruction
    ... (register dump) ...
    [timestamp] <err> os: Faulting instruction address (r15/pc): 0x000003ea
    [timestamp] <err> os: >>> ZEPHYR FATAL ERROR ...
    [timestamp] <err> coredump: #CD:BEGIN#
    [timestamp] <err> coredump: #CD:<hex data>
    ...
    [timestamp] <err> coredump: #CD:<hex data>
    [timestamp] <err> coredump: #CD:END#
    ```
5.  **Analyze Output:** Note the fault type (Usage Fault), reason, register values, and the crucial **Faulting instruction address**.
6.  **Copy Core Dump:** Select and copy all lines from `#CD:BEGIN#` to `#CD:END#` (inclusive) from the terminal. Save this text into a file named `dump.log` in your project folder (`l2/l2_e2`).
7.  **Convert Log to Binary:** Open a terminal in your project folder. Run the parser script (adjust SDK path):
    ```bash
    python <sdk_path>/zephyr/scripts/coredump/coredump_serial_log_parser.py dump.log dump.bin
    ```
8.  **Start GDB Server:** Run the server script with the ELF file and binary dump (adjust paths):
    ```bash
    python <sdk_path>/zephyr/scripts/coredump/coredump_gdbserver.py build/zephyr/zephyr.elf dump.bin
    ```
    It will wait for a connection on port 1234.
9.  **Start GDB Client:** Open *another* terminal in the project folder. Run GDB (adjust toolchain path):
    ```bash
    <toolchain_path>/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb build/zephyr/zephyr.elf
    ```
10. **Connect Client:** In the GDB prompt, type:
    ```gdb
    target remote localhost:1234
    ```
    You should connect and see the location where the crash occurred (e.g., inside `crash_function`).
11. **Get Backtrace:** In the GDB prompt, type `bt`:
    ```gdb
    (gdb) bt
    #0  crash_function (...) at ../src/main.c:XX
    #1  button_handler (...) at ../src/main.c:YY
    #2  ... (ISR handler internals) ...
    ```
    This shows the function call sequence leading to the crash.

#### Debugging with addr2line

Use the faulting address noted in step 5.

12. **Find `addr2line`:** Locate `arm-zephyr-eabi-addr2line` (or `.exe`) within your installed toolchain's `bin` directory.
13. **Run `addr2line`:** Open a terminal in your project folder. Execute (adjust paths, use the correct ELF path for sysbuild if applicable):
    ```bash
    # Example for non-sysbuild, adjust paths
    <toolchain_path>/arm-zephyr-eabi/bin/arm-zephyr-eabi-addr2line -e build/zephyr/zephyr.elf 0x000003ea
    ```
    *(Note: If using Sysbuild, the ELF path is often `build/<app_folder_name>/zephyr/zephyr.elf`)*
14. **Analyze Output:** The tool prints the file and line number corresponding to the address:
    ```
    <project_path>/src/main.c:61
    ```
    Looking at line 61 reveals the `__asm__ volatile("udf #0" :::);` instruction, confirming the source of the crash.

**Comparison:** Core dump provides a full state snapshot (registers, stack, memory) analyzable with GDB, good for complex issues or remote analysis. `addr2line` is simpler, requiring only the address and ELF file to pinpoint the exact line of code causing the fault.

#### Core dump use cases

*   Local debugging (using VS Code debugger) is generally preferred during active development.
*   Core dumps are valuable for:
    *   Debugging issues on **field-deployed devices** without direct debugger access.
    *   **Post-mortem analysis** of crashes.
    *   Intermittent faults hard to catch with a debugger.
*   Core dumps can be stored on-device (flash) and transmitted later (e.g., via Memfault over Cellular/Wi-Fi/BLE) for remote analysis.
*   **Best Practice:** Use local debugging during development, consider enabling core dump capability (possibly stored to flash) for production/field units.

---

## Exercise 3 – Debugging the devicetree

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

This exercise focuses on identifying and fixing a common devicetree-related build error.

### Exercise steps

Open the exercise base code: `l2/l2_e3` (select version) using "Copy a sample". This is a simple application intended to print a message on button press.

1.  **Build the application.** It should fail with an error similar to:
    ```
    error: '__device_dts_ord_11' undeclared here...
    ...
    note: in expansion of macro 'GPIO_DT_SPEC_GET'
    note: in expansion of macro 'DEVICE_DT_GET'
    ...
    ```
2.  **Find the Node ID:** Note the failing ordinal ID from the error message (e.g., `11`).
3.  **Investigate:** Open the generated file `build/zephyr/include/generated/devicetree_generated.h`. Find the line corresponding to the ordinal ID (e.g., `11`).
    ```
    * Node dependency ordering (ordinal and path):
    *   ...
    *   10  /buttons
    *   11  /soc/gpio@50000000 // <--- ID 11 corresponds to gpio0
    *   12  /buttons/button_0
    *   ...
    ```
    The error relates to the `gpio@50000000` node (likely `gpio0`). Ask:
    *   Is the device enabled (`status = "okay"`)? Check `build/zephyr/zephyr.dts`.
    *   Is the driver enabled (`CONFIG_GPIO=y`)? Check `build/zephyr/.config` or use Kconfig GUI.
4.  **Fix the issue:**
    *   Examine `build/zephyr/zephyr.dts`. You'll likely find `status = "disabled";` for `gpio0`.
    *   Examine the application's overlay file (e.g., `boards/nrf52840dk_nrf52840.overlay`). It probably contains `&gpio0 { status = "disabled"; };`.
    *   Change the status in the **overlay file** to `"okay"`:
        ```devicetree
        /* boards/your_board.overlay */
        &gpio0 {
            status = "okay"; /* Change from disabled to okay */
        };
        ```
5.  **Perform a Pristine Build** (essential for devicetree/overlay changes). The application should now build successfully.

---

## Exercise 4 – Remote debugging with Memfault

This exercise introduces **Memfault**, a platform for device observability (metrics, crash reporting) and OTA updates, integrated into nRF Connect SDK. We will set up a free account, enable Memfault in the Hello World sample, send metrics and crash data, and view it in the Memfault web UI.

### Exercise steps

#### Part 1 – Setting up a Memfault account

1.  **Create Account:** Go to [Memfault Nordic Registration](https://app.memfault.com/register/nordic) and sign up for a free account (eligible for Nordic DK users).
2.  **Create Project:** Log in and create a new project (e.g., `hello-memfault`). Select the template matching your development kit (e.g., nRF5340 DK).
3.  **Note Project Key:** Go to Project Settings -> General. Copy the `Project Key`. You'll need this later.

#### Part 2 – Enabling Memfault in an nRF Connect SDK sample

4.  **Open Hello World Sample:** Create a new application using "Copy a sample" from `zephyr/samples/hello-world`. Store it (e.g., `l2/l2_e4`).
5.  Build and flash the *unmodified* sample to verify basic functionality.
6.  **Add Memfault Kconfigs:** Add the following to `prj.conf`, replacing `<YOUR_PROJECT_KEY>` with the key from step 3:
    ```kconfig
    # Enable Shell for testing commands
    CONFIG_SHELL=y
    CONFIG_SHELL_BACKEND_SERIAL=y

    # Enable Memfault Integration
    CONFIG_MEMFAULT=y
    CONFIG_MEMFAULT_NCS_PROJECT_KEY="<YOUR_PROJECT_KEY>"
    CONFIG_MEMFAULT_NCS_FW_TYPE="app" # Or your custom firmware type name
    CONFIG_MEMFAULT_NCS_DEVICE_ID="test-device" # Unique ID for this device
    ```
    > **Note (nRF Connect SDK v3.0.0+):** `CONFIG_MEMFAULT_NCS_PROJECT_KEY` is optional for devices that do not use MDS (Memfault Diagnostic Service) or the Memfault HTTP client to upload chunks directly to the cloud. If your device relies on an upstream gateway to forward data, you can omit this configuration.
7.  Build and flash the modified application. Connect serial terminal. Note the Memfault logs on boot. You should get a shell prompt (`uart:~$`).
8.  **Upload ELF File:** Go to your Memfault project -> Software -> Symbol Files. Click "Upload Symbol File" and upload the `zephyr.elf` file from your build directory (`build/zephyr/zephyr.elf`). This allows Memfault to symbolicate crash traces.
9.  **Trigger Heartbeat:** In the device's serial terminal, run:
    ```bash
    uart:~$ mflt test heartbeat
    ```
    This collects standard device metrics.
10. **Export Data:** Export the collected data (metrics and potentially other diagnostics) via the shell:
    ```bash
    uart:~$ mflt export
    ```
    This prints base64-encoded "chunks" of data.
11. **Upload Chunks:** Copy *all* lines starting with `MC:` from the terminal output. Go to your Memfault project -> Devices -> Chunks Debug. Paste the copied chunks into the input box and click "Submit Chunks".
12. **Explore Metrics:** Go to Devices view. You should see `test-device`. Click it. Go to Dashboards -> Metrics. You should see charts populated with basic device health metrics from the heartbeat. (Might need to click refresh icon on charts).

#### Step 3 – Exploring crash data in Memfault

13. **Trigger Crash:** In the device terminal, trigger a fault using Memfault's test commands (list available with `mflt test`):
    ```bash
    uart:~$ mflt test assert
    ```
    The device will crash and reboot.
14. **Check for Coredump:** After reboot, check if a coredump was saved:
    ```bash
    uart:~$ mflt get_core
    ```
    It should report a coredump with a specific size.
15. **Export and Upload Coredump:** Export the data again and upload the chunks via Chunks Debug (same as steps 10 & 11):
    ```bash
    uart:~$ mflt export
    ```
    (Copy `MC:` lines, paste into Chunks Debug).
16. **View Trace:** Go to Devices -> `test-device`. Find the new trace event on the timeline or under the "Traces" tab. Click it.
17. **Explore Diagnostics:** With the ELF file uploaded (step 8), the trace view will be symbolicated. Explore:
    *   **Summary:** Reason for crash, faulting instruction, PC/LR values.
    *   **Backtrace:** Call stack at the time of crash (showing function names, files, lines).
    *   **RTOS:** Thread states, priorities, stack usage.
    *   **Registers:** CPU register values.
    *   **Locals:** Local variables for the active stack frame.
    *   **Globals & Statics:** View global/static variables.
    *   **Memory Viewer:** Inspect memory contents.

#### (Optional) Step 4 – Exploring remote debugging

18. **(Optional) Alternative Transports:** Instead of manually exporting/uploading via UART shell (Chunks Debug), Memfault supports sending data chunks over production connectivity paths:
    *   **Cellular/Wi-Fi:** Use the [Memfault sample](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/samples/debug/memfault/README.html) (`<sdk>/nrf/samples/debug/memfault/`). Configure connectivity and Memfault project key. Data uploads automatically.
    *   **Bluetooth LE:** Requires a gateway device (phone app or custom gateway) to receive chunks over BLE and forward them to the Memfault cloud API. See [Memfault documentation](https://docs.memfault.com/docs/mcu/nordic-nrf-connect-sdk-guide) for BLE transport implementation details.

### Summary

Memfault provides powerful remote debugging and observability. Enabling it early allows continuous monitoring and faster fault resolution, especially for deployed devices.