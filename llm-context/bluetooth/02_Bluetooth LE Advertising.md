# Advertising process

## Advertising and discovery

When a Bluetooth LE device is in an advertising state, it sends out advertising packets to announce its presence and potentially connect to another device. These advertising packets are sent out periodically at advertising intervals.

**Definition:**
**Advertising intervals:** The interval at which an advertising packet is sent. In the range of 20 ms to 10.24 s, with a step increase of 0.625 ms.

The smaller the advertising interval is, the more frequently the advertising packets are sent, and consequently, the more power is consumed. Therefore, the tradeoff here is power consumption vs. how fast the advertiser’s advertisement packets will be received by a scanner, commonly referred to as discoverability. To avoid packet collisions, a random delay of 0-10 ms is added before each advertisement packet. This ensures that devices with the same advertising interval do not end up with advertisement packet collisions all the time.

### Advertisement channels

Bluetooth LE devices communicate through 40 different frequency channels. These channels are divided into three primary advertisement channels and 37 secondary advertisement channels. Primary advertisement channels are the channels mainly used for advertisement purposes. Secondary advertisement channels can sometimes be used for advertisement purposes as well, but are mainly used for data transfer after establishing a connection. Throughout this course we will focus only on primary advertisement channels.

_Figure showing Bluetooth LE channels, highlighting primary advertising channels 37, 38, and 39._

To ensure a certain degree of redundancy, advertising packets are sent on all three primary advertising channels, channels 37, 38, and 39. Simultaneously, a scanning device will scan these three channels to look for advertising devices.

Since advertising packets are essential to establishing connectivity, the primary advertising channels are carefully chosen. Channels 37, 38, and 39, despite being consecutive numbers, are not actually neighboring channels, as you can see in the figure above. The separation between the three channels serves to avoid adjacent-band interference. Additionally, these three specific channels suffer the least from noise from other technologies using the ISM band, such as Wi-Fi.

### Scan interval and scan window

Similar to an advertising interval, a scan interval refers to how often a scanner will scan for advertisement packets. The scan window refers to the time the scanner spends scanning for packets, which in practice represents the duty cycle in which the device is scanning vs not scanning during each scan interval.

**Definition:**
**Scan interval:** The interval at which a device scans for advertisement packets.
**Scan window:** The time that a device spends scanning for packets at each scan interval.
Both range from 2.5 ms to 10.24 seconds with a step increase of 0.625 ms.

Since a device advertises on three different advertisement channels, the scanner will rotate through the advertisement channels, by switching the channel after each scan interval.

_Example of an advertiser (peripheral) and a scanner (central) illustrating scan intervals and windows._

Short advertising intervals lead to shorter discovery times but increase power consumption. Similarly, a higher scan window to scan interval ratio shortens the discovery time, but increases the power consumption. The time it takes to connect vs the power consumption will always be a tradeoff, but if you control both the scanner and the peripheral, you can choose intervals that work well together. Note that scanning in general uses a lot more power than advertising, and hence, the scanner/central is usually the device with the larger battery.

### Scan request and response

When a peripheral is advertising, a central can also choose to send a scan request to the peripheral, asking for additional information that is not included in the advertisement packets. If the scan request is accepted, the peripheral will respond with what’s called a scan response, also transmitted over the three primary advertisement channels.

**Definition:**
**Scan request:** A message sent by a central device to a peripheral to request additional information not present in the advertisement packet.
**Scan response:** A message sent as a response to a scan request, containing additional user data.

This is a way for the peripherals to send additional data without having to establish a connection with the central first. The peripheral can alternatively choose to send back an empty scan response if it has no more additional information to provide.

Another way to increase the amount of data a peripheral can advertise at once is with a feature called extended advertising. With this feature, the advertisement packets broadcast on the primary advertisement channels are pointing to supplementary information that is being advertised on the secondary advertisement channels. Extended advertising is beyond the scope of this course, and we will be focusing only on legacy advertisement.

## Advertising types

There are many different ways that a peripheral can advertise.

**Definition:**
**Connectable vs. non-connectable:** Determines whether the central can connect to the peripheral or not.
**Scannable vs. non-scannable:** Determines if the peripheral accepts scan requests from a scanner.
**Directed vs. undirected:** Determines whether advertisement packets are targeted to a specific scanner or not.

The type of advertisement being used is set in the advertisement packet. We will take a look at how to set the advertising type in the exercise section of this lesson.

There are four main advertisement types to cover in legacy advertisements, as well as a fifth one that is used in scan responses but will not be covered here.

*   **Scannable and connectable (ADV_IND):** This is the most common type of advertising. If a peripheral uses this type of advertising, it means that it is both scannable and connectable. This means that the peripheral is advertising its presence and allows the central to send a scan request and will respond with a scan response (hence scannable) which is followed by establishing a connection (hence connectable).
*   **Directed connectable (ADV_DIRECT_IND):** This type of advertisement is used for directed advertisement where the advertiser does not accept scan requests. It is directed, connectable but non-scannable. This can be used in cases where the advertiser already knows the scanner and just wants to reconnect quickly. A good example for this scenario is a Bluetooth mouse that has lost the connection with the PC and just wants to reconnect again. In this case, there is no need to accept scan requests and it is faster to send a directed advertisement packet to shorten the connection process.
*   **Non-connectable and scannable (ADV_SCAN_IND):** An advertiser using this type of advertisement will only accept scan requests, but will not allow establishing a connection with it (hence non connectable).
*   **Non-connectable and non-scannable (ADV_NONCONN_IND):** This type of advertisement does not accept scan requests nor does it accept establishing connections. A typical use-case for this type of advertisement is a beacon, where the device does not need to switch the radio to receiver mode since they do not allow receiving any data, which in turn reduces battery consumption.

As you can see, the advertising types are connectable or non-connectable, scannable or non-scannable, directed or undirected. The following table gives an overview of the four types of legacy advertising and their properties.

| Type             | Connectable | Scannable | Directed |
| :--------------- | :---------: | :-------: | :------: |
| ADV_IND          |      x      |     x     |          |
| ADV_DIRECT_IND   |      x      |           |    x     |
| ADV_SCAN_IND     |             |     x     |          |
| ADV_NONCONN_IND  |             |           |          |

## Bluetooth address

Every Bluetooth LE device is identified by a unique 48-bit address. Bluetooth addresses are categorized as either public or random. Random addresses are further classified as either static or private, depending on whether they change or not. And lastly, private addresses are either resolvable or non-resolvable. The image below shows how Bluetooth addresses are categorized. Note that random and private addresses are merely classification types and not actual address types.

_Image showing categorization of Bluetooth addresses._

A Bluetooth LE device uses at least one of these address types:

*   Public address
*   Random static address
*   Random private resolvable
*   Random private non-resolvable

> **Note**
> The public address assigned to a device is drawn out of the same IEEE address pool as MAC addresses (e.g. for ethernet, Wi-Fi), and therefore it is also commonly referred to as a Bluetooth MAC address.

### Public address

A public address is a fixed address that is programmed into the device at the manufacturer. It must be registered with the IEEE registration authority, and it’s globally unique to that device and cannot change during the lifetime of the device. There is a fee associated with obtaining this type of address.

### Random address

A random address is much more commonly used, as it does not require registration with the IEEE and can be manually configured by the user. This is also the default address type in the samples in the nRF Connect SDK. It is either programmed within the device or created during runtime. You can either have a static or a private address.

### Random static address

A random static address can be allocated and fixed throughout the lifetime of the device. It can be altered at bootup, but not during runtime. This is a low-cost alternative to a public address because you don’t need to register it.

As mentioned above, all Bluetooth LE devices must use either a public address or a random static address, where the latter is far more common.

### Random private address

A private address can be used when a device wishes to protect its privacy. This is an address that changes periodically and is used to hide the device’s identity and to deter device tracking.

A random private address can be resolvable or non-resolvable, and are described below.

#### Resolvable random private address

A resolvable private address is, true to its name, resolvable as intended listeners have a pre-shared key by which they can figure out the new address every time it changes. The pre-shared key is the Identity Resolving Key (IRK) and is used both to generate and to resolve the random address.

The random address is basically just used by the peer to be able to resolve the actual address of the Bluetooth LE device, which is still either the public or the random static address. The IRK allows the peer to translate the random private address into the device’s real Bluetooth LE address.

#### Non-resolvable random private address

A non-resolvable private address is not resolvable by other devices and is only intended as a way to prevent tracking. This type of address is not commonly used.

To summarize there are four different types of addresses:

*   **Public address:** Programmed into the device by the manufacturer, and is registered with the IEEE.
*   **Random static address:** Configurable at boot up and is fixed through the lifetime of the device. Does not need to be registered with the IEEE, and is a common alternative to a public address.
*   **Resolvable random private address:** (optional) An address that changes periodically, but is resolvable by means of a pre-shared key.
*   **Non-resolvable random private address:** (optional) An address that changes periodically and is not resolvable.

## Advertisement packet

Let’s examine how an advertisement packet is structured.

The BLE packet is pictured below, with the main portion going to what’s called the Protocol Data Unit (PDU). The PDU consists of either an advertising PDU (sometimes called an advertising channel PDU) or a data PDU (sometimes called a data channel PDU), depending on whether the BLE packet is used for advertisement or data transmission.

_Bluetooth LE packet structure_

As we can see in the image, the advertising PDU consists of a header and a payload.

The header part of the advertising PDU consists of:

_Advertisement PDU header_

*   **PDU Type:** Determines the advertisement type that we discussed in Advertising types, such as ADV_IND.
*   **RFU:** Reserved for future use.
*   **ChSel:** Set to 1 if LE Channel Selection Algorithm #2 is supported.
*   **TxAdd (Tx Address):** 0 or 1, depending on the transmitter address is public or random.
*   **RxAdd (Rx Address):** 0 or 1, depending on the receiver address is public or random.
*   **Length:** The length of the payload

More information about these fields can always be found in the Core Specification of Bluetooth LE on the Bluetooth SIG website.

The payload of the advertising PDU is divided into two sections, where the first 6 bytes represent the advertiser’s address (AdvA) and the rest goes to the actual advertisement data (AdvData).

_Advertisement PDU payload_

*   **AdvA:** Bluetooth address of the advertising device
*   **AdvData:** Advertisement data packet

> **Note**
> The payload structure depends on the kind of advertising. When doing directed advertisement (ADV_DIRECT_IND) some space is needed to also specify the receiver’s address. Therefore, the AdvData field is replaced by a receiver address field with a size of 6 bytes. Advertisement packets of this type (ADV_DIRECT_IND) do not include a payload.

The advertisement data section is represented as shown in the figure below.

_Advertising packet payload_

The advertisement data packet is composed of multiple structures called advertisement data structures (AD structures). Each AD structure has a length field, a field to specify the type (AD Type), and a field for the actual data itself (AD Data). Note that the most common AD type is 1 byte long.

The advertising data types are defined by the Bluetooth specification and are documented in the nRF Connect SDK here, under “EIR/AD data type definitions”.

Below are a few commonly used ones, that we will be using in following exercises.

*   **Complete local name (BT_DATA_NAME_COMPLETE):** This is simply the device name, that the human user sees when scanning for nearby devices (via a smartphone, for instance).
*   **Shortened local name (BT_DATA_NAME_SHORTENED):** A shorter version of the complete local name.
*   **Uniform Resource Identifier (BT_DATA_URI):** Used to advertise a URI like website addresses (URLs).
*   **Service UUID:** The Service Universally Unique Identifier is a number universally unique to a specific service. It can help scanners identify devices that are interesting to connect to. Different options are available here.
*   **Manufacturer Specific Data (BT_DATA_MANUFACTURER_DATA):** This is a popular type that enables companies to define their own custom advertising data, as in the case of iBeacon.
*   **Flags:** 1-bit variables that can flag a certain property or operational mode of that device.

Here is an example of an advertising data structure that is setting the flag BT_LE_AD_NO_BREDR.

_Example of an advertising data structure_

### Flags

The advertisement flags are one-bit flags encapsulated in one byte, meaning that there are up to 8 flags that can be set. We will take a look at some of the most commonly used flags:

*   **BT_LE_AD_LIMITED:** Sets LE Limited Discoverable Mode, used with connectable advertising to indicate to a central that the device is only available for a certain amount of time before the advertising times out
*   **BT_LE_AD_GENERAL:** Sets LE General Discoverable Mode, used in connectable advertising to indicate that advertising is available for a long period of time (timeout = 0).
*   **BT_LE_AD_NO_BREDR:** Indicates that classic Bluetooth (BR/EDR) is not supported

Both BT_LE_AD_LIMITED & BT_LE_AD_GENERAL are meant for a device in a peripheral role.

## Exercise 1: Setting the advertising data

In this exercise, we will cover core Bluetooth LE APIs in nRF Connect SDK. We will first learn how to enable the Bluetooth LE stack and the default configurations associated with it.

Then we will dive into how to configure our hardware to broadcast non-connectable advertising, making the device act as a beacon so that neighboring scanning devices can see the data.

### Exercise steps

**0. Prepare the project and build and flash it to your board**

0.1 Clone the GitHub repository for this course.

> Note that the repository contains the exercise code base and solutions. Make sure to select the branch that corresponds with the nRF Connect SDK version of your choosing:
>
> *   `main` (default branch): For nRF Connect SDK versions v3.0.0-v3.2.0
> *   `v2.9.0-v2.7.0`: For nRF Connect SDK versions v2.9.0 to v2.7.0
> *   `v2.6.2-v2.3.0`: For nRF Connect SDK versions v2.6.2 to v2.3.0
>
> Some of the exercises have varying exercise texts depending on which version you are using. This is reflected by tabs at the beginning of the exercise text.

Copy the link to the repository and use VS Code’s Command Palette (Go to View -> Command Palette -> type Git Clone and paste in the repository link) to clone the `https://github.com/NordicDeveloperAcademy/bt-fund` repository somewhere close to your root directory (Ex: `C:\myfw\btfund`).

Avoid storing the repo in locations with long paths, as the build system might fail on some operating systems (Windows) if the application path is too long.

0.2 In the nRF Connect extension in VS Code, select Open an existing application, and open the base code for this exercise, found in `bt-fund/l2/l2_e1`.

**1. Include the Bluetooth LE stack in your project.**

In order to include the Bluetooth LE stack in your nRF Connect SDK project, you need to enable `CONFIG_BT` in `prj.conf`. This option will already be enabled in all upcoming exercises.

```kconfig
CONFIG_BT=y
```

Enabling this symbol will apply a set of default configurations for the stack.

The highlights of the default configuration are listed below:

*   Broadcast support (`BT_BROADCASTER`) is enabled.
*   The SoftDevice Controller is used (`BT_LL_CHOICE = BT_LL_SOFTDEVICE`).
*   The TX Power is set to 0 dBm (`BT_CTLR_TX_PWR = BT_CTLR_TX_PWR_0`)

**2. Set the Bluetooth LE device name.**

The name is a C string that can theoretically be up to 248 bytes long (excluding NULL termination). In practice, it is highly recommended to keep it as short as possible, as when we include it in the advertising data, we have only 31 bytes, and these 31 bytes are shared by all advertising data. It can also be an empty string. In this exercise, we will call our device `Nordic_Beacon`.

Add the following line in `prj.conf`:

```kconfig
CONFIG_BT_DEVICE_NAME="Nordic_Beacon"
```

We will include the device name in the advertising data in a later step.

**3. Include the header files of the Bluetooth LE stack needed**

Include the following header files needed for enabling the stack, populating the advertising data, and starting advertising.

Add the following lines in `main.c`:

```c
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
```

**4. Prepare the advertising data.**

For the advertising data, we will use both the advertising packet and the scan response packet.

4.1 Prepare the advertising packet.

4.1.1 Declare an array `ad[]` of type `struct bt_data`, which will be the advertising packet to send out.

Add the following lines in `main.c`:

```c
static const struct bt_data ad[] = {
	/* STEP 4.1.2 - Set the advertising flags */

	/* STEP 4.1.3 - Set the advertising packet data  */

};
```

4.1.2 Populate the flags using the helper macro `BT_DATA_BYTES()`.

The first thing we need to prepare in the advertising packet is the advertising flags, `BT_DATA_FLAGS`.

To help us populate these flags, we will use the helper macro `BT_DATA_BYTES()`, which has the following signature:
`BT_DATA_BYTES(type, val...)`

In this exercise, we are creating a broadcaster with non-connectable advertising. Therefore, we will only set the advertising flag `BT_LE_AD_NO_BREDR`, to indicate that classic Bluetooth (BR/EDR) is not supported.

> **Note**
> Since Nordic’s products only support Bluetooth LE, this flag should always be set to this value.

Add the following line in `main.c` (inside the `ad` array):

```c
BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
```

4.1.3 Set the advertising packet data using the helper macro `BT_DATA()`.

There are numerous standard data types that can be included in the advertising data (either in the advertising packet or the scan response). These data types are defined in the Bluetooth Supplement to the Core Specification.

Below are a few commonly used ones, that we will be using in following exercises. The complete list of advertising data types can be found [here](https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/) (link to Bluetooth SIG GAP Assigned Numbers).

*   **Complete local name (BT_DATA_NAME_COMPLETE):** This is simply the device name. We will include the Complete Local Name in the advertising packet of this exercise.
*   **Shortened local name (BT_DATA_NAME_SHORTENED):** A shorter version of the Complete Local name.
*   **Uniform Resource Identifier (BT_DATA_URI):** You can use this type for advertising URI like website addresses (URLs). We will include the URL of Nordic Developer Academy (`https://academy.nordicsemi.com/`) in the scan response packet of this exercise.
*   **Service UUID:** Different options are available here. This is useful if you want a central to filter advertising devices based on services. We will include the UUID for the LBS and NUS services in upcoming exercises.
*   **Manufacturer Specific Data (BT_DATA_MANUFACTURER_DATA):** This is a popular type that enables companies to define their own custom advertising data, as in the case of iBeacon. We will cover using this data type in the exercise of the next lesson.

For now, we want to include the complete local name, `BT_DATA_NAME_COMPLETE`, in the advertising packet.

We will use the macro `BT_DATA()` to populate data into the advertising packet. The macro expects three parameters as shown in the signature below:
`BT_DATA(type, data, data_len)`

Add the following line in `main.c` (inside the `ad` array):

```c
BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
```

> Note: in legacy advertising we have only 31 octets available. To send more than 31 octets in advertisements, we will need to use the scan response, as we will see next.

4.2. Prepare the scan response.

In the scan response, we will include the URL for Nordic Developer Academy, for the sake of demonstration.

4.2.1 Declare the scan response packet.

Same as we did for the advertising packet, we will declare it as an array `sd[]` of type `struct bt_data`.

Add the following code in `main.c`:

```c
static const struct bt_data sd[] = {
        /* 4.2.3 Include the URL data in the scan response packet*/

};
```

4.2.2 Declare the URL data to include in the scan response packet.

We will declare the URL as an array of `static unsigned char`. In the first byte, we need to specify the URI Scheme Name String Mapping as specified in the Assigned Numbers Document from the Bluetooth SIG. The scheme is used to save data transmitted over the air. So for example, instead of transmitting advertising of 6 bytes for the “https:”, we only need to send one byte (0x17).

_Description: URI Scheme Name String Mapping defined by the Bluetooth SIG in Assigned Numbers Document helps compress common URI schemes like "https:" to a single byte._

Add the following lines in `main.c`:

```c
static unsigned char url_data[] ={0x17,'/','/','a','c','a','d','e','m','y','.',
                                 'n','o','r','d','i','c','s','e','m','i','.',
                                 'c','o','m'};
```

4.2.3 Include the URL data in the scan response packet. Add the following line inside the `sd` packet.

```c
BT_DATA(BT_DATA_URI, url_data,sizeof(url_data)),
```

**5. Enable the Bluetooth LE stack.**

The function `bt_enable()` is used to enable the Bluetooth LE stack in the application. This function must be called before any other calls that require communication with the Bluetooth LE hardware (for example, start advertising).

`bt_enable()` is blocking when passing `NULL` to it, and non-blocking if you pass a `bt_ready_cb_t` callback.

Add the following lines in `main()`:

```c
int err; // Ensure err is declared if not already

err = bt_enable(NULL);
if (err) {
	LOG_ERR("Bluetooth init failed (err %d)\n", err);
	return -1; // Or handle error appropriately
}
LOG_INF("Bluetooth initialized\n");
```

**6. Start advertising.**

Now that we have prepared the advertising data (both the advertising packet and the scan response packet), we are ready to start advertising.

Do this by calling the function, `bt_le_adv_start()`, which has the following signature:
`int bt_le_adv_start(const struct bt_le_adv_param *param, const struct bt_data *ad, size_t ad_len, const struct bt_data *sd, size_t sd_len)`

_bt_le_adv_start() function signature_

The first parameter this function expects is the advertising parameters. Here, we can either use predefined macros that cover the most common cases. Or we can declare a variable of type `struct bt_le_adv_param` and set the parameters manually.

For now, we will use one of the predefined macros. In exercise 2, we will set this parameter manually. In our case, we will be using `BT_LE_ADV_NCONN` – non-connectable advertising with a minimum advertising interval of 100 ms and a maximum advertising interval of 150 ms.

The second and third parameters are the advertising packet (created in step 4.1) and its size, while the fourth and fifth parameters are the scan response (created in step 4.2) and its size.

Add the following lines inside `main()`:

```c
err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
if (err) {
	LOG_ERR("Advertising failed to start (err %d)\n", err);
	return -1; // Or handle error appropriately
}
```

**7. Build and flash the application on your board.**

You should notice that LED1 (LED0 on nRF54 Series devices) on your board is blinking, indicating that your board is advertising.

**8. Open nRF Connect for Mobile on your smartphone.**

In the SCANNER tab press on the SCAN button to begin scanning.

**9. Tap on `Nordic_Beacon` to view the advertising data.**

_Mobile app view showing Nordic_Beacon details_

The first thing to notice is that there is no CONNECT button displayed. This is because we are advertising non-connectable packets (`BT_LE_ADV_NCONN`). Let’s now spend some time interpreting the data.

*   RSSI level is the received signal strength indicator on the receiver side
*   Advertising interval. Since we chose `BT_LE_ADV_NCONN`, the advertising interval is between 100-150 ms
*   Advertising flags that we set in step 4.1.2 (Bluetooth Classic BR/EDR not supported)
*   The complete local name, that we set in step 2
*   The URI data that we included in the scan response packet. Note that you can tap on the OPEN button to open the link directly in a browser.

We encourage you to try and change these parameters in the code, build and flash your board to see how easy it is to control the advertising data.

## Exercise 2

_(Note: The exercise text might have variations for different nRF Connect SDK versions: v3.0.0-v3.2.0, v2.9.0 – v2.7.0, v2.6.2 – v2.3.0. The following is based on the provided common text.)_

In this exercise, we will build on top of the previous exercise to focus on advertising parameters, Manufacturer Specific Data, and updating the advertising data dynamically.

We will still broadcast non-connectable advertising in this exercise. However, as opposed to the previous exercise, we will not use a predefined macro to manage the advertising parameters and fine-control the advertising interval.

We will also learn how to dynamically change the content of advertising data. We will create custom advertising data that represents how many times Button 1 (Button 0 on nRF54 Series devices) is pressed on the board and we will include it in the advertising packet.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l2/l2_e2`.

**1. Create the variable `adv_param` of type `bt_le_adv_param`.**

This variable type can control many aspects of advertising. Let’s see how we can use it to control the advertising interval.

To create the variable, we will be using the helper macro `BT_LE_ADV_PARAM()`, which has the following signature:
`BT_LE_ADV_PARAM(options, interval_min, interval_max, peer)`

_BT_LE_ADV_PARAM() helper macro signature_

*   **Advertising options:** Specific macros to configure the advertising options. For example, choosing which channel (37, 38, 39) to advertise on.
*   **Minimum advertising interval:** (N * 0.625 milliseconds): Less than or equal to the maximum advertising interval. The allowable range for N is 32 to 16384, which translates to 20 ms to 10.24 seconds. The API has predefined values to use for advertising intervals.
*   **Maximum advertising interval:** (N * 0.625 milliseconds): Larger than or equal to the minimum advertising interval. The allowable range for N is 32 to 16384, which translates to 20 ms to 10.24 seconds. The API has predefined values to use for advertising intervals.
*   **Peer address:** Included if directed advertising is used. Otherwise, set to `NULL`.

Add the following lines in `main.c`:

```c
static const struct bt_le_adv_param *adv_param =
	BT_LE_ADV_PARAM(BT_LE_ADV_OPT_NONE, /* No options specified */
			800, /* Min Advertising Interval 500ms (800*0.625ms) */
			801, /* Max Advertising Interval 500.625ms (801*0.625ms) */
			NULL); /* Set to NULL for undirected advertising */
```

The above code will set the advertising interval to about 500 ms (500 ms/0.625 ms = 800 (N)). As we discussed before, there will be a random delay added to avoid packet collisions.

**2. Declare the Manufacturer Specific Data**

Manufacturer Specific Data is used to create and include custom data in Bluetooth LE advertising. The data to be included can be in any format suitable for your use case/application.

2.1 Declare the Company identifier (Company ID)

The first two bytes in the Manufacturer Specific Data are the company identifier as registered in Bluetooth SIG in the Assigned Numbers Document. For educational/testing purposes, we will use Nordic Semiconductor Company Identifier.

Add the following line in `main.c`:

```c
#define COMPANY_ID_CODE 0x0059
```

> **Note:** Bluetooth LE products are not allowed to broadcast a custom company ID without being a Bluetooth SIG member. When you are ready to release your product, you would have to apply for a Bluetooth SIG membership to get your own unique Company ID.

2.2 Declare the structure for your custom data

In this exercise, we will keep it simple and declare our custom data as `unsigned short` (2 bytes). The data will represent how many times Button 1 is pressed. Therefore we will create a structure `adv_mfg_data_type` that has two members. The first is, of course, the Company ID and the second is `number_press` which represents how many times button 1 is pressed on the board.

Add the following `struct` definition to `main.c`:

```c
typedef struct adv_mfg_data {
	uint16_t company_code; /* Company Identifier Code. */
	uint16_t number_press; /* Number of times Button 1 is pressed */
} adv_mfg_data_type;
```

2.3 Define and initialize a variable of type `adv_mfg_data_type` that we created in the previous step.

```c
static adv_mfg_data_type adv_mfg_data = { COMPANY_ID_CODE, 0x00 };
```

We are all set with the Manufacturer Specific Data declaration. In the next step, we will add a button press callback function that updates the `number_press` and update the advertising data through the function `bt_le_adv_update_data()`.

**3. Include the Manufacturer Specific Data in the advertising packet.**

Add the following line inside the definition of the advertising packet `ad`.

```c
BT_DATA(BT_DATA_MANUFACTURER_DATA, (unsigned char *)&adv_mfg_data, sizeof(adv_mfg_data)),
```

**4. Initialize the Buttons library and set a callback function.**

4.1 We will use the DK Buttons and LEDs library to setup an interrupt (call back function) to be called every time button 1 (button 0 on nRF54 Series devices) is pressed.

Add the `init_button()` function definition in `main.c`. (Ensure `dk_buttons.h` is included and `button_changed` is declared).

```c
#include <dk_buttons_and_leds.h> // Typically needed

// Forward declaration for button_changed if defined later
static void button_changed(uint32_t button_state, uint32_t has_changed);

static int init_button(void)
{
	int err;

	err = dk_buttons_init(button_changed);
	if (err) {
		printk("Cannot init buttons (err: %d)\n", err);
	}

	return err;
}
```

It calls the `dk_buttons_init()` function to initialize your board’s buttons and assign `button_changed()`, which will be defined in step 5, as a call back function every time a button is pressed.

4.2 Call `init_button()` in `main()`

Add the following line in `main()` (ensure `err` is declared):

```c
// int err; // if not already declared in main
err = init_button();
if (err) {
	printk("Button init failed (err %d)\n", err);
	return -1; // Or handle error
}
```

**5. Add the definition of the callback function and update the advertising data dynamically.**

We will update the advertising data dynamically from inside the `button_changed()` callback function. We will use the function `bt_le_adv_update_data()`.

`int bt_le_adv_update_data(const struct bt_data *ad, size_t ad_len, const struct bt_data *sd, size_t sd_len)`

_bt_le_adv_update_data() API Function signature_

The `bt_le_adv_update_data()` is very similar to the `bt_le_adv_start()` that we covered in the previous exercise, except it does not take Advertising Parameters options. It relies on the advertising parameters set before in `bt_le_adv_start()`.

Add the definition of the button callback function. (Ensure `USER_BUTTON` is defined, typically `DK_BTN1_MSK` or similar).

```c
// Assuming USER_BUTTON is defined, e.g., #define USER_BUTTON DK_BTN1_MSK
// Also ensure 'ad' and 'sd' are accessible (e.g. global or passed appropriately)
static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	if (has_changed & button_state & USER_BUTTON) { // Ensure USER_BUTTON is correctly defined
		adv_mfg_data.number_press += 1;
		bt_le_adv_update_data(ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
	}
}
```

In the callback function, we will do the following:

*   Check which button was pressed: `if (has_changed & button_state & USER_BUTTON)`
*   Update the count of the number of presses: `adv_mfg_data.number_press += 1;`
*   Update the advertising data dynamically `bt_le_adv_update_data(ad, ARRAY_SIZE(ad),sd, ARRAY_SIZE(sd));`

**6. Build and flash the application on your board.**

You should notice that LED1 (LED0 on nRF54 Series devices) on your board is blinking now. Indicating that your board is advertising.

**7. Open nRF Connect for Mobile on your smartphone.**

> **Note**
> On the nRF54L15 DK, the board’s LEDs and Buttons are labeled with PCB labels (PCB silkscreen) that start with 0 (LED0-LED3) and (BUTTON0-BUTTON3). In previous-generation development kits, the indexing starts with 1 (LED1-LED4). So, on the nRF54L14 DK, we will be working with LED0 and BUTTON0.

In the SCANNER tab, press on the SCAN button to begin scanning.

_Mobile app scanner view_

Notice how the connection interval has changed from the previous exercise which was between 100-150 ms associated with the `BT_LE_ADV_NCONN` macro to around 500ms.

**8. Tap on `Nordic_Beacon` to view the advertising data.**

You should notice now that the Manufacturer data is included in the advertising With the company ID of Nordic Semiconductor.

**9. Tap on the Manufacturer data to change how the data is displayed and select Unsigned Int16 or Manufacturer data (Bluetooth Core 4.1).**

_Image: Data shown as Manufacturer data (Bluetooth Core 4.1)_

_Image: Changing data to Unsigned int 16_

The data will be shown as Unsigned Int 16:

_Image: The data shown as Unsigned Int 16_

**10. Press Button 1 on your board (Button 0 on nRF54 Series devices)**

You should observe that the count increments by one every time the button is pressed. You need to be in scanning mode on your phone to see the change. Press SCAN at the top right corner in nRF Connect for Mobile to see the updated data.

By default, the scanning period on nRF Connect for Mobile is set to 45 seconds. You can change the scanning period by going to Settings->Scanner->Scanning Period.

## Exercise 3

_(Note: The exercise text might have variations for different nRF Connect SDK versions: v3.0.0-v3.2.0, v2.9.0 – v2.7.0, v2.6.2 – v2.3.0. The following is based on the provided common text.)_

In this exercise, we will switch gears from non-connectable advertising, which we covered in Exercises 1 and 2, to connectable advertising. Connectable advertising is used by peripherals to advertise their presence and allows centrals to establish a connection to it.

We will also include the UUID for the LBS service in the advertising data, which can be used by a central to determine whether it wants to connect based on available services. Lastly, we will learn how to manually configure the Bluetooth LE address of the peripheral. Note, the address will only appear when using Android phones, not iOS.

The LED Button Service (LBS) is a custom service created by Nordic, and will be covered in more depth in lesson 4.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l2/l2_e3`.

**1. Include support for the peripheral role from the Bluetooth LE stack.**

Add the following line in `prj.conf`:

```kconfig
CONFIG_BT_PERIPHERAL=y
```

When you enable this flag, you will get the functionalities needed for the peripheral role, including GATT and ATT. The default number for simultaneous connections `CONFIG_BT_MAX_CONN` is 1.

**2. Change the Bluetooth LE device name from `Nordic_Beacon` to `Nordic_Peripheral`.**

Add the following line in `prj.conf`:
```kconfig
CONFIG_BT_DEVICE_NAME="Nordic_Peripheral"
```

**3. Prepare the advertising data.**

3.1 Set the flags and populate the device name in the advertising packet.

As done before, we are including the device name in the packet. We are also enabling the discovery mode flag, as the device will act as a Bluetooth LE peripheral, not a beacon. We will set the discovery to `BT_LE_AD_GENERAL`.

Add the following lines inside the definition of `ad` (in `main.c`):

```c
BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
```

3.2 Populate the scan response packet with the LBS service UUID.

3.2.1 Include the header file of the UUID helper macros and definitions.

Add to `main.c`:
```c
#include <zephyr/bluetooth/uuid.h>
```

3.2.2 Include the 128-bit UUID of the LBS service in the scan response packet.

This UUID can be found here: LED Button Service (LBS) – Service UUID.
`(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)`

Add the following line inside `sd` (the scan response data array in `main.c`):

```c
BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)),
```

**4. Set up the random static address.**

4.1 Include the header file for managing Bluetooth LE addresses.

Add to `main.c`:
```c
#include <zephyr/bluetooth/addr.h>
```

4.2 Change the random static address.

In the past two exercises, the Bluetooth LE address generated was a Non-Resolvable Random Private Address, which is generated randomly every time on bootup. This is the default type of address set for non-connectable advertising. For connectable advertising, the default is a random static address. We will learn how to manually configure the random static address in this step and set it to the address `FF:EE:DD:CC:BB:AA`.

We will use the function `bt_id_create()` to set a new random static address. We will use the function `bt_addr_le_from_str()` to convert a string to a Bluetooth LE address. For types, it supports “random” and “public” addresses.

Add the following code snippet inside `main.c` (likely at the beginning of `main()` function, ensure `err` is declared):

```c
// int err; // if not already declared
bt_addr_le_t addr;
err = bt_addr_le_from_str("FF:EE:DD:CC:BB:AA", "random", &addr);
if (err) {
	printk("Invalid BT address (err %d)\n", err);
}

err = bt_id_create(&addr, NULL);
if (err < 0) {
	printk("Creating new ID failed (err %d)\n", err);
}
```

**5. Start to advertise connectable advertising.**

This is done by calling `bt_le_adv_start()`. To specify it is connectable advertising, we could either do it by passing `BT_LE_ADV_CONN` (instead of the `BT_LE_ADV_NCONN` macro used in exercise 1) as the first parameter. Or we can set an advertising parameter as we did in exercise 2. The advantage of creating an advertising parameter is that it gives us more control over advertising.

5.1 Create the advertising parameter for connectable advertising. Again, this is optional, you could also simply pass `BT_LE_ADV_CONN` as the first parameter to `bt_le_adv_start()`.

Define in `main.c`:
```c
static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM((BT_LE_ADV_OPT_CONNECTABLE|BT_LE_ADV_OPT_USE_IDENTITY), 
                800, /*Min Advertising Interval 500ms (800*0.625ms) */
                801, /*Max Advertising Interval 500.625ms (801*0.625ms)*/
                NULL); /* Set to NULL for undirected advertising*/
```

5.2 Start advertising by calling `bt_le_adv_start()` and passing the advertising parameter along with the advertising packet and scan response and their sizes.

Add in `main()`:
```c
// Ensure ad and sd are defined and accessible
err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad),
			      sd, ARRAY_SIZE(sd));
if (err) {
	LOG_ERR("Advertising failed to start (err %d)\n", err);
	return -1; // Or handle error
}
```

**6. Build and flash the application on your board.**

You should notice that LED1 (LED0 on nRF54 Series devices) on your board is blinking now. Indicating that your board is advertising.

**7. Open nRF Connect for Mobile on your smartphone and start scanning.**

_nRF Connect for Mobile (Android example) showing scanned device_

We should see now that the address of the device is set to `FF:EE:DD:CC:BB:AA`, and the UUID of the LBS service is now advertised. Also, we should notice the CONNECT button next to the device name, indicating that the device is advertising in connectable mode. We will cover connections in the next couple of lessons.