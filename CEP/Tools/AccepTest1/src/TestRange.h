/* Fixed parameters for the Acceptance test   */
/* This is done so we're able to run the test */
/* automatically.                             */

const int min_elements  = 50;
const int max_elements  = 50;

const int min_samples   = 50;
const int max_samples   = 50;

const int port          = 1100;
const int channels      = 1;
const int runs          = 10;

const int targets       = 2;       /* The number of compute nodes per front end */

char* frontend_ip       = "172.24.1.31";
