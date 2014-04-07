#!/usr/bin/env python
#coding: iso-8859-15
import os,sys,time,pg
from database import *


#
# copyVICtree(treeID)
#
def copyVICtree(treeID):
    """
    Copy the VIC tree by first copying the nodes and then the parameters.
    """
    # Unfortunately there are no suitable stored procedures for the functionality we need.

    # First copy all nodes in order of original creation so the parentID can be set in the new DB
    fromNodeList = fromDB.query("select * from VIChierarchy where treeid=%d and leaf='false' order by nodeid" % treeID).dictresult()
    print "Found %d nodes in the tree" % len(fromNodeList)
    newNodeIDmap = {}
    newNodeIDmap[0] = 0
    for node in fromNodeList:
        # copy node
        newNodeID = toDB.query("select * from nextval('VIChierarchID')").getresult()[0][0]
        newNodeIDmap[node['nodeid']] = newNodeID
        if node['value'] == None:
            node['value'] = ''
        query = "insert into VIChierarchy(treeid,nodeid,parentID,paramRefID,name,index,leaf,value) \
          values (%d,%d,%d,%d,'%s',%d,'%s','%s')" %  \
          (treeID, newNodeID, newNodeIDmap[node['parentid']], compIDmap[node['paramrefid']], \
           node['name'], node['index'], node['leaf'], node['value'])
        dummy = toDB.query(query)
        print "%s = %s (id:%d -> %d)" % (node['name'], node['value'][0:30], node['nodeid'], newNodeID)

    # Finally copy the parameters
    fromParList = fromDB.query("select * from VIChierarchy where treeid=%d and leaf='true' order by nodeid" % treeID).dictresult()
    print "Found %d parameters in the tree" % len(fromParList)
    for par in fromParList:
        # copy parameter
        newNodeID = toDB.query("select * from nextval('VIChierarchID')").getresult()[0][0]
        newNodeIDmap[par['nodeid']] = newNodeID
        if par['value'] == None:
            par['value'] = ''
        query = "insert into VIChierarchy(treeid,nodeid,parentID,paramRefID,name,index,leaf,value) \
          values (%d,%d,%d,%d,'%s',%d,'%s','%s')" %  \
          (treeID, newNodeID, newNodeIDmap[par['parentid']], parIDmap[par['paramrefid']], \
           par['name'], par['index'], par['leaf'], par['value'])
        dummy = toDB.query(query)
        print "%s = %s (id:%d -> %d)" % (par['name'], par['value'][0:30], par['nodeid'], newNodeID)


       
#
# copyTreeMetaData(treeID, campID)
#
def copyTreeMetaData(treeID, campID):
    """
    Copy the metadata of the tree.
    """
    # First create the tree. Unfortunately there are no suitable stored procedures to do this in a nice way...
	# TODO: Funerable in the current implementation are groupid and pt+pst+strategy
    #       the name is always left empty so we cannot accidently create a default template
    fromTree = fromDB.query("select * from otdbtree where treeid=%d" % treeID).dictresult()[0]
    query = "insert into otdbtree(treeid,momid,originid,classif,treetype,state,creator, \
      campaign,starttime,stoptime,owner,description,groupid,processtype,processsubtype,strategy) values \
      (%d,%d,0,%d::int2,%d::int2,%d::int2,%d,%d::int2,'%s','%s',%d,'%s',%d,'%s','%s','%s')" %  \
      (treeID, fromTree['momid'], fromTree['classif'], fromTree['treetype'], fromTree['state'],  \
      fromTree['creator'], campID, fromTree['starttime'], fromTree['stoptime'],  \
      fromTree['owner'], fromTree['description'], fromTree['groupid'],  \
      fromTree['processtype'], fromTree['processsubtype'], fromTree['strategy'])
    result = toDB.query(query)  # void function
    print "Created metadata for tree %d" % treeID
    return


#
# checkCampaign(campaignName) : newCampaignID
#
def	checkCampaign(campaignName):
    """
    Make sure this campaign exists in the database
    """
    # get campaign info
    fromCamp = fromDB.query("select * from getCampaign('%s')" % campaignName).dictresult()[0]
    # note: we don't want to overwrite the campaign if it already exists...
    try:
        toCamp = toDB.query("select * from getCampaign('%s')" % campaignName).dictresult()
        # it exists, return ID
        print "Campaign '%s' already exists (id=%d)" % (fromCamp['name'], toCamp[0]['id'])
        return toCamp[0]['id']
    except:
        newID = toDB.query("select * from saveCampaign(0,%s,%s,%s,%s,%s)" %
          (fromCamp['name'],fromCamp['title'],fromCamp['pi'],fromCamp['co_i'],fromCamp['contact'])).getresult()[0][0]
        print "Campaign '%s' copied (id=%d) => %d" % (fromCamp['name'], fromCamp['id'], newID)
        return newID

#
# copyOrMapComponents(version)
#
def copyOrMapComponents(version):
    """
    Copy the component of given version from the fromDB to the toDB. If they already exist they are updated.
    """
    # TODO: check the 'unit' table!

    # get all nodes with this version
    nodeList = fromDB.query("select * from getVCNodeList('%%', %d, 'false')" % version).dictresult()
    print "Found %d components to" % len(nodeList)
    for comp in nodeList:
        newNodeID = toDB.query("select * from saveVCnode(1, %d, '%s', %d, 3::int2, '%s', '%s')" % 
                (comp['nodeid'], comp['name'], version, comp['constraints'], comp['description'])).getresult()[0][0]
        compIDmap[comp['nodeid']] = newNodeID
        print "%s (id=%d) => id=%d" % (comp['name'], comp['nodeid'], newNodeID)

    # copy the parameters also
    print "Processing parameters"
    for comp in nodeList:
        parList = fromDB.query("select * from getVCparams(%d)" % comp['nodeid']).dictresult()
        for par in parList:
            newParID = toDB.query(
              "select * from saveVICparamDef(1, %d, '%s', %d::int2, %d::int2, %d::int2, %d::int2, '%s', '%s', '%s')" % 
              (compIDmap[comp['nodeid']], par['name'], par['par_type'], par['unit'], par['pruning'], par['valmoment'],
               par['rtmod'], par['limits'], par['description'])).getresult()[0][0]
            parIDmap[par['paramid']] = newParID
            print "%s.%s (id=%d) => id=%d" % (comp['name'], par['name'], par['paramid'], newParID)
    print "Found %d parameters" % len(parIDmap)

#
# MAIN
#
if __name__ == '__main__':
    """
    copyTree copies 1 tree from 1 database to another. The tree may be a template or a VIC tree.
    Ideally the components of the right version are already in the database but if they are not the components
	are copied also. Idem with campaigns, users and units.
    """

    # check syntax of invocation
    # Expected syntax: copyTree momID fromDB toDB
    if (len(sys.argv) != 4):
        print "Syntax: %s MoMID fromDB toDB" % sys.argv[0]
        sys.exit(1)
    momID      = int(sys.argv[1])
    fromDBname = sys.argv[2]
    toDBname   = sys.argv[3]
    
    # calling stored procedures only works from the pg module for some reason.
    fromDB = pg.connect(user="postgres", host="localhost", dbname=fromDBname)
    print "Connected to database", fromDBname
    toDB   = pg.connect(user="postgres", host="localhost", dbname=toDBname)
    print "Connected to database", toDBname

    # Check for tree-existance in both databases.
    fromDBtree = fromDB.query("select * from gettreelist(0::int2,3::int2,0,'','','') where momid=%d" % momID).dictresult()
    toDBtree = toDB.query("select * from gettreelist(0::int2,3::int2,0,'','','') where momid=%d" % momID).dictresult()
    if len(fromDBtree) == 0:
        print "Tree with MoMId %d not found in database %s" % (momID, fromDBname)
        sys.exit(1)
    if len(toDBtree) != 0:
        print "Tree with MoMId %d already exists in database %s" % (momID, toDBname)
        # TODO: implement -f option to copy the tree under a different number.
        sys.exit(1)
    if fromDBtree[0]['type'] == 10:	# PIC tree?
        print "PIC trees cannot be copied"
        sys.exit(1)

    # What's the version of this tree?
    treeID    = fromDBtree[0]['treeid']
    nodeDefID = fromDB.query("select * from getTopNode(%d)" % treeID).dictresult()[0]
    nodeInfo  = fromDB.query("select * from getVICnodedef(%s)" % nodeDefID['paramdefid']).dictresult()[0]
    version   = nodeInfo['version']
    print "Tree %d was built with components of version %d" % (treeID, version)

    # Does the new DB contain these components?
    compIDmap = {}		# mapping componentID's map[oldID]=newID
    parIDmap  = {}		# mapping parameterID's map[oldID]=newID
    try:
        newDBnode = toDB.query("select * from getVICnodedef('%s', %d, 3::int2)" % (nodeInfo['name'], version)).dictresult()
        print "No need to copy the components"
    except:
        print "Need to copy the components to %s also" % toDBname
    copyOrMapComponents(version)
    # components are now in the new database for sure and the node and par ID's are in the map dicts.

	# make sure the campaign exists also
    newCampaignID = checkCampaign(fromDBtree[0]['campaign'])

    # TODO: check user table (owner of tree must exist)

    # copy the trees metadata first
    copyTreeMetaData(treeID, newCampaignID)

    if fromDBtree[0]['type'] == 20:	# template?
        print "Copy of template trees is not supported yet"
        sys.exit(2)
    else:	# type must be 30 (VIC tree)
        copyVICtree(treeID)

    # TODO: copy treeState table also

    # TODO: copy vickvt table also

    toDB.close()
    fromDB.close()
    sys.exit(0)
