#!/usr/bin/python
import unittest
import sys
from src.resolveFlux import FluxResolver
from src.resolveSimple import SimpleResolver

class UtilsTest(unittest.TestCase):
    def test_flux(self):
        detections = [ 
            #id, ra, ra_err, decl, decl_err, f, f_err
            [101, 0.0, 0.1, 0.0, 0.1, 1.0, 0.1],
            [102, 0.1, 0.1, 0.1, 0.1, 5.0, 0.1]
        ]
        objects = [ 
            #id, ra, ra_err, decl, decl_err, f, f_err
            [1, 0.001, 0.1, 0.0, 0.1, 1.01, 0.1],
            [2, 0.101, 0.1, 0.1, 0.1, 5.01, 0.1]
        ]
        resolver = FluxResolver()
        is_ok, solution = resolver.resolve(detections, objects)
        self.assertTrue(is_ok)
        self.assertEqual(solution, [ [101, 1], [102, 2]])

    def test_simple(self):
        detections = [ 
            #id, ra, ra_err, decl, decl_err, f, f_err
            [101, 0.0, 0.1, 0.0, 0.1, 1.0, 0.1],
            [102, 0.1, 0.1, 0.1, 0.1, 5.0, 0.1],
            [103, 0.15, 0.1, 0.15, 0.1, 5.0, 0.1]
        ]
        objects = [ 
            #id, ra, ra_err, decl, decl_err, f, f_err
            [1, 0.001, 0.1, 0.0, 0.1, 1.01, 0.1],
            [2, 0.101, 0.1, 0.1, 0.1, 5.01, 0.1],
            [3, 0.151, 0.1, 0.15, 0.1, 5.01, 0.1],
        ]
        resolver = SimpleResolver()
        is_ok, solution = resolver.resolve(detections, objects)
        self.assertTrue(is_ok)
        self.assertEqual(solution, [ [101, 1], [102, 2], [103, 3]])

    def test_fail_flux(self):
        detections = [ 
            #id, ra, ra_err, decl, decl_err, f, f_err
            [101, 0.0, 0.5, 0.0, 0.5, 1.0, 0.5],
            [102, 0.1, 0.5, 0.1, 0.5, 1.1, 0.5]
        ]
        objects = [ 
            #id, ra, ra_err, decl, decl_err, f, f_err
            [1, 0.001, 0.5, 0.0, 0.5, 1.01, 0.5],
            [2, 0.101, 0.5, 0.1, 0.5, 1.11, 0.5]
        ]
        resolver = FluxResolver()
        is_ok, solution = resolver.resolve(detections, objects)
        self.assertFalse(is_ok)
        self.assertEqual(solution, [])

    def test_fail_simple(self):
        detections = [ 
            #id, ra, ra_err, decl, decl_err, f, f_err
            [101, 0.0, 0.1, 0.0, 0.1, 1.0, 0.1],
            [102, 0.1, 0.1, 0.1, 0.1, 5.0, 0.1],
            [103, 0.15, 0.1, 0.15, 0.1, 5.0, 0.1]
        ]
        objects = [ 
            #id, ra, ra_err, decl, decl_err, f, f_err
            [1, 0.001, 0.1, 0.0, 0.1, 1.01, 0.1],
            [2, 0.101, 0.1, 0.15, 0.1, 5.01, 0.1],
            [3, 0.151, 0.1, 0.1, 0.1, 5.01, 0.1],
        ]
        resolver = SimpleResolver()
        is_ok, solution = resolver.resolve(detections, objects)
        self.assertFalse(is_ok)
        self.assertEqual(solution, [])
