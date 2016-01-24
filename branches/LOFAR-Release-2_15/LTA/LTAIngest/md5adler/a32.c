# include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include "md5.h"

unsigned int adlercount = 1;

int main(int argc, char *argv[]) {
    FILE *fp;
    char res[16];

    struct pollfd fds;
    fds.fd = 0; /* this is STDIN */
    fds.events = POLLIN;
    int inputViaStdIn = poll(&fds, 1, 0);

    if(inputViaStdIn) {
        fp = stdin;
    }
    else {
        fp=fopen(argv[1],"rb");
        if (fp==(FILE *)NULL) {
            fprintf (stderr,"failed: can't open %s\n", argv[1]);
            exit(-1);
        }
    }

    parse_stream(fp,res);
    fprintf(stdout, "ADLER32 %x\n",adlercount);

    if(!inputViaStdIn)
        fclose(fp);

    exit(0);
}


void process_block(void *buffer, size_t size) {
  adlercount=adler32(adlercount,buffer,size);
}

void process_bytes (void *buffer, size_t sum) {
  adlercount=adler32(adlercount,buffer, sum);
}

int parse_stream (FILE *stream, void *resblock)
{
  char buffer[BLOCKSIZE + 72];
  size_t sum;


  /* Iterate over full file contents.  */
  while (1)
    {
      /* We read the file in blocks of BLOCKSIZE bytes.  One call of the
         computation function processes the whole buffer so that with the
         next round of the loop another block can be read.  */
      size_t n;
      sum = 0;

      /* Read block.  Take care for partial reads.  */
      while (1)
	{
	  n = fread (buffer + sum, 1, BLOCKSIZE - sum, stream);

	  sum += n;

	  if (sum == BLOCKSIZE)
	    break;

	  if (n == 0)
	    {
	      /* Check for the error flag IFF N == 0, so that we don't
	         exit the loop after a partial read due to e.g., EAGAIN
	         or EWOULDBLOCK.  */
	      if (ferror (stream))
		return 1;
	      goto process_partial_block;
	    }

	  /* We've read at least one byte, so ignore errors.  But always
	     check for EOF, since feof may be true even though N > 0.
	     Otherwise, we could end up calling fread after EOF.  */
	  if (feof (stream))
	    goto process_partial_block;
	}

      /* Process buffer with BLOCKSIZE bytes.  Note that
         BLOCKSIZE % 64 == 0
       */
      process_block (buffer, BLOCKSIZE);
    }

process_partial_block:

  /* Process any remaining bytes.  */
  if (sum > 0)
    process_bytes (buffer, sum);

  return 0;
}

