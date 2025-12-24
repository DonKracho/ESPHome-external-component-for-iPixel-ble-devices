## iPixel BLE command stucture  

All commads send to the display start with the command package length encoded as 2 byte little endian followed by a fixed two byte command identifier. If the command payload size exceeds 12KB it is split up into subsequent commads with up to 12 KB payload chunks. This may happen for large gif animations and text only. All commands, exept the commands for setting pixel and rhythm, return an acknowledge notification.

### Set Time (and getting display size)
This is the first command send to the divice by the iPixel Color App after the BLE connections has been established.\
**Return:** a 11 byte acknowledge notification with display model encoded in byte 5. 
 
```
struct set_time {  
  uint16_t cmd_len {0x08, 0x000};       // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x01, 0x80};      // byte 3-4 command identifier  
  uint8_t  hour;                        // byte 5 hour (0 to 23)  
  uint8_t  minute;                      // byte 6 minutes (0 to 59)  
  uint8_t  second;                      // byte 7 minutes (0 to 59)  
  uint8_t  language;                    // byte 8 language set in the app (0 = english)  
};  
```
Setting the time replies a notification of 11 bytes length (e.g. 0B.00.01.80.**84**.03.30.00.00.01.00)\
The fifth byte is a encoded display size (e.g. 84 stands for 16x96). The other bytes look toke the time and language set with the commad.

### Get Firmware Versions  
After the iPixel App connected to the device it sends the current time and then this firmware version getter two times.\
**Return:** a 12 byte acknowledge notification with firmware versions in byte 5 to 12. 

```
struct get_firmware_versions {  
  uint16_t cmd_len {0x0400};            // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x05, 0x80};      // byte 3-4 command identifier  
};
```

Returned notification:

```
struct  versions {  
  uint16_t notify_len {0x0c00};         // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x05, 0x80};      // byte 3-4 command identifier  
  uint8_t  mcu_major                    // byte 5-6
  uint8_t  mcu_minor;                   // byte 7-8
  uint8_t  ble_major;                   // byte 9-10
  uint8_t  ble_minor;                   // byte 11-12
};
```

### Show Digital Clock  
displays a digital clock animation.\
**Return:** a 5 byte acknowledge notification with state 1 
 
```
struct show_digital_clock {  
  uint16_t cmd_len {0x0b, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x06, 0x01};      // byte 3-4 command identifier  
  uint8_t  styte;                       // byte 5 value: 0-9 (depends on display)
  uint8_t  format24;                    // byte 6 12h: 0 24h:1
  uint8_t  show_date;                   // byte 7 0:off 1:alternating time/date
  uint8_t  year;                        // byte 8 value: year - 2000
  uint8_t  month;                       // byte 9 value: 1-12  
  uint8_t  day;                         // byte 10 value: 1-31
  uint8_t  day_of_week;                 // byte 11 value: 1-7 where 7 is sunday! 
};
```

### Set Program List  
The iPixel Color App has a "Program List" feature. There you can checkmark within a list of effects stored on the mobile already. On pressing the Play button all checked effects are send to the display and shown in a endless sequence loop.\

When pressing the play button in the "Program List" feature tab the follwing command is send to the dispaly. As always the command starts with the command length byte count encoded as two byte little endian.A fixed two byte command identifier follows. Then the length of the slot list as two byte little endian. A list of slot numbers involved follows. The nunbers may have a value range from 1 to 100, at least for my devices.\
Slot nunbers 0 and numbers above 100 are reserved. For these numbers the uploaded effects are applied immediately and the progam list halts.\
**Return:** a 5 byte acknowledge notification with state 1 

```
struct set_program_list {  
  uint16_t cmd_len;                     // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x08, 0x80};      // byte 3-4 command identifier  
  uint16_t slot_count;                  // byte 5-6 number of slots little endian  
  uint8_t  slot_list[slot_count];       // byte 7-end List of slot numbers  
};  
```

After that for each slot number in the list the according effect (load image or sending text) has to be send. As long as not all effects have been set the display cycle trough all slots having valid content. While the program list is loading do not send commands which are not taking a slot number. Be carefull: Sending invalid content to a slot may crash the display and cause boot loops!

### Delete Program List (aka Delete Slot/Screen)  
To remove slots from the program list the delete_list_command ist used. This takes a list of numbers too. Be carefull deleting slots while the program list upload is not finished. This may crash the display firmware.\
**Return:** a 5 byte acknowledge notification with state 1 
 
```
struct delete_list {  
  uint16_t cmd_len;                     // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x02, 0x01};      // byte 3-4 command identifier  
  uint16_t slot_count;                  // byte 5-6 number of slots little endian  
  uint8_t  slot_list[slot_count];       // byte 7-end List of slot numbers  
};
```

### Show Image  
The iPixel App can send raw frames, png or gif files. If the image size exceeds 12KB it has to be send in chunks. Then the entire payload size and payload_crc has to be gvien with the first frame already! Not being able to calulate the checksum during sending the chunks makes it difficult to implement multi package transfers on devices with limited resoures.\ 
**Return:** a 5 byte acknowledge notification with state 1 until the last chunk is send. 3 on success. 0 if the crc check fails.

```
struct  show_image {
  uint16_t cmd_len;                     // byte 1-2 entire packet length little endian
  union cmd_id {
    uint8_t  raw[2]{ 0x02, 0x00 };      // byte 3-4
    uint8_t  gif[2]{ 0x03, 0x00 };      // byte 3-4
  }
  uint8_t  has_next_chunk {0x00}        // byte 5 set to 0x02, if there is a data frame following
  uint32_t payload_size;                // byte 6-9
  uint32_t payload_crc;                 // byte 10-13 checksum of payload little endian;
  union {                               // byte 14
    uint8_t  raw[1] { 0x00 };
    uint8_t  gif[1] { 0x02 };
  }
  uint8_t  save_slot;                   // byte 15 used by program feature for raw 0x65 (first frame after the program frames)
                                        // here the payload begins
  uint8_t  data[]                       // up to 12KB of data
};
```

### Show Text  
The text command allows up to 500 characters in the iPixel Color App. It encodes the characters to font bitmaps or jpg images. The payload holds attributes for alignment, animation and colors.\
**Return:** a 5 byte acknowledge notification with state 1 until the last chunk is send. 3 on success. 0 if the crc check fails. 

```
struct show_text {
  uint16_t cmd_len;                     // byte 1-2 entire packet length little endian
  uint8_t  cmd_id[2] {0x00, 0x01};      // byte 3-4 command identifier
  uint8_t  has_next_chunk {0x00};       // byte 5 set to 0x02, if there is a data frame following (@ArtiiP)
  uint32_t payload_size;                // byte 6-9
  uint32_t payload_crc;                 // byte 10-13 checksum of payload little endian
  uint8_t  unknown {0x00};              // byte 14
  uint8_t  save_slot;                   // byte 15 used by program feature
                                        // here the payload begins
  uint16_t char_count;                  // byte 16-17 little endian
  uint8_t  h_align {0x01};              // byte 18 set to 1 for horizontal alignment (@ArtiiP)
  uint8_t  v_align {0x01};              // byte 19 set to 1 for vertical alignment (@ArtiiP)
  uint8_t  animation_mode               // byte 20
  uint8_t  animation_speed;             // byte 21
  uint8_t  text_color_mode;             // byte 22 0: char bitmap color, 1: fixed color, div. rainbow effects
  uint8_t  text_color[3];               // byte 23-25 character color {r, g, b} applied when text_color_mode is 1
  uint8_t  back_color_mode;             // byte 26 0: off (black), 1: the back_color below gets applied
  uint8_t  back_color[3];               // byte 27-29 background color {r, g, b} applied when back_color_mode is 1
  union {
    struct 8_16_bitmap {                // fixed 8x16 font bitmap
      uint8_t flag {0x00};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[(8/8)*16];
    };
    struct 16_16_bitmap {               // fixed 16x16 font bitmap
      uint8_t flag {0x01};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[(16/8)*16];
    };
    struct 16_32_bitmap {               // fixed 16x32 font bitmap
      uint8_t flag {0x02};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[(16/8)*32];
    };
    struct 32_32_bitmap {               // fixed 32x32 font bitmap
      uint8_t flag {0x03};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[(32/8)*32];
    };
    struct 48_24_bitmap {               // fixed 48x24 font bitmap
      uint8_t flag {0x04};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[(48/8)*24];
    };
    struct 48_48_bitmap {               // fixed 48x48 font bitmap
      uint8_t flag {0x05};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[(48/8)*48];
    };
    struct 64_32_bitmap {               // fixed 64x32 font bitmap
      uint8_t flag {0x06};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[(64/8)*32];
    };
    struct 64_64_bitmap {               // fixed 64x64 font bitmap
      uint8_t flag {0x07};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[(64/8)*64];
    };
    struct w_h_bitmap {                 // variable font bitmap
      uint8_t flag {0x08};
      uint8_t color[3];                 // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t used_bits_per_row;        // w (written row data must be a multiple of bytes! (w+7)/8)
      uint8_t rows;                     // h
      uint8_t bitmap[((w+7)/8)*h];
    };
    struct jpeg {                       // jpeg text image (not verified, there is a flag clash!) 
      uint8_t flag;                     // width: 16:0x08, 32:0x09, 20:0x0C, 24:0x0B, 64:0x0A
      uint8_t data_size[3]              // little endian encoded (will tis work for 3 bytes?)
      uint8_t data_jpeg[]
    }
  } char_bitmap_data[char_count];
};
```

### Show Rhythm Animation  
shows a rhythm animation.\
**NOTE:** This command does not send an acknowledge notification.
 
```
struct set_rhythm_animation {  
  uint16_t cmd_len {0x06, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x00, 0x02};      // byte 3-4 command identifier  
  uint8_t  animation_number;            // value: 0-7
  uint8_t  style;                       // value: 0-1  
};
```

### Show Rhythm Levels  
shows some kind of bargraph animation.\
**NOTE:** This command does not send an acknowledge notification.
 
```
struct show_rhythm_level {  
  uint16_t cmd_len {0x10, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x01, 0x02};      // byte 3-4 command identifier  
  uint8_t style;                        // byte 5 value: 0-4 
  uint8_t levels[11]                    // byte 6-16 value: 0-15 for 11 bars 
};
```

### Show Pixel  
draws a single pixel on the display with given color. the origin is the upper left corner. Used for DIY drawing mode.\
**NOTE:** This command does not send an acknowledge notification.

```
struct set_pixel {  
  uint16_t cmd_len {0x0a, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x05, 0x01};      // byte 3-4 command identifier  
  uint8_t  fixed {0x00};				// byte 5  (maybe it allows 0x02 like image or text)
  uint8_t  red;                         // byte 6  value: 0-255
  uint8_t  green;                       // byte 7  value: 0-255
  uint8_t  blue;                        // byte 8  value: 0-255
  uint8_t  x_pos;                       // byte 9  value: 0-display width
  uint8_t  y_pos;                       // byte 10 value: 0-display_height
};
```

### Cear  
clears all slots in the eeprom storage. The initial LED image gets displayed.\
**Return:** a 5 byte acknowledge notify with state 1

```
struct clear {  
  uint16_t cmd_len {0x04, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x03, 0x80};      // byte 3-4 command identifier  
};
```

### Set Power  
turns the display on or off.\
**Return:** a 5 byte acknowledge notify with state 1
 
```
struct set_power {  
  uint16_t cmd_len {0x05, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x04, 0x01};      // byte 3-4 command identifier  
  uint8_t  state;                       // byte 5 0:off 1:on  
};
```

### Set Brightness  
sets the display brightnes from 0 to 100%, but where 0% is not dark.\
**Return:** a 5 byte acknowledge notify with state 1
 
```
struct set_brightness {  
  uint16_t cmd_len {0x05, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x04, 0x80};      // byte 3-4 command identifier  
  uint8_t  brightness;                  // byte 5 value: 0-100  
};
```

### Set Rotation  (not verified)  
sets the display rotation.
 
```
struct set_rotation {  
  uint16_t cmd_len {0x05, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x06, 0x80};      // byte 3-4 command identifier  
  uint8_t  rotation;                    // byte 5 0:0° 1:90° 2:180° 3:270°  
};
```

### Set Speed (not verified)
sets the speed of animations, e.g. scrolling test
 
```
struct set_speed {  
  uint16_t cmd_len {0x05, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x03, 0x80};      // byte 3-4 command identifier  
  uint8_t  speed;                       // byte 5 value. 0-100  
};
```

### Set Fun Mode (not verified)  
enables the fun mode (direct drawing) on or off.
 
```
struct set_fun_mode {  
  uint16_t cmd_len {0x05, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x04, 0x01};      // byte 3-4 command identifier  
  uint8_t  state;                       // byte 5 1:on 0:off  
};
```

### Command Template  
reserved as template.\
**Return:** a 5 byte acknowledge notify with state 

```
struct command_template {  
  uint16_t cmd_len {0x05, 0x00};        // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x00, 0x00};      // byte 3-4 command identifier  
  uint8_t  payload; 
};
```
