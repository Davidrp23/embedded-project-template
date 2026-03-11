# Zephyr RTOS: Beyond the basics

> **nRF Connect SDK Version**: This lesson is compatible with nRF Connect SDK v3.0.0 - v3.2.x. Documentation links point to `docs.nordicsemi.com` (the new documentation portal that replaced `developer.nordicsemi.com`).

## Overview

In this lesson, we will dive into thread management and data passing in the nRF Connect SDK/Zephyr. We have two execution contexts: **Thread context** and **Interrupt context**. Each context has its own usage and timing implications.

We will first examine the different execution primitives, and how different types of threads and different types of interrupts co-exist. Then we will take a closer look at the life cycle of a thread and how the scheduler works. After that, we will also cover frequently used data-passing techniques such as message queues and FIFOs

At the end of the lesson, you will have a solid understanding of the options available to schedule application tasks and their different time constraints. You will also gain a good hands-on grasp on how to safely pass data between threads and the kernel options available.

---

## Boot-up sequence & execution context

For implementing tasks for your application code, you need to pick the proper execution primitive (pre-emptible thread, cooperative thread, work queue, etc.) and set its priority correctly to not block other tasks that are running on the CPU while you also meet the time requirements of the task.

One of the main goals of this lesson is to learn how to schedule application tasks using the right execution primitive with the right priority level. But before we can dive into the topic of choosing the right execution method to run a given task, we need to consider the following questions:

*   How does an nRF Connect SDK application boot up?
*   What are the out-of-the-box threads and ISR in an application, and what is their priority and execution nature?
*   What is the difference between interrupt and thread contexts, and what to do in each?

### Boot-up sequence

1.  **Early Boot Sequence (C Code Preparation phase)**
    The primary function of the early boot sequence is to transition the system from its reset state to a state where it becomes capable of executing C code, thereby initiating the kernel initialization sequence. This stage is a pretty standard phase in embedded devices; as an application developer, it is not of much interest.

2.  **Kernel Initialization**
    This stage presents a process of initializing the state for all enabled static devices. Some devices are enabled out-of-the-box by the RTOS, while others are enabled by your application configuration file (`prj.conf`) and your board configuration files as we learned in the nRF Connect SDK Fundamentals course. These latter devices encompass device driver and system driver objects that are defined using Zephyr APIs in a static manner.

    The initialization order is controlled by assigning them to specific run levels (for example, `PRE_KERNEL_1`, `PRE_KERNEL_2`), and their initialization sequence within the same run level is further defined using priority levels. Keep in mind that at this stage, the scheduler and the Kernel services are not yet available, so these initialization functions do not rely on any Kernel services. We will dive into driver initialization in Lesson 7.

    2.1. *What gets initialized in `PRE_KERNEL_1` by default in all nRF Connect SDK applications?*
    *   **Clock Control driver:** Enables support for the hardware clock controller. Used by other subsystems and for power efficiency.
    *   **A serial driver:** Can be UART(E), RTT, or other transports. Used for debugging output (e.g., boot banner) if debugging is enabled.

    2.2. *What gets initialized in `PRE_KERNEL_2` by default in all nRF Connect SDK applications?*
    *   **System Timer driver:** Usually a Real-time counter peripheral (RTC1 on nRF52/nRF53/nRF91 Series, GRTC on nRF54 Series). Used for kernel timing services (`k_sleep()`, kernel timers). The timer selection is controlled by `CONFIG_NRF_GRTC_TIMER` and `CONFIG_NRF_RTC_TIMER`.

    > **Important**: The list of devices does not include all devices that get initialized in your application. The list only provides the minimal core devices needed by the RTOS. The devices and subsystems initialized will depend on your application configuration and your board configuration file.

3.  **Multithreading Preparation**
    This is where the multithreading features get initialized, including the scheduler. The RTOS will also create two threads (System threads): The RTOS **main thread**, and the **idle thread** (responsible for power management when no other threads are ready).

    During this phase, the `POST_KERNEL` services are initiated, if any exist. Once `POST_KERNEL` services are initiated, the boot banner is printed:
    ```
    *** Booting nRF Connect SDK v3.0.x ***
    ```
    > **Note**: Since nRF Connect SDK v2.5.0, the boot banner displays the nRF Connect SDK version instead of the Zephyr version.

    After that, the `APPLICATION` level services are initialized, if any exist. Then, all application-defined static threads (using `K_THREAD_DEFINE()`) are initiated.

    3.1. *What gets initialized in `POST_KERNEL` by default in all nRF Connect SDK applications?*
    This is where many libraries, RTOS subsystems, and services get initialized, as they require kernel services to be available. By default, the RTOS doesn't initialize anything here itself, but enabled components do. Examples:
    *   Logging module (`CONFIG_LOG` in deferred mode): Logger thread created.
    *   Bluetooth Low Energy stack (`CONFIG_BT`): Stack initialized, RX/TX threads created.
    *   System work queue (`CONFIG_SYSTEM_WORKQUEUE`): System workqueue thread initialized.

    3.2. *What gets initialized in `APPLICATION` by default in all nRF Connect SDK applications?*
    By default, some libraries get initiated here if enabled. For instance, the AT Monitor Library (`AT_MONITOR`) for nRF91 Series.

    The RTOS main thread is the currently active thread during these final initialization phases. After it’s done, it will call your `main()` function, if it exists. If no `main()` exists, the RTOS main thread terminates, and the scheduler picks the next ready thread (user-defined, subsystem, or idle). The choice depends on thread type and priority.

    > **Note**: For multi-core hardware (e.g., nRF5340), other peripherals like the mailbox (mbox) are initialized. If TF-M is used, an entropy source (e.g., `psa-rng`) is initialized.

After boot-up, several threads and interrupts are set up. Next, we compare their execution contexts.

### Thread context vs interrupt context

Interrupt and thread contexts refer to different execution environments with distinct characteristics and intended usages.

**Thread context**

*   **Execution Context:** Normal environment where application and system threads run.
*   **Triggering Event:** Threads created by application/RTOS, scheduled based on rules (type, priority).
*   **Preemption:** Can be preempted by interrupts or higher-priority threads.
*   **Duration:** Can execute longer, more complex operations.
*   **Allowed Operations:**
    *   Access full range of kernel/OS services.
    *   Execute time-consuming operations.
    *   Wait on synchronization primitives (mutexes, semaphores, events).
    *   Perform blocking I/O.
*   **Not Allowed Operations:**
    *   Direct hardware register access without proper synchronization/abstraction.
    *   (Generally) Running extremely time-critical operations (use interrupts).

**Interrupt context**

*   **Execution Context:** Environment when an interrupt handler (ISR) is running.
*   **Triggering Event:** Triggered asynchronously by hardware events (timers, signals, I/O).
*   **Preemption:** Preempts the currently running thread context.
*   **Duration:** Expected to execute **very quickly** to minimize system latency.
*   **Interrupts Nesting:** Zephyr allows nested interrupts (higher priority interrupts can preempt lower priority ISRs).
*   **Allowed Operations:**
    *   Executing time-critical operations.
    *   Access to a restricted set of kernel services (non-blocking ones).
*   **Not Allowed Operations:**
    *   **Blocking operations** (sleeping, waiting on locks/semaphores with timeouts like `K_FOREVER`).
    *   Using most kernel services meant for thread context.

---

## Thread life cycle

A thread is the basic unit of runnable code. The vast majority of firmware code will run in threads – whether it’s a user-defined thread, a thread created by the RTOS (e.g., system workqueue thread), a thread created by an RTOS subsystem (e.g., logger module), or a thread created by a library (e.g., AT Monitor library).

A thread has the following items:

*   **Thread control block:** Type `k_thread`. An instance maintained by the RTOS holding thread metadata.
*   **Stack:** Each thread has its own stack area. Size must be adequate for the thread's processing needs.
*   **Entry point function:** The body/functionality of the thread. Usually contains an infinite loop (exiting terminates the thread). Can receive optional arguments.
*   **Thread priority:** A signed integer governing the thread's scheduling importance and type (cooperative vs. preemptible).
*   **Optional thread options:** Special flags (e.g., `K_ESSENTIAL`) affecting kernel behavior regarding the thread.
*   **Optional starting delay:** Can start immediately (`K_NO_WAIT`) or after a specified delay. Can also be created inactive (`K_FOREVER`) and started later with `k_thread_start()`.

### Creating a thread

Threads are created using either the `K_THREAD_DEFINE()` macro (static, compile-time) or the `k_thread_create()` function (dynamic, run-time, requires manual stack allocation with `K_THREAD_STACK_DEFINE`).

> **Recall**: From the Fundamentals course, threads can be started immediately or with a delay. Once started, they enter the ready queue. Creating with `K_FOREVER` delay makes them inactive until `k_thread_start()` is called.

> **Definition: Ready queue**
> The queue of threads in the Ready state. The scheduler only considers threads in this queue when selecting the next thread to run.

### Thread states

If the scheduler picks a thread for execution, its state transitions to **Running**. It stays Running until:

1.  **It becomes Unready:**
    *   **Sleeping:** Calls `k_sleep()` or derivatives.
    *   **Suspended:** Another thread calls `k_thread_suspend()`.
    *   **Waiting:** Waits for an unavailable kernel object (mutex, semaphore, etc.).
2.  **It Yields or is Preempted:**
    *   **Yielding:** Calls `k_yield()` to give up the CPU voluntarily (moves to end of ready queue).
    *   **Preempted:** Scheduler switches to a higher-priority ready thread (moved to end of ready queue).
3.  **It Terminates or Aborts:**
    *   **Termination:** Exits its entry point function (usually for non-repetitive tasks).
    *   **Aborting:** Encounters a fatal error (e.g., null pointer dereference) or is aborted by itself or another thread using `k_thread_abort()`.

*(Diagram description from original text: Thread life cycle diagram)*

More details on threads can be found on the [Threads page](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/index.html) of the nRF Connect SDK documentation.

---

## Scheduler in-depth

The scheduler has a straightforward task: it picks a thread from the queue of ready threads (the **ready queue**) as the currently active **Running** thread and hands it the CPU.

*(Diagram description from original text: Scheduler picking from ready queue)*

Deciding which thread to run is **100% deterministic**, based on rules determining thread **priority**. The scheduler doesn't consider fairness or execution history; the firmware developer must assign priorities correctly to manage CPU sharing.

### Context switch

During thread execution, CPU registers are used, and memory is accessed. The combined state (registers, stack) forms the thread's **context**.

A thread doesn't know when it will be preempted or interrupted. If preemption occurs just before an operation (e.g., subtracting R0 and R1), other running code might change R0 and R1. If the original thread resumes without its original context, the subtraction yields an incorrect result.

To prevent this, the RTOS **saves the context** of a thread when it's preempted and **restores it** before resuming execution. This save/restore process is called **context switching**.

*(Diagram description from original text: Context switching visualization)*

Context switching consumes time and power. Good firmware design aims to minimize unnecessary context switches. Context switching also occurs for interrupts.

### Thread types

*   **Preemptible threads:** (Non-negative priority) Most common for user applications. Can be preempted by the scheduler if a higher or equal priority thread becomes ready.
*   **Cooperative threads:** (Negative priority) Cannot be preempted by the scheduler. Run until they voluntarily block (sleep, wait, yield). Used for enforcing scheduler locking, often in subsystems, drivers, or performance-critical sections. Interrupts can still preempt cooperative threads, but execution *always* returns to the interrupted cooperative thread afterward.
*   **Meta-IRQ threads:** Special class of cooperative thread, **not for user applications**. Used by drivers for urgent "bottom half" processing triggered immediately after a hardware ISR. Can preempt other threads, including cooperative ones. Example: Bluetooth LE stack uses Meta-IRQ threads.

### Thread priority

Threads are assigned integer priorities. **Lower numbers have higher priority.**
*   Negative priorities (-1 to -16 by default): Cooperative threads. System work queue often runs at -1.
*   Non-negative priorities (0 to 14 by default): Preemptible threads. `main` thread usually runs at 0. `idle` thread runs at the lowest preemptible priority (15 by default). Logger thread often runs at 14.

> **Note**: Avoid using priority 15 (or `CONFIG_NUM_PREEMPT_PRIORITIES` - 1) for user threads, as it's reserved for the idle thread. The lowest recommended user priority is 14 (or `CONFIG_NUM_PREEMPT_PRIORITIES` - 2).

The number of levels is configurable (`CONFIG_NUM_PREEMPT_PRIORITIES`, `CONFIG_NUM_COOP_PRIORITIES`). Priorities can be changed dynamically at runtime (`k_thread_priority_set`), potentially changing a thread between preemptible and cooperative types.

### Scheduler locking and disabling interrupts

*   **Scheduler Locking:** Temporarily prevents context switching between threads, ensuring atomic execution of a code section relative to other threads.
    *   Cooperative threads inherently provide scheduler locking.
    *   Preemptible threads can use `k_sched_lock()` and `k_sched_unlock()`. `k_sched_lock()` effectively makes the current thread behave cooperatively while locked. (Not commonly used in application code).
*   **Disabling Interrupts:** Protects a critical section from both thread preemption *and* ISRs. Use `irq_lock()` (returns an interrupt locking key) and `irq_unlock(key)`. Use with extreme caution and for very short durations.

### Threads with equal priority

Multiple threads can share the same priority level (except the idle priority). The scheduler handles this based on configuration:

*   **Default behavior:** Runs the thread that became ready first (FIFO within the priority level).
*   **Time slicing (`CONFIG_TIMESLICING=y`):** Each thread runs for a fixed duration (`CONFIG_TIMESLICE_SIZE`) before being preempted, allowing other threads at the *same priority* to run. (Covered in Fundamentals Lesson 7, Exercise 2). Only affects priorities >= `CONFIG_TIMESLICE_PRIORITY`.
*   **Earliest Deadline First (EDF) (`CONFIG_SCHED_DEADLINE=y`):** Scheduler picks the thread with the earliest deadline (set via `k_thread_deadline_set()`). Developer is responsible for setting deadlines.

### Rescheduling points

Zephyr's tickless kernel relies on **rescheduling points** (events triggering scheduler evaluation) instead of periodic ticks. Examples:

*   `k_yield()` called.
*   Thread blocks (e.g., `k_sleep()`, waiting on semaphore/mutex).
*   A blocked thread becomes unblocked (e.g., semaphore given, data arrives in queue).
*   Time slice expires (if `CONFIG_TIMESLICING=y`).

---

## Data passing

Now that we understand execution contexts (ISRs, threads), let's learn how to safely exchange data between them. We'll cover message queues and FIFOs.

### Message queue (`k_msgq`)

A **message queue** is a thread-safe kernel object holding a fixed number of fixed-size data items (messages).

*   **Thread Safety:** Kernel manages internal locking; multiple threads can safely put/get messages.
*   **Data Type:** Message size/type defined statically at creation. Can be simple types, structs, pointers, etc.
*   **Capacity:** Fixed number of messages, defined at creation. Limited by available RAM.
*   **Blocking/Timeout:** Threads can wait (`K_FOREVER` or timeout with `K_MSEC()`) if the queue is full (when putting) or empty (when getting).
*   **Priority Handling:** If multiple threads wait, the highest-priority waiting thread is serviced first when space/data becomes available.
*   **ISR Usage:** Can be used from ISRs, but **never** with blocking waits (`K_FOREVER` or timeouts > 0). Use `K_NO_WAIT`.

*(Diagram description from original text: Message queue visualization)*

Message queues use a ring buffer internally. Data size must be a multiple of alignment (usually 4 bytes). Pad data or use compiler attributes (`__aligned(4)`) if needed.

#### How to use

1.  **Decide message data structure.** Example using a union within a struct for variable data types:
    ```c
    struct MyStruct {
      int dataType; // 0=int, 1=float, 2=string...
      union {
        int intValue;
        float floatValue;
        char stringValue[24];
      } data;
    };
    ```
    (Example from Bluetooth HRS sample uses `struct bt_hrs_client_measurement`).

2.  **Define and initialize the message queue** using `K_MSGQ_DEFINE()`:
    *(Description of image: K_MSGQ_DEFINE() parameters)*
    ```c
    // Example: Queue for 16 messages, each sizeof(uint32_t), 4-byte aligned
    K_MSGQ_DEFINE(device_message_queue, sizeof(uint32_t), 16, 4);
    ```

3.  **Write (put) a message** using `k_msgq_put()`:
    ```c
    uint32_t my_message = 123;
    int ret = k_msgq_put(&device_message_queue, &my_message, K_FOREVER); // Wait if full
    if (ret != 0) {
        // Handle error (e.g., -ENOMSG if K_NO_WAIT and full, -EAGAIN if timeout)
    }
    ```
    *   `K_FOREVER`: Wait indefinitely if full.
    *   `K_MSEC(timeout_ms)`: Wait up to `timeout_ms` if full.
    *   `K_NO_WAIT`: Return immediately (-ENOMSG) if full.

4.  **Read (get) a message** using `k_msgq_get()` (removes message) or `k_msgq_peek()` (reads without removing):
    *(Description of image: k_msgq_get() parameters)*
    ```c
    uint32_t received_message;
    int ret = k_msgq_get(&device_message_queue, &received_message, K_FOREVER); // Wait if empty
    if (ret == 0) {
        // Process received_message
    } else {
        // Handle error (e.g., -EAGAIN if timeout, -ENOMSG if K_NO_WAIT and empty)
    }
    ```
    Messages are read in FIFO order.

**Suggested Use:** Transferring fixed-size data items asynchronously between threads (and carefully with ISRs).

### FIFO (`k_fifo`)

A **FIFO** (First-In, First-Out) is a kernel object providing a queue for data items of **variable size and number**. Threads and ISRs can add/remove items.

*   **Dynamic Size/Number:** Doesn't require specifying item size or count statically.
*   **Pointer Queue:** FIFO stores *pointers* to data items, not the data itself.
*   **Memory Management:** Typically requires dynamic memory allocation (heap: `k_malloc`, `k_free`) for the data items themselves. Developer is responsible for freeing memory after consuming items.
*   **Capacity Limit:** Limited only by available heap memory (`CONFIG_HEAP_MEM_POOL_SIZE`).
*   **Blocking/Timeout:** `k_fifo_get()` can block/timeout if empty. `k_fifo_put()` does *not* block (assumes heap has space).
*   **ISR Usage:** Can be used from ISRs (non-blocking `k_fifo_put`, `k_fifo_get` with `K_NO_WAIT`).

*(Diagram description from original text: FIFO visualization using heap)*

#### How to use

1.  **Allocate heap memory** in `prj.conf`. Set based on maximum expected concurrent items and their sizes. Default is 0.
    ```kconfig
    CONFIG_HEAP_MEM_POOL_SIZE=4096 # Example: 4KB heap
    ```
    > **Note**: Handle heap allocation carefully in embedded systems. Ensure `k_free` is always called for consumed items to prevent memory leaks.

2.  **Define the FIFO** statically using `K_FIFO_DEFINE()`:
    ```c
    K_FIFO_DEFINE(my_fifo);
    ```

3.  **Define the data item structure.** The **first member MUST be `void *fifo_reserved;`** for the kernel's internal linked list.
    Example 1: Fixed-size data buffer within the item:
    ```c
    struct data_item_t {
        void *fifo_reserved; // MUST BE FIRST
        uint8_t  data[256];
        uint16_t len;
    };
    ```
    Example 2: Pointer to variable-size data allocated separately:
    ```c
    struct data_item_var_t {
        void *fifo_reserved; // MUST BE FIRST
        void *data; // Pointer to heap-allocated data
        uint16_t len;
    };
    ```

4.  **Add (put) a data item** using `k_fifo_put()`:
    *(Description of image: k_fifo_put() API)*
    ```c
    /* Allocate memory for the data item */
    struct data_item_t *buf = k_malloc(sizeof(struct data_item_t));
    if (buf == NULL) {
        /* Handle allocation failure */
        return;
    }
    /* Populate the data item (e.g., using memcpy, snprintf) */
    // buf->len = ...;
    // memcpy(buf->data, source_data, buf->len);

    /* Put the pointer into the FIFO */
    k_fifo_put(&my_fifo, buf); // Does not block
    ```
    The same item cannot be added twice without being removed first.

5.  **Read (get) a data item** using `k_fifo_get()` (removes pointer from FIFO):
    *(Description of image: k_fifo_get() API)*
    ```c
    struct data_item_t *rec_item = k_fifo_get(&my_fifo, K_FOREVER); // Wait if empty
    if (rec_item != NULL) {
        /* Process FIFO data item using rec_item->data, rec_item->len */

        /* CRITICAL: Free the allocated memory */
        k_free(rec_item);
    } else {
        // Handle error (e.g., NULL if K_NO_WAIT and empty, or other errors)
    }
    ```
    Failure to `k_free()` consumed items leads to heap overflow.

Complete FIFO API list [here](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/data_passing/fifos.html).

**Suggested Use:** Asynchronously transferring variable-size or variable-number data items between threads (and carefully with ISRs), especially when dynamic allocation is acceptable.

> **Important**: You *can* use statically allocated memory pools instead of the heap with FIFOs if dynamic allocation is undesired, but managing the static pool becomes the developer's responsibility. Remember: never add the *same* data item instance twice.

---

## Exercise 1 – Exploring threads and ISRs

In this exercise, you will learn through hands-on exercises how to differentiate between thread context and interrupt context, and to understand the allowed operations in each.

The exercise covers the use of the **Kernel Timer API**. It is used to periodically trigger a function (ISR) that runs in the System timer interrupt context (RTC1 on nRF52/nRF53/nRF91 Series, GRTC on nRF54 Series).

You will practice collecting information about running threads using the **nRF Debug view** in VS Code:
*   Stack allocation and run-time usage.
*   Priority and state (Running, Ready, Unready).
*   Ready queue content.

You will also use a **message queue** to pass data safely between threads.

This exercise assumes you have completed Lesson 1 of the Fundamentals course and have the nRF Connect SDK (v3.0.0 or later) and VS Code set up.

### Exercise steps

#### Getting the course’s repository

(Steps A-D describe adding the course repo (`devacademy-ncsinter`) to the SDK's `west.yml` manifest and running `west update`. Perform these steps once if you haven't already.)

A. Open VS Code, open the **nRF Connect terminal**.
   > **Important**: Use the nRF Connect terminal for `west` commands.
B. Type `west manifest --path` to find your `west.yml` file location. Ctrl+Click the path to open it.
C. Add the course repository to `west.yml` under `projects:`, ensuring correct indentation:
   ```yaml
     # Other third-party repositories
     - name: devacademy-ncsinter
       path: nrf/samples/devacademy/ncs-inter # Or your preferred path
       revision: main # Or specific tag/commit
       url: https://github.com/NordicDeveloperAcademy/ncs-inter
   ```
   Save the file.
D. In the nRF Connect terminal, run `west update -x devacademy-ncsinter` to download the repository. The course code will be in the specified `path`.

#### Opening the code base

(Perform for each exercise)

A. In VS Code nRF Connect extension, click **Create a new application** -> **Copy a sample**.
B. Search for the exercise name: `Lesson 1 – Exercise 1`. Select the base code version matching your SDK (v3.0.0 or later for this guide).
C. Choose a location to store the exercise copy (e.g., `C:\myfw\ncsinter\l1_e1`).
D. Press Enter. VS Code will copy the sample and open it.

#### Building the application

This exercise creates an application with:
*   **5 Threads:** producer (prio 6), consumer (prio 7), main (prio 0), logging (prio 14), idle (prio 15).
*   **1 Timer ISR:** Runs every 500ms in interrupt context, toggling LEDs.

*   **Main thread:** Sets up GPIO, starts timer, terminates.
*   **Producer thread:** Generates emulated sensor data (struct), puts into message queue every 2.2s. Stack 2048 bytes.
*   **Consumer thread:** Waits (`K_FOREVER`) for data on message queue, logs received data. Stack 2048 bytes.
*   **Logging/Idle threads:** System threads as described before.

#### Modifying the code

1.  **Enable debugging options** in `prj.conf`:
    ```kconfig
    # Enable thread info for debugger and optimize for debugging
    CONFIG_DEBUG_THREAD_INFO=y
    CONFIG_DEBUG_OPTIMIZATIONS=y
    ```
    > **Note**: Alternatively, use the "Optimize for debugging (-Og)" option in the VS Code build configuration GUI.

2.  **Create a kernel timer** that runs `timer0_handler` in interrupt context.
    2.1. Define the timer statically (e.g., near top of `main.c`):
    ```c
    // STEP 2.1: Define timer
    K_TIMER_DEFINE(timer0, timer0_handler, NULL); // Expiry func = timer0_handler, Stop func = NULL
    ```
    2.2. Start the timer periodically in `main()`:
    ```c
    // STEP 2.2: Start timer
    /* start periodic timer that expires once every 0.5 second */
    k_timer_start(&timer0, K_MSEC(500), K_MSEC(500)); // Initial delay 500ms, period 500ms
    ```
    2.3. Create the timer handler function (`timer0_handler`) *before* `main()`:
    ```c
    // STEP 2.3: Define timer handler function (runs in ISR context)
    static void timer0_handler(struct k_timer *dummy)
    {
        /* Interrupt Context - System Timer ISR */
        static bool flip = true;
        // Ensure led0 and led1 are defined gpio_dt_spec accessible here
        if (flip) {
            gpio_pin_toggle_dt(&led0);
        } else {
            gpio_pin_toggle_dt(&led1);
        }
        flip = !flip;
    }
    ```

3.  **Pass data using a message queue.**
    3.1. Define the message data structure (e.g., after includes):
    ```c
    // STEP 3.1: Define message structure
    typedef struct {
        uint32_t x_reading;
        uint32_t y_reading;
        uint32_t z_reading;
    } SensorReading;
    ```
    3.2. Define the message queue (e.g., near top of file):
    ```c
    // STEP 3.2: Define message queue (holds 16 SensorReading messages, 4-byte aligned)
    K_MSGQ_DEFINE(device_message_queue, sizeof(SensorReading), 16, 4);
    ```
    3.3. Write messages from the producer thread. Add inside the `producer_func`'s loop:
    ```c
    // STEP 3.3: Put message in queue (inside producer_func loop)
    // Assume 'acc_val' is a variable of type SensorReading populated with data
    ret = k_msgq_put(&device_message_queue, &acc_val, K_FOREVER); // Wait if full
    if (ret) {
        LOG_ERR("Return value from k_msgq_put = %d", ret);
    }
    ```
    3.4. Read messages in the consumer thread. Add inside the `consumer_func`'s loop:
    ```c
    // STEP 3.4: Get message from queue (inside consumer_func loop)
    SensorReading temp; // Local variable to store received message
    ret = k_msgq_get(&device_message_queue, &temp, K_FOREVER); // Wait if empty
    if (ret) {
        LOG_ERR("Return value from k_msgq_get = %d", ret);
    } else {
        // Process received message in 'temp'
        LOG_INF("Values got from the queue: %u.%u.%u", temp.x_reading, temp.y_reading, temp.z_reading);
    }
    ```

4.  **Build** your application (add build configuration if needed).

#### Debugging and analytics

Follow these steps after building:

1.  **Start a debugging session** (Actions view -> Debug button). Execution breaks at `main()`.
2.  Open the **Run and Debug View** (Activity Bar icon).
3.  Open the **Thread Viewer** (Panel View area -> nRF Debug tab).
    *   Observe the initial threads (main, producer, consumer, logging, idle), their priorities, and states. `main` is running.
    *   Use "Enable Tracking" (brush icon) next to Stack Usage to see real-time stack high water marks.

#### Threads in an nRF Connect SDK application

1.  **Set breakpoints:** Click in the gutter next to the line numbers for `k_msgq_put()` in `producer_func` and `k_msgq_get()` in `consumer_func`.
2.  **Continue execution** (F5 or Continue button). Execution stops at the breakpoint in `producer_func`.
    *   Observe in Thread Viewer: `main` is gone (terminated). `producer` is Running.
3.  **Watch the message queue:** Right-click `device_message_queue` in the code -> Add to Watch. Observe `used_msgs: 1` in the Watch view. Check stack usage.
4.  **Continue execution.** Execution stops at the breakpoint in `consumer_func`.
    *   Observe in Thread Viewer: `producer` is Unready (likely sleeping due to `k_msleep`). `consumer` is Running.
5.  **Watch the message queue:** Observe `used_msgs: 0` in the Watch view. Check stack usage updates.
6.  **Continue execution.** `consumer` blocks again on `k_msgq_get` (empty queue), becomes Unready. `logging` thread might run briefly if logs were generated, then becomes Unready. `idle` thread becomes Running (system likely enters low power).
7.  **Remove breakpoints.**
8.  **Continue execution.**
9.  **View serial output:** Switch to nRF Connect extension -> Connected Devices -> Connect to Serial Port. Observe the logged messages from the consumer thread appearing every ~2.2 seconds.

#### ISR in an nRF Connect SDK application

1.  **Add a breakpoint** inside `timer0_handler()`.
2.  **Continue execution.** Breakpoint hits periodically (every 500ms).
3.  **Examine Call Stack:** In the Run and Debug view -> CALL STACK panel, observe that `timer0_handler()` is running in an "Exception handler" context, likely interrupting the `idle` thread or another lower-priority thread.
4.  **Remove the breakpoint.**

#### Experimenting with the dos and don’ts of the interrupt context

1.  **Add a blocking call** inside `timer0_handler()` (DO NOT DO THIS IN REAL CODE):
    ```c
    static void timer0_handler(struct k_timer *dummy)
    {
        k_msleep(2000); // <<< ADD THIS (BAD!)
        // ... rest of handler code ...
    }
    ```
2.  Build the application. It will build without compile errors.
3.  Flash the application. Observe the output. The system will likely crash and continuously reset, printing fatal error messages because blocking (`k_msleep`) is illegal in an ISR context.
    ```terminal
    [timestamp] <err> os: ***** MPU FAULT *****
    ...
    [timestamp] <err> os: >>> ZEPHYR FATAL ERROR ...
    [timestamp] <err> os: Fault during interrupt handling
    ... (board resets and repeats)
    ```
    > **More on this: Build targets with TF-M**
    > If using a TF-M target (e.g., `_ns`), the fatal error might appear on the Secure Processing Environment's log output (not typically routed to the application VCOM). You might just see the application boot, print one log, reset, and repeat.

4.  **Restore the application:** Remove the `k_msleep(2000);` call, rebuild, and re-flash.

> **Important**: If lengthy processing is needed based on a timer, use the timer ISR to submit work to a **work queue** instead of performing the work directly in the ISR.

---

## Exercise 2 – Kernel options

This exercise practices using **FIFO** for data passing and explores kernel configuration options via the **nRF Kconfig GUI**.

You will build an application similar to Exercise 1, but the producer thread will generate a *random* number of data items (4-14) each time it runs, emulating variable data arrival. FIFO is suitable for this scenario.

### Exercise steps

Open the exercise base code: `l1/l1_e2` (select version) from the course repository using "Copy a sample".

1.  **Enable random number generation** in `prj.conf` for emulation:
    ```kconfig
    # STEP 1: Enable Entropy driver for random numbers
    CONFIG_ENTROPY_GENERATOR=y
    ```
    This enables the hardware entropy source. `sys_rand32_get()` can then be used.

2.  **Allocate heap size** in `prj.conf` for FIFO data items. Calculate based on max items (14) * item size (~40 bytes assumed) + overhead. 1024 bytes should be sufficient here.
    ```kconfig
    # STEP 2: Set Heap Size for FIFO items
    CONFIG_HEAP_MEM_POOL_SIZE=1024
    ```
3.  **Define the FIFO** globally in `main.c`:
    ```c
    // STEP 3: Define FIFO
    K_FIFO_DEFINE(my_fifo);
    ```
4.  **Define the FIFO data item structure** (ensure `fifo_reserved` is first):
    ```c
    // STEP 4: Define FIFO data item structure
    #define MAX_DATA_SIZE 40 // Example max size for string
    struct data_item_t {
        void *fifo_reserved; // MUST BE FIRST
        uint8_t  data[MAX_DATA_SIZE];
        uint16_t len; // Actual length of data in the buffer
    };
    ```
5.  **Add data items to the FIFO** in `producer_func()`. Replace the previous message queue logic:
    ```c
    // STEP 5: Modify producer to use FIFO
    #define MIN_DATA_ITEMS 4
    #define MAX_DATA_ITEMS 14
    static uint32_t dataitem_count = 0; // Keep track of sequence

    void producer_func(void) // Renamed for clarity
    {
        LOG_INF("Producer Thread started");
        while (1) {
            int bytes_written;
            /* Generate a random number of items to send */
            uint32_t data_number =
                MIN_DATA_ITEMS + sys_rand32_get() % (MAX_DATA_ITEMS - MIN_DATA_ITEMS + 1);

            for (int i = 0; i < data_number; i++) {
                /* Create (allocate) a data item */
                struct data_item_t *buf = k_malloc(sizeof(struct data_item_t));
                if (buf == NULL) {
                    LOG_ERR("Unable to allocate memory for FIFO item");
                    // Consider how to handle allocation failure (e.g., break, return)
                    continue; // Skip this item if allocation fails
                }
                /* Populate the data item */
                bytes_written = snprintf(buf->data, MAX_DATA_SIZE, "Data Seq. %u: %u",
                                         dataitem_count, sys_rand32_get());
                if (bytes_written < 0 || bytes_written >= MAX_DATA_SIZE) {
                     LOG_ERR("snprintf error or buffer too small");
                     // Handle error, maybe free buffer
                     k_free(buf);
                     continue;
                }
                buf->len = bytes_written;
                dataitem_count++;

                /* Put pointer to item in FIFO */
                k_fifo_put(&my_fifo, buf);
            }
            LOG_INF("Producer: Data Items Generated: %u", data_number);
            k_msleep(PRODUCER_SLEEP_TIME_MS); // Assume defined elsewhere (e.g., 2200)
        }
    }
    ```
6.  **Read data items from the FIFO** in `consumer_func()`. Replace previous message queue logic:
    ```c
    // STEP 6: Modify consumer to use FIFO
    void consumer_func(void) // Renamed for clarity
    {
        LOG_INF("Consumer Thread started");
        while (1) {
            struct data_item_t *rec_item;
            /* Get pointer to item from FIFO (waits if empty) */
            rec_item = k_fifo_get(&my_fifo, K_FOREVER);
            if (rec_item != NULL) {
                 LOG_INF("Consumer: %s\tSize: %u", rec_item->data, rec_item->len);
                 /* CRITICAL: Free the memory allocated by producer */
                 k_free(rec_item);
            }
            // No sleep needed here, k_fifo_get handles waiting
        }
    }
    ```
7.  Build and flash.
8.  Connect serial terminal and examine output. Observe variable numbers of items logged per producer cycle. Sample:
    ```terminal
    [timestamp] <inf> Less1_Exer2: Producer: Data Items Generated: 4
    [timestamp] <inf> Less1_Exer2: Consumer: Data Seq. 742:  1266499320      Size: 25
    [timestamp] <inf> Less1_Exer2: Consumer: Data Seq. 743:  4061392639      Size: 25
    [timestamp] <inf> Less1_Exer2: Consumer: Data Seq. 744:  1452774199      Size: 25
    [timestamp] <inf> Less1_Exer2: Consumer: Data Seq. 745:  721881686       Size: 24
    [timestamp] <inf> Less1_Exer2: Producer: Data Items Generated: 11
    [timestamp] <inf> Less1_Exer2: Consumer: Data Seq. 746:  1090030288      Size: 25
    ...
    ```

#### Exploring kernel options

9.  **Open the nRF Kconfig GUI.**
    > **Important**: For SDK v2.8.0+ (including v3.0.0+), select the application image in the APPLICATIONS view *before* opening the Kconfig GUI.
    Navigate to `Sub Systems and OS Services -> Kernel -> General Kernel Options`.

    9.1. **Examine top options:**
    *   `CONFIG_MULTITHREADING`: Enabled by default.
    *   Priority levels (`CONFIG_NUM_PREEMPT_PRIORITIES`, `CONFIG_NUM_COOP_PRIORITIES`).
    *   Scheduling algorithms (EDF, MetaIRQ enable/disable).
    *   Stack sizes (`CONFIG_MAIN_STACK_SIZE`, `CONFIG_ISR_STACK_SIZE`, `CONFIG_IDLE_STACK_SIZE`).

    9.2. **Scroll down:**
    *   **Scheduler priority queue algorithm:** Default `Simple linked-list` is good for few runnable threads.
    *   **Wait queue priority algorithm:** Default `Simple linked-list` good for few threads blocked per primitive. (Usually leave defaults).

    9.3. **Scroll down:**
    *   **Kernel Debugging and Metrics:**
        *   `CONFIG_INIT_STACKS`: Initialize stacks with 0xaa for manual high water mark checking (nRF Debug view does this automatically).
        *   `CONFIG_BOOT_BANNER`: Prints "*** Booting..." banner.
        *   `CONFIG_BOOT_DELAY`: Adds delay before application init.
        *   `CONFIG_THREAD_MONITOR`: Needed for nRF Debug thread view.
        *   `CONFIG_THREAD_NAME`: Allows naming threads. (Both enabled by `CONFIG_DEBUG_THREAD_INFO`).
        *   `CONFIG_THREAD_RUNTIME_STATS`: Collects stats (not used here).
    *   **Work Queue Options:**
        *   `CONFIG_SYSTEM_WORKQUEUE_STACK_SIZE`: Stack size for system workqueue thread.
        *   `CONFIG_SYSTEM_WORKQUEUE_PRIORITY`: Priority of system workqueue thread (default -1, cooperative).
        *   `CONFIG_SYSTEM_WORKQUEUE_NO_YIELD`: Disable yielding between system work items (use with caution).

    9.4. **Scroll down:**
    *   **Timer API Options:**
        *   `CONFIG_TIMESLICING` options (covered before).
        *   `CONFIG_POOL`: Enables Polling API (`k_poll`).
    *   **Other Kernel Object Options:**
        *   `CONFIG_EVENTS`: Enable `k_event` objects.
        *   `CONFIG_HEAP_MEM_POOL_SIZE`: Heap size (configured earlier).

    9.5. **Scroll down (End):**
    *   System clock frequencies (`CONFIG_SYS_CLOCK_TICKS_PER_SEC`, `CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC`) - usually architecture defaults.
    *   **Security:** `CONFIG_STACK_CANARIES` (adds runtime stack overflow checks, impacts performance/size, good for debugging).
    *   `CONFIG_TICKLESS_KERNEL`: Enabled by default.

### Summary Table

This table summarizes the execution primitives discussed:

| Primitive          | Features                                                                                                | Intended Use                                                                  | Level of Determinism | Example Usage                                                                                                 |
| :----------------- | :------------------------------------------------------------------------------------------------------ | :---------------------------------------------------------------------------- | :------------------- | :------------------------------------------------------------------------------------------------------------ |
| Preemptible thread | Isolated unit, own stack.                                                                               | Application code.                                                             | High                 | Most examples in this course, Logger deferred thread.                                                         |
| Cooperative thread | Can't be preempted by threads*. Enforces scheduler locking.                                             | Subsystems, network stacks, drivers, performance-critical app code.           | High                 | System workqueue, MPSL timeslot sample, BT HCI drivers.                                                       |
| System workqueue   | Lightweight (shared stack). Kernel-created. FIFO order. Yields between items (default).                 | Subsystems, drivers, Deferring ISR work, lightweight non-blocking app code.   | Moderate             | BT stack, MCUmgr, Deferring in bh1749 driver, Central HID sample.                                             |
| User workqueue     | Lightweight (shared stack). User-created. Not shared with subsystems (more deterministic than system q). | Application code, Subsystems creating private queues, Deferring ISR work.     | Above moderate       | Fundamentals L7E3, Wi-Fi Provisioning sample, MQTT sample, Network TCP stack.                               |
| Meta-IRQ           | Special cooperative thread. Can preempt even cooperative threads*.                                      | Driver "bottom half" urgent work.                                             | Very high            | Bluetooth LE stack Link Layer.                                                                                |
| Regular ISR        | Asynchronous hardware trigger.                                                                          | Device drivers, High-determinism non-blocking app code.                       | Extremely high       | Most drivers (e.g., nRF700X Wi-Fi).                                                                         |
| Direct ISR         | Asynchronous, lower overhead/latency than regular ISR, reduced kernel feature access.                   | Low-latency drivers.                                                          | Highest              | Special drivers (timers, radios), BT LE Link Layer Controller.                                                |

*\*Meta-IRQ threads can preempt cooperative threads, but execution returns to the interrupted cooperative thread afterward.*