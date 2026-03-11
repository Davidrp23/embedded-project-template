# Connection process

In the previous lesson, we covered how the advertisement process in Bluetooth LE is a way for the peripheral to make its presence known so it can be discovered by a central, and in the end, establish a connection with it.

## Connecting

Establishing a connection requires two devices, one acting as a peripheral that is currently advertising, and one acting as a central that is currently scanning. When a central device picks up an advertisement packet from a peripheral device, it can initiate a connection. Usually, this involves scanning the contents of the advertisement packet, and then deciding whether or not to initiate a connection based on that. When the central sends a connection request, the peripheral and central have established a bi-directional connection (connection-oriented) channel.

_Figure illustrating a central initiating a connection with an advertising peripheral._

As we can see in the figure above, a peripheral that is sending out advertisements that are connectable will always have a short RX window after every advertisement, which is used to listen for incoming connection requests. We call it a connection request, but in reality, the peripheral can’t choose whether to accept or reject the connection request. It has to always accept the connection request, unless it is using an accept list filter. Later, at any point in time, it may choose to disconnect from the central if it doesn’t want to stay connected.

**More on this:**
Using an accept list filter, formerly known as whitelisting, is a way the peripheral can limit which devices are allowed to send it a connection request. This will be covered in lesson 5.

## During the connection

After the peripheral successfully receives the connection request packet, the two devices are in a connection. When a connection is entered, the devices will no longer use the advertising channels (channel 37, 38 and 39), but start using the data channels (channels 0 to 36). To reduce interference and improve throughput during a connection, Bluetooth LE uses channel hopping, meaning the channel used for data transmission is changed frequently. This way, if they are located in an environment that has a lot of noise on some channels, the messages will be retransmitted on another channel in the next connection interval. To ensure data integrity, all packets transmitted over Bluetooth LE will be retried infinitely until an acknowledgment is received or the connection is terminated.

The nature of a Bluetooth LE connection is a main factor in how the devices achieve such low power consumption. In a connection, both devices spend most of their time sleeping. To accomplish this, they agree on how often they will wake up to talk. Otherwise, they turn off the radio, set a timer and go to sleep. The time they agree on sleeping is known as the connection interval, and is set in the initial connection, while the connection event occurs every connection interval when they wake up to talk.

**Definition:**
**Connection interval:** The interval at which two devices in a connection wake up to exchange data.
**Connection event:** Occurs every connection interval, when the central sends a packet to the peripheral.

The following figure shows what a typical connection looks like. Both the central and the peripheral are waking up every connection interval for the connection events and transmitting data.

_Figure showing a typical connection with central and peripheral waking up for connection events._

The connection interval is initially set by the central in the connection request packet, but it can be changed later in the connection. The two devices can send many packets every connection interval if they need to send a lot of data, but when they stop sending data, they have to wait for the next connection event to send more data. Even if there is no useful data to send, the peers need to send empty packets to sync their clocks. If you want to send more data than there is time for in one connection interval, it will be split over several connection intervals.

## Disconnecting

When two devices are connected, they will remain connected forever if nothing happens. There are two ways that a connection can be terminated, meaning the devices disconnect:

1.  **Disconnected by application**
2.  **Disconnected by supervision timeout**

### Disconnected by application

If either of the devices want to, they can send a termination packet that will disconnect the device. This can, for example, be done because a device no longer wishes to be connected to the other device, but it will also happen if there is something wrong with the connection. E.g. if a device you are connected to is claiming to be a previously connected device, but it is not able to sign the packets with the correct encryption keys. The termination packet will contain a field called “disconnect reason” which will say something about why the devices disconnected. Such as, whether it was the user who wanted to disconnect, or if there was an issue with the stack.

### Disconnected by supervision timeout

The other reason a device may disconnect is if it stops responding to packets. There can be several reasons for this. Either the application on the connected device crashed and reset (which is not too uncommon, especially during the development phase), the connected device ran out of battery, or the connected device was taken out of radio range. The amount of time it takes before the connection times out is set by the connection supervision timeout parameter, which we will discuss in more detail in the next topic.

## Connection parameters

When a peripheral and central device enter into a connection, there is a set of connection parameters that are exchanged. Some of them have a standard start value, for backwards compatibility, while some of them are dictated by the central device, and are included in the connection request packet.

The connection interval and the connection supervision timeout were briefly discussed in the previous topic, and are set by the central in the connection request packet, in addition to the peripheral latency. Peripheral latency allows the peripheral to skip waking up for connection events if it doesn’t have data to send.

The radio mode (1M, 2M or coded PHY) is set to 1M by default for backwards compatibility, but can be changed during the connection. The data length and MTU (Maximum Transfer Unit) are also set for backwards compatibility, but we will take a look at how to change these in the exercise section of this topic.

### Connection interval

A Bluetooth LE device spends most of its time “sleeping” (hence the “Low Energy” in the name). In a connection, this is accomplished by agreeing on a connection interval saying how often the devices will communicate with each other. When they are done communicating, they will turn off the radio, set a timer and go into idle mode, and when the timer times out, they will both wake up and communicate again. The implementation of this is handled by the Bluetooth LE stack, but it is up to your application to decide how often you want the devices to communicate by setting the connection interval.

### Supervision timeout

When two devices are connected, they agree on a parameter that determines how long it should take since the last packet was successfully received until the devices consider the connection lost. This is called the supervision timeout. So if one of the devices is unexpectedly switched off, runs out of battery, or if the devices are out of radio range, then this is the amount of time it takes between successfully receiving the last packet before the connection is considered lost.

### Peripheral latency

Peripheral latency allows the peripheral to skip waking up for a certain number of connection events if it doesn’t have any data to send. Usually, the connection interval is a strict tradeoff between power consumption and low latency or delay in communication. If you want to reduce the latency, but still keep a low power consumption, you can use peripheral latency. This is particularly useful in HID (Human Interface Devices) applications, such as computer mouse and keyboard applications, which usually don’t have any data to send, but when it has data to send, we want to have very low latency. Using the peripheral latency option, we can maintain low latency but reduce power consumption by remaining idle for several connection intervals.

### PHY radio mode

Normal Bluetooth LE (1M PHY) transmits at 1Mbps. However, in Bluetooth 5.0, both high-speed (2M PHY) and long-range (Coded PHY) radio modes were introduced, (as covered in PHY: Radio modes). This gives us two more options.

First, we can increase the modulation scheme to use 2Mbps for higher transmit rates. This either means that you can transfer the data faster, and go back to sleep faster to conserve more power, or you can use that extra time to send even more data, practically doubling the throughput of a Bluetooth LE connection. This does however come with the cost of a slightly shorter range.

The other option is to use Coded PHY which results in a significant increase in range, but at the cost of lower throughput.

### Data length and MTU

The data length and MTU (Maximum Transfer Unit) are two different parameters, but they often go hand in hand.

The MTU is the number of bytes that can be sent in one GATT operation (for example, a send operation), while data length is the number of bytes that can be sent in one Bluetooth LE packet. MTU has a default value of 23 bytes, and data length has a default value of 27 bytes. When MTU is larger than data length, such as MTU being 140 bytes while data length is 27 bytes, the data will be segmented into chunks of the data length’s size. This means that, for your application, it appears like one message is being sent, but on the air, the data is actually split into smaller segments.

Ideally, you want all of your data to be sent in one packet, to reduce the time it takes to send the data, so in Bluetooth 4.2, Data Length Extension (DLE) was introduced to allow the data length to be increased from the default 27 bytes to up to 251 bytes. Packing everything together also reduces the number of bytes you need to transmit over the air, as every packet includes a 3-byte header. This saves both time and power, and in turn allows for higher throughput in your Bluetooth LE connection.

The relation between data length and MTU is not one-to-one. On air, the data length can be up to 251 bytes, while the actual payload that you can send is a maximum of 244 bytes. This is because the 251 byte Data PDU payload needs an L2CAP Header of 4 bytes, and an Attribute header of 3 bytes. This leaves you with 251 – 4 – 3 = 244 bytes that you can actually populate with payload data.

_Relation between MTU and Data Size_

Below is a figure showing what it looks like to send a message with 40 bytes before and after changing the default data length. It is clear that sending all the data in one packet leads to less radio on time.

_Left: 40-byte payload without data length extension._
_Right: 40-byte payload with data length extension._
_Source: Online Power Profiler tool_

### Updating the connection parameters

The connection interval, supervision timeout and peripheral latency are dictated by the central, but the peripheral can request changes. However, it is always the central that has the final say with these requests. So in the case where the central is your phone, it is the OS running on the phone that decides whether to accept or reject the new parameters in the connection parameter request.

As for the PHY radio mode, data length and MTU, these cannot be chosen only by the central. Since the ability to change these parameters was introduced in later releases of the Bluetooth Specification, they are always set to their default values when a connection is first established. When the connection is first established, either device can request to update these parameters with new values. The other device will then either send its supported values or state that it does not support updating one or more of those parameters.

Taking the data length as an example, this is always 27 bytes when the connection is first established. Then let’s say that the peripheral wants to update this to 200 bytes, and sends a request to do so. The central may then reply with a message saying it can do 180 bytes, and then they will agree on having the data length set to 180 bytes.

The default value for the PHY radio mode is 1M, and the default MTU is 23.

## Exercise 1: Connecting to your smartphone

_(Note: This exercise is compatible with nRF Connect SDK versions v3.0.0-v3.2.0.)_

In the last lesson, we started advertising with the Nordic board and then scanned for these advertisements with a phone. You were able to connect to the board via the nRF Connect for Mobile app, but nothing more happened.

In this exercise, we will establish a connection between your Nordic board, as a peripheral, and your smartphone, as a central. Then we will set up some callback functions, to be notified when the connection parameters are changed. Then we will add a temporary service to be able to send data through the established connection.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l3/l3_e1`.

You may notice that we are using the application you ended up with in the previous exercise (lesson 2 exercise 3) as the base code for this exercise.

**1. Include the header file for handling connection events.**

Start by adding the header file needed for handling our Bluetooth Low Energy connections. Add the following close to the top of your `main.c`:

```c
#include <zephyr/bluetooth/conn.h>
```

**2. Set up callbacks to be triggered when a connection has been established or disconnected.**

2.1 Declare a connection callback structure.

Let’s declare a structure called `connection_callbacks` of type `bt_conn_cb`.

This structure is used for tracking the state of the connection. For now, we will use the members `connected` and `disconnected` to track when a new connection has been established and when a connection has been disconnected. Take a look at the API documentation for a full list of all possible events.

> **Note**
> These events are different from the connection events we discussed in the previous topic, which cannot be seen by the application. These events are called connected callback events.

We will call our callbacks `on_connected` and `on_disconnected`.

Add the following lines to `main.c` (ensure `on_connected` and `on_disconnected` are forward-declared or defined before this):

```c
// Forward declarations if defined later
void on_connected(struct bt_conn *conn, uint8_t err);
void on_disconnected(struct bt_conn *conn, uint8_t reason);

struct bt_conn_cb connection_callbacks = {
    .connected              = on_connected,
    .disconnected           = on_disconnected,
};
```

2.2 Define the callback functions `on_connected` and `on_disconnected`.

Because the callbacks are triggered from the Bluetooth libraries, it is important that they have the same function parameters as described in the documentation of the API linked above.

We will start with some simple callback functions that we can build on later. Add the following functions to your `main.c` (ensure `my_conn` is declared, typically as a global `struct bt_conn *my_conn = NULL;` and `LOG_ERR`, `LOG_INF` are available):

```c
// Global or static variable for connection handle
static struct bt_conn *my_conn = NULL;

void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    my_conn = bt_conn_ref(conn);

    /* STEP 3.2  Turn the connection status LED on */
}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(my_conn);
    my_conn = NULL; // Good practice to NULL the pointer

    /* STEP 3.3  Turn the connection status LED off */
}
```

The `my_conn` parameter is just a `bt_conn` pointer that has been declared further up in the `main.c` file. We will use it to keep track of our connection in the next exercise.

2.3 Register the callback structure `connection_callbacks`.

Register the callback structure, using the function, `bt_conn_cb_register`. This needs to be called before we start advertising, to avoid a connection being established before we have registered the callback.

Add the following line in `main.c` (typically in `main()` before `bt_enable` or `bt_le_adv_start`):

```c
bt_conn_cb_register(&connection_callbacks);
```

> **Note**
> Note that in many of the Bluetooth Low Energy samples, you will not see `bt_conn_cb_register()` being used. Instead, they use the macro `BT_CONN_CB_DEFINE()`, which will both define and register these callbacks.

**3. Configure an LED to indicate the current connection status.**

3.1 Define the connection status LED

Add the following line close to the top of your `main.c` (ensure `dk_buttons_and_leds.h` or similar is included for `DK_LED2`):

```c
#include <dk_buttons_and_leds.h> // Or equivalent for your board
#define CONNECTION_STATUS_LED   DK_LED2
```

3.2 In the connected event, turn the connection status LED on.

In the callback function for the connected event, `on_connected()`, add the following line to turn the LED on (ensure `dk_set_led` is available):

```c
dk_set_led(CONNECTION_STATUS_LED, 1);
```

3.3 In the disconnected event, turn the connection status LED off.

In the callback function for the disconnected event, `on_disconnected()`, add the following line to turn the LED off:

```c
dk_set_led(CONNECTION_STATUS_LED, 0);
```

**4. Build and flash the application to your board.**

**5. Open a terminal to see the log output from the application.**

Just like we did in lesson 1, connect to the COM port of your DK in VS Code by expanding your device under Connected Devices and selecting the COM port for the device. The number for the COM port may differ on your PC.

> **Note**
> Most of the newer DKs will have more than one COM port. Try both to see which one belongs to the application core. Usually, but not always, it is the one with the smaller number.

Use the default settings 115200 8n1 rtscrs:off. Then reset the device to see the full log message.

Restart the device to see the full log output. At this point, you should be seeing the following log output:
```
*** Booting nRF Connect SDK vX.Y.Z ***
*** Using Zephyr OS vA.B.C ***
[00:00:00.004,302] <inf> Lesson3_Exercise1: Starting Lesson 3 - Exercise 1
[00:00:00.007,659] <inf> Lesson3_Exercise1: Bluetooth initialized
[00:00:00.008,605] <inf> Lesson3_Exercise1: Advertising successfully started
```

**6. Use your smartphone to scan and connect to your board.**

As we have done in previous exercises, use the nRF Connect for Mobile app the scan for devices. Connect to your device, called “Nordic_Peripheral“, by selecting Connect next to the name. This will also open a new window with a lot of tabs, but we will focus on what happens on the board for now.

Notice that the LED on your board indicates a connection, and you should also be seeing the following log output:
```
[00:00:26.831,720] <inf> Lesson3_Exercise1: Connected
```
This means that we are now in the connected state. The phone, acting as a central, picked up the advertising packets from the nRF device, which is the peripheral in this connection.

> **Important**
> If you are using iOS, you will also see the following warning lines in the log after the connected event, that can be ignored.

**7. Disconnect the device from your smartphone**

Let’s disconnect from the peripheral, by clicking the “Disconnect” button in the upper right-hand corner.

You should see the following log output:
```
[00:00:38.627,004] <inf> Lesson3_Exercise1: Disconnected. Reason 19
```
The error codes are defined in the Bluetooth specification, and can indicate why the connection was terminated.

If you open the file `hci_types.h`, you can see that 19, which is written 0x13 in hexadecimal refers to `BT_HCI_ERR_REMOTE_USER_TERM_CONN`, which means that the remote user terminated the connection. If we do the test again, but instead of disconnecting from the app, we take the phone far away from the device, we should see that the disconnect reason is 8. This is `BT_HCI_ERR_CONN_TIMEOUT`, which means that we just saw a supervision timeout occur.

At this point, we want to be able to send some data between the peripheral and the central. To do this, we need to add a service to our application. We will be adding the LED Button Service to the application.

**More on this:**
We will cover Bluetooth LE services and sending data in more detail in lesson 4. For now, it is only relevant to know that this function sends data to the connected device.

**8. Change the application to send a message whenever button 1 is pressed.**

We want to send some data whenever button 1 is pressed, and to be able to do this we need to add the LED Button Service (LBS).

8.1. Include the LBS in the application.

Start by adding this line to your `prj.conf`.
```kconfig
CONFIG_BT_LBS=y
CONFIG_BT_LBS_POLL_BUTTON=y
```
The first line will set up the service, while the other just says that it is possible to manually read the value. We will not do this directly, but we will later use an app that requires this setting to be set.

8.2 Include the LED Button Service header file near the top of your `main.c`.
```c
#include <bluetooth/services/lbs.h>
```

8.3 Add a callback to be notified when button 1 is pressed.

We want to send the button state of button 1 to the central device (your phone) whenever button 1 on the device is pressed.

Add the `button_changed()` callback in your `main.c` (ensure `USER_BUTTON` is defined, e.g., `DK_BTN1_MSK`):
```c
// Ensure USER_BUTTON is defined, e.g. #define USER_BUTTON DK_BTN1_MSK
static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	int err;
	bool user_button_changed = (has_changed & USER_BUTTON) ? true : false;
	bool user_button_pressed = (button_state & USER_BUTTON) ? true : false;
	if (user_button_changed) {
		LOG_INF("Button %s", (user_button_pressed ? "pressed" : "released"));

		// Ensure my_conn is valid before sending
		if (my_conn) { 
		    err = bt_lbs_send_button_state(user_button_pressed);
		    if (err) {
			    LOG_ERR("Couldn't send notification. (err: %d)", err);
		    }
        } else {
            LOG_WRN("No connection to send button state.");
        }
	}
}
```

8.4 Complete the implementation of the `init_button()` function.

In `init_button()`, we will register the function `button_changed()` to be called when buttons on the board are pressed.
(Ensure `init_button()` is called in `main()` and `button_changed` is declared before `dk_buttons_init` or forward-declared).
```c
// Inside init_button() or ensure this is done
// static int init_button(void) { ...
    err = dk_buttons_init(button_changed);
    if (err) {
        LOG_ERR("Cannot init buttons (err: %d)", err);
    }
// ... }
```

**9. Build and flash the application to your board, and connect to it via your phone.**

When the connection is established, click the tab saying “Cli…” (or Client if your screen is large enough). This tab holds a list of all the services present on the connected device. Look for the “Nordic LED and Button Service”, and press on it to expand it.

Press on the single arrow pointing downwards to read the button characteristic once, or press the icon with multiple arrows pointing downwards to enable notifications from this characteristic.

If you try now to press button 1 on your board (Button 0 on nRF54 Series devices), you should see the Value field changing between “Button Released” and “Button Pressed”.

Remember that you need to subscribe to the notification from the peer (your smartphone or tablet) before pressing the button on the board. If a peer has not subscribed to the notification, and you press button 1, you will get the error `Lesson3_Exercise1: Couldn't send notification. err: -13` printed on the terminal. More on notifications in the next lesson.

## Exercise 2: Updating the connection parameters

_(Note: This exercise is compatible with nRF Connect SDK versions v3.0.0-v3.2.0.)_

In the previous exercise, we established a connection between the Nordic board, as a peripheral, and your smartphone, as a central. We were able to send some simple data using the pre-implemented LED Button Service. Although we didn’t see it, the central already selected some connection parameters when it connected to our peripheral. In this exercise, we will look into what those parameters were, and we will also look at what we can do to change our connection parameters.

We will cover Bluetooth LE services and sending data in more detail in lesson 4. For now, it is only relevant to know that we are sending some data to a connected device.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l3/l3_e2`.

**1. Get the connection parameters for the current connection.**

1.1 Declare a structure to store the connection parameters.

In your `on_connected()` callback function, declare a structure `info` of type `bt_conn_info` to store the connection parameters. Then use the function `bt_conn_get_info()` to populate the `info` struct with the connection parameters used in the connection.
(Add this inside `on_connected()` after `my_conn = bt_conn_ref(conn);` and before turning on the LED).
```c
    struct bt_conn_info info;
	// int err; // already declared in on_connected signature
	err = bt_conn_get_info(conn, &info);
	if (err) {
		LOG_ERR("bt_conn_get_info() returned %d", err);
		// return; // Don't return here, or LED won't turn on
	}
```

1.2 Add the connection parameters to your log.

Let’s log the three main connection parameters that we talked about in Connection parameters. Note that the connection interval is represented by a unit of 1.25 ms, and the supervision timeout in units of 10 ms. So we will do some calculations to make it more readable.

Add the following lines to the end of your `on_connected()` callback in `main.c`:
```c
    if (!err) { // Only log if bt_conn_get_info was successful
        double connection_interval = info.le.interval*1.25; // in ms
        uint16_t supervision_timeout = info.le.timeout*10; // in ms
        LOG_INF("Connection parameters: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, info.le.latency, supervision_timeout);
    }
```

> **Note**
> In order for the log to be able to process float (double) numbers, we have added the config `CONFIG_FPU=y` to the `prj.conf` file. If you used the template from GitHub, this was already added.

**2. Build and flash the sample to your board.**

Your log should look something like this:
```
*** Booting nRF Connect SDK ***
[00:00:00.251,098] <inf> Lesson3_Exercise2: Starting Lesson 3 - Exercise 2
[00:00:00.008,453] <inf> Lesson3_Exercise2: Bluetooth initialized
[00:00:00.009,490] <inf> Lesson3_Exercise2: Advertising successfully started
```

**3. Connect to the device via your smartphone**

Use nRF Connect for Mobile to locate the device, called “Nordic_Peripheral” and connect to it.

The log output will show us the connection parameters for this connection.
```
[00:00:03.989,349] <inf> Lesson3_Exercise2: Connected
[00:00:03.989,379] <inf> Lesson3_Exercise2: Connection parameters: interval 30.00 ms, latency 0 intervals, timeout 240 ms
```
Note that your connection parameters may differ from the ones shown here.

**4. Modify the callbacks to be notified when the parameters for an LE connection have been updated.**

You may have noticed that the `bt_conn_cb` structure we defined in the previous exercise, also has the member `le_param_updated`. This is used to tell our application that the connection parameters for an LE connection have been updated.

> **Note**
> There is also the event `le_param_req` callback member, which is called when the connected device requests an update to the connection parameters. You probably want to have this in your application, but we will not look into that event in this exercise.

4.1 Modify your `connection_callbacks` parameter, by adding the following line (ensure `on_le_param_updated` is declared/defined):
```c
// Forward declaration if defined later
void on_le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout);

// In struct bt_conn_cb connection_callbacks = { ... };
    .le_param_updated       = on_le_param_updated,
```

4.2 Add the `on_le_param_updated()` event.

Define the callback function `on_le_param_updated()` to log the new connection parameters.

Add the following function in `main.c`:
```c
void on_le_param_updated(struct bt_conn *conn, uint16_t interval, uint16_t latency, uint16_t timeout)
{
    double connection_interval = interval*1.25;         // in ms
    uint16_t supervision_timeout = timeout*10;          // in ms
    LOG_INF("Connection parameters updated: interval %.2f ms, latency %d intervals, timeout %d ms", connection_interval, latency, supervision_timeout);
}
```

4.3 Already now, if we flash the application and connect to it, we may see that after some seconds, this callback will trigger, and you might see something like the log below. Note that this will vary depending on your smartphone.
```
[00:00:00.008,483] <inf> Lesson3_Exercise2: Bluetooth initialized
[00:00:00.009,521] <inf> Lesson3_Exercise2: Advertising successfully started
[00:00:21.988,372] <inf> Lesson3_Exercise2: Connected
[00:00:21.988,403] <inf> Lesson3_Exercise2: Connection parameters: interval 30.00 ms, latency 0 intervals, timeout 240 ms

[00:00:27.239,562] <inf> Lesson3_Exercise2: Connection parameters updated: interval 45.00 ms, latency 0 intervals, timeout 420 ms
```
So this entire time, connection parameters changes were in fact being requested by your device. This is because, even though we didn’t actively request the parameter updates when you enable the Bluetooth stack in nRF Connect SDK, default values for a lot of parameters are set, including the peripheral’s preferred connection parameters. And if these parameters don’t match the ones given by the central in the initial connection, the peripheral will automatically request changes to the connection parameters to try to get the preferred values.

Let’s take a look at what their default values are:

*   **Connection interval:**
    *   `config BT_PERIPHERAL_PREF_MIN_INT`: 24 (1.25ms units -> 30ms)
    *   `config BT_PERIPHERAL_PREF_MAX_INT`: 40 (1.25ms units -> 50ms)
*   **Peripheral latency:**
    *   `config BT_PERIPHERAL_PREF_LATENCY`: 0 (Unit is given in number of connection intervals)
*   **Supervision timeout:**
    *   `config BT_PERIPHERAL_PREF_TIMEOUT`: 42 (10ms units -> 420ms)

These are all found in `Kconfig.gatt`, in `<install_path>\zephyr\subsys\bluetooth\host`.

So we can see from the log output above that the initial connection had supervision timeout of 240 ms. Therefore, the peripheral requested a change in the parameters, and after this update, all the parameters turned out to satisfy our preferences.

**5. Change the preferred connection parameters.**

Let’s change our preferred connection parameters by adding the following lines to our `prj.conf`:
```kconfig
CONFIG_BT_PERIPHERAL_PREF_MIN_INT=800
CONFIG_BT_PERIPHERAL_PREF_MAX_INT=800
CONFIG_BT_PERIPHERAL_PREF_LATENCY=0
CONFIG_BT_PERIPHERAL_PREF_TIMEOUT=400
CONFIG_BT_GAP_AUTO_UPDATE_CONN_PARAMS=y
```
The last config, `CONFIG_BT_GAP_AUTO_UPDATE_CONN_PARAMS`, responsible for automatically sending requests for connection parameters updates, is not really needed as it is set to `y` by default. This is why we saw the update take place even though we didn’t manually request it.

You can disable this Kconfig if you do not want your application to ask for updates automatically. In that case, you can request parameter changes manually in your application by using `bt_conn_le_param_update()`.

The above parameters will set both the minimum and maximum preferred connection interval to 1 second. It will set the preferred peripheral latency to 0 connection intervals, and a preferred supervision timeout of 4 seconds.

**6. Build and flash your application, and connect to it with your smartphone.**

The log will output something like this. Note that it will take around 5 seconds after the connected event before the connection parameters are updated.
```
[00:00:00.008,453] <inf> Lesson3_Exercise2: Bluetooth initialized
[00:00:00.009,490] <inf> Lesson3_Exercise2: Advertising successfully started
[00:06:35.863,891] <inf> Lesson3_Exercise2: Connected
[00:06:35.863,922] <inf> Lesson3_Exercise2: Connection parameters: interval 30.00 ms, latency 0 intervals, timeout 240 ms
[00:06:41.113,983] <inf> Lesson3_Exercise2: Connection parameters updated: interval 1005.00 ms, latency 0 intervals, timeout 4000 ms
```
So we can see that the phone initially connected with a connection interval of 30ms and a supervision timeout of 240ms. After we requested some changes, the connection interval was changed to 1.005s and the supervision timeout to 4s. Although the 1005ms is outside of our minimum and maximum preferences, it is up to the central to decide how to reply to your request. These numbers will vary depending on the central (i.e your smartphone) in the connection, but you will most likely see 1000 ms. It is up to the phone, acting as the central, to set the connection parameters.

> **Note**
> To actually observe the connection interval, nRF Blinky is a mobile app that enables you to actually observe the changes in the connection interval when pressing the button on the device.
> Due to the graphical refresh rate of the nRF Connect for Mobile application, you won’t notice any difference in the connection interval when using this app.

We have now made the application less responsive. It may not make sense that an application that feels slower is better, and – in many cases – it isn’t. But remember that in order to decrease the latency, the devices need to communicate more often, thus using more power. This is something to consider when developing your own Bluetooth LE application, For instance, a temperature sensor does not need to update its value 10 times per second. Play around with these connection parameters to find something that suits your application.

**7. Set our preferred PHY.**

First, we need to set up a set of our preferred PHY, we will use 2M PHY.

7.1 Define the function `update_phy()` to update the connection’s PHY.

Create the following function in `main.c`:
```c
static void update_phy(struct bt_conn *conn)
{
    int err;
    const struct bt_conn_le_phy_param preferred_phy = {
        .options = BT_CONN_LE_PHY_OPT_NONE,
        .pref_rx_phy = BT_GAP_LE_PHY_2M,
        .pref_tx_phy = BT_GAP_LE_PHY_2M,
    };
    err = bt_conn_le_phy_update(conn, &preferred_phy);
    if (err) {
        LOG_ERR("bt_conn_le_phy_update() returned %d", err);
    }
}
```
What we want to do is triggering the PHY update directly from the connected callback event. We set our preferred PHY parameter saying that we prefer to use `BT_GAP_LE_PHY_2M`.

7.2 Call the function to update PHY during the connection.

Call the function `update_phy()` from the end of the `on_connected()` callback function, with the `my_conn` as the input parameter. Add the following line (after logging connection parameters and before turning on LED is fine):
```c
    if (my_conn) { // Ensure my_conn is valid
        update_phy(my_conn);
    }
```

**8. Add the callbacks to be notified when the PHY of the connection has changed.**

In theory, this should work, but we want some way to check whether the PHY actually changes. So, let’s add the `le_phy_updated` callback to the `connection_callbacks`, which will tell us if the PHY of the connection changes.

8.1 Implement the `on_le_phy_updated()` callback function. It can look something like this (ensure `on_le_phy_updated` is declared/defined):
```c
// Forward declaration if defined later
void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param);

// ... definition ...
void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
    // PHY Updated
    LOG_INF("PHY updated."); // Simplified initial log
    if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_1M) { // Note: These are TX_POWER enums, might need BT_GAP_LE_PHY_* for actual PHY type
        LOG_INF("New PHY: 1M");
    }
    else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_2M) {
        LOG_INF("New PHY: 2M");
    }
    else if (param->tx_phy == BT_CONN_LE_TX_POWER_PHY_CODED_S8) {
        LOG_INF("New PHY: Long Range (Coded S8)");
    } else {
        LOG_INF("New TX PHY: %u, RX PHY: %u", param->tx_phy, param->rx_phy); // More generic log
    }
}
```
*Self-correction: The `param->tx_phy` and `param->rx_phy` in `bt_conn_le_phy_info` use values like `BT_GAP_LE_PHY_1M`, `BT_GAP_LE_PHY_2M`, `BT_GAP_LE_PHY_CODED`. The `BT_CONN_LE_TX_POWER_PHY_*` are different. Corrected example based on typical usage:*
```c
void on_le_phy_updated(struct bt_conn *conn, struct bt_conn_le_phy_info *param)
{
    LOG_INF("PHY updated.");
    LOG_INF("Selected TX PHY: %s, RX PHY: %s",
            (param->tx_phy == BT_GAP_LE_PHY_1M ? "1M" :
             param->tx_phy == BT_GAP_LE_PHY_2M ? "2M" :
             param->tx_phy == BT_GAP_LE_PHY_CODED ? "Coded" : "Unknown"),
            (param->rx_phy == BT_GAP_LE_PHY_1M ? "1M" :
             param->rx_phy == BT_GAP_LE_PHY_2M ? "2M" :
             param->rx_phy == BT_GAP_LE_PHY_CODED ? "Coded" : "Unknown"));
}
```


8.2 Enable the ability to update the PHY

The callback is ready, but we are currently not able to add it to our `connection_callbacks` struct. If you look at the declaration of `struct bt_conn_cb` in `conn.h`, you will see that this callback is only defined if `CONFIG_BT_USER_PHY_UPDATE` is defined, and by default, it is not.

Add this line to your `prj.conf`:
```kconfig
CONFIG_BT_USER_PHY_UPDATE=y
```

8.3 Add the `le_phy_updated` event to the `connection_callbacks` parameter, by adding the following line to your `connection_callbacks` structure.
```c
// In struct bt_conn_cb connection_callbacks = { ... };
    .le_phy_updated         = on_le_phy_updated,
```

**9. Build and flash the application.**

Now try connecting to your device, and see whether the PHY is updated. What does the log say? It should look something like this:
```
[00:00:00.008,422] <inf> Lesson3_Exercise2: Bluetooth initialized
[00:00:00.009,460] <inf> Lesson3_Exercise2: Advertising successfully started
[00:00:16.133,392] <inf> Lesson3_Exercise2: Connected
[00:00:16.133,422] <inf> Lesson3_Exercise2: Connection parameters: interval 30.00 ms, latency 0 intervals, timeout 240 ms
[00:00:16.224,334] <inf> Lesson3_Exercise2: PHY updated. New PHY: 2M
[00:00:21.189,422] <inf> Lesson3_Exercise2: Connection parameters updated: interval 1005.00 ms, latency 0 intervals, timeout 4000 ms
```

**More on this:**
It is not by chance that we chose 2M as the preferred PHY. Most new phones have support for 2M, while only some phones have support for Coded PHY. If you want to check whether your phone supports Coded PHY, you can replace the `BT_GAP_LE_PHY_2M` in your `preferred_phy` with `BT_GAP_LE_PHY_CODED`. In addition, you need to enable the Kconfig symbol `CONFIG_BT_CTLR_PHY_CODED`.

Lastly, we want to increase our Data Length and MTU size. Even though the LBS service which we are currently using only supports sending one byte of payload data, this is useful in many applications.

**10. Define the function `update_data_length()` to update the data length**

Add this function to your `main.c`:
```c
static void update_data_length(struct bt_conn *conn)
{
    int err;
    struct bt_conn_le_data_len_param my_data_len = {
        .tx_max_len = BT_GAP_DATA_LEN_MAX,
        .tx_max_time = BT_GAP_DATA_TIME_MAX,
    };
    // Make sure to use the passed 'conn' parameter, not global 'my_conn' if it could be different
    err = bt_conn_le_data_len_update(conn, &my_data_len);
    if (err) {
        LOG_ERR("data_len_update failed (err %d)", err);
    }
}
```
Here we are setting the number of bytes and the amount of time to the maximum. Here we are using the defined values `BT_GAP_DATA_LEN_MAX` and `BT_GAP_DATA_TIME_MAX`, which are set to 251 bytes and 17040µs, respectively. You can also set custom parameters if you like. Note that the negotiation that this triggers will result in the maximum parameter that both devices in the connection will support, so you may not get the full 251 bytes that you request. The device that supports the shortest data length will have the final say.

**11.1 Define the function `update_mtu()` to trigger the MTU negotiation**

Add the following function in `main.c` (ensure `exchange_params` and `exchange_func` are declared/defined):
```c
// Forward declaration if defined later
static void exchange_func(struct bt_conn *conn, uint8_t att_err, struct bt_gatt_exchange_params *params);
// Global or static variable
static struct bt_gatt_exchange_params exchange_params;


static void update_mtu(struct bt_conn *conn)
{
    int err;
    exchange_params.func = exchange_func;

    err = bt_gatt_exchange_mtu(conn, &exchange_params);
    if (err) {
        LOG_ERR("bt_gatt_exchange_mtu failed (err %d)", err);
    }
}
```
Similarly to how we requested the data length update, we tell our device to request an MTU update. During an MTU update negotiation, both devices will declare their supported MTU size, and the actual MTU will be set to the lower of the two, since that will be the limiting factor. You may note that this function doesn’t contain the actual MTU size. This is because this needs to be set in your `prj.conf` file, which we will set shortly.

11.2 We also need to declare the `exchange_params` parameter. This needs to be defined outside our `update_mtu()` function, so we will place it close to the top of our `main.c` (done above).

**12. Configure the application to enable data length extension**

Add the following to your `prj.conf` file:
```kconfig
# For General
CONFIG_BT_USER_DATA_LEN_UPDATE=y
CONFIG_BT_CTLR_DATA_LENGTH_MAX=251
CONFIG_BT_BUF_ACL_RX_SIZE=251 # Ensure this is >= CONFIG_BT_CTLR_DATA_LENGTH_MAX
CONFIG_BT_BUF_ACL_TX_SIZE=251 # Ensure this is >= CONFIG_BT_CTLR_DATA_LENGTH_MAX
CONFIG_BT_L2CAP_TX_MTU=247   # This is the MTU your device will request/support
```
_(Note: For nRF5340 DK, specific board files like `nrf5340dk_nrf5340_cpuapp.conf` might override `prj.conf`. Ensure these configs are in the active configuration file.)_

We are first enabling the data length extension by setting `CONFIG_BT_USER_DATA_LEN_UPDATE=y`. Then we set the actual data length by setting `CONFIG_BT_CTLR_DATA_LENGTH_MAX=251`. Then we set the size of the actual buffers that will be used, and lastly, we set the MTU size that we want to use in our application.

**13. Implement the two callback functions that will trigger when the data length is updated and when the MTU is updated.**

13.1 Let us start by adding the data length update callback.
(Ensure `on_le_data_len_updated` is declared/defined).
```c
// Forward declaration if defined later
void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info);

// ... definition ...
void on_le_data_len_updated(struct bt_conn *conn, struct bt_conn_le_data_len_info *info)
{
    uint16_t tx_len     = info->tx_max_len;
    uint16_t tx_time    = info->tx_max_time;
    uint16_t rx_len     = info->rx_max_len;
    uint16_t rx_time    = info->rx_max_time;
    LOG_INF("Data length updated. TX: len %d bytes, time %d us. RX: len %d bytes, time %d us", tx_len, tx_time, rx_len, rx_time);
}
```

13.2 We also need to include it in our `connection_callbacks`:
```c
// In struct bt_conn_cb connection_callbacks = { ... };
    .le_data_len_updated    = on_le_data_len_updated,
```

13.3 Then we need to implement the callback that we set in the `update_mtu()` function (`exchange_func`):
```c
// Definition (declaration was done earlier)
static void exchange_func(struct bt_conn *conn, uint8_t att_err,
			  struct bt_gatt_exchange_params *params)
{
	LOG_INF("MTU exchange %s", att_err == 0 ? "successful" : "failed");
    if (!att_err) {
        // The MTU is the complete L2CAP PDU size. Payload MTU is this minus L2CAP and ATT headers.
        // bt_gatt_get_mtu(conn) returns the negotiated ATT_MTU value.
        // For notifications/indications, payload is ATT_MTU - 3 (Opcode + Handle).
        uint16_t payload_mtu = bt_gatt_get_mtu(conn) - 3;
        LOG_INF("New ATT_MTU: %d bytes, Usable payload for Notifications/Indications: %d bytes", bt_gatt_get_mtu(conn), payload_mtu);
    }
}
```

13.4 Forward the declaration of `exchange_func()`. (This was done in step 11.1)

13.5 Call the functions to update the data length and MTU size in `on_connected()`.

Make sure to call all of the parameter exchange functions that we just implemented from the `on_connected()` callback function (e.g., after `update_phy`):
```c
    // Inside on_connected(), after update_phy(my_conn);
    if (my_conn) {
        k_sleep(K_MSEC(1000));  // Delay added to avoid link layer collisions.
        update_data_length(my_conn);
        update_mtu(my_conn); // This should happen after DLE ideally, or after connection is stable
    }
```
The delay on one second is added to avoid a link layer collision. We don’t need to go into details, but it is not allowed, according to the Bluetooth Low energy specification to have two of these requests active concurrently. *(Self-correction: This refers to LL control procedures. MTU exchange is a GATT procedure, DLE is an LL procedure. They operate on different layers but staggering them is still good practice).*

**14. Build and flash the application to your device, and connect to it using your smartphone.**

Your log output should look something like this:
```
[00:00:00.008,056] <inf> Lesson3_Exercise2: Bluetooth initialized
[00:00:00.009,094] <inf> Lesson3_Exercise2: Advertising successfully started
[00:00:22.159,820] <inf> Lesson3_Exercise2: Connected
[00:00:22.159,820] <inf> Lesson3_Exercise2: Connection parameters: interval 30.00 ms, latency 0 intervals, timeout 240 ms
[00:00:22.508,392] <inf> Lesson3_Exercise2: Data length updated. TX: len 251 bytes, time 2120 us. RX: len 251 bytes, time 2120 us
[00:00:22.627,807] <inf> Lesson3_Exercise2: PHY updated. New PHY: 2M
[00:00:22.658,050] <inf> Lesson3_Exercise2: MTU exchange successful
[00:00:22.658,050] <inf> Lesson3_Exercise2: New ATT_MTU: 247 bytes, Usable payload for Notifications/Indications: 244 bytes

[00:00:27.428,161] <inf> Lesson3_Exercise2: Connection parameters updated: interval 1005.00 ms, latency 0 intervals, timeout 4000 ms
```
You should see a lot of logs from the different callbacks, stating all the different connection parameters for your connection.

**More on this:**
nRF5340 DK: Note that if you look at the solution for this exercise, you probably put all your configurations into your `prj.conf`. In the solution project, `l3/l3_e2/boards`, you will see the files `nrf5340dk_nrf5340_cpuapp.conf` and `nrf5340dk_nrf5340_cpuapp_ns.conf`. Depending on which target you are building for, either of these files will be included as the config file if you are using the nRF5340 DK. The `prj.conf` file will not be included in the build if a `<BOARD_NAME>.conf` is present in the boards folder.