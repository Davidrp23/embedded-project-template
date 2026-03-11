# Pairing process

> **SDK Version Note**
> This lesson is written for nRF Connect SDK v3.0.0-v3.2.0. Board targets use the new format (e.g., `nrf5340dk/nrf5340/cpuapp`). For documentation, refer to [docs.nordicsemi.com](https://docs.nordicsemi.com).
>
> **Bluetooth Core v6.2 Support**
> nRF Connect SDK v3.0.0+ includes support for Bluetooth Core Specification v6.2, which introduces enhanced security features including improved channel sounding security and additional cryptographic capabilities for secure ranging applications.

The most common practice of protecting wireless communication is to encrypt the connection, which converts the data being sent into a form that can only be read by those with the permission to do so.

To encrypt the link, both peers need to have the same keys. The process of generating, distributing, and authenticating these keys for encryption is referred to as the pairing process.

The pairing process requires the devices to repeat the process every time they want to encrypt the link again. In addition to “pairing”, the term “bonding” is used when the peers store the encryption key so they can re-encrypt the link in future connections with the same peer. During bonding, they can also exchange and store identity keys so they can recognize each other for future connections through the random resolvable private address.

**Definition:**
**Pairing:** The process of generating, distributing, and authenticating keys for encryption purposes.
**Bonding:** The process of pairing followed by distribution of keys used to encrypt the link in future reconnections.

Bluetooth LE defines 3 phases in the encryption process.

_Diagram showing the three phases of pairing: Phase 1 (Pairing Feature Exchange), Phase 2 (Authentication and Key Generation), Phase 3 (Transport Specific Key Distribution)._

## Phase 1: Initiate pairing

To initiate the pairing and, in some cases, the bonding process, the central needs to send a Pairing Request and the peripheral responds with a Pairing Response. In this phase, the two devices exchange their pairing features, that will be used to determine what pairing method they will use in phase 2 and what keys are distributed in phase 3.

Most importantly, the peers exchange their I/O (input/output) capabilities, selected from one of the following:

*   **DisplayOnly:** The peer only has a display
*   **DisplayYesNo:** The peer has a display and the option to select “yes” or “no”
*   **KeyboardOnly:** The peer has keyboard only
*   **NoInputNoOutput:** The peer has no input and no output capabilities
*   **KeyboardDisplay:** The peer has keyboard and display capabilities

In addition, they exchange what security features they support, whether or not bonding is requested, and more.

> **Note**
> Only the central can send a Pairing Request. The peripheral, however, can send a Security Request which can trigger a Pairing Request from the central, but it’s not a common practice.

## Phase 2: Perform pairing

In phase 2, the keys used to encrypt the connection are generated. The pairing method used here depends on the information exchanged in phase 1.

In LE Legacy pairing, the peers exchange a Temporary Key (TK) used to generate a Short Term Key (STK) that is then used to encrypt the link. However, since the STK can easily be cracked, Bluetooth v4.2 introduced something called Bluetooth LE Secure Connections. In LE Secure Connections, the devices generate and exchange a more secure type of key, and use it to generate a single Long Term Key (LTK) used to encrypt the connection. See Legacy pairing vs LE Secure Connections later in this lesson for more information on the difference between these two security methods.

Legacy pairing defines three different methods to exchange the TK, called pairing methods. LE Secure Connections supports these three pairing methods but also a fourth (numeric comparison) that is not supported in Legacy pairing. The security of the pairing process depends on which pairing method is used in this phase.

### Pairing methods

*   **Just Works:** Both peers generate the STK based on information exchanged in plain text, and the user is just asked to accept the connection. This method is unauthenticated.
*   **Passkey Entry:** 6-digit number is displayed on one device, and needs to be typed in on the other device. The I/O capabilities of the devices determine which one displays the number and which one inputs it.
*   **Out of Band (OOB):** The encryption keys are exchanged by some other means than Bluetooth LE, for example by using NFC.
*   **Numeric Comparison (LE Secure Connections only):** Both devices display a 6-digit number and the user selects “yes” or “no” to confirm the display.

Which pairing method to use is decided based on the OOB flag, the Man-In-The-Middle (MITM) flag, and the I/O capabilities of the peers, exchanged during phase 1.

The OOB and MITM flags first determine whether to use the OOB pairing method directly or determine the pairing method based on the I/O capabilities.

_Diagram illustrating how OOB and MITM flags on Initiator and Responder determine the pairing method selection process (e.g., Use OOB, Check MITM, Use I/O capabilities, Just Works)._

**Rules for using OOB and MITM flags (based on the provided image):**

| Responder OOB | Responder MITM | Initiator OOB | Initiator MITM | Resulting Action                               |
| :------------ | :------------- | :------------ | :------------- | :--------------------------------------------- |
| Set           | *Any*          | Set           | *Any*          | Use OOB                                        |
| Set           | *Any*          | Not Set       | *Any*          | Use OOB (LE Secure Connections)                |
| Not Set       | *Any*          | Set           | *Any*          | Use OOB (LE Secure Connections)                |
| Not Set       | Set            | Not Set       | Set            | Check MITM (then likely Use I/O capabilities)  |
| Not Set       | Set            | Not Set       | Not Set        | Check MITM (then likely Use I/O capabilities)  |
| Not Set       | Not Set        | Not Set       | Set            | Check MITM (then likely Use I/O capabilities)  |
| Not Set       | Not Set        | Not Set       | Not Set        | Just Works (if no I/O capabilities lead to other methods) / Use I/O capabilities |

Notice that in LE Secure Connections, only one of the peers needs to have the OOB flag set, for this pairing method to be used.

Depending on the OOB and MITM flags, the I/O capabilities of the peers might be used to determine the pairing method. In this case, the following table is used.

_Diagram showing a table that maps Initiator I/O capabilities (Display only, Display Yes No, Keyboard, No Input No Output, Keyboard Display) against Responder I/O capabilities to determine the pairing method (Just Works, Passkey Entry, Numeric Comparison)._

**Mapping of I/O capabilities to key generation method (based on the provided image):**

| Initiator I/O     | Responder I/O     | Pairing Method                                     |
| :---------------- | :---------------- | :------------------------------------------------- |
| Display only      | Display only      | Just Works                                         |
| Display only      | Display Yes No    | Just Works                                         |
| Display only      | Keyboard          | Passkey Entry                                      |
| Display only      | No Input No Output| Just Works                                         |
| Display only      | Keyboard Display  | Passkey Entry                                      |
| Display Yes No    | Display only      | Just Works                                         |
| Display Yes No    | Display Yes No    | Just Works / Numeric Comparison (LE Secure Conn.)    |
| Display Yes No    | Keyboard          | Passkey Entry                                      |
| Display Yes No    | No Input No Output| Just Works                                         |
| Display Yes No    | Keyboard Display  | Passkey Entry / Numeric Comparison (LE Secure Conn.) |
| Keyboard          | Display only      | Passkey Entry                                      |
| Keyboard          | Display Yes No    | Passkey Entry                                      |
| Keyboard          | Keyboard          | Passkey Entry                                      |
| Keyboard          | No Input No Output| Just Works                                         |
| Keyboard          | Keyboard Display  | Passkey Entry                                      |
| No Input No Output| Display only      | Just Works                                         |
| No Input No Output| Display Yes No    | Just Works                                         |
| No Input No Output| Keyboard          | Just Works                                         |
| No Input No Output| No Input No Output| Just Works                                         |
| No Input No Output| Keyboard Display  | Just Works                                         |
| Keyboard Display  | Display only      | Passkey Entry                                      |
| Keyboard Display  | Display Yes No    | Passkey Entry / Numeric Comparison (LE Secure Conn.) |
| Keyboard Display  | Keyboard          | Passkey Entry                                      |
| Keyboard Display  | No Input No Output| Just Works                                         |
| Keyboard Display  | Keyboard Display  | Passkey Entry / Numeric Comparison (LE Secure Conn.) |

The key generated at this phase will be used to encrypt the link after phase 2. If you are only doing pairing, not bonding, then only these 2 phases will be performed and the peers will skip phase 3.

## Phase 3: Key distribution

In this phase, the Long Term Key (LTK) is used to distribute the rest of the keys. In legacy pairing, the LTK is also generated in this phase (in LE Secure Connections, the LTK is generated in phase 2). Other keys are also generated and exchanged in this phase, to identify the peers the next time they re-connect and to be able to re-encrypt the link using the same LTK.

## Legacy pairing vs LE Secure Connections

### Legacy pairing

Prior to Bluetooth v4.2, legacy pairing was the only method of pairing available in Bluetooth LE. It is quite simple and exposes a risk because the Short Term Key (STK) used to encrypt the link can easily be cracked.

When using Just Works in Legacy pairing, the TK is set to 0, which offers no protection in terms of eavesdropping or Man-In-The-Middle (MITM). An attacker can easily brute force the STK and eavesdrop the connection, and there is also no way of verifying the devices.

Using Passkey entry is a bit better, as the TK is now a 6-digit number that is passed between the devices by the user. For example, one of the devices displays the number on its screen, and the user inputs this number on the other devices using a keyboard. Unfortunately, an attacker can easily sniff the values being exchanged, and it is then very easy to figure out the STK and decrypt the connection. Even if it isn’t able to determine the TK directly, the key can easily be cracked by trying all the 999999 combinations.

In Out of Band pairing, the TK is exchanged by some other means than Bluetooth LE, for example, by using NFC. This method supports using a TK as big as 128 bits, which increases the security of the connection. The security of the OOB channel used during the exchange also determines the protection of the Bluetooth LE connection. If the OOB channel is protected from eavesdropping and MITM attacks, so is the Bluetooth LE link.

Legacy pairing is not recommended by the Bluetooth SIG, but if you must use it, use OOB pairing. Out of Band authentication is the sole method that can be considered secure when pairing with legacy pairing.

### LE Secure Connections

For this reason, LE Secure Connections was introduced in Bluetooth v4.2. Instead of using a TK and STK, LE Secure Connections uses Elliptic-Curve Diffie-Hellman (ECDH) cryptography to generate a public-private key pair. The devices exchange their public keys. They will use one of the four pairing methods (Just Works, Passkey entry, OOB or Numeric Comparison) to verify the authenticity of the peer device and generate the LTK based on the Diffie-Hellman key and authentication data.

Although Just Works is more secure when using LE Secure Connections. it still does not offer authentication and is therefore not recommended as a pairing method. The Passkey entry pairing method now uses the passkey, along with the ECDH public key and a 128-bit arbitrary number to authenticate the connection. This means it is much more secure than described in legacy pairing. Using OOB pairing is still a recommended option, as it provides protections as long as the OOB channel is secure, just like in legacy pairing.

Additionally, a new pairing method called Numeric Comparison was introduced with this feature. Although it follows the same procedure as the Just Works pairing method, it adds another step at the end to protect against MITM attacks by having the user perform a manual check based on values displayed on both peers.

The only data exchanged between the peers is the public keys. The use of the ECDH public key cryptography makes it extremely difficult to crack the LTK, and is a significant improvement compared to legacy pairing.

> **Note**
> Even though LE Secure Connections is supported by most devices, there are still some Bluetooth LE products that only support Legacy pairing. Therefore, it is a good idea to enable support for Legacy pairing in addition to LE Secure Connections to achieve better interoperability in your application.

## Security modes

### Security concerns

There isn’t a simple answer to the question “How secure is Bluetooth LE?” It depends heavily on how the pairing process is executed and what I/O capabilities the peer devices have.

There are 3 common types of attacks that Bluetooth LE security must cope with:

1.  **Identity tracking**
2.  **Passive eavesdropping (sniffing)**
3.  **Active eavesdropping (Man-in-the-middle, or MITM)**

Identity tracking exploits the Bluetooth address to track a device. Protecting against such attacks requires privacy protection. This can be done by using a resolvable private address that changes randomly, where only the bonded/trusted devices can resolve the private address. The IRK (Identity resolving key) is used to generate and resolve the private address.

Passive eavesdropping allows an attacker to listen to data being transmitted between devices. This can be protected against by encrypting the communication between the peers. The challenge here is how the peer devices generate and/or exchange the keys to encrypt the connection securely. This was the main drawback that made Bluetooth LE legacy pairing vulnerable, and created the need for LE Secure Connections.

In an active eavesdropping (or man-in-the-middle) attack the attacker impersonates two legitimate devices to fool them into connecting to it. To prevent against this, we need to be sure that the device we are communicating with is in fact the device we want to talk to and not an unauthenticated device.

### Security levels

Bluetooth LE defines 4 security levels in security mode 1:

*   **Level 1:** No security (open text, meaning no authentication and no encryption)
*   **Level 2:** Encryption with unauthenticated pairing
*   **Level 3:** Authenticated pairing with encryption
*   **Level 4:** Authenticated LE Secure Connections pairing with encryption

Each connection starts at security level 1, and then upgraded to a higher security level depending on which pairing method is used.

Using Just Works, either with Legacy Pairing or LE Secure Connections, will bring the connection to security level 2. This method does not protect against MITM attacks, since the link is just encrypted, and not authenticated. To protect against MITM, the connection needs to be at security level 3 or higher. This can be achieved by using either Passkey Entry or OOB pairing method with Legacy pairing, both of which provide authentication bringing security to level 3.

The connection can only get security level 4 if both peers support LE Secure Connections and either Passkey Entry, Numeric Comparison, or OOB authentication method is used.

The Permissions field of a characteristic determines not only whether the attribute is readable and/or writeable, but also the security level required of that connection for the attribute to be accessible. For instance, if a link is encrypted with security level 2, unauthenticated encryption, the peer will not be able to access any characteristic that requires authenticated security of level 3 or 4.

This way, we can configure our attribute table so that the data can only be exchanged when the link is encrypted with a certain level of security.

**More on this:**
Bluetooth LE has a total of 3 security modes. Security mode 2 uses data signing for security and is rarely used. Security mode 3 pertains to isochronous broadcast which is used with Bluetooth LE Audio and is fairly new. This course only focuses on security mode 1.

## Filter Accept List

Filter Accept List, formerly known as Whitelisting, is a way of limiting access to a list of devices in both advertising and scanning.

When used in advertising, only the devices in the Filter Accept List can send a connection request to establish a connection or send a scan request to get the scan response from the advertiser. If a device not on the list sends these requests, the advertiser will ignore the request.

When used in scanning, only the advertising and scan response packet from the devices in the list will be scanned and reported to the application. The scanner will filter out any packet from other advertisers.

By using the peer’s address and the identity keys distributed at phase 3 of the pairing process, we can build the Filter Accept List to only allow a bonded and authorized device to connect to the device. You can decide if a new device can join this list or not by adding a “pairing mode” which temporarily turns off the Filter Accept List and can turn it back on after the pairing is finished. In exercise 2 of this chapter we will have a look at the implementation of this mechanism.

## Exercise 1: Add pairing support to a Bluetooth LE application

In this exercise, we will start with a version of the Bluetooth Peripheral LBS sample that does not have any security support. This is similar to the application we created in Lesson 4 Exercises 1 and 2, where we created our own custom LED Button Service. All characteristics of the service are open and anyone can read and write to them without any encryption. This also means anyone with a sniffer can follow the connection and read the exchanged data.

We will start by adding the encryption requirement to a LED characteristic’s write permission. Then, we will add pairing support to the application and practice encrypting the link to be able to write to the LED characteristic.

The second part of the exercise focuses on increasing the security level to have man-in-the-middle protection, i.e security level 3 and 4. We will add a display callback to display the passkey in the log output so that we can see the passkey and enter the key to the phone. This way, the end user can ensure that they are pairing to the correct device.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l5/l5_e1`.
This is a slightly modified version of the Peripheral LBS sample in nRF Connect SDK. `lbs.c` and `lbs.h` have been moved from the original SDK folder into the applications `src` folder.

**1. Add a security requirement to the LED characteristic**

1.1 Change the LED characteristic permission to require encryption.
In the declaration of the LED characteristic, add encryption requirement by changing `BT_GATT_PERM_WRITE` to `BT_GATT_PERM_WRITE_ENCRYPT`.
This will change the security level requirement from level 1 (no security) to level 2 (unauthenticated encryption) as covered in Security Models topic earlier.
Change the following code in `lbs.c`:
```c
BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED,
                       BT_GATT_CHRC_WRITE,
                       BT_GATT_PERM_WRITE_ENCRYPT, // Changed from BT_GATT_PERM_WRITE
                       NULL, write_led, NULL),
```

**2. Build and flash the application to the board.**

**3. Try to write to the LED characteristic.**

In nRF Connect for Mobile, connect to the device `Nordic_LBS`. Try to write to the LED characteristic to turn it on, as we have done in previous exercises. Notice that the LED3 on the board does not react. This is because the characteristic now requires encryption, but the firmware does not have pairing support. The phone may terminate the connection because of this.
Let's add pairing support to the firmware.

**4. Add the Security Management Protocol layer to the Bluetooth LE stack.**

The Kconfig symbol `CONFIG_BT_SMP` will add the Security manager Protocol to the Bluetooth LE stack, which is the layer that makes it possible to pair devices over Bluetooth LE.
Add the following line to the `prj.conf` file:
```kconfig
CONFIG_BT_SMP=y
```

**5. Add a callback function for when the security level of the connection has changed.**

Recall the connection callback structure `struct bt_conn_cb` that we used in the previous exercises. Let’s add a callback for the `security_changed` event as well.

5.1 Add the `security_changed` member to the callback structure.
Add the following line in `main.c` (inside `connection_callbacks` struct initialization):
```c
    .security_changed = on_security_changed,
```

5.2 Define the callback function `on_security_changed()`.
We want this callback function to display the current security level of the connection and inform if the link has been encrypted successfully or not.
Add the following code in `main.c` (ensure `on_security_changed` is declared if defined after `connection_callbacks`):
```c
static void on_security_changed(struct bt_conn *conn, bt_security_t level,
                                enum bt_security_err err)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (!err) {
		LOG_INF("Security changed: %s level %u", addr, level); // Corrected newline
	} else {
		LOG_WRN("Security failed: %s level %u err %d", addr, level, // Corrected newline and log level
			err);
	}
}
```

**6. Build and flash the application to the board.**

Open up a terminal to see the log output from the application, which we will need to get the passkey.
Just like in previous exercises, you can go to Visual Studio Code and select the COM port for the device.

**7. Try to write to the LED characteristic.**

In nRF Connect for Mobile, connect to the device `Nordic_LBS`. Try to write to the LED characteristic to turn it on.
Verify that a pairing pop-up appears after you click Write. What happens here is that the phone automatically sends a pairing request after it get rejected when it try to write to the characteristic due to insufficient authentication.
Select Pair to accept the pairing and the LED characteristic should now be possible to control. The Bluetooth LE link is now encrypted.
In the log, you should see the security level updated to level 2 in the UART log:
```
*** Booting nRF Connect SDK ***
Starting Lesson 5 - Exercise 1
Bluetooth initialized
Advertising successfully started
Connected
Security changed 7B:9E:28:DB:38:7A (random) level 2
```

**8. Change the LED characteristic permission to require pairing with authentication**

In the declaration of the LED characteristic, add the authentication requirement by changing `BT_GATT_PERM_WRITE_ENCRYPT` to `BT_GATT_PERM_WRITE_AUTHEN`.
Change the following code in `lbs.c`:
```c
BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED,
                       BT_GATT_CHRC_WRITE,
                       BT_GATT_PERM_WRITE_AUTHEN, // Changed from BT_GATT_PERM_WRITE_ENCRYPT
                       NULL, write_led, NULL),
```
This will increase the security level of the write permission of this characteristic from level 2 to level 3 or 4, depending in whether you are using legacy pairing or LE Secure Connections.
At this stage, even though you would still be able to pair with the board, the phone wouldn’t be able to control the LED. This is because the security level of the application doesn’t meet the requirement of the characteristic permission.

**9. Define authentication callback functions**

We have the authenticated pairing callback structure `struct bt_conn_auth_cb` with numerous members. In our case, we will only add two.

9.1 Define the callback function `auth_passkey_display`.
Let’s define a function for the `passkey_display` event, which has the signature: `void (*passkey_display)(struct bt_conn *conn, unsigned int passkey);`
This will print the passkey needed for the central (your phone) to pair with the peripheral (the board).
Add the following code in `main.c`:
```c
static void auth_passkey_display(struct bt_conn *conn, unsigned int passkey)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_INF("Passkey for %s: %06u", addr, passkey); // Corrected newline
}
```

9.2 Define the callback function `auth_cancel`.
Let’s define a function for the `cancel` event, which has the signature: `void (*cancel)(struct bt_conn *conn);`
This will let us know when the pairing has been cancelled.
Add the following code in `main.c`:
```c
static void auth_cancel(struct bt_conn *conn)
{
	char addr[BT_ADDR_LE_STR_LEN];
	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));
	LOG_WRN("Pairing cancelled: %s", addr); // Corrected newline and log level
}
```

9.3 Declare the authenticated pairing callback structure `struct bt_conn_auth_cb`.
Let’s now declare the callback structure with the two member functions that we created in the previous steps.
Add the following code to `main.c` (ensure callbacks are declared/defined before this):
```c
static struct bt_conn_auth_cb conn_auth_callbacks = {
	.passkey_display = auth_passkey_display,
	.cancel = auth_cancel,
};
```

**10. Register the authentication callbacks.**

Add the following code in `main.c` (ensure `err` is declared):
```c
// int err; // if not already declared
err = bt_conn_auth_cb_register(&conn_auth_callbacks);
if (err) {
	LOG_ERR("Failed to register authorization callbacks."); // Corrected newline
	return -1; // or handle error
}
```
The application now fulfills the requirements of pairing with MITM authentication needed for security level 3 or level 4.

**11. Build and flash the application on the board.**

11.1 Make sure you select Erase And Flash To Board, to remove the previous bonding information.
_Description: IDE option "Erase And Flash To Board" highlighted._

11.2 Remove the bond information from your phone.
On the phone, you would need to remove the bond information of the device as well.
_Description: Android Bluetooth settings showing "Unpair" option for a device._

**12. Use your phone to connect the device and prompt the pairing request by writing to the LED characteristic.**

As we have done previously, use nRF Connect for Mobile to connect to the device. Then try to write to the LED characteristic and the pairing pop-up window should appear, now with a PIN request.
The passkey, a 6-digit randomly generated number, will be in the log output from the device. You will need to type this passkey to the pop-up window on the phone.
In the log output below, the passkey is `043166`.
```
*** Booting nRF Connect SDK ***
Starting Lesson 5 - Exercise 1
Bluetooth initialized
Advertising successfully started
Connected
Passkey for 48:18:67:01:CC:A8 (random): 043166
```
If you enter the passkey correctly, the phone and the device will pair successfully and you will be able to control the LED.
Verify in the log output that you now have security level 3 (for legacy pairing) or 4 (for LE Secure Connections).
```
Connected
Passkey for 48:18:67:01:CC:A8 (random): 043166
Security changed 48:18:67:01:CC:A8 (random) level 4
```

## Exercise 2: Implement bonding and a Filter Accept List

_(Note: This exercise is written for nRF Connect SDK v3.0.0-v3.2.0.)_

In this exercise, we will add storing keys to the application. Then we will use the stored keys to limit the access to only devices previously paired with the peripheral (your board). This is called an Filter Accept List. The peripheral processes only the connection requests from devices in the Filter Accept List. Requests from other devices will be ignored. This makes our device exclusively available to bonded devices.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l5/l5_e2`.

**1. Add bonding support to the application**

While walking through exercise 1, you may have noticed that if you disconnected your phone from the peripheral and then reconnected, you could re-encrypt the link without pairing again. But if we reset the device and tried to reconnect, we needed to pair again. This is because bonding is supported in SMP by default (through `CONFIG_BT_BONDABLE`).
However, key storing and restoring requires writing data to flash and restoring it from flash. So you will need to include the Bluetooth setting which handles flash, in your application, to be able to store and restore the bond information to flash, through `CONFIG_BT_SETTINGS`.

1.1 Add setting support in your application to store data in flash.
Enable `CONFIG_BT_SETTINGS`, and its dependency `CONFIG_SETTINGS`, to make sure the Bluetooth stack takes care of storing (and restoring) the pairing keys.
Add the following lines to the `prj.conf` file:
```kconfig
# For All other DKs (and generally needed)
CONFIG_SETTINGS=y
CONFIG_BT_SETTINGS=y
CONFIG_FLASH=y
CONFIG_FLASH_PAGE_LAYOUT=y
CONFIG_FLASH_MAP=y
CONFIG_NVS=y
```

1.2 Include the header file in `main.c`:
```c
#include <zephyr/settings/settings.h>
```

1.3 Call `settings_load()` after initializing Bluetooth so that previous bonds can be restored.
As per the documentation for `CONFIG_BT_SETTINGS`, we must call the API function `settings_load()` manually to be able to store the pairing keys and configuration persistently.
The signature is: `int settings_load(void);` or `int settings_load_subtree(const char *subtree);`
Add the following line in `main.c` (typically after `bt_enable()` and before starting advertising or registering auth callbacks, but the exact placement might depend on when settings are needed):
```c
    // After bt_enable() if it's non-blocking and uses a callback,
    // or after it returns if blocking.
    // If using a bt_ready callback, settings_load might be called there.
    // For simplicity here, assuming it's in main() after bt_enable() succeeds.
    if (bt_is_ready()) { // Or a similar check
        settings_load();
    }
```

1.4 Build and flash the application to your board.

1.5. Verify that after you pair your phone with the device, you can reset the hardware and can connect to the same phone again without having to pair again. This means the information was stored during the first pairing, i.e the devices were bonded.

**2. Delete the stored bond**

Our application can now store bond information. But it should be able to remove the stored bond information when needed as well. We can do this by calling `bt_unpair()`, which has the signature: `int bt_unpair(uint8_t id, const bt_addr_le_t *addr);`

2.1 Add an extra button handling to remove bond information. In this case, we will use button 2.
Add the following line in `main.c`:
```c
#define BOND_DELETE_BUTTON             DK_BTN2_MSK
```

2.2 Call `bt_unpair()` to erase all bonded devices.
We want to erase all bonded devices whenever button 2 is pressed, by calling `bt_unpair()` with `BT_ID_DEFAULT` as the address. This will erase all bonded devices. If you want to erase one single device in the bond list, you would need to input the address of the device you want to delete.
Add the following code in the `button_changed()` function in `main.c`:
```c
    if (has_changed & BOND_DELETE_BUTTON) {
        uint32_t bond_delete_button_state = button_state & BOND_DELETE_BUTTON;
        // Typically, action is on button press (state becomes 1 if active high, or 0 if active low and pressed)
        // Assuming button_state is 0 when pressed for active low buttons (common on DKs)
        if (bond_delete_button_state == 0) { // Or adjust based on your button logic
            int err = bt_unpair(BT_ID_DEFAULT, BT_ADDR_LE_ANY);
            if (err) {
                LOG_ERR("Cannot delete bond (err: %d)", err); // Corrected log level
            } else {
                LOG_INF("Bond deleted successfully"); // Corrected newline
            }
        }
    }
```

2.3 Build and flash the application to your board.

2.4. Connect your smart phone to the device then disconnect and press button 2 to erase all bonded devices. If you try connecting again, you will notice that the phone can't re-encrypt the link. This usually results in the connection being dropped. You would need to remove the bond on the phone (in Bluetooth settings -> Forget this device) before you connect again to be able to make a new pairing.

**3. Add a Filter Accept List to the application.**

Now that we have the bond information stored in flash, we will use it to create a Filter Accept List that will only allow the devices on this list to connect to the peripheral.

3.1 Enable support for Filter Accept List and Privacy Features.
Let’s enable the Filter Accept List API (`CONFIG_BT_FILTER_ACCEPT_LIST`) and the Private Feature support, which makes it possible to generate and use Resolvable Private Addresses (`CONFIG_BT_PRIVACY`).
Add the following lines in the `prj.conf` file:
```kconfig
CONFIG_BT_FILTER_ACCEPT_LIST=y
CONFIG_BT_PRIVACY=y
```

3.2 Add new advertising parameters based on the Filter Accept List.
We want our advertising packet to depend on whether or not we are using the Filter Accept List. Let’s create two different ones using the `BT_LE_ADV_PARAM` helper macro that we used in Lesson 2.
The `BT_LE_ADV_PARAM()` helper macro signature: `BT_LE_ADV_PARAM(options, interval_min, interval_max, peer)`

3.2.1 Define the advertising parameter `BT_LE_ADV_CONN_NO_ACCEPT_LIST` for when the Filter Accept List is not used.
For options, we want the advertising to be connectable by using `BT_LE_ADV_OPT_CONNECTABLE`. We also want to use `BT_LE_ADV_OPT_ONE_TIME`, so the device will not automatically advertise after disconnecting. So that we can do advertise with the new Filter Accept List after it’s bonded to the first device.
Define in `main.c` or a relevant header:
```c
#define BT_LE_ADV_CONN_NO_ACCEPT_LIST  BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE|BT_LE_ADV_OPT_ONE_TIME, \
                                       BT_GAP_ADV_FAST_INT_MIN_2, \
                                       BT_GAP_ADV_FAST_INT_MAX_2, NULL)
```

3.2.2 Define the advertising parameter `BT_LE_ADV_CONN_ACCEPT_LIST` for when the Filter Accept List is used.
This will be similar to the previous one. However, we will use `BT_LE_ADV_OPT_FILTER_CONN` to filter out the connection requests from devices not in the list.
Define in `main.c` or a relevant header:
```c
#define BT_LE_ADV_CONN_ACCEPT_LIST BT_LE_ADV_PARAM(BT_LE_ADV_OPT_CONNECTABLE|BT_LE_ADV_OPT_FILTER_CONN|BT_LE_ADV_OPT_ONE_TIME, \
                                       BT_GAP_ADV_FAST_INT_MIN_2, \
                                       BT_GAP_ADV_FAST_INT_MAX_2, NULL)
```

3.3 Define the function `setup_accept_list()` to loop through the bond list and add the addresses to the Filter Accept List.

3.3.1 Define the callback `setup_accept_list_cb` to add an address to the Filter Accept List.
Define the callback function that will be called for every iteration of the bond list to add the peer address to the Filter Accept List, using `bt_le_filter_accept_list_add()`, which has the signature: `int bt_le_filter_accept_list_add(const bt_addr_le_t *addr);`
Add the following code snippet in `main.c`:
```c
static void setup_accept_list_cb(const struct bt_bond_info *info, void *user_data)
{
	int *bond_cnt = user_data;
	if ((*bond_cnt) < 0) { // Stop if a previous add failed
		return;
	}
	int err = bt_le_filter_accept_list_add(&info->addr);
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(&info->addr, addr_str, sizeof(addr_str));
	LOG_INF("Attempting to add peer %s to Filter Accept List", addr_str); // Corrected log
	if (err) {
		LOG_ERR("Cannot add peer %s to Filter Accept List (err: %d)", addr_str, err); // Corrected log
		(*bond_cnt) = -EIO; // Indicate error
	} else {
		(*bond_cnt)++;
	}
}
```

3.3.2 Define a function to iterate through all existing bonds, using `bt_foreach_bond()` and then call `setup_accept_list_cb()`.
We can use the function `bt_foreach_bond()` to iterate through all existing bonds. Signature: `void bt_foreach_bond(uint8_t id, bt_bond_cb_t func, void *user_data);`
For each bond, we will call `setup_accept_list_cb()` to add the peer address into the Filter Accept List.
Add the following function in `main.c`:
```c
static int setup_accept_list(uint8_t local_id)
{
	int err = bt_le_filter_accept_list_clear();
	if (err) {
		LOG_ERR("Cannot clear Filter Accept List (err: %d)", err);
		return err;
	}
	int bond_cnt = 0;
	bt_foreach_bond(local_id, setup_accept_list_cb, &bond_cnt);
	return bond_cnt; // Returns number of successfully added bonds or negative error code
}
```

3.4.1 Define the function `advertise_with_acceptlist()` to begin advertising with the Filter Accept List.
The next step is to make a work queue function that will call `setup_accept_list()` to build the Filter Accept List, and then to start advertising with the Filter Accept List (if it is non-empty), otherwise, advertise with no Filter Accept List.
In this function, we will advertise with `BT_LE_ADV_CONN_NO_ACCEPT_LIST` if the list is empty. This will do open advertising, but the advertising will not automatically restart when disconnected as explained earlier at step 3.2.
This way we will be able to change advertising setting to use Filter Accept List when disconnected.
Notice how it’s defined as a work queue thread instead of a normal function. The reason for it is explained at Step 3.5.
Add the following code in `main.c` (ensure `ad` and `sd` advertising data arrays are defined):
```c
// Assuming ad and sd are globally defined advertising data structures
void advertise_with_acceptlist(struct k_work *work)
{
	int err=0;
	int allowed_cnt = setup_accept_list(BT_ID_DEFAULT);
	if (allowed_cnt < 0){
		LOG_ERR("Acceptlist setup failed (err:%d)", allowed_cnt); // Corrected log
	} else {
		if (allowed_cnt == 0){
			LOG_INF("Advertising with no Filter Accept list");
			err = bt_le_adv_start(BT_LE_ADV_CONN_NO_ACCEPT_LIST, ad, ARRAY_SIZE(ad),
					sd, ARRAY_SIZE(sd));
		}
		else {
			LOG_INF("Acceptlist setup, number of devices = %d", allowed_cnt); // Corrected log
			err = bt_le_adv_start(BT_LE_ADV_CONN_ACCEPT_LIST, ad, ARRAY_SIZE(ad),
				sd, ARRAY_SIZE(sd));
		}
		if (err) {
		 	LOG_ERR("Advertising failed to start (err %d)", err); // Corrected log
			return;
		}
		LOG_INF("Advertising successfully started");
	}
}
K_WORK_DEFINE(advertise_acceptlist_work, advertise_with_acceptlist);
```

3.4.2 Submit this work queue in `main`, right before the original advertising code (or replacing it).
In `main()`:
```c
    // Remove or comment out previous bt_le_adv_start()
    k_work_submit(&advertise_acceptlist_work);
```

3.4.3 Remove the original advertising code that advertises without using Filter Accept List.
(This step is covered by 3.4.2 if the old code is removed/commented).

3.5 Submit the same work queue in the `on_disconnected()` callback.
Submit the work queue for `advertise_acceptlist()` in the callback for a disconnected event. So if there is any new bond we will have it updated to the Filter Accept List upon disconnection.
In `on_disconnected()`:
```c
    // Inside on_disconnected()
    k_work_submit(&advertise_acceptlist_work);
```
We submit a work item here because if we call `bt_le_adv_start()` directly in callback we will receive an `-ENOMEM` error (-12). This is because in the `disconnected()` callback, the connection object is still reserved for the connection that was just terminated. If we start advertising here, we will need to increase the maximum number of connections (`CONFIG_BT_MAX_CONN`) by one., in our case, it would need to be two. In order to save on RAM, we submit a work item using `k_work_submit` that will delay starting advertising until after the callback has returned.

3.6 Verify if the Filter Accept List works as intended.
To perform this step, you will need an extra phone or a central device to verify if the Filter Accept List works or not. We will start by erasing the bond information on both the Nordic hardware and your phone that has previously been connected to the device. Next, connect and bond the device with your phone, as we have done previously, and make sure you can control the LED from your phone. Then disconnect.
Now use another central device to connect to the Nordic device. If the Filter Accept List works as it should, you will not be able to connect with this new central device. Now use the original phone that you have bonded to the Nordic device and you should be able to connect.
This shows that the Filter Accept List now works as expected. The device only accepts connections from the previously bonded phone.

**4. Add “Pairing mode” to allow new devices to be added to the Filter Accept List**

The code we have built so far, only allows a single device to be added to the Filter Accept List.
To allow new devices to connect and bond, we need to add an option to do “open advertising”, often called “Pairing mode”. We will use a button press to enable “Pairing mode”.

4.1 Increase the number of maximum paired devices.
Increase the number of paired devices allowed at one time, using `CONFIG_BT_MAX_PAIRED`, which has a default value of 1.
Let’s increase this to 5, by adding the following line in the `prj.conf` file:
```kconfig
CONFIG_BT_MAX_PAIRED=5
```

4.2.1 Add the button handling code to enable pairing mode.
Define a new button for “pairing mode”. We will use button 3.
In `main.c`:
```c
#define PAIRING_BUTTON             DK_BTN3_MSK
```

4.2.2 Add the following code to `button_changed()` callback.
When button 3 is pressed, we want to stop advertising, clear the Filter Accept List and start to advertise with the advertising parameters we made for the case with no Filter Accept List.
Add the following code in `main.c` (inside `button_changed()`):
```c
    if (has_changed & PAIRING_BUTTON) {
        uint32_t pairing_button_state = button_state & PAIRING_BUTTON;
        // Assuming button_state is 0 when pressed for active low buttons
        if (pairing_button_state == 0) { // Or adjust based on your button logic
            int err_code = bt_le_adv_stop();
            if (err_code) {
                LOG_ERR("Cannot stop advertising err= %d", err_code); // Corrected log
                return;
            }
            err_code = bt_le_filter_accept_list_clear();
            if (err_code) {
                LOG_ERR("Cannot clear accept list (err: %d)", err_code); // Corrected log
            } else {
                LOG_INF("Filter Accept List cleared successfully"); // Corrected newline
            }
            err_code = bt_le_adv_start(BT_LE_ADV_CONN_NO_ACCEPT_LIST, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
            if (err_code) {
                LOG_ERR("Cannot start open advertising (err: %d)", err_code); // Corrected log
            } else {
                LOG_INF("Advertising in pairing mode started");
            }
        }
    }
```
Note that if you want to advertise with an Filter Accept List again you have two options: either reset the device or make a new bond then disconnect.
> **Note**
> The button should only be pressed when the device is advertising and not connected to a peer. It is possible to set a flag, so that if the device is in a connection it will return to open advertising once it is disconnected. But this is not covered in the scope of this exercise.

4.3 Verify the pairing mode.
You will need an extra phone or a central device to perform this step. Start by erasing the bond information on the Nordic hardware and your phone that was previously connected to the device. Next, connect and bond the device with your phone, as we have done previously, and ensure you can control the LED from your phone. Then disconnect.
Now use another central device to try and connect to the Nordic device. Just like we saw previously, you should not be able to connect.
Now press button 3 to enable "Pairing mode". When in "Pairing mode", try to use the second phone to connect again. This time it should be able to connect, and you can perform pairing with this phone. After this, your Filter Accept List will contain 2 devices.