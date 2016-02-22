import os
from lofarpipe.support.data_map import DataMap
from lofarpipe.support.data_map import DataProduct


# mandatory arguments:
# cmdline for type of mapfile creation
# options: mapfile-dir, filename, identifier(name in parsetparset)
def plugin_main(args, **kwargs):
    #print 'PLUGIN KWARG: ', kwargs
    result = {}
    datamap = None
    fileid = kwargs['mapfile_in']
    datamap = DataMap.load(fileid)

    if 'join_files' in kwargs:
        for item in datamap:
            item.file = os.path.join(item.file,kwargs['join_files'])
    if 'add_name' in kwargs:
        for item in datamap:
            item.file = item.file + kwargs['add_name']
    if 'newname' in kwargs:
        fileid = os.path.join(os.path.dirname(fileid), kwargs['newname'])

    if datamap:
        print 'Writing mapfile: ',fileid
        datamap.save(fileid)
        result['mapfile'] = fileid
    return result
