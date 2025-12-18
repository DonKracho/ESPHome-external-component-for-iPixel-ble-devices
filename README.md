## EESPHome Integration for a LED Pixel Board

<img src="docs/LEDPixelBoard.jpg" alt="LED Pixel Board" width="600">

This repository contains an ESPHome external component that enables control of a Bluetooth‐enabled iPixel Display. It is non intrusive - no modification of the display is required. It just emulates the known iPixel Color App commands. At least ESPHome version 2025.11.0 is required.

## Update 2025/17/12
code cleanup and support of Proram List feature

## Update 2025/10/12
Using an ESP32-S3-N16R8 module now online PNG, JPG and BMP files can be loaded and displayed. Just switch to te Load Image effect and copy the URL of an online image into the data text field. The images get resized to 32x32. Due to the limited resources even of an ESP32-S3 it is recommended to use small images only. This will keep loading times short and prevents the module from running out of memory or crashing in worst case.

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
- My main usage of the display will be a nerdy clock visualization provided by the display firmware itself.
- Synchronizing the time without having to use the app.
- Enabled to intergrate with HomeAssistant / ESPhome.
- Send a text message from HomeAssistant automations.
- The existing approaches did not show text and images on my specific 32x32 device.
- Cheap (using an ESP32-C3 Super Lite module about 3$).
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
  A small and efficient CRC32 checksum calculator. Right now it is restricted for arduino use only, but there are no dependencies to the arduino framework. For convenience I copied the two source code files directly into the component/ipixel-ble folder.

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
I did some BLE sniffig and figured out that this display does support monospaced 16x8, 16x16 and 32x16 font matrixes only. Therefore sending text chars is different from the existing implementations. (These also have got fixed meanwhile)

## Part Four: External Component Implementation
### What Has Been Done?
- I had to figure out a way to get color values and text and image data accesible in the component. 
- Selecting a set of useful parameters, sensors and buttons required to control the iPixel device. 
- Ensure extensibility: The component is designed to support other versions of the display too.

### Key Insights
One major discovery is that no single protocol exists for communicating with all BLE-enabled controllers. Most existing documentation and repositories focus on one. But at least other do exist the iPixel Color app is aware of only by evaluating firmware versions and display size. It requires people who own these devices, especially the none working, to do some debugging.

### Framework Selection
The BLE stack occupies a significant amount of flash memory. Combined with several sensors, buttons, and other components, this quickly exceeds the 4MB flash capacity (2MB App, 2MB OTA) of an ESP32-C3 Super Lite module. I found that the ESP-IDF framework produces a smaller binary compared to the Arduino framework. Announced for 2026 ESPHome will support the ESP-IDF only! Therefore it is strongly regcommended to use it right now. (The majpr difference is using the ESP_LOGx macros instead of Arduino-Sprintf and std::strings instead of Arduino-String)

## Part Five: Future Work / ToDo

- **Known issues:**  
  - tested with display size 32x32 and 96x16 only. The size is captued from the device. If the size recognition fails the width and height parameters can be set in ipixel_ESP-xyz.yaml display section.
  - switching off and on the light does set the effect to None. I found no elegant way to propagate the last running effect to the light component.
  - 16x16 and 16x32 font bitmaps are generated by stupid doubling bits and rows with no smoothing. It was just an easy way to get it to work.
  - With the 96x16 display the first 2 pixels of a loaded frame are garbage.
  
- **Support of animated GIFs is not implemented**  
  I need to figure out how to decode GIF Files using existing ESPHome components.

- **PNG support requires ESP32-S3 with PSRAM**  
  Unsing the OTA partition the ESP32 C3 Lite the compiled code is already eating about 84% of its flash size. Reaching 90% causes instability for unknown reasons. I do not want to skip the OTA functionality! To use online image support an ESP32-S3-N16R8 is recommended (you can get them from ali for less tha 4€/piece). Please refer to the according yaml example.
