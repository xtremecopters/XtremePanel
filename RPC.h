/*
 * Remote Procedure Call module
 */
 
int usb_bulk_available(void);
int usb_bulk_recv(void *buffer, uint32_t timeout);
int usb_bulk_send(const void *buffer, uint32_t timeout);

class usb_bulk_class
{
public:
    int available(void) {return usb_bulk_available(); }
    int recv(void *buffer, uint16_t timeout) { return usb_bulk_recv(buffer, timeout); }
    int send(const void *buffer, uint16_t timeout) { return usb_bulk_send(buffer, timeout); }
};

usb_bulk_class usbraw;


// Category of the command
enum class rpcType
{
  System = 0,                    // [ 0] System related
  IO,                            // [ 1] I/O related
  Event,                         // [ 2] Event related
  Display,                       // [ 3] Display related
  Drawing,                       // [ 4] Drawing related
                                 // [ 5 - 14] Unused
  Framebuffer = 15               // [>= 15] Framebuffer related
};

// System commands
enum class rpcSystem
{
  Reset = 0,                     // Force software reset / load bootloader
  Version,                       // Query firmware version
  Ping,                          // Ping? Pong!
  Timestamp,                     // Set current date and time via unix timestamp
};

// Input/Output commands
enum class rpcIO
{
  // Data buffer transfers
  XferRecv,                      // data transfer to LED matrix panel controller (Teensy)
  XferSend,                      // data transfer to LED controller's master (Linux/Host)
  
  // Serial port(s) operations
  SerialPortConfig,              // 
  SerialPortDataSend,            // 
  SerialPortDataRecv,            // 
};

// Event commands
enum class rpcEvent
{
  Packed = 0,                    // Packed event commands
  
  IRRemote,                      // Infrared Remote event
};
static const tRPCPacked m_RPCP_Event[] =
{
  { (uint8_t)rpcEvent::IRRemote,   sizeof(tRemoteEvent) },
  {0, 0} // end of list
};

// Display commands
enum class rpcDisplay
{
  Packed = 0,                    // Packed display commands
  
  Resolution,                    // Query display resolution
  Brightness,                    // Set display brightness
  SwapBuffers,                   // Make drawn framebuffer active
  Mode,                          // Display operation mode
};
static const tRPCPacked m_RPCP_Display[] =
{
  { (uint8_t)rpcDisplay::Resolution,   2 * sizeof(uint16_t) },
  { (uint8_t)rpcDisplay::Brightness,   2 },
  { (uint8_t)rpcDisplay::SwapBuffers,  1 },
  { (uint8_t)rpcDisplay::Mode,         1},
  {0, 0} // end of list
};

enum class rpcDrawing
{
  Packed = 0,                    // Packed drawing commands
  
  // Scrollers
  ScrollSelect,                  // Select a text scroller
  ScrollDefaultSet,              // Save or restore current scroller's default settings
  ScrollEvent,                   // Scroller status callback
  ScrollText,                    // Set text that is to be scrolled
  ScrollState,                   // Scroller current state; ex. start, stop, pause, etc..
  ScrollMode,                    // Scroller mode
  ScrollSpeed,                   // Scroller speed
  ScrollFont,                    // Scroller text font
  ScrollColor,                   // Scroller text color
  ScrollOffsetHorizontal,        // Scroller horizontal offset position
  ScrollOffsetVertical,          // Scroller vertical offset position
  ScrollBoundary,                // Scroller rectangle bounds
  
  DrawPixel = 20,
  DrawLine,
  DrawFastVLine,
  DrawFastHLine,
  DrawCircle,
  FillCircle,
  DrawEllipse,
  DrawTriangle,
  FillTriangle,
  DrawRectangle,
  FillRectangle,
  DrawRoundRectangle,
  FillRoundRectangle,
  FillScreen,
  SetFont,
  DrawChar,
  DrawString,
  DrawMonoBitmap,
  GIFAnimation,

};
static const tRPCPacked m_RPCP_Drawing[] =
{
  { (uint8_t)rpcDrawing::ScrollSelect,        1 },
  { (uint8_t)rpcDrawing::ScrollDefaultSet,    1 },
  { (uint8_t)rpcDrawing::ScrollEvent,         2 + sizeof(uint32_t) },
  { (uint8_t)rpcDrawing::ScrollText,          1 },
  { (uint8_t)rpcDrawing::ScrollState,         2 },
  { (uint8_t)rpcDrawing::ScrollMode,          1 },
  { (uint8_t)rpcDrawing::ScrollSpeed,         1 },
  { (uint8_t)rpcDrawing::ScrollFont,          1 },
  { (uint8_t)rpcDrawing::ScrollColor,         sizeof(rgb24) },
  { (uint8_t)rpcDrawing::ScrollOffsetHorizontal, 2 },
  { (uint8_t)rpcDrawing::ScrollOffsetVertical,   2 },
  { (uint8_t)rpcDrawing::ScrollBoundary,      sizeof(int16_t)*4 },

  { (uint8_t)rpcDrawing::DrawPixel,           sizeof(int16_t)*2 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawLine,            sizeof(int16_t)*4 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawFastVLine,       sizeof(int16_t)*3 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawFastHLine,       sizeof(int16_t)*3 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawCircle,          sizeof(int16_t)*3 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::FillCircle,          sizeof(int16_t)*3 + sizeof(rgb24)*2 },
  { (uint8_t)rpcDrawing::DrawEllipse,         sizeof(int16_t)*4 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::DrawTriangle,        sizeof(int16_t)*6 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::FillTriangle,        sizeof(int16_t)*6 + sizeof(rgb24)*2 },
  { (uint8_t)rpcDrawing::DrawRectangle,       sizeof(int16_t)*4 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::FillRectangle,       sizeof(int16_t)*4 + sizeof(rgb24)*2 },
  { (uint8_t)rpcDrawing::DrawRoundRectangle,  sizeof(int16_t)*5 + sizeof(rgb24) },
  { (uint8_t)rpcDrawing::FillRoundRectangle,  sizeof(int16_t)*5 + sizeof(rgb24)*2 },
  { (uint8_t)rpcDrawing::FillScreen,          sizeof(rgb24) },
  { (uint8_t)rpcDrawing::SetFont,             1 },
  { (uint8_t)rpcDrawing::DrawChar,            sizeof(int16_t)*2 + sizeof(rgb24) + 1 },
  { (uint8_t)rpcDrawing::DrawString,          sizeof(int16_t)*2 + sizeof(rgb24)*2 + 1 },
  { (uint8_t)rpcDrawing::DrawMonoBitmap,      sizeof(int16_t)*2 + 1 },
  { (uint8_t)rpcDrawing::GIFAnimation,        2 + sizeof(int16_t)*2 },
  
  {0, 0} // end of list
};


#define RPCDATA_SIZE  64                          // storage buffer size in bytes
#define RPCC_SIZE     2                           // command size in bytes
#define RPCPL_SIZE    (RPCDATA_SIZE - RPCC_SIZE)  // payload size in bytes

static uint8_t rpcDataTX[RPCDATA_SIZE];
static uint8_t rpcDataRX[RPCDATA_SIZE];



bool RPC_Send(rpcType type, uint8_t cmd, uint8_t *data, size_t size, bool clean = false);
template<typename T>
  inline bool RPC_Send(rpcType type, T cmd, uint8_t *data, size_t size, bool clean = false)
  {
    return RPC_Send(type, (uint8_t)cmd, data, size, clean);
  }

void RPC_System(rpcSystem cmd, uint8_t *data, size_t size);
void RPC_IO(rpcIO cmd, uint8_t *data, size_t size);
void RPC_Display(rpcDisplay cmd, uint8_t *data, size_t size);
void RPC_Drawing(rpcDrawing cmd, uint8_t *data, size_t size);


void RPC_Init()
{
  
}

void RPC_Tick()
{
  while(usbraw.available())
  {
    // retrieve packet
    usbraw.recv(rpcDataRX, 0);
    
    // first byte is always command
    rpcType  type  = (rpcType)rpcDataRX[0];
    uint8_t  *data = &rpcDataRX[1];
    
    // handle base command or type identifier
    switch(type)
    {
      case rpcType::System:  RPC_System( (rpcSystem)data[0],  data +1, RPCPL_SIZE); break;
      case rpcType::IO:      RPC_IO(     (rpcIO)data[0],      data +1, RPCPL_SIZE); break;
//      case Event:  // events are output only
      case rpcType::Display: RPC_Display((rpcDisplay)data[0], data +1, RPCPL_SIZE); break;
      case rpcType::Drawing: RPC_Drawing((rpcDrawing)data[0], data +1, RPCPL_SIZE); break;
      
      default:
        if((uint8_t)type < (uint8_t)rpcType::Framebuffer)
          break;

      case rpcType::Framebuffer:
      {
        if(Matrix_Control((uint8_t)type, data, RPCPL_SIZE +1))
          screenSwapping = true;
        break;
      }
    }
    
  }
  
  
  // send notification when framebuffer has been swapped
  if(screenSwapping && !matrix.swapPending)
  {
    screenSwapping = false;
    RPC_Send(rpcType::Display, rpcDisplay::SwapBuffers, NULL, 0);
  }
  
  // send scroller status changes
  for(int i=0; i<MATRIX_SCROLLERS; i++)
  {
    if(m_Scrollers[i].update == eScrollerEvent::None)
      continue;
    
    struct
    {
      uint8_t  id, status;
      uint32_t value;
    } PACKED p = // parameters
    {
      (uint8_t)i, (uint8_t)m_Scrollers[i].update, 0
    };
    
    if(m_Scrollers[i].update != eScrollerEvent::Stopped)
      p.value = matrix.getScroller(i).text.remain();
    
    m_Scrollers[i].update = eScrollerEvent::None;
    RPC_Send(rpcType::Drawing, rpcDrawing::ScrollEvent, (uint8_t *)&p, sizeof(p));
  }
  
}

//bool RPC_Send(rpcCommands cmd, uint8_t *data, size_t size, bool clean)
bool RPC_Send(rpcType type, uint8_t cmd, uint8_t *data, size_t size, bool clean)
{
  // reject over max payload size
  if(size > RPCPL_SIZE)
    return false;
  
  // set command bytes
  rpcDataTX[0] = (uint8_t)type;
  rpcDataTX[1] = (uint8_t)cmd;

  // store passed data to transmit buffer and
  // zero-out remaining of packet if requested
  if(size)
    memcpy(&rpcDataTX[RPCC_SIZE], data, size);
  if(clean && (size < RPCPL_SIZE))
    memset(&rpcDataTX[RPCC_SIZE + size], 0, RPCPL_SIZE - size);
  
  // transmit, wait forever
  if(usbraw.send(rpcDataTX, 0x0FFF) < FC_TX_SIZE)
    return false; // failed
  
  // success
  return true;
}


void RPC_System(rpcSystem cmd, uint8_t *data, size_t size)
{
  switch(cmd)
  {
    case rpcSystem::Reset:
    {
      // require an explicit request for reset to bootloader
//      if(!memcmp(F("RESET"), data, 6))
        system_reset();
      break;
    }
    case rpcSystem::Version:
    {
      // reply with version
      RPC_Send(rpcType::System, cmd, (uint8_t *)FIRMWARE_VERSION, strlen(FIRMWARE_VERSION) +1);
      break;
    }
    case rpcSystem::Ping:
    {
      // reply/pong back with passed payload
      RPC_Send(rpcType::System, cmd, data, size);
      break;
    }
    case rpcSystem::Timestamp:
    {
      uint64_t ts;
      
      memcpy(&ts, data, sizeof(ts));      
      Teensy3Clock.set((time_t)ts);
      break;
    }
  }
}

void RPC_IO(rpcIO cmd, uint8_t *data, size_t size)
{
  switch(cmd)
  {
    // Data buffer transfers
    case rpcIO::XferRecv:
    {
      // data transfer to LED matrix panel controller (Teensy)
      tRPCXfer xfer;
      memcpy(&xfer, data, sizeof(xfer));
      
      uint8_t slot  = xfer.index & RPCXFER_MASK_SLOT;
      if(slot >= IOBUFFERS_COUNT)
        break;
      
      tIOBuffer *buffer = &gm_IOBuffers[slot];
      
      if((xfer.index & RPCXFER_APPEND) == 0)
        buffer->used = 0;
      if(xfer.size > (size - sizeof(xfer)))
        xfer.size = (size - sizeof(xfer));
      
      size = xfer.size;
      if((size + (size_t)buffer->used) > (size_t)buffer->size)
        size = (size_t)buffer->size - (size_t)buffer->used;
      
      if(size)
      {
        memcpy(buffer->data + buffer->used, data + sizeof(xfer), size);
        buffer->used += (uint16_t)size;
      }
      break;
    }
    case rpcIO::XferSend:
    {
      // data transfer to LED controller's master (Linux/Host)
      break;
    }
  
    // Serial port(s) operations
    case rpcIO::SerialPortConfig:
    {
      tSerialConfig config;
      memcpy(&config, data, sizeof(config));
      
      HardwareSerial &sport = getSerialPort(config.index & 0x03);
      
      // check for port enable bit
      if(!(config.index & 0x80))
      {
        sport.end();
        break;
      }
      
      sport.begin(config.baud, config.format);
      break;
    }
    case rpcIO::SerialPortDataSend:
    {
      uint8_t index = data[0] & 0x03;
      uint8_t size  = data[0] >> 2;
      
      getSerialPort(index).write(&data[1], size);
      break;
    }
    // SerialPortDataRecv output only
    
    default:
      break;
  }
}

void RPC_Display(rpcDisplay cmd, uint8_t *data, size_t size)
{
  static eDisplayState displayState  = DisplayState_None;


  switch(cmd)
  {
    case rpcDisplay::Packed:
      break; // TODO
    case rpcDisplay::Resolution:
    {
      uint16_t dims[2] = // dimensions
      {
        (uint16_t)screenW, (uint16_t)screenH
      };
      
      // reply with display resolution
      RPC_Send(rpcType::Display, cmd, (uint8_t *)dims, sizeof(dims));
      break;
    }
    case rpcDisplay::Brightness:
    {
      matrix.setBrightness(data[0]);
      matrix.setBackgroundBrightness(data[1]);
      break;
    }
    case rpcDisplay::SwapBuffers:
    {
      screenSwapping = true;
      matrix.swapBuffers((bool)data[0]);
      break;
    }
    case rpcDisplay::Mode:
    {
      eDisplayState newState = (eDisplayState)data[0];
      
      if(newState == displayState)
        break;

      if(displayState == DisplayState_None)
        displayState = m_StateNow;

      if(newState == DisplayState_None)
      {
        newState = displayState;
        displayState = DisplayState_None;
      }
      
      m_StateNow = newState;
      break;
    }
  }
}

void RPC_Drawing(rpcDrawing cmd, uint8_t *data, size_t size)
{
  static TextScroller *scroller      = &matrix.getScroller(0);
  static uint8_t       scrollerIndex = 0;
  uint8_t              i;
  
  
  switch(cmd)
  {
    case rpcDrawing::Packed:
      break; // TODO
    
    //
    // Scrollers
    //
    
    // Select a text scroller
    case rpcDrawing::ScrollSelect:
    {
      i = data[0];
      if(i < MATRIX_SCROLLERS)
        scroller = &matrix.getScroller((size_t)i);
      
      scrollerIndex = i;
      break;
    }
    
    // Save or restore current scroller's default settings
    case rpcDrawing::ScrollDefaultSet:
    
    // case ScrollEvent is output only
    
    // Set text that is to be scrolled
    case rpcDrawing::ScrollText:
    {
#if 0
      if(!size || (size >= sizeof(gm_scrollText[0])))
        break;
      
      memcpy(gm_scrollText[scrollerIndex], data, size);
      gm_scrollText[scrollerIndex][size] = 0;
#endif // 0
      break;
    }
    
    // Set scroller current state; ex. start, stop, pause, etc..
    case rpcDrawing::ScrollState:
    {
      switch(data[0])
      {
        case ScrollState_Stop:
        {
          scroller->stopScrollText();
          break;
        }
        
        case ScrollState_Start:
        {
          scroller->stopScrollText();
          
          if(scroller->ringEnabled)
            scroller->setRingBuffer(NULL, 0);
          
          tIOBuffer *buffer = &gm_IOBuffers[IOBUFFERS_LSTART + scrollerIndex];
          if(buffer->used == buffer->size)
            buffer->data[buffer->used -1] = 0;
          else
            buffer->data[buffer->used] = 0;
          
          scroller->scrollText((const char *)buffer->data, (int)((int8_t)data[1]));
          break;
        }
        
        case ScrollState_Append:
        {
          tIOBuffer *buffer;
          
          if(!scroller->ringEnabled)
          {
            buffer = &gm_IOBuffers[IOBUFFERS_LSTART + scrollerIndex];
            
            scroller->stopScrollText();
            scroller->setRingBuffer(buffer->data, buffer->size);
          }
          
          uint8_t buffnum = data[1];
          if(buffnum < IOBUFFERS_COUNT)
          {
            tIOBuffer *buffer = &gm_IOBuffers[buffnum];
            
            // enforce NULL terminator
            if(buffer->used == buffer->size)
              buffer->data[buffer->used -1] = 0;
            else
              buffer->data[buffer->used] = 0;
            
            scroller->appendRing((const char*)buffer->data);
          }
          break;
        }
          
        case ScrollState_Pause:
        default:
          break;  // TODO: not yet implemented
      }
      break;
    }
    
    // Scroller mode
    case rpcDrawing::ScrollMode:
    {
      i = data[0];
      scroller->setScrollMode((ScrollMode)i);
      break;
    }
    
    // Scroller speed
    case rpcDrawing::ScrollSpeed:
    {
      i = data[0];
      scroller->setScrollSpeed(i? i:1); // prevent divide by zero
      break;
    }
    
    // Scroller text font
    case rpcDrawing::ScrollFont:
    {
      i = data[0];
      scroller->setScrollFont((fontChoices)((i>FONT_MAXINDEX)? FONT_MAXINDEX:i)); // prevent out of range violation
      break;
    }
    
    // Scroller text color
    case rpcDrawing::ScrollColor:
    {
      rgb24 c;
      c.red   = data[0];
      c.green = data[1];
      c.blue  = data[2];
      scroller->setScrollColor(c);
      break;
    }
    
    // Scroller horizontal offset position
    case rpcDrawing::ScrollOffsetHorizontal:
    {
      uint16_t si = UInt8PToUInt16(data);
      scroller->setScrollStartOffsetFromLeft((int)si);
      break;
    }
    
    // Scroller vertical offset position
    case rpcDrawing::ScrollOffsetVertical:
    {
      uint16_t si = UInt8PToUInt16(data);
      scroller->setScrollOffsetFromTop((int)si);
      break;
    }
    
    // Scroller rectangle bounds
    case rpcDrawing::ScrollBoundary:
    {
      int16_t bounds[4];
      memcpy(bounds, data, sizeof(bounds));
      scroller->setScrollBoundary(bounds[0], bounds[1], bounds[2], bounds[3]);
      break;
    }
    
    
    //
    // Background drawing
    //

    case rpcDrawing::DrawPixel:
    {
      struct
      {
        int16_t  x, y;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawPixel(p.x, p.y, p.color);
      break;
    }
    case rpcDrawing::DrawLine:
    {
      struct
      {
        int16_t  x0, y0, x1, y1;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawLine(p.x0, p.y0, p.x1, p.y1, p.color);
      break;
    }
    case rpcDrawing::DrawFastVLine:
    {
      struct
      {
        int16_t  x, y0, y1;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawFastVLine(p.x, p.y0, p.y1, p.color);
      break;
    }
    case rpcDrawing::DrawFastHLine:
    {
      struct
      {
        int16_t  x0, x1, y;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawFastHLine(p.x0, p.x1, p.y, p.color);
      break;
    }
    case rpcDrawing::DrawCircle:
    {
      struct
      {
        int16_t  x, y, radius;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawCircle(p.x, p.y, p.radius, p.color);
      break;
    }
    case rpcDrawing::FillCircle:
    {
      struct
      {
        int16_t  x, y, radius;
        rgb24    colorOutline, colorFill;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      if(p.colorOutline == p.colorFill)
        matrix.fillCircle(p.x, p.y, p.radius, p.colorFill);
      else
        matrix.fillCircle(p.x, p.y, p.radius, p.colorOutline, p.colorFill);
      break;
    }
    case rpcDrawing::DrawEllipse:
    {
      struct
      {
        int16_t  x, y, radiusX, radiusY;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawEllipse(p.x, p.y, p.radiusX, p.radiusY, p.color);
      break;
    }
    case rpcDrawing::DrawTriangle:
    {
      struct
      {
        int16_t  x0, y0, x1, y1, x2, y2;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawTriangle(p.x0, p.y0, p.x1, p.y1, p.x2, p.y2, p.color);
      break;
    }
    case rpcDrawing::FillTriangle:
    {
      struct
      {
        int16_t  x0, y0, x1, y1, x2, y2;
        rgb24    colorOutline, colorFill;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      if(p.colorOutline == p.colorFill)
        matrix.fillTriangle(p.x0, p.y0, p.x1, p.y1, p.x2, p.y2, p.colorFill);
      else
        matrix.fillTriangle(p.x0, p.y0, p.x1, p.y1, p.x2, p.y2, p.colorOutline, p.colorFill);
      break;
    }
    case rpcDrawing::DrawRectangle:
    {
      struct
      {
        int16_t  x0, y0, x1, y1;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawRectangle(p.x0, p.y0, p.x1, p.y1, p.color);
      break;
    }
    case rpcDrawing::FillRectangle:
    {
      struct
      {
        int16_t  x0, y0, x1, y1;
        rgb24    colorOutline, colorFill;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      if(p.colorOutline == p.colorFill)
        matrix.fillRectangle(p.x0, p.y0, p.x1, p.y1, p.colorFill);
      else
        matrix.fillRectangle(p.x0, p.y0, p.x1, p.y1, p.colorOutline, p.colorFill);
      break;
    }
    case rpcDrawing::DrawRoundRectangle:
    {
      struct
      {
        int16_t  x0, y0, x1, y1, radius;
        rgb24    color;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawRoundRectangle(p.x0, p.y0, p.x1, p.y1, p.radius, p.color);
      break;
    }
    case rpcDrawing::FillRoundRectangle:
    {
      struct
      {
        int16_t  x0, y0, x1, y1, radius;
        rgb24    colorOutline, colorFill;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      if(p.colorOutline == p.colorFill)
        matrix.fillRoundRectangle(p.x0, p.y0, p.x1, p.y1, p.radius, p.colorFill);
      else
        matrix.fillRoundRectangle(p.x0, p.y0, p.x1, p.y1, p.radius, p.colorOutline, p.colorFill);
      break;
    }
    case rpcDrawing::FillScreen:
    {
      rgb24 color;
      
      color.red   = data[0];
      color.green = data[1];
      color.blue  = data[2];
      matrix.fillScreen(color);
      break;
    }
    case rpcDrawing::SetFont:
    {
      i = data[0];
      matrix.setFont((fontChoices)((i>FONT_MAXINDEX)? FONT_MAXINDEX:i)); // prevent out of range violation
      break;
    }
    case rpcDrawing::DrawChar:
    {
      struct
      {
        int16_t  x, y;
        rgb24    color;
        char     chr;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      matrix.drawChar(p.x, p.y, p.color, p.chr);
      break;
    }
    case rpcDrawing::DrawString:
    {
      struct
      {
        int16_t  x, y;
        rgb24    colorFG, colorBG;
        uint8_t  buffer;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      if(p.buffer < IOBUFFERS_COUNT)
      {
        tIOBuffer *buffer = &gm_IOBuffers[p.buffer];
        
        // enforce NULL terminator
        if(buffer->used == buffer->size)
          buffer->data[buffer->used -1] = 0;
        else
          buffer->data[buffer->used] = 0;
        
        if(p.colorFG == p.colorBG)
          matrix.drawString(p.x, p.y, p.colorFG, (const char*)buffer->data);
        else
          matrix.drawString(p.x, p.y, p.colorFG, p.colorBG, (const char*)buffer->data);
      }
      break;
    }
    case rpcDrawing::DrawMonoBitmap:
    {
      struct
      {
        int16_t  x, y;
        rgb24    color;
        uint8_t  buffer;
      } PACKED p; // parameters
      
      memcpy(&p, data, sizeof(p));
      // not implemented at this time
      break;
    }
    
    case rpcDrawing::GIFAnimation:
    {
      uint8_t control = data[0];
      data++;
      
      // Load GIF from filesystem flag
      if(control & RPCGIF_FLAG_LOAD)
      {
        uint8_t buffnum = data[0];
        data++;
        
        if(buffnum < IOBUFFERS_COUNT)
        {
          tIOBuffer *buffer = &gm_IOBuffers[buffnum];
          
          // enforce NULL terminator
          if(buffer->used == buffer->size)
            buffer->data[buffer->used -1] = 0;
          else
            buffer->data[buffer->used] = 0;
          
          Matrix_GIFLoad((const char*)buffer->data);
        }
      }
      
      // Set GIFPlayer position flag
      if(control & RPCGIF_FLAG_POS)
      {
        struct
        {
          int16_t x, y;
        } PACKED p; // parameters
        
        memcpy(&p, data, sizeof(p));
        data += sizeof(p);
        
        m_GIF.x = p.x;
        m_GIF.y = p.y;
      }
      
      // Set GIFPlayer animation interval flag
      if(control & RPCGIF_FLAG_INTERVAL)
      {
        struct
        {
          uint16_t interval;
        } PACKED p; // parameters
        
        memcpy(&p, data, sizeof(p));
        data += sizeof(p);
        
        m_GIF.interval = (uint32_t)p.interval;
      }
      
      m_GIF.state = (rpcGIFState)(control & RPCGIF_MASK_STATE);
      break;
    }
    
    default:
      break;
  }
}


//=============================================================================
// USB Bulk API
//=============================================================================

int usb_bulk_available(void)
{
	uint32_t count;

	if (!usb_configuration) return 0;
	count = usb_rx_byte_count(FC_RX_ENDPOINT);
	return count;
}

int usb_bulk_recv(void *buffer, uint32_t timeout)
{
      	usb_packet_t *rx_packet;
      	uint32_t begin = millis();
      
      	while (1) {
      		if (!usb_configuration) return -1;
      		rx_packet = usb_rx(FC_RX_ENDPOINT);
      		if (rx_packet) break;
      		if (millis() - begin > timeout || !timeout) return 0;
      		yield();
      	}
      	memcpy(buffer, rx_packet->buf, FC_RX_SIZE);
      	usb_free(rx_packet);
      	return FC_RX_SIZE;
}

// Maximum number of transmit packets to queue so we don't starve other endpoints for memory
#define TX_PACKET_LIMIT 4

int usb_bulk_send(const void *buffer, uint32_t timeout)
{
	usb_packet_t *tx_packet;
	uint32_t begin = millis();

	while (1) {
		if (!usb_configuration) return -1;
		if (usb_tx_packet_count(FC_TX_ENDPOINT) < TX_PACKET_LIMIT) {
			tx_packet = usb_malloc();
			if (tx_packet) break;
		}
		if (millis() - begin > timeout) return 0;
		yield();
	}
	memcpy(tx_packet->buf, buffer, FC_TX_SIZE);
	tx_packet->len = FC_TX_SIZE;
	usb_tx(FC_TX_ENDPOINT, tx_packet);
	return FC_TX_SIZE;
}


