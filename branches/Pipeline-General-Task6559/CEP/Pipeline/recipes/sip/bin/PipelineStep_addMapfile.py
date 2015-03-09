import os
from lofarpipe.support.data_map import DataMap
from lofarpipe.support.data_map import DataProduct

# mandatory arguments:
# cmdline for type of mapfile creation
# options: mapfile-dir, filename, identifier(name in parsetparset)
def plugin_main(args, **kwargs):
    result = {}
    datamap = None
    if args[0] == 'mapfile_from_folder':
        datamap = _create_mapfile_from_folder(kwargs['folder'])
    if args[0] == 'mapfile_from_parset':
        datamap = _create_mapfile_from_parset(kwargs['parset'], kwargs['identifier'])
    if args[0] == 'changed_mapfile_from_parset':
        datamap = _create_mapfile_from_parset(kwargs['parset'], kwargs['identifier'])
        folder = '/private/regression_test_runner_workdir/msss_imager_genericpipeline'
        for item in datamap:
            item.file = os.path.join(folder, 'concat.ms')
    if args[0] == 'mapfile_empty':
        fileid = os.path.join(kwargs['mapfile_dir'], kwargs['filename'])
        DataMap().save(fileid)
        result['mapfile'] = fileid
    if datamap:
        fileid = os.path.join(kwargs['mapfile_dir'], kwargs['filename'])
        datamap.save(fileid)
        result['mapfile'] = fileid
    return result


# helper function
def _create_mapfile_from_folder(folder):
    maps = DataMap([])
    measurements = os.listdir(folder)
    measurements.sort()
    for ms in measurements:
        maps.data.append(DataProduct('localhost', folder + '/' + ms, False))
    return maps


def _create_mapfile_from_parset(parset, identifier):
    dps = parset.makeSubset(
        parset.fullModuleName('DataProducts') + '.'
    )
    datamap = DataMap([
        tuple(os.path.join(location, filename).split(':')) + (skip,)
        for location, filename, skip in zip(
            dps.getStringVector(identifier + '.locations'),
            dps.getStringVector(identifier + '.filenames'),
            dps.getBoolVector(identifier + '.skip'))
    ])
    return datamap
