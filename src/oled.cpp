/**
 * @file oled.cpp
 *
 * @mainpage OLED display support
 * 
 */

#include "oled.h"

// Test result message
void test_msg(Adafruit_SSD1306 &display, const char *name, const char *result) {
  int16_t x;  // Cursor position
  int16_t y;  // Cursor position
  int16_t x1; // Boundary coordinate
  int16_t y1; // Boundary coordinate
  uint16_t w; // Width of text box
  uint16_t h; // Height of text box

  // Get display size
  int16_t width = display.width();
  int16_t height = display.height();

  // First pass, re-initialize display
  if (result == nullptr) {  
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);      // Default 6x8 character size
    display.setTextColor(SSD1306_WHITE); // Draw white text
    display.cp437(true);         // Use full 256 char 'Code Page 437' font
    display.display();
  }

  // Display test name
  display.getTextBounds(name, 0, 0, &x1, &y1, &w, &h);
  


}


// OLED display message box
void msg_box(Adafruit_SSD1306 &display, const char *msg, justification_t pos) {
  int16_t x;  // Cursor position
  int16_t y;  // Cursor position
  int16_t x1; // Boundary coordinate
  int16_t y1; // Boundary coordinate
  uint16_t w; // Width of text box
  uint16_t h; // Height of text box

  // Get display size
  int16_t width = display.width();
  int16_t height = display.height();

  // Set window parameters
  int16_t window_width = width - 8;
  int16_t window_height = height - 8;
  int16_t window_origin_x = 4;
  int16_t window_origin_y = 4;

  display.clearDisplay();
  display.setTextSize(1);      // Default 6x8 character size
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  // Window frame
  display.drawRect(0,0,width-1,height-1,SSD1306_WHITE);

  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);

  switch (pos) {
    case CENTER:
      x = window_origin_x + (window_width - w)/2;
      y = window_origin_y;
      break;
    case LEFT:
      x = window_origin_x;
      y = window_origin_y;
      break;
    case RIGHT:
      x = window_origin_x + (window_width - w);
      y = window_origin_y;
      break;
    default:
      Serial.printf("Error: Invalid value for display justification\n");
      x = window_origin_x;
      y = window_origin_y;
  }

  display.setCursor(x,y);
  display.println(msg);
  display.display();

  // Serial.printf("'%s' x=%d, y=%d, x1=%d, y1=%d, w=%d, h=%d\n", msg,x,y,x1,y1,w,h);
}