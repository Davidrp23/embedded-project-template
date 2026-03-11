# Thread synchronization

> **nRF Connect SDK Version:** v3.0.0 - v3.2.0
>
> **Documentation:** [Zephyr Semaphores](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/synchronization/semaphores.html) | [Zephyr Mutexes](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/synchronization/mutexes.html)

## Overview

In multithreaded applications, the need for thread synchronization arises when multiple threads are running concurrently. In this lesson, we will explain the need for thread synchronization and how to use semaphores and mutexes as thread synchronization mechanisms.

In the exercise section, we will highlight two common thread synchronization problems and show how to solve them using semaphores and mutexes.

### Learning Objectives

After completing this lesson, you will be able to:
- Understand the need for thread synchronization mechanisms
- Learn the basic properties of semaphores and mutexes
- Practice using semaphores and mutexes for thread synchronization through hands-on exercises

### Thread synchronization

In a multithreaded application, there are multiple threads running concurrently. If more than one thread tries to access the same piece of code simultaneously, usually referred to as the **critical section**, this can lead to unexpected or erroneous behavior. This is where the need for **thread synchronization** arises; it’s a mechanism to ensure that only one thread executes the critical section at any given time.

Two mechanisms you can utilize to achieve thread synchronization are **semaphores** or **mutexes**. They have some differing properties, but in essence they are both variables that are changed before and after the critical section by a thread to make sure that no other threads can execute the segment before that thread has completed it. The main differences are that semaphores have a maximum count value that is set at initialization, while mutexes have an ownership property (i.e., only the thread that locks it can unlock it).

In the following sections, we will discuss in more detail about both semaphores and mutexes, including a list of properties and a visualization description. There are also two exercises to be completed, practicing how to use semaphores and mutexes as thread synchronization mechanisms.

### Semaphores

In its simplest form, a **semaphore** is a kernel object that implements a traditional counting semaphore. Semaphores can be seen as a resource-sharing mechanism, where you have a finite instance of a resource that you want to manage access for multiple threads. They are a signaling mechanism used to control access to a given number of instances of a resource.

A semaphore has two key attributes:
- **Count**: Indicates the number of times the semaphore can be taken. A count of zero indicates that the semaphore is unavailable.
- **Limit**: Indicates the maximum value the semaphore's count can reach.

Semaphores have the following properties:

*   At initialization, you set an initial count (greater or equal to 0) and a maximum limit.
*   **"Give"** (`k_sem_give()`): Increments the semaphore count unless the count is already at the maximum limit. "Give" can be done from any thread or ISR.
*   **"Take"** (`k_sem_take()`): Decrements the semaphore count unless the semaphore is unavailable (count at zero). Any thread trying to take an unavailable semaphore needs to wait (block) until another thread makes it available (by giving the semaphore). "Take" can usually only block within threads, not ISRs (ISRs can take with `K_NO_WAIT` but cannot block).
*   **No ownership:** A semaphore can be taken by one thread and given by any other thread or ISR.
*   **No priority inheritance:** The thread taking the semaphore is not eligible for priority inheritance because any thread can give the semaphore.

#### Defining and Initializing a Semaphore

A semaphore can be defined and initialized at compile-time using `K_SEM_DEFINE`:

```c
K_SEM_DEFINE(my_sem, initial_count, count_limit);
```

Or at runtime using `k_sem_init()`:

```c
struct k_sem my_sem;
k_sem_init(&my_sem, initial_count, count_limit);
```

#### Taking a Semaphore with Timeout

```c
if (k_sem_take(&my_sem, K_MSEC(50)) != 0) {
    printk("Semaphore not available within 50 ms!");
}
```

> **Note**:
> *   You may initialize a "full" semaphore (count equal to limit) to limit the number of threads able to execute the critical section concurrently.
> *   You may also initialize an "empty semaphore" (count equal to 0, limit equal to 1) to create a gate where no waiting thread may pass until the semaphore is given first.
> *   When the semaphore is given and multiple threads are waiting, it is taken by the highest priority thread that has waited longest.

### Mutexes

As opposed to semaphores, **mutexes** (Mutual Exclusion locks) are kernel objects that implement traditional reentrant mutexes. A mutex allows multiple threads to safely share an associated hardware or software resource by ensuring mutually exclusive access to the resource. Mutexes can only take two logical states: **locked** or **unlocked**. Additionally, mutexes have **ownership properties**: only the thread that locks (acquires) the mutex can unlock (release) it. Think of it as a locking/unlocking mechanism with a single key for a single resource (like a critical section or shared data structure).

A mutex has two key attributes:
- **Lock count**: Tracks how many times the owning thread has locked it (zero = unlocked)
- **Owning thread**: Identifies which thread currently holds the lock

A thread wishing to access the protected resource must first acquire the unlocked mutex. If the mutex is already locked by another thread, the requesting thread blocks until the owner unlocks it. When the mutex becomes unlocked, it is then locked by the highest-priority thread that has waited the longest.

A typical use of a mutex is to protect a critical section of code accessed by multiple threads, preventing race conditions where shared data could be corrupted if accessed concurrently.

Mutexes have the following properties:

*   **Locking** (`k_mutex_lock()`): A thread acquires the mutex if unlocked. If already locked by another thread, the requesting thread blocks. Recursive locking (the owner locking again) is allowed and increments an internal lock count.
*   **Unlocking** (`k_mutex_unlock()`): Decrements the internal lock count. When the count reaches zero, the mutex becomes unlocked and available for other threads to acquire.
*   **Ownership:** Only the thread that locked the mutex can unlock it.
*   **Context:** Mutex locking and unlocking can only be done from threads, **not** ISRs (due to ownership and potential blocking). Mutexes are not designed for use by ISRs.
*   **Priority Inheritance:** The thread locking the mutex *is* eligible for priority inheritance. If a higher-priority thread blocks waiting for the mutex, the kernel temporarily elevates the priority of the mutex-holding thread to help it finish faster and release the mutex, preventing priority inversion.

#### Defining and Initializing a Mutex

A mutex can be defined and initialized at compile-time using `K_MUTEX_DEFINE`:

```c
K_MUTEX_DEFINE(my_mutex);
```

Or at runtime using `k_mutex_init()`:

```c
struct k_mutex my_mutex;
k_mutex_init(&my_mutex);
```

#### Locking a Mutex with Timeout

```c
k_mutex_lock(&my_mutex, K_FOREVER);   // Wait indefinitely
k_mutex_lock(&my_mutex, K_MSEC(100)); // Wait up to 100 ms
```

#### Unlocking a Mutex

```c
k_mutex_unlock(&my_mutex);
```

> **Note**: Zephyr also provides related primitives:
> - **k_futex**: A lightweight alternative that minimizes kernel involvement
> - **sys_mutex**: A user-mode compatible variant functioning in user memory

---

## Exercise 1: Semaphores

Let's see the use of semaphores in action. In this exercise, we will initialize a semaphore that will have the count-property reflect the number of instances of a particular resource in the system. There will be one **consumer thread** that will want to consume the resource and one **producer thread** that will produce it.

The exercise demonstrates a classic producer-consumer synchronization problem where the consumer thread attempts to take resources faster than the producer can supply them, leading to a race condition that we will fix using a semaphore.

### Exercise steps

1.  In the GitHub repository for this course, open the base code for this exercise, found in `l8/l8_e1` (select your SDK version subfolder, e.g., `v3.0.0`). Open it as an existing application in VS Code.
2.  Set the thread priorities in `src/main.c`. Make the consumer higher priority to demonstrate contention:
    ```c
    #define PRODUCER_PRIORITY        5
    #define CONSUMER_PRIORITY        4
    ```
3.  Define a variable to track the available resource instances (initially 10):
    ```c
    volatile uint32_t available_instance_count = 10;
    ```
4.  Create the producer thread function. It releases the resource (increments count) and sleeps randomly:
    ```c
    void producer(void)
    {
        printk("Producer thread started\n");
        while (1) {
            release_access(); // Function defined later
            // Assume the resource instance access is released at this point
            k_msleep(500 + sys_rand32_get() % 10); // Sleep 500-509 ms
        }
    }
    ```
5.  Create the consumer thread function. It gets the resource (decrements count) and sleeps randomly:
    ```c
    void consumer(void)
    {
        printk("Consumer thread started\n");
        while (1) {
            get_access(); // Function defined later
            // Assume the resource instance is acquired and being used here
            k_msleep(sys_rand32_get() % 10); // Sleep 0-9 ms
        }
    }
    ```
6.  Define the `get_access()` and `release_access()` functions (before the thread functions):
    6.1 In `get_access()`, decrement the count and print:
    ```c
    void get_access(void)
    {
        available_instance_count--;
        printk("Resource taken and available_instance_count = %d\n", available_instance_count);
    }
    ```
    6.2 In `release_access()`, increment the count and print:
    ```c
    void release_access(void)
    {
        available_instance_count++;
        printk("Resource given and available_instance_count = %d\n", available_instance_count);
    }
    ```
    Currently, there are no checks. If `available_instance_count` goes below 0 or above 10, it indicates a race condition.

7.  Define the threads using `K_THREAD_DEFINE` after the function definitions:
    ```c
    K_THREAD_DEFINE(producer_id, 1024, producer, NULL, NULL, NULL,
                    PRODUCER_PRIORITY, 0, 0);
    K_THREAD_DEFINE(consumer_id, 1024, consumer, NULL, NULL, NULL,
                    CONSUMER_PRIORITY, 0, 0);
    ```
8.  Build and flash. Observe the terminal output. You will likely see negative counts, indicating the race condition:
    ```terminal
    *** Booting nRF Connect SDK ... ***
    Consumer thread started
    Resource taken and available_instance_count = 9
    Producer thread started
    Resource taken and available_instance_count = 8
    Resource taken and available_instance_count = 7
    ...
    Resource taken and available_instance_count = 0
    Resource taken and available_instance_count = -1 // ERROR!
    Resource taken and available_instance_count = -2 // ERROR!
    ... (counts become increasingly negative)
    Resource given and available_instance_count = -67 // Producer eventually adds back
    ...
    ```
    The higher-priority consumer runs more often, decrementing the count below zero because there's no mechanism preventing it from taking a non-existent resource.

9.  **Fix the race condition using a semaphore.** Define a semaphore near the top of the file (after includes, before variable definitions) with an initial count of 10 (matching available resources) and a limit of 10:
    ```c
    // STEP 9: Define semaphore
    K_SEM_DEFINE(instance_monitor_sem, 10, 10); // initial_count=10, count_limit=10
    ```
10. Modify the access functions to use the semaphore:
    10.1 In `get_access()`, **take** the semaphore *before* decrementing the count. `K_FOREVER` means wait indefinitely if unavailable:
    ```c
    void get_access(void)
    {
        // STEP 10.1: Take semaphore
        k_sem_take(&instance_monitor_sem, K_FOREVER);
        available_instance_count--;
        printk("Resource taken and available_instance_count = %d\n", available_instance_count);
    }
    ```
    10.2 In `release_access()`, **give** the semaphore *after* incrementing the count:
    ```c
    void release_access(void)
    {
        available_instance_count++;
        printk("Resource given and available_instance_count = %d\n", available_instance_count);
        // STEP 10.2: Give semaphore
        k_sem_give(&instance_monitor_sem);
    }
    ```
    Now, the consumer thread will block on `k_sem_take` if the semaphore count (representing available resources) is 0, waiting until the producer calls `k_sem_give`.

11. Build and flash again. Observe the output:
    ```terminal
    *** Booting nRF Connect SDK ... ***
    Consumer thread started
    Resource taken and available_instance_count = 9
    Producer thread started
    Resource taken and available_instance_count = 8
    ...
    Resource taken and available_instance_count = 1
    Resource taken and available_instance_count = 0
    // Consumer now blocks here, waiting for producer
    Resource given and available_instance_count = 1 // Producer runs, gives semaphore
    Resource taken and available_instance_count = 0 // Consumer runs, takes semaphore
    Resource given and available_instance_count = 1
    Resource taken and available_instance_count = 0
    ... (count stays between 0 and 10)
    ```
    The `available_instance_count` now correctly stays between 0 and 10. The semaphore successfully synchronizes access to the limited resource.

The solution for this exercise can be found in the GitHub repository, `l8/l8_e1_sol` (select your SDK version subfolder, e.g., `v3.0.0`).

---

## Exercise 2: Mutexes

In this exercise, we will create an application with two threads running and accessing the same code section. The logic looks perfect when only one thread is accessing the critical section, but when two different threads try to access the code section simultaneously, unexpected things happen (race condition). We will demonstrate how a mutex can be utilized to synchronize these two threads.

The threads will have the same priority (4), and time slicing will be enabled (10 ms) so they preempt each other. This means each thread will be given 10 ms to finish its task before it is forcefully preempted by the scheduler to allow the other equal priority threads to run, increasing the likelihood of the race condition occurring.

### Exercise steps

1.  In the GitHub repository, open the base code for this exercise: `l8/l8_e2` (select your SDK version subfolder, e.g., `v3.0.0`).
2.  Enable multithreading in `prj.conf` (though often enabled by default):
    ```kconfig
    CONFIG_MULTITHREADING=y
    # Enable time slicing for demonstration
    CONFIG_TIMESLICING=y
    CONFIG_TIMESLICE_SIZE=10
    CONFIG_TIMESLICE_PRIORITY=0
    ```
3.  Set equal priorities for the threads in `src/main.c`:
    ```c
    #define THREAD0_PRIORITY        4
    #define THREAD1_PRIORITY        4
    ```
4.  Define the thread entry functions. Both call `shared_code_section()`. Comment out the call in `thread1` initially:
    ```c
    void thread0(void)
    {
        printk("Thread 0 started\n");
        while (1) {
            shared_code_section();
        }
    }

    void thread1(void)
    {
        printk("Thread 1 started\n");
        while (1) {
            // shared_code_section(); // Initially commented out
        }
    }
    ```
5.  Define two global counters and their expected combined total:
    ```c
    #define COMBINED_TOTAL   40
    int32_t increment_count = 0;
    int32_t decrement_count = COMBINED_TOTAL;
    ```
6.  Define the `shared_code_section()` function (before the thread functions). This function modifies both counters:
    ```c
    void shared_code_section(void)
    {
        increment_count += 1;
        increment_count = increment_count % COMBINED_TOTAL; // Wrap increment counter
        decrement_count -= 1;
        if (decrement_count == 0) {
            decrement_count = COMBINED_TOTAL; // Wrap decrement counter
        }
        // Check invariant AFTER modifications
        check_counters(); // Separate check function called later
    }
    ```
    > **Note**: If implemented correctly without interruption, `increment_count + decrement_count` should *always* equal `COMBINED_TOTAL`. A race condition occurs if a thread switch happens *between* the modification of `increment_count` and `decrement_count`.

7.  Define a `check_counters()` function (called by `shared_code_section`) to print an error if the invariant is broken:
    ```c
    void check_counters(void) // Renamed for clarity
    {
        if (increment_count + decrement_count != COMBINED_TOTAL) {
            printk("Race condition happened!\n");
            printk("Increment_count (%d) + Decrement_count (%d) = %d \n",
                    increment_count, decrement_count, (increment_count + decrement_count));
            // Sleep briefly to avoid flooding console if error persists
            k_msleep(400 + sys_rand32_get() % 10);
        }
    }
    ```
    *(Ensure `shared_code_section` calls `check_counters()` at the end)*
8.  Define the threads using `K_THREAD_DEFINE` (add scheduling delay for terminal connection):
    ```c
    K_THREAD_DEFINE(thread0_id, 1024, thread0, NULL, NULL, NULL,
                    THREAD0_PRIORITY, 0, 5000); // 5 sec delay
    K_THREAD_DEFINE(thread1_id, 1024, thread1, NULL, NULL, NULL,
                    THREAD1_PRIORITY, 0, 5000); // 5 sec delay
    ```
    > **Note**: The 5000 ms startup delay gives time to connect the serial terminal before threads start running.

9.  Build and flash. Observe the output. With only `thread0` calling `shared_code_section()`, no "Race condition" messages should appear:
    ```terminal
    *** Booting nRF Connect SDK ... ***
    Thread 0 started
    Thread 1 started
    (No further output, indicating invariant holds)
    ```
10. **Introduce the race condition.** Uncomment the `shared_code_section();` line in `thread1`:
    ```c
    void thread1(void)
    {
        printk("Thread 1 started\n");
        while (1) {
            shared_code_section(); // Uncomment this line
        }
    }
    ```
11. Build and flash again. Observe the output. You should now see "Race condition" messages frequently:
    ```terminal
    *** Booting nRF Connect SDK ... ***
    Thread 0 started
    Thread 1 started
    Race condition happened!
    Increment_count (6) + Decrement_count (35) = 41
    Race condition happened!
    Increment_count (7) + Decrement_count (34) = 41
    Race condition happened!
    Increment_count (0) + Decrement_count (1) = 1
    ... (various incorrect sums)
    ```
    This happens because time slicing preempts one thread *after* it modifies `increment_count` but *before* it modifies `decrement_count`. The other thread then runs, modifies the counters based on the inconsistent intermediate state, leading to the invariant `sum == COMBINED_TOTAL` being broken when checked.

12. **Fix the race condition using a mutex.**
    12.1 Define a mutex globally (near the top of the file):
    ```c
    // STEP 11: Define mutex
    K_MUTEX_DEFINE(test_mutex);
    ```
    12.2 Modify `shared_code_section()` to lock the mutex *before* accessing the shared counters and unlock it *after* modifications are complete (but *before* the check, or move the check inside):
    ```c
    void shared_code_section(void)
    {
        // STEP 12.1: Lock mutex
        k_mutex_lock(&test_mutex, K_FOREVER);

        // --- Critical Section Start ---
        increment_count += 1;
        increment_count = increment_count % COMBINED_TOTAL;
        decrement_count -= 1;
        if (decrement_count == 0) {
            decrement_count = COMBINED_TOTAL;
        }
        // --- Critical Section End ---

        // STEP 12.2: Unlock mutex
        k_mutex_unlock(&test_mutex);

        // Check can be outside the lock now, as modifications were atomic relative to other threads
        check_counters();
    }
    ```
    Now, only one thread can execute the code between `k_mutex_lock` and `k_mutex_unlock` at a time, ensuring the counter modifications happen atomically with respect to other threads accessing the same critical section.

13. Build and flash. Observe the output. No "Race condition" messages should appear, similar to the output in step 9:
    ```terminal
    *** Booting nRF Connect SDK ... ***
    Thread 0 started
    Thread 1 started
    (No further output)
    ```
    The mutex successfully protects the critical section, ensuring the integrity of the shared variables.

The solution for this exercise can be found in the GitHub repository, `l8/l8_e2_sol` (select your SDK version subfolder, e.g., `v3.0.0`).

---

## Summary

In this lesson, you learned about thread synchronization in multithreaded applications:

- **Critical sections** are pieces of code that need to be protected from concurrent access by multiple threads
- **Semaphores** are counting mechanisms that control access to a finite number of resource instances
  - No ownership - any thread can give/take
  - Can be used from ISRs (with `K_NO_WAIT`)
  - No priority inheritance
- **Mutexes** provide exclusive access to a single resource with ownership semantics
  - Only the owning thread can unlock
  - Cannot be used from ISRs
  - Supports priority inheritance to prevent priority inversion
  - Supports recursive locking

Both mechanisms are essential tools for building robust multithreaded applications on Zephyr RTOS.

## Additional Resources

- [Zephyr Semaphores Documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/synchronization/semaphores.html)
- [Zephyr Mutexes Documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/kernel/services/synchronization/mutexes.html)
- [Zephyr Mutex API Reference](https://docs.zephyrproject.org/apidoc/latest/group__mutex__apis.html)
- [Nordic Developer Academy - nRF Connect SDK Fundamentals](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/)