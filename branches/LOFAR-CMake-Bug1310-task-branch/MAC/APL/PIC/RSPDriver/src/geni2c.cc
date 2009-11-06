#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int /*argc*/, char** /*argv*/)
{
  FILE* protocol = 0;
  FILE* result = 0;
  char byte[3][3];

  // null terminate byte strings
  for (int i = 0; i < 3; i++) {
    byte[i][2] = '\0';
  }

  if (0 == (protocol = fopen("protocol.list", "w"))) {
    fprintf(stderr, "error: Failed to open 'protocol.list' output file.\n");
    exit(EXIT_FAILURE);
  }

  if (0 == (result = fopen("result.list", "w"))) {
    fprintf(stderr, "error: Failed to open 'result.list' output file.\n");
    exit(EXIT_FAILURE);
  }

  int n = 1;
  while (3 == scanf("%2c%2c%2c\n", byte[0], byte[1], byte[2])) {

    fprintf(protocol, "0x06, ");
    fprintf(protocol, "0x%s >> 1, ", byte[0]);
    fprintf(protocol, "0x%s, ", byte[1]);
    fprintf(protocol, "0x%s, ", byte[2]);
    fprintf(protocol, "0x07, ", n);
    fprintf(protocol, "0x%s >> 1, ", byte[0]);
    fprintf(protocol, "0x%s, ", byte[1]);
    if (0 == n % 2) fprintf(protocol, "\n");

    fprintf(result, "0x00, ", n);
    fprintf(result, "0x%s, ", byte[2]);
    fprintf(result, "0x00, ", n);
    if (0 == n % 8) fprintf(result, "\n");

    n++;
  }
}
