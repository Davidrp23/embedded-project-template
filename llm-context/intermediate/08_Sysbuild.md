# Sysbuild

## Overview

Sysbuild is a high-level build system that simplifies the management of complex multi-image builds. It is an improved and extensible build system for multi-image builds, and **starting with nRF Connect SDK v3.0, Sysbuild is the ONLY supported system for multi-image builds**. The legacy nRF Connect SDK-specific Multi-image builds (parent-child image) system has been completely removed.

Sysbuild became available in nRF Connect SDK version 2.7.0, was enabled by default from version 2.8.0, and is now **mandatory and the sole multi-image build system** in nRF Connect SDK v3.0.0 and later.

In this lesson, we will provide an overview of Sysbuild, highlighting its use cases and the default images provided by the SDK. We will explore the situations where Sysbuild is essential and learn how to configure it within an application. This will include setting Sysbuild-specific Kconfig options in the relevant files. We will explain the structure and purpose of key configuration files, as well as the directory used for image overlays. Furthermore, we will discuss memory partitioning schemes in nRF Connect SDK and cover how to configure and manage partitions using the Partition Manager. Additionally, we will provide guidance on utilizing tools like the nRF Kconfig GUI in VS Code to assist with Sysbuild configuration.

In the hands-on portion of this lesson, we will practice both adding/configuring an SDK-provided Sysbuild image (MCUboot bootloader) and creating a custom image from scratch.

---

## Sysbuild explained

This lesson aims to explain Sysbuild, its use case, and how to use it. **Sysbuild is mandatory in nRF Connect SDK v3.0.0 and later** - the legacy parent-child multi-image build system has been removed entirely.

### Concepts

To quote [Sysbuild documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/build/sysbuild/index.html): "Sysbuild is a higher-level build system that can be used to combine multiple other build systems together. It is a higher-level layer that combines one or more Zephyr build systems and optional additional build systems into a hierarchical build system."

In other words, Sysbuild runs before the normal Zephyr build system and helps handle multiple build processes as a single, unified build.

**Key changes in nRF Connect SDK v3.0:**

*   **Sysbuild is mandatory** - The legacy parent-child multi-image build system has been completely removed.
*   **No migration path** - Projects using the old multi-image system must be converted to Sysbuild.
*   Clear separation of files/boundaries between images.
*   Ability to seamlessly configure images separately or combined.
*   Compatibility with upstream Zephyr & integration with HWMv2 (Hardware Model v2, covered in Lesson 3).

### Use cases

In the nRF Connect SDK, Sysbuild is primarily used for:

1.  **Multi-core Applications:** Building separate images for different cores on a single SoC (e.g., application core and network core on nRF5340). Sysbuild manages the configuration and build process for all cores within a single project structure, simplifying development.
    *(Diagram description: Sysbuild managing builds for App Core and Network Core)*
2.  **Bootloaders:** Building a bootloader (like MCUboot) and an application image for the same core, ensuring they are placed correctly in memory and potentially merging them into a single flashable hex file.
    *(Diagram description: Sysbuild managing builds for Bootloader and Application for a single core)*

> **Note**: Sysbuild is always active in nRF Connect SDK v3.0+, even for single-image builds, although its multi-image management features are not utilized in that case.

See ["Sysbuild in the nRF Connect SDK"](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/build_and_config_system/sysbuild/index.html) for comprehensive documentation.

### Why do I need to know about Sysbuild?

**In nRF Connect SDK v3.0+, Sysbuild knowledge is essential** because it is the only way to build multi-image applications. While creating custom images is less common, developers frequently need to **configure existing images** provided by the SDK via Sysbuild.

As of nRF Connect SDK v3.0.0, default images include:

*   **Bootloaders:**
    *   MCUboot
    *   Nordic Secure Immutable Bootloader (NSIB)
*   **Network Core Images (for nRF5340 SoC):**
    *   nRF5340: Empty firmware
    *   HCI IPC
    *   Bluetooth: Host for nRF RPC Bluetooth Low Energy
    *   IEEE 802.15.4 over RPMsg
    *   IPC radio firmware

These images often require project-specific configuration (e.g., disabling MCUboot logging for release builds). Understanding Sysbuild configuration is necessary for this.

Sysbuild Kconfig options (prefixed with `SB_CONFIG_`) can enable these default images. Configuration details are covered later.

### A high-level build system

Key rules for Sysbuild interaction:

*   Sysbuild configuration mainly affects Sysbuild itself.
*   Sysbuild can configure its child images (e.g., application, bootloader).
*   Images **cannot** configure Sysbuild or other images directly.

*(Diagram description: Arrows showing configuration flow: Sysbuild Config -> Sysbuild Features, Sysbuild Config -> App Config, Sysbuild Config -> Extra Image Config, Sysbuild Config -> Spawn New Images. No arrows originate from App or Extra Image.)*

### Documentation

*   [Sysbuild in the nRF Connect SDK (NCS Docs)](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/build_and_config_system/sysbuild/index.html)
*   [Configuring Sysbuild (NCS Docs)](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/build_and_config_system/sysbuild/sysbuild_configuring.html)
*   [Bootloader Image IDs (NCS Docs)](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/bootloaders_dfu/index.html)
*   [Sysbuild Samples (NCS)](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/samples/samples_sysbuild.html)
*   [Sysbuild Design (Zephyr Docs)](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/build/sysbuild/index.html)
*   [Migrating to Sysbuild (NCS Docs)](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/releases_and_maturity/migration/migration_guide_3.0.html)

---

## Sysbuild configuration

*(Information relevant for nRF Connect SDK v3.0.0 - v3.2.0)*

This section explains how to configure Sysbuild within an application project.

### Sysbuild Kconfig

Sysbuild uses Kconfig symbols, similar to application configuration (`prj.conf`), but with specific rules:

*   **Prefix:** Sysbuild Kconfig symbols start with `SB_CONFIG_` (e.g., `SB_CONFIG_BOOTLOADER_MCUBOOT`).
*   **File:** Sysbuild Kconfig options are set in a file named `sysbuild.conf` located in the main application's root directory.

*(Diagram description: Folder structure showing `sysbuild.conf` alongside `prj.conf` and `CMakeLists.txt`)*

A list of common Sysbuild Kconfig options is in the [Sysbuild Configuration Guide](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/build_and_config_system/sysbuild/sysbuild_configuring.html). You can also explore options using `west build -t sysbuild_menuconfig` or the nRF Kconfig GUI in VS Code (by selecting the "(sysbuild)" entry in the APPLICATIONS view).

*(Diagram description: VS Code showing how to select the sysbuild context and open its Kconfig GUI)*

### Sysbuild configuration folder

The main application's root folder acts as the "home" for Sysbuild configuration for the entire project.

**Sysbuild Configuration Files:**

| File Path                     | Explanation                                    | Project Equivalent (for comparison) |
| :---------------------------- | :--------------------------------------------- | :---------------------------------- |
| `<main_app>/sysbuild.conf`    | Sets Sysbuild Kconfig options                  | `prj.conf`                          |
| `<main_app>/Kconfig.sysbuild` | Defines custom Sysbuild Kconfig symbols        | `Kconfig`                           |
| `<main_app>/sysbuild.cmake`   | Manages CMake logic for Sysbuild               | `CMakeLists.txt`                    |
| `<main_app>/sysbuild/`        | Directory for *overlaying* child image configs | N/A                                 |

This lesson covers `sysbuild.conf`, `sysbuild.cmake`, and the `sysbuild/` directory.

See the `zephyr/samples/sysbuild/hello_world` sample for a practical example.

### Image names

Each build target within Sysbuild has an **image name**.
*   The main application image typically uses the application's folder name (e.g., `hello_world`, `l8_e1`).
*   Extra images have defined names (e.g., `mcuboot`, `hci_ipc`, `custom_image`).
You can see the image names as subdirectory names within the main `build/` folder after building.

*(Diagram description: Build folder structure showing `build/hello_world/` and `build/mcuboot/`)*

### Image overlays and Kconfig fragments

The `<main_app>/sysbuild/` directory is used to **customize the configuration of extra (child) images** without modifying their source code.

**Methods for Customizing Child Images:**

1.  **Overlaying (Recommended):** Modifies the child image's existing configuration.
    *   **Kconfig:** Create `<main_app>/sysbuild/<image_name>.conf`. This file acts as a Kconfig fragment applied *on top of* the child image's default `prj.conf`.
    *   **Devicetree:** Create `<main_app>/sysbuild/<image_name>.overlay`. This file overlays the child image's base devicetree.

2.  **Overwriting (Less Common):** Completely replaces parts of the child image's configuration. *Use with caution.*
    *   Place files inside `<main_app>/sysbuild/<image_name>/`.
    *   `<...>/prj.conf`: Replaces the child image's base `prj.conf`. **Must exist** if using any other overwriting files below for that image.
    *   `<...>/app.overlay`: Replaces the child image's base devicetree overlay (if any).
    *   `<...>/boards/<board>.conf`: Board-specific Kconfig fragment *for the child image*.
    *   `<...>/boards/<board>.overlay`: Board-specific devicetree overlay *for the child image*.

See [Sysbuild Configuration Guide](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/build_and_config_system/sysbuild/sysbuild_configuring.html) for more examples.

---

## Sysbuild – Partition Manager

### Partition Manager

Zephyr typically uses fixed partitions defined in the devicetree (`fixed-partitions` compatible). However, nRF Connect SDK often needs a more dynamic approach, especially for multi-image builds and DFU/FOTA. This is handled by the **Partition Manager**.

*   **Enabled by Default:** Partition Manager (`SB_CONFIG_PARTITION_MANAGER=y`) is enabled by default when Sysbuild is active.
*   **YAML Configuration:** Uses `.yml` files (instead of DTS) to define partition requirements and layout rules.
*   **Build-Time Generation:** Generates the final partition scheme based on enabled application features and image requirements.
*   **Sysbuild Level:** Partitioning defined by Partition Manager applies consistently across **all** images in the Sysbuild project, ensuring compatibility (no overlaps, correct addresses for linking, bootloader/app agreement). DTS partitioning must be consistent manually if used instead.

### Partitioning in nRF Connect SDK

Three main schemes:

1.  **Devicetree Fixed Partitioning:** Uses standard Zephyr `fixed-partitions`. Suitable for simple, single-image applications without DFU.
2.  **Dynamic Partitioning (Partition Manager):** Default for many NCS scenarios (nRF91+TF-M, nRF53, nRF52+DFU). Automatically creates partitions based on Kconfig flags (e.g., `CONFIG_NVS`, `CONFIG_BOOTLOADER_MCUBOOT`). Rules defined in `pm.yml` files within SDK subsystems/modules.
3.  **Static Partitioning (Partition Manager):** Uses a `pm_static.yml` file *in the application directory* to define a fixed, explicit partition layout. Overrides dynamic rules. Required for production DFU/FOTA firmware; ensures layout stability across builds. Used by default in some complex samples (Matter, Thingy:53).

### Why is partitioning important?

For multi-image builds (bootloader+app, multi-core), ensuring images don't overlap, are linked to the correct addresses, and agree on partition boundaries is critical. Partition Manager automates this consistency.

### Working with the Partition Manager

*   **View Layout:** Check `build/partitions.yml` after building, or use the VS Code Memory Report tool (available after build).
*   **CLI Report:** `west build -t partition_manager_report` (see [Partition Manager docs](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/scripts/partition_manager/partition_manager.html)).
*   **`pm.yml` Files:** Define partition requirements and default sizes. Found in SDK components (e.g., `nrf/samples/bootloader/pm.yml`, `nrf/subsys/partition_manager/`).
*   **Configuring Partitions:** Use Kconfig options defined in relevant `pm.yml` files (often prefixed `CONFIG_PM_PARTITION_SIZE_...`). Set these options in the Kconfig fragment for the *relevant image* (e.g., set `CONFIG_PM_PARTITION_SIZE_MCUBOOT` in `<main_app>/sysbuild/mcuboot.conf`). Searching `CONFIG_PM_` in Kconfig GUI helps, but includes Power Management options too.
*   **Static Configuration (`pm_static.yml`):** Create this file in the main application root to define a fixed layout. Copying `build/partitions.yml` is a good starting point.

---

## Exercise 1 – Configuring extra image

Enable and configure MCUboot (an SDK-provided Sysbuild image) in a basic project.

### Exercise steps

Open the exercise base code: `l8/l8_e1` (select version) using "Copy a sample".

1.  **Build and Flash Base:** Build for your board (e.g., `nrf5340dk/nrf5340/cpuapp`) and flash. Verify the "Hello from DevAcademy..." log message appears. Note only one image (`l8_e1`) in the VS Code APPLICATIONS view.
2.  **Add MCUboot via Sysbuild:**
    2.1 Create `sysbuild.conf` in the project root (`l8/l8_e1/`).
    2.2 Enable MCUboot by adding to `sysbuild.conf`:
       ```kconfig
       # STEP 2.1 - Add MCUboot
       SB_CONFIG_BOOTLOADER_MCUBOOT=y
       ```
       > **Important (nRF7002 DK only):** Add a file `sysbuild/mcuboot.conf` and disable SPI NOR driver to avoid conflicts on this specific DK:
       > ```kconfig
       > # sysbuild/mcuboot.conf for nRF7002DK
       > CONFIG_SPI=n
       > CONFIG_SPI_NOR=n
       > ```
3.  **Pristine Build and Flash:** Perform a pristine build and flash. Observe the terminal: MCUboot logs appear first, followed by the application logs. Note the two images (`l8_e1` and `mcuboot`) now listed in the VS Code APPLICATIONS view.
4.  **Configure MCUboot Image:**
    4.1 Create the directory `l8_e1/sysbuild/`.
    4.2 Create the file `l8_e1/sysbuild/mcuboot.conf`.
    4.3 Add Kconfig options to `mcuboot.conf` to disable serial output from MCUboot:
       ```kconfig
       # STEP 4.1 - Disable anything on UART in MCUboot
       CONFIG_SERIAL=n
       # Optional: Also disable console and logging for smaller size
       # CONFIG_CONSOLE=n
       # CONFIG_LOG=n
       ```
5.  **Pristine Build and Flash:** Perform another pristine build and flash. Observe the terminal: Only the application logs should appear now; MCUboot logs are suppressed.
6.  **Verify Configuration (Optional):**
    *   Check generated config files: `build/mcuboot/zephyr/.config` (should show `CONFIG_SERIAL=n`), `build/l8_e1/zephyr/.config` (should show `CONFIG_SERIAL=y`), `build/zephyr/.config` (Sysbuild's own config).
    *   Use **nRF Kconfig GUI**:
        *   In VS Code APPLICATIONS view, click on `mcuboot (...)`.
        *   Open nRF Kconfig GUI (Actions view).
        *   Search for `CONFIG_SERIAL`. It should be **unchecked**.
        *   Switch context back to `l8_e1 (...)` in APPLICATIONS view.
        *   Open nRF Kconfig GUI again.
        *   Search for `CONFIG_SERIAL`. It should be **checked**.

---

## Exercise 2 – Adding custom image

Add a custom secondary image (`custom_image`) to run alongside the main application (`l8_e2`) on a multi-core device (nRF5340).

> **Requirements:** nRF5340 DK required for this exercise.

### Exercise steps

The base code contains two separate application folders: `l8/l8_e2` (main app) and `l8/custom_image` (secondary app).

1.  **Build/Test Individual Apps:**
    1.1 Build `l8/l8_e2` targeting the *application core* (`nrf5340dk/nrf5340/cpuapp`). Flash and verify its "Hello from DevAcademy..." and LED toggle output.
    1.2 Build `l8/custom_image` *also targeting the application core* for testing. Flash and verify its Nordic logo and benchmark output. (Running directly on the secondary core is complex due to initialization dependencies).
2.  **Configure Sysbuild in Main App (`l8/l8_e2`):**
    2.1 Create `l8/l8_e2/sysbuild.cmake`. Add the `ExternalZephyrProject_Add` call to include `custom_image`, targeting the network core.
       ```cmake
       # l8/l8_e2/sysbuild.cmake
       cmake_minimum_required(VERSION 3.20.5)
       find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

       # Add custom_image targeting the Network core
       ExternalZephyrProject_Add(
         APPLICATION custom_image
         SOURCE_DIR ${APP_DIR}/../custom_image # Path relative to l8_e2
         BOARD nrf5340dk/nrf5340/cpunet        # Target Network core (note: / not _)
       )
       ```
       > **Important (Partition Manager):** For simplicity, this exercise disables Partition Manager. To enable it with Sysbuild, additional lines are needed in `sysbuild.cmake` to share partition info (see Sysbuild Hello World sample).
    2.2 Create `l8/l8_e2/sysbuild.conf` and disable Partition Manager:
       ```kconfig
       # l8/l8_e2/sysbuild.conf
       # STEP 2.2 - Disable the Partition Manager
       SB_CONFIG_PARTITION_MANAGER=n
       ```
3.  **Initialize Secondary Core from Application Core (`l8/l8_e2`):** The application core needs to start the network core.
    *   **For nRF5340 DK (Network Core):**
        *   The network core typically requires the Remote Procedure Call library (`CONFIG_NRF_RPC`) or similar IPC mechanism enabled in *both* cores' `prj.conf` files for communication and startup.
        *   The application core usually needs C code to explicitly release the network core from reset (e.g., using `<nrfx_clock.h>`, `<nrfx_reset_reason.h>`). Check nRF53 examples like Peripheral LBS or Peripheral UART for standard startup procedures.
4.  **Build and Flash Main App:** Perform a **Pristine Build** of `l8/l8_e2` targeting the *application core* (`nrf5340dk/nrf5340/cpuapp`). Flash the resulting merged hex file.
    *   Observe the VS Code APPLICATIONS view showing both `l8_e2 (cpuapp)` and `custom_image (cpunet)` under the build.
5.  **Test:**
    *   Open *two* serial terminals, one for each VCOM port associated with your DK (check Device Manager or `dmesg`).
    *   Reset the board.
    *   One terminal should show the `l8_e2` application core logs (LED toggling).
    *   The other terminal should show the `custom_image` network core logs (Nordic logo, benchmarks).

    > **Note**: By default, nRF53 Network core logs via `UART0`. These are routed to different VCOM ports on the DKs. Custom logging pins require overlays in the `custom_image` project.

---

## Important: Migration from Legacy Multi-Image Builds

**If you are migrating a project from nRF Connect SDK v2.x to v3.0+**, be aware that:

1.  **Parent-child image system is completely removed** - There is no compatibility layer.
2.  **All multi-image projects must use Sysbuild** - This is not optional.
3.  **Configuration file locations have changed** - Child image configs now go in `sysbuild/` directory.
4.  **Board targets use `/` separator** - For example, `nrf5340dk/nrf5340/cpuapp` instead of `nrf5340dk_nrf5340_cpuapp`.

For detailed migration instructions, see the [nRF Connect SDK v3.0 Migration Guide](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/releases_and_maturity/migration/migration_guide_3.0.html).
