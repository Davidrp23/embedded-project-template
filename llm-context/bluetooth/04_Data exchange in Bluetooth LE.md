# GATT operations

As we’ve seen, the GATT layer defines services and characteristics, made up of attributes, that are stored in the GATT server. In this lesson we will discuss data exchange in Bluetooth LE which refers to the operations that are executed between the server and the client to learn about attributes and exchange their values in accordance with attributes permissions.

The server can either send data directly to the client or the client can poll the data from the server. But for the client to know what to request from the server, it needs to know what services and characteristics the GATT server offers. The client will therefore perform service discovery at the beginning of the connection, to learn about the services and characteristics of the server, before performing any operations to access them.

**Definition:**
**Service discovery:** The process in which a GATT client discovers the services and characteristics in an attribute table hosted by a GATT server.

## Data access

Recall that the communication here is based on a client-server architecture where the server holds the data and can either send it directly to the client or the client can poll the data from the server. Hence, GATT operations are classified into client-initiated operations and server-initiated operations.

### Client-initiated operations

Client-initiated operations are GATT operations where the client requests data from the GATT server. The client can request to either read or write to an attribute, and in the case of the latter, it can choose whether to receive an acknowledgment from the server.

We will take a closer look at these operations in exercise 1 of this lesson.

*   **Read**
    If a client wishes to read a certain value stored in an attribute on a GATT server, the client sends a read request to the server. To which the server responds by returning the attribute value.

*   **Write**
    If the client wishes to write a certain value to an attribute, it sends a write request and provides data that matches the same format of the target attribute. If the server accepts the write operation, it responds with an acknowledgement.

*   **Write without response**
    If this operation is enabled, a client can write data to an attribute without waiting for an acknowledgment from the server. This is a non-acknowledged write operation that can be used when quick data exchange is needed.

### Server-initiated operations

The other category of GATT operations are server-initiated operations, where the server sends information directly to the client, without receiving a request first. In this case, the server can either notify or indicate.

We will take a closer look at server-initiated operations in exercise 2 of this lesson.

*   **Notify**
    A Notify operation is used by the server to automatically push the value of a certain attribute to the client, without the client asking for it. This, for example, can be used to update the client about a certain sensor reading which has changed recently.
    Notifications require no acknowledgment back from the client.

*   **Indicate**
    Similar to the Notify operation, Indicate will also push the attribute value directly to the client. However, in this case, an acknowledgment from the client is required. Because of the acknowledgement requirement, you can only send one Indication per connection interval, meaning Indications are slower than notifications.

Although these operations are initiated by the server, the client is required to enable them first by subscribing to the characteristic and enabling either notifications or indications. This will be explained more in the next topic when we discuss the client characteristic configuration descriptor.

## Services and characteristics

As discussed in previous topics, the ATT layer defines attributes and how data is exposed between a client and a server. As such, one of the main functions of GATT is the hierarchal structuring of attributes stored in a GATT server into standardized entities (services and characteristics) providing seamless interoperability between different Bluetooth LE devices.

### Attributes

The ATT layer defines how data is stored and accessed in a server’s database. The data is stored in the form of data structures called Attributes. Attributes are the core data units that both the ATT and GATT layers are based on. Attributes hold user data as well as metadata describing the attribute itself, its type, security permissions, etc. Data exchange occurring between ATT servers and clients, or, GATT servers and clients, is in the form of attributes. When discussing only attributes, they are said to be stored in an ATT server. Whereas, as we will see further in this lesson, when we start classifying attributes into services and characteristics, we refer to that data structure as a GATT server.

An attribute consists of 4 blocks of data:

_An attribute structure typically contains:_
*   **Handle:** A 16-bit unique index to a specific attribute in the attribute table, assigned by the stack. An attribute is addressed via its handle. You can think of it as the row number in the attribute table, although handles are not necessarily sequential.
*   **Type (UUID):** Universally unique ID (UUID), which tells us the attribute type. For example, if this attribute declares a characteristic, this will be reflected in its Type field as it will hold a UUID used specifically to indicate declaring a characteristic.
*   **Permissions:** The security level required (encryption and/or authorization) to handle that attribute, in addition to indicating whether it’s a readable and/or writeable attribute.
*   **Value:**
    *   The actual user data (ex: sensor reading) that is stored in the attribute. This field accepts any data type. It can hold a heart rate monitor value (beats per minute), a temperature reading, or even a string.
    *   It can also hold information (metadata) about another attribute, as we will see later in this lesson.

### Universally unique ID (UUID)

A UUID is an abbreviation you will see a lot in the Bluetooth LE world. It is a unique number used to identify attributes and tells us about their significance. UUIDs have two types.

The first type is the SIG-defined 16-bit UUID. For example, the SIG-defined Heart rate service has the UUID `0x180D` and one of its enclosed characteristics, the Heart Rate Measurement characteristic, has the UUID `0x2A37`. The 16-bit UUID is energy and memory efficient, but since it only provides a relatively limited number of unique IDs, there is a need for more UUID to cover all vendors, users, and use cases.

The second type is a 128-bit UUID, sometimes referred to as a vendor-specific UUID. This is the type of UUID you need to use when you are making your own custom services and characteristics. It looks something like this: `4A98-xxxx-1CC4-E7C1-C757-F1267DD021E8` and is called the “base UUID”. The four x’s represent a field where you will insert your own 16-bit IDs for your custom services and characteristics and use them just like a predefined UUID. This way you can store the base UUID once in memory, forget about it, and work with 16-bit IDs as normal.

### Services

Let’s start by examining what constitutes a service and how attributes are hierarchically structured in a given service. As shown in the below figure, attributes are the main building blocks for services. A service definition (commonly referred to as a service) is comprised of multiple attributes arranged in a GATT-specified format which facilitates standardized data exchange between Bluetooth LE devices.

_A service is formed by attributes, including declarations for the service itself and its characteristics._

#### Service declaration attribute

Service definitions always start with a service declaration attribute. This attribute holds metadata about the service, it also indicates the beginning of a service in the sequence of services stored on a GATT server.

_A service declaration attribute structure includes:_
*   **Handle:** A unique identifier for the attribute.
*   **Type (UUID):** `0x2800` (SIG-defined UUID for primary service declaration).
*   **Permissions:** Typically "read only" and no authentication needed.
*   **Value:** The UUID of the service being declared (e.g., `0x180D` for Heart Rate Service).

The Handle is similar to a row number by which the attribute is addressed. The service declaration attribute’s Type field holds the UUID (`0x2800`) which is a unique SIG-defined UUID used only to indicate the beginning of a service.

The Permissions field here indicates “read only” and no authentication needed. This is expected in a service declaration attribute as there is no reason to have a write-permission for it, it only declares the beginning of a service.

Lastly, the Value field holds the UUID of the service being declared. For example, the Heart Rate Service is a SIG-defined service and is referred to by the UUID `0x180D` which is stored in the Value field of the Heart Rate Service-Service declaration attribute.

### Characteristics

Subsequently, a service can have zero or more characteristic definitions (commonly referred to as characteristics). A characteristic is comprised of at least two attributes and optionally more.

_Attributes forming a characteristic typically include a declaration, a value, and optional descriptors._

Similar to a service definition, a characteristic definition starts with a declaration attribute, to indicate the beginning of a characteristic in the sequence of characteristics in a service definition. This is followed by the characteristic value attribute which holds the actual user data. Optionally, a characteristic can also have one or more characteristic descriptor attributes.

*   **Characteristic declaration attribute:** Holds metadata about the Characteristic Value Attribute.
*   **Characteristic value attribute:** Holds the actual user data.
*   **Characteristic descriptor attribute (optional):** Holds more metadata about the characteristic.

#### Characteristic declaration attribute

A characteristic definition starts with a characteristic declaration attribute, to indicate the beginning of a characteristic in the sequence of characteristics in a service definition. The characteristic declaration attribute’s Type field holds the UUID (`0x2803`) used only to declare a characteristic. The declaration attribute has read-only Permissions, ensuring that clients can read the value but not write to it.

_A characteristic declaration attribute structure includes:_
*   **Handle:** A unique identifier for the attribute.
*   **Type (UUID):** `0x2803` (SIG-defined UUID for characteristic declaration).
*   **Permissions:** Typically "read only".
*   **Value:** Contains characteristic properties, value handle, and characteristic UUID.

The Value field holds important information about the characteristic being declared, specifically three separate fields:

1.  **Characteristic properties:** What kind of GATT operations are permitted on this characteristic.
2.  **Characteristic value handle:** The handle (address) of the attribute that contains the user data (value), i.e the characteristic value attribute.
3.  **Characteristic UUID:** The UUID of the characteristic being declared.

#### Characteristic value attribute

After the attribute declaring the characteristic comes the characteristic value attribute. This is where the actual user data is stored. Its Handle and Type are the ones referred to in the Characteristic Declaration Attribute Value field. Naturally, its Value field is where the actual user data is stored. The Permissions field indicates whether the client can read and/or write to this attribute.

_A characteristic value attribute structure includes:_
*   **Handle:** A unique identifier for the attribute.
*   **Type (UUID):** The UUID of the characteristic (e.g., a custom 128-bit UUID).
*   **Permissions:** Read and/or write permissions as defined.
*   **Value:** The actual user data.

#### Characteristic descriptors

The characteristic descriptor attributes are optional. They hold additional metadata about the characteristic, giving the client more information about the nature of the characteristic. There are several kinds of descriptors, but they are generally divided into two categories, GATT-defined and custom.

_A characteristic descriptor attribute structure includes:_
*   **Handle:** A unique identifier for the attribute.
*   **Type (UUID):** The UUID of the descriptor type (e.g., `0x2902` for CCCD).
*   **Permissions:** Read and/or write permissions as defined for the descriptor.
*   **Value:** Data specific to the descriptor type.

Descriptors also allow the client to set permissions for certain server-initiated GATT operations. In this course, we will focus on the GATT-defined Client Characteristic Configuration Descriptor (CCCD) as it is the most commonly used.

##### Client characteristic configuration descriptor (CCCD)

The Client characteristic configuration descriptor (CCCD) is a specific type of characteristic descriptor that is necessary when the characteristic supports server-initiated operations (i.e Notify and Indicate). This is a writable descriptor that allows the GATT client to enable and disable notifications or indications for that characteristic. The GATT client can subscribe to the characteristic that it wishes to receive updates about, by enabling either Indications or Notifications in the CCCD of that specific characteristic.

For example, in the Heart Rate Service, there is a characteristic called the Heart Rate Measurement. The GATT client (your mobile phone for instance) can use the CCCD of this characteristic to receive updates about this characteristic. So it subscribes to the Heart Rate Measurement characteristic by enabling either Indications or Notifications in the CCCD of said characteristic. This means the GATT server (most likely a heart rate sensor device) will push these measurements to your phone, without your phone having to poll for these measurements.

The CCCD attribute’s format is as pictured below. The UUID for CCCDs is `0x2902`. A CCCD must always be readable and writable. Descriptors with the Type CCCD only have 2 bits in their Value field. The first bit signals whether Notifications are enabled, and the second bit is for Indications.

_A Client Characteristic Configuration Descriptor (CCCD) attribute structure includes:_
*   **Handle:** A unique identifier for the attribute.
*   **Type (UUID):** `0x2902` (SIG-defined UUID for CCCD).
*   **Permissions:** Must be readable and writable.
*   **Value:** A 2-bit field where bit 0 enables/disables notifications, and bit 1 enables/disables indications. (e.g., `0x0001` for Notify, `0x0002` for Indicate).

We will take a closer look at how to do this in exercise 2 of this lesson.

## Attribute table

To best visualize how attributes are stored in a GATT server, let’s examine an example attribute table. An attribute table is how attributes are stored in the GATT server. The attribute table below is derived from a custom service that we will create in exercise 2 of this lesson.

**my_lbs attribute table**

| Description                                    | Handle | UUID                                         | Attribute Permissions | Attribute Value                                                                                                                                            |
| :--------------------------------------------- | :----- | :------------------------------------------- | :-------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **my_lbs Service Declaration**                 | 0x0001 | `0x2800` (Primary Service)                   | Read                  | `00001523-1212-efde-1523-785feabcd123` (UUID of my_lbs service)                                                                                             |
| Button Characteristic Declaration              | 0x0002 | `0x2803` (Characteristic)                    | Read                  | Properties: Read or Indicate <br> Handle of value: 0x0003 <br> UUID: `00001524-1212-efde-1523-785feabcd123` (Button Characteristic UUID)                  |
| Button Characteristic Value Declaration        | 0x0003 | `00001524-1212-efde-1523-785feabcd123`        | Read                  | User data: (e.g., `0x01` for pressed, `0x00` for released, image shows example `0x20002689` but text implies boolean)                                         |
| Button Descriptor Declaration (CCCD)           | 0x0004 | `0x2902` (Client Characteristic Configuration) | Read & write          | Indicate: `0x02` (Indications enabled)                                                                                                                       |
| LED Characteristic Declaration                 | 0x0005 | `0x2803` (Characteristic)                    | Read                  | Properties: Write <br> Handle of value: 0x0006 <br> UUID: `00001525-1212-efde-1523-785feabcd123` (LED Characteristic UUID)                                |
| LED Characteristic Value Declaration           | 0x0006 | `00001525-1212-efde-1523-785feabcd123`        | Write                 | User data (e.g., `0x01` for ON, `0x00` for OFF)                                                                                                            |
| MySensor Characteristic Declaration            | 0x0007 | `0x2803` (Characteristic)                    | Read                  | Properties: Notify <br> Handle of value: 0x0008 <br> UUID: `00001526-1212-efde-1523-785feabcd123` (MySensor Characteristic UUID)                        |
| MySensor Characteristic Value Declaration      | 0x0008 | `00001526-1212-efde-1523-785feabcd123`        | None (Value is Notified) | User data (e.g., sensor reading)                                                                                                                             |
| MySensor Descriptor Declaration (CCCD)         | 0x0009 | `0x2902` (Client Characteristic Configuration) | Read & write          | Notify: `0x01` (Notifications enabled)                                                                                                                       |

This attribute table depicts a custom service called “my_lbs”. The my_lbs service holds three characteristics:

1.  Button Characteristic
2.  LED Characteristic
3.  MySensor Characteristic

### Service declaration

The first line in the table declares this service with a service declaration attribute. As previously discussed, the Type field in a declaration attribute holds a unique SIG-defined value, and for service declarations, the UUID is `0x2800`. There is no reason to write to a declaration attribute, therefore its permissions are always Read-only. Lastly, the value field of the service declaration attribute holds the UUID of the service it’s declaring.
_Corresponds to Handle 0x0001 in the table above._

### Button characteristic

The Button characteristic is defined first. The Button characteristic is first declared with the Button Characteristic Declaration. The Type field of a characteristic declaration attribute is always `0x2803` to declare a characteristic and the permissions are always Read-only. The Value field holds the handle of the value attribute, properties, and the UUID of the characteristic it’s declaring.

Then comes the button characteristic value attribute which holds the actual user data, in this case, whether the button is pushed or not.

Lastly, since the button characteristic supports the Indicate operation, there is a CCCD to enable indications for the Button characteristic. The CCCD will always have the UUID `0x2902` and the permission to Read and write. In this specific table, we can see that the Indicate bit-field has been enabled. Notice that the CCCD isn’t referenced in the characteristic declaration like the value declaration is, but can be recognized by the distinct UUID.

Even though it is not referenced in the characteristic declaration, the central knows which characteristic it belongs to based on the handle, because it is listed “under” the characteristic declaration. All items under the characteristic declaration belong to that specific characteristic until there is a new characteristic declaration (UUID `0x2803`) in the list.
_Corresponds to Handles 0x0002 through 0x0004 in the table above._

### LED characteristic

After this comes the LED characteristic, which is very similar to the button characteristic. Notice that this characteristic only supports the Write operation, and therefore does not have a CCCD. It has only 2 attributes, declaration and value.
_Corresponds to Handles 0x0005 and 0x0006 in the table above._

### MySensor characteristic

Lastly, the definition of the MySensor characteristic, which only supports the Notify operation. Notice how the bitfield for the Notify operation has been enabled in the CCCD of the MySensor characteristic.
_Corresponds to Handles 0x0007 through 0x0009 in the table above._

## Exercise 1: Creating a custom service and characteristics

In this exercise, we will learn how to create our own custom service and characteristics. We will practice using the GATT API in nRF Connect SDK, which is again based on Zephyr RTOS, to create and add services and characteristics to our board’s GATT table.

For educational purposes, we will implement our own custom LED Button Service (LBS), which will be called `my_lbs` to separate it from the actual implementation of LBS in nRF Connect SDK.

LBS is a custom service created by Nordic with two characteristics that allow you to control the LEDs and monitor the state of the buttons on your Nordic board.

In this exercise, we will focus on the client-initiated GATT operations Read and Write. In the next exercise, we will add the server-initiated Notify operation.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l4/l4_e1`.

**1. Define the 128-bit UUIDs for the GATT service and its characteristics.**

Add the following lines to the file `my_lbs.h`.
```c
#define BT_UUID_LBS_VAL \
	BT_UUID_128_ENCODE(0x00001523, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

/** @brief Button Characteristic UUID. */
#define BT_UUID_LBS_BUTTON_VAL \
	BT_UUID_128_ENCODE(0x00001524, 0x1212, 0xefde, 0x1523, 0x785feabcd123)

/** @brief LED Characteristic UUID. */
#define BT_UUID_LBS_LED_VAL \
	BT_UUID_128_ENCODE(0x00001525, 0x1212, 0xefde, 0x1523, 0x785feabcd123)


#define BT_UUID_LBS           BT_UUID_DECLARE_128(BT_UUID_LBS_VAL)
#define BT_UUID_LBS_BUTTON    BT_UUID_DECLARE_128(BT_UUID_LBS_BUTTON_VAL)
#define BT_UUID_LBS_LED       BT_UUID_DECLARE_128(BT_UUID_LBS_LED_VAL)
```
Notice how the base UUID used for the service is `0xXXXXXXXX, 0x1212, 0xefde, 0x1523, 0x785feabcd123` and how the first part `0xXXXXXXXX` is incremented by one for each attribute.

As we discussed earlier, the UUID is intended to represent the type of an attribute. The GATT client uses the UUID to know how to treat the value data. We will see later that nRF Connect for Mobile recognizes these UUIDs “types” and therefore presents us with a custom GUI to interact with it.

> **Note**
> Notice that in the header file `my_lbs.h`, there is a definition of the struct `my_lbs_cb` which has the two members `led_cb` and `button_cb`.
>
> The purpose of this structure is to facilitate decoupling of the code responsible for controlling the LEDs and monitoring the buttons (in our case `main.c`) from the Bluetooth LE connectivity code (`my_lbs.c`). These two members are simply function pointers to allow storing two functions in your application code to be triggered anytime the button characteristic is read, or the LED characteristic is written and provide/update the data needed. The function `my_lbs_init()` does the actual assigning of these pointers from `main.c` to `my_lbs.c`.

**2. Create and add the service to the Bluetooth LE stack.**

Now we will statically add the service to the attributes table of our board (the GATT server) using the `BT_GATT_SERVICE_DEFINE()` macros to statically create and add a service.

Add the following code in `my_lbs.c`:
```c
BT_GATT_SERVICE_DEFINE(my_lbs_svc,
    BT_GATT_PRIMARY_SERVICE(BT_UUID_LBS),
    /* STEP 3 - Create and add the Button characteristic */

    /* STEP 4 - Create and add the LED characteristic. */
);
```
The above code creates and adds an empty primary service to the attribute table and assigns it the UUID defined in `BT_UUID_LBS`.

**3. Create and add the custom Button characteristic.**

We will use the `BT_GATT_CHARACTERISTIC()` macro to statically create and add characteristics inside the service.
The `BT_GATT_CHARACTERISTIC()` API takes the following parameters:
1.  Characteristic UUID (`_uuid`)
2.  Characteristic Properties (`_props`)
3.  Attribute Permissions (`_perm`)
4.  Read callback function (`_read`)
5.  Write callback function (`_write`)
6.  User data pointer (`_user_data`)

The first parameter to add is the UUID of the characteristic, `BT_UUID_LBS_BUTTON`, defined in step 1. Then the second and third parameters are the attribute properties and attribute permissions for the characteristic. We are only adding the Read operation for now, so we will set them to `BT_GATT_CHRC_READ` and `BT_GATT_PERM_READ` respectively.

The fourth parameter is the read callback. This is a callback function that is triggered whenever someone tries to read the Button characteristic. We will call this `read_button` and define it in a later step.

The fifth parameter will be set to `NULL` as we are not supporting the Write operation on the Button characteristic.

Lastly, we will pass the user data `button_state`, which is a boolean (`0x01`, `0x00`) representing the button state (Button pressed or Button released). Note that the user data is optional, but we will use it in our LBS implementation.

Add the following code inside the service definition:
```c
    BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON,
                           BT_GATT_CHRC_READ,
                           BT_GATT_PERM_READ, read_button, NULL,
                           &button_state),
```

**4. Create and add the custom LED characteristic.**

This step is similar to step 3. The first parameter to add is the UUID of the characteristic, `BT_UUID_LBS_LED`. The second and third parameters are the attribute properties and attribute permission, in the case of the LED characteristic, we want to support the Write operation, so we will pass `BT_GATT_CHRC_WRITE` and `BT_GATT_PERM_WRITE` respectively.

In this case, the fourth parameter is set to `NULL`, as the LED characteristic will not support the Read operation.

The write callback function, which is triggered whenever someone tries to write to the LED characteristic, is set to `write_led` and will be defined in step 6. No user data is set in the LED characteristic since we will get this value from the GATT client (the central device).

Add the following code inside the service definition:
```c
    BT_GATT_CHARACTERISTIC(BT_UUID_LBS_LED,
                           BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE,
                           NULL, write_led, NULL),
```

**5. Implement the read callback function `read_button()` of the Button characteristic.**

The read callback function is triggered when a request to read the Button characteristic is received. The read callback function must have the `bt_gatt_attr_read_func_t` function signature:
`ssize_t (*bt_gatt_attr_read_func_t)(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)`

We want the read callback function to call the registered application callback function to read the current value of the button (pressed or released), then call the function `bt_gatt_attr_read()` to send the value to the GATT client (the central device).

The `bt_gatt_attr_read()` takes the following parameters: `conn`, `attr`, `buf`, `buf_len`, `offset`, `value`, `value_len`.

For the `conn`, `attr`, `buf`, `buf_len`, and `offset`, we will simply forward the values passed to us from the stack. While for `value` and `value_len` we will rely on the application callback function to update it.

Add the following code in `my_lbs.c`:
```c
static ssize_t read_button(struct bt_conn *conn,
                          const struct bt_gatt_attr *attr,
                          void *buf,
                          uint16_t len,
                          uint16_t offset)
{
	//get a pointer to button_state which is passed in the BT_GATT_CHARACTERISTIC() and stored in attr->user_data
	const char *value = attr->user_data;

	LOG_DBG("Attribute read, handle: %u, conn: %p", attr->handle,
		(void *)conn);

	if (lbs_cb.button_cb) {
		// Call the application callback function to update the get the current value of the button
		button_state = lbs_cb.button_cb(); // Assuming button_state is a global/static bool
		return bt_gatt_attr_read(conn, attr, buf, len, offset, &button_state, // Pass address of button_state
					 sizeof(button_state)); // Use sizeof(button_state)
	}

	return 0;
}
```

**6. Implement the write callback function `write_led()` of the LED characteristic.**

The write callback function is triggered when a request to write to the LED characteristic is received. The write callback function must have the `bt_gatt_attr_write_func_t` function signature:
`ssize_t (*bt_gatt_attr_write_func_t)(struct bt_conn *conn, const struct bt_gatt_attr *attr, const void *buf, uint16_t len, uint16_t offset, uint8_t flags)`

We want the write callback function to read the value received from the central device, stored in `buf`, and then call the registered application callback function to update the state of the LED.

Add the following code in `my_lbs.c`:
```c
static ssize_t write_led(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
                         const void *buf,
                         uint16_t len, uint16_t offset, uint8_t flags)
{
	LOG_DBG("Attribute write, handle: %u, conn: %p", attr->handle,
		(void *)conn);

	if (len != 1U) {
		LOG_DBG("Write led: Incorrect data length");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
	}

	if (offset != 0) {
		LOG_DBG("Write led: Incorrect data offset");
		return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
	}

	if (lbs_cb.led_cb) {
		//Read the received value
		uint8_t val = *((uint8_t *)buf);

		if (val == 0x00 || val == 0x01) {
			//Call the application callback function to update the LED state
			lbs_cb.led_cb(val ? true : false);
		} else {
			LOG_DBG("Write led: Incorrect value");
			return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
		}
	}

	return len;
}
```

**7. Include the header file of the customer service `my_lbs.h`**

Now that we are done defining our custom LBS service, we can add it to the main application code.
Add the following line in `main.c`:
```c
#include "my_lbs.h"
```

**8. Controlling the LED.**

8.1 Specify controlling LED3 on the board.
Add the following line in `main.c`:
```c
#define USER_LED                DK_LED3
```

8.2 Define the application callback function for controlling the LED.
We will simply rely on the Buttons and LED library and call `dk_set_led()` to set the state of the LED. This function will be called by the write callback function of the LED characteristic, `write_led()`, and it will pass either `True` or `False`.
Add the following code in `main.c`:
```c
static void app_led_cb(bool led_state)
{
	dk_set_led(USER_LED, led_state);
}
```

**9. Monitoring the button.**

9.1 Specify monitoring Button 1 on the board.
Add the following line in `main.c`:
```c
#define USER_BUTTON             DK_BTN1_MSK
```

9.2 Define the application callback function for reading the state of the button.
This function will simply return the global variable `app_button_state` to the caller (the read callback function of the Button characteristic). Since `app_button_state` is updated in the `button_changed()` function already defined in `main.c` that is called whenever a button is pressed, this variable will represent the state of the button.
Add the following lines in `main.c`:
```c
// Ensure app_button_state is declared, e.g., static bool app_button_state = false;
static bool app_button_cb(void)
{
	return app_button_state;
}
```

**10. Declare a variable `app_callbacks` of type `my_lbs_cb`**

Declare the variable of type `my_lbs_cb` and initiate its members to the application callback functions `app_led_cb` and `app_button_cb`.
Add the following code in `main.c`:
```c
static struct my_lbs_cb app_callbacks = {
	.led_cb    = app_led_cb,
	.button_cb = app_button_cb,
};
```

**11. Pass the application callback functions stored in `app_callbacks` to our custom LBS service.**

This is done by passing `app_callbacks` to `my_lbs_init()`, a function already defined in `my_lbs.c` to register application callbacks for both the LED and the Button characteristics.
Add the following code in `main.c` (ensure `err` is declared):
```c
// int err; // if not already declared in main()
err = my_lbs_init(&app_callbacks);
if (err) {
	printk("Failed to init LBS (err:%d)\n", err);
	return -1; // or handle error appropriately
}
```

**12. Build and flash the application on your board.**

LED1 on your board should be blinking, indicating that your board is advertising.

**13. Connect to your board using your smartphone.**

Open nRF Connect for Mobile on your smartphone, and connect to your device named “MY_LBS1“, in the Scanner tab.
_Description: nRF Connect for Mobile screenshot showing the "Nordic LED Button Service" with "Button" (Read) and "LED" (Write) characteristics._

Here we can see our LBS service, and nRF Connect for Mobile recognizes the UUID for the service and its characteristics and labels the service as “Nordic LED Button Service” and its characteristics as “Button” and “LED”. Also, notice that the Button characteristic currently supports only the GATT read operation, while the LED characteristic supports the GATT write operation.

**14. Control the LED**

In nRF Connect for Mobile, press on the arrow next to the LED characteristic to write to it.
_Description: nRF Connect for Mobile screenshot highlighting the write arrow for the LED characteristic._

A pop-up window will appear, allowing you to either turn on or turn off the LED on the board.
_Description: nRF Connect for Mobile pop-up for writing to LED characteristic, with ON/OFF options._

Select ON and then SEND to turn LED3 on. Then select the arrow again, and select OFF to turn off LED3.

**15. Read the button status.**

Press and hold button 1 on your board while simultaneously pressing the arrow next to the button characteristic to read it. You should see that the value is now updated to the Button pressed.

In the next exercise, we will add the Notify operation to the Button characteristic so the button status will be updated without the need for us to manually poll read.
> **Note**
> Although the values sent are simply `0x00` and `0x01`, nRF Connect for Mobile presents these values as Button released and Button pressed for better visualization since it recognizes the UUID for the Button characteristic.

## Exercise 2: Adding notification and indication support

In this exercise, we will add support for the GATT operations Notify and Indicate. Recall that these operations are server-initiated, but the GATT client must subscribe to the desired data to receive the messages.

The exercise is divided into two parts.

First, we will add support for the Indicate operation to the Button characteristic and then subscribe to the Button characteristic from nRF Connect for Mobile to be notified whenever the button is pushed.

In the second part, we will add another custom characteristic to our service that will only support notifications. We will call this characteristic MYSENSOR, and use it to periodically send simulated sensor data to your phone. This is a very relevant use-case, for example, if your board has a sensor that collects some data that you want to periodically transmit to a central device.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l4/l4_e2`.

**1. Modify the Button characteristic declaration to support indication.**

Modify the declaration of the Button characteristic to pass the `BT_GATT_CHRC_INDICATE` attribute property, as well as the read property. Notice how we use the bitwise or operator `|` to support both read and indication.
Change the following code in `my_lbs.c`:
```c
    BT_GATT_CHARACTERISTIC(BT_UUID_LBS_BUTTON,
                           BT_GATT_CHRC_READ | BT_GATT_CHRC_INDICATE,
                           BT_GATT_PERM_READ, read_button, NULL,
                           &button_state),
```

**2. Create and add the Client Characteristic Configuration Descriptor.**

Since we are now using indication, we need to have a Client Characteristic Configuration Descriptor to enable devices acting as GATT clients (for example, nRF Connect for Mobile) to subscribe to this characteristic.

We will use the macro `BT_GATT_CCC()` to create and add the Client Characteristic Configuration Descriptor. Its signature is `BT_GATT_CCC(cfg_changed, perm)`.
The first parameter is the configuration change callback function that we will call `mylbsbc_ccc_cfg_changed` and implement in the next step, and the second parameter is the access permission for the descriptor which we will grant both read and write permissions.

Add the following code (right below the Button Characteristic definition in `my_lbs.c`):
```c
    BT_GATT_CCC(mylbsbc_ccc_cfg_changed,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
```
This code is added right below the Buttons Characteristic definition. The location of where to place this definition is important, as discussed in the Attribute table topic.

**3. Implement the configuration change callback function.**

This function is called when a GATT client enables or disables indication. The callback function must have the signature as shown in the code below.
We want this callback function to update the boolean `indicate_enabled` to true if the client (your phone) enables indications or false if the client disables indications.
Add the following code in `my_lbs.c` (ensure `indicate_enabled` is declared, e.g., `static bool indicate_enabled = false;`):
```c
static void mylbsbc_ccc_cfg_changed(const struct bt_gatt_attr *attr,
                                  uint16_t value)
{
	indicate_enabled = (value == BT_GATT_CCC_INDICATE);
}
```

**4. Define an indication parameter**

In order to use the Indicate operation, we need to define a variable of type `struct bt_gatt_indicate_params` to hold the variable that you want to send out and the characteristic that you want to associate the indication with.
Add the following line in `my_lbs.c`:
```c
static struct bt_gatt_indicate_params ind_params;
```
We will populate the fields of this struct variable in the next step.

**5. Define the function to send indications.**

This is the function that your application code can call to send data to subscribed clients whenever it needs to.
In this function, we will use the GATT API function `bt_gatt_indicate()` to do the work of sending indications.
The `bt_gatt_indicate()` function takes a connection pointer (`conn`, `NULL` for all subscribed) and a pointer to `bt_gatt_indicate_params`.

Before calling `bt_gatt_indicate()`, we need to check if indication is enabled (`indicate_enabled` is true or false). If `indicate_enabled` is false, we will simply return and not send indications.

5.1 Add the following function definition in `my_lbs.c`:
```c
// Ensure button_state is accessible if this function needs it directly,
// or it's passed as a parameter.
// Assuming indicate_cb is defined:
// static void indicate_cb(struct bt_conn *conn, struct bt_gatt_indicate_params *params, uint8_t err) { /* Log indication status */ }

int my_lbs_send_button_state_indicate(bool current_button_state) // Changed to accept current state
{
	if (!indicate_enabled) {
		return -EACCES;
	}

	/* STEP 5.2 - Populate the indication */
    // Placeholder for next step's code
    return 0; // Placeholder
}
```

5.2 If indications are enabled, populate `ind_params`.
The structure `ind_params` is of type `bt_gatt_indicate_params` which has several members, some of which are mandatory to fill and some that are optional.
The first two members are `uuid` (optional, can be `NULL` if `attr` is provided) and `attr` (pointer to the characteristic value attribute).
_Description: The `bt_gatt_indicate_params` struct has members like `uuid`, `attr`, `func`, `destroy`, `data`, `len`._

We need to provide either the UUID or a pointer to the attribute of the characteristic that we want to have Indicate support. In this case, we will be using the latter.
Remember as we discussed before, that a characteristic will have at least two attributes, the characteristic declaration attribute and the characteristic value attribute. So we need to provide a pointer to either the Button characteristic declaration or the Button characteristic value.

_Description: An example attribute table for "My LBS" service is shown, with indices for attributes: `my_lbs_svc.attrs[0]`, `my_lbs_svc.attrs[1]` (Button Characteristic Declaration), `my_lbs_svc.attrs[2]` (Button Characteristic Value)._
> **Note**
> To see your own attributes table, you need to watch the variables `my_lbs_svc.attrs[i]` for every attribute in the table while debugging the application.
> In the Watch window in the Debug panel, select the plus icon to Add Expression and type in the fields you want to watch, `my_lbs_svc`, `my_lbs_svc.attrs[0]`, `my_lbs_svc.attrs[1]`, etc…

So we can pass either the Button characteristic declaration in index 1 (`&my_lbs_svc.attrs[1]`) or the Button characteristic value in index 2 (`&my_lbs_svc.attrs[2]`). We should pass the characteristic *value* attribute.

The optional member `func` allows you to register a function when the remote device acknowledges an indication. We will set it to `indicate_cb` which prints it to the debug log.
The optional member `destroy` allows you to register a function when indication ends, which we will set to `NULL`.
For the last two parameters, the Indicate Value data and its length, we need to specify the data we want to send out and its size. Here, we will simply pass the `button_state` and its size.

Add the following lines to the function definition (inside the `if (indicate_enabled)` block):
```c
    // This state needs to be the *actual current* button state.
    // It's better to update a global/static button_state and use that,
    // or pass the current state to this function.
    // For this example, assuming 'current_button_state' is the up-to-date value.
    // static bool global_button_state; // if used globally
    // global_button_state = current_button_state;

	ind_params.attr = &my_lbs_svc.attrs[2]; // Index of Button Characteristic Value
	ind_params.func = indicate_cb;         // Make sure indicate_cb is defined
	ind_params.destroy = NULL;
	ind_params.data = &current_button_state; // Use the passed current state
	ind_params.len = sizeof(current_button_state);
	return bt_gatt_indicate(NULL, &ind_params); // NULL for conn sends to all subscribed & connected
```

**6. Send indication on a button press.**

Now we want the GATT server to send the indication upon a button press. We will trigger indication in the `button_changed()` function so that every time button 1 is pressed, we will send its status as indication.
Add the following line in `main.c` inside `button_changed()` (ensure `user_button_state` reflects the current state):
```c
    // Inside button_changed() in main.c, after determining user_button_state
    // bool user_button_state = (button_state & USER_BUTTON) ? true : false;
    my_lbs_send_button_state_indicate(user_button_state);
```
Note that we are still supporting the Read operation that we did in the previous exercise, so the Button characteristic now supports both read and indication.

**7. Build and flash the application on your board.**

LED1 on your board should be blinking, indicating that your board is advertising.

**8. Connect to your board using your smartphone.**

Open nRF Connect for Mobile on your smartphone, and connect to your device, now named “MY_LBS2“, in the Scanner tab.
Notice how the Button characteristic now includes support for the GATT indicate operation. Also, notice the symbol consisting of two arrows. This allows your smartphone, acting as a GATT client, to subscribe to Button characteristic indication.
_Description: nRF Connect for Mobile screenshot showing "MY_LBS2" with the Button characteristic now showing indicate capability (double arrows icon)._

**9. Subscribe to the Button characteristic indication.**

Press on the two arrows symbol to subscribe to the Button characteristic indication.

**10. Press Button 1 on your board, and notice how indication is sent by the board as soon as the button is pressed or released.**

With this, we have covered how to create a custom Bluetooth LE service from scratch. It’s worth noting that the nRF Connect SDK comes with the LBS service, among many others (from nRF Connect SDK and Zephyr RTOS ) that you can use out-of-the-box, and you don’t have to implement it from scratch as we did in this exercise and the past exercise. We only did this for learning purposes.

You can easily use the built-in LBS service in nRF Connect SDK by enabling the Kconfig symbol `CONFIG_BT_LBS` in `prj.conf` and registering your application callback functions to read the button status and update the LEDs.

---
In this part of the exercise, we will add another custom characteristic that supports only notifications. We will call this characteristic MYSENSOR, and use it to periodically stream data over Bluetooth LE.

**11. Add the UUID for the MYSENSOR characteristic**

11.1 Assign a UUID to the new characteristic. We will use the base UUID for the LBS service `0x00001526, 0x1212, 0xefde, 0x1523, 0x785feabcd123` where the first portion is simply incremented from the last characteristic.
> **Note**
> You can also generate your own UUID using this website or by using Python or similar scripting languages.

Add the following code in `my_lbs.h`:
```c
/** @brief MySensor Characteristic UUID. */ // Corrected comment
#define BT_UUID_LBS_MYSENSOR_VAL \
	BT_UUID_128_ENCODE(0x00001526, 0x1212, 0xefde, 0x1523, 0x785feabcd123)
```

11.2 Convert the array to a generic UUID by using the macro `BT_UUID_DECLARE_128()`.
Add the following line in `my_lbs.h`:
```c
#define BT_UUID_LBS_MYSENSOR       BT_UUID_DECLARE_128(BT_UUID_LBS_MYSENSOR_VAL)
```

**12. Create and add the MYSENSOR characteristic and its Client Characteristic Configuration Descriptor.**

We are only supporting the GATT Notify operation.
Add the following code in `my_lbs.c` in the service declaration (after the LED characteristic or Button characteristic with its CCCD).
```c
    BT_GATT_CHARACTERISTIC(BT_UUID_LBS_MYSENSOR,
                           BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_NONE, NULL, NULL, // Read/Write perms are NONE for notify-only value
                           NULL), // No user_data pointer needed if value is sent dynamically

    BT_GATT_CCC(mylbsbc_ccc_mysensor_cfg_changed,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
```

**13. Define the configuration change callback function to detect enabling/disabling notifications for the MYSENSOR characteristic.**

In this function, we will update the boolean `notify_mysensor_enabled` to true if the client (the remote device) enables notification or false if the client disables notification on the MYSENSOR characteristic.
Add the following code in `my_lbs.c` (ensure `notify_mysensor_enabled` is declared, e.g., `static bool notify_mysensor_enabled = false;`):
```c
static void mylbsbc_ccc_mysensor_cfg_changed(const struct bt_gatt_attr *attr,
                                          uint16_t value)
{
	notify_mysensor_enabled = (value == BT_GATT_CCC_NOTIFY);
}
```

**14. Define the function `my_lbs_send_sensor_notify()` to send notifications for the MYSENSOR characteristic.**

This is the function that your application code can call to send data to subscribed clients whenever it needs to.
In this function, we will use the GATT API function `bt_gatt_notify()` to do the work of sending notifications.
The `bt_gatt_notify()` function takes `conn` (or `NULL`), `attr` (pointer to characteristic value attribute), `data`, and `len`.

As we discussed earlier, we need to provide a pointer to either the MYSENSOR characteristic declaration or the MYSENSOR characteristic value. For `bt_gatt_notify`, we point to the characteristic *value* attribute. From the table image, if MYSENSOR characteristic declaration is at index `N`, its value is at `N+1`. Let's assume MYSENSOR characteristic declaration is at `attrs[6]`, then its value is at `attrs[7]`.

And we need to specify the data we want to send out and its size. This time we are sending 4 bytes (or 32 bits, the size of the `uint32_t` type) to represent the sensor reading. You can send any data type of your preference over Bluetooth LE.

Before calling `bt_gatt_notify()`, we need to check if notification is enabled on the MYSENSOR characteristics, meaning a client has subscribed to it. This is done by simply checking the value of the boolean variable `notify_mysensor_enabled`. If it is not enabled, we will return and do nothing.
Add the following code in `my_lbs.c`:
```c
int my_lbs_send_sensor_notify(uint32_t sensor_value)
{
	if (!notify_mysensor_enabled) {
		return -EACCES;
	}
    // Assuming MYSENSOR characteristic value attribute is at index 7
    // (Service Decl[0], Btn Decl[1], Btn Val[2], Btn CCCD[3], LED Decl[4], LED Val[5], MySensor Decl[6], MySensor Val[7], MySensor CCCD[8])
    // The value attribute for MYSENSOR would be my_lbs_svc.attrs[7] based on the table structure provided.
	return bt_gatt_notify(NULL, &my_lbs_svc.attrs[7], 
			              &sensor_value,
			              sizeof(sensor_value));
}
```

**15. Define the data you want to stream over Bluetooth LE**

We will declare a `uint32_t` (4 bytes) to hold the simulated sensor readings and assign it an initial value of 100.
Add the following line in `main.c`:
```c
static uint32_t app_sensor_value = 100;
```

**16. Define a function `simulate_data()` to simulate the data.**

Just for the sake of demonstration and to visually see the change of data on the remote device, we will include a function that increments the `app_sensor_value` sent over Bluetooth LE by 1 each time. `app_sensor_value` will start from 100 and get incremented by one on every notification push; it will roll back to 100 once the value reaches 200.
In a real application, we would normally call a sensor API to get actual data.
Add the following code to `main.c`:
```c
static void simulate_data(void)
{
	app_sensor_value++;
	if (app_sensor_value == 200) {
		app_sensor_value = 100;
	}
}
```

**17. Define the interval at which you want to send data at (streaming interval).**

We will set this to 500 ms. Typically, you must set this value to meet your application’s needs.
Add the following code in `main.c`:
```c
#define NOTIFY_INTERVAL         500
```

**18. Create a thread to periodically send data**

The nRF Connect SDK, which is based on Zephyr RTOS, has many options to schedule tasks periodically. One option is to create a separate thread, as covered in the nRF Connect SDK Fundamentals course – Lesson 7.
We will create a thread dedicated to sending notifications periodically.

18.1 Define the thread function
In this function, we will call `simulate_data()` to increment the simulated data sensor by one, send it to the remote device as Bluetooth LE notification, and sleep for `NOTIFY_INTERVAL`.
Add the following code in `main.c`:
```c
void send_data_thread(void)
{
	while(1){
		/* Simulate data */
		simulate_data();
		/* Send notification, the function sends notifications only if a client is subscribed */
		my_lbs_send_sensor_notify(app_sensor_value);

		k_sleep(K_MSEC(NOTIFY_INTERVAL));
	}
}
```

18.2. Define and initialize the thread to periodically send the data.
Add the following line in `main.c` (ensure `STACKSIZE` and `PRIORITY` are defined, e.g., `#define STACKSIZE 1024`, `#define PRIORITY 7`):
```c
K_THREAD_DEFINE(send_data_thread_id, STACKSIZE, send_data_thread, NULL, NULL,
                NULL, PRIORITY, 0, 0);
```

**19. Build and flash the application on your board, and connect to it using your smartphone.**

**20. Subscribe to the MYSENSOR characteristic**

The MYSENSOR characteristic is shown as Unknown Characteristic as the nRF Connect for Mobile has no registered name for the new UUID chosen for the MYSENSOR characteristic.
> **Note**
> (In Android) You can add a name to it in nRF Connect for Mobile by pressing the MYSENSOR characteristic and clicking on the pen symbol at the top to assign it a name.

Subscribe to the characteristic and notice how data is received periodically from your board approximately twice a second (NOTIFY_INTERVAL was set to 500ms).

## Exercise 3: Sending data between a UART and a Bluetooth LE connection

_(Note: This exercise is compatible with nRF Connect SDK v3.0.0-v3.2.0.)_

This exercise will focus on Nordic UART Service (NUS). NUS is quite a popular and versatile custom GATT service. The service emulates a serial port over Bluetooth LE, which allows you to send any sort of data back and forth between a UART connection (or a virtual serial port emulated over USB) and a Bluetooth LE connection.

_Diagram illustrating NUS: UART (PC) <-> Board (NUS Peripheral) <-> Bluetooth LE (Central/Phone)_

The service has two characteristics:
*   **RX Characteristic (Write, Write w/o response):** For sending data *to* the board
*   **TX Characteristic (Notify):** For receiving data *from* the board

When a Bluetooth LE connection is established between a peripheral and a central, NUS forwards any data received on the RX pin of the UART0 peripheral to the Bluetooth LE central as notifications through the TX Characteristic. Any data sent from the Bluetooth LE central through the RX Characteristic is sent out of the UART0 peripheral’s TX pin.

Remember that on Nordic DKs, the UART0 peripheral is typically gated through the SEGGER debugger/programmer chip (aka: interface MCU) to a USB CDC virtual serial port that you can connect directly to your PC.
> **Note**
> The code provided in this exercise uses NUS to forward/receive data to/from the UART0 peripheral. You could easily modify the exercise to use NUS with UART1 or other peripherals.

In this exercise, we will learn how to use NUS to exchange data over Bluetooth LE between your PC and your smartphone or tablet running nRF Connect for Mobile.

### Exercise steps

In the GitHub repository for this course, go to the base code for this exercise, found in `l4/l4_e3`.

**1. Include NUS in your application.**

Add the following line to your `prj.conf` file:
```kconfig
CONFIG_BT_NUS=y
```
Enabling this Kconfig will make the build system include the `nus.c` and the `nus.h` files. Since `nus.c` already includes the static service declaration and its characteristics, including the source files will by itself add NUS to the attribute table of your application.

In the application code (`main.c`), we will mainly need to do the following tasks:
*   Initialize the UART peripheral.
*   Define and register an application callback function to forward the data received from a Bluetooth LE connection to UART.
*   Call a function to send data received from UART to a Bluetooth LE connection.

We will first spend some time examining the NUS service implementation.

**2. Examine the NUS service declaration**

This is declared in `nus.c`, (found in `<install_path>\nrf\subsys\bluetooth\services\nus.c`). We will not modify the source files of the NUS service.
The declaration statically creates and adds the service with the UUID `BT_UUID_NUS_SERVICE` (defined in `nus.h`). and its two characteristics, the RX Characteristic and TX Characteristic.

_The NUS service declaration includes a primary service, an RX characteristic (for writing data to the device), and a TX characteristic (for notifying data from the device)._

Notice the presence of the conditional compilation flag `CONFIG_BT_NUS_AUTHEN`. If enabled, it will do the following to the static configurations of the characteristics of the NUS service:
*   Change the characteristic access permission of the TX Characteristic from `BT_GATT_PERM_READ` to `BT_GATT_PERM_READ_AUTHEN` (requires authentication & encryption).
*   Change the characteristic access permission of the Client Characteristic Configuration Descriptor of the TX Characteristic from `BT_GATT_PERM_READ | BT_GATT_PERM_WRITE` to `BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN` (requires authentication & encryption).
*   Change the characteristic access permission of the RX Characteristic from `BT_GATT_PERM_READ | BT_GATT_PERM_WRITE` to `BT_GATT_PERM_READ_AUTHEN | BT_GATT_PERM_WRITE_AUTHEN` (requires authentication & encryption).

Authenticated connections and encryption will be the focus of the next lesson; therefore, we will disable the `CONFIG_BT_NUS_AUTHEN` flag in this lesson.
Also, notice that for the write callback of the RX Characteristic, we are registering the function `on_receive()`. This function is called every time the Bluetooth central device writes data to the RX Characteristic. We will dissect it in the next step.

**3. Examine the write callback function of the RX Characteristic**

Let’s examine the write callback function of the RX Characteristic `on_receive()` in `nus.c`. The function calls the application callback function and passes it three parameters. The connection handle (in case multiconnection is used), the data received over Bluetooth LE, and its size.
_The `on_receive` function in `nus.c` essentially acts as a bridge, calling a user-registered callback (`cb->received`) with the connection, data, and length when data is written to the NUS RX characteristic._

**4. Examine the `bt_nus_init()` which is also defined in `nus.c`**

This function and the structure `bt_nus_cb` (defined in `nus.h`) have a similar intent to the `my_lbs_init()` we saw in Exercise 1. The purpose of `bt_nus_init()` and the `bt_nus_cb` struct is to facilitate decoupling of the code responsible for actually reading/writing to the UART peripheral (application code, i.e `main.c`) from the Bluetooth LE connectivity code (`nus.c`). Code decoupling adds a bit of complexity, but it does make the code way easier to maintain and scale.
_The `bt_nus_init` function takes a pointer to a `bt_nus_cb` struct, which contains function pointers for application-defined callbacks related to NUS operations._

Later, we will call this function in `main.c` and pass it a pointer to our application callback functions. Notice that the `bt_nus_init()` function can register three application callback functions:
*   **Data received callback (Mandatory).** The data has been received as a write request on the RX Characteristic, so the application callback function must do the action needed to forward the data to the UART peripheral.
*   **Data sent callback (Optional).** This function pointer can allow you to register an application callback function to be triggered when data has been sent as a notification to the TX Characteristic. We will not use it in this exercise.
*   **Send state callback (Optional).** This function pointer can allow you to register an application callback function to be triggered when a remote Bluetooth LE device subscribes or unsubscribes to the TX Characteristic notifications. We will not use it in this exercise.

**5. Examine the function responsible for sending notifications**

The last function we will examine in `nus.c`, is the function responsible for sending notifications over a Bluetooth LE connection `bt_nus_send()`. We will call `bt_nus_send()` from the application code (`main.c`) to forward the data received from the UART to a remote device over a Bluetooth LE connection.
Unlike in LBS, where we supported a single Bluetooth LE connection, NUS can support simultaneous connections. Therefore, the implementation of sending notifications is slightly different.
We will check the connection parameter `conn` (of type `struct bt_conn`) if it equals `NULL`, we will send notifications to all connected devices. On the other hand, if a specific connection is passed to the `bt_nus_send()` function, we will manually check if notification is enabled by the client on that connection and then send notification to that particular client.
_The `bt_nus_send` function handles sending data via NUS TX characteristic notifications. If `conn` is `NULL`, it iterates through all connections; otherwise, it sends to the specified connection, checking subscription status._

With this, we have a good understanding of the implementation of Nordic UART Service. In the next steps, we will cover how to use it in an application code.

**6. Declare two FIFO data structures**

Declare two FIFOs data structures and the FIFO data item to hold the following:
*   The data received from the UART peripheral, and we want to send over a Bluetooth LE connection (`fifo_uart_rx_data`).
*   The data was received from a Bluetooth LE connection, and we want to send over UART (`fifo_uart_tx_data`).

6.1 Declare the FIFOs
Add the following code in `main.c`:
```c
static K_FIFO_DEFINE(fifo_uart_tx_data);
static K_FIFO_DEFINE(fifo_uart_rx_data);
```

6.2 Declare the struct of the data item of the FIFOs
Add the following code in `main.c` (ensure `UART_BUF_SIZE` is defined, e.g., from `CONFIG_BT_NUS_UART_BUFFER_SIZE` or a local define):
```c
// Assuming UART_BUF_SIZE is defined, e.g., #define UART_BUF_SIZE CONFIG_BT_NUS_UART_BUFFER_SIZE
struct uart_data_t {
	void *fifo_reserved; // Must be first element
	uint8_t data[UART_BUF_SIZE];
	uint16_t len;
};
```
Notice that the 1st word is reserved for use by FIFO as required by the FIFO implementation. The second member of the structure, which will hold the actual data, is an array of bytes of size `CONFIG_BT_NUS_UART_BUFFER_SIZE`. It is user configurable. The default size is 40 bytes.

**7. Initialize the UART peripheral**

Setting up the UART peripheral driver, using its asynchronous API, and assigning an application callback function is covered thoroughly in the nRF Connect SDK Fundamentals course – Lesson 5. Feel free to revisit the Lesson in the nRF Connect SDK Fundamentals course to refresh the information if needed.
Add the call to `uart_init()` in `main()` as shown below (ensure `uart_init()` is defined and `err`, `error()` are available):
```c
// int err; // if not already declared in main()
// void error(void) { /* Handle error, e.g., loop forever */ }
err = uart_init();
if (err) {
	error();
}
```

**8. Forward the data received from a Bluetooth LE connection to the UART peripheral.**

8.1 Create a variable of type `bt_nus_cb` and initialize it.
This variable will hold the application callback functions for the NUS service.
Add the following code in `main.c` (ensure `bt_receive_cb` is declared/defined):
```c
// Forward declaration or definition of bt_receive_cb
// static void bt_receive_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len);
static struct bt_nus_cb nus_cb = {
	.received = bt_receive_cb,
};
```
We will set the data received callback to `bt_receive_cb`, which is covered in a following step.

8.2 Pass your application callback functions stored in `nus_cb` to the NUS service by calling `bt_nus_init()`
Add the following code in `main()`:
```c
// int err; // if not already declared
err = bt_nus_init(&nus_cb);
if (err) {
	LOG_ERR("Failed to initialize UART service (err: %d)", err);
	return 0; // or handle error appropriately
}
```

8.3 The `bt_receive_cb()` function will be called by NUS when data has been received as a write request on the RX Characteristic. The data received from a Bluetooth LE connection will be available through the pointer `data` with the length `len`. We will call the UART peripheral function `uart_tx` to forward the data received over Bluetooth LE to the UART peripheral.
Add the following code inside `bt_receive_cb()` function (this part implies `bt_receive_cb` allocates `tx` and puts it in a FIFO if `uart_tx` fails immediately, which is a common pattern for handling backpressure):
```c
// Inside bt_receive_cb(struct bt_conn *conn, const uint8_t *const data, uint16_t len)
// This example assumes 'tx' is a struct uart_data_t * allocated and populated with 'data' and 'len'.
// A more direct approach might be:
// err = uart_tx(uart_dev, data, len, SYS_FOREVER_MS);
// if (err) { /* handle error, maybe put into a TX FIFO for later retry */ }

// The provided snippet implies a FIFO is used if immediate tx fails:
// struct uart_data_t *tx = k_malloc(sizeof(*tx) + len); /* Simplified allocation example */
// if (tx) {
//    memcpy(tx->data, data, len);
//    tx->len = len;
//    err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS); // uart is the device struct pointer
//    if (err) {
//        k_fifo_put(&fifo_uart_tx_data, tx);
//    } else {
//        k_free(tx);
//    }
// }
// The original text is:
// err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS);
// if (err) {
//     k_fifo_put(&fifo_uart_tx_data, tx);
// }
// This assumes 'tx' is already a `struct uart_data_t *` properly populated.
// Let's assume `tx` is correctly prepared before this snippet.
		err = uart_tx(uart, tx->data, tx->len, SYS_FOREVER_MS); // `uart` should be your UART device instance
		if (err) {
			k_fifo_put(&fifo_uart_tx_data, tx);
		}
```
In case there is an ongoing transfer already or an error, we will put the data in the `fifo_uart_tx_data` and try to send it again inside the `uart_cb` UART callback.

**9. Receiving data from the UART peripheral and sending it to a Bluetooth LE connection.**

9.1 Push the data received from the UART peripheral into the `fifo_uart_rx_data` FIFO.
On the `UART_RX_BUF_RELEASED` event in the `uart_cb` callback function of the UART peripheral, we will put the data received from UART into the FIFO by calling `k_fifo_put()`. The `UART_RX_BUF_RELEASED` event is triggered when the buffer is no longer used by UART driver.
Add the following line inside the UART callback function in the `UART_RX_BUF_RELEASED` event:
```c
// case UART_RX_BUF_RELEASED:
//    struct uart_data_t *buf = CONTAINER_OF(evt->data.rx_buf.buf, struct uart_data_t, data);
//    // This assumes `buf` is correctly retrieved. The original text just shows:
    k_fifo_put(&fifo_uart_rx_data, buf); // `buf` should be `struct uart_data_t *`
```

9.2 Create a dedicated thread for sending the data over Bluetooth LE.
We will create a thread and associate it with the function `ble_write_thread()` which we will develop in the next step. The thread is assigned the stack `STACKSIZE` (1024 by default), and the priority `PRIORITY` (7 by default).
Add the following line of code in `main.c`:
```c
// Ensure STACKSIZE and PRIORITY are defined.
// Forward declare ble_write_thread if defined later.
K_THREAD_DEFINE(ble_write_thread_id, STACKSIZE, ble_write_thread, NULL, NULL,
                NULL, PRIORITY, 0, 0);
```

9.3 Define the thread function
```c
// Ensure ble_init_ok semaphore is defined and initialized.
// k_sem ble_init_ok;
// k_sem_init(&ble_init_ok, 0, 1); /* In main before starting BLE */
// k_sem_give(&ble_init_ok);      /* After bt_enable completes */

void ble_write_thread(void)
{
	/* Don't go any further until BLE is initialized */
	k_sem_take(&ble_init_ok, K_FOREVER);
	struct uart_data_t nus_data = { // Temporary buffer to accumulate data for sending
		.len = 0,
	};

	for (;;) {
		/* Wait indefinitely for data to be sent over bluetooth */
		struct uart_data_t *buf = k_fifo_get(&fifo_uart_rx_data,
						     K_FOREVER);

		// This logic attempts to send data in chunks up to nus_data.data size
		// or until a newline/carriage return is found.
		int plen = MIN(sizeof(nus_data.data) - nus_data.len, buf->len);
		int loc = 0;

		while (plen > 0) {
			memcpy(&nus_data.data[nus_data.len], &buf->data[loc], plen);
			nus_data.len += plen;
			loc += plen;

			if (nus_data.len >= sizeof(nus_data.data) ||
			   (nus_data.data[nus_data.len - 1] == '\n') ||
			   (nus_data.data[nus_data.len - 1] == '\r')) {
				if (bt_nus_send(NULL, nus_data.data, nus_data.len)) {
					LOG_WRN("Failed to send data over BLE connection");
				}
				nus_data.len = 0; // Reset buffer
			}
			// Prepare for next chunk from the same buf if any left
			plen = MIN(sizeof(nus_data.data) - nus_data.len, buf->len - loc);
		}
		k_free(buf); // Assuming buf was allocated with k_malloc for the FIFO
	}
}
```
In this thread, we have an infinite loop where we call `k_fifo_get()` to get the data from the FIFO. We will send the data to connected Bluetooth LE device(s) as notification by calling the NUS function `bt_nus_send()`.
Notice that we passed `K_FOREVER` as the second parameter for `k_fifo_get()`. This means the thread will be scheduled out if there is no data in the FIFO. Once the UART peripheral callback function `uart_cb` puts data in the FIFO, the thread will be scheduled back for execution.
Also, notice that the thread will only start after the Bluetooth LE stack has been initialized through the use of the semaphore: `k_sem_take(&ble_init_ok, K_FOREVER)`.

**10. Build and flash the application on your board.**

You should notice that LED1 on your board is blinking now, indicating that your board is advertising.

### Testing

**11. Open a terminal to view the log output from the application**

Just like we have done in previous exercises, connect to the COM port of your DK in VS Code by expanding your device under Connected Devices and selecting the COM port for the device. Note on some development kits; there might be more than one COM port. Use the one that you see the `Starting Nordic UART service example` log on.
Your log should look like below:
```
*** Booting nRF Connect SDK ***
Starting Nordic UART service example
```
If you don’t see that log on the terminal, since it only prints on bootup once, you can press the reset button on the board to see it.

**12. Connect to your device via your smartphone.**

Open nRF Connect for Mobile on your smartphone. In the Scanner tab, locate the device, now named “Nordic_UART_Service” and connect to it, as done in the previous exercises.

**13. Send data from your phone to the board.**

In nRF Connect for Mobile, press on the arrow next to the RX Characteristic. You will be prompted with a small window.
Type a message in the Write value box, select the type of operation: Request (write with response) or Command (write with no response), and press SEND.
_Description: nRF Connect for Mobile screenshot showing the dialog to write a value to the NUS RX characteristic._

The message will be forwarded to the UART peripheral and should appear in your PC's terminal connected to the DK:
```
*** Booting nRF Connect SDK ***
Starting Nordic UART service example
Hello from phone!
```

**14. Send data from your PC to your phone through the board.**

14.1 In nRF Connect for mobile, subscribe to the TX Characteristic by pressing on the icon next to the TX Characteristic (usually three downward arrows).

14.2 In your PC's terminal application (e.g., VS Code terminal connected to the DK's COM port), type a message to send to the remote device (for example `Hello from PC!`) and hit enter (to send an end-of-line and carriage return).
> **Important**
> Please note that the data you type will not be visible in the terminal since most terminals are in char mode by default (local echo might be off). Once you hit enter on your keyboard, you will be able to see the data on the Smartphone/Tablet side using nRF Connect for Mobile.

The message will be forwarded from the UART peripheral through the Bluetooth LE connection to the central device running nRF Connect for Mobile and show up there.
_Description: nRF Connect for Mobile screenshot showing "Hello from PC!" received on the NUS TX characteristic._

> **Note**
> The default Maximum Transmission Unit (MTU) set in the nRF Connect SDK Bluetooth stack is 23 bytes. It means you can't send more data than can fit in the ATT MTU in one notification push. If you want to send more data in a single go, you need to increase this value; longer ATT payloads can be achieved, increasing the ATT throughput. This was covered in Lesson 3 – Exercise 2. Also, more details are available [here](https://docs.nordicsemi.com/bundle/ncs-latest/page/zephyr/connectivity/bluetooth/bluetooth-mtu-update.html).