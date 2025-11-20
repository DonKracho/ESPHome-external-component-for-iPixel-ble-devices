#include "iPixelCommands.h"
#include <esp_log.h>

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
        frame[7] = 0x00;

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

    std::vector<uint8_t> ledOff() {
        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x07;
        frame[3] = 0x01;
        frame[4] = 0x00;

        return frame;
    }

    std::vector<uint8_t> ledOn() {
        std::vector<uint8_t> frame(5);
        frame[0] = 0x05;
        frame[1] = 0x00;
        frame[2] = 0x07;
        frame[3] = 0x01;
        frame[4] = 0x01;

        return frame;
    }

    std::vector<uint8_t> deleteScreen(int screen) {
        checkRange("Screen", screen, 0, 10);

        std::vector<uint8_t> frame(6);
        frame[0] = 0x07;
        frame[1] = 0x00;
        frame[2] = 0x02;
        frame[3] = 0x01;
        frame[4] = 0x00;
        frame[5] = (uint8_t)screen;

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
        checkRange("Style", style, 1, 8);
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

    std::vector<uint8_t> sendText(const std::string& text, int animation, int save_slot, int speed, uint8_t r, uint8_t g, uint8_t b, int rainbow_mode, int font_flag) {
        checkRange("Text Length", text.length(), 1, 128);
        checkRange("Animation", animation, 0, 6);
        checkRange("Save Slot", save_slot, 1, 10);
        checkRange("Speed", speed, 0, 100);
        checkRange("colorR", r, 0, 255);
        checkRange("colorG", g, 0, 255);
        checkRange("colorB", b, 0, 255);
        checkRange("rainbow_mode", rainbow_mode, 0, 9);
        checkRange("font_flag", font_flag, 0, 4);

        uint length = text.length();
        std::vector<uint8_t> chars_bytes;
        Helpers::encodeText(text, font_flag, r, g, b, length, chars_bytes);

        ESP_LOGD(TAG, "sendText %s length=%d", text.c_str(), length);

        // --- Validation ---
        if (length == 0 || length > 128 || font_flag > 4) return {};

        // --- Header calculation ---
        const uint16_t HEADER_1_MG = 0x1D;
        const uint16_t HEADER_3_MG = 0x0E;
        // font_flag 0: 8x16 1: 16x16 2: 16x32 3: 8x16 -> 0x80 
        const uint16_t CHAR_HEADER = (font_flag >= 3) ? 6 : 4;
        const uint16_t CHAR_BYTES  = (font_flag < 1) ? 16 : (font_flag < 2) ? 32 : (font_flag < 3) ? 64 : (font_flag < 4) ? 16 : 32;
        
        uint16_t header_gap = CHAR_HEADER + CHAR_BYTES;

        uint16_t header_1_val = HEADER_1_MG + length * header_gap;
        uint16_t header_3_val = HEADER_3_MG + length * header_gap;

        std::vector<uint8_t> header;

        auto appendHeader16 = [&header](uint16_t val) {
            std::vector<uint8_t> temp = { 
                (uint8_t)((val >> 8) & 0xFF),
                (uint8_t)(val & 0xFF)
            };
            auto switched = Helpers::switchEndian(temp);
            header.insert(header.end(), switched.begin(), switched.end());
        };

        appendHeader16(header_1_val); //Byte 1-2
        header.push_back(0x00); //Byte 3
        header.push_back(0x01); //Byte 4
        header.push_back(0x00); //Byte 5
        appendHeader16(header_3_val); //Byte 6-7
        header.push_back(0x00); //Byte 8
        header.push_back(0x00); //Byte 9

        // --- Save slot ---
        uint16_t save_slot_val = (uint16_t)(save_slot);
        std::vector<uint8_t> save_slot_bytes = {
            (uint8_t)(save_slot_val & 0xFF),
            (uint8_t)((save_slot_val >> 8) & 0xFF)
        };
        save_slot_bytes = save_slot_bytes; //Byte 14-15

        // --- Payload ---
        std::vector<uint8_t> payload;
        payload.push_back((uint8_t)(length));                                       // number of characters
        payload.push_back(0x00); payload.push_back(0x01); payload.push_back(0x01);  // fixed prefix

        payload.push_back((uint8_t)(animation));
        payload.push_back((uint8_t)(speed));
        payload.push_back((uint8_t)(rainbow_mode));

        // Append "ffffff00000000" as bytes
        payload.push_back(0xFF); payload.push_back(0xFF); payload.push_back(0xFF);
        payload.push_back(0x00); payload.push_back(0x00); payload.push_back(0x00);
        payload.push_back(0x00);

        // Append encoded characters
        payload.insert(payload.end(), chars_bytes.begin(), chars_bytes.end());

        // --- CRC ---
        std::vector<uint8_t> crc_bytes = Helpers::calculateCRC32Bytes(payload); //Byte 10-13

        // --- Assemble final message ---
        std::vector<uint8_t> result;
        result.insert(result.end(), header.begin(), header.end());
        result.insert(result.end(), crc_bytes.begin(), crc_bytes.end());
        result.insert(result.end(), save_slot_bytes.begin(), save_slot_bytes.end());
        result.insert(result.end(), payload.begin(), payload.end());

        return result;
    }

    std::vector<uint8_t> sendPNG(const std::vector<uint8_t> &pngData) {
        std::vector<uint8_t> checksum = Helpers::calculateCRC32Bytes(pngData);
        std::vector<uint8_t> pngSize = Helpers::getFrameSize(pngData, 4);

        std::vector<uint8_t> result;
        result.insert(result.end(), { 0xFF, 0xFF }); //Placeholder for Frame Size
        result.insert(result.end(), { 0x02, 0x00, 0x00 }); //Prefix
        result.insert(result.end(), pngSize.begin(), pngSize.end()); //PNG Size
        result.insert(result.end(), checksum.begin(), checksum.end()); //Checksum
        result.insert(result.end(), { 0x00, 0x65 }); //Mid
        result.insert(result.end(), pngData.begin(), pngData.end()); //Data

        //Replace Placeholder for Frame Size
        std::vector<uint8_t> frameSize = Helpers::getFrameSize(result, 2);
        result[0] = frameSize[0];
        result[1] = frameSize[1];
        
        return result;
    }

    std::vector<uint8_t> sendGIF(const std::vector<uint8_t> &gifData) {
        std::vector<uint8_t> checksum = Helpers::calculateCRC32Bytes(gifData);
        std::vector<uint8_t> pngSize = Helpers::getFrameSize(gifData, 4);

        std::vector<uint8_t> result;
        result.insert(result.end(), { 0xFF, 0xFF }); //Placeholder for Frame Size
        result.insert(result.end(), { 0x03, 0x00, 0x00 }); //Prefix
        result.insert(result.end(), pngSize.begin(), pngSize.end()); //PNG Size
        result.insert(result.end(), checksum.begin(), checksum.end()); //Checksum
        result.insert(result.end(), { 0x02, 0x01 }); //Mid
        result.insert(result.end(), gifData.begin(), gifData.end()); //Data

        //Replace Placeholder for Frame Size
        std::vector<uint8_t> frameSize = Helpers::getFrameSize(result, 2);
        result[0] = frameSize[0];
        result[1] = frameSize[1];
        
        return result;
    }
}
