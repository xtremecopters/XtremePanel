/*
 * XtremePanel main
 */

// system headers
#include <Arduino.h>
#include <HardwareSerial.h>
#include <Time.h>
#include "usb_dev.h"
#include <SPI.h>
#include <SD.h>

// library headers
#include <SmartMatrix_16x32.h>
#include <IRremote.h>
//#define Enable_IRRemote

bool gm_InitError   = false;
bool gm_SDAvailable = false;

// project headers
#include "XtremePanelCommon.h"
#include "IRRemote.h"
#include <led_sysdefs.h>
#include <pixeltypes.h>
#include "GifPlayer.h"
#include "Display.h"
#include "Screens.h"
#include "RPC.h"


// Teensy 3.0 has the LED on pin 13
const int ledPin = 13;

// local function prototypes
static time_t getTeensy3Time();
void system_reset() __attribute__((noreturn));


// the setup() method runs once, when the sketch starts
void setup()
{
#if 0
  // switch pins to use 16/17 for I2C instead of 18/19
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  CORE_PIN16_CONFIG = (PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS);
  CORE_PIN17_CONFIG = (PORT_PCR_MUX(2) | PORT_PCR_PE | PORT_PCR_PS);
#endif // 0

  // initialize the digital pin as an output.
//  pinMode(ledPin, OUTPUT);
  
  // set getRandom() seed with ADC noise
  randomSeed(analogRead(0));
  
  // set callback function for time retrieval
  setSyncProvider(getTeensy3Time);
  
  // initialize modules
  LEDMatrix_Init();
#if defined(Enable_IRRemote)
  Remote_Init();
#endif // Enable_IRRemote

  pinMode(SD_CARD_CS, OUTPUT);
  gm_SDAvailable = SD.begin(SD_CARD_CS);
  
  // initialize realtime clock
  // set current time
  Teensy3Clock.set((time_t)1425919305); // set a valid timestamp
  setSyncInterval(1);
  
  LEDMatrix_Status(F("Waiting for system to bootup..."));
  while(!usb_configuration);
  
  LEDMatrix_Status(F("Ready for commands"));
  Matrix_GIFLoad((const char *)F("gifs/PacMan.gif"));
  
  // done
}

static time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}


// the loop() method runs over and over again,
// as long as the board has power
void loop()
{
  // check on communications
  RPC_Tick();


#if defined(Enable_IRRemote)
  // poll IR remote codes
  static uint32_t lastRemote  = 0;
  static bool     longCmdSeen = false;
  InputCommand cmd;
  uint32_t clock = millis();
  
  // wait 200ms between IR captures
  if(clock - lastRemote > IR_RECV_POLLRATE)
  {
#if 0
    usb_was_really_init = 1;
    if(usb_was_really_init)
    {
      static bool toggle = false;
      digitalWrite(ledPin, (toggle = !toggle)? LOW:HIGH);
    }
#endif // 0
    
    // remember time IR codes were captured
    lastRemote = clock;

    cmd = readCommand();
    
    // check for repeated command (button held)
    if(cmd == InputCommand::Same)
      cmd = gm_Settings.command;
    
    if(gm_Settings.command == cmd)
    {
      // command not changed, tick increment
      gm_Settings.commandTicks++;
    } else
    {
      tRemoteEvent event =
      {
        (uint8_t)cmd,
        (uint8_t)gm_Settings.command,
        gm_Settings.commandTicks
      };
      
      // notify master of remote input events
      RPC_Send(rpcType::Event, rpcEvent::IRRemote, (uint8_t *)&event, sizeof(event));
      
      
      // command changed, set new command and reset ticks
      gm_Settings.command        = cmd;
      gm_Settings.commandTicks   = 0;
      gm_Settings.commandChanged = true;

      longCmdSeen = false;
      
      // while a remote button is pressed light up onboard LED
//      digitalWrite(ledPin, (cmd == InputCommand::None)? LOW:HIGH);
    }


    // check for the following commands/buttons held for at least 3 seconds
    if(!longCmdSeen && (gm_Settings.command != InputCommand::None) && (gm_Settings.getCommandTime() > 3000))
    {
      longCmdSeen = true;
      
      switch(gm_Settings.command)
      {
        case InputCommand::Num5:
          // reset to bootloader
          system_reset(); // no return
          break;
        
        case InputCommand::PlayPause:
          // toggle display output enable
//          displayOn = !displayOn;
          matrix.fillScreen({0, 0, 0});
          matrix.swapBuffers();
          break;
          
        default:
          break;
      }
    }
    
  }
#endif // Enable_IRRemote

  // tick each module
  LEDMatrix_Tick();
  
  gm_Settings.commandChanged = false;
}


// reboot
void system_reset()
{
  __asm__ volatile("bkpt");

  // wait for reset
  for(;;);
}


