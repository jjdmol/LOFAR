#!/usr/bin/env python
#coding: iso-8859-15
import os,sys,time,pg
from optparse import OptionParser


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
        print "%s = %s (id:%d -> %d)" % (node['name'], node['value'][0:30], node['nodeid'], newNodeID)
        query = "insert into VIChierarchy(treeid,nodeid,parentID,paramRefID,name,index,leaf,value) \
          values (%d,%d,%d,%d,'%s',%d,'%s','%s')" %  \
          (treeID, newNodeID, newNodeIDmap[node['parentid']], compIDmap[node['paramrefid']], \
           node['name'], node['index'], node['leaf'], node['value'])
        dummy = toDB.query(query)

    # Finally copy the parameters
    fromParList = fromDB.query("select * from VIChierarchy where treeid=%d and leaf='true' order by nodeid" % treeID).dictresult()
    print "Found %d parameters in the tree" % len(fromParList)
    for par in fromParList:
        # copy parameter
        newNodeID = toDB.query("select * from nextval('VIChierarchID')").getresult()[0][0]
        newNodeIDmap[par['nodeid']] = newNodeID
        if par['value'] == None:
            par['value'] = ''
        print "%s = %s (id:%d -> %d)" % (par['name'], par['value'][0:30], par['nodeid'], newNodeID)
        query = "insert into VIChierarchy(treeid,nodeid,parentID,paramRefID,name,index,leaf,value) \
          values (%d,%d,%d,%d,'%s',%d,'%s','%s')" %  \
          (treeID, newNodeID, newNodeIDmap[par['parentid']], parIDmap[par['paramrefid']], \
           par['name'], par['index'], par['leaf'], par['value'])
        dummy = toDB.query(query)

#
# copyTemplateTree(treeID)
#
def copyTemplateTree(treeID):
    """
    Copy the Template tree by first copying the nodes and then the parameters.
    """
    # Unfortunately there are no suitable stored procedures for the functionality we need.

    # First copy all nodes in order of original creation so the parentID can be set in the new DB
    fromNodeList = fromDB.query("select * from VICtemplate where treeid=%d and leaf='false' order by nodeid" % treeID).dictresult()
    print "Found %d nodes in the tree" % len(fromNodeList)
    newNodeIDmap = {}
    newNodeIDmap[0] = 0
    for node in fromNodeList:
        # copy node
        newNodeID = toDB.query("select * from nextval('VICtemplateID')").getresult()[0][0]
        newNodeIDmap[node['nodeid']] = newNodeID
        if node['limits'] == None:
            node['limits'] = ''
        print "%s = %s (id:%d -> %d)" % (node['name'], node['limits'][0:30], node['nodeid'], newNodeID)
        query = "insert into VICtemplate(treeid,nodeid,parentID,originID,name,index,leaf,instances,limits) \
          values (%d,%d,%d,%d,'%s',%d,'%s',%d,'%s')" %  \
          (treeID, newNodeID, newNodeIDmap[node['parentid']], compIDmap[node['originid']], \
           node['name'], node['index'], node['leaf'], node['instances'], node['limits'])
        dummy = toDB.query(query)

    # Finally copy the parameters
    fromParList = fromDB.query("select * from VICtemplate where treeid=%d and leaf='true' order by nodeid" % treeID).dictresult()
    print "Found %d parameters in the tree" % len(fromParList)
    for par in fromParList:
        # copy parameter
        newNodeID = toDB.query("select * from nextval('VICtemplateID')").getresult()[0][0]
        newNodeIDmap[par['nodeid']] = newNodeID
        if par['limits'] == None:
            par['limits'] = ''
        print "%s = %s (id:%d -> %d)" % (par['name'], par['limits'][0:30], par['nodeid'], newNodeID)
        query = "insert into VICtemplate(treeid,nodeid,parentID,originID,name,index,leaf,instances,limits) \
          values (%d,%d,%d,%d,'%s',%d,'%s',%d,'%s')" %  \
          (treeID, newNodeID, newNodeIDmap[par['parentid']], parIDmap[par['originid']], \
           par['name'], par['index'], par['leaf'], par['instances'], par['limits'])
        dummy = toDB.query(query)

       
#
# copyTreeMetaData(treeID, campID)
#
def copyTreeMetaData(treeID, campID, templateName):
    """
    Copy the metadata of the tree.
    """
    # First create the tree. Unfortunately there are no suitable stored procedures to do this in a nice way...
	# TODO: Funerable in the current implementation are groupid and pt+pst+strategy
    fromTree = fromDB.query("select * from otdbtree where treeid=%d" % treeID).dictresult()[0]
    query = ''
    if fromTree['treetype'] == 20:
      query = "insert into otdbtree(treeid,momid,originid,classif,treetype,state,creator, \
      campaign,owner,description,groupid,processtype,processsubtype,strategy,name) values \
      (%d,%d,0,%d::int2,%d::int2,%d::int2,%d,%d::int2,%d,'%s',%d,'%s','%s','%s','%s')" %  \
      (treeID, fromTree['momid'], fromTree['classif'], fromTree['treetype'], fromTree['state'],  \
      fromTree['creator'], campID, \
      fromTree['owner'], fromTree['description'], fromTree['groupid'],  \
      fromTree['processtype'], fromTree['processsubtype'], fromTree['strategy'], templateName)
    else:
      query = "insert into otdbtree(treeid,momid,originid,classif,treetype,state,creator, \
      campaign,owner,description,groupid,processtype,processsubtype,strategy) values \
      (%d,%d,0,%d::int2,%d::int2,%d::int2,%d,%d::int2,%d,'%s',%d,'%s','%s','%s')" %  \
      (treeID, fromTree['momid'], fromTree['classif'], fromTree['treetype'], fromTree['state'],  \
      fromTree['creator'], campID, \
      fromTree['owner'], fromTree['description'], fromTree['groupid'],  \
      fromTree['processtype'], fromTree['processsubtype'], fromTree['strategy'])
    print query
    result = toDB.query(query)  # void function
    print "Created metadata for tree %d" % treeID
    return


#
# copyStateHistory(treeID)
#
def copyStateHistory(treeID):
    """
    Copy the state-history of the tree.
    """
    # Unfortunately there are no suitable stored procedures to do this in a nice way...
    fromStateList = fromDB.query("select * from statehistory where treeid=%d" % treeID).dictresult()
    for fromState in fromStateList:
        # copy state
        query = "insert into StateHistory(treeID,momID,state,userID,timestamp) values \
          (%d,%d,%d::int2,%d,'%s')" %  \
          (treeID, fromState['momid'], fromState['state'], fromState['userid'], fromState['timestamp'])
        result = toDB.query(query)  # void function
    print "Copied state-history for tree %d" % treeID
    return


#
# copyVICkvt(treeID)
#
def copyVICkvt(treeID):
    """
    Copy the key-value information of this tree.
    """
    # Unfortunately there are no suitable stored procedures to do this in a nice way...
    fromKvtList = fromDB.query("select * from vickvt where treeid=%d" % treeID).dictresult()
    for fromkvt in fromKvtList:
        # copy kvt
        query = "insert into vickvt(treeID,paramname,value,time) values \
          (%d,'%s','%s','%s')" %  \
          (treeID, fromkvt['paramname'], fromkvt['value'], fromkvt['timestamp'])
        result = toDB.query(query)  # void function
    print "Copied key-value information for tree %d" % treeID
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
        newID = toDB.query("select * from saveCampaign(0,'%s','%s','%s','%s','%s')" % 
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
    print "Found %d components to map" % len(nodeList)
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

    parser = OptionParser("Usage: %prog [options]" )
    parser.add_option("-D", "--sourcedatabase",
                      dest="fromDBname",
                      type="string",
                      default="",
                      help="Name of source OTDB database to use")

    parser.add_option("-S", "--sourcehost",
                      dest="fromDBhost",
                      type="string",
                      default="localhost",
                      help="Hostname of source OTDB database")

    
    parser.add_option("-d", "--destdatabase",
                      dest="toDBname",
                      type="string",
                      default="",
                      help="Name of destination OTDB database to use")

    parser.add_option("-s", "--desthost",
                      dest="toDBhost",
                      type="string",
                      default="localhost",
                      help="Hostname of destination OTDB database")

    parser.add_option("-t", "--treeid",
                      dest="treeID",
                      type="int",
                      default=0,
                      help="SASID (treeID) of default template to copy")

    # parse arguments

    (options, args) = parser.parse_args()

    if not options.fromDBname:
        print "Provide the name of source OTDB database to use!"
        print
        parser.print_help()
        sys.exit(1)

    if not options.toDBname:
        print "Provide the name of destination OTDB database to use!"
        print
        parser.print_help()
        sys.exit(1)

    if not options.treeID:
        print "Provide SASID (treeID) of default template to copy!"
        print
        parser.print_help()
        sys.exit(1)

    # Fill variables used in remainder of code
    fromDBhost=options.fromDBhost
    fromDBname=options.fromDBname
    toDBhost=options.toDBhost
    toDBname=options.toDBname
    treeID=options.treeID

    # calling stored procedures only works from the pg module for some reason.
    fromDB = pg.connect(user="postgres", host=fromDBhost, dbname=fromDBname)
    print "Connected to source database", fromDBname, "on host ",fromDBhost 
    toDB   = pg.connect(user="postgres", host=toDBhost, dbname=toDBname)
    print "Connected to destination database", toDBname, "on host ",toDBhost

    # Check for tree-existance in both databases.
    fromDBtree = fromDB.query("select * from OTDBtree t INNER JOIN campaign c ON c.ID = t.campaign where treeID=%d" % treeID).dictresult()
    toDBtree = toDB.query("select * from otdbtree where treeID=%d" % treeID).dictresult()
    if len(fromDBtree) == 0:
        print "Tree with treeID %d not found in database %s" % (treeID, fromDBname)
        sys.exit(1)
    if len(toDBtree) != 0:
        print "Tree with treeID %d already exists in database %s" % (treeID, toDBname)
        # TODO: implement -f option to copy the tree under a different number.
        sys.exit(1)
    if fromDBtree[0]['treetype'] == 10:	# PIC tree?
        print "PIC trees cannot be copied"
        sys.exit(1)

    # If copying a default template check that we don't create duplicates
    templateName=''
    if fromDBtree[0]['treetype'] == 20:
        templateName = fromDB.query("select name from otdbtree where treeID=%d" % treeID).getresult()[0][0]
        try:
          toTemplateID = toDB.query("select treeid from OTDBtree where name='%s'" % templateName).getresult()[0][0]
          print "The destination database has already a default-template with the name: %s" % templateName
          sys.exit(1)
        except IndexError:
          pass
        if fromDBtree[0]['processtype'] != '':
          try:
            toTemplateID = toDB.query("select treeid from OTDBtree where processtype='%s' and processsubtype='%s' and strategy='%s'" % (fromDBtree[0]['processtype'],fromDBtree[0]['processsubtype'],fromDBtree[0]['strategy'])).getresult()[0][0]
            print "Copying the tree would result in duplicate processtype/processsubtype/strategy combination"
            sys.exit(1)
          except IndexError, e:
            pass
        print "Safe to copy default template '%s' to the new database." % templateName

    # What's the version of this tree?
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
    newCampaignID = checkCampaign(fromDBtree[0]['name'])

    # TODO: check user table (owner of tree must exist)

    # copy the trees metadata first
    copyTreeMetaData(treeID, newCampaignID, templateName)

    if fromDBtree[0]['treetype'] == 20:	# template?
        copyTemplateTree(treeID)
    else:	# type must be 30 (VIC tree)
        copyVICtree(treeID)

    copyStateHistory(treeID)

    copyVICkvt(treeID)

    toDB.close()
    fromDB.close()
    sys.exit(0)
