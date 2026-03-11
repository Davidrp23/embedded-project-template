## Elements of an nRF Connect SDK application

### Overview

In nRF Connect SDK, an application has a number of different elements that are used by the build system in some way to create the final runnable file. Understanding these elements, why they are needed, and how they interact with each other is crucial when creating your own application. In this lesson, we will explore each of these elements to understand how they all function in relation to each other. In the exercise portion, we will create a minimal working application from scratch and add our own custom files and configurations to customize the application.

> **What's new in nRF Connect SDK v3.0.0:**
> - **Hardware Model v1 removed**: All boards must now use Hardware Model v2 with `/` separator in board targets (e.g., `nrf5340dk/nrf5340/cpuapp` instead of `nrf5340dk_nrf5340_cpuapp`). Board files are located in `boards/nordic/` instead of the old `boards/arm/` structure.
> - **Multi-image builds removed**: The parent-child image system has been completely removed. All multi-image projects must use Sysbuild.
> - **Flashing with nRFUtil**: The `west flash` command now uses nRFUtil instead of nrfjprog for more consistent behavior.
> - **Sysbuild is mandatory**: For any multi-image application (bootloaders, multi-core), Sysbuild is the only supported build system.

Typical application structure:
```
app/
|-- CMakeLists.txt
|-- Kconfig
|-- prj.conf
|-- <board_name>.overlay
|-- src/
    |-- main.c
```

---

## Configuration files

*(Información relevante para nRF Connect SDK v3.0.0 – v3.2.0)*

### Application and board configurations

Each application in the nRF Connect SDK must have an **application configuration file**, usually called `prj.conf`, that describes the software modules and kernel services used and their settings. The application configuration file is text-based and contains configuration options (often called symbols) in the form of:

```kconfig
CONFIG_<symbol_name>=<value>
```

Each configuration option must start with the prefix `CONFIG_` followed by the name of the software module to configure, then the value to be set, with no spaces around the equals sign.

Let’s take a look at the application configuration file `prj.conf` from the blinky sample, which we covered in-depth in the previous lesson.

This file has only one line, seen below, that includes the GPIO driver.
```kconfig
CONFIG_GPIO=y
```
In other words, setting the `CONFIG_GPIO` symbol will enable the inclusion of the source code of the GPIO driver into the build process and hence our application will be able to use it.

In addition to the application configuration file, an application inherits the **board configuration file**, `<board_name>_defconfig`, of the board that the application is built for.

Let’s take an example, the nRF52833 DK has the board configuration file `nrf52833dk_nrf52833_defconfig` available in `<nRF Connect SDK Installation Path>\zephyr\boards\nordic\nrf52833dk`.

```kconfig
# SPDX-License-Identifier: Apache-2.0

CONFIG_SOC_SERIES_NRF52X=y
CONFIG_SOC_NRF52833_QIAA=y
CONFIG_BOARD_NRF52833DK_NRF52833=y

# Enable MPU
CONFIG_ARM_MPU=y

# Enable hardware stack protection
CONFIG_HW_STACK_PROTECTION=y

# Enable RTT
CONFIG_USE_SEGGER_RTT=y

# enable GPIO
CONFIG_GPIO=y

# enable uart driver
CONFIG_SERIAL=y

# enable console
CONFIG_CONSOLE=y
CONFIG_UART_CONSOLE=y

# additional board options
CONFIG_GPIO_AS_PINRESET=y
```
*Board configuration file for the nRF52833 DK: nrf52833dk_nrf52833_defconfig*

> **Note**
> Configuration options can be set both in the application configuration file and the board configuration file. In the example, `CONFIG_GPIO` symbol is set in both files. In this case, the value in the application configuration file always takes precedence.

The first three lines in the board configuration file includes software support specific to the nRF52833 DK and its nRF52833 SoC. The `CONFIG_ARM_MPU` is for the memory protection unit. This brings us to the question: How can we know the meaning of all these configuration options?

If you are using nRF Connect for VS Code, simply hovering the mouse over the configuration symbol will show a box that contains a description about the symbol.

Additionally, you can visit the [documentation webpage](https://docs.nordicsemi.com/bundle/ncs-latest/page/kconfig/index.html) and find the documentation for the specific symbol, for instance, `CONFIG_GPIO`.

Further examining the board configuration file of the nRF52833 DK, we can see that the UART driver (`CONFIG_SERIAL`), the RTT drivers, the libraries (`CONFIG_USE_SEGGER_RTT`), and the console drivers (`CONFIG_CONSOLE`) are included. `CONFIG_UART_CONSOLE` will make UART the default console output. All of these software components are core to interacting with a console.

> **Note**
> You should **never** modify any board configuration files. Instead, rely on the application configuration file (`prj.conf`) to set new configurations and subsequently overwrite any default board configurations if needed. If you change the board configuration file directly, then these changes will apply for all projects using that board.

### Kernel Configuration (nRF Kconfig GUI)

In the nRF Connect SDK, symbols are grouped into menus and sub-menus to easily browse all available configurations.

An alternative way to modify the contents of the `prj.conf` (application configuration file) is by using the **nRF Kconfig GUI**. Kconfig refers to the software configurations and it groups all functionalities provided by the nRF Connect SDK and Zephyr into menus and submenus, which can be viewed in a graphical tree format.

The nRF Kconfig GUI view allows us to browse and implement the different functionalities available easily. Selecting/deselecting functionalities in the nRF Kconfig GUI corresponds to adding/removing lines in the `prj.conf` file.

nRF Kconfig GUI can be found under **Actions** in the nRF Connect extension in VS Code.

> **Important**
> Starting from nRF Connect SDK v2.8.0, Sysbuild is the default build system (and the **only** supported multi-image build system as of v3.0.0). nRF Kconfig GUI allows you to display the Kconfig configurations for Sysbuild itself and any of its images in the project. This is done by first selecting the image of interest from the **APPLICATIONS** view and then clicking on the **nRF Kconfig GUI** in the **ACTIONS** view.

There are other visual editors for Kconfig; you can access them through the submenu under nRF Kconfig GUI. You can also change the default Kconfig visual editor via `File -> Preferences -> Settings -> Extensions -> nRF Connect -> Kconfig Interface`.

Kconfig is organized into menus that group related configurations together, such as Modules, Device Drivers, C Library, and Boot Options. Menus can have submenus, and submenus can have submenus of their own. If we expand any of these menus, for instance, Device Drivers, all symbols related to this category will show up, some with menus of their own.

*(Description of image: Kconfig tree structure in nRF Connect for VS Code)*

A checked box indicates that this symbol is enabled, either in the application configuration file or in the board configuration file. For example, searching for “GPIO” shows that the GPIO drivers are enabled in the blinky sample configuration.

*(Description of image: GPIO driver enabled in Kconfig)*

After making changes in the Kconfig menu, there are 3 different ways to save those changes:
*(Description of image: Save options in Kconfig GUI)*

1.  **Apply:** Saves changes to a temporary configuration file (`build/zephyr/.config`). These changes are reverted on a pristine build.
2.  **Save to file:** Saves changes directly to `prj.conf`, preserving them across builds.
3.  **Save minimal:** Saves only the changes made into a separate file.

> **More on this**
> There are two interactive Kconfig interfaces (`nRF Kconfig GUI`, `menuconfig`) supported by nRF Connect for VS Code. They all allow you to explore related configuration options and know their values. One advantage of `menuconfig` is that it lists the dependencies and where the Symbol is set. However, the disadvantage of `menuconfig` is that configurations are only set temporarily and will be lost any time you do a clean (pristine) build. On the other hand, `nRF Kconfig GUI` is the only graphical interface that can allow you to save the configurations permanently in `prj.conf`.

---

## Devicetree overlays, CMake, and build systems

### Devicetree overlays

In lesson 2, we covered the devicetree, a hierarchical data structure that describes hardware through nodes with belonging properties. It is not recommended to modify the base devicetree files directly. Instead, we use **devicetree overlays** to modify the hardware description for a specific application. The overlay only needs to include the node and property it wants to modify.

```devicetree
/* Example overlay enabling SPI1 and configuring MOSI pin */
&spi1{
    status = "okay";
};

&pinctrl {
    spi1_default: spi1_default {
        group1 {
            psels = <NRF_PSEL(SPIM_MOSI, 0, 25)>;
        };
    };
    spi1_sleep: spi1_sleep {
        group1 {
            psels = <NRF_PSEL(SPIM_MOSI, 0, 25)>;
        };
    };
};
```
The overlay file shown above will set node `spi1` to have the status `okay`, essentially enabling this node. Then it is changing the pin configuration for the `SPIM_MOSI` line to pin `P0.25` by changing the appropriate sub-nodes and properties in the `&pinctrl` node. Note that you must change the pin configuration for both the `default` and `sleep` states if modifying pin control this way.

If an overlay file sets a node’s property to a value it already has, the node will just remain unchanged.

One easy way to create an overlay file is to create a file with the name of the board and the extension `.overlay` (e.g., `nrf52833dk_nrf52833.overlay`) and place it directly in the application root directory. The build system will automatically search for this file type and include it if it finds one.

However, there are several other ways to include overlay files. See [Set devicetree overlays](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/build/dts/howtos.html#set-devicetree-overlays) for a list of ways to include a devicetree overlay in your application.

> **Note**
> Overlays are also DTS files; the `.overlay` extension is just a convention that makes their purpose clear.

### CMake

In the nRF Connect SDK, all applications are **CMake** projects. This means that the application controls the configuration and build process of itself, Zephyr, and all sourced libraries. The file `CMakeLists.txt` is the main CMake project file and the source of this build process configuration.

We will take a closer look at some of the functions in the exercise.

### Sysbuild

**Sysbuild** is a high-level build system that simplifies the management of complex multi-image builds. It is an improved and extensible build system for multi-image builds, replacing the nRF Connect SDK-specific Multi-image builds system used in older nRF Connect SDK versions.

Sysbuild is mainly used for two use cases: **Multi-core applications** and **bootloaders**.

Sysbuild became available in nRF Connect SDK version 2.7.0 and has been the default build system since version 2.8.0. In nRF Connect SDK v3.0.0, **the previous multi-image builds system (parent-child images) was completely removed**, making Sysbuild the only supported method for multi-image builds.

Sysbuild works by configuring and building at least a Zephyr application and, optionally, as many additional projects as you want. The additional projects can be either Zephyr applications or other types of builds.

To distinguish CMake variables and Kconfig options specific to the underlying build systems, sysbuild uses **namespacing**. For example, sysbuild-specific Kconfig options are preceded by `SB_` before `CONFIG` and application-specific CMake options are preceded by the application name.

Sysbuild is integrated with `west`. The sysbuild build configuration is generated using the sysbuild’s `CMakeLists.txt` file (which provides information about each underlying build system and CMake variables) and the sysbuild’s Kconfig options (which are usually gathered in the `sysbuild.conf` file).

> To learn more about Sysbuild, there is a dedicated lesson on it in the nRF Connect SDK Intermediate course – Lesson 8- Sysbuild

#### A note on Kconfig.sysbuild

For the nRF5340, we need to run an image on the network core if we want to use the Bluetooth LE radio. This is done automatically by the build system using the Sysbuild feature. Here is a brief explanation of how to add an image to the network core, specifically as done in the Bluetooth LE Fundamentals course, the Wi-Fi Fundamentals course, and the nRF Connect SDK samples.

Sysbuild uses Kconfig in a similar way to the `prj.conf`/`something.conf` we know. However, for Sysbuild, they are named `sysbuild.conf` / `Kconfig.sysbuild`.

To select an image for the network core, we set one of the configurations found under the docs for [Sysbuild: Enabling images](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/config_and_build/sysbuild/sysbuild_images.html). In most Bluetooth LE samples, **IPC Radio** is used, so we will also use IPC Radio.

Normally, this can be set in `sysbuild.conf` by adding:
```kconfig
# Enable IPC Radio for the network core
SB_CONFIG_NETCORE_IPC_RADIO=y
# Configure protocol for IPC Radio
SB_CONFIG_NETCORE_IPC_RADIO_BT_HCI_IPC=y
```
However, both in DevAcademy courses and the nRF Connect SDK Bluetooth LE samples set these instead in `Kconfig.sysbuild`, as shown below:
```kconfig
source "${ZEPHYR_BASE}/share/sysbuild/Kconfig"

config NRF_DEFAULT_IPC_RADIO
    default y

config NETCORE_IPC_RADIO_BT_HCI_IPC
    default y

```
This is because `SB_CONFIG_NETCORE_IPC_RADIO` only works for multi-core chips (Ex: nRF5340 SoC), so if this is set in `sysbuild.conf`, the samples would get a warning when building for single-core chips. However, when the configurations are set in `Kconfig.sysbuild`, the warning does not appear.

*(Explanation from original text about why Kconfig.sysbuild avoids the warning by overlaying default based on `SUPPORT_NETCORE_IPC_RADIO`)*

---

## Trusted Firmware-M (TF-M)

**Trusted Firmware-M (TF-M)** is a blueprint for constructing a Secure Processing Environment (SPE) tailored to Arm M-profile architectures. TF-M relies on the principle of security through separation to safeguard sensitive credentials and code. Additionally, TF-M extends its protective capabilities to applications by offering security services, including Protected Storage, Cryptography, and Attestation.

The Nordic Semiconductor Series, which implements the Armv8-M architecture (Arm Cortex-M33), incorporates **TrustZone** technology. This technology enforces hardware-based segregation between Secure and Non-secure Processing Environments, effectively creating separate Trusted and Non-Trusted build images.

This means that we have two options for boards based on the nRF54L/nRF53/nRF91 Series. Either:

*   **Option 1 – `<board_target>/ns`**
    Enforce security by separation by utilizing TF-M. Our application runs in the **Non-Secure Processing Environment (NSPE)** and TF-M runs in the **Secure Processing Environment (SPE)**.
*   **Option 2 – `<board_target>`**
    Do not enforce security by separation. Our application runs as a single image with full access privileges.

Let’s take a board with a board target `<board_target>` as an example. When building an application for the board, you could build for either `<board_target>` or `<board_target>/ns`.

*   `<board_target>`: The application is built as a single image without security by separation.
*   `<board_target>/ns`: The application will be built as a Non-Secure image. Hence, you will get security by separation as TF-M will automatically be built as the Secure image. The two images will be merged to form a combined image that will be used when programming or updating the device.

*(Diagram description from original text comparing Option 1 and Option 2)*

> **Note**
> In nRF Connect SDK v2.6.2 and below, build targets were referenced as `nrf9160dk_nrf9160` and `nrf9160dk_nrf9160_ns` (hardware model v1). Starting from nRF Connect SDK v2.7.0, hardware model v2 is used with `/` instead of `_`. **In nRF Connect SDK v3.0.0, hardware model v1 was completely removed**, so all boards must now use the new naming convention. For example: `nrf5340dk/nrf5340/cpuapp/ns` instead of `nrf5340dk_nrf5340_cpuapp_ns`. Hardware support and custom boards are covered in-depth in the nRF Connect SDK Intermediate course – Lesson 3.

---

## Child and parent images (removed in v3.0.0)

> **Important**: The child/parent image build system has been **completely removed** in nRF Connect SDK v3.0.0. If you have projects using this legacy system, you must migrate to Sysbuild. See the [Migration from multi-image builds to Sysbuild](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/releases_and_maturity/migration/migration_sysbuild.html) documentation.

When using the older multi-image build system (nRF Connect SDK v2.8.0 and earlier), the build consisted of a parent image and one or more child images, where the child image was included by the parent image.

This build system was deprecated in nRF Connect SDK v2.8.0 and replaced by Sysbuild. **It was completely removed in v3.0.0**.

---

## Exercise 1: Creating an application

In this exercise, we will learn how to create a working “Hello World” application in the nRF Connect SDK from scratch. We are only doing this for educational purposes to expose you to all the elements of an nRF Connect SDK application.

Keep in mind that this is **not** the recommended way to create an application. As we explained in lesson 1, the recommended way is by using one of the samples that you can find in the SDK as a baseline.

### Exercise steps

1.  In the exercise folder for this lesson, create a folder and name it `l3_e1`.
2.  Open this folder and create the files needed for a minimum working application:
    *   `prj.conf` (leave empty for now)
    *   `CMakeLists.txt`
    *   Create a subdirectory `src`
    *   Inside `src`, create an empty file `main.c`
3.  Open `CMakeLists.txt` and add the minimum functions needed for the application to build:
    ```cmake
    cmake_minimum_required(VERSION 3.20.0)
    find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
    project(hello_world)
    target_sources(app PRIVATE src/main.c)
    ```
    *   `cmake_minimum_required`: ensures the build fails if the CMake version is too old.
    *   `find_package`: pulls in the Zephyr build systems, which creates a CMake target named `app`.
    *   `project`: sets the name of the project.
    *   `target_sources`: adds the source file to the project.

4.  In `src/main.c`, include the header file for Zephyr Kernel and the printk module:
    ```c
    #include <zephyr/kernel.h>
    #include <zephyr/sys/printk.h>
    ```
5.  In `src/main.c`, define the `main` function to continuously print the statement “Hello World!” and then sleep for a small amount of time (to avoid flooding the console).
    ```c
    int main(void)
    {
        while (1) {
            printk("Hello World!\n");
            k_msleep(1000); // Sleep for 1 second
        }
        return 0; // Should not be reached
    }
    ```
    > **Note**: `prj.conf` is left empty because the console drivers needed by `printk()` are enabled by default by the board configuration file. This is covered in-depth in Lesson 4.

6.  Add this as an existing application in nRF Connect for VS Code. Go to nRF Connect for VS Code and under the **Welcome View**, select **Open an existing application** and select the directory `l3_e1`. Observe the project showing up under the **Applications View**.
7.  Just like in Lesson 1, select **Add Build Configuration** by hovering your mouse over the project name in the Applications View and selecting the icon that appears. Choose the correct board, then select **Build Configuration**.
8.  Flash the application to your board.
    > **Note**: In nRF Connect SDK v3.0.0+, the `west flash` command uses **nRFUtil** instead of nrfjprog. This provides more consistent behavior across all platforms. Make sure you have nRFUtil installed (it comes with the nRF Connect SDK installation).
9.  To view the output of the application, configure a terminal emulator. We will show how to use the built-in **nRF Terminal** in VS Code or the **Serial Terminal** application in nRF Connect for Desktop.

    Two important parameters for UART console:
    *   **Speed (baud rate):** Default for Nordic DKs is 115200 bps.
    *   **Serial line (COM port):** Varies by machine and board. Boards might have multiple Virtual COM (VCOM) ports (e.g., VCOM0, VCOM1).

    > **Note: Which Virtual COM (VCOM) port to choose?**
    > Nordic DKs have an interface MCU providing UART-to-USB and VCOM ports. Different VCOMs might be for specific uses (e.g., TF-M logs vs. application logs). If you don't see expected output on one VCOM port, try another and reset the board.

    #### Using nRF Terminal (VS Code)
    1. Go to the **Connected Devices** tab and expand your connected device.
    2. Click the **Connect To Serial Port In nRF Terminal** icon.
    3. Choose the serial port.
    4. Set the configuration:
        * Baud rate: 115200
        * 8 data bits, no parity, 1 stop bit (8N1)
        * No flow control (RTS/CTS off)
    5. Observe the output:
       ```
       *** Booting Zephyr OS build zephyr-v3.x.x ***
       Hello World!
       Hello World!
       ...
       ```

    #### Using Serial Terminal (nRF Connect for Desktop)
    1. Open the **Serial Terminal** app in nRF Connect for Desktop.
    2. Select the correct COM port for your board from the dropdown list.
    3. Set the baud rate to 115200.
    4. Ensure other settings match (8N1, no flow control).
    5. Click **Open**.
    6. You might need to press the **Reset** button on your DK to see the initial boot messages and output.
    7. Observe the output similar to the nRF Terminal example above.


The solution for this exercise can be found in the GitHub repository, `l3/l3_e1_sol` (select your version subfolder).

---

## Exercise 2: Customizing the application

During application development, it can be useful to customize aspects of the nRF Connect SDK for your specific application, without modifying the SDK files themselves. In this exercise, we will customize our application by adding custom files, configurations, and modifying the devicetree.

### Exercise steps

Use the previous exercise (`l3_e1`) as your starting point. Copy the completed exercise folder to a new directory and name it `l3_e2`. Open this new folder in VS Code.

#### Adding custom files

1.  Create a `.c` file and a `.h` file in the `src/` directory, alongside `main.c`. Call them `myfunction.c` and `myfunction.h`.
2.  Define a function `sum()` in `myfunction.c` and declare it in `myfunction.h`.
    *   `myfunction.c`:
        ```c
        #include "myfunction.h"

        int sum(int a, int b){
            return a+b;
        }
        ```
    *   `myfunction.h`:
        ```c
        #ifndef MY_FUNCTION_H
        #define MY_FUNCTION_H

        int sum(int a, int b);

        #endif // MY_FUNCTION_H
        ```
        > **Definition: Include guards**
        > A construct (usually preprocessor macros) used to prevent problems caused by including the same header file multiple times within a single translation unit.

3.  **Include the custom source file in the build.** Add the following line to `CMakeLists.txt`:
    ```cmake
    target_sources(app PRIVATE src/myfunction.c)
    ```
4.  **Include the header file in `main.c`**. Add this line at the top of `src/main.c`:
    ```c
    #include "myfunction.h"
    ```
5.  **Replace `main()` to run the custom function.** Modify the `main` function in `src/main.c`:
    ```c
    int main(void)
    {
        int a = 3, b = 4;
        while(1){
            printk("The sum of %d and %d is %d\n", a, b, sum(a,b));
            k_msleep(1000);
        }
        return 0; // Should not be reached
    }
    ```
6.  Build and flash the application to your board. Open a terminal output and you should see:
    ```
    *** Booting nRF Connect SDK ... ***
    *** Using Zephyr OS ... ***
    The sum of 3 and 4 is 7
    The sum of 3 and 4 is 7
    ...
    ```
    The issue with including source code directly via `target_sources()` is that the file is always included, even if the function isn't used. Next, we'll control this using Kconfig.

#### Adding custom configurations

6.  **Define a custom Kconfig symbol.** Create a file named `Kconfig` (no extension) in the application root directory (`l3_e2/`). Add the following content:
    ```kconfig
    # SPDX-License-Identifier: Apache-2.0

    source "Kconfig.zephyr"

    mainmenu "Application Configuration"

    config MYFUNCTION
        bool "Enable my function"
        default n
        help
          Enable the custom sum function defined in myfunction.c/h.
    ```
    *   `source "Kconfig.zephyr"`: Necessary to include base Zephyr configurations.
    *   `config MYFUNCTION`: Defines the symbol `CONFIG_MYFUNCTION`.
    *   `bool`: Specifies it's a boolean type.
    *   `"Enable my function"`: The prompt displayed in configuration interfaces.
    *   `default n`: Sets the default value to no (disabled).
    *   `help`: Provides descriptive text.

    (See Kconfig documentation for more on creating menus).

7.  **Make the addition of the custom source file conditional.** In `CMakeLists.txt`, comment out or remove the `target_sources` line from step 3 and add the following line instead, using `target_sources_ifdef()`:
    ```cmake
    # target_sources(app PRIVATE src/myfunction.c) # Commented out
    target_sources_ifdef(CONFIG_MYFUNCTION app PRIVATE src/myfunction.c)
    ```
    The build will now only include `myfunction.c` if `CONFIG_MYFUNCTION` is enabled (`=y`).

    > **Note**
    > This strategy is used extensively in nRF Connect SDK to only include the source code of libraries that you plan to use, limiting application size. Modules are included only when enabled via Kconfig.

8.  **Enable the custom Kconfig symbol.** Add the following line to `prj.conf`:
    ```kconfig
    CONFIG_MYFUNCTION=y
    ```
9.  **Update `main.c` to check the Kconfig symbol.**
    9.1 Conditionally include the header file:
    ```c
    #ifdef CONFIG_MYFUNCTION
    #include "myfunction.h"
    #endif
    ```
    9.2 Conditionally call the function within `main()`:
    ```c
    int main(void)
    {
        while (1) {
    #ifdef CONFIG_MYFUNCTION
            int a = 3, b = 4;
            printk("The sum of %d and %d is %d\n", a, b, sum(a, b));
    #else
            printk("MYFUNCTION not enabled\n");
            // Removed return 0 here to keep the loop running
            // You might want different behavior if disabled
    #endif
            k_msleep(1000);
        }
        return 0; // Should not be reached
    }
    ```
10. Build and flash the application. You should see the "The sum..." output again.
11. **Try disabling the Kconfig symbol.**
    11.1 In `prj.conf`, change the line to `CONFIG_MYFUNCTION=n`.
    11.2 **Perform a Pristine Build** (required for Kconfig changes affecting CMake to take effect reliably). In VS Code Actions view, click the "Pristine Build" icon next to the Build action, or use the Command Palette (`Ctrl+Shift+P`) and search for `Pristine Build`. Flash the application.
         > **Definition: Pristine build**
         > Creates a new build directory. All byproducts from previous builds have been removed.
    11.3 You should now see the output:
         ```
         *** Booting nRF Connect SDK ... ***
         *** Using Zephyr OS ... ***
         MYFUNCTION not enabled
         MYFUNCTION not enabled
         ...
         ```
    11.4 **Re-enable** the symbol in `prj.conf` (`CONFIG_MYFUNCTION=y`) for the next section. Remember to do another build (pristine might not be strictly necessary here, but safe) and flash.

#### Modifying the devicetree

In this section, we will modify the devicetree for this specific application using an overlay file to change the UART console baud rate.

12. **Create an overlay file.**
    *   In the application root directory (`l3_e2/`), create a subdirectory named `boards`.
    *   Inside the `boards` directory, create an overlay file named `<board_target>.overlay`.
        *   **For nRF Connect SDK v3.0+** (Hardware Model v2): Board targets use `/` as separator. For the nRF52833 DK, this would be `nrf52833dk_nrf52833.overlay`. For multi-core boards like nRF5340 DK, use `nrf5340dk_nrf5340_cpuapp.overlay` (note: the file name still uses underscores, but the build target uses slashes like `nrf5340dk/nrf5340/cpuapp`).
        *   **Important**: Use the actual **board target** string used in the build configuration (e.g., `nrf5340dk_nrf5340_cpuapp_ns.overlay` for nRF5340 DK with TF-M), not just the marketing name.
    *   Alternatively, use the VS Code nRF Connect extension GUI: Select the application context, expand `Config files -> Devicetree`, and click `+ Create overlay file`. This helps ensure the correct name.

13. **Change the baud rate for the UART node.** Add the following to your `.overlay` file to change the `current-speed` property of the default UART console peripheral (`uart0` for most DKs, check your board's DTS if unsure, e.g., `uart20` for nRF54L15).
    ```devicetree
    /* Overlay for changing UART0 baud rate */
    &uart0 {
        current-speed = <9600>; /* Change baud rate to 9600 */
    };
    ```
    *(Note: If using nRF54L15 DK, replace `&uart0` with `&uart20` or the correct UART instance)*.
    Common things changed via overlays include peripheral `status` (`okay`/`disabled`) and pin assignments (`pinctrl-*`). See [Set devicetree overlays](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/build/dts/howtos.html#set-devicetree-overlays) for more methods.

14. **Do a pristine build** (essential for devicetree changes) and flash the application.
    *   Use the Pristine Build icon in VS Code Actions or the Command Palette.
    *   To confirm the change, view the compiled devicetree output: Use the Command Palette (`Ctrl+Shift+P`) and run `nRF DeviceTree: Open Compiled Output`. Search for `uart0` (or your UART instance) and verify the `current-speed` property is `9600`. The compiled file is usually located at `<app_dir>/build/<build_dir_name>/zephyr/zephyr.dts`.
    *   You can also use the **DeviceTree Visual Editor** (accessible from Actions View or Command Palette) after a build. Navigate to the `uart` node and inspect its properties.
        > **Note**: Ensure the `nRF DeviceTree` extension (part of the nRF Connect Extension Pack) is installed to use the visual editor.

15. Observe that the serial terminal likely shows **garbled output or no output**. This is because the application is now transmitting at 9600 baud, while the terminal is likely still configured for 115200 baud.

16. **Change the baud rate in the serial terminal** to match the application (9600).
    *   **nRF Terminal (VS Code):**
        1. Close the existing terminal connection.
        2. Reconnect to the serial port.
        3. When prompted for settings (or click the cogwheel icon), select `[Other]` for baud rate and enter `9600`.
        4. Keep other settings (8N1, no flow control).
        5. Observe the correct "The sum..." log output.
    *   **Serial Terminal (nRF Connect for Desktop):**
        1. Close the connection if open.
        2. Change the baud rate setting in the app to `9600`.
        3. Open the connection.
        4. Reset the board if needed.
        5. Observe the correct log output.

17. **Devicetree Visual Editor (further reading).**
    Once you have an overlay and a compiled build, the DeviceTree Visual Editor provides a GUI to explore and modify the devicetree structure. You can view node properties and edit them directly, which automatically updates the corresponding `.overlay` file. It's a useful tool for learning Devicetree syntax as changes in the GUI reflect in the text file and vice-versa. Access it from the Actions view or Command Palette.

    *(Description of original image showing visual editor modifying UART properties)*

The solution for this exercise can be found in the GitHub repository, `l3/l3_e2_sol` (select your version subfolder).

---