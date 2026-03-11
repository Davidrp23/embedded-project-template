# nRF Connect SDK Introduction

## Overview
The nRF Connect SDK is a software development kit for building low-power wireless applications based on Nordic Semiconductor’s nRF54, nRF53, nRF52, nRF70, or nRF91 Series devices. We will take a look under the hood of the SDK to have a better understanding of its structure and content.

In the exercise section of this lesson, we will cover all the steps needed to download and install the nRF Connect SDK with the Visual Studio Code Integrated Development Environment (VS Code IDE). Then, we will test the installation by building and flashing the sample program blinky, which toggles an LED on your Nordic Semiconductor board. Finally, we will cover how to modify the blinky source code and rebuild the program to make the LED toggle at a different frequency.

The exercise is divided into two parts:

Exercise 1 – Installing nRF Connect SDK and VS Code.
Exercise 2 – Building your first nRF Connect SDK application.

## nRF Connect SDK structure and content

nRF Connect SDK is a scalable and unified software development kit for building low-power wireless applications based on the Nordic Semiconductor nRF54, nRF53, nRF52, nRF70, or nRF91 Series wireless devices. It offers an extensible framework for building size-optimized software for memory-constrained devices as well as powerful and complex software for more advanced devices and applications.

It integrates the Zephyr Real-Time Operating System (RTOS) and a wide range of complete applications, samples, and protocol stacks such as Bluetooth Low Energy, Bluetooth mesh, Wi-Fi, Matter, Thread/Zigbee and LTE-M/NB-IoT/GPS, TCP/IP. It also includes middleware such as CoAP, MQTT, LwM2M, various libraries, hardware drivers, Trusted Firmware-M for security, and a secure bootloader (MCUBoot).

*(Diagram described in original text: Architecture of the nRF Connect SDK)*

Zephyr RTOS and third-party components (e.g. MCUBoot, Trusted Firmware-M) are shown in dark blue in the above diagram description.

**Zephyr RTOS** is an open-source real-time operating system for connected and resource-constrained embedded devices. It includes a scheduler that ensures predictable/deterministic execution patterns and abstracts out the timing requirements. It also comes with a rich set of fundamental libraries and middleware that simplifies development and helps reduce a product’s time to market. Zephyr RTOS is highly configurable and enables scalable configurations from very small configurations for memory-constrained devices (minimum 8 kilobytes, for example, simple LED blinking application) to powerful, feature-rich, high-processing power devices (multiple MBs of memory) with large memory configurations.

The nRF Connect SDK offers a single code base for all of Nordic’s devices and software components. It simplifies porting modules, libraries, and drivers from one application to another, thus reducing development time. By enabling developers to pick and choose the essential software components for their application, high memory efficiency is guaranteed. The nRF Connect SDK is publicly hosted on GitHub.

Internally, the nRF Connect SDK code is organized into four main repositories:

*   `nrf` – Applications, samples, connectivity protocols (Nordic)
*   `nrfxlib` – Common libraries and stacks (Nordic)
*   `Zephyr` – RTOS & Board configurations (open source)
*   `MCUBoot` – Secure Bootloader (open source)

In addition to the four main repositories, there are also other repositories like the `TrustedFirmware-M` repository and the `Matter` repository.

The figure below (description from original text) visualizes the toolchain in the nRF Connect SDK, which is based on the Zephyr toolchain. You can see a set of different tools, each playing a role in the creation of an application, from configuring the system to building it.

*(Diagram described in original text: nRF Connect SDK tools and configuration methods)*

**Kconfig** (covered in Lesson 3) generates definitions that configure the whole system, for example, which wireless protocol or which libraries to include in your application. **Devicetree** (covered in Lesson 2) describes the hardware. **CMake** then uses the information from Kconfig and the devicetree to generate build files, which **Ninja** (comparable to make) will use to build the program. The **GCC** compiler system is used to create the executables.

This high decoupling of the source code (`*.c`) and the configuration system through Kconfig (`*.conf`) and the hardware description system through devicetree (`*.dts`) makes it easy to use the same application source code on different hardware and with different configurations with minimal changes. This has a huge impact on the ease of portability and maintainability.

*(Diagram described in original text: Build process for nRF Connect SDK)*

> **Note on Hardware Model v2 (HWMv2)**
> Starting with nRF Connect SDK v3.0.0, the SDK exclusively uses Hardware Model v2 (HWMv2). The previous Hardware Model v1 (HWMv1), which was deprecated in v2.7.0, has been completely removed. HWMv2 offers improved support for modern hardware architectures, including multi-core and multi-SoC systems. Key improvements include:
> - **Vendor-Based Organization**: Boards are organized under their respective vendors rather than architectures.
> - **YAML-Based Descriptions**: Hardware components are defined using YAML files for better readability.
> - **Improved Multi-Target Support**: Better handling of boards with multiple targets (e.g., separate application and network cores).
> - **New Board Target Format**: Board targets now use forward slashes instead of underscores (e.g., `nrf5340dk/nrf5340/cpuapp` instead of `nrf5340dk_nrf5340_cpuapp`).

Now that we have an understanding of the content and structure of the nRF Connect SDK, let's move to the hands-on part of this lesson where we install the nRF Connect SDK through some high-level tools that simplify the process of obtaining the repositories and setting up the needed toolchains.

---

## Exercise 1: Installing nRF Connect SDK and VS Code

*(The following instructions correspond to nRF Connect SDK v3.0.0 - v3.2.0.)*

Follow the steps below to install the nRF Connect SDK, its toolchain, and VS Code IDE.

> **Important**
> Starting with nRF Connect SDK v3.1.0, the SDK, its toolchain, and nrfutil are bundled together in a single "Pre-packaged SDKs & Toolchains" package. This makes installation significantly simpler. If you are using v3.1.0 or later, you can skip steps 1-3 below and proceed directly to step 4 (Install VS Code), as nrfutil and the device command will be included in the toolchain bundle.

For nRF Connect SDK v3.0.x or if you need to install flashing tools separately, follow steps 1-3 below.

1.  **Install SEGGER J-Link**
    Download the latest installer for your platform from [SEGGER J-Link Software](https://www.segger.com/downloads/jlink/). Run the installer; when you reach the 'Choose Optional Components' window during installation, be sure to select 'Install Legacy USB Driver for J-Link'. This driver is necessary for some supported Development Kits.

2.  **Install nrfutil and the nrfutil device command**

    > **Note**
    > Starting with nRF Connect SDK v3.0.0, the `west flash` command uses nrfutil instead of nrfjprog by default. The nRF Command Line Tools have been deprecated and replaced by nRF Util. For v3.1.0+, nrfutil is included in the toolchain bundle.

    2.1 Download the binary compatible with your OS from the [nRF Util product page](https://www.nordicsemi.com/Products/Development-tools/nrf-util) and store it somewhere on your disk drive (For example `C:\nordic_tools\nrfutil.exe` for Windows).
    If you are running on macOS or Linux, you can store it in a folder that is already added in the system's PATH (E.g. `/usr/bin/`), so that you can skip step 2.2.
    > **Note**
    > For Linux, keep in mind that nrfutil has some prerequisites that are listed in [Installing nRF Util prerequisites](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/installation/install_nrf_util.html). Make sure you also download them, if you don't have them on your machine already.
    2.2 (Windows) Update your system's PATH to include the location where nrfutil is stored. Open `Edit environment variable for your account` and add the path where you stored the nrfutil binary.
    2.3 The `nrfutil` binary you just downloaded does not come with any pre-installed commands. In this step, we will upgrade the core `nrfutil` and download the `device` command.
    2.3.1 To make sure we have the latest nrfutil version, run the following command in a terminal (Command Prompt or PowerShell). It doesn't matter which terminal since nrfutil is set globally in step 2.2.
    ```bash
    nrfutil self-upgrade
    ```
    2.3.2 Install the `nrfutil device` command. For this course, we will need the `device` command to flash binaries to development kits. In your active terminal, type:
    ```bash
    nrfutil install device
    ```
    You should see output similar to:
    ```
    [00:00:02] ###### 100% [Install packages] Install packages
    ```

3.  **(Optional) Install nRF Command Line Tools.**
    > **Note**
    > The nRF Command Line Tools have been deprecated in nRF Connect SDK v3.0.0 and replaced by nRF Util. You only need to install this if you require backward compatibility with older projects or need to use nrfjprog directly.

    Download link: [https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools/Download?lang=en#infotabs](https://www.nordicsemi.com/Products/Development-tools/nRF-Command-Line-Tools/Download?lang=en#infotabs).

4.  **Install VS Code.**
    Go to [https://code.visualstudio.com/download](https://code.visualstudio.com/download) and install the version that matches your operating system.

5.  **Install nRF Connect Extension Pack.**
    In the Activity Bar in VS Code, click the Extensions icon, then type `nRF Connect for VS Code Extension Pack` in the search field, and click on `Install`.

    **nRF Connect for VS Code extension pack** allows developers to use the popular Visual Studio Code Integrated Development Environment (VS Code IDE) to develop, build, debug, and deploy embedded applications based on Nordic’s nRF Connect SDK (Software Development Kit). It includes an interface to the compiler, linker, the whole build system, an RTOS-aware debugger, a seamless interface to the nRF Connect SDK, the Devicetree Visual Editor, and an integrated serial terminal, among other valuable development tools.

    nRF Connect for VS Code extension pack consists of the following components:
    *   **nRF Connect for VS Code:** The main extension contains an interface to the build system and nRF Connect SDK. It also provides an interface to manage nRF Connect SDK versions and toolchains.
    *   **nRF DeviceTree:** Provides Devicetree language support and the Devicetree Visual Editor.
    *   **nRF Kconfig:** Provides Kconfig language support.
    *   **nRF Terminal:** A serial and RTT terminal.
    *   **C/C++ from Microsoft:** Adds language support for C/C++, including features such as IntelliSense.
    *   **CMake:** CMake language support.
    *   **GNU Linker Map Files:** Linker map files support.

    We can download any nRF Connect SDK version of our preference and its toolchain through the extension. Complete documentation of the nRF Connect for VS Code is available [here](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/installation/install_ncs.html).

    > **Note**
    > It's crucial to install the `nRF Connect for VS Code Extension Pack`, not just `nRF Connect for VS Code`, in order to get all the available features.

6.  **Install SDK and Toolchain.**
    The toolchain is a set of tools that are used together to build nRF Connect SDK applications. It includes the assembler, compiler, linker, CMake, and (starting with v3.1.0) nrfutil, among other tools.

    The first time you open nRF Connect for VS Code, it will prompt you to install a toolchain. This usually happens when the extension does not find any installed toolchain on your machine.

    **For nRF Connect SDK v3.1.0 and later (Pre-packaged installation):**

    Starting with v3.1.0, the SDK and toolchain are bundled together in a single "Pre-packaged SDKs & Toolchains" package:

    1. In the nRF Connect for VS Code extension, click on `Manage SDKs`.
    2. Click on `Install SDK`. You will see "Pre-packaged SDKs & Toolchains" listed.
    3. Select the version you want to install (e.g., v3.1.0 or v3.2.0). This will download and install both the SDK and its matching toolchain together.

    > **Note**
    > The pre-packaged installation includes nrfutil and the device command, so you do not need to install them separately.

    **For nRF Connect SDK v3.0.x (Separate installation):**

    If you need to install v3.0.x, the SDK and toolchain are installed separately:

    1. Click on `Install Toolchain`. Select the toolchain version that matches the nRF Connect SDK version you plan to use (e.g., toolchain v3.0.0 for SDK v3.0.0).
    2. After the toolchain is installed, click on `Manage SDKs`, then `Install SDK`. Select the SDK version you wish to use.

    Installing will take some time, depending on your development platform specifications and internet speed.

    > **Note**
    > By default, nRF Connect for VS Code displays only released tags (also known as stable versions). If you are evaluating new features and want to use a preview tag or a different type of tag (for example, customer sampling -cs), click on "Show All Toolchain Versions" or "Show All SDK Versions." Note that for production code, only released tags should be used.

    > **Note**
    > If you have opened the SDK folder in VS Code, the `Manage SDKs` menu option will not be present, and you will instead see `Manage west workspace`. To fix this, open another window or folder in VS Code. If you don't see either option, make sure you have the latest version of the nRF Connect for VS Code extension pack.

    > **Important**
    > We recommend installing the latest released tag of nRF Connect SDK (currently v3.1.0 or v3.2.0) to get all the latest features and patches.

    With this, we have completed the installation of nRF Connect SDK and VS Code.

    > **Note**: It's also worth noting that nRF Connect SDK is IDE agnostic, which means you can use it with the IDE of your choice or without an IDE. It is possible to download and install the nRF Connect SDK using a command-line interface (CLI) through nRF Util (`nrfutil`). However, we highly recommend using VS Code with our nRF Connect for VS Code extension pack as it contains both a convenient graphical user interface (GUI) and an efficient command-line interface (CLI) in one place, in addition to many features that make firmware development much easier. Setting up another IDE to work with the nRF Connect SDK will require some extra manual steps that are out of the scope of this course.

---

## Exercise 2: Build and flash your first nRF Connect SDK application

*(The following instructions correspond to nRF Connect SDK v3.0.0 - v3.2.0.)*

In this exercise, we will program a simple application based on the `blinky` sample to toggle an LED on our board. Any supported Nordic Semiconductor development board will work (nRF54, nRF53, nRF52, nRF70, or nRF91 Series). The idea is to make sure that all the tools needed to build and flash a sample are set up correctly. The focus is to learn how to create an application from a template (“Copy a sample”), build the application, and flash it on a Nordic-powered board.

Basically, we will walk you through the three bullet points below:

*   Create a new application based on a template(sample).
*   Build an application.
*   Flash an application to a board.

### Exercise steps

1.  Create a folder close to your root directory that will hold all the exercises that we will be working on throughout this course.
    Example: `C:\myfw\ncsfund`. Avoid storing your applications in locations with long paths, as the build system might fail on some operating systems (Windows) if the application path is too long. Also, avoid using whitespaces and special characters in the path.

2.  In VS Code, click on the **nRF Connect Extension icon**. In the **WELCOME View**, click on **Create a new application**.

3.  In the **Create new application** dialog, select **Copy a sample**.
    You will be presented with three options:
    *   `Create a blank application`: Creates a blank application with an empty `main()` function.
    *   `Copy a sample`: Presents all the templates (“samples”) that come from the different modules in nRF Connect SDK and enables you to create an application based on a template. (Note: If you have multiple SDK versions installed, you will be prompted to select which SDK version to copy a sample from.)
    *   `Browse application index`: (Out of scope for this course) Copies a revision of an existing out-of-tree application compatible with the nRF Connect SDK from the online application index and sets up a west workspace repository around it.

    > **Note**
    > The SDK contains a rich set of templates. In the context of templates:
    > *   **Samples** are simple and showcase a single feature or library (e.g., SHA256 sample, UART sample, PWM sample, blinky sample, etc.).
    > *   **Applications** are complex and include a variety of libraries to implement a specific use case (e.g., Asset tracker application, Keyboard application, Mouse application).

    3.1 Type `blinky` in the search field and select the **Blinky Sample**. The Blinky sample blinks LED1 on your board indefinitely. We will base our first application on the Blinky sample. The Blinky sample comes from the Zephyr module in nRF Connect SDK, hence the sample path: `zephyr\samples\basic\blinky`.
    (Icons mentioned in original text allow access to the in-tree sample, online documentation, and GitHub repository respectively, and filtering options).

    3.2 Input the full path for where you want the application to be stored.
    1.  Select where you want to store your application (e.g., `C:\myfw\ncsfund\`).
    2.  Name your application `l1_e2`. This is the naming convention we will use for exercises throughout this course. This will create a folder for your application called `l1_e2`.
    Press Enter when you have input the full path.

    VS Code will ask if you want to open the application in the same VS Code instance or open a new VS Code instance. Select **Open** to open the application in the same VS Code instance.

    This will make a copy of the Blinky sample template, store it in the specified application directory, and add an unbuilt application to VS Code.

    > **Note**
    > nRF Connect for VS Code might show some error squiggles (e.g., header files not found) for an unbuilt application, often appearing in the PROBLEMS tab. Please ignore these error squiggles at this stage. These errors will automatically be resolved once you add and build a configuration (next step).

4.  **Add a build configuration.**
    This step specifies the hardware (development board or custom board) and software configuration (*.conf and possible devicetree overlays) for the build. What you set here controls the parameters passed to the underlying command-line tool, `west`.

    In the **APPLICATIONS view**, under the application name (`l1_e2`), click on **Add Build Configuration**. This opens the **Add Build Configuration** window.

    The GUI will inform you about the SDK/toolchain version being used. Ensure it matches the SDK/toolchain you intend to use and the SDK version from which you copied the sample. If not, click the arrow to select the correct version.

    4.1. Using the **Board target**, choose the board you want to flash your application to. See the table below for an overview of board targets for Nordic devices.

    | Device          | Board target                   |
    | :-------------- | :----------------------------- |
    | nRF54L15 DK*    | `nrf54l15dk/nrf54l15/cpuapp/ns`  |
    | nRF5340 DK      | `nrf5340dk/nrf5340/cpuapp/ns`  |
    | nRF52840 DK     | `nrf52840dk/nrf52840`          |
    | nRF52833 DK     | `nrf52833dk/nrf52833`          |
    | nRF52 DK        | `nrf52dk/nrf52832`             |
    | nRF9160 DK      | `nrf9160dk/nrf9160/ns`         |
    | nRF9161 DK*     | `nrf9161dk/nrf9161/ns`         |
    | nRF9151 DK*     | `nrf9151dk/nrf9151/ns`         |
    | Thingy:91 X     | `thingy91x/nrf9151/ns`         |
    | Thingy:91       | `thingy91/nrf9160/ns`          |
    | Thingy:53       | `thingy53/nrf5340/cpuapp/ns`   |
    | nRF7002 DK*     | `nrf7002dk/nrf5340/cpuapp/ns`  |

    *\*Board support added in specific SDK versions as noted in the original text.*

    > **Important**
    > The nRF54L15 PDK v0.8.1 is functionally equivalent to a DK. Use the board targets: `nrf54l15dk/nrf54l15/cpuapp/ns` (with TF-M) or `nrf54l15dk/nrf54l15/cpuapp` (without TF-M).

    (Example from original text: Choosing nRF52833 DK by specifying `nrf52833dk/nrf52833`).

    4.2. The **Board Revision** is usually on a sticker (format X.X.X). This field is optional and only needed if the board has multiple revisions (like nRF9160 DK). Leaving it blank uses the board definition's default.

    We will keep the default build configuration for the remaining options. However, understanding their roles is important:

    4.3. **Base configuration files:** Lists base application configuration files (e.g., `prj.conf`). Some templates might offer variants (e.g., `prj_minimal.conf`). Blinky has only `prj.conf`. Equivalent to `FILE_SUFFIX` (and `CONF_FILE`) in `west`.

    4.4. **Extra Kconfig fragments:** Lists additional Kconfig fragments (modifiers to the base configuration) found in the template or added to the application folder. Blinky has none. Equivalent to `EXTRA_CONF_FILE` in `west`.

    4.5. **Base Devicetree overlays:** Lists Devicetree overlays (hardware description modifiers) found in the template or added to the application folder. Blinky has none. Equivalent to `DTC_OVERLAY_FILE` in `west`.

    4.6. **Extra Devicetree overlays:** Provides additional custom devicetree overlay files to be mixed in. Equivalent to `EXTRA_DTC_OVERLAY_FILE` in `west`.

    4.7. **Snippets:** Bundles combining software (Kconfig) and hardware (Devicetree) configuration. Several exist in the SDK.

    4.8. **Extra CMake arguments:** Allows passing arguments to the build system. Leave blank for this demo.

    4.9. **Build directory name:** Option to manually name the build output directory. Default is usually `build` or `build_n`.

    4.10. **Optimization level:**
    *   `Use project default`: Uses settings from `prj.conf`.
    *   `Optimize for debugging (-Og)`: Important for debugging. Passes `CONFIG_DEBUG_THREAD_INFO="y"` and `CONFIG_DEBUG_OPTIMIZATIONS="y"`.
    *   `Optimize for speed(-O2)`: Passes `CONFIG_SPEED_OPTIMIZATIONS="y"`.
    *   `Optimize for size (-Os)`: Passes `CONFIG_SIZE_OPTIMIZATIONS="y"`.
    Leave as `Use project default` for this exercise. Remember to use `Optimize for debugging (-Og)` for debugging sessions.

    Leave the **Build after generating configuration** option enabled.

    4.11. **Sysbuild:** Enabled by default and required for multi-image applications. In nRF Connect SDK v3.0.0, the previous multi-image builds (parent-child images) system was removed and fully replaced by Sysbuild. Leave this enabled for most applications.

    4.12. Click the **Build Configuration** button.

    The build process will start. Open the **nRF Connect Terminal** (`View` -> `Terminal`) to see the progress. A successful build displays the application’s memory usage.

    > **More on this:**
    > A key component is `west`, a core command-line utility. It handles repository management, building, flashing, etc., and is invoked by the VS Code extension. You can use `west` commands manually in the terminal (use the `nRF Connect Terminal` where the toolchain is sourced). Type `west help` for details or check the `west` documentation.

    After building, a `build` sub-directory appears in your application folder, containing build outputs, including the binary file for flashing.

5.  Make sure your development kit is connected to your computer and switched on. It should be listed in the **Connected Devices View** in the nRF Connect for VS Code extension.

    > **Important**
    > If using a Thingy:53, Thingy:91 X, or Thingy:91 (no onboard debugger), the flash procedure differs, and the device won't appear in the Connected Devices View. Skip steps 5 and 6 and follow the specific flashing instructions for [Thingy:53](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/device_guides/working_with_nrf/nrf53/thingy53_gs.html) or [Thingy:91/Thingy:91 X](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/device_guides/working_with_nrf/nrf91/thingy91_gsg.html).

    If your board isn't listed, press the **Refresh Connected Devices** icon in the Connected Devices View.

6.  From the **Actions View**, click **Flash** to flash the application to the board. Monitor the progress in the Terminal Panel.

    > **Note**
    > Starting with nRF Connect SDK v3.0.0, the `west flash` command uses nrfutil instead of nrfjprog by default.

    > **Note**
    > `Flash` writes the application. `Erase And Flash To Board` erases the entire device first, including any saved application data (e.g., mesh network provisioning). Note that in v3.0.0+, the erase behavior is now consistent: it always erases the complete chip regardless of which utility is used.

    LED1 on your board should now blink at one-second intervals.

    > **Note**
    > On the nRF54L15 DK, board LEDs are labeled starting from 0 (LED0-LED3). Previous DKs start from 1 (LED1-LED4). Running this sample on nRF54L15 DK will blink LED0.

7.  Let’s change the LED blink rate for demonstration.
    Locate `main.c` (in `Source Files->Applications` or via Explorer). On line 11 (or similar), change `SLEEP_TIME_MS` from `1000` to `100`.
    ```c
    #define SLEEP_TIME_MS   100
    ```

8.  Rebuild (click the build icon/button again for the existing configuration) and re-flash the application. Observe the LED blinking at a higher frequency.

---