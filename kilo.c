#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

/* Turning echo off (terminal raw mode)
 * don't print to terminal what I type
 * terminal attrs read into termios struct
 * by tcgetattr()
 * modify the attributes and re-apply then using
 * tcsetattr(). TCSAFLUSH tells when to apply the change
 */

struct termios orig_termios;

void disableRawMode() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void enableRawMode() {
  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  // ECHO is a bitflag defined as 0100.
  // ~ (NOT) reverses the bits to 1011
  raw.c_lflag &= ~(ECHO);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRawMode();
  
  char c;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    if (c == 'q') {
      break;
    }
  }
  return 0;
}
  
