## EESPHome Integration for a LED Pixel Board

![alt text](images/display.png)

This repository contains an ESPHome external component that enables control of a Bluetooth‐enabled Pixel Display. it is non intrusive - no modification of the display is required. It just emulates the iPixel Color App commands. 

## Table of Contents

- [Disclaimer](#disclaimer)
- [Part One: Why?](#part-one-why)
  - [How Can It Be Remotely Controlled?](#how-can-it-be-remotely-controlled)
  - [Is there a one for all solution?](#is-there-a-one-for-all-solution)
- [Part Two: Hasn’t This Been Done Already?](#part-two-hasnt-this-been-done-already)
- [Part Three: Inventory](#part-three-inventory)
    - [What is working](#what-is-working)
    - [What was going wrong?](#what-was-going-wrong)
    - [Knowing the problem is half of its solution](#knowing-the-problem-is-half-of-its-solution)
- [Part Four: External Component Implementation](#part-four-external-component-implementation)
  - [What Has Been Done?](#what-has-been-done)
  - [Key Insights](#key-insights)
  - [Framework Selection](#framework-selection)
  - [Future Work / TODO](#future-work--todo)


# **Disclaimer**
This is work in progress. Because everytrhing is working I wanted to use, I will not spend a lot of time for this repository now. Feel free to contribute or fork.

# Part One: Why?
I purchased a 32x32 LED Pixel Board fom Action in Germany. I wanted to integrate it in HomeAssistant via ESPHome.

## How Can It Be Remotely Controlled?
The display is controlled by the iPixel Color App via bluetooth
```
+--------------+                   #-----------------+
| iPixel Color |        BLE        |      32x32      |
|      APP     | < ------------ >  | LED Pixel Board |
+--------------+                   +-----------------+
```

## Is there a one for all solution?
Definitely the App only. The App is quite omplex providing several image databases and a drawing interface that will not fit into a micro cheap controller.

# Part Two: Hasn’t This Been Done Already?

Many have requested to control these devices, but none of the available solutions met my specific needs:
- My main usage of the display will be a nerdy clock visualization provided by the display firmware.
- Synchronizing the clock without having to use the app.
- Enabled to intrgrate with HomeAssistant / ESPhome.
- Send a text/alarm message from HomeAssistant automations.
- The existing approaches did not show text and images on my specific device.
- Cheap (using a RSP32 C3 super lite module about 3$).
- So far my Python knowledge is linited. I wanted to use my C++ skills.
- Get rid of any Arduino Framewort stuff, because in 2026 ESPHome will support the esp-idf only.

Fortunately some smart people already investigated into this type of display controlled with the Ipixel Color App
and figured out the BLE protocol required.

My credits go to:

- **[lucagoc/iPixel-CLI](https://github.com/lucagoc/iPixel-CLI)**  
  A Python command line solution, without HA integration.
  
- **[lucagoc/iPixel-CLI](https://github.com/lucagoc/iPixel-ESPHome**
  An ESPHome integration in progress using lambda functions supporting a few commands right now.
  
- **[ToBiDi0410/iPixel-ESP32](https://github.com/ToBiDi0410/iPixel-ESP32)**
  An ESP32 S3 Port using a REST API supporting all stuff known right now.
  Unfortunately some comands did fail with my display.

- **[Erriez/ErriezCRC32](https://github.com/Erriez/ErriezCRC32)**
  small and efficient CRC32 checksum calculator. I used my forked version because it is restricted for arduino use only. There are no dependencies to the arduino framework. I just had to modify the lirary.json.

- **[spacerace/romfont](https://github.com/spacerace/romfont)**
  used the MORSEKP800_8x16 for caracter bitmap rendering.

- **[warehog/esphome-diesel-heater-ble](https://github.com/warehog/esphome-diesel-heater-ble)**  
  I'm not that familar creating complex ESPHome components.  
  For my opinion a great and clean project to be used as skeleton for my component

# Part Three: Inventory

## What is working
All commands sendes a few bytes only, like led on/off set pixel, setting and selecting clock are worling.

## What was going wrong?
The display did not show any test and images with the existing repositorys.

## Knowing the problem is half of its solution
I did some BLE sniffig and figured out that this display does support monospaced 8x16, 16x16 and 32x16 Fontmatrixes only. Therefore sending testchars ist different from the existing implementations.

# Part Four: External Component Implementation
## What Has Been Done?
- I had to figure out a way to get color values and text and image data accesible in the code. 
- Selecting a set of useful parameters, sensors and buttons required to controll the device. 
- Ensured Extensibility: The component is designed to support other versions of the display.

## Key Insights
One major discovery is that no single protocol exists for communicating with all BLE-enabled controllers. Most existing documentation and repositories focus on one of at least other do exist only the iPixel Color app is aware of by evaluating firmware versions and display size. It requires people who own these devices, especially none working, to do some debugging.

## Framework Selection
The BLE stack occupies a significant amount of flash memory. Combined with several sensors, buttons, and other components, this quickly exceeded the 4MB flash capacity of my ESP32-C3 Super Lite. I found that the ESP-IDF framework produces a smaller binary compared to the Arduino framework. In future ESPHome will support the ESP-IDF only. It is strongly regcommended to use it.

## Future Work / TODO

- **Known issues:**
  -- switching off and on the light does sett the effect to none. Found no elegant way to propagate the last running effect to the light component. (pepared accordng the dev branch but the functions are not available in version 2025.2.0.
  -- 16x16 and 16x32 font bitmaps are generated by stupid doubling bits and rows with no smoothing. It was just an easy way to get it work.
  -- The yaml file has some internal comtonemts because they are not used right now.
  -- There has been a deep copy issue with huge strings runing out of memory. Did not check the entire code for unnecesary deeep copies of huge data.

- **Getting the display matrix size from the device itself**  
  Setting a fixed size isn't that smart;) The the app is able ti identify the display size.

- **Sending PNG and animated GIFs**
  Unfortunately the text (data) component is linited to 255 bytes. Setting the max_length parameter did not apply.  I need to figure out how to get a large amount of data into the ESOHome device. A http request using an URL pointing an image or reenabling the REST api may be a suggestion.

- **internal PNG and GIF to RGB converter**
  Unsing the OTA partition the ESP32 C3 Lite is alredy about 84% of its flash size. Reaching 90% causes instability for unknown reasons. Because I do not want to skip the OTA functionality, I will have to upgrade to a ESP32 S3 with 8MB flash memory for further deveolpment.

- ****  

- **and last but not least: implementing a display component**  
  This will enable to do some graphical stuff but will require a device like the ESP32 S3 with more flash memory. Be aware that sending huge amount of data via BLE takes it's time.
