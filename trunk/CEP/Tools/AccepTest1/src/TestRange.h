/* Fixed parameters for the Acceptance test   */
/* This is done so we're able to run the test */
/* automatically.                             */

#ifndef TESTRANGE_H
#define TESTRANGE_H

const int min_elements  = 50;   // equals the #stations
const int max_elements  = 110;

const int min_samples   = 500;   // integration tim of the correlator
const int max_samples   = 1000;

const int port          = 1100;  // startport; port-port+2*targets will be used
const int channels      = 10;    // frequency channels per correlator
const int polarisations = 2;     // number of polarisations (usually two)
const int runs          = 2;    // length of the test run

const int targets       = 8;     // The number of compute nodes per front end

char* frontend_ip       = "192.168.100.254";

#endif
