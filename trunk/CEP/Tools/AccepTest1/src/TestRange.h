/* Fixed parameters for the Acceptance test   */
/* This is done so we're able to run the test */
/* automatically.                             */

const int min_elements  = 12;
const int max_elements  = 12;

const int min_samples   = 23;
const int max_samples   = 24;

const int port          = 6666;
const int channels      = 1;
const int runs          = 15;

const int targets       = 1;       /* The number of compute nodes per front end */

char* frontend_ip       = "127.0.0.1";
