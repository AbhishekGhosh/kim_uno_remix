/*
 * display
 *
 *   This drives the LEDs for common anode, cathode, and size configuration
 *   It also handles text rendering
 */

#ifndef __DISPLAY_H__
#define __DISPLAY_H__
extern "C" {
  
extern char shiftKey;   // is the keypad shift key in effect?
extern char kimHex[6];  // LED display

// displayText
//  pass it in one of these values, and the number of ms to display (eg 500)
//  for text display

#define kDt_blank   (0)
#define kDt_SST_ON  (1)
#define kDt_SST_OFF (2)
#define kDt_EE_RW   (3)
#define kDt_EE_RO   (4)
#define kDt_Uno     (5)
#define kDt_Scott   (6)
#define kDt_Oscar   (7)

void displayText( int which, long timeMillis );
void disableLEDs();
void driveLEDs();

}
#endif
