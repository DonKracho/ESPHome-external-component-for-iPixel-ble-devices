#pragma once

#include "Helpers.h"
#include "esphome/core/color.h"

/**
 * Provides functions to generate BLE commands for the
 * iPixel Color Displays
 */
namespace iPixelCommads {

    bool checkRange(const char* name, int value, int minVal, int maxVal);

    std::vector<uint8_t> setTime(int hour, int minute, int second); // returns notification with display paramters
    std::vector<uint8_t> notifyFirmwareVersions();
    std::vector<uint8_t> setFunMode(bool value);
    std::vector<uint8_t> setOrientation(int orientation);
    std::vector<uint8_t> clear();
    std::vector<uint8_t> setBrightness(int brightness);
    std::vector<uint8_t> setSpeed(int speed);
    std::vector<uint8_t> setPower(bool state);
    std::vector<uint8_t> setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
    std::vector<uint8_t> setClockMode(int style, int dayOfWeek, int year, int month, int day, bool showDate, bool format24);
    std::vector<uint8_t> setRhythmLevelMode(int style, const int levels[11]);
    std::vector<uint8_t> setRhythmAnimationMode(int style, int frameNumber);
    std::vector<uint8_t> sendText(const std::string &text, uint8_t animation, uint8_t speed, esphome::Color txt_color,  uint8_t rainbow_mode, uint8_t font_flag, uint8_t save_slot = 1, esphome::Color bg_color = {0, 0, 0});
    std::vector<uint8_t> sendImage(const std::vector<uint8_t> &data, uint8_t save_slot = 1, uint8_t chunk_index = 0, bool is_gif = false, size_t total_size = 0L, std::vector<uint8_t> total_crc = std::vector<uint8_t>{});
    std::vector<uint8_t> startProgramList(const std::vector<uint8_t> &slots);
    std::vector<uint8_t> deleteSlotList(const std::vector<uint8_t> &slots);
    std::vector<uint8_t> deleteSlot(uint8_t slot);
}
