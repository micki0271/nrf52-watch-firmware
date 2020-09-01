#ifdef NATIVE

#include <graphics.h>

Graphics::Graphics() : Adafruit_GFX_dummy_display(SCREEN_WIDTH, SCREEN_HEIGHT, 3 /* zoom */) {
}

void Graphics::clearBuffer() {
  fillScreen(0);
}

#endif