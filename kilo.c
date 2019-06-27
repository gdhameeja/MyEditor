#include <ctype.h>
#include <stdio.h>
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
  // Ctrl-S stops output until Ctrl-Q is pressed.
  // turn off this feature using IXON
  raw.c_iflag &= ~(IXON);
  // ECHO is a bitflag defined as 0100.
  // ~ (NOT) reverses the bits to 1011
  // ICANON makes the program read byte-by-byte (Canonical processing)
  // instead of reading input every-time we press enter
  // ISIG disables Ctrl-C and Ctrl-Z (stop/suspend a process)
  raw.c_lflag &= ~(ECHO | ICANON | ISIG);
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int main() {
  enableRawMode();

  char c;
  while (read(STDIN_FILENO, &c, 1) == 1) {
    // exits immediately as ICANON flag is set
    if (c == 'q') {
      break;
    } else if (iscntrl(c)) {
      // iscntrl tells is a char is control char (ascii 0-31)
      // control chars are non-printable
      printf("%d\n", c);
    } else {
      printf("%d ('%c')\n", c, c);
    }
  }
  return 0;
}
