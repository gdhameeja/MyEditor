/*** includes ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>

/*** defines ***/
// mimic how Ctrl works for ascii
// set upper 3 bits to 0
#define CTRL_KEY(k) ((k) &0x1f)

/*** data ***/

// original termial attributes
// needs to be saved in order to set the terminal's original attrs
// when the program exits
struct termios orig_termios;

/*** terminal ***/

void die(const char *s) {
  write(STDOUT_FILENO, "\x1b[2J", 4);
  write(STDOUT_FILENO, "\x1b[H", 3);
  
  // got perror and exit from errno.h
  // exit code 1 states (non-zero) something went wrong
  // perror prints the error
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

char editorReadKey() {
  int nread = 0;
  char c;
  while ((nread == read(STDIN_FILENO, &c, 1))) {
    if (nread == -1 && errno != EAGAIN)
      die("read");
  }
  return c;
}

/*** input ***/
void editorProcessKeyPress() {
  char c = editorReadKey();
  switch(c) {
  case CTRL_KEY('q'):
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    
    exit(0);
    break;
  }
}

/*** output ***/

void editorDrawRows() {
  int y;
  for (y = 0; y < 24; y++) {
    write(STDOUT_FILENO, "~\r\n", 3);
  }
}


/*
 * Escape sequences start with escape char (27 in ascii) 
 * followed by [ character.
 * Escape sequences allow terminal to do formatting tasks such as 
 * coloring the text, moving cursor around, clearing parts of screen
 */
void editorRefreshScreen() {
  // write 4 bytes to terminal
  // first byte is x1b (escape character)
  // other three bytes are [2J
  // J command is erase in display (clear the screen)
  write(STDOUT_FILENO, "\x1b[2J", 4);

  // this escape seq is 3 bytes long
  // uses H command to position the cursor
  write(STDOUT_FILENO, "\x1b[H", 3);

  editorDrawRows();

  write(STDOUT_FILENO, "\x1b[H", 3);
}


/*** init ***/

int main() {
  enableRawMode();

  while (1) {
    editorRefreshScreen();
    editorProcessKeyPress();
  }
  return 0;
}
