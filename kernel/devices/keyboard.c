#include "types.h"
#include "keyboard.h"

#define INPUT_BUF 128
struct {
  char buf[INPUT_BUF];
  u32 r;  // Read index
  u32 w;  // Write index
  u32 e;  // Edit index
} input;

void keyboard_interrupt_handler() {

}
