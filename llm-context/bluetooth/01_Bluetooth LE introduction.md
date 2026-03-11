# What is Bluetooth LE?

> **SDK Version:** This lesson is compatible with nRF Connect SDK v3.0.0 - v3.2.0. For the latest documentation, visit [docs.nordicsemi.com](https://docs.nordicsemi.com).

Bluetooth Classic is the version of Bluetooth you have most likely used before due to its prevalence in common items such as wireless headsets. It is quite evident that Bluetooth is a perfect fit for applications such as streaming music. The data throughput is high enough to support it without connection problems or packet loss, plus it's very easy to use. You probably need to recharge your smartphone and your wireless speakers after some time, but that is not a problem in this kind of application.

However, for low-power wearables or massive IoT applications, frequent battery charging is not feasible, especially when such high data transfer speeds are not necessary. Therefore, starting from Bluetooth Core Specification version 4.0, the Bluetooth SIG (Special Interest Group) introduced Bluetooth Low Energy (LE), with the intention of making it a key enabler for low-power IoT applications. The nRF Connect SDK v3.2.0 supports up to Bluetooth Core Specification v6.2.

## Bluetooth LE features

Bluetooth LE, as its name implies, focuses on achieving low energy consumption by sacrificing data rate. Sacrificing data rate refers to two mechanisms here. Firstly, data packets are made smaller, ranging from 27 to 251 bytes. Secondly, data is being sent as seldom as possible to avoid long radio-on times, which is a significant factor in power consumption. This makes Bluetooth LE more suitable for battery-operated devices that need to operate on minimal power and only send small bursts of data.

Bluetooth LE differs from Bluetooth Classic in other aspects as well, such as supported topologies and node types. This is because Bluetooth LE was intended for completely different use cases than Bluetooth classic, therefore different network topologies were necessary.

The table below summarizes some of the key aspects of Bluetooth LE.

| Feature                                              | Value                                     |
| :--------------------------------------------------- | :---------------------------------------- |
| Operating band                                       | 2400 MHz – 2483.5 MHz<br/>~ 2.4 GHz        |
| Channel bandwidth                                    | 2 MHz                                     |
| Number of RF channels                                | 40                                        |
| Maximum transmit power                               | 20 dBm<br/>0.1 W                          |
| Maximum application data throughput                  | 1.4 Mbps                                  |
| Maximum range at reduced data rates (125 & 500 kbps) | ~1000 m                                   |
_Summary of Bluetooth LE specification_

> **Note**
> Range will always depend on a number of factors related to the software and hardware configuration of the devices used, as well as the specific environment where the devices operate. Therefore, it is very difficult to have a generalized precise range estimate.

However, an online range estimator like the one here [https://www.bluetooth.com/learn-about-bluetooth/key-attributes/range/](https://www.bluetooth.com/learn-about-bluetooth/key-attributes/range/) could give an acceptable estimate of what range to expect out of your Bluetooth LE devices, given the configuration indicated in the guide calculator.

A key advantage of Bluetooth Low Energy is its low cost when compared to other low-power personal area networks, making it attractive for applications requiring mass deployments.

The technology is also prevalent in smartphones (most smartphones support both Bluetooth Classic and Bluetooth LE), making it easy to test and prototype applications everywhere. In addition to the smartphone, one more Bluetooth LE device is needed to be able to test two-way communication. Since practically everyone has a smartphone, this reduces the cost and complexity required to conduct tests compared to other technologies where specific hardware is needed.

With Nordic Semiconductor's wide variety of Bluetooth LE offerings, you will also get implementation flexibility, open-source documentation and continuous customer support. You can read more about Nordic Semiconductor's Bluetooth LE offerings on [docs.nordicsemi.com](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrf/protocols/bt/index.html).

## Bluetooth LE protocol stack

A deep understanding of all the individual layers of the protocol stack is not an absolute necessity for enabling and using Bluetooth LE in your application. Nonetheless, understanding the basics of the different layers and some of their main functions does help with having an overview of what’s happening under the hood with the Bluetooth LE stack. This is what we will cover in next section.

Certain layers, due to their importance, will be further explained in subsequent lessons.

The image below depicts the protocol stack architecture, showing which layers make up the Bluetooth LE host and which make up the Bluetooth LE controller.

At the top, you have the application. This is the layer that the user interacts with, through API’s, to make use of the Bluetooth LE protocol. Important parts of the application layer include profiles, services, and characteristics, which will be explained more thoroughly in the upcoming lessons. The next layers make up the host, which essentially determines how Bluetooth LE devices store and exchange data between each other. Lastly, the controller makes up the lower layers, with the most notable part being the physical radio which generates the radio waves and encodes the signals with the data you want to send.

### Host

The Bluetooth LE host consists of the following layers:

*   **Logical Link Control & Adaptation Protocol (L2CAP):** provides data encapsulation services to the upper layers.
*   **Security Manager Protocol (SMP):** defines and provides methods for secure communication.
*   **Attribute Protocol (ATT):** allows a device to expose certain pieces of data to another device.
*   **Generic Attribute Profile (GATT):** defines the necessary sub-procedures for using the ATT layer.
*   **Generic Access Profile (GAP):** interfaces directly with the application to handle device discovery and connection-related services.

The Zephyr Bluetooth Host implements all these layers and provides an API for applications.

### Controller

The Bluetooth LE controller is comprised of the following layers:

*   **Physical Layer (PHY):** determines how the actual data is modulated onto the radio waves, and how it is transmitted and received.
*   **Link Layer (LL):** manages the state of the radio, defined as one of the following – standby, advertising, scanning, initiating, connection.

The Bluetooth LE controller implementation we will be using in this course is the SoftDevice Controller, found in the nRF Connect SDK. Both the SoftDevice Controller and the Zephyr Bluetooth Host form the full Bluetooth LE protocol stack that is available in the nRF Connect SDK. For more details, see the [SoftDevice Controller documentation](https://docs.nordicsemi.com/bundle/ncs-latest/page/nrfxlib/softdevice_controller/README.html).

> **Important**
> The nRF Connect SDK contains an alternative controller implementation, the Zephyr Bluetooth LE Controller. However, we strongly recommend using the SoftDevice Controller, as it is specifically designed for the nRF52, nRF53 and nRF54 Series devices.

## GAP: Device roles and topologies

The Bluetooth LE protocol supports two different communication styles: connection-oriented communication and broadcast communication.

**Definition:**
**Connection-oriented communication:** When there is a dedicated connection between devices, forming bi-directional communication.
**Broadcast communication:** When devices communicate without establishing a connection first, by broadcasting data packets to all devices within range.

### Device roles

The GAP layer defines specific device roles for nodes in a Bluetooth LE network. These roles determine important aspects such as how the device advertises its presence, or how it scans and connects to other nodes.

Advertising and scanning refers to the process by which Bluetooth LE devices become aware of each other’s presence and connection possibilities. For two Bluetooth LE devices to connect to each other, one of them needs to advertise its presence and willingness to connect, while the other will scan for such devices.

**Definition:**
**Advertising:** The process of transmitting advertising packets, either just to broadcast data or to be discovered by another device.
**Scanning:** The process of listening for advertising packets.

### Central and peripheral

In the previous example, the device that advertises its presence and willingness to connect is acting as the peripheral. While the device that scans for advertisements is the central. If the peripheral’s advertisement packets are scanned by the central, the central can choose to initiate a connection by sending a connection request to the peripheral. The peripheral and the central are then said to have established a connection.

**Definition:**
**Central:** A device role that scans and initiates connections with peripherals.
**Peripheral:** A device role that advertises and accepts connections from centrals.

The central device can send connection requests to more than one peripheral simultaneously, and it assumes the role of the host in this connection. Peripherals can also accept connection requests from other centrals by restarting the advertising process after a connection has been established.

Since the central acts as the host, it is responsible for typical host-role duties such as connection management and much of the data processing. This means peripherals generally use less power than a central.

IoT devices that are resource constrained and require low-power are usually the peripheral device in a Bluetooth LE connection, while the central device is something like a mobile phone, which has more power.

> **Note**
> Since the most common use-case for our devices is as a peripheral, that is the connection topology we will examine in the exercises in this course.

### Broadcaster and observer

Sometimes devices only wish to broadcast information without being in a connection with another device. In this case, a special kind of peripheral, called a broadcaster, can send out advertisement packets, but without receiving any packets or allowing connection requests. The information that is broadcasted is included in the advertisement packets. A very good example of broadcasters are beacon devices. They only transmit information, without the need to connect to a specific device. On the other end, a special kind of central device called an observer, listens for the advertising packets, but without sending a connection request to initiate a connection.

**Definition:**
**Broadcaster:** A special kind of peripheral that broadcasts advertisement packets without accepting any connection requests.
**Observer:** A special kind of central that listens to advertising packets without initiating a connection.

For simplicity, we will stick to the peripheral/central device roles throughout this course, unless otherwise noted.

### Network topologies

Now that we have established the different roles of a Bluetooth LE device, let’s look at how the device roles are used in the different network topologies possible with Bluetooth LE.

#### Broadcast topology

In a broadcast topology, data transfer happens without the devices ever establishing a connection. This is done by using the advertisement packets to broadcast the data to any device that is in range to receive the packets. A peripheral (more specifically a broadcaster) advertises the data, and the central (more specifically an observer) will scan and read the data from the advertisement packets.

_An example of a broadcast topology_

This type of communication is commonly used in proximity beacons, in indoor navigation, and many other applications that require a low-power device to transmit small amounts of data to several devices simultaneously.

The advantage to a broadcast topology is that there is no limit to how many devices one can broadcast to. Anyone in range of the advertisement packets can receive the information. This is also much more power efficient than connection-oriented communication. However, due to the limited data available in the advertisement packets, the throughput is limited. There is also no acknowledgment from the receiving devices.

#### Connected topology

A connected network topology establishes a connection before data transfer occurs. Unlike the broadcast topology, the communication is now bidirectional.

> **Note**
> Although the Bluetooth LE specification technically does not limit the number of connections possible, there are bandwidth and hardware limitations that come with using small embedded devices.

Below is an example of a connected topology where a central has established communication with three peripherals, and one of those peripherals is already connected to two other centrals.

_An example of a connected topology_

The advantage of a connected topology is the increased throughput that comes with establishing a direct link before communication. Additionally, the communication is bi-directional, meaning that the central and peripheral can communicate with each other, as opposed to broadcasting, where the peripheral just broadcasts to the central without being able to receive anything back.

**More on this:**
With the introduction of Periodic Advertising with Responses (PAwR) in Bluetooth 5.4, bidirectional communication in connectionless mode is possible. This is however beyond the scope of this course.

#### Multi-role topology

A single device can also operate in multiple different roles simultaneously. For instance, the same device can act as a peripheral in one setting, and a central in another.

_Example of a multi-role topology_

This multi-role functionality is often used in systems where a device, let’s call it a hub, is receiving sensor data from multiple sensors, but also wants to forward this data to mobile phones. In this case, the hub can act as a central and connect to multiple sensors (peripherals), and can also act as a peripheral and transmit sensor data to one or more smartphones (centrals).

## ATT & GATT: Data representation and exchange

In the previous topic, we discussed the role that the GAP layer plays in defining how Bluetooth LE devices communicate with each other in both the advertisement and connection phases. The communication during advertising is only used for device discovery or for broadcasting data and is handled by the GAP layer itself. However, after establishing a connection, there is a need for bidirectional data exchange. This requires specific data structures and protocols tailored for these purposes.

The Attribute protocol (ATT) layer, and the Generic Attribute Profile (GATT) layer right above it, define how data is represented and exchanged between Bluetooth LE devices. The ATT and GATT layers are concerned with the phase after a connection has been established, as opposed to the GAP layer which takes care of the advertisement process which occurs before a connection is established.

### The Attribute Protocol

The ATT layer is the basis on which data is transmitted, received, and handled in the connection phase of Bluetooth LE devices. It is based on a client-server architecture where the server holds the data and can either send it directly to the client or the client can poll the data from the server.

The client and server roles defined in this layer are assigned independently from the peripheral and central roles defined in the GAP layer. So a central can be both a client or a server, and same with a peripheral. This all depends on the application use case and the nature of the data being transferred.

In most cases, the peripheral will be a server since the peripheral is the device that acquires data and holds it. Similarly, the central is typically the client as it is the device receiving said data from the server.

These roles are used by the GATT layer, so they are often referred to as the GATT server and the GATT client.

**Definition:**
**GATT server:** Device that stores data and provides methods for the GATT client to access the data.
**GATT client:** Device that accesses the data on the GATT server, through specific GATT operations which will be explained in Lesson 4.

The ATT layer defines a data structure called the attribute, which is used by the GATT server to store data. The server can hold a number of different attributes at the same time.

**Definition:**
**Attribute:** A standardized data representation format defined by the ATT protocol.

### The Generic Attribute Profile

The Generic Attribute Profile (GATT) layer sits directly on top of the ATT layer, and builds on it by hierarchically classifying attributes into profiles, services and characteristics. The GATT layer uses these concepts to govern the data transfer between Bluetooth LE devices.

_Example of a general profile with services and characteristics_

#### Profiles, services and characteristics

Let’s use a sensor device that measures the heart rate as an example. The heart rate value will be saved as an attribute, called the characteristic value attribute. There will also be another attribute holding metadata about the data stored in the value attribute, called the characteristic declaration attribute. These two attributes together form what’s called a characteristic. In this example, it’s the Heart Rate Measurement characteristic.

All characteristics are enclosed in what’s called a service. Services usually contain multiple characteristics. For this example, the Heart Rate Measurement characteristic is contained in the Heart Rate service. This service also has other characteristics, like the Body Sensor Location characteristic for example.

Then above this, a profile is one or more services that address the same use case. The Heart Rate service is found in the Heart Rate profile, along with the Device Information Service. The Device Information service contains characteristics such as the Manufacturer Name characteristic and the Firmware Revision characteristic.

_Example of the Heart Rate profile_

Before a client starts interacting with a server, the client is not aware of the nature of the attributes stored on that server. Therefore, the client first performs what’s called Service Discovery where it inquires from the server about the attributes.

> **Note**
> The complete list of GATT profiles defined by the Bluetooth SIG can be found [here](https://www.bluetooth.com/specifications/gatt/). The Bluetooth specification also allows vendors to define their own profiles for use cases not covered by the SIG-defined profiles.

## PHY: Radio modes

At the bottom of the Bluetooth LE stack is the physical layer (PHY). PHY refers to the physical layer radio specifications that govern the operation of the Bluetooth LE radio. This layer defines different modulation and coding schemes adopted by Bluetooth LE radio transmitters that affect things like the throughput of the radio. This in turn changes the battery consumption of the device or the range of the connection.

### 1M PHY

1M PHY, or 1 Megabit PHY, is the classic PHY supported by all Bluetooth LE devices. As its name implies, 1M PHY uses 1 megabit per second.

When initiating a connection between two Bluetooth LE devices, this is the mode that will be used, to begin with. Then the peers can request another mode if both devices support it.

### 2M PHY

2 Megabit PHY is a new mode introduced in Bluetooth v5.0. As the name implies, it effectively doubles the data rate to 2 megabit per second, or 2 Mbps. Since the data is transmitted at a higher data rate (faster), the radio needs to stay on for less time, decreasing battery usage. The downside is the decrease in receiver sensitivity which translates to less communication range.

### Coded PHY

While 2M PHY exists for users willing to sacrifice range for increased data rate, coded PHY was introduced to serve applications where users can achieve longer communication range by sacrificing data rate. Coded PHY uses coding schemes to correct packet errors more effectively, which also means that a single bit is represented by more than 1 symbol. Coded PHY uses 2 modes, S=2 and S=8. In the S=2 mode, 2 symbols represent 1 bit, therefore the data rate is 500 kbps. While in the S=8 mode, 8 symbols are used to represent a bit and the data rate becomes 125 kbps.

## Exercise 1: Testing a Bluetooth LE connection

In this exercise, we will be using the Bluetooth: Peripheral LBS sample, running on your Nordic board, to establish a Bluetooth LE connection between the board and your smartphone. The Peripheral LBS sample makes the Nordic board act as a Bluetooth LE peripheral, advertising its presence to your smartphone, which will act as the central.

When connected, the Nordic board will act as a GATT Server, exposing the LED Button Service (LBS) to your smartphone, the GATT Client. LBS has two characteristics, Button and LED. The GATT Client, in our case a smartphone, can write and read to these characteristics to control the LEDs and read the status of the button on the Nordic board. The Nordic board acts as the GATT server here.

For all exercises in this course, when building the sample application, you can use any of the boards listed under Hardware Requirements on the course page and their corresponding build targets.

> **Note**
> Starting with nRF Connect SDK v3.0.0, board targets use `/` instead of `_` as separators. For example:
> - nRF5340 DK: `nrf5340dk/nrf5340/cpuapp` (previously `nrf5340dk_nrf5340_cpuapp`)
> - nRF52840 DK: `nrf52840dk/nrf52840` (previously `nrf52840dk_nrf52840`)
>
> When flashing, use `west flash` which now uses nrfutil under the hood.

> **Note**
> This course builds on the nRF Connect SDK Fundamentals course and focuses on Bluetooth Low Energy. You need to read Lesson 1 in the nRF Connect SDK Fundamentals to learn how to install nRF Connect SDK on your machine and build applications. Furthermore, completing the nRF Connect SDK Fundamentals course is strongly advised as it provides a comprehensive understanding of the nRF Connect SDK.

The exercises in this course assume you have a basic understanding of the nRF Connect SDK.

### Exercise steps

1.  **Build and flash the Peripheral LBS sample.**
    1.1 In Visual Studio Code, in the WELCOME panel, select “Browse samples” and search for “Bluetooth LE LED Button service”.
    _Select this sample, and it will appear under the APPLICATIONS panel._
    1.2 Add a build configuration and select whichever board you are using.
    1.3 Build and flash the application to your device.
    If this was successful, you should notice LED1 (LED0 on nRF54 DKs) blinking on your device.

2.  **Use your smartphone to test the application**
    2.1 Download nRF Connect for Mobile from your app store, and launch it on your smartphone.
    2.2 Turn on Bluetooth and Location services on your smartphone.

3.  **(optional) Open a terminal to view the log output from the application**
    The UART peripheral is gated through the SEGGER debugger/programmer chip (i.e interface MCU) to a USB-CDC virtual serial port that you can connect directly to your PC.
    _Connect to the COM port of your DK in VS Code by expanding your device under Connected Devices and selecting the COM port for the device. The number for the COM port may differ on your PC._

    > **Note**
    > When using the nRF5340 DK, you will have two COM ports, one for the application core and one for the network core. We want to view the output from the application core.

    Use the default settings 115200 8n1 rtscrs:off. Then reset the device to see the full log message.
    If the advertising was successful, your log output should look like this:
    ```
    *** Booting nRF Connect SDK ***
    Starting Bluetooth Peripheral LBS example
    I: 2 Sectors of 4096 bytes
    I: alloc wra: 0, f58
    I: data wra: 0, 140
    Bluetooth initialized
    Advertising successfully started
    ```
    LED1 (LED0 on nRF54 DKs) should also be blinking, indicating that the device is advertising.

    > **Note**
    > On the nRF54 DKs (Ex: nRF54L15 DK), the board’s LEDs and Buttons are labeled with PCB labels (PCB silkscreen) that start with 0 (LED0-LED3) and (BUTTON0-BUTTON3). In previous-generation development kits, the indexing starts with 1 (LED1-LED4) and (BUTTON1-BUTTON4).

4.  **Establish a connection to your Nordic board.**
    4.1 From the nRF Connect for Mobile app, press the SCAN button. Your smartphone will now act as a central device, scanning for any Bluetooth LE devices that are advertising in its presence.
    4.2 Several Bluetooth-enabled devices surrounding you will start appearing. Choose the one called Nordic_LBS and press CONNECT.
    4.3 Press Connect again if prompted with this view.
    In rare cases with some Android phones, you might need to first press the 3 dots at the top right corner, then press “Bond”, before connecting to the NORDIC_LBS device.
    4.4 You will know a connection has been established when LED2 (LED1 on nRF54 DKs) on your device is on.

5.  **Observe the interface on the app.**
    *   You can check that you are connected to the correct device.
    *   The services available on this device. Here we see the Nordic LED Button Service, since this is the sample we have flashed to the device.
    *   Inside that Service, there is a Characteristic called “Button“, or “Blinky Button State“. It reflects the status of button 1 on the Nordic board. Hence, its properties are “Read” and “Notify”. “Notify” refers to the case when the peripheral pushes data to the central. More on notifications in subsequent lessons.
    *   Inside that same Service, there is also another Characteristic called “LED” or “Blinky LED State“. It allows you to toggle LED 3 (LED2 on nRF54 DKs) on the Nordic board. Hence, its property is “WRITE”.

6.  **Read the status of button 1 using the “Button” Characteristic.**
    *   To receive and view the button 1 status(button 0 on nRF54 DKs), you need to view the “Value” tab in your “Button” Characteristic. This is done by pressing the single down-arrow icon.
    *   To enable notifications regarding this Characteristic to be pushed to your app, you need to press the icon with several down-arrows.
    *   Now, you should be able to see the status of button 1 (button 0 on nRF54 DKs) as released.

7.  **Observe the status of button 1 (button 0 on nRF54 DKs).**
    7.1 Press button 1 on your Nordic board.
    7.2 Observe the value shown below changes to “Button pressed”.

8.  **Toggle LED 3 (LED2 on nRF54 DKs).**
    8.1 Press the up arrow icon next to the LED characteristic.
    8.2 Write a value to the LED characteristic to turn it on. Select ON and then SEND to turn the LED on.
    8.3 Observe that LED3 is turned on.
    8.4 Turn off the LED by selecting OFF.