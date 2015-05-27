/*
 * Display Screens module
 */


//=============================================================================
// Screen: Messages
//=============================================================================


// SMS message display state
enum class eMessageState
{
  None = 0,
  From,
  Showing,
  Pause
};

//static eMessageState m_StateMsg = eMessageState::None;

static void LEDMatrix_Display_Messages()
{
#if 0
  char * const           text            = gm_text;

  static bool            stateRedraw     = false;
  static bool            stateChanged    = false;
  static fontChoices     font            = (fontChoices)0;
  static tSMSMsg         msg;
  static InputCommand    lastCmd         = InputCommand::None;
  static int             cmd             = 0;
  static int             msgNum          = 0;
  
  
  // check for screen view change
  if((m_StateNow != m_StateBefore))
  {
    stateRedraw  = true;
    stateChanged = true;
    
    matrix.stopScrollText();
  }
  
  if(command_CheckNextPrev(lastCmd, cmd, -1, 1))
  {
    msgNum += cmd;
    if(msgNum < 0)
      msgNum = m_smsIndexCount;
    if(msgNum > m_smsIndexCount)
      msgNum = 0;
    
    cmd         = 0;
    m_StateMsg  = (!msgNum)? eMessageState::None : eMessageState::Showing;
    stateRedraw = true;
    
    matrix.stopScrollText();
  }
  
  
  // avoid unnecessary updates
  if(!stateRedraw)
    return;
  
  // clear screen
  matrix.fillScreen(defaultBackgroundColor);
  
  switch(m_StateMsg)
  {
    case eMessageState::None:
    {
      // no message shown, display states instead
      
      matrix.setFont(font = font5x7);
      
      const bitmap_font *pFont = matrix.fontLookup(font);
      int        x=0, y=0, len;
      const char *pText;
      char       *pt;
      
      
      sprintf(text, "Messages\n%u New, %u Old", m_smsNewCount, m_smsIndexCount - m_smsNewCount);
      
      // find newline
      pt = strchr(text, '\n');
      
      // draw first line
      pText = text;
      len   = PTRLEN(pText, pt) * pFont->Width;
      x     = (screenW / 2) - (len / 2);
      y     = (screenH / 2) - (pFont->Height * 2 / 2);
      matrix.drawString(x, y, color, pText);
      
      // draw second line
      pText = pt +1;
      len   = strlen(pText) * pFont->Width;
      x     = (screenW / 2) - (len / 2);
      y    += pFont->Height;      
      matrix.drawString(x, y, color, pText);
      break;
    }
    
    case eMessageState::From:
    {
      break;
    }
    
    case eMessageState::Showing:
    {
      if(!stateRedraw)
        break;
      
      if(!Modem_Message_Retrieve(msg, m_smsIndex[msgNum -1].index))
      {
        // retrieval error
        sprintf(text, (const char *)F("Failed to get SMS #%d"), msgNum);
        matrix.scrollText(text, -1);
        break;
      }
      
      // format message display: SMS #sortedIndexNum From <phone#> -- <message>
      sprintf(text, (const char *)F("SMS #%d From %s -- %s"),
        msgNum, msg.from, (msg.size)? msg.msg : (const char *)F("<Empty>"));
        
      // display message
      matrix.scrollText(text, -1);
      break;
    }
    
    case eMessageState::Pause:
    {
      break;
    }
  }
  
  // reset state flags
  stateRedraw  = false;
  stateChanged = false;
  
  // show drawn graphics
  matrix.swapBuffers(true);

  // done
#endif // 0
}



//=============================================================================
// Screen: Date & Time
//=============================================================================

static void LEDMatrix_Display_DateTime()
{
  char * const           text            = gm_text;
  const  int             alertIntveral   = 2; // seconds to wait before toggling alert
  
  static rgb24           color;
  static bool            syncUpdate      = false;
  static bool            syncOnStop      = false;
  static time_t          lastTime        = -1;               // 0 wasn't working
//  static ScrollMode      lastScrollMode  = wrapForward;
  static int             mode            = 0;                // presentation mode/style
  static int             doAMPM          = 1;
  static int             doAlert         = 0;
  static InputCommand    modeCmd         = InputCommand::None;
  static InputCommand    doAMPMCmd       = InputCommand::None;
//  static InputCommand    lastCmd         = InputCommand::None;
  static fontChoices     font            = (fontChoices)0;
  time_t                 now             = ::now();
  char                   *pt;
  
  int                    scrollStatus    = matrix.getScrollStatus();

  // check for:
  //   Presentation mode change
  //   Time formatting mode change
  if(command_CheckNextPrev(modeCmd, mode, 0, 1) ||
     command_CheckLeftRight(doAMPMCmd, doAMPM, 0, 1) ||
     (syncOnStop && !scrollStatus)
    )
  {
    syncUpdate = true;
  }

  // check for time changes
  if(syncUpdate || (now != lastTime))
  {
    // check new SMS messages
    if(m_smsNewCount)
    {
      if((mode == 1) && (now != lastTime))
      {
        if(--doAlert < -alertIntveral)
          doAlert = alertIntveral;
      } else
      if(syncOnStop && !scrollStatus)
      {
        doAlert = !doAlert;
      }
    }
    
    if(doAlert > 0)
    {
      const char *plural = (const char *)((m_smsNewCount == 1)? F("\nis"):F("s\nare"));
      const char *format = (const char *)((mode == 1)?
                           F("%u New msg%s waiting") :
                           F("%u New message%s available"));
      
      sprintf(text, format, (uint32_t)m_smsNewCount, plural);
    } else
    {
      tmElements_t tm;
      
      breakTime(now, tm);
      if(doAMPM)
      {
        bool pm;
        
        pm = (tm.Hour >= 12);
        if(tm.Hour == 0)
          tm.Hour  = 12;
        else if(tm.Hour > 12)
          tm.Hour -= 12;
        
        // YYYY-MM-DD HH:MM:SS xM
        sprintf(text, (const char*)F("%04lu-%02u-%02u\n%02u:%02u:%02u %cM"),
                ((uint32_t)tm.Year + 1970UL), tm.Month, tm.Day,
                tm.Hour, tm.Minute, tm.Second, ((pm)? 'P':'A'));
      } else
      {
        // YYYY-MM-DD HH:MM:SS
        sprintf(text, (const char*)F("%04lu-%02u-%02u\n%02u:%02u:%02u   "),
                ((uint32_t)tm.Year + 1970UL), tm.Month, tm.Day,
                tm.Hour, tm.Minute, tm.Second);
      }
    }
    
    lastTime = now;
  }
  
  // check for mode change
  if(syncUpdate || (m_StateNow != m_StateBefore))
  {
    syncUpdate = false;
    syncOnStop = false;
    
    // allow us to be swapped out
    m_DispMaySwap = true;
    
    if(m_StateNow != m_StateBefore)
    {
      command_CheckReset(modeCmd);
      command_CheckReset(doAMPMCmd);
      matrix.stopScrollText();
    }
    
    // handle presentation mode
    if(mode == 0)
    {
      // scrolling one line
      
      pt = strchr(text, '\n');
      if(pt)
        *pt = ' ';
      
      matrix.fillScreen(defaultBackgroundColor);
      matrix.swapBuffers();
      matrix.setScrollFont(font = font8x13);
      matrix.setScrollMode(wrapForward);
//      matrix.scrollText(text, -1);
    } else
    {
      // dual-line mode
      matrix.stopScrollText();
      matrix.setFont(font = font5x7);
    }
  }
  
  static uint32_t lastClock = 0;
  static uint8_t pos = 0;
  uint32_t clock = millis();
  
  // update display at a fixed interval (20Hz / 50ms)
  if(clock - lastClock > 50)
  {
    lastClock = clock;
    colorWheel(color, ++pos);
    matrix.setScrollColor(color);
    
    // check presentation mode
    if(mode == 0)
    {
      // scrolling one line
      
      if(!matrix.getScrollStatus())
      {
        matrix.scrollText(text, 1);
        
        if(m_smsNewCount)
          syncOnStop = true;
      }
    } else
    {
      // dual-line mode
      
      const bitmap_font *pFont = matrix.fontLookup(font);
      int x=0, y=0, len;
      const char *pText;
      
      
      matrix.fillScreen(defaultBackgroundColor);
      
      // find newline
      pt = strchr(text, '\n');
      
      // draw first line
      pText = text;
      len   = PTRLEN(pText, pt) * pFont->Width;
      x     = (screenW / 2) - (len / 2);
      y     = (screenH / 2) - (pFont->Height * 2 / 2);
      matrix.drawString(x, y, color, pText);
      
      // draw second line
      pText = pt +1;
      len   = strlen(pText) * pFont->Width;
      x     = (screenW / 2) - (len / 2);
      y    += pFont->Height;      
      matrix.drawString(x, y, color, pText);
      
      matrix.swapBuffers();
    }
  }

}



//=============================================================================
// Screen: Others, not implemented yet
//=============================================================================

static void LEDMatrix_Display_Settings()
{
}

static void LEDMatrix_Display_Alarm()
{
}

static void LEDMatrix_Display_Timer()
{
}


