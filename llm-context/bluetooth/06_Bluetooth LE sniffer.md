# Sniffing Bluetooth LE packets

A Bluetooth sniffer is a tool used to intercept the Bluetooth LE packets as they are transmitted, ie. “sniff” the packets, and view them in real-time. This not only provides an overview of what happens over the air but also offers you a better understanding of the protocol. It gives you very detailed information about each and every packet exchanged between two Bluetooth LE devices in near real-time, even when the connection is encrypted.

It’s also an excellent tool to capture and provide the sniffer trace to our Technical Support team so that they can quickly inspect your data without having to reproduce the whole setup on their side. This can significantly speed up the investigation and troubleshooting process.

## nRF Sniffer for Bluetooth LE

Nordic provides a simple-to-use and easy-to-set-up Bluetooth LE Sniffer called the nRF Sniffer for Bluetooth LE. nRF Sniffer works by running in Bluetooth LE hardware placed in radio range of the Bluetooth LE communication you would like to debug. Therefore, it requires an extra nRF52 development kit or dongle to use as the hardware for the sniffer.

_Diagram illustrating a Bluetooth LE sniffer setup with a DK/Dongle capturing packets between two communicating BLE devices._

nRF Sniffer was initially built in the spare time of our engineers when they were developing Bluetooth LE applications. The aim was to have an alternative to the rather expensive professional Bluetooth LE sniffer equipment to test and verify our own Bluetooth LE stack. And it quickly proved to be an imperative tool, not only for us but also for our customers. Anyone with a spare nRF52 development kit can use it as a sniffer for debugging.

### How does nRF Sniffer work?

The sniffer works by using the radio hardware on the Nordic SoC running Nordic proprietary firmware that utilizes said radio to observe and analyze Bluetooth LE packets between other devices. The firmware does not use the Bluetooth stack we have been using in this course, but a stack that was written in bare metal (i.e without an operating system). This gives the sniffer more flexibility and control over how we can utilize the radio to capture Bluetooth LE packets.

Since the Nordic chips only have one antenna, the sniffer can only observe a single RF channel at a time. As we have covered, Bluetooth LE advertising transmits advertisement packets on three channels, channel 37, 38 and 39. This is solved by utilizing the advertising pattern. So the majority of the time, the sniffer will scan in the first channel in the pattern, say channel 37. When it captures a packet in channel 37, it will automatically switch to scanning channel 38 until it has captured another packet, and then switch to channel 39.

What about connection-oriented communication where up to 37 channels can be used? Luckily, the channel hopping when in connection can be easily tracked by looking at the connection request and the channel map update. nRF Sniffer will automatically detect that and follow the channel hopping of the connection.

Because of the limitations on the SoC (with only one radio and one antenna) it is not possible to follow more than one connection at a time. For example, if you are following the connection from an advertising device, you will not be able to capture advertising packets from other devices.

## Setting up nRF Sniffer for Bluetooth LE

In this chapter, we will go through how to set up nRF Sniffer and verify that it functional by sniffing a Bluetooth LE packet.

nRF Sniffer has a comprehensive documentation on how to setup the nRF Sniffer on your computer. Since the time of writing this chapter the way of installing nRF Sniffer has been changed. You can either follow the linked documentation to use `nrfutil` tool to install the Sniffer for your or you can continue with the manual installation described in this document. It’s recommended to follow the instruction in the nRFSniffer’s documentation linked above as it should contain the most up-to-date installation instructions.

### Programming the nRF Sniffer firmware

The nRF Sniffer firmware supports the following boards:

*   nRF52840 DK
*   nRF52840 Dongle
*   nRF52833 DK
*   nRF52 DK
*   nRF51 DK
*   nRF51 Dongle

> **Important**
> Due to a recent update of the nRF52833 DK version 3 and nRF52840 DK version 3, the new Interface IC on the DK is not fully compatible with the nRF Sniffer software. If you have an nRF52840DK v3, you will need to use the nRF USB port instead of the Interface IC USB port. The nRF52833 DK v3 is not compatible with the sniffer software at the moment, so you will need to use another DK as the sniffer backend. nRF52833 DK v2 and earlier works fine.

#### Download the firmware

1.  Download nRF Sniffer for Bluetooth LE v4.x or later and extract the contents of the zip file into a folder of your choice.
    In the following sections, this folder is referred to as `Sniffer_Software`.
    All the firmware HEX files are located in `Sniffer_Software/hex`.

    | Development kit/dongle | Firmware file name                        |
    | :--------------------- | :---------------------------------------- |
    | nRF52840 DK            | `sniffer_nrf52840dk_nrf52840_*.hex`       |
    | nRF52840 Dongle        | `sniffer_nrf52840dongle_nrf52840_*.hex`   |
    | nRF52833 DK            | `sniffer_nrf52833dk_nrf52833_*.hex`       |
    | nRF52 DK               | `sniffer_nrf52dk_nrf52832_*.hex`          |
    | nRF51 DK               | `sniffer_nrf51dk_nrf51422_*.hex`          |
    | nRF51 Dongle           | `sniffer_nrf51dongle_nrf51422_*.hex`      |

2.  Open up nRF Connect for Desktop and install and launch the Programmer application.
    On macOS and Linux: install the SEGGER J-Link software before proceeding to the next step.
    If you are running an M1-based Mac, you must install the Intel/x86 variants of J-Link.

3.  In the upper left hand corner, select the board you are using as the Bluetooth LE sniffer.

4.  Select Add file and Browse, then navigate to `Sniffer_Software/hex` and select the file that applies to the hardware you are using, see the table above. Select Open.

5.  Click Erase & write to flash the firmware to your board.

_Screenshot of nRF Connect Programmer application showing device selection, adding HEX file, and erase & write button._

### Installing Wireshark

This will explain the installation process for Windows and macOS. For instructions on Ubuntu Linux, see Installing Wireshark on Ubuntu Linux.

1.  Go to the [Wireshark download page](https://www.wireshark.org/download.html).
2.  In the Stable Release list at the top of the page, select the release package for your operating system.
    The download should start automatically.
3.  Open up the file when it’s finished downloading, and follow the instructions to download Wireshark.

Wireshark is an open-source packet analyzer, and can be used for many different protocols. To use it with the nRF Sniffer firmware, we offer an external capture plugin to use with Wireshark.

### Installing the nRF Sniffer capture tool

The nRF Sniffer capture tool comes either as stand alone tool or as an external capture plugin for Wireshark.

1.  Install nRF Sniffer command.
    1.1 Open a command window and navigate to the folder `Sniffer_Software/extcap`.
    1.2 Run the following commands to install Python’s requirements (make sure you have Python v3.6 or later installed on your computer):
    ```bash
    pip3 install -r requirements.txt
    ```

2.  Copy the nRF Sniffer capture tool into Wireshark
    2.1 Open Wireshark
    2.2 Go to Help > About Wireshark (on Windows or Linux) or Wireshark > About Wireshark (on macOS).
    2.3 Select the Folders tab.
    2.4 Double-click the location for the Personal Extcap path to open this folder.
    _Screenshot of Wireshark About dialog, Folders tab, highlighting Personal Extcap path._
    You may be prompted with a notice saying `The directory does not exist.` Click Yes to create it.
    2.5 Copy the contents of the `Sniffer_Software/extcap/` folder into this folder.
    _Screenshot showing files from `extcap` folder being copied to Wireshark's Personal Extcap folder._

3.  Enable the nRF Sniffer capture tool in Wireshark.
    3.1 Refresh the interfaces in Wireshark by selecting Capture > Refresh Interfaces or pressing F5.
    3.2 Select View > Interface Toolbars > nRF Sniffer for Bluetooth LE to enable the nRF Sniffer interface.
    You should see that nRF Sniffer is displayed as one of the interfaces on the Wireshark capture screen, and you should see the nRF Sniffer toolbar.
    _Screenshot of Wireshark interface showing nRF Sniffer as a capture interface and its toolbar._

### Running the nRF Sniffer

1.  To start sniffing, make sure the nRF Sniffer (your DK or dongle running the nRF Sniffer firmware) is turned on and place it between the two devices that are communicating over Bluetooth LE.
    _Diagram showing the physical setup for sniffing: two BLE devices communicating, with the nRF Sniffer DK/Dongle placed between them._

2.  In Wireshark, under Capture, double-click on the hardware interface `nRF Sniffer for Bluetooth LE COM port`, see below.
    _Screenshot of Wireshark capture interface selection, highlighting the nRF Sniffer COM port._

3.  Wireshark should now look something like the image below, listing all Bluetooth LE packets in radio range.
    _Screenshot of Wireshark live capture window showing a list of Bluetooth LE packets._

### Explaining Wireshark in Live Capture

Before proceeding to the exercise portion of this lesson, let’s explain what we are seeing in the Wireshark window.
Your window should be divided into three parts, the packet list, packet details and packet bytes.
If you cannot see all three windows, select View and make sure the following three lines are checked off:
_Screenshot of Wireshark View menu options related to displaying Packet List, Packet Details, and Packet Bytes panes._

*   **Packet List:** Displays all the packets in the current capture session. Each line corresponds to one packet, and if you select a line, more details about the packet will be displayed in the “Packet Details” and “Packet Bytes” panes, below.
*   **Packet Details:** Shows the current packet, selected in the Packet List window, in a more detailed form.
*   **Packet Bytes:** Shows the data of the current packet, selected in the Packet List window, in a hexdump style.

Clicking on a specific section of the data in the Packet Bytes window will show where in the Packet Details window. And similarly, selecting a header in the Packet Details window, will show where in the data this information is defined, in the Packet Bytes window.

#### Columns in the Packet List window

Let’s take a look at the columns in the Packet List window. Your column headers should look like this:
_Screenshot of Wireshark column headers: No., Time, Source, Protocol, Length, Info._

If you are missing any of the column headers, go to the Packet Details window and expand `nRF Sniffer for Bluetooth LE`. Then right-click on any of the parameters you are missing, select `Apply as Column` and it will show up in the main view as a column.
_Screenshot showing how to right-click on a parameter in Packet Details and select "Apply as Column"._

Now you should have the following column headers in your live capture view.
_Screenshot showing updated Wireshark column headers including Event counter, Channel Index, Delta time._

*   **No.:** The packet number, incremented for every packet the sniffer captures.
*   **Time:** The timestamp for when the packet was captured, relative to how long the sniffer has been running.
*   **Source:** The address of the device that the packet came from.
*   **Protocol:** Which Bluetooth LE stack layer the packet came from, most will come from the link layer (LE LL). Connection parameter updates come from L2CAP, while GATT operations come from the ATT layer, and packets having to do with encrypting and pairing come from SMP.
*   **Length:** The number of bytes captured in the packet.
*   **Event counter:** The instant number of each connection event starting from 0 when the connection is established.
*   **Channel Index:** Channel number the packet was captured on.
*   **Delta time (start to start):** The time between the start of the previous packet until the start of the current packet. It’s often used to count the distance between each connection event, and very close to the actual connection interval.
*   **Info:** Information about the packet.

You are now ready for the exercise portion of this lesson, where we will go into more detail on what you are seeing.

> **Important**
> If you have issues setting up the nRF Sniffer, there is a [Troubleshooting section](https://docs.nordicsemi.com/bundle/ug_sniffer_ble/page/UG/sniffer_ble/troubleshooting.html) from the documentation that you can take a look at.

## Exercise 1: Capture and analyze Bluetooth advertising packets

_(Note: The exercise text is based on nRF Connect SDK versions v3.0.0 – v3.2.0.)_

This exercise builds on the firmware we made in Exercise 2 of Lesson 2, where the peripheral advertised in scannable, non-connectable mode, and we also made a scan response packet containing a URL.
We will use the sniffer to capture advertising packets and analyze the content of the advertising packet, as well as the scan request packet and scan response packet.

### Exercise steps

In the GitHub repository for this course, go to the code for this exercise, found in `l6/l6_e1`.
Note that this is the same firmware as the solution to exercise 2 found in `l2/l2_e2_sol`.

1.  **Build and flash the application on your board.**
    LED1 on your board should be blinking, indicating that your board is advertising.

2.  **Run the sniffer on Wireshark**
    Open Wireshark. Under Capture, double-click on the hardware interface `nRF Sniffer for Bluetooth LE COM port`, just like we did when setting up nRF Sniffer.

3.  **Filter advertising packets from “Nordic_Beacon“**
    You may notice that the captured advertising packets are from multiple different advertising devices.
    Let’s filter these out, by clicking on the Device drop-down list to find the “Nordic_Beacon“.
    _Screenshot of nRF Sniffer toolbar in Wireshark, highlighting the Device drop-down list._
    > **Note**
    > If the name of the advertiser is not shown, you can also look for the address. You can find the address of the device in nRF Connect for Mobile (on Android).

    Now you should only see the advertising packets from this device, including the scan requests and scan response related to this device.
    _Screenshot showing Wireshark filtered to display packets only from "Nordic_Beacon"._
    Make sure that the “Automatic Scroll” function is enabled to always see the latest packets.
    _Screenshot of Wireshark toolbar, highlighting the "Automatic Scroll" button._
    > **Note**
    > Since the peripheral in exercise 2 from lesson 2 uses a randomly generated address that is assigned boot-up, resetting the device will give it a new address. The sniffer won’t be able to track that, so you will need to select “All advertising devices” to track all advertisers and then select the new Nordic_Beacon device with the new address.

4.  **(optional) Apply an RSSI filter for the Capture interface**
    If you are working in a dense environment, with many Bluetooth LE devices, this list of devices can be quite long. In the next step, we will filter out these devices based on their vicinity to the sniffer, using the RSSI. If you don’t have a problem with too many devices, you can skip straight to step 5.
    Let’s apply a filter for the capture interface, based on the RSSI, or Received Signal Strength Indicator, of the advertising packets.

    4.1 Close and re-open Wireshark, to see the Capture menu again.
    4.2 In the Capture interface selection, type in `RSSI >= -50`.
    Make sure to click on `nRF Sniffer for Bluetooth LE COM port` before typing in the filter. Otherwise, the bar will turn red.
    This will filter out all Bluetooth LE packets that have RSSI smaller than -50dBm.
    _Screenshot showing Wireshark capture filter input field with `RSSI >= -50` entered._
    4.3 Start the sniffer again, and you should see much fewer devices in the Device drop-down list.

5.  **Inspect the broadcasted advertising packets**
    _Screenshot of Wireshark showing ADV_SCAN_IND packets from Nordic_Beacon on channels 37, 38, 39 with ~500ms interval._
    1. First, observe that the advertising packets are of type `ADV_SCAN_IND`, which is non-connectable and scannable. This means the advertiser will accept scan request, but not connection requests.
    2. Notice that the advertising packets are broadcasted consecutively on the three advertising channels 37, 38 and 39.

    Another thing to note, in the `Delta time` column, is that the advertising interval is roughly 500 ms. The three packets are relatively close together, with around 1.5ms between them, and then about 500ms later, another cluster of three advertising packets are sent.
    Notice the slight difference in the delta time between each advertising event. It’s not exactly 500ms between them. This is because of the 10ms random delay added to each advertising event to avoid continuous collisions if two advertisers have the same advertising interval.

6.  **Inspect the contents of an advertising packet**
    Let’s inspect an advertising packet, by clicking on a `ADV_SCAN_IND` packet from the Nordic_Beacon device.
    The bottom half of your window (the Packet Details and Packet Bytes window) will now be updated to show this advertising packet.
    _Screenshot of Wireshark Packet Details and Packet Bytes panes for an ADV_SCAN_IND packet._
    Expand `Bluetooth Low Energy Link Layer` and `Advertising Data`. Clicking on the different subitems (Flags, Device Name, Manufacturer Specific) will highlight in which part of the packet this data is.
    We can match the advertising data in binary to the format we learned in Lesson 2. Recall that the advertising data consists of multiple advertising data structures, starting with the length followed by the type and then the content.
    _Diagram illustrating the AD structure: Length, AD Type, AD Data._
    In this case, we can see `0x02 0x01 0x04` is the first advertising structure. In this we have the length is `0x02` byte, the type is `0x01` (meaning it’s a flag) and the value of the flag is `BT_LE_AD_NO_BREDR` (`0x04`). You can find the same pattern repeated with the Manufacturer Specific data, starting with the length `0x05`, then the type `0xFF` (Manufacturer Specific), and the actual contents which consist of the Company ID (`0x0059`) and then the Data `0x0000`.

7.  **Inspect the scan response packet**
    To inspect a scan response packet, we need the central, i.e your smartphone, to send a scan request packet to the advertiser.

    7.1 Open the nRF Connect for Mobile application and start scanning. This will trigger the phone to automatically send a scan request.
    The `SCAN_REQ` (Scan Request) from the scanner is sent after an advertising packet (in this case on channel 39) and it is followed by a `SCAN_RSP` (Scan Response) from the advertiser. Both `SCAN_REQ` and `SCAN_RSP` is performed on the same channel as the advertising packet it follows.
    7.2 Select the `SCAN_RSP` packet to inspect the contents.
    _Screenshot of Wireshark SCAN_RSP packet details showing URI data._
    You can find the same pattern of the advertising structure here. It starts with the length `0x1A` (26 bytes) followed by type `0x24` (which is URI) and then the actual data which is the URL: `//academy.nordicsemi.com`

8.  **Observe the dynamic data being updated**
    Recall from lesson 2 exercise 2, that we learned how to dynamically change the contents of the advertising data, triggered by pressing button 1.

    8.1 Select any of the advertising packets, `ADV_SCAN_IND`, and observe the Manufacturer Specific data value of `0x0000`.
    _Screenshot of Wireshark showing Manufacturer Specific Data as 0x0000._
    8.2 Now press button 1 on the board acting as the peripheral.
    8.3 Inspect a new advertising packet.
    Observe that the contents of the advertising packets will now be updated, to `0x0100`.
    _Screenshot of Wireshark showing Manufacturer Specific Data updated to 0x0100._
    Each time you press button 1 on the board that is advertising, the value of the Manufacturer Specific Data will be increased by one.
    > **Note**
    > Bluetooth LE uses little endianness to represent the data in GAP and GATT layers, which is why it increments from `0x00 00` to `0x01 00`, etc.

## Exercise 2: Inspect a Bluetooth connection, analyze GAP and GATT packets

_(Note: The exercise text is based on nRF Connect SDK versions v3.0.0 – v3.2.0.)_

In this exercise, we will be sniffing the packets in a Bluetooth LE connection between your Nordic device and your phone. This exercise will help you apply what you learned in Lesson 3 about connection establishment and connection parameters to what actually happens over the Bluetooth LE link.
We will be using the application we made in Lesson 3 Exercise 2 for this exercise.

### Exercise steps

In the GitHub repository for this course, go to the code for this exercise, found in `l6/l6_e2`.
Note that this is the same firmware as the solution to exercise 2 found in `l3/l3_e2_sol`.

1.  **Build and flash the application on your board.**
    LED1 on your board should be blinking, indicating that your board is advertising.

2.  **Filter only the device packets from Nordic_Peripheral.**
    In Wireshark, run the sniffer again and open the Device drop-down list to select `Nordic_Peripheral`. If you only see the name from the previous exercise (Nordic_Beacon), you may need to close Wireshark, and open it again.
    If there are too many devices, you can use the RSSI filter as we did in the previous exercise.
    _Screenshot of Wireshark Device drop-down list._
    After you select `Nordic_Peripheral`, you should only see the packets from this device, in this case, the device address is `cd:f6:1b:aa:5f`. Here we can see mainly advertising packets (`ADV_IND`), and some scan requests (`SCAN_REQ`) and scan response (`SCAN_RESP`) from the same advertiser.
    _Screenshot of Wireshark filtered for "Nordic_Peripheral" showing ADV_IND, SCAN_REQ, SCAN_RSP packets._
    Make sure that the “Automatic Scroll” function is enabled to see the latest packets.
    _Screenshot of Wireshark Automatic Scroll button._

3.  **Establish a Bluetooth LE connection**
    Launch nRF Connect for Mobile and connect to the `Nordic_Peripheral` device, just like we have done several times previously.
    When the connection is established, you should be able to see this from the nRF Sniffer in Wireshark.
    _Screenshot of Wireshark showing a connection established, with many Empty PDU packets._
    Notice a large number of packets being exchanged between the peers at a high speed. Most of them are empty PDU packets.
    > **Note**
    > If you don’t see any empty PDU packets, the sniffer may have missed the connection. This happens if the sniffer fails to capture the connection request packet to follow the connection. Try to connect again, by disconnecting the peripheral from the nRF Connect application, and initiating the connection again.

4.  **Inspect the connection parameter update request packet.**
    Find a packet with the Info: “Connection Parameter Update Request”, from the L2CAP protocol, see below.
    It might be a good idea to turn off “Auto Scroll” to easier navigate through all the captured packets.
    _Screenshot of Wireshark showing an L2CAP Connection Parameter Update Request packet._
    Expand `Bluetooth L2CAP Protocol` and `Command: Connection Parameter Update Request` to find the request minimum and maximum connection interval of 800 (1000 ms) which is what we requested in Lesson 3 Exercise 2.

5.  **Inspect the connection update indication packet (`LL_CONNECTION_UPDATE_IND`).**
    Right after the request, you can see the `LL_CONNECT_UPDATE_IND` packet from the phone. This packet dictates the new updated connection parameters according to what was requested by the peripheral.
    _Screenshot of Wireshark showing an LL_CONNECTION_UPDATE_IND packet._
    Inspecting `LL_CONNECT_UPDATE_IND` will show you the new connection parameters. In this case, we see the connection interval of 800 (1000ms) and the connection timeout of 400 (4 seconds). This corresponds to what was requested in the code. Also, notice the `Instant` value of 177. This value tells the peers at which connection event the new parameters will take effect.
    As we can see from the `Delta time` column of the packets after this, the new connection interval of 1000 ms takes effect at Event number 177.

6.  **Apply a filter to filter out the empty data packets.**
    The majority of the packets you see in the Live Capture are just empty PDU packets that are sent to keep-alive the connection between the peers. The central sends a packet at the beginning of the connection event and the peripheral responds with a packet as an Acknowledge. When they don’t have anything to send, they simply send Empty PDU packets that only contain a data header and no payload. Without these packets being sent and acknowledged, the connection times out after the connection supervision timeout.
    However, these Empty PDU packets make it difficult to inspect the communication, especially when you need to find a certain meaningful packet. The best way of solving this is to create a filter to hide these Empty PDUs.
    Let’s apply the filter by selecting an Empty PDU. Then in the Packet Details window, expand `Data Header` and right-click where it says `Length: 0`. Then click `Apply as a Filter > Not Selected`.
    _Screenshot showing how to apply a Wireshark filter to hide Empty PDU packets by right-clicking on "Length: 0"._
    Alternatively, you can write the filter manually in the “Apply a display filter” text box in the top left corner. To filter out packets with empty length you can enter the following filter command: `!(btle.data_header.length == 0)`
    After the filter has been applied, you will only see the packets that actually have data payload.
    _Screenshot of Wireshark with Empty PDU filter applied, showing only packets with data payload._

7.  **Inspect the communication when connected.**
    Now that all the empty packets are filtered out, let’s inspect the packet exchanges when two devices are connected.
    The following image shows the anatomy of a connection between the phone and the “Nordic_Peripheral” where we enable notification on the button characteristic via the phone and then press and release the button a few times:
    _Screenshot of Wireshark showing a sequence of packets: CONNECT_IND, LL_PHY_REQ, Write Request (to CCCD), Handle Value Notification (for button press/release)._
    Try to reproduce a similar capture log, by subscribing to notifications from the Button Characteristic in nRF Connect for Mobile, like we have done in previous exercises.
    Let’s inspect some of the other packets in the connection lifecycle.

    7.1 Inspect the connection request, `CONNECT_IND`.
    The connection request is sent from your phone to the device, when you select Connect in nRF Connect for Mobile. This is where we will find the initial connection parameters, like the connection interval and the connection supervision timeout.
    _Screenshot of Wireshark CONNECT_IND packet details showing initial connection parameters._
    Here we can see that we get an initial connection interval of 30 ms, which explains the delta time between the empty PDU packet we saw in a previous step. And a connection supervision timeout of 5000 ms.

    7.2 Inspect the PHY request, `LL_PHY_REQ`.
    The packet `LL_PHY_REQ` is the request from the peripheral to use 2M PHY radio mode, where the peripheral informs that it prefers to use the LE 2M PHY (left image). We can also see the response from the central in `LL_PHY_UPDATE_IND`, indicating that LE 2M PHY shall be used (right image).
    _Left: Screenshot of Wireshark LL_PHY_REQ packet. Right: Screenshot of Wireshark LL_PHY_UPDATE_IND packet._
    > **Note**
    > The actual packets exchanged depend on whether the central or peripheral initiates the procedure.
    > If the procedure is initiated by the central, which is what happened in this case, it sends an `LL_PHY_REQ` and the peripheral responds with an `LL_PHY_RESP`, before the central sends the `LL_PHY_UPDATE_IND`.
    > If the procedure is initiated by the peripheral, it sends an `LL_PHY_REQ` and the central responds directly with an `LL_PHY_UPDATE_IND`.

    7.3 Inspect the write request to the CCCD to enable notifications.
    Find the packet that sends a write request with the handle of the CCCD attribute (`0x0013`). Upon inspection, notice that it is setting the notification bit to true, to enable notifications from this characteristic.
    _Screenshot of Wireshark Write Request packet to CCCD (handle 0x0013) with value enabling notifications._

    7.4 Inspect the notifications when the button is pressed and when it’s released.
    You may notice many consecutive “Rcvd Handle Value Notification” packets from the ATT layer in the capture log.
    _Screenshot of Wireshark showing multiple Handle Value Notification packets for button state changes._
    The handle of the notification value attribute is `0x0012`. It’s very common in the attribute table that the value attribute is located right above the CCCD attribute `0x0013`. Also notice that the value of the characteristic when the button is pressed is `0x01` and when the button is released is `0x00`. This is how the app on the phone detects whether to display “Button Pressed” or “Button Released”.

This marks the end of this exercise. You should be able to start using the sniffer to inspect other exercises. For example, the screenshot below is from the capture log of Lesson 4 Exercise 3, where we use NUS service to send UART data from the computer to the phone. In this case, we have typed “Nordic Academy” into a serial terminal and can now see the notification that contains this data being sent to the phone:
_Screenshot of Wireshark showing a NUS notification packet containing the string "Nordic Academy"._

## Exercise 3: Follow and decrypt a paired connection

_(Note: The exercise text is based on nRF Connect SDK versions v3.0.0 – v3.2.0.)_

It can be tricky to follow an encrypted connection, especially when LE Secure Connections is used. Even with Just Works pairing, it’s not possible for the sniffer to automatically decrypt the encrypted connection without a security key provided.
This exercise can be used in supplement to Exercise 1 Lesson 5 Security. If you haven’t completed the security exercise, you can use the provided solution. The solution is available at `l5/l5_e1_sol`.

### Exercise steps

In the GitHub repository for this course, go to the code for this exercise, found in `l6/l6_e3`.
Note that this is the same firmware as the solution to exercise 1 found in `l5/l5_e1_sol`.

1.  **Build and flash the application on your board.**
    LED1 on your board should be blinking, indicating that your board is advertising.

2.  **View the log output from the application.**
    Open up a terminal window to see the log output from the application.

3.  **Filter only the device packets from Nordic_LBS.**
    Close and re-open Wireshark and run the sniffer again. Go to the Device drop-down list to select `Nordic_LBS` (or `Nordic_Peripheral` if the name changed), just like we have done in the previous exercises.

4.  **Connect to the device using your phone.**
    Using the nRF Connect for Mobile application, connect to the device `Nordic_LBS`. Make sure that you can follow the connection with the sniffer.
    Observe in the sniffer, that the connection is established (`CONNECT_IND`). You may want to use the Empty packet filter again to make it easier to follow.

5.  **Trigger bonding by doing a write command to change the LED state.**
    Just like we did in lesson 5, try writing a value to change the LED state, and you will be prompted to write in a 6-digit passkey that is printed in the UART log, like this.
    `Passkey for 4B:DA:4C:3B:E4:E0 (random): xxxxxx`
    When this is complete, you should see the following in the sniffer, look for the three packets highlighted below:
    _Screenshot of Wireshark showing sequence: Sent Write Request, Rcvd Error Response (Insufficient Authentication), Sent Pairing Request._
    *   `Sent Write Request`: The Write Request gets sent when we tried to write to the LED characteristic from the central (your phone).
    *   `Rcvd Error Respond`: The Write Request gets rejected with Insufficient Authentication, because the connection is still open, and not encrypted, at security level 1. Recall from Exercise 1 in Lesson 5, that the requirement for accessing the LED characteristic is at security level 3.
    *   `Sent Pairing Request`: After receiving the rejection, the phone will send a Pairing Request to start the process of upgrading security level of the connection.

    Continue with the pairing process in the sniffer trace, you will see the key is generated and the link is encrypted after the `LL_START_ENC_REQ` packet.
    _Screenshot of Wireshark showing LL_START_ENC_REQ followed by encrypted packets marked "Encrypted packet decrypted incorrectly"._
    Notice that `LL_START_ENC_REQ` is the last packet where the communication is not encrypted. After this packet, the sniffer is not able to decrypt the communication and all the messages from there on out are marked as “Encrypted packet decrypted incorrectly”. Shortly after this, the sniffer won’t able to capture any more encrypted messages. This is because when the sniffer can’t decrypt the communication, it won’t be able to follow the change of timing or channel hopping of the connection.

6.  **Enable Bluetooth LE log information for sniffer.**
    To be able to decrypt the communication, we need to provide the sniffer with the LTK from the connection. The Kconfig symbol `BT_LOG_SNIFFER_INFO` will log the LTK of the connection after pairing so we can provide this key to the sniffer.
    Add the following Kconfig symbols in the `prj.conf` file of the application:
    ```kconfig
    CONFIG_BT_LOG_SNIFFER_INFO=y
    ```

7.  **Build and flash the application on the board.**
    7.1 Make sure you select Erase And Flash To Board, to remove the previous bonding information.
    _Screenshot of IDE option "Erase And Flash To Board"._
    7.2 Remove the bond information from your phone.
    On the phone, you would need to remove the bond information of the device as well.
    _Screenshot of Android Bluetooth settings showing "Unpair" option._

8.  **Perform the same pairing process as we did in step 5.**
    Connect to the device in nRF Connect for Mobile and try to write a value to the LED characteristic to be prompted with the Pairing Request.
    Input the 6-digit passkey and notice in the UART log, that there is an extra line printed, containing the LTK:
    ```
    *** Booting Zephyr OS build v3.7.99-ncs1 ***
    Starting Bluetooth Peripheral LBS example
    Bluetooth initialized
    Advertising successfully started
    Connected
    Passkey for 40:A1:08:FF:EF:95 (random):947467
    Security changed 40:A1:08:FF:EF:95 (random) level 4
    I: SC LTK: 0xe58c6433d0b6fa31cc5593483878ad536
    ```

9.  **Provide the sniffer with the outputted LTK, to decrypt the connection.**
    Back in the sniffer, in the header above the Packet List window, under Key, select `SC LTK` and then copy the LTK from the log output into the `Value` section. Click the small arrow to the right of the `Value` section to apply the changes.
    _Screenshot of nRF Sniffer toolbar in Wireshark, showing Key selection (SC LTK) and Value input field for the LTK._
    > **Note**
    > The LTK is randomly generated after each pairing, so you will have your own LTK that you need to copy and paste into the nRF Sniffer. The example LTK provided here will not work for your connection.

10. **Disconnect and re-connect the device from the phone.**
    In nRF Connect for Mobile, disconnect the device to the phone and then connect to it again. This time the LTK generated in the last connection will be re-used and the phone and the device will automatically re-encrypt the link. The sniffer will use the provided key to decrypt the link.
    To confirm this, open a random packet, expand `nRF Sniffer for Bluetooth LE` and then `Flags` and note that the `Encrypted` flag is set.
    _Screenshot of Wireshark Packet Details showing nRF Sniffer flags with Encrypted set to Yes._
    Observe that there is no new Pairing Request and the central only sends `LL_ENC_REQ` to start re-encrypting the link. After the `LL_START_ENC_REQ` packet the link is encrypted, but this time the sniffer is able to decrypt the link and the `Encrypted Flag` is now changed to `Yes` after the `LL_START_ENC_REQ`.
    Try to send an LED write request from the phone, and it will be observed by the sniffer trace, as shown above.
    If your phone doesn’t support LE Secure Connection and only supports Legacy Pairing, you will need to select `Legacy LTK` instead of `SC LTK` in the Key dropdown menu.