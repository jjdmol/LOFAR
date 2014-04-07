#!/usr/bin/python
from src.reprocessor import Reprocessor
from tests.pipelinegeneral import PipelineGeneralTest
from src.gsmparset import GSMParset


class ReprocessorTest(PipelineGeneralTest):
    """
    """
    def setUp(self):
        super(ReprocessorTest, self).setUp()
        self.pipeline.conn.close()
        self.pipeline = Reprocessor(custom_cm=self.cm, database='test')

    def test_simple(self):
        parset = GSMParset('tests/pipeline1.parset')
        self.pipeline.run_parset(parset)
        parset = GSMParset('tests/image1.parset')
        self.pipeline.run_parset(parset)
        parset = GSMParset('tests/image2.parset')
        self.pipeline.run_parset(parset)
        self.check_datapoints()
        self.pipeline.reprocess_image(2)
        self.pipeline.reprocess_image(2)

