#pragma once

#include "Helpers.h"

/**
 * Provides functions to generate BLE commands for the
 * iPixel Color Displays
 */
namespace iPixelCommads {

    bool checkRange(const char* name, int value, int minVal, int maxVal);

    std::vector<uint8_t> setTime(int hour, int minute, int second);
    std::vector<uint8_t> setFunMode(bool value);
    std::vector<uint8_t> setOrientation(int orientation);
    std::vector<uint8_t> clear();
    std::vector<uint8_t> setBrightness(int brightness);
    std::vector<uint8_t> setSpeed(int speed);
    std::vector<uint8_t> ledOff();
    std::vector<uint8_t> ledOn();
    std::vector<uint8_t> deleteScreen(int screen);
    std::vector<uint8_t> setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    std::vector<uint8_t> setClockMode(int style, int dayOfWeek, int year, int month, int day, bool showDate, bool format24);
    std::vector<uint8_t> setRhythmLevelMode(int style, const int levels[11]);
    std::vector<uint8_t> setRhythmAnimationMode(int style, int frameNumber);
    std::vector<uint8_t> sendText(const std::string &text, int animation, int save_slot, int speed, uint8_t colorR, uint8_t colorG, uint8_t colorB, int text_mode, int font_flag);
    std::vector<uint8_t> sendPNG(const std::vector<uint8_t> &data);
    std::vector<uint8_t> sendGIF(const std::vector<uint8_t> &data);
}
