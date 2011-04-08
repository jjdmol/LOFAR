#!/usr/bin/env python
#coding: iso-8859-15
import os,sys,pg
from database import *

# get info from database.py
dbName=getDBname()
dbHost=getDBhost()

# calling stored procedures only works from the pg module for some reason.
db2 = pg.connect(user="postgres", host=dbHost, dbname=dbName)

#
# addIndexedComponent(treeID, keyName)
#
def addIndexedComponent(treeID, keyName):
    # When parameter belongs to indexed node try to find parent unindexed component in the newtree
    # eg. keyName = ObsSW.Observation.Beam[5].angle1
    parts = keyName.rsplit('.', 2)
    if len(parts) == 3 and parts[1].endswith(']'):
        nodeName = parts[0]+'.'+parts[1].rstrip('[]0123456789') # ObsSW.Observation.Beam
        dupIndex = parts[1].rstrip(']').split('[')[1]           # 5
        orgNodeID = db2.query("select * from getVTitem(%s, '%s')" % (treeID, nodeName)).getresult()[0][0]
        newNodeID = db2.query("select * from dupVTnode(1, %s, %s, '%s')" % (treeID, orgNodeID, dupIndex))
        print "   %s: %-60s added to the tree" % (treeID, parts[0]+'.'+parts[1])
    return newNodeID


#
# createNewDefaultTemplate(orgTemplateID, newMasterTemplateID, orgTemplateInfo)
#
def createNewDefaultTemplate(orgTmplID, newMasterTmplID, orgTmplInfo):
    """
    Create a new defaultTemplate based on the 'newMaster' information that has the changed values
    of the original default template.
    """
    print "=> Reconstructing tree %s" % orgTmplID
    newTmplID = db2.query("select * from copyTree(1, %s)" % newMasterTmplID).getresult()[0][0]
    print "   copy has ID: %s" % newTmplID
    db2.query("select * from setDescription(1, %s, '%s')" % (newTmplID, orgTmplInfo['description']))
    db2.query("select * from assignTemplateName(1, %s, '_new_ %s')" % (newTmplID, orgTmplInfo['treeName']))

    # loop over all values there were changed in the old template
    treeIdentification = "%s%d" % (orgTmplInfo['nodeName'], orgTmplInfo['version'])
    for line in os.popen("comm -23 dfltTree%s MasterTree_%s" % (orgTmplID, treeIdentification)).read().splitlines():
        (key, value) = line.split('=',1)
        # search same item in the new template
        # (nodeid, parentid, paramdefid, name, index, leaf, instances, limits, description) 
        (nodeid, _, _, _, _, _, instances, limits, _) = \
              db2.query("select * from getVTitem(%s, '%s')" % (newTmplID, key)).getresult()[0]

        # if it doesn't exist, add it when it is a parameter from an indexed node
        if nodeid == None:
            try:
                newNodeID = addIndexedComponent(newTmplID, key)
            except:
                print "   %s: %-60s not in the new tree"  % (newTmplID, key)
                continue
            else:
                # no exception: try again to get the parameter in the new template
                (nodeid, _, _, _, _, _, instances, limits, _) = \
                      db2.query("select * from getVTitem(%s, '%s')" % (newTmplID, key)).getresult()[0]

        if limits == value:
            print "   %s: %-60s value is equal"  % (newTmplID, key)
        else:
            print "   %s: %-60s %s --> %s" % (newTmplID, key, limits, value)
            db2.query("select * from updateVTnode(1, %s, %s, '%s', '%s')" % (newTmplID, nodeid, instances, value))


#
# createParsetFile(treeID, nodeID, fileName)
#
def createParsetFile(treeID, nodeID, fileName):
    """
    Create a parset file with name fileName from tree treeID starting at nodeID.
    """
    parset = db2.query("select * from exportTree(%s, %s, %s)" % (1, treeID, nodeID)).getresult()
    print "   Creating parset %s" % fileName
    file = open(fileName, 'w');
    file.write(parset[0][0])
    file.close()


#
# MAIN
#
if __name__ == '__main__':

    # check syntax of invocation
    # Expected syntax: load_measurement stationname objecttypes datafile
    #
    #if (len(sys.argv) != 2):
    #    print "Syntax: %s datafile" % sys.argv[0]
    #    sys.exit(1)
    newVersion = 40506
    newMasterID = 0
    
    # Check if a component LOFAR of this version exists
    # TODO
    
    print "=> Collecting info about default templates..."
    # built dictionary with componentID, nodeID, nodeName, version and treeName of the default templates like:
    # {6171: (412, 2589, 'LOFAR', 40506, 'master template 4.5.6'), 
    #  6121: (203,  426, 'LOFAR', 40000, 'test template')}
    dfltTmplInfo = {}
    dfltTemplateIDs = db2.query("select * from getDefaultTemplates()").dictresult()
    for dfltTemplate in dfltTemplateIDs:
        (_, _, _, _, _, _, _, _, _, _, _, description) = \
              db2.query("select * from getTreeInfo(%s, 'false')" % dfltTemplate['treeid']).getresult()[0]
        nodeDefID = db2.query("select * from getTopNode(%s)" % dfltTemplate['treeid']).dictresult()[0]
        nodeInfo  = db2.query("select * from getVICnodedef(%s)" % nodeDefID['paramdefid']).dictresult()
        dfltTmplInfo[dfltTemplate['treeid']] = \
                {'componentID' : nodeDefID['paramdefid'], \
                 'nodeID'      : nodeDefID['nodeid'], \
                 'nodeName'    : nodeDefID['name'], \
                 'version'     : nodeInfo[0]['version'], \
                 'treeName'    : dfltTemplate['name'], \
                 'description' : description}
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
    masterTmplInfo = {}
    for dfltTmpl in dfltTmplInfo.values():
        treeIdentification = "%s%d" % (dfltTmpl['nodeName'], dfltTmpl['version'])
        # if we didn't constructed it before do so now
        if not masterTmplInfo.has_key(treeIdentification):
            masterTmplID = db2.query("select * from instanciateVTtree(1, %s, '4')" % dfltTmpl['componentID']).getresult()[0][0]
            db2.query("select * from setDescription(1, %s, 'MasterTemplate %s')" % (masterTmplID, treeIdentification))
            masterTmplInfo[treeIdentification] = masterTmplID
            print "   Master template '%s' version %s = %s" % (dfltTmpl['nodeName'], dfltTmpl['version'], masterTmplID)
            # when this master template is the destination master remember its ID
            if dfltTmpl['version'] == newVersion:
                newMasterID = masterTmplID
            # Create the corresponding parsetFile
            nodeDefID = db2.query("select * from getTopNode(%s)" % masterTmplID).dictresult()[0]
            createParsetFile(masterTmplID, nodeDefID['nodeid'], "MasterTree_%s" % treeIdentification)

#    print masterTmplInfo 

    # for each old default template make a new template
    print "   TreeID of new master template = %s" % newMasterID
    print "=> Creating new default templates for version %d" % newVersion
    for treeID in dfltTmplInfo:
        createNewDefaultTemplate(treeID, newMasterID, dfltTmplInfo[treeID])

    db2.close()
    sys.exit(0)
