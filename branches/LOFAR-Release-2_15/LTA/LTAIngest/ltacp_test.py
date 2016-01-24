import ltacp, os

# Returns True if a test transfer succeeds. Also removes the file from the SRM afterwards, but test result only depends on the transfer.
def test_transfer():
    surl = 'srm://srm.grid.sara.nl/pnfs/grid.sara.nl/data/lofar/ops/fifotest/ltacp-test.py'
    turl = ltacp.convert_surl_to_turl(surl)
    host = 'localhost'
    file = os.path.realpath(__file__)
    print 'transferring '+host+':'+file +' to '+turl
    ret = ltacp.transfer(host,file,turl)
    ltacp.srmrm(surl)
    return (ret == 0)

# Returns True if srm interaction to create missing directory tree works
def test_srmmkdirs():
    ret = ltacp.create_missing_directories('srm://srm.grid.sara.nl/pnfs/grid.sara.nl/data/lofar/ops/fifotest/ltacptest/directory/ltacptest.file') 
    ltacp.srmrmdir('srm://srm.grid.sara.nl/pnfs/grid.sara.nl/data/lofar/ops/fifotest/ltacptest/directory/')
    ltacp.srmrmdir('srm://srm.grid.sara.nl/pnfs/grid.sara.nl/data/lofar/ops/fifotest/ltacptest/')
    return (ret == 0)

# Returns True if the checksum is correct that is parsed from the srm listing of a test file 
def test_getchecksum():
    checksum = ltacp.get_srm_checksum('srm://srm.grid.sara.nl/pnfs/grid.sara.nl/data/lofar/ops/fifotest/file1M')
    return (checksum == '00f00001')

# Runs a full test, returns True if all succeed
def test_full():
    
    print "Testing get checksums... "
    if test_getchecksum(): 
        print 'Checksum test success.'
    else:
        print '! Checksum test failed!'
        return False
    
    print "Testing directory creation... "
    if test_srmmkdirs(): 
        print 'Directory creation test success.'
    else:
        print '! Directory creation test failed!'
        return False
    
    
    print "Testing file transfer... "
    if test_transfer():
        print 'Transfer test success.'
    else:
        print '! Transfer test failed!'
        return False
    
    return True


# limited standalone mode for testing:
# usage: ltacp.py <remote-host> <remote-path> <surl>
if __name__ == '__main__':
    test_full();