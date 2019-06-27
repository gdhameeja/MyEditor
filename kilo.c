/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

// original termial attributes
// needs to be saved in order to set the terminal's original attrs
// when the program exits

/*** data ***/

struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
  perror(s);
  exit(1);
}

void disableRawMode() {
  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
    die("tcsetattr");
}

/* Turning echo off (terminal raw mode)
 * don't print to terminal what I type
 * terminal attrs read into termios struct
 * by tcgetattr()
 * modify the attributes and re-apply then using
 * tcsetattr(). TCSAFLUSH tells when to apply the change
 */
void enableRawMode() {
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
    die("tcgetattr");
  atexit(disableRawMode);

  struct termios raw = orig_termios;
  
  // Ctrl-S stops output until Ctrl-Q is pressed.
  // turn off this feature using IXON
  // Ctrl-M returns 10 when it should return 10. (Enter also returns 10)
  // turn this off using ICRNL
  // BRKINT send a SIGINT (like the one Ctrl-C) does whenever a break condition occurs
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  
  // OPOST turns off all output processing features
  // \n is converted to \r\n (terminal needs both); refer carriage return
  
  raw.c_oflag &= ~(OPOST);
  // ECHO is a bitflag defined as 0100.
  // ~ (NOT) reverses the bits to 1011
  // ICANON makes the program read byte-by-byte (Canonical processing)
  // instead of reading input every-time we press enter (line-by-line)
  // ISIG disables Ctrl-C and Ctrl-Z (stop/suspend a process)
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);

  raw.c_cflag |= ~(CS8);

  if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    die("tcsetattr");
}


/*** init ***/

int main() {
  enableRawMode();
  
  while (1) {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN)
      die("read");
    // exits immediately as ICANON flag is set
    if (c == 'q') {
      break;
    } else if (iscntrl(c)) {
      // iscntrl tells is a char is control char (ascii 0-31)
      // control chars are non-printable
      printf("%d\r\n", c);
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
  }
  return 0;
}
