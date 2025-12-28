#include "iPixelCommands.h"
#include <algorithm>

//#undef ESPHOME_LOG_LEVEL
//#define ESPHOME_LOG_LEVEL ESPHOME_LOG_LEVEL_DEBUG  // lower than global yanl setting gets not applied:(
#include "esphome/core/log.h"

namespace iPixelCommads {

    const char *TAG = "iPixelCommads";

    bool checkRange(const char* name, int value, int minVal, int maxVal) {
        if (value < minVal || value > maxVal) {
			std::string err = std::string(name) + " out of range (" + std::to_string(minVal) + "-" + std::to_string(maxVal) + ") got " + std::to_string(value);
            ESP_LOGI(TAG, "%s\n", err.c_str());
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

    std::vector<uint8_t> getFirmwareVersions() {
        std::vector<uint8_t> result {0x04, 0x00, 0x05, 0x80};
        return result;
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
        
    std::vector<uint8_t> setRotation(int rotation) {
        checkRange("Rotation", rotation, 0, 3);

        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x06;
        frame[3] = 0x80;
        frame[4] = (uint8_t)rotation;

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

    std::vector<uint8_t> setSpeed(int speed) {      // ATTENTION: malformed command
        checkRange("Speed", speed, 0, 100);

        std::vector<uint8_t> frame(4);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x03;
        frame[3] = 0x80;
        frame[4] = (uint8_t)speed;

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

    std::vector<uint8_t> showPixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
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

    std::vector<uint8_t> showClock(int style, int dayOfWeek, int year, int month, int day, bool showDate, bool format24) {
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

    std::vector<uint8_t> showRhythmLevels(int style, const int levels[11]) {
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

    std::vector<uint8_t> showRhythmAnimation(int style, int frameNumber) {
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

    std::vector<uint8_t> showText(const std::string &text, uint8_t animation, uint8_t speed, esphome::Color txt_color,  uint8_t rainbow_mode, uint8_t font_flag, uint8_t save_slot, esphome::Color bg_color) {
        checkRange("Text Length", text.length(), 1, 500);
        checkRange("Animation", animation, 0, 6);
        checkRange("Save Slot", save_slot, 0, 255);
        checkRange("Speed", speed, 0, 100);
        checkRange("rainbow_mode", rainbow_mode, 0, 9);
        checkRange("font_flag", font_flag, 0, 4);

        size_t length = text.length();
        std::vector<uint8_t> chars_bytes;
        Helpers::encodeText(text, font_flag, txt_color.r, txt_color.g, txt_color.b, length, chars_bytes);

        ESP_LOGI(TAG, "sendText %s length=%d", text.c_str(), length);

        // --- Validation ---
        if (length == 0 || length > 500 || font_flag > 4) return {};

        uint32_t size = 0x0e + chars_bytes.size();                                      // payload size payload header + encoded_byte_count

        std::vector<uint8_t> size_command = Helpers::getLittleEndian(size + 15, 2);     // 2 bytes little-endian (header + payload)
        std::vector<uint8_t> size_payload = Helpers::getLittleEndian(size, 4);          // 4 bytes little-endian
    
        std::vector<uint8_t> result;

        result.insert(result.end(), size_command.begin(), size_command.end());          // byte 1-2 command size little endian
        result.push_back(0x00);                                                         // byte 3
        result.push_back(0x01);                                                         // byte 4
        result.push_back(0x00);                                                         // byte 5 (0x02 if it has a next frame)
        result.insert(result.end(), size_payload.begin(), size_payload.end());          // byte 6-9 payload size little endian

        // --- build Payload ---
        std::vector<uint8_t> char_count = Helpers::getLittleEndian(length, 2);

        std::vector<uint8_t> payload;
        payload.insert(payload.end(), char_count.begin(), char_count.end());            // number of characters little endian
        payload.push_back(0x01); payload.push_back(0x01);                               // use horizontal and vertical align

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
        std::vector<uint8_t> crc_bytes = Helpers::calculateCRC32Bytes(payload);         // payload checksum 4 bytes little-endian

        // --- Assemble final message ---
        result.insert(result.end(), crc_bytes.begin(), crc_bytes.end());                // byte 10-13   
        result.push_back(0x00);                                                         // byte 14
        result.push_back(save_slot);                                                    // byte 15
        result.insert(result.end(), payload.begin(), payload.end());

        return result;
    }

    std::vector<uint8_t> showImage(const std::vector<uint8_t> &data, uint8_t save_slot, uint8_t chunk_index, bool is_gif,
                                   size_t total_size, std::vector<uint8_t> total_crc) {
        size_t size = data.size();
        size_t max_size = 12*1024L;
        std::vector<uint8_t> result;
        checkRange("Save Slot", save_slot, 0, 255);
        checkRange("Data Size", size , 1, max_size);

        if (size == 0 || size > max_size) return result;

        if (total_size == 0L) total_size = size;

        std::vector<uint8_t> size_cmd = Helpers::getLittleEndian(size + 15, 2);     // 4 bytes little-endian (header + payload)
        std::vector<uint8_t> size_payl = Helpers::getLittleEndian(total_size, 4);   // 4 bytes little-endian
        std::vector<uint8_t> crc_bytes = total_crc;                                 // 4 bytes little-endian
        if (crc_bytes.size() == 0) {
            crc_bytes = Helpers::calculateCRC32Bytes(data);
        }
        uint8_t option = chunk_index == 0 ? 0x00 : 0x02;
        uint8_t header_end;

        result.insert(result.end(), size_cmd.begin(), size_cmd.end());          // entite command size

        if (is_gif) {
            result.insert(result.end(), { 0x03, 0x00, option });                // animated prefix
            header_end = 0x02;
        } else {          
            result.insert(result.end(), { 0x02, 0x00, option });                // raw prefix
            header_end = 0x00;
        }

        result.insert(result.end(), size_payl.begin(), size_payl.end());        // payload size (4 bytes)
        result.insert(result.end(), crc_bytes.begin(), crc_bytes.end());        // payloaf checksum (4 bytes)
        result.insert(result.end(), { header_end, save_slot });                 // header end
        // payload
        result.insert(result.end(), data.begin(), data.begin() + size);         // insert data chunk

        return result;
    }

    std::vector<uint8_t> setProgramList(const std::vector<uint8_t> &slot_list) {
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

    std::vector<uint8_t> delProgramList(const std::vector<uint8_t> &slot_list) {
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

        std::vector<uint8_t> slot_list {slot};

        return delProgramList(slot_list);
    }

} // namespace iPixelCommads
