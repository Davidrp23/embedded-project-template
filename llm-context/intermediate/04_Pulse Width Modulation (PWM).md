# Pulse Width Modulation (PWM)

## Overview

Pulse Width Modulation (PWM) is a technique commonly used for controlling analog devices using a digital control signal. With this analog-like digital signal, we can control a range of devices, from motors, lighting, power control and conversion. In this lesson, we will go through the basics of PWM control and take a closer look at how we can configure the PWM API for different things by changing the active duty cycle of the signal and by adding different compatible nodes to the overlay file in nRF Connect SDK.

In the exercise section of this lesson, we will practice controlling an LED on the DK using the PWM API. Then we will modify the devicetree to control an external servo motor device, and how to add a second PWM instance to be able to control both the servo and an LED on the board.

---

## Pulse Width Modulation (PWM)

**Pulse Width Modulation (PWM)** is a technique used to encode information in the form of a varying signal by changing the width of pulses in a sequence of pulses. It is commonly used in electronics to control the average power delivered to a load, such as in motor control, light dimming, or generating analog signals from digital ones.

By varying the width of the pulse (determining the amount of time the signal stays high/on and low/off), we can control the average power delivered to the assigned GPIOs. The portion of the period where the PWM signal is high is called the **duty cycle**.

The duty cycle in PWM is expressed as a percentage and represents the ratio of time the signal is high relative to the total period. A higher duty cycle means the signal is on for a larger percentage of the period, resulting in a higher average output voltage. A lower duty cycle leads to a lower average output voltage.

Key terms and formulas:

*   **Period:** Cycle time (Time ON + Time OFF)
*   **Frequency:** 1 / Period
*   **Amplitude:** Maximum voltage – minimum voltage
*   **Duty Cycle:** Time ON / Period

*(Diagram description: Visual representation of PWM signal with varying duty cycles)*

The PWM module enables the generation of pulse-width modulated signals on GPIOs. It typically implements an up or down counter with multiple PWM channels (e.g., four channels on nRF52840) that drive assigned GPIOs. Each channel can often have individual polarity and duty cycle values.

PWM output configurations can include:

*   **Single-edge controlled:** Only the falling or rising edge position is controlled relative to the period start.
*   **Double edge controlled:** Both rising and falling edge positions can be controlled (allows center-aligned PWM).

*(Diagram descriptions: Examples of PWM up-and-down counter and up counter)*

---

## Zephyr PWM API

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

To learn how to set up the PWM module in nRF Connect SDK, we will use the PWM API available in Zephyr (`<zephyr/drivers/pwm.h>`).

> **Note**: It's also possible to use the lower-level nrfx PWM driver directly, but the Zephyr API provides a hardware-agnostic interface.

### Enabling driver

1.  Enable the PWM driver Kconfig option in `prj.conf`:
    ```kconfig
    CONFIG_PWM=y
    ```
2.  Include the PWM API header file in your source code (`.c` file):
    ```c
    #include <zephyr/drivers/pwm.h>
    ```

### Configure the devicetree

If the device controlled by PWM (e.g., an LED specifically for PWM control, a motor) isn't already defined with PWM properties in the board's base devicetree, you need to define or modify it using a devicetree **overlay file** (`.overlay`).

Typically, this involves:

1.  **Defining a node** for your PWM-controlled device (e.g., a servo motor) with a suitable `compatible` string and `pwms` property linking it to a specific PWM peripheral instance and channel.
2.  **(Optional) Configuring Pin Control:** Defining custom pin control states (`pinctrl`) if you need to assign the PWM output to a specific GPIO pin different from the board's default or if no default exists.
3.  **(Optional) Assigning Pin Control:** Modifying the PWM peripheral node (`&pwm0`, `&pwm1`, etc.) in the overlay to use your custom pin control state.

**Example: Defining a Servo Motor Node**
Assume we want to control a servo motor using channel 1 of the `pwm0` peripheral.
Add this to your board's `.overlay` file:
```devicetree
/ { /* Ensure nodes are added under the root or appropriate parent */
    servo: servo { /* Node label 'servo', node name also 'servo' */
        compatible = "pwm-servo"; /* Custom compatible string (needs binding) */
        /* pwms property: phandle to PWM controller, channel index, period (ns), flags */
        pwms = <&pwm0 1 PWM_MSEC(20) PWM_POLARITY_NORMAL>; /* Use pwm0, channel 1, 20ms period, normal polarity */
        min-pulse = <PWM_USEC(700)>; /* Custom property: Min pulse width (ns) */
        max-pulse = <PWM_USEC(2500)>; /* Custom property: Max pulse width (ns) */
    };
};
```
*(Note: `pwm-servo` compatible requires a corresponding binding file, shown later)*

**Example: Configuring Pins (if needed)**
If `pwm0` needs to output on `P0.13`:
1.  **Define custom pin states** in the overlay (inside `&pinctrl`):
    ```devicetree
    &pinctrl {
        pwm0_default_custom: pwm0_default_custom { // Custom state name
            group1 {
                psels = <NRF_PSEL(PWM_OUT0, 0, 13)>; // Assign PWM0 channel 0 to P0.13
                /* nordic,invert; // Add this if output needs to be inverted */
            };
        };
        /* Define pwm0_sleep_custom if needed */
    };
    ```
2.  **Assign custom state** to `&pwm0` in the overlay:
    ```devicetree
    &pwm0 {
        status = "okay"; /* Make sure PWM peripheral itself is enabled */
        pinctrl-0 = <&pwm0_default_custom>; /* Use custom default state */
        /* /delete-property/ pinctrl-1; // Optionally remove sleep state if unused */
        pinctrl-names = "default"; /* Adjust names if sleep state removed */
    };
    ```

### Initializing the device

The Zephyr PWM API uses a specific structure `pwm_dt_spec` to hold device information:
*(Description of image: `pwm_dt_spec` struct definition)*
It contains the PWM device pointer (`dev`), channel number (`channel`), period (`period`), and flags (`flags`).

Use API-specific macros to get an initializer for this struct from a devicetree node:
*   `PWM_DT_SPEC_GET(node_id)`: Gets specifier from the first `pwms` entry in the node.
*   `PWM_DT_SPEC_GET_BY_NAME(node_id, name)`: Gets specifier by name (if `pwms` property has names).
*   `PWM_DT_SPEC_GET_BY_IDX(node_id, idx)`: Gets specifier by index within the `pwms` property.

Example using the `servo` node defined earlier:
```c
// Use DT_NODELABEL or DT_ALIAS if the node has one
#define SERVO_NODE DT_NODELABEL(servo) // Assuming 'servo' is the node label

static const struct pwm_dt_spec pwm_servo_spec = PWM_DT_SPEC_GET(SERVO_NODE);
```
This populates `pwm_servo_spec` with information from the `pwms` property of the `servo` node (device=&pwm0, channel=1, period=20ms, flags=NORMAL polarity).

### Validate device is ready

Before using the PWM device, check if it's ready using `pwm_is_ready_dt()`:
*(Description of image: `pwm_is_ready_dt()` signature)*
```c
if (!pwm_is_ready_dt(&pwm_servo_spec)) {
    LOG_ERR("PWM device %s is not ready", pwm_servo_spec.dev->name);
    return -ENODEV; // Or handle error
}
```

### Set period and pulse

Use API functions to control the PWM output:

*   `pwm_set_dt()`: Sets both the period and pulse width (duty cycle duration) in nanoseconds.
    *(Description of image: `pwm_set_dt()` signature)*
    ```c
    uint32_t desired_period_ns = 20000000; // 20 ms
    uint32_t desired_pulse_ns = 1500000;  // 1.5 ms pulse width
    int err = pwm_set_dt(&pwm_servo_spec, desired_period_ns, desired_pulse_ns);
    if (err) {
        // Handle error
    }
    ```
*   `pwm_set_pulse_dt()`: Sets only the pulse width, keeping the period defined in the devicetree or set previously.
    *(Description of image: `pwm_set_pulse_dt()` signature)*
    ```c
    uint32_t new_pulse_ns = 1000000; // 1 ms pulse width
    int err = pwm_set_pulse_dt(&pwm_servo_spec, new_pulse_ns);
    if (err) {
        // Handle error
    }
    ```

---

## Exercise 1 – Controlling an LED with PWM

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

Use a PWM signal to control the brightness/blinking of an LED on the DK. This exercise uses the pre-defined `pwm-leds` compatible node.

### Exercise steps

Open the exercise base code: `l4/l4_e1` (select version) using "Copy a sample".

1.  **Enable PWM and related modules** in `prj.conf`:
    ```kconfig
    # STEP 1.1: Enable PWM and set log level
    CONFIG_PWM=y
    CONFIG_PWM_LOG_LEVEL_DBG=y

    # STEP 1.2: Enable LED drivers
    CONFIG_LED=y
    CONFIG_LED_PWM=y
    ```
    Include header in `main.c`:
    ```c
    // STEP 1.3: Include headers
    #include <zephyr/device.h>
    #include <zephyr/drivers/pwm.h>
    #include <zephyr/logging/log.h> // Include for logging
    LOG_MODULE_REGISTER(Lesson4_Exercise1); // Register log module
    ```
2.  **Define PWM parameters** in `main.c`:
    ```c
    // STEP 2: Define PWM parameters (Example values)
    #define PWM_PERIOD_NS      20000000  // 20 ms = 50 Hz
    #define PWM_PULSE_WIDTH_NS 10000000  // 10 ms (50% duty cycle)
    ```
3.  **Initialize the PWM LED device:**
    3.1 Get node identifier using the standard alias `pwm-led0` (defined in board DTS):
       ```c
       // STEP 3.1: Get node ID via alias
       #define PWM_LED0_NODE DT_ALIAS(pwm_led0)
       ```
    3.2 Get the `pwm_dt_spec` initializer using `PWM_DT_SPEC_GET`:
       ```c
       // STEP 3.2: Get PWM device specification from Devicetree
       static const struct pwm_dt_spec pwm_led0_spec = PWM_DT_SPEC_GET(PWM_LED0_NODE);
       ```
    3.3 Check if the device is ready in `main()`:
       ```c
       // STEP 3.3: Check if PWM device is ready
       if (!pwm_is_ready_dt(&pwm_led0_spec)) {
           LOG_ERR("Error: PWM device %s is not ready", pwm_led0_spec.dev->name);
           return 0;
       }
       ```
4.  **Set the PWM output** in `main()`:
    ```c
    // STEP 4: Set PWM period and pulse width
    int err = pwm_set_dt(&pwm_led0_spec, PWM_PERIOD_NS, PWM_PULSE_WIDTH_NS);
    if (err) {
        LOG_ERR("Error %d: failed to set pulse width\n", err);
        return 0;
    }
    LOG_INF("PWM output set for pwm_led0");
    // Add a loop or sleep so main doesn't exit immediately
    while(1) { k_sleep(K_SECONDS(1)); }
    ```
5.  Build and flash. LED1 (or the LED mapped to `pwm-led0` alias on your board) should glow (e.g., at 50% brightness with the example values).
6.  **Experiment:** Change `PWM_PULSE_WIDTH_NS`.
    *   Smaller value = dimmer LED.
    *   Larger value = brighter LED.
    *   Very small/large values might turn it fully off/on.
    Try changing `PWM_PERIOD_NS` as well (e.g., 100ms = 10Hz). With a 50% duty cycle (`PWM_PULSE_WIDTH_NS = 50000000`), the LED will blink noticeably.

The solution is in the GitHub repo (`l4/l4_e1_sol`).

---

## Exercise 2 – Using PWM to control a servo motor

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

Control a Tower Pro Micro Servo SG90 using PWM. If you don't have a servo, follow along using an LED as in Exercise 1, adjusting pins/parameters.

> **Important**:
> *   nRF91x1 DKs and nRF7002 DK output 1.8V, likely insufficient for the SG90 servo. Use an LED or a level shifter.
> *   nRF54L15 DK defaults to 1.8V. Use the [nRF Connect Board Configurator](https://www.nordicsemi.com/Products/Development-tools/nrf-connect-for-desktop/board-configurator) app to set VDD to 3.3V before connecting the servo.

Datasheet ([link](http://www.ee.ic.ac.uk/pcheung/teaching/DE1_EE/stores/sg90_datasheet.pdf)) indicates:
*   Period: 20ms (50Hz)
*   Pulse Width Range: ~1ms (-90°) to ~2ms (+90°). 1.5ms is center (0°).

We will create a custom devicetree node for the servo using a custom binding and control it using PWM instance `pwm1` (to leave `pwm0` for the LED).

### Exercise steps

Open the exercise base code: `l4/l4_e2` (select version) using "Copy a sample".

> **Note**: Build the application once initially to enable VS Code devicetree features like "Go to Definition".

1.  **Creating a custom PWM device in Devicetree:**
    1.1 **Create/Rename Overlay:** In `l4_e2/boards/`, ensure you have an overlay file named correctly for your build target (e.g., `nrf52840dk_nrf52840.overlay`). Note that in nRF Connect SDK v3.0+, Zephyr board files are located under `boards/nordic/` in the SDK installation.
    1.2 **Define `pwm-servo` Binding:** Create `dts/bindings/pwm-servo.yaml` relative to your project root (`l4_e2/`) with the following content:
       ```yaml
       # dts/bindings/pwm-servo.yaml
       description: PWM-driven servo motor.
       compatible: "pwm-servo" # The compatible string we will use
       include: base.yaml
       properties:
         pwms:
           required: true
           type: phandle-array
           description: PWM specifier driving the servo motor (<&pwm_controller channel period flags>)
         min-pulse:
           required: true
           type: int
           description: Minimum pulse width (nanoseconds).
         max-pulse:
           required: true
           type: int
           description: Maximum pulse width (nanoseconds).
       ```
    1.3 **Add Servo Node to Overlay:** Add the following inside the root node (`/ { ... };`) of your `.overlay` file:
       ```devicetree
       /* boards/your_board.overlay */
       / {
           servo: servo { // Node label 'servo'
               compatible = "pwm-servo"; // Use the binding we created
               pwms = <&pwm1 0 PWM_MSEC(20) PWM_POLARITY_NORMAL>; // Use pwm1, channel 0, 20ms period
               min-pulse = <PWM_USEC(1000)>; // 1ms min pulse
               max-pulse = <PWM_USEC(2000)>; // 2ms max pulse
           };
           // ... other overlay nodes ...
       };
       ```
       *(Adjust `&pwm1` if your board uses a different instance, e.g., `&pwm21` for nRF54L15)*
    1.4 **Configure Pins for `pwm1`:** Add pin control definitions for `pwm1` to your `.overlay` file, mapping it to a free GPIO pin. Consult your DK's schematic/pinout. The solution uses specific pins per board (see table below).
       Example for nRF52840 DK (using P0.03):
       ```devicetree
       /* boards/nrf52840dk_nrf52840.overlay */
       &pwm1 {
           status = "okay"; // Enable pwm1
           pinctrl-0 = <&pwm1_custom_motor>;
           pinctrl-1 = <&pwm1_csleep_motor>; // Optional sleep state
           pinctrl-names = "default", "sleep";
       };

       &pinctrl {
           pwm1_custom_motor: pwm1_custom_motor {
               group1 {
                   psels = <NRF_PSEL(PWM_OUT0, 0, 3)>; // PWM1 Ch0 -> P0.03
                   /* nordic,invert not typically needed for servos */
               };
           };
           pwm1_csleep_motor: pwm1_csleep_motor { // Optional sleep state
               group1 {
                   psels = <NRF_PSEL(PWM_OUT0, 0, 3)>;
                   low-power-enable;
               };
           };
           // ... other pinctrl nodes ...
       };
       ```
       | Board         | GPIO in Solution | PWM Instance Used |
       | :------------ | :--------------- | :---------------- |
       | nRF52 DK      | P0.03            | `pwm1`            |
       | nRF52833 DK   | P0.03            | `pwm1`            |
       | nRF52840 DK   | P0.03            | `pwm1`            |
       | nRF5340 DK    | P0.05            | `pwm1`            |
       | nRF54L15 DK   | P1.11            | `pwm21`           |
       | nRF7002 DK    | P0.07 (LED2)     | `pwm1`            |
       | nRF9160 DK    | P0.10            | `pwm1`            |
       | nRF91x1 DK    | P0.01 (LED2)     | `pwm1`            |
       *(Adjust the `psels` value and `&pwmX` references in the overlay according to your board and chosen GPIO)*

2.  **Control the servo motor angle** in `main.c`:
    2.1 **Get Servo PWM Spec:** Initialize `pwm_dt_spec` for the servo:
       ```c
       // STEP 5.4: Get servo device spec
       #define SERVO_MOTOR_NODE DT_NODELABEL(servo)
       static const struct pwm_dt_spec pwm_servo_spec = PWM_DT_SPEC_GET(SERVO_MOTOR_NODE);
       ```
    2.2 **Get Min/Max Pulse from Devicetree:** Use `DT_PROP` to read the custom properties:
       ```c
       // STEP 5.5: Get min/max pulse from DT
       #define PWM_SERVO_MIN_PULSE_WIDTH  DT_PROP(SERVO_MOTOR_NODE, min_pulse)
       #define PWM_SERVO_MAX_PULSE_WIDTH  DT_PROP(SERVO_MOTOR_NODE, max_pulse)
       ```
    2.3 **Check Servo PWM Ready & Set Initial Position:** Add in `main()`:
       ```c
       // STEP 5.7 Check servo readiness and set initial angle
       if (!pwm_is_ready_dt(&pwm_servo_spec)) {
           LOG_ERR("Error: PWM servo device %s is not ready", pwm_servo_spec.dev->name);
           return 0;
       }
       // Set initial position (e.g., min angle)
       err = pwm_set_pulse_dt(&pwm_servo_spec, PWM_SERVO_MIN_PULSE_WIDTH);
       if (err) {
           LOG_ERR("Error %d: failed to set initial servo pulse width\n", err);
           return 0;
       }
       ```
       *(Note: We use `pwm_set_pulse_dt` here assuming the 20ms period from the DT `pwms` property is correct and fixed).*
    2.4 **Create `set_motor_angle` function:** (Uses `pwm_servo_spec`)
       ```c
       // STEP 5.8 + 2.1 combined: Function to set servo pulse width
       int set_motor_angle(uint32_t pulse_width_ns)
       {
           int err = pwm_set_pulse_dt(&pwm_servo_spec, pulse_width_ns);
           if (err) {
               LOG_ERR("Error %d: failed to set servo pulse width %u", err, pulse_width_ns);
           }
           return err;
       }
       ```
    2.5 **Update `button_handler`** to control the servo using the min/max pulse widths from DT:
       ```c
       // STEP 5.6: Update button handler for servo
       // Inside button_handler switch statement:
       case DK_BTN1_MSK:
           LOG_INF("Button 1 pressed - Servo Min Angle");
           err = set_motor_angle(PWM_SERVO_MIN_PULSE_WIDTH);
           break;
       case DK_BTN2_MSK:
           LOG_INF("Button 2 pressed - Servo Max Angle");
           err = set_motor_angle(PWM_SERVO_MAX_PULSE_WIDTH);
           break;
       ```

3.  **Testing the code:**
    3.1 **Connect Servo:**
        *   Brown wire -> GND on DK
        *   Red wire -> VOUT/VDD (3.3V) on DK (ensure correct voltage!)
        *   Yellow wire -> Chosen GPIO pin (e.g., P0.03 on nRF52840 DK)
        *(Diagram description: Wiring example)*
        > **Important**: For nRF9160 DK, set SW9 to 3V. For nRF54L15 DK, ensure VDD is set to 3.3V via Board Configurator.
    3.2 **Build and Flash.**
    3.3 Press Button 1 and Button 2. Observe the servo motor moving between its minimum and maximum angles.

4.  **(Optional) Reconfigure `pwm0` to drive LED1 again:** Modify the overlay to point `pwm0_custom` back to the LED1 pin (e.g., P0.13 on nRF52840 DK) and adjust pulse widths/button handler logic for LED blinking as desired (similar to steps 4.1-4.4 in original text, using `pwm_led0_spec`).

5.  **(Done in Step 1) Add `pwm1` instance for servo.** (Steps 5.1-5.8 in the original text are integrated into Step 1 & 2 above for clarity).

6.  **Build and Test Combined Code:** If you reconfigured `pwm0` for the LED and added `pwm1` for the servo, build and flash. Test that Buttons 1/2 control the servo and Buttons 3/4 control the LED blinking frequency/brightness.

The solution is in the GitHub repo (`l4/l4_e2_sol`).