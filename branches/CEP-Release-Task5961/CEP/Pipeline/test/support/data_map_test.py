import os
import shutil
import tempfile
import unittest

from lofarpipe.support.data_map import (
    DataMap, MultiDataMap, DataProduct, MultiDataProduct,
    DataMapError, load_data_map, store_data_map, align_data_maps
)

from lofarpipe.support.lofarexceptions import DataMapError

class DataMapTest(unittest.TestCase):
    """
    Test class for the DataMap class in lofarpipe.support.data_map
    """
    def __init__(self, arg):
        super(DataMapTest, self).__init__(arg)
        self.old_style_map = [
            ('locus001', 'L12345_SB101.MS'),
            ('locus002', 'L12345_SB102.MS'),
            ('locus003', 'L12345_SB103.MS'),
            ('locus004', 'L12345_SB104.MS')
        ]
        self.new_style_map = [
            {'host': 'locus001', 'file': 'L12345_SB101.MS', 'skip': True},
            {'host': 'locus002', 'file': 'L12345_SB102.MS', 'skip': False},
            {'host': 'locus003', 'file': 'L12345_SB103.MS', 'skip': True},
            {'host': 'locus004', 'file': 'L12345_SB104.MS', 'skip': False}
        ]

    def setUp(self):
        """
        Create scratch directory and create required input files in there.
        """
        self.tmpdir = tempfile.mkdtemp()
        self.old_style_map_file = self._create_old_style_map_file()
        self.new_style_map_file = self._create_new_style_map_file()
        self.syntax_error_map_file = self._create_syntax_error_map_file()

    def tearDown(self):
        """
        Cleanup all the files that were produced by this test
        """
        shutil.rmtree(self.tmpdir, ignore_errors=True)

    def _create_old_style_map_file(self):
        f = open(os.path.join(self.tmpdir, 'old_style.map'), 'w')
        f.write(repr(self.old_style_map))
        f.close()
        return f.name

    def _create_new_style_map_file(self):
        f = open(os.path.join(self.tmpdir, 'new_style.map'), 'w')
        f.write(repr(self.new_style_map))
        f.close()
        return f.name

    def _create_syntax_error_map_file(self):
        f = open(os.path.join(self.tmpdir, 'syntax_error.map'), 'w')
        f.write("[{'host': 'locus001']")
        f.close()
        return f.name

    def test_old_style_map(self):
        data_map = DataMap(self.old_style_map)
        self.assertEqual(len(data_map), 4)
        self.assertEqual(data_map[0].host, 'locus001')
        self.assertEqual(data_map[1].file, 'L12345_SB102.MS')
        self.assertTrue(all(item.skip for item in data_map))

    def test_old_style_load_store(self):
        tmp_file = self.old_style_map_file + '.tmp'
        data_map = DataMap(self.old_style_map)
        store_data_map(tmp_file, self.old_style_map)
        reloaded_data_map = load_data_map(tmp_file)
        self.assertEqual(data_map, reloaded_data_map)
        self.assertTrue(reloaded_data_map.iterator is DataMap.TupleIterator)

    def test_new_style_map(self):
        data_map = DataMap(self.new_style_map)
        self.assertEqual(len(data_map), 4)
        self.assertEqual(data_map[0].host, 'locus001')
        self.assertEqual(data_map[1].file, 'L12345_SB102.MS')
        self.assertTrue(data_map[2].skip)

    def test_new_style_load_store(self):
        tmp_file = self.new_style_map_file + '.tmp'
        data_map = DataMap(self.new_style_map)
        data_map.save(tmp_file)
        reloaded_data_map = DataMap.load(tmp_file)
        self.assertEqual(data_map, reloaded_data_map)

    def test_tuple_iterator(self):
        data_map = DataMap(self.new_style_map)
        data_map.iterator = DataMap.TupleIterator
        tuples = [item for item in data_map]
        self.assertEqual(len(tuples), 4)
        self.assertTrue(all(isinstance(item, tuple) for item in tuples))
        self.assertTrue(all(len(item) == 2 for item in tuples))
        self.assertEqual(tuples[0], ('locus001', 'L12345_SB101.MS'))

    def test_skip_iterator(self):
        data_map = DataMap(self.new_style_map)
        data_map.iterator = DataMap.SkipIterator
        unskipped = [item for item in data_map]
        self.assertEqual(len(unskipped), 2)
        self.assertTrue(all(isinstance(item, DataProduct) for item in unskipped))
        self.assertEqual(unskipped[0].host, 'locus002')
        self.assertEqual(unskipped[0].file, 'L12345_SB102.MS')

    def test_syntax_error_map_file(self):
        self.assertRaises(SyntaxError, DataMap.load, self.syntax_error_map_file)

    def test_data_map_errors(self):
        error_maps = [
            42, # integer
            [1, 2, 3], # list of integer        
            'foo', # string
            ('foo', 'bar', 'baz'), # tuple of string
            [{'file': 'L12345_SB101.MS', 'skip': True}], # missing key
            [{'host': 'locus001', 'file': 'L12345_SB101.MS',
                'slip': True}], # misspelled key
            [{'host': 'locus001', 'file': 'L12345_SB101.MS',
                'skip': True, 'spurious':'Oops'}]           # spurious key
        ]
        for data_map in error_maps:
            self.assertRaises(DataMapError, DataMap, data_map)


class MultiDataMapTest(unittest.TestCase):
    """
    Test class for the MultiDataMap class in lofarpipe.support.data_map
    """
    def __init__(self, arg):
        super(MultiDataMapTest, self).__init__(arg)
        self.old_style_map = [
            ('locus001', ['L12345_SB101.MS']),
            ('locus002', ['L12345_SB102.MS']),
            ('locus003', ['L12345_SB103.MS']),
            ('locus004', ['L12345_SB104.MS'])
        ]
        self.new_style_map = [
            {'host': 'locus001', 'file': ['L12345_SB101.MS'],
             'file_skip':[True], 'skip': True},
            {'host': 'locus002', 'file': ['L12345_SB102.MS'],
             'file_skip':[False], 'skip': False},
            {'host': 'locus003', 'file': ['L12345_SB103.MS'],
             'file_skip':[True], 'skip': True},
            {'host': 'locus004', 'file': ['L12345_SB104.MS'],
             'file_skip':[False], 'skip': False}
        ]

    def setUp(self):
        """
        Create scratch directory and create required input files in there.
        """
        self.tmpdir = tempfile.mkdtemp()
        self.old_style_map_file = self._create_old_style_map_file()
        self.new_style_map_file = self._create_new_style_map_file()
        self.syntax_error_map_file = self._create_syntax_error_map_file()

    def tearDown(self):
        """
        Cleanup all the files that were produced by this test
        """
        shutil.rmtree(self.tmpdir, ignore_errors=True)

    def _create_old_style_map_file(self):
        f = open(os.path.join(self.tmpdir, 'old_style.map'), 'w')
        f.write(repr(self.old_style_map))
        f.close()
        return f.name

    def _create_new_style_map_file(self):
        f = open(os.path.join(self.tmpdir, 'new_style.map'), 'w')
        f.write(repr(self.new_style_map))
        f.close()
        return f.name

    def _create_syntax_error_map_file(self):
        f = open(os.path.join(self.tmpdir, 'syntax_error.map'), 'w')
        f.write("[{'host': 'locus001']")
        f.close()
        return f.name

    def test_old_style_map(self):
        data_map = MultiDataMap(self.old_style_map)
        self.assertEqual(len(data_map), 4)
        self.assertEqual(data_map[0].host, 'locus001')
        self.assertEqual(data_map[1].file, ['L12345_SB102.MS'])
        self.assertEqual(data_map[2].file_skip, [True])
        self.assertEqual(data_map[2].skip, True)
        self.assertTrue(all(item.skip for item in data_map))

    def test_new_style_map(self):
        data_map = MultiDataMap(self.new_style_map)
        self.assertEqual(len(data_map), 4)
        self.assertEqual(data_map[0].host, 'locus001')
        self.assertEqual(data_map[1].file, ['L12345_SB102.MS'])
        self.assertEqual(data_map[1].file_skip, [False])
        self.assertTrue(data_map[2].skip)

    def test_new_style_load_store(self):
        tmp_file = self.new_style_map_file + '.tmp'
        data_map = MultiDataMap(self.new_style_map)
        data_map.save(tmp_file)
        reloaded_data_map = MultiDataMap.load(tmp_file)
        self.assertEqual(data_map, reloaded_data_map)

    def test_tuple_iterator(self):
        data_map = MultiDataMap(self.new_style_map)
        data_map.iterator = MultiDataMap.TupleIterator
        tuples = [item for item in data_map]
        self.assertEqual(len(tuples), 4)
        self.assertTrue(all(isinstance(item, tuple) for item in tuples))
        self.assertTrue(all(len(item) == 2 for item in tuples))
        self.assertEqual(tuples[0], ('locus001', ['L12345_SB101.MS']))

    def test_skip_iterator(self):
        data_map = MultiDataMap(self.new_style_map)
        data_map.iterator = MultiDataMap.SkipIterator
        unskipped = [item for item in data_map]
        self.assertEqual(len(unskipped), 2)
        self.assertTrue(all(isinstance(item, MultiDataProduct) for item in unskipped))
        self.assertEqual(unskipped[0].host, 'locus002')
        self.assertEqual(unskipped[0].file, ['L12345_SB102.MS'])


    def test_syntax_error_map_file(self):
        self.assertRaises(SyntaxError, MultiDataMap.load, self.syntax_error_map_file)

    def test_data_map_errors(self):
        error_maps = [
            42, # integer
            [1, 2, 3], # list of integer        
            'foo', # string
            ('foo', 'bar', 'baz'), # tuple of string
            [{'file': 'L12345_SB101.MS', 'skip': True}], # missing key
            [{'host': 'locus001', 'file': 'L12345_SB101.MS',
                'slip': True}], # misspelled key
            [{'host': 'locus001', 'file': 'L12345_SB101.MS',
                'skip': True, 'spurious':'Oops'}], # spurious key
            [{'host': 'locus001', 'file_skip':["dsf"],
              'file': ['L12345_SB101.MS'], 'skip': True}], # incorrect boollist  
            [{'host': 'locus001', 'file_skip':[True, False],
              'file': ['L12345_SB101.MS'], 'skip': True}], #len != len
        ]
        for data_map in error_maps:
            self.assertRaises(DataMapError, MultiDataMap, data_map)

    def test_compare_DataMap_and_MultiDataMap(self):
        data_map = DataMap([])
        multi_data_map = MultiDataMap([])
        # Empty maps should be unequal also
        self.assertNotEqual(data_map, multi_data_map)


class HelperFunctionDataMapTest(unittest.TestCase):
    """
    Test class for the helper functions in lofarpipe.support.data_map
    Currently the align_data_maps is tested
    """
    def __init__(self, arg):
        super(HelperFunctionDataMapTest, self).__init__(arg)
        self.old_style_multi_map = [
            ('locus001', ['L12345_SB101.MS']),
            ('locus002', ['L12345_SB102.MS']),
            ('locus003', ['L12345_SB103.MS']),
            ('locus004', ['L12345_SB104.MS'])
        ]
        self.new_style_multi_map = [
            {'host': 'locus001', 'file': ['L12345_SB101.MS'],
             'file_skip':[True], 'skip': True},
            {'host': 'locus002', 'file': ['L12345_SB102.MS'],
             'file_skip':[False], 'skip': False},
            {'host': 'locus003', 'file': ['L12345_SB103.MS'],
             'file_skip':[True], 'skip': True},
            {'host': 'locus004', 'file': ['L12345_SB104.MS'],
             'file_skip':[False], 'skip': False}
        ]
        self.new_style_map = [
            {'host': 'locus001', 'file': 'L12345_SB101.MS', 'skip': True},
            {'host': 'locus002', 'file': 'L12345_SB102.MS', 'skip': False},
            {'host': 'locus003', 'file': 'L12345_SB103.MS', 'skip': True},
            {'host': 'locus004', 'file': 'L12345_SB104.MS', 'skip': False}
        ]

    def setUp(self):
        """
        Create scratch directory and create required input files in there.
        """
        self.tmpdir = tempfile.mkdtemp()
        self.old_style_map_file = self._create_old_style_map_file()
        self.new_style_map_file = self._create_new_style_map_file()
        self.syntax_error_map_file = self._create_syntax_error_map_file()

    def tearDown(self):
        """
        Cleanup all the files that were produced by this test
        """
        shutil.rmtree(self.tmpdir, ignore_errors=True)

    def _create_old_style_map_file(self):
        f = open(os.path.join(self.tmpdir, 'old_style.map'), 'w')
        f.write(repr(self.old_style_multi_map))
        f.close()
        return f.name

    def _create_new_style_map_file(self):
        f = open(os.path.join(self.tmpdir, 'new_style.map'), 'w')
        f.write(repr(self.new_style_multi_map))
        f.close()
        return f.name

    def _create_syntax_error_map_file(self):
        f = open(os.path.join(self.tmpdir, 'syntax_error.map'), 'w')
        f.write("[{'host': 'locus001']")
        f.close()
        return f.name

    def test_align_data_maps_not_enough_arguments(self):
        # If called with zere datamaps expect DataMapError
        self.assertRaises(DataMapError, align_data_maps)

        # if called with single datamap expect DataMapError
        data_map = MultiDataMap(self.new_style_multi_map)
        self.assertRaises(DataMapError, align_data_maps, [data_map])

    def test_align_data_maps_different_length_maps(self):
        data_map = MultiDataMap(self.new_style_multi_map)
        data_map_other_length = MultiDataMap(
            [{'host': 'locus001', 'file': ['L12345_SB101.MS'],
             'file_skip':[True], 'skip': True}])

        self.assertRaises(DataMapError, align_data_maps, [
                                data_map, data_map_other_length])


    def test_align_data_maps_equal_maps_no_change(self):
        data_map = MultiDataMap(self.new_style_multi_map)
        data_map2 = MultiDataMap(self.new_style_multi_map)
        data_map3 = MultiDataMap(self.new_style_multi_map)

        # Perform an align
        align_data_maps(data_map, data_map2)

        # There should be no changes and the three maps should be the same
        for entrie1, entrie2, entrie3 in zip(data_map, data_map2, data_map3):
            self.assertEqual(entrie1, entrie3)
            self.assertEqual(entrie2, entrie3)

    def test_align_data_maps_equal_maps_skip_set(self):
        data_map = MultiDataMap(self.new_style_multi_map)
        data_map2 = MultiDataMap(self.new_style_multi_map)

        # Change a single entrie in the datamap to True
        data_map.data[1].skip = True

        # Perform an align
        align_data_maps(data_map, data_map2)

        # The second entrie.skip should be set to True 
        self.assertTrue(data_map2.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")

    def test_align_data_maps_equal_maps_skip_set_5_maps(self):
        # test for DataMap align 
        data_map = DataMap(self.new_style_map)
        data_map2 = DataMap(self.new_style_map)
        data_map3 = DataMap(self.new_style_map)
        data_map4 = DataMap(self.new_style_map)
        data_map5 = DataMap(self.new_style_map)

        # change single skip value 
        data_map.data[1].skip = True

        # Perform an align
        align_data_maps(data_map, data_map2, data_map3, data_map4, data_map5)

        self.assertTrue(data_map2.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")
        self.assertTrue(data_map3.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")
        self.assertTrue(data_map4.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")
        self.assertTrue(data_map5.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")

    def test_align_data_maps_equal_maps_skip_set_5_multi_maps(self):
        # test for DataMap align 
        data_map = MultiDataMap(self.new_style_multi_map)
        data_map2 = MultiDataMap(self.new_style_multi_map)
        data_map3 = MultiDataMap(self.new_style_multi_map)
        data_map4 = MultiDataMap(self.new_style_multi_map)
        data_map5 = MultiDataMap(self.new_style_multi_map)

        # change single skip value 
        data_map.data[1].skip = True

        # Perform an align
        align_data_maps(data_map, data_map2, data_map3, data_map4, data_map5)

        self.assertTrue(data_map2.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")
        self.assertTrue(data_map3.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")
        self.assertTrue(data_map4.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")
        self.assertTrue(data_map5.data[2].skip, "The skip field was not"
                         " alligned correctly in the second entrie")

if __name__ == '__main__':
    import xmlrunner
    unittest.main(testRunner=xmlrunner.XMLTestRunner(output='result.xml'))


