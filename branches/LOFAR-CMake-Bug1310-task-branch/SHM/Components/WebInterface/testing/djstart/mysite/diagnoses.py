def diagnose_LOFAR():
    #here you have to put a lydia call
    return("N/A")

def diagnose_site(station):
    #
    #
    return("N/A")

def diagnose_system(system = 'LOFAR', instance = 'LOFAR'):
    # this is where you have to put the lydia calls
    # look closely at Sidney's component scheme.
    #
    #This cries out for OOifying.
    #
    if system.lower() == 'lofar':
        return("N/A")
    if system.lower() == "site":
        return("N/A")
    if system.lower() == "antenna":
        return("N/A")
    else:
        return("N/A")
    
