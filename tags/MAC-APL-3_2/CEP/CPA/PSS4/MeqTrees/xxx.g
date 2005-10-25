# file: ../MeqTrees/MasterScript.g

#---------------------------------------------------------

# pragma include once
print '\n\n\n\n\n\n=================================================';
print 'include MeqNode.g    h10sep/d11sep/h14sep/d15sep2003';

include 'unset.g';
# include 'genericClosureFunctions.g';

include 'MeqNode.g';          # also has MeqName()
include 'MeqTree.g';          # also has MeqName()


############################################################################
# Example of a MeqTree master-script
############################################################################

MasterScript := function () {
    include 'inspect.g';
    print '\n\n****************************************************';
    print '** MasterScript:';

    # Create global object(s):
    print '\n** Create global objects';
    global msv, mqn, mqe, mqs, mqp, mqt;
    print '\n',msv := dummyMeqServer('msv');
    # print '\n',mqn := MeqNode('mqn');
    print ' ';

    # Inspect global object(s):
    # msv.inspect('msv');
    # mqn.inspect('mqn');
    # mqp.inspect('mqp');
    # mqe.inspect('mqe');

    # mqn.agent -> test_event(mqn.type());
    # msv.agent -> test_event(msv.type());
    # print 'msv.command() ->',msv.command('<command>');

    # print 'mqn.defrec() ->',mqn.defrec();

    return T;
}
# Execute:
MasterScript();

############################################################################
