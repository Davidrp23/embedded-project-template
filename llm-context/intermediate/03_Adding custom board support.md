# Adding custom board support

## Overview

In the context of nRF Connect SDK/Zephyr, a **board definition** refers to configuration and initialization files that define the hardware characteristics and behavior of a specific development board or hardware platform.

nRF Connect SDK comes with support for a large number of boards (Development Kits, Prototyping Platforms, and Reference Designs), and it also offers the flexibility of adjusting the hardware specifications of existing board definitions through the use of devicetree overlay files and Kconfig fragments (Covered in the nRF Connect SDK Fundamentals course).

However, there are situations where we would like to create an entirely new board definition with its unique name and unique hardware specifications. This is usually the case when you have developed your schematics and own PCB for your Nordic-based product. We call this a **"custom board"** as the SDK itself does not provide it.

> **Important**: This lesson covers defining boards using Hardware Model V2 (HWMv2) for nRF Connect SDK v3.0.0 - v3.2.0. Hardware Model V1 (HWMv1) has been completely removed in v3.0.0 and is no longer supported.

---

## Board definition

*(Information relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

When developing applications with nRF Connect SDK, the concept of a "board" is central. The SDK's high portability, based on Zephyr RTOS concepts, relies heavily on abstracting hardware differences.

A **Board** is simply the target hardware you want to build an application for. This abstraction makes moving applications between different hardware (e.g., nRF52840 DK to nRF5340 DK, or DK to custom hardware) relatively straightforward. Hardware specifics are decoupled from application code via Devicetree and Kconfig.

Think of a "board" definition as a folder containing several key components:

*   **Devicetree files (`.dts`, `.dtsi`):** Describe the hardware - SoC(s) used, peripherals, pin configurations, memory layout, etc.
*   **Kconfig files (`Kconfig`, `_defconfig`):** Define software/RTOS configurations needed for the hardware (drivers, features).
*   **Metadata files (`board.yml`):** Describe high-level board information - name, vendor, SoC(s), revisions, variants. **This file is mandatory in v3.0+**.
*   **Optional C code (`.c`):** For special hardware initialization routines.
*   **Optional Documentation (`doc/`):** For board specifics.

A board definition includes information about:

*   **SoC configuration:** Exact SoC(s) (e.g., nRF5340 QKAA), clock frequencies, memory details.
*   **Peripheral configuration:** Enabling and configuring available peripherals (UART, SPI, I2C, GPIO, Timers, etc.).
*   **Memory configuration:** RAM/Flash sizes, partitioning (application, bootloader, storage).
*   **Pin mapping:** Assigning SoC pins to specific functions (GPIO, UART TX/RX, etc.).
*   **Clock configuration:** Setting up system clocks.
*   **Interrupt configuration:** Assigning interrupt vectors and priorities.
*   **Driver configuration:** Default settings for drivers used by the board.
*   **Special Initialization Routines:** Custom C code for board-specific setup (muxes, PMICs, etc.).

Luckily, much of the lower-level configuration (Architecture, CPU, SoC Series/Family/Variant) is provided by the SDK. You primarily focus on the board-specific layer.

### Hardware support hierarchy (HWMv2)

nRF Connect SDK v3.0+ uses Zephyr's Hardware Model V2 (HWMv2), organized in layers from specific to general:

1.  **Board:** Your specific hardware implementation (DK, custom board).
2.  **SoC:** Exact System-on-Chip used (e.g., nrf52833_qiaa). *(Provided by SDK)*
3.  **SoC Family:** Group of similar SoCs (e.g., nrf52). *(Provided by SDK)*
4.  **SoC Series:** Tightly related SoCs (e.g., nrf52x). *(Provided by SDK)*
5.  **CPU Core:** Specific CPU (e.g., cortex_m4). *(Provided by SDK)*
6.  **Architecture:** Instruction Set Architecture (e.g., arm). *(Provided by SDK)*

Key characteristics of HWMv2:

*   **Board targets use `/` separator:** Board targets now use forward slashes instead of underscores (e.g., `nrf5340dk/nrf5340/cpuapp` instead of `nrf5340dk_nrf5340_cpuapp`).
*   **Boards located under `/boards/<vendor>/`:** No longer organized by architecture (`/boards/arm/`).
*   **Better multi-target support:** Improved integration with Sysbuild for managing multi-image builds (e.g., MCUboot, network cores).
*   **YAML-based metadata:** Uses `.yml` files for describing boards, SoCs, revisions, variants, providing flexibility.

**Example: nRF5340 DK**
*   **Board:** `nrf5340dk`
*   **SoC:** `nrf5340`
*   **SoC Family:** `nrf53`
*   **SoC Series:** `nrf53x`
*   **CPU Cores:** `cortex_m33` (Application), `cortex_m33` (Network)
*   **Architecture:** `arm`

**Board Targets for nRF5340 DK:**
*   `nrf5340dk/nrf5340/cpuapp` - Application core without TF-M
*   `nrf5340dk/nrf5340/cpuapp/ns` - Application core with TF-M (Non-Secure)
*   `nrf5340dk/nrf5340/cpunet` - Network core

*(Table showing hierarchy for other Nordic DKs)*

### A note on drivers

Drivers for external components (sensors, displays, etc.) connected via peripherals (I2C, SPI) are *not* part of the board definition folder. They reside in dedicated driver locations within the SDK (`zephyr/drivers/`, `nrf/drivers/`). Drivers are matched to devicetree nodes via the `compatible` property. If a driver doesn't exist for your component, you may need to implement one.

> **More on this**: Creating custom drivers is covered in Lesson 7 of the Intermediate course.

---

## Creating board files

*(Information relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

This topic covers creating custom board files, naming conventions, file structure, and the role of each file in HWMv2.

> **Note**: The "Create a new board" GUI in VS Code uses the `west ncs-genboard` extension command, which can also be used via CLI.

### Naming your custom board

*   **Unique Name:** Must be unique across all boards known to `west`. Run `west boards` to see existing names.
*   **No SoC in Name:** HWMv2 doesn't require including the SoC in the board name; it's specified in `board.yml`.
*   **Board ID vs. Human Name:**
    *   The VS Code GUI asks for a **human-readable name** (can have spaces, caps, e.g., "My Custom Board").
    *   It automatically generates a **board ID** (used by build tools) from the human name (lowercase, spaces replaced by underscores, e.g., `my_custom_board`).

### HWMv2 - Board terminology

*(Diagram description: BL5340 DVK example showing terms)*

*   **Board Name (Board ID):** Machine-readable identifier (e.g., `devacademyl3e1`).
*   **Board Revision:** Optional version identifier (e.g., `@0.2.0`). Captures hardware changes.
*   **Board Qualifiers:** Additional tokens after the name/revision, separated by `/`. Define specific build targets.
    *   **SoC/CPU Cluster:** Identifies the target processor on multi-processor boards (e.g., `cpuapp`, `cpunet`, `cpuflpr`). Defined by the SoC layer.
    *   **Variant:** Designates specific configurations or build types (e.g., `ns` for Non-Secure/TF-M build, `xip` for execute-in-place, `sense` for a sensor-equipped hardware variant).
*   **Board Target:** The full string used by build tools (e.g., `west build -b <target>`). Combines name, revision (optional), and qualifiers. **In v3.0+, uses `/` as separator.**
    *   Single-core example: `devacademyl3e1/nrf52833`
    *   Multi-target example: `nrf5340dk/nrf5340/cpuapp/ns`

### Where to define your custom board

1.  **Upstream Zephyr:** For public DKs, modules, reference designs, open-source products. Requires documentation and maintainer review.
2.  **Dedicated Directory (Out-of-Tree):** Recommended for closed-source products. Store board files outside the SDK/Zephyr tree. Pass the directory path to the build system (`-DBOARD_ROOT=`). **(Method used in this lesson's exercises).**
3.  **Application `boards/` Folder:** Suitable for prototyping/debugging within a specific application.

### Board files (HWMv2 Structure)

Assuming a board ID `devacademyl3e1` from vendor `<vendor>`:

**Mandatory Files:**

```
boards/<vendor>/devacademyl3e1/
├── board.yml                     # Core board metadata (SoC, revisions, variants) - MANDATORY in v3.0+
├── Kconfig.devacademyl3e1          # Selects SoC Kconfig support
├── devacademyl3e1_<qualifiers>.dts # Board-level Devicetree (one per target)
└── devacademyl3e1_<qualifiers>-pinctrl.dtsi # Pin control definitions (one per target)
```
*(Note: `<qualifiers>` part is omitted if the board only has one target)*

**Optional & Special Use Case Files:**

```
boards/<vendor>/devacademyl3e1/
├── ... (mandatory files) ...
├── devacademyl3e1_<qualifiers>_defconfig # Board default Kconfig settings (one per target)
├── Kconfig.defconfig                   # Default values for board Kconfig options
├── board.cmake                         # Flash/debug runner support
├── CMakeLists.txt                      # Add custom C init files (rarely needed)
├── <board_init_files>.c                # Custom C init code (rarely needed)
├── doc/                                # Documentation (for upstreaming)
│   ├── devacademyl3e1.png
│   └── index.rst
├── Kconfig                             # Defines board-specific Kconfig menu/options
├── devacademyl3e1_<qualifiers>.yml     # Twister (test runner) metadata (one per target)
├── devacademyl3e1_<qualifiers>_<revision>.conf    # Kconfig fragment for specific revision
├── devacademyl3e1_<qualifiers>_<revision>.overlay # Devicetree overlay for specific revision
└── dts/                                # Optional subfolder for DTS includes/bindings
    └── bindings/                       # Custom devicetree bindings
```

#### File Details:

1.  **`board.yml` (Mandatory):**
    *   Defines board `name` (ID), `vendor`. Optionally `full_name` (human-readable).
    *   Lists `socs` used (at least one).
    *   Optionally defines `revisions` and `variants`.
    *   CPU clusters are inherited from the SoC definition.
    *   VS Code extension auto-generates this; usually needs minimal changes.
    *   **This file is mandatory in v3.0+ - boards without board.yml will not be recognized.**
    *(Examples shown for nRF52840 Dongle, nRF5340 DK)*

2.  **`Kconfig.<board_id>` (Mandatory):**
    *   Selects the Kconfig support for the specific SoC variant(s) used (e.g., `select SOC_NRF52833_QIAA`).
    *   Defines the `BOARD_<NORMALIZED_BOARD_NAME>` symbol (e.g., `config BOARD_DEVACADEMYL3E1`).
    *   Uses `if/endif` blocks based on normalized board target for multi-target boards.
    *(Examples shown for single-target devacademyl3e1 and multi-target nRF5340 DK)*

3.  **`<board_target>.dts` (Mandatory, one per target):**
    *   Board-level devicetree. Includes the base SoC DTS (`#include <nordic/nrf52833_qiaa.dtsi>`).
    *   Defines board-specific hardware: connectors, LEDs, buttons, sensors, enabled peripherals, memory partitioning (`chosen` node).
    *   Can include `<board_target>-pinctrl.dtsi`.
    *   VS Code DeviceTree editor helps populate this.

4.  **`<board_target>-pinctrl.dtsi` (Mandatory, one per target):**
    *   Defines pin multiplexing configurations for peripherals using the `&pinctrl` node structure. Maps peripheral signals (e.g., `UART_TX`) to physical pins (e.g., `P0.06`).

5.  **`<board_target>_defconfig` (Optional but Recommended, one per target):**
    *   Kconfig fragment automatically included for builds targeting this board.
    *   Should enable the *bare minimum* hardware support needed for the board to function (e.g., GPIO, basic console if present).
    *   Application `prj.conf` is responsible for enabling application-specific features.
    *(Example shows adding UART, RTT, GPIO support for devacademyl3e1)*

6.  **`Kconfig.defconfig` (Optional but Recommended):**
    *   Sets *default values* for Kconfig options *specific to this board*, often conditionally based on the build target.
    *   Uses `if BOARD_<NORMALIZED_TARGET> / endif` blocks.
    *   Example: Setting default `BT_CTLR` if `CONFIG_BT` is enabled by the application.
    *(Examples shown for devacademyl3e1 and nRF5340 DK)*

7.  **Other Optional Files:**
    *   `board.cmake`: Defines flash/debug commands (e.g., which runner `nrfjprog`, `jlink`).
    *   `CMakeLists.txt` / `<init>.c`: Add custom C initialization code run pre/post-kernel (rare).
    *   `doc/`: Documentation files (required for upstreaming to Zephyr).
    *   `Kconfig`: Defines board-specific Kconfig options/menus visible in tools like `menuconfig`.
    *   `<board_target>.yml`: Twister test framework metadata.
    *   `<board_target>_<revision>.conf/.overlay`: Revision-specific Kconfig/DTS fragments applied on top of base files when building for that revision (e.g., `myboard@1.0.0`). Update `board.yml` to define revisions.

> **Note**: When creating a custom board, examining existing DK definitions for the same SoC in `<sdk>/zephyr/boards/nordic/` and `<sdk>/nrf/boards/nordic/` is highly recommended. Also see the [Zephyr Board Porting Guide](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/hardware/porting/board_porting.html).

---

## Board files for multi-core hardware & TF-M

*(Information relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

This topic covers specifics for defining boards based on SoCs with multiple cores or TrustZone-M support (nRF53, nRF91 Series).

Key differences from nRF52 Series:
*   **Architecture:** Armv8-M (vs. Armv7-M).
*   **CPU:** Cortex-M33 (vs. Cortex-M4).
*   **TrustZone-M:** Hardware security extension enabling Secure (SPE) and Non-Secure (NSPE) Processing Environments, often used with **Trusted Firmware-M (TF-M)**.
*   **Multiple Cores:** nRF53 has Application + Network cores.

### Trusted Firmware-M (TF-M)

TF-M provides a Secure Processing Environment (SPE). Applications typically run in the Non-Secure Processing Environment (NSPE).

*   **Build Target Option 1: `<board_target>/<cpu_cluster>/ns`** (e.g., `nrf9151dk/nrf9151/ns`, `nrf5340dk/nrf5340/cpuapp/ns`)
    *   Enforces security separation.
    *   Application built as NSPE image.
    *   TF-M (or another SPE image like SPM) is automatically built as the SPE image.
    *   Images are merged for flashing. **(Required for many nRF Connect SDK samples, especially networking/modem related).**
*   **Build Target Option 2: `<board_target>/<cpu_cluster>`** (e.g., `nrf9151dk/nrf9151`, `nrf5340dk/nrf5340/cpuapp`)
    *   No security separation (TF-M disabled).
    *   Application built as a single image with full privileges.

*(Diagram description: Comparing Secure/Non-Secure builds)*

### Working with the nRF53 Series (nRF5340)

*   **Network core (cpunet):** Low-power M33 core for radio protocols. Build target: `<board>/nrf5340/cpunet`.
*   **Application core (cpuapp):** High-performance M33 core with TrustZone. Build targets:
    *   `<board>/nrf5340/cpuapp` (TF-M disabled)
    *   `<board>/nrf5340/cpuapp/ns` (TF-M enabled, application runs as NSPE)

**Example board targets for nRF5340 DK:**
```
west build -b nrf5340dk/nrf5340/cpuapp        # Application core, no TF-M
west build -b nrf5340dk/nrf5340/cpuapp/ns     # Application core, with TF-M
west build -b nrf5340dk/nrf5340/cpunet        # Network core
```

### Working with the nRF91 Series (nRF9160/61/51/31)

*   Single user-programmable Cortex-M33 core with TrustZone.
*   Separate modem core (runs precompiled firmware).
*   Build targets:
    *   `<board>/nrf91xx` (TF-M disabled - *naming varies slightly based on exact SiP*)
    *   `<board>/nrf91xx/ns` (TF-M enabled, application runs as NSPE)

**Example board targets for nRF9151 DK:**
```
west build -b nrf9151dk/nrf9151               # Without TF-M
west build -b nrf9151dk/nrf9151/ns            # With TF-M (Non-Secure)
```

### Enabling TF-M in board definition

Assuming a custom board `devacademyl3e2` based on nRF9151.

**In Kconfig and Metadata Files:**

1.  **`board.yml`:** Must define the `ns` variant for the appropriate SoC.
    ```yaml
    board:
      name: devacademyl3e2
      vendor: nordic
      socs:
      - name: nrf9151 # Assumes SoC definition provides cpuapp cluster implicitly
        variants:
        - name: 'ns' # Define the non-secure variant
    ```
2.  **`Kconfig.defconfig`:** Conditionally set `FLASH_LOAD_OFFSET` and `FLASH_LOAD_SIZE` based on whether building the secure (`BOARD_...`) or non-secure (`BOARD_..._NS`) target, typically using values from devicetree partitions (`dt_chosen...` macros).
    *(See original text for full Kconfig.defconfig example logic)*
3.  **Two `_defconfig` files:**
    *   `devacademyl3e2_nrf9151_defconfig` (for secure/non-TF-M build): Must enable `CONFIG_ARM_TRUSTZONE_M=y`. Include other board defaults (GPIO, UART, etc.).
    *   `devacademyl3e2_nrf9151_ns_defconfig` (for non-secure build): Must enable `CONFIG_ARM_TRUSTZONE_M=y` *and* `CONFIG_TRUSTED_EXECUTION_NONSECURE=y`. Include other board defaults.

**In Devicetree Files:**

4.  **Memory Partitioning:** Define memory regions for Secure (`sram0_s`, `slot0_partition`) and Non-Secure (`sram0_ns`, `slot0_ns_partition`) execution. This is often done by including a common partitioning file provided by the SDK (e.g., `<sdk>/dts/common/nordic/nrf91xx_partition.dtsi`).
    4.1 Create a common DTS include file (e.g., `devacademyl3e2_nrf9151_common.dtsi`) that includes the common pinctrl file and the SoC partitioning file.
       > **More on this**: If using MCUboot, the Partition Manager often overrides DTS partitioning. Covered in Lesson 9.
    4.2 Create separate board DTS files for each target (`devacademyl3e2_nrf9151.dts` and `devacademyl3e2_nrf9151_ns.dts`).
       *   Each includes the common DTS file (`#include "devacademyl3e2_nrf9151_common.dtsi"`).
       *   Each includes the appropriate SoC definition (`nrf9151_laca.dtsi` or `nrf9151ns_laca.dtsi`).
       *   Each uses the `chosen` node to select the correct flash (`zephyr,code-partition`) and SRAM (`zephyr,sram`) partitions for that specific build target (secure vs. non-secure).
       *   The non-secure DTS (`_ns.dts`) should disable peripherals allocated to the secure domain (e.g., `&uart1 { status = "disabled"; };` if UART1 is used by TF-M).
    *(See original text for example `.dts` content)*

---

## Exercise 1 - Custom board for single-core SoC

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

Create a custom board definition (`DevAcademyL3E1`) based on nRF52833 DK schematics (minus Arduino headers), using the VS Code "Create a new board" tool.

### Exercise steps

1.  Create a root directory for custom boards (e.g., `C:\my_boards`).
2.  In VS Code -> Welcome page -> **Create a new board**.
3.  Follow prompts:
    *   Board Name: `DevAcademyL3E1` (Human readable)
    *   Board ID: `devacademyl3e1` (Auto-generated)
    *   SDK Version: Select your v3.0.0+ version.
    *   SoC: `nrf52833_qiaa` (Select exact variant).
    *   Board Root: `C:\my_boards` (Directory from step 1).
    *   Vendor: `nordic` (Or your company name, no spaces).
4.  **Add Board Root to VS Code Settings:** File -> Preferences -> Settings -> Extensions -> nRF Connect -> Board Roots -> Add Item -> Enter `C:\my_boards`.
5.  **Open Custom Board Folder:** File -> Open Folder -> `C:\my_boards\devacademyl3e1`.
6.  **Edit `devacademyl3e1_defconfig`:** Add default Kconfigs for basic functionality based on DK schematic (assuming interface MCU provides these):
    ```kconfig
    # Append these lines
    CONFIG_USE_SEGGER_RTT=y
    CONFIG_GPIO=y
    CONFIG_SERIAL=y
    CONFIG_CONSOLE=y
    CONFIG_UART_CONSOLE=y
    ```
7.  **Create/Build Hello World:** Create a new application from the Hello World sample. Add a build configuration targeting your *new* custom board (`devacademyl3e1/nrf52833`). Build it (it might not run correctly yet, but building enables the DeviceTree Visual Editor).
    ```bash
    west build -b devacademyl3e1/nrf52833 -- -DBOARD_ROOT=C:/my_boards
    ```
8.  **Populate Devicetree using Editor/Text:**
    *   Open DeviceTree Visual Editor (Actions Panel).
    *   Open `devacademyl3e1.dts` and `devacademyl3e1-pinctrl.dtsi` in text editor.
    *   **Enable GPIO/GPIOTE:** Add to `devacademyl3e1.dts`:
        ```devicetree
        &gpiote { status = "okay"; };
        &gpio0 { status = "okay"; };
        &gpio1 { status = "okay"; }; // Assuming nRF52833 has gpio1
        ```
    *   **Define LEDs (P0.13-P0.16, Active Low):** Add inside root (`/`) node in `devacademyl3e1.dts`:
        ```devicetree
            leds {
                compatible = "gpio-leds";
                led0: led_0 { gpios = <&gpio0 13 GPIO_ACTIVE_LOW>; label = "Green LED 0"; };
                led1: led_1 { gpios = <&gpio0 14 GPIO_ACTIVE_LOW>; label = "Green LED 1"; };
                led2: led_2 { gpios = <&gpio0 15 GPIO_ACTIVE_LOW>; label = "Green LED 2"; };
                led3: led_3 { gpios = <&gpio0 16 GPIO_ACTIVE_LOW>; label = "Green LED 3"; };
            };
        ```
    *   **Define Buttons (P0.11, P0.12, P0.24, P0.25, Active Low, Pull-up):**
        *   Add `#include <zephyr/dt-bindings/input/input-event-codes.h>` at top of `devacademyl3e1.dts`.
        *   Add inside root (`/`) node in `devacademyl3e1.dts`:
            ```devicetree
            buttons {
                compatible = "gpio-keys";
                button0: button_0 { gpios = <&gpio0 11 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>; label = "Push button 0"; zephyr,code = <INPUT_KEY_0>; };
                button1: button_1 { gpios = <&gpio0 12 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>; label = "Push button 1"; zephyr,code = <INPUT_KEY_1>; };
                button2: button_2 { gpios = <&gpio0 24 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>; label = "Push button 2"; zephyr,code = <INPUT_KEY_2>; };
                button3: button_3 { gpios = <&gpio0 25 (GPIO_PULL_UP | GPIO_ACTIVE_LOW)>; label = "Push button 3"; zephyr,code = <INPUT_KEY_3>; };
            };
            ```
    *   **Configure UART0 (P0.06=TX, P0.08=RX, P0.05=RTS, P0.07=CTS):**
        *   Add to `devacademyl3e1.dts`:
            ```devicetree
            &uart0 {
                compatible = "nordic,nrf-uarte";
                status = "okay";
                current-speed = <115200>;
                pinctrl-0 = <&uart0_default>;
                pinctrl-1 = <&uart0_sleep>;
                pinctrl-names = "default", "sleep";
            };
            ```
        *   Add inside `&pinctrl` node in `devacademyl3e1-pinctrl.dtsi`:
            ```devicetree
            uart0_default: uart0_default { /* Same as nRF52833 DK */ ... };
            uart0_sleep: uart0_sleep { /* Same as nRF52833 DK */ ... };
            ```
            *(Copy exact content from nRF52833 DK pinctrl file or exercise solution)*
        *   Add chosen properties inside `/ { chosen { ... } };` in `devacademyl3e1.dts`:
            ```devicetree
            zephyr,console =  &uart0;
            zephyr,shell-uart =  &uart0;
            zephyr,uart-mcumgr = &uart0; // If needed
            ```
    *   **Configure I2C0 (P0.26=SDA, P0.27=SCL):**
        *   Add to `devacademyl3e1-pinctrl.dtsi`: `i2c0_default`, `i2c0_sleep` definitions (copy from DK/solution).
        *   Add to `devacademyl3e1.dts`:
            ```devicetree
            &i2c0 { status = "okay"; pinctrl-0 = <&i2c0_default>; ... };
            ```
    *   **Configure SPI1 (P0.31=SCK, P0.30=MOSI, P1.08=MISO):**
        *   Add to `devacademyl3e1-pinctrl.dtsi`: `spi1_default`, `spi1_sleep` definitions (copy from DK/solution).
        *   Add to `devacademyl3e1.dts`:
            ```devicetree
            &spi1 { status = "okay"; pinctrl-0 = <&spi1_default>; ... };
            ```
    *   **Configure PWM0 (Output on LED0 pin P0.13):**
        *   Add to `devacademyl3e1-pinctrl.dtsi`: `pwm0_default`, `pwm0_sleep` definitions (copy from DK/solution, include `nordic,invert;` if needed for LED).
        *   Add to `devacademyl3e1.dts`:
            ```devicetree
            &pwm0 { status = "okay"; pinctrl-0 = <&pwm0_default>; ... };
            ```
        *   Add PWM LED definition inside root (`/`) node in `devacademyl3e1.dts`:
            ```devicetree
            pwmleds {
                compatible = "pwm-leds";
                pwm_led0: pwm_led_0 {
                    pwms = <&pwm0 0 PWM_MSEC(20) PWM_POLARITY_INVERTED>; // Channel 0, 20ms period, inverted for active-low LED
                };
            };
            ```
    *   **Add Aliases:** Add standard aliases inside `/ { aliases { ... } };` in `devacademyl3e1.dts`:
        ```devicetree
        led0 = &led0; led1 = &led1; led2 = &led2; led3 = &led3;
        pwm-led0 = &pwm_led0;
        sw0 = &button0; sw1 = &button1; sw2 = &button2; sw3 = &button3;
        bootloader-led0 = &led0; mcuboot-button0 = &button0; mcuboot-led0 = &led0;
        watchdog0 = &wdt0; // If watchdog is enabled
        ```
9.  **Testing:**
    *   Build and flash **Hello World** for `devacademyl3e1/nrf52833`. Verify console output.
        ```bash
        west build -b devacademyl3e1/nrf52833 -- -DBOARD_ROOT=C:/my_boards
        west flash
        ```
    *   Build and flash **Button sample** (`zephyr/samples/basic/button`). Verify LED1 toggles on Button 1 press and console output appears.
    *   Build and flash **PWM LED sample** (`zephyr/samples/basic/pwm_blinky`). Verify LED1 fades/blinks and console output appears.
    *   Build and flash a **BLE sample** (e.g., `peripheral_uart`). Verify advertising/connection. (Radio node usually enabled in SoC DTS).
    *   Test I2C/SPI with external hardware following relevant lessons/samples.

The solution for this exercise can be found in the GitHub repository for this course.

---

## Exercise 2 - Custom board for a multi-core & TF-M capable SoC/SiP

*(Instructions relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

Create a custom board definition (`DevAcademyL3E2`) based on nRF9151 DK, demonstrating multi-target support (with/without TF-M). This involves copying and renaming existing DK files.

### Exercise steps

#### Preparing the template

1.  Create root directory `C:\my_boards`.
2.  **Copy DK Folder:** Copy the entire `<sdk_path>/zephyr/boards/nordic/nrf9151dk` folder into `C:\my_boards`.
    > **Important**: If your target SiP/SoC is different (e.g., nRF5340), copy the corresponding DK folder (e.g., `nrf5340dk`) from `<sdk_path>/zephyr/boards/nordic/`.
3.  **Add Board Root to VS Code Settings** (as done in Exercise 1, Step 4).
4.  **Target Name:** We will name the board `DevAcademyL3E2` (human), `devacademyl3e2` (ID). Targets will be `devacademyl3e2/nrf9151` and `devacademyl3e2/nrf9151/ns`.
5.  **Rename Files and Content:**
    5.1 **Rename Folder:** Rename `C:\my_boards\nrf9151dk` to `C:\my_boards\devacademyl3e2`.
    5.2 **Open Folder:** Open `C:\my_boards\devacademyl3e2` in VS Code (ensure it's the only folder in the workspace).
    5.3 **Rename Files:** Rename all files starting with `nrf9151dk_...` to `devacademyl3e2_...`. Rename `Kconfig.nrf9151dk` to `Kconfig.devacademyl3e2`.
    5.4 **Delete `doc` folder.**
    5.5 **Replace Content (Case Sensitive):**
        *   Edit -> Replace in Files (Ctrl+Shift+H).
        *   Search for `nrf9151dk` (Match Case ON), Replace with `devacademyl3e2`. Click Replace All.
        *   Search for `NRF9151DK` (Match Case ON), Replace with `DEVACADEMYL3E2`. Click Replace All.
        *   Search for `nRF9151 DK` (Match Case ON), Replace with `DevAcademyL3E2`. Click Replace All.
        *(This updates `board.yml`, includes, Kconfig symbols, CMake files, comments etc.)*

#### Adjusting the template to match your schematic / Software Configuration

6.  **Modify Files:** Now that you have a renamed template, modify the `.dts`, `-pinctrl.dtsi`, `_defconfig`, and potentially `Kconfig.defconfig` files within `C:\my_boards\devacademyl3e2` to match the *actual* hardware schematics and desired default software configurations for *your* custom `DevAcademyL3E2` board. Use the principles from Exercise 1 and the multi-target/TF-M concepts covered earlier (e.g., ensuring correct partition selection in `Kconfig.defconfig`, enabling `TRUSTED_EXECUTION_NONSECURE` in the `_ns_defconfig`, etc.).

#### Testing

7.  **Build and Flash Samples:**
    *   Create new applications based on samples.
    *   In the "Add Build Configuration" window, you should now see your custom board `devacademyl3e2` listed under "Custom Boards", with its targets (`devacademyl3e2/nrf9151`, `devacademyl3e2/nrf9151/ns`).
    *   **Test Serial Console:** Build/flash Hello World for both targets (`.../ns` and non-`ns`). Verify output. Note the TF-M child image build for the `ns` target.
        ```bash
        west build -b devacademyl3e2/nrf9151 -- -DBOARD_ROOT=C:/my_boards
        west build -b devacademyl3e2/nrf9151/ns -- -DBOARD_ROOT=C:/my_boards
        ```
    *   **Test LEDs/Buttons:** Build/flash Button sample for `.../ns` target. Verify functionality.
    *   **Test PWM:** Build/flash PWM LED sample for `.../ns` target. Verify functionality.
    *   **Test Modem:** Build/flash AT Client or AT Monitor sample for `.../ns` target (required for modem samples). Verify AT commands work.
    *   **Test I2C/SPI:** Use external hardware and relevant samples/lessons.

    > **Important**: For complex samples that include their own `boards/` overlays/configs (like `nrf_cloud_multi_service`), you may need to copy the existing `nrf9151dk_nrf9151_ns.conf/.overlay` files within that *sample's* `boards/` directory and rename the copies to match your custom board target (e.g., `devacademyl3e2_nrf9151_ns.conf/.overlay`) for the sample to build correctly for your board.

The solution for this exercise can be found in the GitHub repository for this course (`l3/l3_e2_sol` select version).

---

## Migration from v2.x to v3.0

If you have existing custom board definitions created for nRF Connect SDK v2.x, you may need to update them for v3.0+:

### Key changes in v3.0.0

1.  **board.yml is now mandatory:** All boards must have a `board.yml` file. Boards without this file will not be recognized.

2.  **Board target syntax changed:** Use `/` instead of `_` as separator:
    *   Old (v2.x): `nrf5340dk_nrf5340_cpuapp`
    *   New (v3.0+): `nrf5340dk/nrf5340/cpuapp`

3.  **Directory structure:** Boards are located under `boards/<vendor>/` (e.g., `boards/nordic/nrf5340dk/`).

4.  **HWMv1 removed:** Hardware Model V1 is no longer supported. All boards must use HWMv2 structure.

### Migration steps

1.  Ensure your board has a valid `board.yml` file
2.  Update any build scripts or CI pipelines to use the new board target syntax with `/`
3.  Update overlay file names if they reference board targets (replace `_` with `_` in filenames, but use `/` in build commands)
4.  Verify board builds correctly with the new SDK version

For more information, see the [nRF Connect SDK v3.0.0 Migration Guide](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/releases_and_maturity/migration/migration_guide_3.0.html).
