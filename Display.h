/*
 * LED Matrix module
 */

#define FONT_MAXINDEX 5 // 0 - 5 or 6 total


SmartMatrix  matrix;
GifPlayer    gifPlayer;

static int            defaultBrightness       =  30 * (255/100);    // 30% brightness
static int            defaultScrollSpeed      = 50;
static const int      defaultScrollOffset     =  1;
static const rgb24    defaultBackgroundColor  = {0, 0, 0};
             rgb24    color;

static volatile bool  screenSwapping;
static int            screenW;
static int            screenH;
static size_t         rasterPos;
static size_t         rasterSize;

static int            m_smsNewCount = 0;


// display state machine
static eDisplayState m_StateNow    = DisplayState_None; //DisplayState_DateTime;    // current display state
static eDisplayState m_StateBefore = DisplayState_None;        // previous display state
static bool          m_DispMaySwap = false;                    // display state may be interrupted and swapped flag

struct
{
  int16_t            x, y;
  uint32_t           interval;
  rpcGIFState        state;
}                    m_GIF = {0, 0, 80, rpcGIFState::Play};

struct
{
  eScrollerEvent     update = eScrollerEvent::None;
}                    m_Scrollers[MATRIX_SCROLLERS];


// local function prototypes
static void LEDMatrix_Display_Settings();
static void LEDMatrix_Display_DateTime();
static void LEDMatrix_Display_Alarm();
static void LEDMatrix_Display_Timer();
static void LEDMatrix_Display_Messages();
static rgb24 getRandomColor();
static void colorWheel(rgb24 &color, uint8_t WheelPos);
bool Matrix_GIFLoad(const char *filepath);
void Matrix_GIFPlay(int x=0, int y=0);
void scrollerCallback(TextScroller *scroller, eScrollerEvent event);


/**
 * LED matrix initialization function.
 */
bool LEDMatrix_Init()
{
  // ready LED matrix array
  matrix.begin();
  matrix.setBrightness(defaultBrightness);
  matrix.setScrollOffsetFromTop(defaultScrollOffset);
  matrix.setColorCorrection(cc24);
  
  // randomize color to use
  color = getRandomColor();
  
  // starting attributes
  matrix.setScrollColor({0xff, 0xff, 0xff});
  matrix.setScrollMode(wrapForward);
  matrix.setScrollSpeed(defaultScrollSpeed);
  matrix.setScrollFont(font8x13);
  
  // get panel dimensions
  screenW = matrix.getScreenWidth();
  screenH = matrix.getScreenHeight();
  
  screenSwapping  = false;
  rasterPos       = 0;
  rasterSize      = screenW * screenH * sizeof(rgb24);
  
  // display an initializing screen
  matrix.drawFastHLine(0, screenW -1, 0, {128, 0, 0});
  matrix.drawFastHLine(1, screenW -1, screenH -1, {0, 0, 128});
  matrix.drawFastVLine(0, 1, screenH -1, {0, 128, 0});
  matrix.drawFastVLine(screenW -1, 0, screenH -2, {128, 128, 0});
  matrix.setScrollMode(wrapForward);
  matrix.scrollText((const char*)F("Loading"), -1);
  matrix.swapBuffers();
  
#if 0
  // second scroller test
  TextScroller &scroll2 = matrix.getScroller(1);
  scroll2.setScrollOffsetFromTop(0);
  scroll2.setScrollColor({0x40, 0xe0, 0xd0}); // turquoise
  scroll2.setScrollMode(wrapForward);
  scroll2.setScrollSpeed(40);
  scroll2.setScrollFont(font5x7);
  scroll2.scrollText((const char*)F("Second scroller.  Wee!  ;-)"), -1);

  // fourth scroller test
  TextScroller &scroll4 = matrix.getScroller(3);
  scroll4.setScrollOffsetFromTop(7);
  scroll4.setScrollColor({0xff, 0xd3, 0x00}); // unique hue yellow
  scroll4.setScrollMode(wrapForward);
  scroll4.setScrollSpeed(20);
  scroll4.setScrollFont(gohufont11);
  scroll4.scrollText((const char*)F("<<< The Fourth Scroller >>>"), -1);

#endif // 0

  for(int i=0; i<MATRIX_SCROLLERS; i++)
  {
    matrix.getScroller(i).setEventCallback(scrollerCallback);
  }
  
  // success
  return true; 
}

void LEDMatrix_Status(const char *text)
{
  matrix.scrollText(text, -1);
}
void LEDMatrix_Status(const __FlashStringHelper *text)
{
  matrix.scrollText((const char*)text, -1);
}


/**
 * LED matrix maintenance function.
 */
void LEDMatrix_Tick()
{
//  uint32_t clock = millis();
  static InputCommand  lastCmd        = InputCommand::None;
  static uint32_t      lastCmdTick    = 0;
  static bool          lastCmdLocked  = false;

  
  // reset command lock on command change
  if(lastCmdLocked && (gm_Settings.command != lastCmd))
  {
    // reset last command state machine
    lastCmdTick   = 0;
    lastCmdLocked = false;
  }
  
  // check for Brightness decrement and increment commands
  if(gm_Settings.command == InputCommand::BrightDec ||
     gm_Settings.command == InputCommand::BrightInc)
  {
    // require that:
    //  Haven't seen this command tick before
    // AND
    //   Haven't seen this command before
    //  OR
    //   Command stayed for 0.8 seconds
    if((lastCmdTick != gm_Settings.commandTicks) &&
       (!lastCmdLocked || (gm_Settings.getCommandTime() > 800)))
    {
      lastCmd       = gm_Settings.command;
      lastCmdTick   = gm_Settings.commandTicks;
      lastCmdLocked = true;

      // use high rate when command stayed for over 3 seconds
      int rate = (gm_Settings.getCommandTime() > 3000)? 5 : 1;
      
      defaultBrightness += (gm_Settings.command == InputCommand::BrightDec)? -rate : rate;
      defaultBrightness  = (defaultBrightness > 255)? 255 : defaultBrightness;
      defaultBrightness  = (defaultBrightness <   0)?   0 : defaultBrightness;
      
      matrix.setBrightness((uint8_t)defaultBrightness);
    }
  }
  
  // don't interfere with displayed error message if init failed
  if(gm_InitError)
    return;
  
  // do not interfere with manual display operation, except for drawing GIF animations
  if(m_StateNow == DisplayState_Manual)
  {
    static uint32_t lastGIFTime = 0;
    uint32_t clock = millis();
    
    if((m_GIF.state != rpcGIFState::Play) || ((clock - lastGIFTime) < m_GIF.interval))
      return;
    
    lastGIFTime = clock;
    Matrix_GIFPlay(m_GIF.x, m_GIF.y);
    matrix.swapBuffers();
    return;
  }
  
  // check for screen mode change action
  if(!lastCmdLocked && (gm_Settings.command == InputCommand::StopMode))
  {
    lastCmd       = gm_Settings.command;
    lastCmdLocked = true;
    
    // just toggle between date & time and messages for now
    m_StateNow = (m_StateNow == DisplayState_DateTime)? DisplayState_Messages : DisplayState_DateTime;
  }
  
  // view pages
  switch(m_StateNow)
  {
    default:
    case DisplayState_None:
    {
      static uint32_t lastGIFTime = 0;
      uint32_t clock = millis();
      if((m_GIF.state != rpcGIFState::Play) || ((clock - lastGIFTime) < m_GIF.interval))
        break;
      lastGIFTime = clock;


#if 0
      // bounce GIF animation horizontally
      static int  px    = 0;
      static bool pdir  = false;
      const  int  pstep = 1;
      if(!pdir)
      {
        px += pstep;
        if(px > (screenW -1 -32 - pstep))
          pdir = !pdir;
      } else
      {
        px -= pstep;
        if(px < pstep)
          pdir = !pdir;
      }
      Matrix_GIFPlay(px, screenH -32 -1);
#else
      Matrix_GIFPlay(m_GIF.x, m_GIF.y);
#endif // 0
      matrix.swapBuffers();
      break;
    }
    case DisplayState_Settings: LEDMatrix_Display_Settings(); break;
    case DisplayState_DateTime: LEDMatrix_Display_DateTime(); break;
    case DisplayState_Alarm:    LEDMatrix_Display_Alarm(); break;
    case DisplayState_Timer:    LEDMatrix_Display_Timer(); break;
    case DisplayState_Messages: LEDMatrix_Display_Messages(); break;
  }
  
  m_StateBefore = m_StateNow;
}



static rgb24 getRandomColor()
{
//  const uint transitionTime = 10;
//  unsigned long currentMillis;
  
//  currentMillis = millis();

//        while (millis() - currentMillis < transitionTime) {
//            float fraction = ((float)millis() - currentMillis) / ((float)transitionTime / 2);

//            if (millis() - currentMillis < transitionTime / 2) {
//                color.red = 255 - 255.0 * fraction;
//                color.green = 255.0 * fraction;
//                color.blue = 0;
//            }
//            else {
//                color.red = 0;
//                color.green = 255 - 255.0 / 2 * (fraction - 1.0);
//                color.blue = 255.0 * (fraction - 1.0);
//            }
//        }

  rgb24 color;
  
  color.red    = random(255);
  color.green  = random(255);
  color.blue   = random(255);
  
  return(color);
}

static void colorWheel(rgb24 &color, uint8_t WheelPos)
{
  if(WheelPos < 85)
  {
    color.red   = WheelPos * 3;
    color.green = 255 - WheelPos * 3;
    color.blue  = 0;
    return;
  } else if(WheelPos < 170)
  {
    WheelPos   -=  85;
    color.red   = 255 - WheelPos * 3;
    color.green = 0;
    color.blue  = WheelPos * 3;
    return;
  } else
  {
    WheelPos   -= 170;
    color.red   = 0;
    color.green = WheelPos * 3;
    color.blue  = 255 - WheelPos * 3;
  }
}


static File fileGIF;
bool Matrix_GIFLoad(const char *filepath)
{
  // close opened file before opening another
  if(filepath && fileGIF)
    fileGIF.close();
  
  // Abort upon the following conditions:
  if(!gm_SDAvailable ||                                        // No SD card, or
     (!fileGIF && !filepath) ||                                // No file loaded and NULL path provided, or
     (filepath && !(fileGIF = SD.open(filepath, FILE_READ)) )  // File path provided and but failed to open file
    )
    return false;
  
  // when filepath NULL assume request to reuse already opened file
  if(!filepath)
    fileGIF.seek(0);
  else
    gifPlayer.setFile(fileGIF);
    
  if(!gifPlayer.parseGifHeader())
  {
    fileGIF.close();
    return false;
  }
  
  gifPlayer.parseLogicalScreenDescriptor();
  gifPlayer.parseGlobalColorTable();
  
  return true;
}

void Matrix_GIFPlay(int x, int y)
{
  long result;
  
  if(!fileGIF)
    return;
  
  result = gifPlayer.drawFrame(x, y);
  if(result == ERROR_FINISHED)
    Matrix_GIFLoad(NULL);
}

bool Matrix_Control(uint8_t control, uint8_t *data, size_t size)
{
  control = control >> 4;
  static int calls=0;
  
  // check for append bit
  if((control & 0x01) == 0)
  {
    rasterPos = 0;
    //calls = 0;
  }
  
  // check and prevent buffer overflow
  if((size + rasterPos) >= rasterSize)
    size = rasterSize - rasterPos;
  if(size)
  {
// test raster mode, a visual debug aid
if(control & 0x08)
{
    static rgb24 ccs[] =
    {
      {255,   0,   0},
      {255, 128,   0},
      {255, 255,   0},
      {128, 255,   0},
      {  0, 255,   0},
      {  0, 255, 128},
      {  0, 255, 255},
      {  0, 128, 255},
      {  0,   0, 255},
      {128,   0, 255},
      {255,   0, 255},
      {255,   0, 128},
    };
    rgb24 &testcolor = ccs[calls++ % 12];
    rgb24 *colors = matrix.getRealBackBuffer();
    
    for(size_t i=0; i<(size/3); i++)
      colors[i + (rasterPos/3)] = testcolor;
      
} else {
  
    uint8_t *fb = (uint8_t *)matrix.backBuffer();
    memcpy(&fb[rasterPos], data, size);

}

    rasterPos += size;
  }
  
  // check for flush/swap buffers bit
  if(control & 0x02)
  {
    rasterPos = 0;
    matrix.swapBuffers();
    return true;
  }
  
  return false;
}

void scrollerCallback(TextScroller *scroller, eScrollerEvent event)
{
//  size_t index = ((size_t)&matrix.getScroller(0) - (size_t)scroller) / sizeof(TextScroller);
  
  // nasty inefficient hack, need to replace it
  for(size_t i=0; i<MATRIX_SCROLLERS; i++)
  {
    if(&matrix.getScroller(i) == scroller)
    {
      m_Scrollers[i].update = event;
      break;
    }
  }
}


