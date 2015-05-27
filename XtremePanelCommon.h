#ifndef XTREMEPANELCOMMON_H_
#define XTREMEPANELCOMMON_H_

const char *FIRMWARE_VERSION = (const char*)F("XP 1 1");    // XtremePanel version 1.1


//=============================================================================
// Hardare Pins Used
//=============================================================================

// IR Remote
#define IR_RECV_PIN 17

// SD card
#define SD_CARD_CS 15


//=============================================================================
// Display
//=============================================================================

enum eDisplayState
{
  DisplayState_None = 0,
  DisplayState_Settings,
  DisplayState_DateTime,
  DisplayState_Alarm,
  DisplayState_Timer,
  DisplayState_Messages,
  DisplayState_Manual,    // display controlled via serial draw commands

  DisplayState__End
};

extern SmartMatrix  matrix;


//=============================================================================
// IR Remote
//=============================================================================

#define IR_RECV_POLLRATE  200    // milliseconds

enum class InputCommand
{
    None = 0,
    Same, // same code as before
    BrightDec,
    BrightInc,
    PlayPause,
    Setup,
    StopMode,
    Up,
    Down,
    Left,
    Right,
    Select,
    ZeroTen,
    Back,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    
    _Count
//    Brightness,
//    PlayMode,
//    Palette,
//    Clock,
//    Power,
//    Back,
//    BrightnessUp,
//    BrightnessDown,
//    Menu,
};


//=============================================================================
// Global Configuration Settings
//=============================================================================

class GlobalSettings
{
public:
  InputCommand  command;
  uint32_t      commandTicks;
  bool          commandChanged;


  GlobalSettings()
  {
    command        = InputCommand::None;
    commandTicks   = 0;
    commandChanged = false;
  }
  ~GlobalSettings()
  {
  }
  
  uint32_t getCommandTime()
  {
    return commandTicks * IR_RECV_POLLRATE;
  }

};

GlobalSettings gm_Settings;


//=============================================================================
// Remote Procedure Call (RPC)
//=============================================================================

#pragma pack(1)
struct tRPCPacked
{
  uint8_t    cmd;      // command
  uint8_t   size;      // payload size, else 0xFF/255
};


#define RPCXFER_MASK_SLOT    0x0F
#define RPCXFER_MASK_FLAGS   0xF0
#define RPCXFER_DIR          0x10
#define RPCXFER_APPEND       0x20

struct tRPCXfer
{
  uint8_t   index;     // [bits 7 - 6]: reserved, [bit 5]: append flag, [bit 4]: direction(1=Recv,0=Send), [bits 3 - 0]: buffer slot
  uint8_t   size;      // size in bytes of payload for this command
};

struct tSerialConfig
{
  uint8_t   index;      // [bit 7]: enable bit, [bits 6 - 2]: reserved, [bits 1 & 0]: serial port identifier
  uint32_t  baud;       // baud rate
  uint32_t  format;     // Arduino/Teensy 3.1 defined configuration format
};

struct tSerialXfer
{
  uint8_t   index;      // [bits 7 - 2]: data length, [bits 1 & 0]: serial port identifier
  // rest data..
};

struct tRemoteEvent
{
  uint8_t   command;        // new and current command
  uint8_t   commandOld;     // previous command
  uint32_t  commandTicks;   // previous command lifetime (in units of 200ms)
};

#pragma pack()


enum SCmdScroll
{
  ScrollState_Stop = 0,
  ScrollState_Start,
  ScrollState_Pause,
  ScrollState_Append
};

// Drawing -> GIFAnimation
#define RPCGIF_FLAG_LOAD      0x80    // Load GIF from filesystem flag
#define RPCGIF_FLAG_POS       0x40    // Set GIFPlayer position flag
#define RPCGIF_FLAG_INTERVAL  0x20    // Set GIFPlayer animation interval flag
#define RPCGIF_MASK_FLAGS     0xF0    // Bitmask for flags
#define RPCGIF_MASK_STATE     0x03    // Bitmask for GIFPlayer state

enum class rpcGIFState
{
  Stop = 0,                       // No GIF image displayed
  Play,                           // Animate GIF
  Pause,                          // Freeze frame of GIF animation
};


//=============================================================================
// Utility macros
//=============================================================================
#define IOBUFFERS_COUNT    8  // unified I/O buffer count
#define IOBUFFERS_SSIZE   64  // each small buffer size in bytes
#define IOBUFFERS_SCOUNT   4  // number of small buffers
#define IOBUFFERS_SSTART   0
#define IOBUFFERS_LSIZE  256  // each large buffer size in bytes
#define IOBUFFERS_LCOUNT   4  // number of large buffers
#define IOBUFFERS_LSTART   IOBUFFERS_SCOUNT

uint8_t gm_IOSmallBuffers[IOBUFFERS_SCOUNT][IOBUFFERS_SSIZE];
uint8_t gm_IOLargeBuffers[IOBUFFERS_LCOUNT][IOBUFFERS_LSIZE];

struct tIOBuffer
{
  const uint16_t  size;   // buffer capacity in bytes
  uint16_t        used;   // used capacity in bytes
  uint8_t * const data;   // pointer to storage buffer
} gm_IOBuffers[IOBUFFERS_COUNT] =
{
  // small buffers
  { IOBUFFERS_SSIZE, 0, &gm_IOSmallBuffers[0][0] },
  { IOBUFFERS_SSIZE, 0, &gm_IOSmallBuffers[1][0] },
  { IOBUFFERS_SSIZE, 0, &gm_IOSmallBuffers[2][0] },
  { IOBUFFERS_SSIZE, 0, &gm_IOSmallBuffers[3][0] },

  // large buffers
  { IOBUFFERS_LSIZE, 0, &gm_IOLargeBuffers[0][0] },
  { IOBUFFERS_LSIZE, 0, &gm_IOLargeBuffers[1][0] },
  { IOBUFFERS_LSIZE, 0, &gm_IOLargeBuffers[2][0] },
  { IOBUFFERS_LSIZE, 0, &gm_IOLargeBuffers[3][0] },
};


#define GM_TEXT_SIZE  512
char gm_text[GM_TEXT_SIZE];


#define PTRLEN(_s, _e)      ((size_t)(_e) - (size_t)(_s))
#define ARRAYSIZE(_a)       (sizeof(_a) / sizeof(_a[0]))

#define MakeUInt16(_l, _h)  (((uint16_t)_l) | (((uint16_t)_h) <<  8))
#define MakeUInt32(_l, _h)  (((uint32_t)_l) | (((uint32_t)_h) << 16))
#define MakeUInt64(_l, _h)  (((uint64_t)_l) | (((uint64_t)_h) << 32LL))

#define UInt8PToUInt16(_p)   MakeUInt16(_p[0], _p[1])
#define UInt8PToUInt32(_p)   MakeUInt32(MakeUInt16(_p[0], _p[1]), MakeUInt16(_p[2], _p[3]))
#define UInt8PToUInt64(_p)   MakeUInt64(MakeUInt32(MakeUInt16(_p[0], _p[1]), MakeUInt16(_p[2], _p[3])), \
                                        MakeUInt32(MakeUInt16(_p[4], _p[5]), MakeUInt16(_p[6], _p[7])))

//#define PACKED __attribute__((packed))
#define PACKED __attribute__ ((aligned(1), packed))


//=============================================================================
// Utility functions
//=============================================================================


HardwareSerial& getSerialPort(size_t index)
{
  switch(index)
  {
    default:
    case 0:  return Serial1;
    case 1:  return Serial2;
    case 2:  return Serial3;
  }
}


#endif // XTREMEPANELCOMMON_H_

