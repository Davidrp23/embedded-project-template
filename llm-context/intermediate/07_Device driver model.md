# Device driver model

> **nRF Connect SDK Version:** v3.0.0-v3.2.0
> **Documentation:** [Zephyr Device Driver Model](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/drivers/index.html)

## Overview

A device driver is a combination of statically allocated structures maintained in the kernel that contain information about the device instance. Some of the device information that is worth mentioning is config, data, state, and an API to access the device-specific implementation of all the features supported for this specific device.

In this lesson, we will cover the device driver model, API, instances and implementation. Then we will take a look at the sensor driver API available in nRF Connect SDK, and how to add a custom sensor to it.

In the exercise section, we will use the BME280 sensor with the SPI interface that was used in Lesson 5. We will practice creating a custom driver for this sensor using the sensor driver API.

---

## Device driver model

The Zephyr RTOS that the nRF Connect SDK is based on provides a **device driver model**. This driver implementation is highly decoupled from its API, which allows developers to switch out the low-lever driver implementation without modifying the application on top, because we can use the same generic API.

This decoupling has many benefits, including a high level of portability, as it makes it possible to use the same code on different boards without manually modifying the underlying driver implementation.

> **Recall**: The Zephyr device driver model was also covered in [Lesson 2 – Device driver model](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-2-reading-buttons-and-controlling-leds/topic/device-driver-model/) of the nRF Connect SDK Fundamentals course

The Zephyr device driver model is the architecture that defines the association between the generic APIs and the device driver implementations. We can split the model into three different sections, as shown in the image description below.

*(Diagram description: Application -> Device Driver API -> Device Driver Instances -> Device Driver Implementation -> Hardware)*

The three sub-categories for the device driver model are:

1.  **Device driver APIs:** High-level, hardware-agnostic functions (e.g., `gpio_pin_set()`, `spi_transceive()`, `sensor_sample_fetch()`). Defined in header files like `<zephyr/drivers/gpio.h>`. See [Zephyr Peripheral APIs](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/hardware/peripherals/index.html).
2.  **Device driver instances:** Runtime representation of a specific hardware device (e.g., `&gpio0`, `&spi1`). Obtained via devicetree macros (`DEVICE_DT_GET()`, `GPIO_DT_SPEC_GET()`, etc.). Represented by `const struct device *`.
3.  **Device driver implementation:** Low-level, hardware-specific code that implements the functionality defined by the APIs for a particular peripheral. Linked to instances via the devicetree `compatible` property and driver registration macros.

The application interacts with the generic API. The API uses the device driver instance (device pointer) obtained from the devicetree to call the correct functions within the device driver implementation layer.

### Device driver implementation

When implementing a device driver, one needs to consider several key components, mostly centered around the driver's data structures.

#### Zephyr devices (`struct device`)

Zephyr devices are represented by `struct device`, defined in `<zephyr/device.h>`. This structure holds references to resources and information defined by the driver implementation.

Simplified `struct device`:
```c
struct device {
      const char *name;      // Device name (unique), from DT label property
      const void *config;    // Pointer to read-only config struct (from DT properties)
      void * const data;     // Pointer to runtime data struct (state, counters, etc.)
      const void *api;       // Pointer to the driver's API function struct
      // ... other internal kernel fields ...
};
```

#### Device definition

Devices are typically defined statically at compile time using macros:

*   `DEVICE_DEFINE()`: For non-devicetree devices (less common).
*   `DEVICE_DT_DEFINE()`: For devicetree-based devices, using node ID.
*   `DEVICE_DT_INST_DEFINE()`: For devicetree-based devices, using an instance number based on a `compatible`. **Most common for drivers supporting multiple instances.**

Devices defined this way are automatically initialized by the kernel during boot, before `main()` is called, based on init levels and priorities.

**Using `DEVICE_DT_INST_DEFINE()`:**

Instance-based macros rely on `DT_DRV_COMPAT` being defined *before* the macro call. Set `DT_DRV_COMPAT` to the lowercase-and-underscores version of the `compatible` string your driver supports.

Example: If your driver supports `compatible = "vendor,mysensor"`, define this at the top of your driver `.c` file:
```c
#define DT_DRV_COMPAT vendor_mysensor
```

A typical driver definition using instance macros looks like this:

```c
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h> // Example dependency
// ... other includes ...

// Define compatible for this driver
#define DT_DRV_COMPAT vendor_mysensor

// Forward declarations for driver structs and functions
struct mysensor_config {
    struct spi_dt_spec spi; // Example: Store SPI config from DT
    // Other config...
};

struct mysensor_data {
    // Runtime data like sensor readings, state flags, etc.
    int reading;
};

static int mysensor_init(const struct device *dev);
// ... other driver API function implementations ...

// Define the driver's API function structure
static const struct sensor_driver_api mysensor_api_funcs = { // Using sensor API example
    // .sample_fetch = mysensor_sample_fetch,
    // .channel_get = mysensor_channel_get,
    // ... other API functions ...
};

// Macro to define one instance of the driver
#define MYSENSOR_DEFINE(inst)                                           \
    /* Define instance-specific data */                                 \
    static struct mysensor_data data_##inst;                            \
    /* Define instance-specific config, populated from Devicetree */    \
    static const struct mysensor_config config_##inst = {               \
        .spi = SPI_DT_SPEC_INST_GET(inst, SPI_OP_MODE_MASTER, 0),       \
        /* Get other DT properties: .some_prop = DT_INST_PROP(inst, some_prop), */ \
    };                                                                  \
                                                                        \
    /* Define the device instance */                                    \
    DEVICE_DT_INST_DEFINE(inst,                                         \
                          mysensor_init,        /* Init function */     \
                          NULL,                 /* PM handle (optional)*/ \
                          &data_##inst,         /* Data struct ptr */   \
                          &config_##inst,       /* Config struct ptr */ \
                          POST_KERNEL,          /* Init level */        \
                          CONFIG_SENSOR_INIT_PRIORITY, /* Init priority */ \
                          &mysensor_api_funcs); /* API struct ptr */

// Instantiate the driver for each enabled devicetree node with matching compatible
DT_INST_FOREACH_STATUS_OKAY(MYSENSOR_DEFINE)
```

*   `MYSENSOR_DEFINE(inst)`: A helper macro defining static data/config structs and calling `DEVICE_DT_INST_DEFINE` for a given instance `inst`.
*   `DEVICE_DT_INST_DEFINE()`: Registers the device instance with the kernel, linking the instance number (`inst`), init function, data, config, API struct, and init level/priority.
*   `DT_INST_FOREACH_STATUS_OKAY(MYSENSOR_DEFINE)`: This crucial macro expands the `MYSENSOR_DEFINE` macro for every devicetree node instance (`inst` = 0, 1, 2...) that has `compatible = "vendor,mysensor"` and `status = "okay"`.

#### Key components of a device driver

1.  **Configuration (`config` struct):** Holds read-only configuration data, often populated from devicetree properties during initialization. Pointed to by `dev->config`.
2.  **Runtime Data (`data` struct):** Holds variables that change during runtime (state, counters, buffers, etc.). Pointed to by `dev->data`.
3.  **Initialization Function (`init`):** Runs during kernel boot (before `main`). Sets up hardware, allocates resources, configures pins, registers interrupts. Defined via `DEVICE_DT..._DEFINE`. Uses `SYS_INIT()` internally.
4.  **Interrupt Handlers (ISRs):** Code executed when the hardware generates an interrupt. Registered during init. Should be fast; often defers work to threads (e.g., using workqueues).
5.  **Power Management (Optional):** Implements `struct pm_device_ops` and registers with `DEVICE_DT..._DEFINE` to participate in Zephyr's power management system (suspend/resume device state).
6.  **API Structure (`api` struct):** A struct containing function pointers that implement the standard driver API (e.g., `struct sensor_driver_api`, `struct gpio_driver_api`). Allows application code to call driver functions via the generic API headers. Pointed to by `dev->api`. See [Sensor API Reference](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/hardware/peripherals/sensor.html).

---

## Setting up a west workspace

> **Reference:** [Application Development](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/app_dev/index.html) and [West Basics](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/develop/west/basics.html)

### Application types

Based on location relative to the Zephyr west workspace and SDK repositories:

1.  **Repository Application:** Located *inside* the SDK source tree (e.g., `nrf/samples/`, `zephyr/samples/`).
    *(Diagram description: File structure showing app inside nrf/ or zephyr/)*
2.  **Freestanding Application:** Located *outside* the SDK workspace directory. Needs `$ZEPHYR_BASE` environment variable set or passed to CMake. (Examples: Nordic Developer Academy course exercises).
    *(Diagram description: File structure showing SDK and app in separate top-level folders)*
3.  **Workspace Application:** Located *inside* the west workspace directory, but *outside* the main SDK/Zephyr repositories (often in a sibling directory). Managed as part of the workspace.
    *(Diagram description: File structure showing app folder alongside nrf/, zephyr/)*

### Setting up a workspace application

Use `west` commands to manage workspaces.

*   **`west init`:** Creates a new workspace.
    *   From remote manifest: Clones the manifest repository (which lists projects like nrf, zephyr, modules, and potentially your application repo) and sets up the workspace structure.
        ```bash
        # Example: Initialize workspace using ncs-example-application manifest
        west init -m https://github.com/nrfconnect/ncs-example-application --mr main my-workspace
        cd my-workspace
        west update # Clones/updates projects listed in the manifest (nrf, zephyr, etc.)
        ```
    *   From local manifest: Creates a workspace around an existing local directory containing a `west.yml` manifest file.
        ```bash
        # Assume ncs-example-application is already cloned locally
        west init -l ncs-example-application
        # cd into the new workspace directory if needed
        # west update might still be needed depending on the state
        ```
*   **`west update`:** Clones or updates projects listed in the workspace's manifest file (`west.yml`) to their specified revisions.