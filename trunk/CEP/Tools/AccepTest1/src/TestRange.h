/* Fixed parameters for the Acceptance test   */
/* This is done so we're able to run the test */
/* automatically.                             */

#ifndef TESTRANGE_H
#define TESTRANGE_H

const int min_elements  = 50;   // equals the #stations
const int max_elements  = 100;
const int stp_elements  = 1;

const int min_samples   = 500;   // integration time of the correlator
const int max_samples   = 1000;
const int stp_samples   = 5;

const int port          = 1100;  // startport; [port,port+2*targets] will be used
const int channels      = 1;    // frequency channels per correlator
const int polarisations = 2;     // number of polarisations (usually two)
const int runs          = 20;     // length of the test run

const int targets       = 8;     // The number of compute nodes per front end

char* frontend_ip       = "192.168.100.254";
//char* frontend_ip       = "172.24.1.32";  // bgfe02
//char* frontend_ip       = "172.30.3.2"; // Rochester frontend

#endif
