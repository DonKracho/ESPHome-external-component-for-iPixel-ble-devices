#include "iPixelCommands.h"
#include <algorithm>

//#undef ESPHOME_LOG_LEVEL
//#define ESPHOME_LOG_LEVEL ESPHOME_LOG_LEVEL_DEBUG
#include "esphome/core/log.h"

namespace iPixelCommads {

    const char *TAG = "iPixelCommads";

    bool checkRange(const char* name, int value, int minVal, int maxVal) {
        if (value < minVal || value > maxVal) {
			std::string err = std::string(name) + " out of range (" + std::to_string(minVal) + "-" + std::to_string(maxVal) + ") got " + std::to_string(value);
            ESP_LOGE(TAG, "%s\n", err.c_str());
            return false;
        }
        return true;
    }

    std::vector<uint8_t> setTime(int hour, int minute, int second) {
        checkRange("Hour", hour, 0, 23);
        checkRange("Minute", minute, 0, 59);
        checkRange("Second", second, 0, 59);

        std::vector<uint8_t> frame(8);
        frame[0] = 0x08;
        frame[1] = 0x00;
        frame[2] = 0x01;
        frame[3] = 0x80;
        frame[4] = (uint8_t)hour;
        frame[5] = (uint8_t)minute;
        frame[6] = (uint8_t)second;
        frame[7] = 0x00; // language English

        return frame;
    }

    std::vector<uint8_t> setFunMode(bool value) {
        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x04;
        frame[3] = 0x01;
        frame[4] = value ? 0x01 : 0x00;

        return frame;
    }
        
    std::vector<uint8_t> setOrientation(int orientation) {
        checkRange("Orientation", orientation, 0, 2);

        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x06;
        frame[3] = 0x80;
        frame[4] = (uint8_t)orientation;

        return frame;
    }

    std::vector<uint8_t> clear() {
        std::vector<uint8_t> frame(4);
        frame[0] = 0x04;
        frame[1] = 0x00;
        frame[2] = 0x03;
        frame[3] = 0x80;

        return frame;
    }

    std::vector<uint8_t> setBrightness(int brightness) {
        checkRange("Brightness", brightness, 0, 100);

        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x04;
        frame[3] = 0x80;
        frame[4] = (uint8_t)brightness;

        return frame;
    }

    std::vector<uint8_t> setSpeed(int speed) {
        checkRange("Speed", speed, 0, 100);

        std::vector<uint8_t> frame(4);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x03;
        frame[3] = (uint8_t)speed;

        return frame;
    }

    std::vector<uint8_t> setPower(bool state) {
        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x07;
        frame[3] = 0x01;
        frame[4] = state ? 0x01 : 0x00;

        return frame;
    }

    std::vector<uint8_t> setPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
        checkRange("X", x, 0, 255);
        checkRange("Y", y, 0, 255);
        checkRange("R", r, 0, 255);
        checkRange("G", g, 0, 255);
        checkRange("B", b, 0, 255);

        std::vector<uint8_t> frame(10);
        frame[0] = 0x0A;
        frame[1] = 0x00;
        frame[2] = 0x05;
        frame[3] = 0x01;
        frame[4] = 0x00;
        frame[5] = r;
        frame[6] = g;
        frame[7] = b;
        frame[8] = (uint8_t)x;
        frame[9] = (uint8_t)y;

        return frame;
    }

    std::vector<uint8_t> setClockMode(int style, int dayOfWeek, int year, int month, int day, bool showDate, bool format24) {
        checkRange("Style", style, 1, 9);
        checkRange("Day of Week", dayOfWeek, 1, 7);
        checkRange("Year", year, 0, 99);
        checkRange("Month", month, 1, 12);
        checkRange("Day", day, 1, 31);

        std::vector<uint8_t> frame(11);
        frame[0] = 0x0B;
        frame[1] = 0x00;
        frame[2] = 0x06;
        frame[3] = 0x01;
        frame[4] = (uint8_t)style;
        frame[5] = format24 ? 0x01 : 0x00;
        frame[6] = showDate ? 0x01 : 0x00;
        frame[7] = (uint8_t)year;
        frame[8] = (uint8_t)month;
        frame[9] = (uint8_t)day;
        frame[10] = (uint8_t)dayOfWeek;

        return frame;
    }

    std::vector<uint8_t> setRhythmLevelMode(int style, const int levels[11]) {
        checkRange("Style", style, 0, 4);
        for (int i = 0; i < 11; i++) checkRange("Level", levels[i], 0, 15);

        std::vector<uint8_t> frame(5 + 11);
        frame[0] = 0x10;
        frame[1] = 0x00;
        frame[2] = 0x01;
        frame[3] = 0x02;
        frame[4] = (uint8_t)style;
        for (int i = 0; i < 11; i++)
            frame[5 + i] = (uint8_t)levels[i];

        return frame;
    }

    std::vector<uint8_t> setRhythmAnimationMode(int style, int frameNumber) {
        checkRange("Style", style, 0, 1);
        checkRange("Frame", frameNumber, 0, 7);

        std::vector<uint8_t> frame(6);
        frame[0] = 0x06;
        frame[1] = 0x00;
        frame[2] = 0x00;
        frame[3] = 0x02;
        frame[4] = (uint8_t)frameNumber;
        frame[5] = (uint8_t)style;

        return frame;
    }

    std::vector<uint8_t> sendText(const std::string &text, uint8_t animation, uint8_t speed, esphome::Color txt_color,  uint8_t rainbow_mode, uint8_t font_flag, uint8_t save_slot, esphome::Color bg_color) {
        checkRange("Text Length", text.length(), 1, 500);
        checkRange("Animation", animation, 0, 6);
        checkRange("Save Slot", save_slot, 0, 255);
        checkRange("Speed", speed, 0, 100);
        checkRange("rainbow_mode", rainbow_mode, 0, 9);
        checkRange("font_flag", font_flag, 0, 4);

        size_t length = text.length();
        std::vector<uint8_t> chars_bytes;
        Helpers::encodeText(text, font_flag, txt_color.r, txt_color.g, txt_color.b, length, chars_bytes);

        ESP_LOGD(TAG, "sendText %s length=%d", text.c_str(), length);

        // --- Validation ---
        if (length == 0 || length > 500 || font_flag > 4) return {};

        // --- Header calculation ---
        const uint16_t HEADER_LENGTH = 0x1D;
        // font_flag 0: 8x16 1: 16x16 2: 16x32 3: 8x16 -> 0x80 
        const uint16_t CHAR_HEADER = (font_flag >= 3) ? 6 : 4;
        const uint16_t CHAR_BYTES  = (font_flag < 1) ? 16 : (font_flag < 2) ? 32 : (font_flag < 3) ? 64 : (font_flag < 4) ? 16 : 32;
        
        uint16_t char_bytes = CHAR_HEADER + CHAR_BYTES;

        uint16_t cmd_length = HEADER_LENGTH + length * char_bytes;

        std::vector<uint8_t> header;

        auto appendHeader16 = [&header](uint16_t val) {
            std::vector<uint8_t> temp = { 
                (uint8_t)((val >> 8) & 0xFF),
                (uint8_t)(val & 0xFF)
            };
            auto switched = Helpers::switchEndian(temp);
            header.insert(header.end(), switched.begin(), switched.end());
        };

        appendHeader16(cmd_length); //Byte 1-2
        header.push_back(0x00); //Byte 3
        header.push_back(0x01); //Byte 4
        header.push_back(0x00); //Byte 5
        appendHeader16(cmd_length - 0x0f); //Byte 6-7
        header.push_back(0x00); //Byte 8
        header.push_back(0x00); //Byte 9

        // --- build Payload ---
        std::vector<uint8_t> char_count = Helpers::getLittleEndian(length, 2);

        std::vector<uint8_t> payload;
        payload.insert(payload.end(), char_count.begin(), char_count.end());    // number of characters litthe endian
        payload.push_back(0x01); payload.push_back(0x01);                       // fixed prefix

        payload.push_back((uint8_t)(animation));
        payload.push_back((uint8_t)(speed));
    
        // the following char and backgrond colors do apply when rainbow mode is set to one only
        payload.push_back((uint8_t)(rainbow_mode));
        payload.push_back(txt_color.r); payload.push_back(txt_color.r); payload.push_back(txt_color.r); // text color
        payload.push_back((uint8_t)(0x01));
        payload.push_back(bg_color.r); payload.push_back(bg_color.g); payload.push_back(bg_color.b);    // background color

        // Append encoded characters
        payload.insert(payload.end(), chars_bytes.begin(), chars_bytes.end());

        // --- CRC ---
        std::vector<uint8_t> crc_bytes = Helpers::calculateCRC32Bytes(payload); //Byte 10-13

        // --- Assemble final message ---
        std::vector<uint8_t> result;
        result.insert(result.end(), header.begin(), header.end());
        result.insert(result.end(), crc_bytes.begin(), crc_bytes.end());
        result.push_back(0x00);         // byte 14
        result.push_back(save_slot);    // byte 15
        result.insert(result.end(), payload.begin(), payload.end());

        return result;
    }

    std::vector<uint8_t> sendImage(const std::vector<uint8_t> &data, uint8_t save_slot, bool is_gif) {
        checkRange("Save Slot", save_slot, 0, 255);
    
        std::vector<uint8_t> size_bytes = Helpers::getLittleEndian(data.size(), 4); // 4 bytes little-endian
        std::vector<uint8_t> crc_bytes = Helpers::calculateCRC32Bytes(data);        // 4 bytes little-endian

        std::vector<uint8_t> result;

        const size_t data_size = data.size();
        const size_t chunk_size = 12 * 1024;
        size_t chunk_index = 0;
        size_t pos = 0;
        size_t accu_size = 0;
        size_t curr_size;

        while (pos < data.size())
        {
            size_t chunk_end = std::min(pos + chunk_size, data.size());
            ESP_LOGD(TAG, "data_size=%d chunk_end=%d", data_size, chunk_end);

            uint8_t option = chunk_index == 0 ? 0x00 : 0x02;

            result.insert(result.end(), { 0xFF, 0xFF });                        // placeholder for command chunk size

            if (is_gif) {
                result.insert(result.end(), { 0x03, 0x00, option });            // animated prefix
            } else {          
                result.insert(result.end(), { 0x02, 0x00, option });            // raw prefix
            }

            result.insert(result.end(), size_bytes.begin(), size_bytes.end());  // size (4 bytes)
            result.insert(result.end(), crc_bytes.begin(), crc_bytes.end());    // checksum (4 bytes)
            result.insert(result.end(), { 0x02, save_slot });                   // header end

            result.insert(result.end(), data.begin() + pos, data.begin() + pos + chunk_end); // insert data chunk

            curr_size = result.size() - accu_size;
            accu_size += curr_size;

            // replace placeholder for command data size
            std::vector<uint8_t> cmdChunkSize = Helpers::getLittleEndian(curr_size, 2);
            result[0] = cmdChunkSize[0];
            result[1] = cmdChunkSize[1];

            chunk_index += 1;
            pos = chunk_end;
        }

        return result;
    }

    std::vector<uint8_t> startProgramList(const std::vector<uint8_t> &slot_list) {
        std::vector<uint8_t> result;

        int16_t list_size = slot_list.size();

        if (list_size <= 100) {
            std::vector<uint8_t> cmd_length = Helpers::getLittleEndian(list_size + 6, 2);
            std::vector<uint8_t> slot_count = Helpers::getLittleEndian(list_size, 2);

            result.insert(result.end(), cmd_length.begin(), cmd_length.end());
            result.insert(result.end(), {0x08, 0x80});
            result.insert(result.end(), slot_count.begin(), slot_count.end());
            result.insert(result.end(), slot_list.begin(), slot_list.end());
        }

        return result;
    }

    std::vector<uint8_t> deleteSlotList(const std::vector<uint8_t> &slot_list) {
        std::vector<uint8_t> result;

        int16_t list_size = slot_list.size();

        if (list_size <= 100) {
            std::vector<uint8_t> cmd_length = Helpers::getLittleEndian(list_size + 6, 2);
            std::vector<uint8_t> slot_count = Helpers::getLittleEndian(list_size, 2);

            result.insert(result.end(), cmd_length.begin(), cmd_length.end());
            result.insert(result.end(), {0x02, 0x01});
            result.insert(result.end(), slot_count.begin(), slot_count.end());
            result.insert(result.end(), slot_list.begin(), slot_list.end());
        }

        return result;
    }

    std::vector<uint8_t> deleteSlot(uint8_t slot) {
        checkRange("Slot", slot, 1, 100);

        std::vector<uint8_t> frame {0x07, 0x00, 0x02, 0x01, 0x01, 0x00};
        frame.push_back(slot);

        return frame;
    }

    std::vector<uint8_t> notifyFirmwareVersions() {
        std::vector<uint8_t> result {0x04, 0x00, 0x05, 0x80};
        return result;
    }

} // namespace iPixelCommads
