#include "VisDM/VisDM.h"
#include "VisDM/TID-VisDM.h"
    
// make sure registry is puled in
static int dum = aidRegistry_VisDM();    
    
// init the debug context
InitDebugContext(VisDMDebugContext,"VisDM");
