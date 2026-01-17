// start and stop the ringer a bunch
#include "ring.h"

int main(void) {
  ringer* r = new_ringer("/home/main/prog/pomo/resource/sample.wav");

  for (int i = 0; i < 10; i++) {
    start_ringer(r);
    stop_ringer(r);
  }

  del_ringer(r);
  return 0;
}