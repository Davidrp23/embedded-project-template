# Multithreaded applications

> **nRF Connect SDK Version:** v3.0.0-v3.2.0

## Overview

nRF Connect SDK uses the Zephyr RTOS, a real-time OS designed for embedded development. Zephyr includes numerous kernel services and other features including threads, that allow for multithreaded application development. In this lesson, we will go through thread concepts and the thread-related services that are provided by the nRF Connect SDK/Zephyr.

In the exercise section, we will cover how to create threads with different priorities and learn about the scheduler's behavior through features like time slicing and workqueues.

---

## Bare-metal vs RTOS programming

### Bare-metal application

A bare-metal application, at its core, is just a big loop in the main function right after you have initialized the hardware/software at the device powerup/reset routines. All the execution is sequential logic, in other words, all instructions are executed in sequence unless interrupted by an interrupt service routine (ISR). So the only non-sequential logic you have in bare-metal programming makes use of exceptions.

While bare-metal programming is often associated with greater power efficiency, reduced memory usage, and potentially faster performance, this isn't always the case. For simple applications, employing a single sequential logic loop can suffice, benefiting from the inherent power efficiency and memory conservation of bare-metal programs. However, as application complexity grows, maintaining firmware architecture solely through sequential logic can become challenging, unscalable, and unportable. This is where the utilization of a real-time operating system (RTOS) proves advantageous.

### RTOS-based application

Designing your application on top of an operating system allows you to have multiple concurrent logics in your application running in different execution units called **threads**, making your architecture simple, as opposed to just one sequential logic running in your main function in standalone mode.

The core of an RTOS is called **the kernel** and controls everything in the system. The other big added advantage is the huge resources of libraries, drivers, and protocol stacks that are natively available by an RTOS like Zephyr.

**Interrupt Service Routines (ISRs)** are available in both RTOS-based applications and bare-metal applications. They are generated asynchronously by the different devices drivers configured (including callback functions) and protocols stacks.

*(Diagram: Visualization of bare-metal (left) and RTOS-based (right) application)*

> **Note**: Having the `main()` function is optional in Zephyr RTOS-based applications. This is because the main thread automatically spawned by the RTOS will do the necessary RTOS initialization, including scheduler/kernel setup, and core drivers setup. After that, it will try to call the user-defined `main()`, if one exists. If no `main()` function exists, the main thread will exit. The system will still be functional and other threads can be scheduled normally.

---

## Zephyr RTOS basics

In this unit, we will talk about the execution units within nRF Connect SDK, namely threads, ISRs and we will also talk about the scheduler and its default behavior.

### Threads

A **thread** is the smallest logical unit of execution for the RTOS scheduler (covered later in this topic) that is competing for the CPU time.

In nRF Connect SDK, there are two main types of threads: cooperative threads (negative priority value) and preemptible threads (non-negative priority). Cooperative threads have negative priority and have very limited usage. Therefore, they are not within the scope of this course.

A thread can be in one of the following states at any given time:

*   **Running:** The running thread is the one that is currently being executed by the CPU. This means that the scheduler of the RTOS has already selected this thread as the one that should get the CPU time and loaded this thread's context into the CPU registers.
*   **Runnable (Ready):** A thread is marked as "Runnable" when it has no other dependencies with other threads or other system resources to proceed further in execution. The only resource this thread is waiting for is the CPU time. The scheduler includes these threads in the scheduling algorithm that selects the next thread to run after the current running thread changes its state.
*   **Non-runnable (Unready):** A thread that has one or more factors that prevent its execution is deemed to be unready, and cannot be selected as the current thread. This can, for example, be because they are waiting for some resource that is not yet available or they have been terminated or suspended. The scheduler does not include these threads in the scheduling algorithm to select the next thread to run.

#### System threads

A **system thread** is a type of thread that is spawned automatically by Zephyr RTOS during initialization. There are always two threads spawned by default, the **main thread** and the **idle thread**.

*   The **main thread** executes the necessary RTOS initializations and calls the application's `main()` function, if it exists. If no user-defined `main()` is supplied, the main thread will exit normally, though the system would be fully functional.
*   The **idle thread** runs when there is no other work to do, either running an empty loop or, if supported, will activate power management to save power (this is the case for Nordic devices).

#### User-created threads

In addition to system threads, a user can define their own threads to assign tasks to. For example, a user can create a thread to delegate reading sensor data, another thread to process data, and so on. Threads are assigned a priority, which instructs the scheduler how to allocate CPU time to the thread. We will cover creating user-defined threads in-depth in Exercise 1.

#### Workqueue threads

Another common execution unit in nRF Connect SDK is a **work item**, which is nothing more than a user-defined function that gets called by a dedicated thread called a **[workqueue thread](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/workqueue.html)**.

A workqueue thread is a dedicated thread to process work items that are pulled out of a kernel object called a **workqueue** in a "first in first out" (FIFO) fashion. Each work item has a specified handler function which is called to process the work item. The main use of this is to **offload non-urgent work** from an ISR or a high-priority thread to a lower priority thread.

A system can have multiple workqueue threads; the default one is known as the **system workqueue**, available to any application or kernel code. The thread processing the work items in the system workqueue is a system thread, and you do not need to create and initialize a workqueue if submitting work items to the system workqueue.

*(Diagram: Workflow of a workqueue)*

As you can see in the image description above, the ISR or high priority thread submits work into a workqueue, and the dedicated workqueue thread pulls out a work item in a FIFO order. The thread that pulls work items from the queue always yields after it has processed one work item, so that other equal priority threads are not blocked for a long time.

The advantage of delegating work as a work item instead of a dedicated thread is that since work items all share one stack (the workqueue stack), a work item is lighter than a thread because no separate stack is allocated for it.

We will cover work items and workqueue threads in-depth in Exercise 3.

### Threads Priority

Threads are assigned an integer value to indicate their priority, which can be either negative or non-negative. **Lower numerical values take precedence over higher values**, meaning a thread with priority 4 will be given higher priority than a thread with priority 7. Similarly, a thread with priority -2 will have higher priority than both a thread with priority 4 and a thread with priority 7.

The scheduler distinguishes between two types of threads based on their priority: **cooperative** and **preemptible**.

*   A thread with a **negative priority** is classified as a **cooperative thread**. Once a cooperative thread becomes the current thread, it will remain so until it performs an action that makes it unready (e.g., yields, sleeps, waits for an event).
*   A thread with a **non-negative priority** is classified as a **preemptible thread**. Once a preemptible thread becomes the current thread, it may be replaced (preempted) at any time if a cooperative thread or a preemptible thread of higher or equal priority becomes ready.

The number of non-negative priorities (for preemptible threads) is configurable through the Kconfig symbol `CONFIG_NUM_PREEMPT_PRIORITIES` and is, by default, equal to 15. The main thread has a priority of 0, while the idle thread has a priority of 15 by default.

Similarly, the number of negative priorities (for cooperative threads) is configurable through the Kconfig symbol `CONFIG_NUM_COOP_PRIORITIES` and is, by default, equal to 16. (Cooperative threads are not covered in this fundamentals course).

### Scheduler

Like anything in the physical world, CPU time is a limited resource, and when an application has multiple concurrent logics, it's not guaranteed that there would be enough CPU time for all of them to run concurrently. This is where the **[scheduler](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/scheduling/index.html)** comes in. The scheduler is the part of the RTOS responsible for scheduling which tasks are running (i.e., using CPU time) at any given time. It does this using a [schedule algorithm](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/scheduling/index.html#scheduling_algorithm) to determine which task should be the next to run.

> **Note**: The number of running threads possible at any instant is equal to the number of application cores. For example on the nRF52840, there is one application core, allowing for only one thread to be running at a time.

#### Rescheduling point

Zephyr RTOS is by default a **tickless RTOS**, meaning it is completely event-driven. Instead of having periodic timer interrupts to wake up the scheduler, it is woken based on events known as **rescheduling points**.

A rescheduling point is an instant in time when the scheduler gets called to select the thread to run next. Any time the state of the Ready threads changes, a rescheduling point is triggered. Some examples of rescheduling points are:

*   When a thread calls `k_yield()`, the thread's state is changed from "Running" to "Ready".
*   Unblocking a thread by giving/sending a kernel synchronization object (like a semaphore, mutex, or alert) causes the thread's state to be changed from "Unready" to "Ready".
*   When a receiving thread gets new data from other threads using [data passing](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/data_passing/index.html) kernel objects, the data receiving thread's state is changed from "Waiting" to "Ready".
*   When [time slicing](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/scheduling/index.html#time_slicing) is enabled (covered in Exercise 2) and the thread has run continuously for the maximum time slice time allowed, the thread's state is changed from "Running" to "Ready".

### ISRs

Interrupt Service Routines (ISRs) are generated asynchronously by device drivers and protocol stacks. **They are not scheduled.** This includes callback functions, which are the application extension of ISRs. It is important to remember that ISRs **preempt** the execution of the current thread, allowing the response to occur with very low overhead. Thread execution resumes only once all ISR work has been completed.

Therefore, it is crucial to ensure that ISRs (including callback functions) **do not contain time-consuming work or involve blocking functionalities**, as they will starve all other threads. Work that is time-consuming or involves blocking should be handed off to a thread using work items or other appropriate mechanisms, as we will see in Exercise 3.

---

## Exercise 1: Thread creation and priorities

In this exercise, we will learn how to create and initialize two threads and learn how they can affect one another with their priorities. There are two ways to create a thread in Zephyr: dynamically (at run-time) through [`k_thread_create()`](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/index.html#c.k_thread_create) and statically (at compile time) using the [`K_THREAD_DEFINE()`](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/index.html#c.K_THREAD_DEFINE) macro. The static method is more frequently used.

`K_THREAD_DEFINE()` API signature:

```c
K_THREAD_DEFINE(name, stack_size, entry, p1, p2, p3, prio, options, delay)
```

*   **name** - Name of the thread.
*   **stack_size** - Stack size in bytes.
*   **entry** - Thread entry function.
*   **p1** - 1st entry point parameter.
*   **p2** - 2nd entry point parameter.
*   **p3** - 3rd entry point parameter.
*   **prio** - Thread priority.
*   **options** - Thread options.
*   **delay** - Scheduling delay (in milliseconds), zero for no delay.

### Exercise steps

1.  In the GitHub repository for this course, open the base code for this exercise, found in `l7/l7_e1`.

#### Creating and initializing threads

2.  Define the stack size and scheduling priority for two threads near the top of `src/main.c`:
    ```c
    #define STACKSIZE 1024
    #define THREAD0_PRIORITY 7
    #define THREAD1_PRIORITY 7
    ```
    Even though the threads are simple, we use a stack size of 1024 bytes. Stack sizes should always be a power of two (512, 1024, 2048, etc.).
    > **Note**: In actual application development, you should choose the stack sizes more carefully, to avoid unnecessarily using the stack size. We do not need that here for a simple application.

    We give both threads the same priority initially.

3.  The thread entry functions (`thread0` and `thread1`) are provided but empty. Make them print a simple string in a loop. Add inside each function's `while(1)` loop (adjust the string for `thread1`):
    ```c
    // Inside thread0():
    printk("Hello, I am thread0\n");

    // Inside thread1():
    printk("Hello, I am thread1\n");
    ```
    Since the threads have no dependencies and neither yield nor sleep yet, they will always be in the "Runnable" state, competing for the CPU.

4.  Define the two threads using `K_THREAD_DEFINE()`. Add these lines after the thread entry function definitions:
    ```c
    K_THREAD_DEFINE(thread0_id, STACKSIZE, thread0, NULL, NULL, NULL,
            THREAD0_PRIORITY, 0, 0);
    K_THREAD_DEFINE(thread1_id, STACKSIZE, thread1, NULL, NULL, NULL,
            THREAD1_PRIORITY, 0, 0);
    ```
    *   `thread0_id`, `thread1_id`: Names (thread IDs) for the threads. The name can be anything but is used as the thread ID, so name wisely.
    *   `STACKSIZE`: Stack size defined earlier.
    *   `thread0`, `thread1`: Thread entry functions.
    *   `NULL, NULL, NULL`: Optional arguments passed to the entry functions (none here).
    *   `THREAD0_PRIORITY`, `THREAD1_PRIORITY`: Priorities defined earlier.
    *   `0`: Optional [thread options](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/index.html#thread_options) (0 means default, no special options like `K_ESSENTIAL`).
    *   `0`: Optional scheduling delay (0 means start immediately in ready state).

5.  Build the application and flash it. Observe the serial terminal output:
    ```terminal
    *** Booting nRF Connect SDK ***
    Hello, I am thread0
    Hello, I am thread0
    Hello, I am thread0
    ... (only thread0 prints)
    ```
    Notice only `thread0` runs. Because `thread0` never yields or sleeps (never enters a state that triggers a rescheduling point where it might lose the CPU), it starves `thread1`.

#### Thread yielding

To avoid starvation, we can make `thread0` voluntarily yield using [`k_yield()`](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/index.html#c.k_yield).

`k_yield()` causes the current thread to relinquish the CPU to another thread of the *same or higher* priority. The yielding thread remains "Runnable" but is moved to the end of the ready queue for its priority level. If no other threads of the same or higher priority are ready, the yielding thread runs again immediately.

> **Note**: To give lower priority threads a chance to run, the current thread needs to be put to "Non-runnable". This can be done using `k_sleep()`, which we will see further on in this exercise.

Let's make `thread0` yield after printing.

6.  Add `k_yield();` inside the `while(1)` loop in `thread0`, after the `printk()` call:
    ```c
    void thread0(void)
    {
        while (1) {
            printk("Hello, I am thread0\n");
            k_yield(); // Add this line
        }
    }
    ```
7.  Build and flash. Observe the output:
    ```terminal
    *** Booting nRF Connect SDK ***
    Hello, I am thread0
    Hello, I am thread1
    Hello, I am thread1
    Hello, I am thread1
    ... (only thread1 prints after the first message from thread0)
    ```
    `thread0` prints once and yields. The scheduler sees `thread1` (same priority, ready) and switches to it. Since `thread1` never yields, it runs forever, now starving `thread0`.

8.  Make `thread1` yield as well. Add `k_yield();` inside the `while(1)` loop in `thread1`, after its `printk()` call:
    ```c
    void thread1(void)
    {
        while (1) {
            printk("Hello, I am thread1\n");
            k_yield(); // Add this line
        }
    }
    ```
9.  Build and flash. Observe the output:
    ```terminal
    *** Booting nRF Connect SDK ***
    Hello, I am thread0
    Hello, I am thread1
    Hello, I am thread0
    Hello, I am thread1
    Hello, I am thread0
    Hello, I am thread1
    ... (threads alternate)
    ```
    *(Diagram: Timeline showing threads alternating execution due to yielding)*

    Now, each thread yields after printing. The scheduler finds the other (equal priority, ready) thread and switches, resulting in alternation. However, frequent yielding invokes the scheduler often, consuming CPU time and power. Better architecture involves designing threads with appropriate priorities and considerate blocking (sleeping/waiting).

#### Thread sleeping

Since printing is non-critical here, sleeping is more efficient than yielding constantly. `k_sleep()` (or derivatives like `k_msleep()`) puts a thread into the "Non-runnable" state for a duration.

> **More on this**: `k_sleep()` takes a `k_timeout_t` (construct with `K_MSEC()`, `K_SECONDS()`, etc.). Simpler derivatives like `k_msleep()` and `k_usleep()` take integer time units directly.

10. Replace `k_yield()` in both threads with `k_msleep(5);` (sleep for 5 milliseconds):
    ```c
    void thread0(void)
    {
        while (1) {
            printk("Hello, I am thread0\n");
            k_msleep(5); // Replaced k_yield()
        }
    }

    void thread1(void)
    {
        while (1) {
            printk("Hello, I am thread1\n");
            k_msleep(5); // Replaced k_yield()
        }
    }
    ```
11. Build and flash. The output looks the same as step 9 (alternating prints), but the underlying behavior is different:
    ```terminal
    *** Booting nRF Connect SDK ***
    Hello, I am thread0
    Hello, I am thread1
    Hello, I am thread0
    Hello, I am thread1
    ... (threads alternate)
    ```
    *(Diagram: Timeline showing threads sleeping, allowing idle thread to run)*

    When both threads sleep and no other threads are ready, the scheduler runs the **idle thread**, allowing the system to enter low-power states between thread executions.

**Conclusion:**

*   `k_yield()`: Changes state from Running -> Runnable. Thread remains eligible to run immediately if no equal/higher priority threads are ready. Good for cooperative sharing among equal priorities.
*   `k_sleep()`: Changes state from Running -> Non-runnable (for duration) -> Runnable. Thread is ineligible to run during sleep. Better for introducing delays and allowing lower-priority or idle thread execution.

The solution for this exercise can be found in the GitHub repository, `l7/l7_e1_sol`.

---

## Exercise 2: Time slicing

If you don't want to manually manage yielding between equal priority threads, you can enable **time slicing**. The scheduler will automatically preempt a running thread after a configured time slice, allowing other threads of the *same priority* to run.

In this exercise, we will observe how time slicing affects thread interaction.

### Exercise steps

1.  In the GitHub repository, open the base code for this exercise: `l7/l7_e2`.
2.  Observe `main.c`. The threads neither sleep nor yield but use `k_busy_wait()`:
    ```c
    void thread0(void)
    {
        while (1) {
                printk("Hello, I am thread0\n");
                k_busy_wait(1000000); // Busy wait for 1 second
        }
    }

    void thread1(void)
    {
        while (1) {
                printk("Hello, I am thread1\n");
                k_busy_wait(1000000); // Busy wait for 1 second
        }
    }
    ```
    Without time slicing, the first thread to run would block the other indefinitely. `k_busy_wait()` executes a do-nothing loop for a specified time (microseconds), keeping the CPU busy. **It is intended for debugging only, not production code.**

3.  Enable time slicing in `prj.conf`:
    ```kconfig
    # STEP 3: Enable Time Slicing
    CONFIG_TIMESLICING=y
    CONFIG_TIMESLICE_SIZE=10 # milliseconds
    CONFIG_TIMESLICE_PRIORITY=0 # Max priority level affected by time slicing
    ```
    *   `CONFIG_TIMESLICING=y`: Enables the feature.
    *   `CONFIG_TIMESLICE_SIZE=10`: Sets the maximum time slice to 10 ms.
    *   `CONFIG_TIMESLICE_PRIORITY=0`: Sets the *highest* priority level where time slicing applies. Threads with priority 0 or numerically higher (lower logical priority, e.g., 1, 2,... 7) will be time-sliced if multiple threads exist at that level. Threads with priority numerically lower than 0 (e.g., -1, -2) or higher logical priority are not time-sliced. Setting it to 0 ensures our priority 7 threads (from Exercise 1, default here is likely similar) are affected. **Remember: Time slicing only affects threads of the *same* priority.**

4.  Build and flash the application. Observe the output:
    ```terminal
    *** Booting nRF Connect SDK ***
    Hello, I am thread0
    Hello, I am thread1
    Hello, I am thread0
    Hello, I am thread1
    ... (output alternates, possibly with interrupted printk messages)
    Hello, I amHello, I am  thread0
    thread1
    ```
    The scheduler preempts the running thread after ~10 ms, even mid-`printk()`. The other thread resumes. Because `printk` itself isn't atomic with respect to time slicing preemption, you might see interleaved output.

    *(Diagram: Timeline showing preemptive time slicing)*

    The scheduler forces context switches between the equal-priority threads every 10 ms.

5.  Now, let's see what happens with different priorities. Change `THREAD0_PRIORITY` to 6 in `main.c`, making it higher priority than `thread1` (priority 7):
    ```c
    #define THREAD0_PRIORITY 6
    #define THREAD1_PRIORITY 7
    ```
6.  Build and flash. Observe the output:
    ```terminal
    *** Booting nRF Connect SDK ***
    Hello, I am thread0
    Hello, I am thread0
    Hello, I am thread0
    ... (only thread0 prints)
    ```
    `thread0` runs uninterrupted, starving `thread1`. Even though time slicing is enabled and configured to potentially affect priority 6, the scheduler preempts `thread0` every 10 ms *only to check for other ready threads of priority 6 or higher*. Since `thread1` has lower priority (7), `thread0` immediately gets the CPU back and runs forever. **Time slicing does not grant CPU time to lower-priority threads.**

The solution for this exercise can be found in the GitHub repository, `l7/l7_e2_sol`.

---

## Exercise 3: Workqueue creation and work item submission

Since higher priority threads can starve lower priority ones, it's good practice to **offload non-urgent work** from high-priority threads to lower-priority execution contexts, such as workqueues.

In this exercise, we will create a workqueue and submit work items to it from a higher-priority thread.

### Exercise steps

1.  In the GitHub repository, open the base code for this exercise: `l7/l7_e3`.

#### Threads with different priorities

2.  Define priorities for three threads near the top of `main.c`. `thread0` will be high priority, `thread1` medium, and the workqueue thread low priority:
    ```c
    #define THREAD0_PRIORITY 2
    #define THREAD1_PRIORITY 3
    #define WORKQ_PRIORITY   4
    ```
3.  Examine `thread0` (provided in the base code). It measures and prints the duration of `emulate_work()` and sleeps for 20 ms in a loop:
    ```c
    void thread0(void)
    {
        uint64_t time_stamp;
        int64_t delta_time;

        while (1) {
            time_stamp = k_uptime_get();
            emulate_work(); // Simulate work directly in the high-priority thread
            delta_time = k_uptime_delta(&time_stamp);

            printk("thread0 yielding this round in %lld ms\n", delta_time);
            k_msleep(20);
        }
    }
    ```
4.  Make `thread1` do the same thing. Add the following code for `thread1`:
    ```c
    void thread1(void)
    {
        uint64_t time_stamp;
        int64_t delta_time;

        while (1) {
            time_stamp = k_uptime_get();
            emulate_work(); // Simulate work directly in the medium-priority thread
            delta_time = k_uptime_delta(&time_stamp);

            printk("thread1 yielding this round in %lld ms\n", delta_time);
            k_msleep(20);
        }
    }
    ```
    `thread1` will be frequently preempted by the higher-priority `thread0`.

5.  Define an inline function `emulate_work()` before the thread functions:
    ```c
    static inline void emulate_work(void)
    {
        // Loop to consume CPU time
        for(volatile int count_out = 0; count_out < 300000; count_out ++);
    }
    ```
    This function takes roughly ~50 ms on an nRF52840 @ 64 MHz (timing may vary based on SoC/clock speed).

6.  Build and flash. Observe the output (times may vary based on SoC/clock speed):
    ```terminal
    *** Booting nRF Connect SDK ***
    *** Using Zephyr OS ***
    thread0 yielding this round in 50 ms
    thread0 yielding this round in 50 ms
    thread1 yielding this round in 100 ms // Takes much longer due to preemption
    thread0 yielding this round in 50 ms
    thread0 yielding this round in 50 ms
    thread1 yielding this round in 100 ms
    ...
    ```
    `thread0` (higher priority) completes its work quickly. `thread1` takes significantly longer because `thread0` preempts it frequently while `thread0` runs `emulate_work()`.

    *(Diagram: Timeline showing thread1 being preempted by thread0)*

#### Offloading work from high priority task

Since `emulate_work()` represents non-urgent processing for `thread0`, let's offload it to a lower-priority workqueue thread to avoid blocking `thread1` unnecessarily.

7.  Define a structure to hold work item information and the work handler function *before* `main()`:
    ```c
    // Structure to pass work item info (can be extended)
    struct work_info {
        struct k_work work; // Kernel work item structure MUST be first member
        char name[25];      // Example: data associated with the work
    } my_work; // Global instance of our work item

    // Work handler function - executes the offloaded work
    void offload_function(struct k_work *work_term)
    {
        emulate_work(); // Perform the actual work
    }
    ```
8.  In `main()`, before defining threads, define the workqueue stack and initialize the workqueue and work item. Add these lines (assuming `STACKSIZE` is defined):
    ```c
    // Define stack area for the workqueue's thread
    K_THREAD_STACK_DEFINE(my_stack_area, STACKSIZE);
    // Define the workqueue itself
    struct k_work_q offload_work_q;

    // Inside main(), initialize the workqueue
    k_work_queue_init(&offload_work_q);

    // Start the workqueue thread
    k_work_queue_start(&offload_work_q, my_stack_area,
                       K_THREAD_STACK_SIZEOF(my_stack_area), WORKQ_PRIORITY,
                       NULL); // No specific options

    // Initialize the work item, linking it to the handler function
    strcpy(my_work.name, "Thread0 emulate_work()"); // Example: set associated data
    k_work_init(&my_work.work, offload_function);
    ```
9.  Modify `thread0` to submit the work item instead of calling `emulate_work()` directly. Replace the `emulate_work();` line inside `thread0`'s `while` loop with:
    ```c
    // Inside thread0's while loop:
    // time_stamp = k_uptime_get(); // Keep timing if desired
    // emulate_work(); // REMOVE THIS LINE
    k_work_submit_to_queue(&offload_work_q, &my_work.work); // Submit work item
    // delta_time = k_uptime_delta(&time_stamp); // Timing now measures submission overhead
    ```
    `thread0` now quickly submits the work and goes back to sleep, allowing `thread1` more CPU time. The actual `emulate_work()` runs later in the lower-priority workqueue thread.

10. Build and flash. Observe the output:
    ```terminal
    *** Booting nRF Connect SDK ***
    *** Using Zephyr OS ***
    thread0 yielding this round in 0 ms // Very fast, just submits work
    thread0 yielding this round in 0 ms
    thread1 yielding this round in 52 ms // Much faster now!
    thread0 yielding this round in 0 ms
    thread0 yielding this round in 0 ms
    thread1 yielding this round in 52 ms
    ...
    ```

    *(Diagram: Timeline showing thread0 quickly submitting work, thread1 running more, and workqueue thread executing later)*

    `thread0` now completes its loop very quickly (<1 ms) because it only submits the work item. This gives `thread1` significantly more time to execute `emulate_work()` without being preempted as often by `thread0`. The actual `emulate_work()` initiated by `thread0` runs in the `offload_work_q` thread at `WORKQ_PRIORITY` (lower priority) when both `thread0` and `thread1` are sleeping. This demonstrates better resource sharing by offloading non-critical work.

The solution for this exercise can be found in the GitHub repository, `l7/l7_e3_sol`.

---

## Additional Resources

- [Zephyr Threads Documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/index.html)
- [Zephyr Scheduling Documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/scheduling/index.html)
- [Zephyr Workqueue Threads](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/threads/workqueue.html)
