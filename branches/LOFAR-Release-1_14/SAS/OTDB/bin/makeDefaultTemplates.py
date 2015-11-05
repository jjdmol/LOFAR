#!/usr/bin/env python
#coding: iso-8859-15
import os,sys,time,pg
from optparse import OptionParser

#
# addIndexedComponent(treeID, keyName, orgTreeID)
#
def addIndexedComponent(treeID, keyName, orgTreeID):
    """
    When parameter belongs to indexed node try to find parent unindexed component in the newtree
    eg. keyName = ObsSW.Observation.Beam[5].angle1
    """
    parts = keyName.rsplit('.', 2)                              # [ObsSW.Observation , Beam[5], angle1 ]
    if len(parts) == 3 and parts[1].endswith(']'):
        nodeName = parts[0]+'.'+parts[1].rstrip('[]0123456789') # ObsSW.Observation.Beam
        dupIndex = parts[1].rstrip(']').split('[')[1]           # 5
        orgNodeID = otdb.query("select * from getVTitem(%s, '%s')" % (treeID, nodeName)).getresult()[0][0]
        newNodeID = otdb.query("select * from dupVTnode(1, %s, %s, '%s')" % (treeID, orgNodeID, dupIndex))
        print "   %s: %-75s added to the tree" % (treeID, parts[0]+'.'+parts[1])
        # copy nrInstances setting from base component from original tree
        (instances, limits) = \
              otdb.query("select instances,limits from getVTitem(%s, '%s')" % (orgTreeID, nodeName)).getresult()[0]
        otdb.query("select * from updateVTnode(1, %s, %s, '%s', '%s')" % (treeID, orgNodeID, instances, limits))
    return newNodeID

#
# removeElement(orgTree, newTree, key)
#
def removeElement(orgTmplID, newTmplID, key, always):
    """
    Removes the given key from the new tree. If the remaining node is empty afterwards it is deleted also.
    """
    parentname = key.rsplit('.',1)[0]
    oldparentid = otdb.query("select nodeid from getVTitem(%s, '%s')" % (orgTmplID, parentname)).getresult()[0][0]
    if oldparentid == None:
        # parent of parameter was removed from old template, safe to delete it in the new template too
        nodeid = otdb.query("select nodeid from getVTitem(%s, '%s')" % (newTmplID, parentname)).getresult()[0][0]
        if nodeid != None:
            otdb.query ("select * from removeVTNode(1, %s, %s)" % (newTmplID, nodeid))
            print "   %s: %-75s removed node deleted" % (newTmplID, parentname)
            # new parent may also be a 'dangling' node, try that.
            removeElement(orgTmplID, newTmplID, parentname, False)
    else:
        if not always: # coming from a recursive call?
            return
        # parent of parameter still exists in old template, remove parameter itself only
        nodeid = otdb.query("select nodeid from getVTitem(%s, '%s')" % (newTmplID, key)).getresult()[0][0]
        if nodeid != None:
            # found item: delete it
            otdb.query ("select * from removeVTleafNode(%s)" % nodeid)
            print "   %s: %-75s parameter deleted" % (newTmplID, key)
        
#
# createNewDefaultTemplate(orgTemplateID, newMasterTemplateID, orgTemplateInfo)
#
def createNewDefaultTemplate(orgTmplID, newMasterTmplID, orgTmplInfo):
    """
    Create a new defaultTemplate based on the 'newMaster' information that has the changed values
    of the original default template.
    """
    # copy tree including description and template name
    print "=> Reconstructing tree %s" % orgTmplID
    newTmplID = otdb.query("select * from copyTree(1, %s)" % newMasterTmplID).getresult()[0][0]
    print "   copy has ID: %s" % newTmplID
    otdb.query("select * from setDescription(1, %s, '%s')" % (newTmplID, orgTmplInfo['description']))
    otdb.query("select * from classify(1, %s, '%s')" % (newTmplID, orgTmplInfo['classification']))
    # set the old default template state to obsolete (1200)
    otdb.query("select * from settreestate(1, %s, '1200')" % (orgTmplID))
    # rename the old template with a '# ' before its original name
    otdb.query("select * from assignTemplateName(1, %s, '#%-.31s')" % (orgTmplID, orgTmplInfo['treeName']))
    otdb.query("select * from assignTemplateName(1, %s, '%s')" % (newTmplID, orgTmplInfo['treeName']))
    otdb.query("select * from assignProcessType (1, %s, '#%-.19s', '#%-.49s', '#%-.29s')" % (orgTmplID, orgTmplInfo['processType'], orgTmplInfo['processSubtype'], orgTmplInfo['strategy']))
    otdb.query("select * from assignProcessType (1, %s, '%s', '%s', '%s')" % (newTmplID, orgTmplInfo['processType'], orgTmplInfo['processSubtype'], orgTmplInfo['strategy']))

    # loop over all values that were changed in the old template
    treeIdentification = "%s%d" % (orgTmplInfo['nodeName'], orgTmplInfo['version'])
    for line in os.popen("comm -23 dfltTree%s MasterTree_%s" % (orgTmplID, treeIdentification)).read().splitlines():
        (key, value) = line.split('=',1)
        # search same item in the new template
        # (nodeid, parentid, paramdefid, name, index, leaf, instances, limits, description) 
        (nodeid, instances, limits) = \
              otdb.query("select nodeid,instances,limits from getVTitem(%s, '%s')" % (newTmplID, key)).getresult()[0]

        # if it doesn't exist, add it when it is a parameter from an indexed node
        if nodeid == None:
            try:
                dummy = addIndexedComponent(newTmplID, key, orgTmplID)
            except:
                print "   %s: %-75s not in the new tree"  % (newTmplID, key)
                continue
            else:
                # no exception: try again to get the parameter in the new template
                (nodeid, instances, limits) = \
                      otdb.query("select nodeid,instances,limits from getVTitem(%s, '%s')" % (newTmplID, key)).getresult()[0]

        # update value if needed
        if limits == value:
            print "   %s: %-75s value is equal"  % (newTmplID, key)
        else:
            print "   %s: %-75s %s --> %s" % (newTmplID, key, limits, value)
            otdb.query("select * from updateVTnode(1, %s, %s, '%s', '%s')" % (newTmplID, nodeid, instances, value))

    # get a list with the removed items
	# -13 -> items uniq in Master --> removed in template OR different value
	# -23 -> items uniq in template --> added to template OR different value
	# comm -23 d1 d2 --> removed in template irt Mastertree.
    command = """comm -13 dfltTree%s MasterTree_%s | cut -d'=' -f1 | sort >diff1 ; 
                 comm -23 dfltTree%s MasterTree_%s | cut -d'=' -f1 | sort >diff2 ; 
                 comm -23 diff1 diff2 ; rm diff1 diff2
              """ % (orgTmplID, treeIdentification, orgTmplID, treeIdentification)
    # loop over the list: when the NODE(=parent) of this parameter was removed in the ORIGINAL default template
    # remove the NODE in the new template otherwise remove the parameter only
    for key in os.popen(command).read().splitlines():
        removeElement(orgTmplID, newTmplID, key, True)

    # Almost ready... when adding Indexed components we might have added to many nodes, 
    # that is: the use might have removed subtrees in the index componenttree.
    # make an parset of the new created tree and delete the nodes(subtrees) that are obsolete
    topNodeID = otdb.query("select nodeid from getTopNode(%s)" % newTmplID).getresult()[0][0]
    createParsetFile(newTmplID, topNodeID, "newTree%s" % newTmplID)
    command = """comm -13 newTree%s dfltTree%s | cut -d'=' -f1 | sort >diff1 ; 
                 comm -23 newTree%s dfltTree%s | cut -d'=' -f1 | sort >diff2 ; 
                 comm -13 diff1 diff2 ; rm diff1 diff2
              """ % (newTmplID, orgTmplID, newTmplID, orgTmplID)
    # loop over the list of nodes that are in the newTree but not in the old tree.
    for key in os.popen(command).read().splitlines():
        print "Removing? ", key,
        # if none indexed node exists in mastertree then it was removed by the user.
        grepcmd = "grep `echo %s | sed 's/\[.*\]//g'` MasterTree_%s 1>/dev/null 2>/dev/null; echo $?" % (key, treeIdentification)
        result = os.popen(grepcmd).read().splitlines()[0]
        if result == "0":
            print " Yes"
            parentname = key.rsplit('.',1)[0]
            nodeid = otdb.query("select nodeid from getVTitem(%s, '%s')" % (newTmplID, parentname)).getresult()[0][0]
            if nodeid != None:
                otdb.query ("select * from removeVTNode(1, %s, %s)" % (newTmplID, nodeid))
                print "   %s: %-75s removed node deleted" % (newTmplID, parentname)
        else:
            print " No"
	
       
#
# createParsetFile(treeID, nodeID, fileName)
#
def createParsetFile(treeID, nodeID, fileName):
    """
    Create a parset file with name fileName from tree treeID starting at nodeID.
    """
    parset = otdb.query("select * from exportTree(%s, %s, %s)" % (1, treeID, nodeID)).getresult()
    print "   Creating parset %s" % fileName
    file = open(fileName, 'w');
    file.write(parset[0][0])
    file.close()


#
# makeMasterTemplateTreeAndParset(treeIdent, topNodeID) : templateID
#
def makeMasterTemplateTreeAndParset(treeIdent, topNodeID):
    """
    Create a template tree in OTDB and save its parset as a master template.
    """
    templateID = otdb.query("select * from instanciateVTtree(1, %s, '4')" % topNodeID).getresult()[0][0]
    otdb.query("select * from setDescription(1, %s, 'MasterTemplate %s')" % (templateID, treeIdent))
    # Create the corresponding parsetFile
    nodeDefID = otdb.query("select * from getTopNode(%s)" % templateID).dictresult()[0]
    createParsetFile(templateID, nodeDefID['nodeid'], "MasterTree_%s" % treeIdent)
    return templateID

#
# MAIN
#
if __name__ == '__main__':
    """
    makeDefaultTemplates reconstructs ALL default templates in OTDB to match a new master-tree.
    Each default templates is compared with the master tree it originates from and the difference are applied
    to a copy of the new master tree.
    """

    parser = OptionParser("Usage: %prog [options]" )
    parser.add_option("-D", "--database",
                      dest="dbName",
                      type="string",
                      default="",
                      help="Name of OTDB database to use")

    parser.add_option("-H", "--host",
                      dest="dbHost",
                      type="string",
                      default="sasdb",
                      help="Hostname of OTDB database")

    parser.add_option("-v","--version",
                      dest="newVersion",
                      type="int",
                      default=0,
                      help="Version number of new master components")

    # parse arguments

    (options, args) = parser.parse_args()

    if not options.dbName:
        print "Provide the name of OTDB database to use!"
        print
        parser.print_help()
        sys.exit(0)

    if not options.newVersion:
        print "Provide the Version number of new master components to use!"
        print
        parser.print_help()
        sys.exit(0)
    
    dbName = options.dbName
    dbHost = options.dbHost
    newVersion = options.newVersion

    # get info from database.py
    #dbName=getDBname()
    #dbHost=getDBhost()

    # calling stored procedures only works from the pg module for some reason.
    otdb = pg.connect(user="postgres", host=dbHost, dbname=dbName)

    # Check if a component LOFAR of this version exists
    versions = [v[0] for v in otdb.query("select version from getVCnodeList('LOFAR', 0, false)").getresult()]
    versions.sort()
    if newVersion not in versions:
        print "ERROR: There is no LOFAR component with version %s.\nAvailable versions: %s" % (newVersion, versions)
        sys.exit(1)

    # Give user escape possibility
    print "About to create new default templates in database %s on host %s. Starting in 5 seconds..." % (dbName, dbHost)
    time.sleep(5)
    
    print "=> Collecting info about default templates..."
    # built dictionary with componentID, nodeID, nodeName, version and treeName of the default templates like:
    # {6171: (412, 2589, 'LOFAR', 40506, 'master template 4.5.6'), 
    #  6121: (203,  426, 'LOFAR', 40000, 'test template')}
    dfltTmplInfo = {}
    dfltTemplateIDs = otdb.query("select * from getDefaultTemplates()").dictresult()
    for dfltTemplate in dfltTemplateIDs:
        state       = otdb.query("select state from getTreeInfo(%s, 'false')" % dfltTemplate['treeid']).getresult()[0][0]
        if state != 1200 :
            treeInfo  = otdb.query("select classification,description from getTreeInfo(%s, 'false')" % dfltTemplate['treeid']).getresult()[0]
            nodeDefID = otdb.query("select * from getTopNode(%s)" % dfltTemplate['treeid']).dictresult()[0]
            nodeInfo  = otdb.query("select * from getVICnodedef(%s)" % nodeDefID['paramdefid']).dictresult()
            dfltTmplInfo[dfltTemplate['treeid']] = \
                    {'componentID'    : nodeDefID['paramdefid'], \
                     'nodeID'         : nodeDefID['nodeid'], \
                     'nodeName'       : nodeDefID['name'], \
                     'version'        : nodeInfo[0]['version'], \
                     'treeName'       : dfltTemplate['name'], \
                     'processType'    : dfltTemplate['processtype'], \
                     'processSubtype' : dfltTemplate['processsubtype'], \
                     'strategy'       : dfltTemplate['strategy'], \
                     'classification' : treeInfo[0], \
                     'description'    : treeInfo[1]}
            print "   DefaultTemplate %s starts at %s (version %d) : %s" % \
                   (dfltTemplate['treeid'], nodeDefID['name'], nodeInfo[0]['version'], dfltTemplate['name'])

    # second step create temporarely parsetfiles from all DefaultTemplates
    print "=> Creating temporarely parsetfiles from the DefaultTemplates..."
    for treeID in dfltTmplInfo:
        createParsetFile(treeID, dfltTmplInfo[treeID]['nodeID'], "dfltTree%s" % treeID)

    # create parsets from the masterTemplates (original template)
    # Note: Since multiple defaultTemplates can have the same Master template remember the
    #       master template parsetfile in masterTmplInfo
    print "=> Creating temporarely master templates in the OTDB and create parsetfiles from them"
    newMasterID = 0
    masterTmplInfo = {}
    for dfltTmpl in dfltTmplInfo.values():
        treeIdentification = "%s%d" % (dfltTmpl['nodeName'], dfltTmpl['version'])
        # if we didn't constructed it before do so now
        if not masterTmplInfo.has_key(treeIdentification):
            masterTmplID = makeMasterTemplateTreeAndParset(treeIdentification, dfltTmpl['componentID'])
            masterTmplInfo[treeIdentification] = masterTmplID
            print "   Master template '%s' version %s = %s" % (dfltTmpl['nodeName'], dfltTmpl['version'], masterTmplID)
            # when this master template is the destination master remember its ID
            if dfltTmpl['version'] == newVersion:
                newMasterID = masterTmplID

    # did we create a template for the new tree-version already
    if newMasterID == 0:
        topComponent = otdb.query("select nodeid from getVCnodelist('LOFAR', %d, false)" % newVersion).getresult()[0]
        newMasterID  = makeMasterTemplateTreeAndParset("LOFAR%d" % newVersion, topComponent)

    # for each old default template make a new template
    print "   TreeID of new master template = %s" % newMasterID
    print "=> Creating new default templates for version %d" % newVersion
    for treeID in dfltTmplInfo:
        createNewDefaultTemplate(treeID, newMasterID, dfltTmplInfo[treeID])

    otdb.close()
    sys.exit(0)
