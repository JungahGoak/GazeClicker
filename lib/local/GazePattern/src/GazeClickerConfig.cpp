#include "GazeClickerConfig.h"
#include <ApplicationServices/ApplicationServices.h>

int screen_width = 0;
int screen_height = 0;

void setScreenSize() {
    CGDirectDisplayID displayID = CGMainDisplayID();  
    screen_width = CGDisplayPixelsWide(displayID);
    screen_height = CGDisplayPixelsHigh(displayID);
}
