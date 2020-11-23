# LwM2M Overview

The Lightweight Machine To Machine designed by the Open Mobile Alliance is a device management protocol for the Internet of Things (IoT).

## LwM2M Concepts

### LwM2M Client/Device

The LwM2M Client/Device is the entity being managed by the LwM2M Server. The LwM2M Client presents resources for the LwM2M Server to manipulate.

### LwM2M Server

The LwM2M Server is the entity managing the LwM2M Devices. It can Read, Write, Execute, Create or Delete the Devices resources.

The LwM2M Server URI can have one of the following formats:

- "coap://{hostname}\[:{port}\]" for unsecure UDP transport
- "coaps://{hostname}\[:{port}\]" for secure UDP transport (using DTLS)
- "coap+tcp://{hostname}\[:{port}\]" for unsecure TCP transport
- "coaps+tcp://{hostname}\[:{port}\]" for secure TCP transport (using TLS)
- "sms://{msisdn number}" for SMS binding in binary mode
- "lorawan://{FPort}" for LoRaWAN binding with FPort between 1 and 255

For UDP and TCP transports, if *{port}* is not present, the default port is used:

- *5683* for communication without security
- *5684* for secure communication

### LwM2M Bootstrap Server

The LwM2M Bootstrap Server is a special [LwM2M Server][LwM2M Server]. It is the only Server allowed to provision or delete Server Accounts on the LwM2M Devices. It can Read (LwM2M 1.1), Write, Create and Delete the Devices resources.

### LwM2M Resource

A LwM2M Resource is a data item presented by the LwM2M Device. A resource has a data type, a list of allowed operations and a unique URI.

Data types are defined in Lightweight M2M as follows:

* signed integer,
* unsigned integer,
* float,
* boolean,
* UTF-8 string,
* core link string,
* opaque binary,
* Unix time,
* reference to a LwM2M Object.

A resource can also be an array of the mentioned data types.

Operations on resources are Read, Write and Execute.

### LwM2M Objects

The LwM2M Objects are a group of Device resources essential for some Device features such as firmware update, connectivity monitoring, physical sensor, etc. Standardized LwM2M Objects contain mandatory resources to support and optional resources to implement.

A LwM2M Device can have several instances of the same LwM2M Object.

All the Device resources are part of a LwM2M Object.

### LwM2M Object Instance

Some LwM2M Objects are defined as multi-instance objects, meaning that a LwM2M Device can have several instances of the same LwM2M Object. These instances are identified by a numerical instance ID.

When a LwM2M Object is defined as a single-instance, the instance ID is always 0.

### LwM2M URI

Resources and Objects have well-known IDs. The URI to a LwM2M resource is in the form: Object_ID ["/" Object_Instance_ID ["/" Resource_ID ["/" Resource_Instance_ID]]]

An URI can address an object, an object instance, a resource or a resource instance (i.e. an element when the resource is an array).

### Server Account

A Server Account is a set of data enabling a LwM2M Device to connect to a LwM2M Server. It includes the Server URI and the Security Keys to use.

### Bootstrap

A Bootstrap is the process by which the Device retrieves the Server Accounts. The information can come from factory settings, a SmartCard, or a LwM2M Bootstrap Server.

### Registration

A LwM2M Server can only manage Devices registered with it.

When registering with the Bootstrap Server, the Device communicates only its unique ID.

When registering with the LwM2M Server, the Device can communicate:

* its unique ID,
* its lifetime,
* the list of LwM2M Objects implemented on the Device,
* its MSISDN,
* whether the Device requires Queue Mode or not.

### Observation

The LwM2M Server can subscribe to one or several readable Device resources. When the value of the resource changes, the Device sends a Notification message to the Server containing the new value of the resource.

### Lifetime

The Lifetime is the validity period of a LwM2M Device registration to the LwM2M Server. When this lifetime expires, the Server should no longer try to manage the Device.

The Device sends Registration Update messages to the Server to renew its registration validity period.

### Queue Mode

Devices may go in stand-by mode and thus not be reachable by the LwM2M Servers at any time. To cope with this, the Device requests the Server to operate in Queue Mode. In this mode, the Device will initiate the communication with the Server (when sending a Notification or a Registration Update message). In the meantime, the Server is expected to queue its requests (hence the name).

### SMS Trigger

A SMS Trigger is a special SMS sent to the Device. Upon reception of this SMS, the Device will either register to the Server or send a Registration Update message.

## LwM2M Operations Overview

### Bootstrap

When an endpoint is registered to a LwM2M Bootstrap Server, it can be managed by performing LwM2M operations on the endpoint's resources. The LwM2M operations are:

* Bootstrap Finish,
* Bootstrap Read,
* Bootstrap Write,
* Bootstrap Delete,
* Bootstrap Discover.

#### Bootstrap Request

The first action of LwM2M Client is to connect to the LwM2M Bootstrap Server and ask a Bootstrap Request. The Bootstrap Request message contains an endpoint’s unique ID.

#### Bootstrap Finish

A **Bootstrap Finish** operation is sent by the LwM2M Bootstrap Server. Upon reception of this operation, the LwM2M Client will perform a Bootstrap Consistency check to verify the Bootstrap information received from the LwM2M Bootstrap Server is valid. This means at least one LwM2M server is configured on the LwM2M Client.

#### Bootstrap Read

In LwM2M 1.1 or later, a **Bootstrap Read** operation can target the LwM2M Objects Access Control List or Server, or their LwM2M Object Instances. The endpoint returns data encoded in either TLV, JSON, SenML CBOR, or SenML JSON.

#### Bootstrap Write

A **Bootstrap Write** operation targets a LwM2M Object Instance. It contains data encoded in either TLV or JSON. The payload does not need to contain values for all the Object's resources.

In LwM2M 1.1 or in later version, it contains data encoded in either SenML CBOR or SenML JSON.

#### Bootstrap Delete

A **Bootstrap Delete** operation can target the root path, a LwM2M Object or a LwM2M Instance Object.

#### Bootstrap Discover

A **Bootstrap Discover** operation can target the root path or a LwM2M Object. The endpoint returns only the list of Objects and Object Instances with some attributes: LwM2M Enabler version ("lwm2m="), Short Server ID ("ssid="), and LwM2M Server URI ("uri=").

### Registration

#### Initial Registration

The first action of LwM2M Client is to register to the LwM2M Server. The registration message contains an endpoint’s unique ID and information about its capabilities (in the form of LwM2M Objects). The registration message also contains a *lifetime* parameter. When this *lifetime* expires, the LwM2M Server cancels the LwM2M Client registration.

The LwM2M Server replies with a "location". This location is a handle to the registration. The endpoint uses this handle to update or cancel its registration.

#### Registration Update

To keep its registration active or in case the list of LwM2M Objects is modified, the endpoint sends a Registration Update message.

Upon reception of this message, the LwM2M Server resets its internal *lifetime* timer.

#### De-registration

To cancel its registration, the LwM2M Client sends a De-registration message. The LwM2M Server may also automatically cancel an endpoint's registration if the registration *lifetime* expires.

### Device Management and Service Enablement

When an endpoint is registered to a LwM2M Server, it can be managed by performing LwM2M operations on the endpoint's resources. The LwM2M operations are:

* Read,
* Write,
* Execute,
* Discover,
* Create,
* Delete,
* Read composite,
* Write composite.

#### Read

A **Read** operation can target a LwM2M Object, a LwM2M Object Instance or a LwM2M Resource. The endpoint returns data encoded in either TLV or JSON. For a single resource, a text encoding is possible.

In LwM2M 1.1 or later, the endpoint returns data encoded in either SenML CBOR or SenML JSON.

#### Write

A **Write** operation targets a LwM2M Object Instance. It contains data encoded in either TLV or JSON. The payload does not need to contain values for all the Object's resources.

In LwM2M 1.1 or in later version, it contains data encoded in either SenML CBOR or SenML JSON.

#### Execute

An **Execute** operation can only target a LwM2M Resource.

#### Discover

A **Discover** operation can target a LwM2M Object, a LwM2M Object Instance or a LwM2M Resource.

#### Create

A **Create** operation can target a LwM2M Object. It creates a new Object Instance.

#### Delete

A **Delete** operation can target a LwM2M Object Instance. It deletes an Object Instance.

#### Read composite

In LwM2M 1.1 or in later version, a **Read composite** operation can selectively Read a number of Resources, and/or Resource Instances of different Objects in a single request. The endpoint returns data encoded in either SenML CBOR or SenML JSON.

#### Write composite

In LwM2M 1.1 or in later version, a **Write composite** operation can update values of a number of different Resources across different Instances of one or more Objects in a single request. It contains data encoded in either SenML CBOR or SenML JSON.

### Information Reporting

When a LwM2M Client is registered to a LwM2M Server, its resources can be observed. Whenever the value of the resource changes, the LwM2M Client sends a notification to the LwM2M Server. Moreover in LwM2M 1.1, the Client can send data to the LwM2M Server without initial solicitation. The LwM2M operations are:

* Observation,
* Observation composite,
* Cancel Observation,
* Write attributes,
* Send.

#### Observation

An observation can target a LwM2M Object, a LwM2M Object Instance or a LwM2M Resource.

#### Observe composite

In LwM2M 1.1 or in later version, a **Observe composite** operation can initiate observations or selectively read a group of Object, Object Instance, Resources, and/or Resource Instances of different Objects in a single request. The endpoint returns data encoded in either SenML CBOR or SenML JSON.

#### Notification

When an observed resource's value changes, the endpoint sends a notification message to the LwM2M Server.

#### Cancel Observation

The LwM2M Server can cancel an on-going observation.

#### Write attributes

The LwM2M Server can set the parameters of an observation. The following attributes are:

| Name | Level | Description |
| ---- | --------------- | ------------------------------------------------ |
| pmin | Object, Object Instance, Resource | The minimum period in seconds to wait between notifications. |
| pmax | Object, Object Instance, Resource | The maximum period in seconds to wait between notifications. |
| gt | Numerical Resource | An upper threshold. A notification is sent when the resource value crosses this threshold. |
| lt | Numerical Resource | A lower threshold. A notification is sent when the resource value crosses this threshold. |
| st | Numerical Resource | A minimum difference in a resource value for a notification to be sent. |
| epmin | Object, Object Instance, Resource | The minimum sample time in seconds for the observed sensor in LwM2M 1.1 or in later version. |
| epmax | Object, Object Instance, Resource | The maximum sample time in seconds for the observed sensor in LwM2M 1.1 or in later version. |

Setting an attribute is in the form `Name "=" value` with some constraints:

* `lt` value < `gt` value
* `lt` value + 2 * `st` value < `gt` value
* If `pmax` < `pmin`, `pmax` is ignored
* `epmax` > `epmin`

Clearing an attribute is in the form `Name`.

##### Examples

* Receiving a notification every minute at most even if the observed URI did not change: `"pmax=60"`.

* Receiving only one notification per hour even if the observed URI changed several times per minute: `"pmin=3600"`.

* Receiving exactly one notification every sixty seconds: `"pmin=59&pmax=60"`.

* Receiving a notification when the resource value exceeds 95 or falls below 10, and when the resource value returns below 95 or above 10: `"lt=10&gt=95"`.

* Clearing the previously set minimum period and setting a maximum period of five minutes: `"pmin&pmax=300"`.

#### Send

In LwM2M 1.1 or in later version, a **Send** operation is used by the LwM2M Client to send data to the LwM2M Server without explicit request by that Server. It contains data encoded in either SenML CBOR or SenML JSON.

Later in the documentation, the **Send** operation is called a Data Push.

## Standardized LwM2M Objects

The list of publics LwM2M Objects is available at the OMNA Registry:
<http://www.openmobilealliance.org/wp/OMNA/LwM2M/LwM2MRegistry.html>

### Security Object

[LwM2M Registry for Object 0](https://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http://www.openmobilealliance.org/tech/profiles/LWM2M_Security-v1_1.xml)

This object provides the keying material of a LwM2M Client appropriate to access a specified LwM2M Server. One Object Instance should address a LwM2M Bootstrap-Server.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | LwM2M Server URI | | Yes | String | Uniquely identifies the LwM2M Server or LwM2M Bootstrap-Server. The format of the CoAP URI is defined in Section 6 of RFC 7252. |
| 1 | Bootstrap-Server | | Yes | Boolean | Determines if the current instance concerns a LwM2M Bootstrap-Server (true) or a standard LwM2M Server (false). |
| 2 | Security Mode | | Yes | Integer | Determines which UDP payload security mode is used. |
| 3 | Public Key or Identity | | Yes | Opaque | Stores the LwM2M Client’s Certificate (Certificate mode), public key (RPK mode) or PSK Identity (PSK mode). |
| 4 | Server Public Key | | Yes | Opaque | Stores the LwM2M Server’s or LwM2M Bootstrap-Server’s Certificate (Certificate mode), public key (RPK mode). |
| 5 | Secret Key | | Yes | Opaque | Stores the secret key or private key of the security mode. |
| 6 | SMS Security Mode | | No | Integer | Determines which SMS security mode is used. |
| 7 | SMS Binding Key Parameters | | No | Opaque | Stores the KIc, KID, SPI and TAR. |
| 8 | SMS Binding Secret Key(s) | | No | Opaque | Stores the values of the key(s) for the SMS binding. |
| 9 | LwM2M Server SMS Number | | No | String | MSISDN used by the LwM2M Client to send messages to the LwM2M Server via the SMS binding. |
| 10 | Short Server ID | | No | Integer | This identifier uniquely identifies each LwM2M Server configured for the LwM2M Client. |
| 11 | Client Hold Off Time | | No | Integer | Relevant information for a Bootstrap-Server only. |
| 12 | Bootstrap-Server Account Timeout | | No | Integer | The LwM2M Client MUST purge the LwM2M Bootstrap-Server Account after the timeout value given by this resource. The lowest timeout value is 1. |
| 13 | Matching Type | | No | Unsigned Integer | The Matching Type Resource specifies how the certificate or raw public key in in the Server Public Key is presented. |
| 14 | SNI | | No | String | This resource holds the value of the Server Name Indication (SNI) value to be used during the TLS handshake. |
| 15 | Certificate Usage | | No | Unsigned Integer | The Certificate Usage Resource specifies the semantic of the certificate. |
| 16 | DTLS/TLS Ciphersuite | | No | Unsigned Integer | It instructs the TLS/DTLS client to propose the indicated ciphersuite(s) in the ClientHello of the handshake |
| 17 | OSCORE Security Mode | | No | Object link | It provides a link to the OSCORE Object Instance. |

### Server Object

[LwM2M Registry for Object 1](https://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http://www.openmobilealliance.org/tech/profiles/LWM2M_Server-v1_1.xml)

This object provides the data related to a LwM2M Server. A Bootstrap-Server has no such an Object Instance associated to it.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Short Server ID | R | Yes | Integer | Used as link to associate server Object Instance. |
| 1 | Lifetime | RW | Yes | Integer | Specify the lifetime of the registration in seconds. |
| 2 | Default Minimum Period | RW | No | Integer | The default value the LwM2M Client should use for the Minimum Period of an Observation in the absence of this parameter being included in an Observation. |
| 3 | Default Maximum Period | RW | No | Integer | The default value the LwM2M Client should use for the Maximum Period of an Observation in the absence of this parameter being included in an Observation. |
| 4 | Disable | E | No | | If this Resource is executed, this LwM2M Server Object is disabled for a certain period defined in the Disabled Timeout Resource. |
| 5 | Disable Timeout | RW | No | Integer | A period to disable the Server. |
| 6 | Notification Storing When Disabled or Offline | RW | Yes | Boolean | If true, the LwM2M Client stores "Notify" operations to the LwM2M Server while the LwM2M Server account is disabled or the LwM2M Client is offline. |
| 7 | Binding | RW | Yes | String | This Resource defines the transport binding configured for the LwM2M Client. |
| 8 | Registration Update Trigger | E | Yes | | If this Resource is executed the LwM2M Client MUST perform an "Update" operation with this LwM2M Server using that transport for the Current Binding Mode. |
| 9 | Bootstrap-Request Trigger | E | No | | If this Resource is executed the LwM2M Client MUST initiate a "Client Initiated Bootstrap" procedure in using the LwM2M Bootstrap-Server Account. |
| 10 | APN Link | RW | No | Object link | It provides a link to the APN connection profile Object Instance (OMNA registered Object ID:11) to be used to communicate with this server. |
| 11 | TLS-DTLS Alert Code | R | No | Unsigned Integer | It contains the most recent TLS / DTLS alert message received from the LwM2M Server. |
| 12 | Last Bootstrapped | R | No | Time | It represents the last time that the bootstrap server updated this LwM2M Server Account. |
| 13 | Registration Priority Order | | No | Unsigned Integer | The LwM2M Client sequences the LwM2M Server registrations in increasing order of this value. |
| 14 | Initial Registration Delay Timer | | No | Unsigned Integer | The delay before registration is attempted for this LwM2M Server based upon the completion of registration of the previous LwM2M Server in the registration order. |
| 15 | Registration Failure Block | | No | Boolean | Prevent or not the registration on the next LwM2M Server when the registration fails to connect with this LwM2M server. |
| 16 | Bootstrap on Registration Failure | | No | Boolean | Initiate a Bootstrap Request or not when the registration fails to connect with this LwM2M server. |
| 17 | Communication Retry Count | | No | Unsigned Integer | The number of successive communication attempts before which a communication sequence is considered as failed. |
| 18 | Communication Retry Timer | | No | Unsigned Integer | The delay between successive communication attempts in a communication sequence. |
| 19 | Communication Sequence Delay Timer | | No | Unsigned Integer | The delay between successive communication sequences. |
| 20 | Communication Sequence Retry Count | | No | Unsigned Integer | The number of successive communication sequences before which a registration attempt is considered as failed. |
| 21 | Trigger | RW | No | Boolean | Using the Trigger Resource a LwM2M Client can indicate whether it is reachable over SMS (value set to 'true') or not (value set to 'false'). |
| 22 | Preferred Transport | RW | No | String | When the LwM2M client supports multiple transports, it MAY use this transport to initiate a connection. |
| 23 | Mute Send | RW | No | Boolean | De-activated the LwM2M Client Send command capability. |

### Access Control List Object

[LwM2M Registry for Object 2](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Access_Control-v1_0_2.xml)

This object is used to check whether the LwM2M Server has access right for performing an operation.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Object ID | R | Yes | Integer | Used as link to associate Object which the access right is applicable. |
| 1 | Object Instance ID | R | Yes | Integer | Used as link to associate Object Instance which the access right is applicable. |
| 2 | ACL | RW | No | Integer | Set the access right for the corresponding LwM2M Servers. |
| 3 | Access Control Owner | RW | Yes | Integer | Short Server ID of a LwM2M Server. Only this LwM2M Server can manage the Resources of this Object Instance. |

### Device Object

[LwM2M Registry for Object 3](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Device-v1_1.xml)

This object contains the device information: model, manufacturer, serial number, power source type, battery level, etc... It also contains a resource allowing the Server to reboot the Device.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Manufacturer | R | No | String | Human readable manufacturer name |
| 1 | Model Number | R | No | String | A model identifier (manufacturer specified string) |
| 2 | Serial Number | R | No | String | Serial Number |
| 3 | Firmware Version | R | No | String | Current firmware version of the Device. |
| 4 | Reboot | E | Yes | none | Reboot the LwM2M Device. |
| 5 | Factory Reset | E | No | none | Perform factory reset of the LwM2M Device to make the LwM2M Device to go through initial deployment sequence where provisioning and bootstrap sequence is performed. |
| 6 | Available Power Sources | R | No | Integer array | Available power sources of the Device. |
| 7 | Power Source Voltage | R | No | Integer array | Present voltage for each Available Power Sources. |
| 8 | Power Source Current | R | No | Integer array | Present current for each Available Power Sources. |
| 9 | Battery Level | R | No | Integer | Contains the current battery level as a percentage. |
| 10 | Memory Free | R | No | Integer | Estimated current available amount of storage space in the Device (expressed in kilobytes). |
| 11 | Error Code | R | Yes | Integer array | List of errors experienced by the Device. |
| 12 | Reset Error Code | E | No | none | Delete all error codes in the previous resource. |
| 13 | Current Time | R/W | No | Time | Current UNIX time of the LwM2M Client. |
| 14 | UTC Offset | R/W | No | String | Indicates the UTC offset currently in effect for this LwM2M Device. |
| 15 | Timezone | R/W | No | String | Indicates in which time zone the LwM2M Device is located. |
| 16 | Supported Bindings | R | Yes | String | Indicates which transports are supported by the Device. |
| 17 | Device Type | R | No | String | Type of the device (manufacturer specified string). |
| 18 | Hardware Version | R | No | String | Current hardware version of the device. |
| 19 | Software Version | R | No | String | Current software version of the device. |
| 20 | Battery Status | R | No | Integer | Status of the Device battery (normal, charging, etc...) |
| 21 | Memory Total | R | No | Integer | Total amount of storage space in the Device (expressed in kilobytes). |
| 22 | ExtDevInfo | R | No | Object link | Reference to a vendor specific object containing device information. |

### Connectivity Monitoring Object

[LwM2M Registry for Object 4](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Connectivity_Monitoring-v1_2.xml)

This object provides high-level information on the current network type, signal strength, IP address, etc…

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Network Bearer | R | Yes | Integer | The network bearer used for the current LwM2M communication session. |
| 1 | Available Network Bearer | R | Yes | Integer array | List of current available network bearer. |
| 2 | Radio Signal Strength | R | Yes | Integer | The average value of the received signal strength. |
| 3 | Link Quality | R | No | Integer | The received link quality. |
| 4 | IP Addresses | R | Yes | String array | The IP addresses assigned to the connectivity interface. |
| 5 | Router IP Addresses | R | No | String array | The IP addresses of the next-hop IP routers. |
| 6 | Link Utilization | R | No | Integer | The average utilization of the link in %. |
| 7 | APN | R | No | String array | Access Point Names in case Network Bearer Resource is a Cellular Network. |
| 8 | Cell ID | R | No | Integer | Serving Cell ID in case Network Bearer Resource is a Cellular Network. |
| 9 | SMNC | R | No | Integer | Serving Mobile Network Code. |
| 10 | SMCC | R | No | Integer | Serving Mobile Country Code. |
| 11 | SignalSNR | R | No | Integer | Signal to Interference plus Noise Ratio SINR is the ratio of the strength of the received signal to the strength of the received interference signal. |
| 12 | LAC | R | No | Integer | Location Area Code in case Network Bearer Resource is a Cellular Network. |

### Firmware Update Object

[LwM2M Registry for Object 5](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Firmware_Update-v1_0_1.xml)

This object allows to update the firmware of the device. The firmware package can either be pushed by the Server to the Device or the Device can download it using the provided URI.

|ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Package | W | Yes | Opaque | The firmware package pushed by the Server. |
| 1 | Package URI | R/W | Yes | String | The URI from where the Device can download the firmware package. |
| 2 | Update | E | Yes | none | Updates the Device firmware by using the firmware package. |
| 3 | State | R | Yes | Integer | Indicates current state with respect to this firmware update. |
| 5 | Update Result | R | Yes | Integer | Contains the result of downloading or updating the firmware. |
| 6 | PkgName | R | No | String | The name of the Firmware Package. |
| 7 | PkgVersion | R | No | String | The version of the Firmware package. |
| 8 | Firmware Update Protocol Support | R | No | Integer | The download protocols the Device implements. |
| 9 | Firmware Update Delivery Method | R | Yes | Integer | The delivery methods the Device supports. |

### Location Object

[LwM2M Registry for Object 6](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Location-v1_0.xml)

This object contains information on the device position and speed.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Latitude | R | Yes | Float | The decimal notation of latitude. |
| 1 | Longitude | R | Yes | Float | The decimal notation of longitude. |
| 2 | Altitude | R | No | Float | The decimal notation of altitude in meters above sea level. |
| 3 | Radius | R | No | Float | The size in meters of a circular area around a point of geometry. |
| 4 | Velocity | R | No | Opaque | The velocity in the Device. |
| 5 | Timestamp | R | No | Time | The timestamp of when the location measurement was performed. |
| 6 | Speed | R | No | Float | The speed of the Device in meters per second. |

### Connectivity Statistics Object

[LwM2M Registry for Object 7](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Connectivity_Statistics-v1_0_1.xml)

This object collects statistics on the network usage.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | SMS Tx Counter | R | No | Integer | The total number of SMS successfully transmitted during the collection period. |
| 1 | SMS Rx Counter | R | No | Integer | The total number of SMS successfully received during the collection period. |
| 2 | Tx Data | R | No | Integer | The total amount of IP data transmitted during the collection period. |
| 3 | Rx Data | R | No | Integer | The total amount of IP data received during the collection period. |
| 4 | Max Message Size | R | No | Integer | The maximum IP message size that is used during the collection period. |
| 5 | Average Message Size | R | No | Integer | The average IP message size that is used during the collection period. |
| 6 | Start | E | Yes | none | Start to collect information. |
| 7 | Stop | E | Yes | none | Stop collecting information. |
| 8 | Collection Period | R/W | No | Integer | The default collection period in seconds. |

### Software Management Object

[LwM2M Registry for Object 9](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Software_Management-v1_0.xml)

This object allows to update the software of the device. The software package can either be pushed by the Server to the Device or the Device can download it using the provided URI.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | PkgName | R | Yes | String | Name of the software package. |
| 1 | PkgVersion | R | Yes | String | Version of the software package. |
| 2 | Package | W | No | Opaque | The software package pushed by the Server. |
| 3 | Package URI | W | No | String | The URI from where the Device can download the software package. |
| 4 | Install | E | Yes | none | Install the software package. |
| 5 | Checkpoint | R | No | Objlink | Link to a Checkpoint object which allows to specify conditions/dependencies for a software update. |
| 6 | Uninstall | E | Yes | none | Uninstall the software package. |
| 7 | Update State | R/W | No | Boolean | Indicates current state with respect to this software update. |
| 8 | Update Supported Object | R | Yes | Integer | Indicates if the LwM2M Client MUST inform the registered LwM2M Servers of Objects and Object Instances parameter by sending an Update or Registration message after the software update operation. |
| 9 | Update Result | R | Yes | Integer | Contains the result of downloading or updating the software. |
| 10 | Activate | E | Yes | none | Activate the previously installed software package. |
| 11 | Deactivate | E | Yes | none | Deactivate the previously installed software package. |
| 12 | Activation State | R | Yes | Boolean | Indicate the current activation state of the software. |
| 13 | User Name | W | No | String | User name for access to software Update Package in pull mode. |
| 14 | Password | W | No | String | Password for access to software Update Package in pull mode. |
| 15 | Software Component Link | R | No | Objlink | Reference to SW Components downloaded and installed in scope of the present SW Update Package. |

### Cellular Connectivity Object

[LwM2M Registry for Object 10](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Cellular_connectivity-v1_0.xml)

This object is used to configure the cellular connectivity of the Device.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 11 | Activated Profile Names | R | Yes | String array | Links to instances of the "APN connection profile" object. |
| 0 | SMSC address | R/W | No | String | E.164 address of SMSC. |
| 1 | Disable radio period | R/W | No | Integer | Time period for which the device shall disconnect from cellular radio. |
| 2 | Module activation code | R/W | No | String | AT command to activate the module. |
| 3 | Vendor specific extensions | R | No | Object Link | Link to a vendor specific object. |
| 4 | PSM Timer | R/W | No | Integer | Power saving mode timer. |
| 5 | Active Timer | R/W | No | Integer | Active timer = T3324 as defined in [3GPP-TS_24.008]. |
| 6 | Serving PLMN Rate control | R | No | Integer | Maximum the number of allowed uplink PDU transmissions per 6 minute interval. |
| 7 | eDRX parameters for Iu mode | R/W | No | Opaque | Extended DRX parameters (Paging Time Window and eDRX value) for Iu mode. |
| 8 | eDRX parameters for WB-S1 mode | R/W | No | Opaque | Extended DRX parameters (Paging Time Window and eDRX value) for WB-S1 mode. |
| 9 | eDRX parameters for NB-S1 mode | R/W | No | Opaque | Extended DRX parameters (Paging Time Window and eDRX value) for NB-S1 mode. |
| 10 | eDRX parameters for A/Gb mode | R/W | No | Opaque | Extended DRX parameters (Paging Time Window and eDRX value) for A/Gb mode. |

### APN Connection Profile Object

[LwM2M Registry for Object 11](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_APN_connection_profile-v1_0.xml)

This object specifies resources to enable a device to connect to an APN.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Profile name | R/W | Yes | String | Human-readable identifier. |
| 1 | APN | R/W | No | String | Presented to network during connection to PDN. |
| 2 | Auto select APN by device | R/W | No | Boolean | Enables the device to choose an APN according to a device specific algorithm. |
| 3 | Enable status | R/W | No | Boolean | Connection is activated or not. |
| 4 | Authentication Type | R/W | Yes | Integer | . |
| 5 | User Name | R/W | No | String | . |
| 6 | Secret | R/W | No | String | . |
| 7 | Reconnect Schedule | R/W | No | String | Comma separated list of retry delay values in seconds. |
| 8 | Validity (MCC, MNC) | R/W | No | String array | Coma separated mobile country code, then mobile network code. |
| 9 | Connection establishment time | R | No | Time array | UTC time of connection request. |
| 10 | Connection establishment result | R | No | Integer array | . |
| 11 | Connection establishment reject cause | R | No | Integer array | Reject cause, see [3GPP-TS_24.008, 3GPP-TS_24.301]. |
| 12 | Connection end time | R | No | Time array | UTC time of connection end. |
| 13 | TotalBytesSent | R | No | Integer array | Rolling counter for total number of bytes sent via this interface since last device reset. |
| 14 | TotalBytesReceived | R | No | Integer | Rolling counter for total number of bytes received via this interface since last device reset. |
| 15 | IP address | R/W | No | String array | IP addresses. |
| 16 | Prefix length | R/W | No | String array | Associated with IPv6 addresses. |
| 17 | Subnet mask | R/W | No | String array | Subnet masks. |
| 18 | Gateway | R/W | No | String array | Gateways. |
| 19 | Primary DNS address | R/W | No | String array | Primary DNS addresses. |
| 20 | Secondary DNS address | R/W | No | String array | Secondary DNS addresses. |
| 21 | QCI | R | No | Integer | Quality of service Class Identifier. |
| 22 | Vendor specific extensions | R | No | Objlnk | Links to a vendor specific object. |
| 23 | TotalPacketsSent | R | No | Integer | Rolling counter for total number of packets sent via this interface since last device reset. |
| 24 | PDN Type | R/W | No | Integer | . |
| 25 | APN Rate Control | R | No | Integer | Determines the number of allowed uplink PDU transmissions per time interval per APN. |

### Bearer selection Object

[LwM2M Registry for Object 13](http://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_Bearer_selection-v1_0.xml)

This object specifies resources to enable a device to choose a PLMN/network on which to attach/register and what type of bearer to then connect.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Preferred Communications Bearer | R/W | No | Integer array | Used in network selection and, if applicable, in subsequent mobility management procedures. |
| 1 | Acceptable RSSI (GSM) | R/W | No | Integer | Provides guide to the application when performing manual network selection. |
| 2 | Acceptable RSCP (UMTS) | R/W | No | Integer | Provides guide to the application when performing manual network selection. |
| 3 | Acceptable RSRP (LTE) | R/W | No | Integer | Provides guide to the application when performing manual network selection. |
| 4 | Acceptable RSSI (1xEV-DO) | R/W | No | Integer | Provides guide to the application when performing manual network selection. |
| 5 | Cell lock list | R/W | No | String | Comma separated list of allowed Global Cell Identities. |
| 6 | Operator list | R/W | No | String | Comma separated list of MCC+MNC of operators, in priority order. |
| 7 | Operator list mode | R/W | No | Boolean | Indicates whether resource "operator list" represents the allowed operator list (white list), or, the preferred operator list. |
| 8 | List of available PLMNs | R | No | String | Allows server to see results of network scan. |
| 9 | Vendor specific extensions | R | No | Object link | Links to a vendor specific object. |
| 10 | Acceptable RSRP (NB-IoT) | R/W | No | Integer | Provides guide to the application when performing manual network selection. |
| 11 | Higher Priority PLMN Search Timer | R/W | No | Integer | Interval between periodic searches for higher priority PLMN. |
| 12 | Attach without PDN connection | R/W | No | Boolean |.|

### Software Component Object

[LwM2M Registry for Object 14](https://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=https%3a%2f%2fwww.openmobilealliance.org%2ftech%2fprofiles%2fLWM2M_Software_Component-v1_0.xml)

This object provides the resources needed to activate/deactivate software components on the device.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | Component Identity | R | No | String | Name or identifier of the software component. |
| 1 | Component Pack | R | No | Opaque | Software components is stored in this resource. |
| 2 | Component Version | R | No | String | Version of the software component. |
| 3 | Activate | E | No | none | Activate the installed software component. |
| 4 | Deactivate | E | No | none | Deactivate the installed software component. |
| 5 | Activation State | R | No | Boolean | Indicate the current activation state of the software. |

### LwM2M OSCORE

[LwM2M Registry for Object 21](https://devtoolkit.openmobilealliance.org/OEditor/LWMOView?url=http%3A%2F%2Fwww.openmobilealliance.org%2Ftech%2Fprofiles%2FLWM2M_OSCORE-v1_0_1.xml)

This object provides the resources needed to activate/deactivate software components on the device.

| ID | Resource Name | Access Type | Mandatory | Type | Description |
| ------ | -------------- | ------- | ---------- | -------- | ------------------------------------------------- |
| 0 | OSCORE Master Secret | | Yes | String | Store the pre-shared key used in LwM2M Client and LwM2M Server/Bootstrap-Server, called the Master Secret. |
| 1 | OSCORE Sender ID | | Yes | String | Store an OSCORE identifier for the LwM2M Client called the Sender ID. |
| 2 | OSCORE Recipient ID | | Yes | String | Store an OSCORE identifier for the LwM2M Client called the Recipient ID. |
| 3 | OSCORE AEAD Algorithm | | No | Integer | Store the encoding of the AEAD Algorithm as defined in Table 10 of RFC 8152. The AEAD is used by OSCORE for encryption and integrity protection of CoAP message fields. |
| 4 | OSCORE HMAC Algorithm | | No | Integer | Store the encoding of the HMAC Algorithm used in the HKDF. The encoding of HMAC algorithms are defined in Table 7 of RFC 8152. The HKDF is used to derive the security context used by OSCORE. |
| 5 | OSCORE Master Salt | | No | String | Store a non-secret random value called the Master Salt. The Master Salt is used to derive the security context used by OSCORE. |
