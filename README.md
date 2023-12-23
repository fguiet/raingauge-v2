# Raingauge v2

## Programming the ATtiny85

Here is the ATtiny 85 pinout

![](images/attiny85-pinout.png)

Using Arduino IDE v2, use this tutorial : <https://www.instructables.com/How-to-Program-an-Attiny85-From-an-Arduino-Uno/>

1. Use this following URL to add ATtiny Board to Arduino IE <https://raw.githubusercontent.com/damellis/attiny/ide-1.6.x-boards-manager/package_damellis_attiny_index.json>. File => Preferences => Aditional Board URL
2. Install the ATtiny boards (Board Manager on the menu on the left, then Install)
3. __Burn the ArduinoISP Exemple in the Arduino Board (select Arduino Board)__ => Don't forget that ! and make sure ATTiny pin are well connected to the bread board
4. Select ATtiny 85 board
5. Select Clock Internal 1 Mhz (it is ok for this project)
6. Select Programmer => Arduino as ISP
7. Burn Boatloader
8. Open sketch to upload then click on the menu Sketch => Upload using programmer

A quick overview of the Arduino and ATtiny 85 connexion.

![](images/arduino_attiny85_connexion.png)

__To program (Arduino <=> ATtiny 85):__

VCC (3.3v ou 5V) <=> VCC (ATtiny pin8)
Arduino GND <=> GND (ATtiny pin4)
Arduino Pin 10 <=> ATtiny Pin1
Arduino Pin 11 <=> ATtiny Pin5
Arduino Pin 12 <=> ATTiny Pin6
Arduino Pin 13 <=> ATTiny Pin7

## Configuring XBee S1 with XCTU v6.5.13

I am using old XBee S1

### XBee firmware update

Update the XBee firmware with last version using XCTU v6.5.13

![](images/reset_xbee.jpg)

Click on Default (To reset XBee to factory default), then Write
Click on Update to upload the last firmware available for this XBee S1 unit

In my case as of 2023/12 :

![](images/last_xbee_s1_version.jpg)

To make the Xbee talks to each other just config them with the same Channel and PAN (Personnal Area Network ID). Messages will be broadcasted.

![](images/xbee_address_settings.jpg)

### Test XBee communication

You can use XCTU to test that a mesage is correcly sent.

In my case I have got an Raspberry Pi with NodeRed installed and an XBeee connected to the Raspberry Pi.

Just use the XCTU Console and send a test packet, here : "Rainguage projet rocks!" in a JSON format

![](images/xctu_send_message.png)

Here a part of the NodeRed flow I am using.

![](images/nodered_xbee.png)

Careful, you must configure the NodeRed serial flow to split a message as soon as the '}' character is reveiced (end of JSON message)

![](images/nodered_serial_node_config.png)

If everything is going well then you should received the message on the NodeRed debug console

![](images/nodered_debug_message_received.png)

### XBee pinout

![](images/xbee_pinout.jpg)

### Sleep mode

To config the Xee to use the sleep mode, just change the sleep mode setting

![](images/xbee_sleep_mode.jpg)

Sleep mode will be control by pin 9, see <https://www.digi.com/resources/documentation/Digidocs/90001456-13/reference/r_wk_sleep_mode.htm>

PIN 9 HIGH : Xbee sleeps
PIN 9 DOWN : Xbee awake

## Librairies used in this project

To communicate with the XBee (send data), this project used the following library : `ATtinySerialOut`, version 2.2.0 (2023/12)

GitHub : <https://github.com/ArminJo/ATtinySerialOut/>

![](images/attinyserialout-lib.png)


## Freecad

__Some notes__

* On ne peut pas utiliser le mirroring sur la pièce que l'on veut "mirrorer" n'a pas de lien avec un objet du body en cours (pas de corps multiples dans un body)

* Utilisation de ShapeBinder pour pouvoir avoir des points de repères d'un autre body (voir vidéo : <https://www.youtube.com/watch?v=KYD9Ojugi8Q>)

### Support tipping bucket

![](images/support_tipping_bucket.png)

Les trous pour placer les inserts font 4.2mm sur Freecad. les inserts à utiliser sont des M3*4 (4 mm de haut pour type vis M3)

## References

* Madman's Github : <https://github.com/PricelessToolkit/MailBoxGuard>

* ATTiny and LoRa : <https://github.com/novag/LoRa-ATtiny-Node>

* MrDIY : <https://gitlab.com/MrDIYca/ultra-low-power-trigger-sensors-version-3>

* [MakerMeik : ESP8266 as Window Sensor with years of battery life](https://www.youtube.com/watch?v=vxbuO1zWo3w)

* [ESP32-C3 based Smart Door/Window sensor | DEEP SLEEP 30nA? | Long battery life 5/10 years](https://www.youtube.com/watch?v=he_Z-mDLnlM)

* [Raingauge version 1](https://www.youtube.com/watch?v=DnNW4rJkFhY&t=649s)

* [Raingauge version 2](https://www.youtube.com/watch?v=P2-hs2m6eCE)

* [Communication XBee](https://www.redohm.fr/2015/03/communication-xbee/)

* [Configuration XBee](https://www.electro-info.ovh/
configuration-d-un-module-XBEE-avec-XCTU)

* [Un programme qui ressemble a ce que je fais](https://github.com/cano64/ATTiny85-ATTiny84-BMP085-Arduino-Library-FastAltitude/issues/1)

* [How works a switch without neutral](https://www.youtube.com/watch?v=VNYcD7MEp4A)