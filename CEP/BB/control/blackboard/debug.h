#include <iostream>
#include <string>
#include <typeinfo>

#define DEBUG(x) if(debug) std::cout << " rank(" << ::rank << "): " << x << std::endl;

    //util vars
extern int rank;
extern bool debug;
