#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

/*
     * 'open_port()' - Open serial port 1.
     *
     * Returns the file descriptor on success or -1 on error.
     */

int
open_port(void)
{
  int fd; /* File descriptor for the port */
  

  fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
    {
      /*
       * Could not open the port.
       */

      perror("open_port: Unable to open /dev/ttyS0 - ");
    }
  else{
    //    fcntl(fd, F_SETFL, 0);
    fcntl(fd, F_SETFL, FNDELAY);
  }
  return (fd);
}


int main(int argc, char *argv[])
{
  char buffer[1024];  /* Input buffer */
  char *bufptr;      /* Current char in buffer */
  char sendBuf[255]; /* Output buffer*/
  int bufLen;        /* Output buffer content size*/
  int totalChars;    /* Total number of chars read from tty*/


  if (argc == 2)
    {
      sprintf(sendBuf,"%s\r", *++argv);
    }

  bufLen = strlen(sendBuf);
  int ttyFd;
  ttyFd = open_port();
  if (ttyFd == -1)
    return -1;

  int n;
  n = write(ttyFd, sendBuf, bufLen);
  if (n < 0)
    fputs("write() of 4 bytes failed!\n", stderr);


  /* read characters into our string buffer until we get a CR or NL */
  bufptr = buffer;
  long int count;
  int  nbytes;       /* Number of bytes read */
  while (count < 1000000)
    {
      nbytes = read(ttyFd, bufptr, buffer + sizeof(buffer) - bufptr - 1);
      if (nbytes != -1)
	{
	  totalChars += nbytes;
	  bufptr += nbytes;
	  if (bufptr[-1] == '\n' || bufptr[-1] == '\r')
	    count = 10000000;
	}
      else
	count += 1;

    }
  
  /* nul terminate the string */
  *bufptr = '\0';
  printf("%s", buffer);

  close(ttyFd);
  return totalChars;
}
