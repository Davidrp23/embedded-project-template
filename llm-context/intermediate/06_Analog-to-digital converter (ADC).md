# Analog-to-digital converter (ADC)

## Overview

ADC stands for Analog-to-Digital Converter. An ADC is used to transform an analog signal, such as continuous analog voltage, into a digital format, enabling it to be read by a processor and enabling the processor to take actions based on the read values.

Embedded systems frequently require the measurement of physical parameters, typically represented in a continuous (analog) format rather than a digital form. Examples of physical parameters are temperature, light intensity, and pressure. Usually, sensors have an internal ADC that provides a converted digital form of the physical parameter, but this is not always the case. In such a case, we need to use the ADC inside the SoC and SiP to convert the physical parameter from an analog form into a digital form.

In this lesson, we will first examine the ADC peripheral available on Nordic devices (nRF52, nRF53, and nRF91 Series), which is the type Successive approximation analog-to-digital converter (SAADC) and understand its capabilities and features.

Then, in exercise 1, we will learn how to interact with the SAADC using the Zephyr API to measure a voltage on one of the analog capable pins or the supply voltage. In exercises 2 and 3 we will learn how to interact with the SAADC peripheral using advanced methods by utilizing the nrfx drivers directly. The goal of these exercises is to teach you the advantages and implications of using these different methods.

---

## ADC peripheral on Nordic devices

*(Information relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

ADC stands for Analog-to-Digital Converter. An ADC transforms an analog signal (like voltage) into a digital representation.

The ADC on Nordic devices (nRF5x, nRF91 Series) is a **Successive Approximation Analog-to-Digital Converter (SAADC)**. SAADCs are common in microcontrollers due to their efficiency and low power consumption. They use a comparator, a Digital-to-Analog Converter (DAC), and a Successive Approximation Register (SAR) to perform a binary search through possible digital values to find the closest match to the sampled analog input voltage.

*(Diagram description: Basic 4-bit SAR ADC block diagram from Wikipedia)*

### SAADC concepts

*   **Sampling Rate (Frequency):** The rate at which the analog input is sampled. A continuous signal is converted to a discrete-time signal. The rate must be high enough to capture changes in the signal. According to the Nyquist-Shannon theorem, the sampling rate must be at least twice the maximum frequency of the analog signal (`f_sample >= 2 * f_max`). The maximum rate is limited by the acquisition time (`t_ACQ`) and conversion time (`t_conv`): `f_sample < 1 / (t_ACQ + t_conv)`.
    *   nRF52/nRF53/nRF91: Max 200 ksps (5 us interval).

*   **Conversion Time (`t_conv`):** Time taken by the SAR core to convert the sampled voltage into a digital value. Depends on the selected resolution. Higher resolution means longer `t_conv`.
    *   nRF52/nRF53/nRF91: Typically 2 us.

*   **Acquisition Time (`t_ACQ`):** Time required for the internal sample-and-hold circuit capacitor to accurately charge to the input voltage level. Depends on the input source impedance. Typically 3-40 us (configurable). Affects the maximum sampling rate.
    *(Diagram description: SAADC input stage showing source resistance and internal capacitor)*

*   **Resolution:** The number of bits used to represent the digital output. Higher resolution means smaller distinguishable voltage steps (finer granularity) but typically longer conversion times.
    *   nRF52/nRF53/nRF91: 8, 10, 12 bits. 14-bit effective resolution possible with oversampling.
    *   Oversampling improves Signal-to-Noise Ratio (SNR) but not linearity (INL/DNL).

*   **Input Mode:**
    *   **Single-ended (SE):** Measures voltage on one pin relative to the internal ADC ground. Default mode. Simpler but more susceptible to noise.
    *   **Differential (Diff):** Measures the voltage difference *between* two input pins. More complex interface but better noise immunity.
    *   The SAADC typically has 8 analog input pins (AIN0-AIN7). These can be configured as 8 SE channels, 4 Diff channels, or a mix. VDD and VDDHDIV5 (if available) can also be selected as inputs.
    *(Diagram description: Single-ended vs. Differential input)*

*   **Gain:** Amplifies or attenuates the input signal before conversion. Allows adjusting the effective input voltage range.
    *   nRF5x/nRF91: Gains 1/6, 1/5, 1/4, 1/3, 1/2, 1, 2, 4.

*   **Reference Voltage:** The voltage against which the input is compared. Determines the ADC's full-scale range.
    *   **Internal Reference (VBG):** Typically 0.6 V. Provides a stable reference independent of VDD. Results in ADC core input range of +/-0.6 V.
    *   **VDD/4:** Uses the supply voltage (VDD) divided by 4 as the reference. Results in ADC core input range of +/-VDD/4.

*   **Input Range Calculation:** The actual voltage range measurable at the AIN pins depends on the reference and gain:
    `Input Range = (Core Input Range) / Gain`
    `Input Range = (+/- Reference Voltage) / Gain`
    *   Example 1 (Internal Ref, Gain 1/6): `Input Range = (+/- 0.6 V) / (1/6) = +/- 3.6 V`. Since input cannot exceed VDD, the practical positive range is `min(3.6V, VDD)`. The negative range is limited by GND (0V) for single-ended.
    *   Example 2 (VDD/4 Ref, Gain 1/4): `Input Range = (+/- VDD/4) / (1/4) = +/- VDD`. Practical range 0V to VDD.
    *   **Important**: AIN pins voltage must *never* exceed VDD.

Full SAADC documentation is in the SoC/SiP Product Specification.

---

## Choosing between Zephyr ADC API and nrfx SAADC driver API

Two ways to interact with the SAADC peripheral:

1.  **Zephyr ADC API (`<zephyr/drivers/adc.h>`):** **Preferred method** when possible.
2.  **nrfx SAADC driver API (`<nrfx_saadc.h>`):** Lower-level, direct hardware access.

### Advantages of Zephyr API

*   **Portability:** Platform-agnostic. Code using Zephyr APIs should work across different chips/vendors with minimal changes. nrfx drivers are Nordic-specific.
*   **Zephyr Integration:** Leverages Zephyr features like automatic power management. nrfx drivers are OS-agnostic and require manual power management handling if used in a Zephyr application.

### nrfx SAADC use-cases

Consider using nrfx drivers directly when Zephyr API limitations prevent achieving specific requirements:

1.  **High Sample Rate / Precise Timing:**
    *   Triggering samples via software interrupts at very high rates (e.g., near 200 ksps) consumes significant CPU time.
    *   Software triggering introduces latency and jitter, making precise timing difficult, especially with high-priority tasks running.
    *   **Solution:** Use nrfx SAADC driver combined with nrfx (D)PPI driver. (D)PPI allows hardware peripherals (like a TIMER) to trigger tasks on other peripherals (like SAADC sampling) directly, **without CPU involvement**. This ensures precise timing and low overhead. Zephyr ADC API currently lacks direct (D)PPI integration for sample triggering.

The nrfx SAADC driver offers two modes:

*   **Simple Mode:** Basic single-sample acquisition per channel, triggered by software call (blocking or non-blocking).
*   **Advanced Mode:** Supports double-buffered conversions of arbitrary length. Can be triggered by software, internal timer, or **externally via (D)PPI**. Supports automatic buffer switching using (D)PPI. Ideal for continuous high-speed sampling.

> **Note**: nrfx driver examples are in `<sdk>/modules/hal/nordic/nrfx/samples`.

You might *need* nrfx drivers if a specific peripheral feature isn't exposed by the Zephyr API (e.g., I2C target/slave mode historically required nrfx TWIS). For ADC, you often have a choice: prioritize portability/integration (Zephyr API) or maximum control/performance/(D)PPI features (nrfx driver).

---

## Exercise 1 - Interfacing with ADC using Zephyr API

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

Read an analog voltage using the Zephyr ADC API.

### Exercise steps

Open the exercise base code: `l6/l6_e1` (select version) using "Copy a sample".

1.  **Enable ADC API** in `prj.conf`:
    ```kconfig
    CONFIG_ADC=y
    ```
2.  **Define ADC channel in Devicetree:**
    2.1 **Create/Rename Overlay:** Ensure `l6/l6_e1/boards/your_board.overlay` exists and is named correctly.
    2.2 **Specify `io-channels`:** Under the root (`/`) node, create `zephyr,user` and point `io-channels` to the ADC channel(s) you intend to use via the Zephyr API. We'll use channel 0 of the `adc` peripheral instance. Add to overlay:
       ```devicetree
       / {
           zephyr,user {
               /* ADC channel 0 (&adc controller, index 0) */
               io-channels = <&adc 0>;
           };
       };
       ```
       *(Multiple channels: `<&adc 0>, <&adc 1>, ...`)*
    2.3 **Configure the ADC channel:** Define properties for channel 0 within the `&adc` node in the overlay.
       Example for most DKs (adjust `zephyr,input-positive` based on SoC pin mapping, see table below):
       ```devicetree
       &adc { /* Reference the SAADC peripheral node */
           #address-cells = <1>;
           #size-cells = <0>;
           status = "okay"; /* Enable the ADC peripheral */

           channel@0 { /* Configure channel 0 */
               reg = <0>; /* Channel index */
               zephyr,gain = "ADC_GAIN_1_6"; /* Gain 1/6 */
               zephyr,reference = "ADC_REF_INTERNAL"; /* Internal 0.6V reference */
               zephyr,acquisition-time = <ADC_ACQ_TIME_DEFAULT>; /* Default (e.g., 10us) */
               zephyr,input-positive = <NRF_SAADC_AIN0>; /* Positive input = AIN0 pin */
               /* Negative input defaults to internal GND for single-ended */
               zephyr,resolution = <12>; /* 12-bit resolution */
           };
           /* Add channel@1 { ... } etc. for more channels */
       };
       ```
       *   Consult [ADC DT Binding](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/hardware/peripherals/adc.html#devicetree-configuration) for all properties.
       *   **Pin Mapping (AIN0):** nRF52=P0.02, nRF53=P0.04, nRF91=P0.13 (Check Product Spec!).
       *   Can measure VDD using `zephyr,input-positive = <NRF_SAADC_VDD>;`.

3.  **Retrieve ADC channel specifier** in `main.c`:
    3.1 Include header:
       ```c
       #include <zephyr/drivers/adc.h>
       #include <zephyr/logging/log.h> // For logging
       LOG_MODULE_REGISTER(Lesson6_Exercise1); // Register log module
       ```
    3.2 Get `adc_dt_spec` using `ADC_DT_SPEC_GET` which reads the `io-channels` property from `zephyr,user`:
       ```c
       // STEP 3.2: Get ADC channel spec from DT
       static const struct adc_dt_spec adc_channel = ADC_DT_SPEC_GET(DT_PATH(zephyr_user));
       ```
       *(If multiple channels in `io-channels`, use `ADC_DT_SPEC_GET_BY_IDX(DT_PATH(zephyr_user), index)`)*
    3.3 Validate ADC device readiness in `main()`:
       ```c
       // STEP 3.3: Validate ADC device readiness
       if (!adc_is_ready_dt(&adc_channel)) {
           LOG_ERR("ADC controller device %s not ready", adc_channel.dev->name);
           return 0;
       }
       ```
    3.4 Setup the channel based on DT configuration in `main()`:
       ```c
       // STEP 3.4: Setup ADC channel
       int err = adc_channel_setup_dt(&adc_channel);
       if (err < 0) {
           LOG_ERR("Could not setup channel #%d (%d)", adc_channel.channel_id, err);
           return 0;
       }
       ```

4.  **Define ADC read sequence:**
    4.1 Define sequence struct and sample buffer in `main()` (inside the loop for this example):
       ```c
       // STEP 4.1: Define sequence and buffer
       int16_t sample_buffer; // Buffer for one sample
       struct adc_sequence sequence = {
           .buffer = &sample_buffer,
           /* buffer size in bytes, not number of samples */
           .buffer_size = sizeof(sample_buffer),
           /* Optional: Calibrate before reading */
           //.calibrate = true,
       };
       ```
    4.2 Initialize the sequence based on the channel spec:
       ```c
       // STEP 4.2: Initialize sequence
       err = adc_sequence_init_dt(&adc_channel, &sequence);
       if (err < 0) {
           LOG_ERR("Could not initialize sequence (%d)", err);
           // continue; // Or handle error
       }
       ```

5.  **Read ADC sample** inside the `while(1)` loop using `adc_read()`:
    ```c
    // STEP 5: Read ADC value
    LOG_INF("ADC reading[%u]:", count++);
    // Use adc_read() with device pointer from spec
    err = adc_read(adc_channel.dev, &sequence);
    if (err < 0) {
        LOG_ERR("Could not read (%d)", err);
        continue; // Skip rest of loop iteration on error
    } else {
         LOG_INF(" Raw value: %d", sample_buffer);
    }
    ```

6.  **Convert raw value to millivolts** using `adc_raw_to_millivolts_dt()` which uses the gain/reference from the DT spec:
    ```c
    // STEP 6: Convert raw value to millivolts
    int32_t millivolts;
    err = adc_raw_to_millivolts_dt(&adc_channel, &millivolts);
    if (err < 0) {
        LOG_WRN(" (value in mV not available, err %d)\n", err);
    } else {
        LOG_INF(" = %d mV", millivolts);
    }
    ```

#### Testing

7.  Build and flash.
8.  Connect AIN0 pin (e.g., P0.02 on nRF52840DK, P0.04 on nRF5340DK) to a voltage source (e.g., VDD pin on DK, GND pin, battery, PPK2 VOUT). Ensure shared GND.
    *(Diagram description: Jumper wire connection)*
    > **Note**: Input voltage must NOT exceed VDD. Use voltage divider if necessary.
9.  Observe serial terminal output. You should see raw ADC values and converted millivolt readings printed periodically. Try connecting AIN0 to VDD and then to GND to see the readings change. Small non-zero readings when connected to GND are expected due to noise in single-ended mode.

The solution is in the GitHub repo (`l6/l6_e1_sol`).

---

## Exercise 2 - Interfacing with ADC using nrfx driver and software timers

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

Use the **nrfx SAADC driver** in **simple mode** to measure a voltage source periodically using a Zephyr kernel timer (`k_timer`).

### Exercise steps

Open the exercise base code: `l6/l6_e2` (select version) using "Copy a sample".

1.  **Enable nrfx SAADC driver** in `prj.conf`:
    ```kconfig
    CONFIG_NRFX_SAADC=y
    ```
2.  **Include nrfx SAADC header** in `main.c`:
    ```c
    #include <nrfx_saadc.h>
    #include <zephyr/logging/log.h> // For logging
    LOG_MODULE_REGISTER(Lesson6_Exercise2); // Register log module
    ```
3.  **Declare nrfx SAADC objects** globally or statically:
    3.1 Declare channel configuration struct using default single-ended macro:
       ```c
       // STEP 3.1: Declare channel config
       #define SAADC_INPUT_PIN NRF_SAADC_INPUT_AIN0 // Use AIN0 for most DKs
       static nrfx_saadc_channel_t channel = NRFX_SAADC_DEFAULT_CHANNEL_SE(SAADC_INPUT_PIN, 0); // Channel index 0
       ```
    3.2 Declare sample buffer (for one sample in simple mode):
       ```c
       // STEP 3.2: Declare sample buffer
       static int16_t sample;
       ```
4.  **Setup Zephyr timer** for periodic triggering:
    4.1 Define sample interval:
       ```c
       // STEP 4.1: Define sample interval
       #define BATTERY_SAMPLE_INTERVAL_MS 2000 // 2 seconds
       ```
    4.2 Define timer instance and forward declare handler:
       ```c
       // STEP 4.3: Forward declare handler
       static void battery_sample_timer_handler(struct k_timer *timer);
       // STEP 4.2: Define timer
       K_TIMER_DEFINE(battery_sample_timer, battery_sample_timer_handler, NULL);
       ```
5.  **Configure SAADC driver** (e.g., in a setup function called from `main`):
    5.1 **Connect interrupt:** Use `IRQ_CONNECT` with info from devicetree (`&adc` node):
       ```c
       // STEP 5.1: Connect SAADC interrupt
       IRQ_CONNECT(DT_IRQN(DT_NODELABEL(adc)),
                   DT_IRQ(DT_NODELABEL(adc), priority),
                   nrfx_isr, nrfx_saadc_irq_handler, 0);
       ```
       *(Ensure `&adc { status = "okay"; };` is in your overlay if using a custom board where it's not default)*.
    5.2 **Initialize nrfx driver:**
       ```c
       // STEP 5.2: Initialize nrfx SAADC driver
       nrfx_err_t err = nrfx_saadc_init(DT_IRQ(DT_NODELABEL(adc), priority));
       if (err != NRFX_SUCCESS) {
           LOG_ERR("nrfx_saadc_init error: %08x", err);
           return; // Handle error
       }
       ```
    5.3 **Configure channel:** Modify default gain before applying channel config.
       ```c
       // STEP 5.3: Configure SAADC channel gain
       channel.channel_config.gain = NRF_SAADC_GAIN1_6; // Gain 1/6 for VDD measurement
       err = nrfx_saadc_channels_config(&channel, 1); // Configure 1 channel
       if (err != NRFX_SUCCESS) {
           LOG_ERR("nrfx_saadc_channels_config error: %08x", err);
           return; // Handle error
       }
       ```
    5.4 **Set simple mode:** Configure resolution, oversampling, and set handler to `NULL` for blocking mode.
       ```c
       // STEP 5.4: Set simple mode (blocking)
       err = nrfx_saadc_simple_mode_set(
                 BIT(channel.channel_id), // Use channel index (0)
                 NRF_SAADC_RESOLUTION_12BIT, // 12-bit resolution
                 NRF_SAADC_OVERSAMPLE_DISABLED, // No oversampling
                 NULL); // NULL handler = blocking mode
       if (err != NRFX_SUCCESS) {
           LOG_ERR("nrfx_saadc_simple_mode_set error: %08x", err);
           return; // Handle error
       }
       ```
       > **Note**: Consider offset calibration (`nrfx_saadc_offset_calibrate()`) for better accuracy, especially if temperature varies.
    5.5 **Set sample buffer:** Point the driver to the single sample buffer.
       ```c
       // STEP 5.5: Set sample buffer
       err = nrfx_saadc_buffer_set(&sample, 1); // Buffer pointer, size = 1 sample
       if (err != NRFX_SUCCESS) {
           LOG_ERR("nrfx_saadc_buffer_set error: %08x", err);
           return; // Handle error
       }
       ```
6.  **Start the periodic timer** at the end of the SAADC configuration function:
    ```c
    // STEP 6: Start periodic timer
    k_timer_start(&battery_sample_timer, K_NO_WAIT, K_MSEC(BATTERY_SAMPLE_INTERVAL_MS));
    ```
7.  **Implement timer callback handler** (`battery_sample_timer_handler`):
    7.1 Add function definition:
       ```c
       // STEP 7.1: Implement timer handler
       void battery_sample_timer_handler(struct k_timer *timer)
       {
           // STEP 7.2: Trigger sampling
           nrfx_err_t err = nrfx_saadc_mode_trigger();
           if (err != NRFX_SUCCESS) {
               LOG_ERR("nrfx_saadc_mode_trigger error: %08x", err);
               return;
           }
           // Since mode is blocking, 'sample' variable is updated now

           // STEP 7.3: Calculate and print voltage
           // Formula for nRF5x/nRF91 (0.6V ref, 1/6 gain, 12-bit)
           int battery_voltage = ((sample * (600 * 6)) / (1 << 12));

           LOG_INF("SAADC sample: %d", sample);
           LOG_INF("Battery Voltage: %d mV", battery_voltage);
       }
       ```

#### Testing

8.  Build and flash.
9.  Connect analog input (AIN0) to a voltage source (e.g., battery, VDD pin). Ensure shared GND.
10. Observe serial terminal. You should see the raw sample value and calculated voltage printed every 2 seconds.

The solution is in the GitHub repo (`l6/l6_e2_sol`).

---

## Exercise 3 - Interfacing with ADC using nrfx drivers and TIMER/(D)PPI

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

Use the **nrfx SAADC driver** in **advanced mode** with **double buffering** to perform high-speed sampling triggered by a hardware TIMER via **(D)PPI**, eliminating CPU involvement for triggering.

### Exercise steps

Open the exercise base code: `l6/l6_e3` (select version) using "Copy a sample".

1.  **Enable nrfx drivers** in `prj.conf`:
    ```kconfig
    CONFIG_NRFX_SAADC=y
    CONFIG_NRFX_PPI=y  # Enable for older chips (e.g., nRF52)
    CONFIG_NRFX_DPPI=y # Enable for newer chips (e.g., nRF53, nRF91)
    # Enable specific TIMER instance
    CONFIG_NRFX_TIMER2=y
    ```
    > **Important**: Ignore build warnings about PPI/DPPI being 'y' but getting 'n'. This happens because only one is present per chip. Using Kconfig fragments per board can resolve this.

2.  **Include nrfx headers** in `main.c`:
    ```c
    #include <nrfx_saadc.h>
    #include <nrfx_timer.h>
    #include <helpers/nrfx_gppi.h> // Helper API for both PPI and DPPI
    #if defined(DPPI_PRESENT)
    #include <nrfx_dppi.h>
    #else
    #include <nrfx_ppi.h>
    #endif
    #include <zephyr/logging/log.h>
    LOG_MODULE_REGISTER(Lesson6_Exercise3);
    ```
3.  **Configure hardware TIMER** (e.g., in `configure_timer()`):
    3.1 Define sample interval:
       ```c
       #define SAADC_SAMPLE_INTERVAL_US 50 // e.g., 50 us = 20 ksps
       ```
    3.2 Declare correct TIMER instance:
       ```c
       // STEP 3.2: Declare TIMER instance
       static const nrfx_timer_t timer_instance = NRFX_TIMER_INSTANCE(2);
       ```
    3.3 Initialize TIMER (e.g., 1 MHz frequency, no event handler needed):
       ```c
       // STEP 3.3: Initialize TIMER
       nrfx_err_t err;
       nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG(1000000UL); // 1 MHz
       err = nrfx_timer_init(&timer_instance, &timer_config, NULL); // NULL handler
       // Handle error...
       ```
    3.4 Set compare channel 0 to trigger periodically and clear timer:
       ```c
       // STEP 3.4: Set periodic compare event
       uint32_t timer_ticks = nrfx_timer_us_to_ticks(&timer_instance, SAADC_SAMPLE_INTERVAL_US);
       // Set CC[0], enable clear shortcut on compare match, disable interrupt
       nrfx_timer_extended_compare(&timer_instance, NRF_TIMER_CC_CHANNEL0, timer_ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, false);
       ```
4.  **Configure SAADC driver** (e.g., in `configure_saadc()`):
    4.1 Define buffer size:
       ```c
       #define SAADC_BUFFER_SIZE 100 // Example: buffer holds 100 samples
       ```
    4.2 Declare double buffers (static or global):
       ```c
       static int16_t saadc_sample_buffer[2][SAADC_BUFFER_SIZE];
       ```
    4.3 Declare buffer index tracker:
       ```c
       static uint32_t saadc_current_buffer = 0; // Tracks next buffer to provide
       ```
    4.4 Connect interrupt (same as Exercise 2):
       ```c
       IRQ_CONNECT(...nrfx_saadc_irq_handler...);
       ```
    4.5 Initialize nrfx driver (same as Exercise 2):
       ```c
       err = nrfx_saadc_init(...);
       // Handle error...
       ```
    4.6 Declare channel configuration (same as Exercise 2):
       ```c
       #define SAADC_INPUT_PIN NRF_SAADC_INPUT_AIN0
       static nrfx_saadc_channel_t channel = NRFX_SAADC_DEFAULT_CHANNEL_SE(...);
       ```
    4.7 Configure channel gain (same as Exercise 2):
       ```c
       channel.channel_config.gain = NRF_SAADC_GAIN1_6;
       err = nrfx_saadc_channels_config(&channel, 1);
       // Handle error...
       ```
    4.8 Set **advanced mode** (non-blocking with event handler):
       ```c
       // STEP 4.8: Set advanced mode (non-blocking)
       nrfx_saadc_adv_config_t saadc_adv_config = NRFX_SAADC_DEFAULT_ADV_CONFIG;
       // Default disables oversampling, burst, internal timer trigger
       err = nrfx_saadc_advanced_mode_set(
                 BIT(channel.channel_id),          // Channel mask (only channel 0)
                 NRF_SAADC_RESOLUTION_12BIT,     // Resolution
                 &saadc_adv_config,              // Advanced config struct
                 saadc_event_handler);           // Event handler function
       if (err != NRFX_SUCCESS) {
           LOG_ERR("nrfx_saadc_advanced_mode_set error: %08x", err);
           return; // Handle error
       }
       ```
    4.9 **Set up double buffering:** Call `nrfx_saadc_buffer_set()` twice, once for each buffer.
       ```c
       // STEP 4.9: Provide both buffers initially
       err = nrfx_saadc_buffer_set(saadc_sample_buffer[0], SAADC_BUFFER_SIZE);
       // Handle error...
       err = nrfx_saadc_buffer_set(saadc_sample_buffer[1], SAADC_BUFFER_SIZE);
       // Handle error...
       ```
    4.10 Trigger SAADC mode (prepares first buffer for sampling):
        ```c
        // STEP 4.10: Trigger SAADC mode
        err = nrfx_saadc_mode_trigger();
        // Handle error...
        ```
5.  **Implement SAADC event handler** (`saadc_event_handler`):
    5.1 **`NRFX_SAADC_EVT_READY`:** Triggered when first buffer is ready. Start the TIMER here.
       ```c
       // Inside saadc_event_handler switch statement
       case NRFX_SAADC_EVT_READY:
           // STEP 5.1: Start the timer on READY event
           nrfx_timer_enable(&timer_instance);
           break;
       ```
    5.2 **`NRFX_SAADC_EVT_BUF_REQ`:** Triggered when driver needs the *next* buffer. Provide the other buffer.
       ```c
       // Inside saadc_event_handler switch statement
       case NRFX_SAADC_EVT_BUF_REQ:
           // STEP 5.2: Provide the next buffer
           err = nrfx_saadc_buffer_set(saadc_sample_buffer[(saadc_current_buffer++) % 2],
                                       SAADC_BUFFER_SIZE);
           if (err != NRFX_SUCCESS) {
               LOG_ERR("nrfx_saadc_buffer_set error: %08x", err);
               // Handle error - might need to stop sampling
           }
           break;
       ```
    5.3 **`NRFX_SAADC_EVT_DONE`:** Triggered when a buffer (`p_event->data.done.p_buffer`) is filled with `p_event->data.done.size` samples. Process the filled buffer here.
       ```c
       // Inside saadc_event_handler switch statement
       case NRFX_SAADC_EVT_DONE:
           { // Use braces for local variable scope
               // STEP 5.3: Process the filled buffer
               int16_t *p_buffer = (int16_t *)p_event->data.done.p_buffer;
               uint32_t samples_count = p_event->data.done.size;
               int64_t average = 0;
               int16_t max = INT16_MIN;
               int16_t min = INT16_MAX;

               for (uint32_t i = 0; i < samples_count; i++) {
                   int16_t current_value = p_buffer[i];
                   average += current_value;
                   if (current_value > max) max = current_value;
                   if (current_value < min) min = current_value;
               }
               average = average / samples_count;

               LOG_INF("SAADC buffer at %p filled (%u samples)", p_buffer, samples_count);
               LOG_INF("AVG=%d, MIN=%d, MAX=%d", (int16_t)average, min, max);
           } // End scope for local variables
           break;
       ```
       *(Add other cases like `NRFX_SAADC_EVT_ERROR` as needed)*

6.  **Setup (D)PPI channels** (e.g., in `configure_ppi()`):
    6.1 Declare channel variables:
       ```c
       // STEP 6.1: Declare PPI/DPPI channel variables
       static uint8_t m_timer_compare_to_saadc_sample_ppi_channel;
       static uint8_t m_saadc_end_to_saadc_start_ppi_channel;
       ```
    6.2 Allocate channels using `nrfx_gppi` helper:
       ```c
       // STEP 6.2: Allocate channels
       err = nrfx_gppi_channel_alloc(&m_timer_compare_to_saadc_sample_ppi_channel);
       // Handle error...
       err = nrfx_gppi_channel_alloc(&m_saadc_end_to_saadc_start_ppi_channel);
       // Handle error...
       ```
    6.3 **Connect TIMER Compare event to SAADC Sample task:**
       ```c
       // STEP 6.3: Connect TIMER COMPARE[0] event -> SAADC SAMPLE task
       nrfx_gppi_channel_endpoints_setup(m_timer_compare_to_saadc_sample_ppi_channel,
           nrfx_timer_compare_event_address_get(&timer_instance, NRF_TIMER_CC_CHANNEL0),
           nrfx_saadc_task_address_get(NRF_SAADC, NRF_SAADC_TASK_SAMPLE));
       ```
    6.4 **Connect SAADC End event to SAADC Start task:** (For automatic buffer switching without CPU delay)
       ```c
       // STEP 6.4: Connect SAADC END event -> SAADC START task
       nrfx_gppi_channel_endpoints_setup(m_saadc_end_to_saadc_start_ppi_channel,
           nrfx_saadc_event_address_get(NRF_SAADC, NRF_SAADC_EVENT_END),
           nrfx_saadc_task_address_get(NRF_SAADC, NRF_SAADC_TASK_START));
       ```
    6.5 **Enable the channels:**
       ```c
       // STEP 6.5: Enable channels
       nrfx_gppi_channels_enable(BIT(m_timer_compare_to_saadc_sample_ppi_channel));
       nrfx_gppi_channels_enable(BIT(m_saadc_end_to_saadc_start_ppi_channel));
       ```

#### Testing

7.  Build and flash.
8.  Connect analog input (AIN0) to a voltage source. Ensure shared GND.
9.  Observe serial terminal. You should see logs indicating buffers being filled at regular intervals (determined by `SAADC_BUFFER_SIZE` * `SAADC_SAMPLE_INTERVAL_US`), along with the calculated AVG/MIN/MAX for each buffer. The buffer address reported should alternate between the two allocated buffers.

The solution is in the GitHub repo (`l6/l6_e3_sol`).
