## EESPHome Integration for a LED Pixel Board

<img src="docs/LEDPixelBoard.jpg" alt="LED Pixel Board" width="600">

This repository contains an ESPHome external component that enables control of a Bluetooth‐enabled Pixel Display. It is non intrusive - no modification of the display is required. It just emulates the known iPixel Color App commands.

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
- [Part Five: Future Work / ToDo](#part-five-future-work--todo)


## **Disclaimer**
This is work in progress. Because everytrhing is working I wanted to use, I will not spend a lot of time for this repository now. Feel free to contribute or fork.

## Part One: Why?
I purchased a 32x32 LED Pixel Board fom Action in Germany. I wanted to integrate it in HomeAssistant via ESPHome.

### How Can It Be Remotely Controlled?
The display is controlled by the iPixel Color App via bluetooth
```
+--------------+                   #-----------------+
| iPixel Color |        BLE        |      32x32      |
|      APP     | < ------------ >  | LED Pixel Board |
+--------------+                   +-----------------+
```

### Is there a one for all solution?
Definitely the App only. The App is quite complpex providing several image databases and a drawing interface that will not fit into a cheap micro controller.

## Part Two: Hasn’t This Been Done Already?

Many have requested to control these devices, but none of the available solutions met my specific needs:
- My main usage of the display will be a nerdy clock visualization provided by the display firmware.
- Synchronizing the time without having to use the app.
- Enabled to intrgrate with HomeAssistant / ESPhome.
- Send a text message from HomeAssistant automations.
- The existing approaches did not show text and images on my specific 32x32 device.
- Cheap (using a RSP32 C3 super lite module about 3$).
- So far my Python knowledge is limited. I wanted to use my C++ skills.
- Get rid of any Arduino framewort stuff, because in 2026 ESPHome will support the esp-idf only.

Fortunately some smart people already investigated into this type of display controlled with the IPixel Color App
and figured out most of the BLE protocol required.

**My credits go to:**

- **[lucagoc/pypixelcolor](https://github.com/lucagoc/pypixelcolor)**  
  A Python command line solution, without HA integration.
  
- **[lucagoc/iPixel-ESPHome](https://github.com/lucagoc/iPixel-ESPHome)**  
  An ESPHome integration in progress using lambda functions supporting a few commands right now.
  
- **[ToBiDi0410/iPixel-ESP32](https://github.com/ToBiDi0410/iPixel-ESP32)**  
  An ESP32 S3 Port using a REST API supporting all stuff known right now.
  Unfortunately some comands did fail with my display.

- **[Erriez/ErriezCRC32](https://github.com/Erriez/ErriezCRC32)**  
  A small and efficient CRC32 checksum calculator. I used my forked version because it is restricted for arduino use only. There are no dependencies to the arduino framework. I just had to modify the library.json.

- **[spacerace/romfont](https://github.com/spacerace/romfont)**  
  The MORSEKP800_8x16 font is used for caracter bitmap rendering. I just made te german 'ß' taller. Even in 8x16 size this font is readable well from distance. 

- **[warehog/esphome-diesel-heater-ble](https://github.com/warehog/esphome-diesel-heater-ble)**  
  I'm not that familar creating complex ESPHome components.  
  For my opinion tis is a great and clean project to be used as skeleton for my external component.

## Part Three: Inventory

### What is working
All commands sending a few bytes only, like led on/off set pixel, setting and selecting clock are working.

### What was going wrong?
The display did not show any text and images with the existing repositories.

### Knowing the problem is half of its solution
I did some BLE sniffig and figured out that this display does support monospaced 8x16, 16x16 and 32x16 font matrixes only. Therefore sending text chars ist different from the existing implementations.

## Part Four: External Component Implementation
### What Has Been Done?
- I had to figure out a way to get color values and text and image data accesible in the component. 
- Selecting a set of useful parameters, sensors and buttons required to control the iPixel device. 
- Ensure extensibility: The component is designed to support other versions of the display too.

### Key Insights
One major discovery is that no single protocol exists for communicating with all BLE-enabled controllers. Most existing documentation and repositories focus on one. But at least other do exist the iPixel Color app is aware of only by evaluating firmware versions and display size. It requires people who own these devices, especially the none working, to do some debugging.

### Framework Selection
The BLE stack occupies a significant amount of flash memory. Combined with several sensors, buttons, and other components, this quickly exceeded the 4MB flash capacity (2MB App, 2MB OTA) of my ESP32-C3 Super Lite. I found that the ESP-IDF framework produces a smaller binary compared to the Arduino framework. In future ESPHome will support the ESP-IDF only. Therefore it is strongly regcommended to use it right now.

## Part Five: Future Work / ToDo

- **Known issues:**  
  - tested with display size 32x32 and 16x96 only. The size is captued from the device. If the size recognition fails the width and height parameters can be set in ipixel_ble.yaml display section.
  - switching off and on the light does set the effect to None. I found no elegant way to propagate the last running effect to the light component. (I pepared it according the dev branch of ESPHome but the functions are not available in version 2025.2.0)
  - 16x16 and 16x32 font bitmaps are generated by stupid doubling bits and rows with no smoothing. It was just an easy way to get it to work.
  - The yaml file has some internal components. They are pepared, but not used and implemented right now. Deleting these may cause compilation erros!
  - With the 16x96 display the first 2 pixels of a frame load are garbage.
  
- **Sending PNG and animated GIFs**  
  Unfortunately the text (data) component is limited to 255 bytes. Using the max_length parameter in text.py configuration file did not apply. I need to figure out how to get a big data into the ESPHome device. A http request using an URL pointing to an image source or reenabling the REST api may be a suggestion.

- **internal PNG and GIF to RGB converter**  
  Unsing the OTA partition the ESP32 C3 Lite the compiled code is already eating about 84% of its flash size. Reaching 90% causes instability for unknown reasons. Because I do not want to skip the OTA functionality, I will have to upgrade to a ESP32 S3 with 8MB flash memory for further deveolpment.

- **reverse engeneering of the play program feature**  
  A nice feature to queue image and text effects.














