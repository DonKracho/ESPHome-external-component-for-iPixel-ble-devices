## iPixel BLE command stucture  

All commads send to the display start with the command length encodes as 2 byte little endian followed by a fixed command identiier.
If the command length exceeds 12KB it is split up in chunks. This may happen for large gif animations only.

### Text Command  
The text command allows up to 500 characters. It encodes the characters to font bitmaps.
It has attributes for animation and global colors. Tor 

```
struct text_command {
  uint16_t cmd_len;                      // byte 1-2 entire packet length little endian
  uint8_t  cmd_id[2] {0x00, 0x01};       // byte 3-4 command identifier
  uint8_t  has_next_chunk {0x00};        // byte 5 set to 0x02, if there is a data frame following (@ArtiiP)
  uint32_t payload_size;                 // byte 6-9 maybe a deprecated text command not knowing all attributes
  uint32_t payload_crc;                  // byte 10-13 checksum of payload little endian
  uint8_t  unknown {0x00};               // byte 14
  uint8_t  save_slot;                    // byte 15 used by program feature
                                         // here the payload begins
  uint16_t char_count;                   // byte 16-17 little endian
  uint8_t  h_align {0x01};               // byte 18 set to 1 for horizontal alignment (@ArtiiP)
  uint8_t  v_align {0x01};               // byte 19 set to 1 for vertical alignment (@ArtiiP)
  uint8_t  animation_mode                // byte 20
  uint8_t  animation_speed;              // byte 21
  uint8_t  rainbow_mode;                 // byte 22
  uint8_t  char_color[3];	             // byte 23-25 character color {r, g, b} applied when rainbow_mode is 1
  uint8_t  back_color_active;            // byte 26 0: off (black), 1: the back_color below gets applied
  uint8_t  back_color[3];	             // byte 27-29 background color {r, g, b} applied when rainbow mode is 1
  union {
    struct 8_16 {                        // fixed 8x16 font bitmap
      uint8_t flag {0x00};
      uint8_t color[3];                  // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[8*16/8];
    };
    struct 16_16 {                       // fixed 16x16 font bitmap
      uint8_t flag {0x01};
      uint8_t color[3];                  // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[16*16/8];
    };
    struct 16_32 {                       // fixed 16x32 font bitmap
      uint8_t flag {0x02};
      uint8_t color[3];                  // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t bitmap[16*32/8];
    };
    struct x_y {                         // variable font bitmap
      uint8_t flag {0x08};
      uint8_t color[3];                  // {r, g, b} individual character color applied for rainbow_mode 0 only
      uint8_t used_bits_per_row;         // x (written row data must be a multiple of bytes)
      uint8_t rows;                      // y
      uint8_t bitmap[x*y/8];
    };
  } char_bitmap_data[char_count];
};
```

### Program List Command  
The iPixel Color App has a "Program List" feature. There you can checkmark within a list of effects stored on the mobile already.
On pressing the Play button all checked effects are send to the display and shown in a endless sequence loop.

When pressing the play button the follwing command is send to the dispaly.
As always the command starts with the command length byte count encoded as two byte little endian.
A fixed two byte command identifier follows. Then the length of the slot list as two byte little endian.
A list of slot numbers involved follows. The nunbers may have a value range from 1 to 100, at least for my devices.
Slot nunbers 0 and numbers above 100 are reserved. For these numbers the uploaded effects are applied immediately and the progam list halts.

```
struct program_list_command {  
  uint16_t cmd_len;                      // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x08, 0x80};       // byte 3-4 command identifier  
  uint16_t slot_count;                   // byte 5-6 number of slots little endian  
  uint8_t  slot_list[slot_count];        // byte 7-end List of slot numbers  
};  
```

After that for each slot number in the list the according effect (load image or sending text) has to be send.
As long as not all effects have been set the display cycle trough all slots having valid content. While the program
list is loading do not send commands which are not taking a slot number. 

To remove slots from the program list the delete_list_command ist used. This takes a list of numbers too.
Be carefull deleting slots while the program list is not finished. This may crash the display firmware.
 
```
struct delete_list_command {  
  uint16_t cmd_len;                      // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x02, 0x01};       // byte 3-4 command identifier  
  uint16_t slot_count;                   // byte 5-6 number of slots little endian  
  uint8_t  slot_list[slot_count];        // byte 7-end List of slot numbers  
};
```

### Firmware Versions  
after the iPixel App connected to the device it sends a version getter two times.

```
struct notify_firmWare_versions {  
  uint16_t cmd_len {0x0400};             // byte 1-2 entire packet lenhth little endian  
  uint8_t  cmd_id[2] {0x05, 0x80};       // byte 3-4 command identifier  
};
```

It returns a notification

```
struct  versions {  
  uint16_t notify_len {0x0c00};          // byte 1-2 entire packet length little endian  
  uint8_t  cmd_id[2] {0x05, 0x80};       // byte 3-4 command identifier  
  uint8_t  mcu_major                     // byte 5-6
  uint8_t  mcu_minor;                    // byte 7-8
  uint8_t  ble_major;                    // byte 9-10
  uint8_t  ble_minor;                    // byte 11-12
};
```

### send image  
The iPixel App can send raw franes png or gif files. If the file exceed 13 KB in size it
has to be sent in chunks,

```
struct  send_image {
  uint16_t cmd_len;                      // byte 1-2 entire packet length little endian
  union cmd_id {
    uint8_t  gif[2]{ 0x03, 0x00 };       // byte 3-4
    uint8_t  raw[2]{ 0x02, 0x00 };       // byte 3-4
  }
  uint8_t  has_next_chunk {0x00}         // byte 5 set to 0x02, if there is a data frame following
  uint32_t payload_size;                 // byte 6-9
  uint32_t payload_crc;                  // byte 10-13 checksum of payload little endian;
  union {                                // byte 14
    uint8_t  gif[1] { 0x02 };
    uint8_t  raw[1] { 0x00 };
  }
  uint8_t  save_slot;                    // byte 15 used by program feature for raw 0x65 (101, the first frame after the pragamable frames)
                                         // here the payload begins
  uint8_t  data[]                        // up to 12 KB of data
};
```
