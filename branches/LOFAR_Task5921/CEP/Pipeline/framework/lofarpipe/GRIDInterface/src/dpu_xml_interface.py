"""
Interface to the DPU that converts a XML input file into a batch of DPU jobs.
"""

import os
import sys
import time

from xml.dom import minidom
from xml.parsers.expat import ExpatError

from common.net.dpu_client_test import dpu_IO
from common.config.Environment import Env
from awlofar.toolbox.msss.pipeline_job import cep_pipeline_job
from lofar.parameterset import parameterset
from lofarpipe.support.data_map import *

class pipeline:
    """
    The pipeline class stores and interprets the information about the pipeline
    that was read from the XML file.
    """
    
    def __init__(self, id, predecessors='', inputs='', outputs='', parset=''):
        self.id = id
        self.predecessors_as_str = predecessors
        self.inputs = inputs
        self.output = outputs
        self.parset_as_str = str(parset)
        self.parset = parameterset()
        
    def read_parset(self):
        """
        Convert the string containing a parset into parameterset object.
        """
        self.parset.adoptArgv(str(self.parset_as_str).split('\n'))
        
    def get_predecessors(self):
        """
        Convert the string containing the pipeline's predecessors into
        a Python list by splitting on the comma symbol.
        An empty list is returned if the pipeline does not have any predecessors.
        """
        if self.predecessors_as_str:
            return self.predecessors_as_str.split(',')
        else:
            # In this case, the predecessors_as_str attribute was an empty string.
            return []
    
    def get_command(self):
        """
        Read the pipeline's top-level Python script, used to start the pipeline, from its parset.
        """
        return self.parset.getString('ObsSW.Observation.ObservationControl.PythonControl.pythonProgram', '')
        
    def get_tasks(self):
        """
        Convert the pipeline into DPU tasks. We assume that the pipeline can be parallelized by
        creating independent tasks for all its input files. Furthermore, we do take into account
        that there might be dependencies between different pipelines. In that case, task number i
        for input file i of the next pipeline will start when task number i for input file i of the 
        previous pipeline has finished.
        
        As an example, the following shows how a calibration pipeline followed by a target pipeline
        (which should wait for the calibration pipeline to finish) are parallelized:
        
                                Tasks
                        0      1     ...   N
        Pipeline 0:   SB000  SB001       SB00N  (all executed independently)
        (calibration)
        
        Pipeline 1:   SB000  SB001       SB00N  (horizontally independent, but vertically depending on the previous task)
        (target)
        
        The dependencies between the pipelines will be handled at a later stage.
        """
        
        # First, interpret the parset and get all the information about the
        # input and output files as was defined in the XML.
        self.read_parset()
        inputs_filenames_keys  = map(lambda input:  str( input['filenames']), self.inputs.values())
        inputs_locations_keys  = map(lambda input:  str( input['locations']), self.inputs.values())
        inputs_skip_keys       = map(lambda input:  str( input['skip']),      self.inputs.values())
        outputs_filenames_keys = map(lambda output: str(output['filenames']), self.outputs.values())
        outputs_locations_keys = map(lambda output: str(output['locations']), self.outputs.values())
        outputs_skip_keys      = map(lambda output: str(output['skip']),      self.outputs.values())
            
        input_map_list = []
        output_map_list = []
        # Combine the information about each input and output into tuples.
        # Note that the order of these keys are used when creating the individual jobs:
        # filenames, locations, skip values
        input_map_keys = zip(inputs_filenames_keys, inputs_locations_keys, inputs_skip_keys )
        output_map_keys = zip(outputs_filenames_keys, outputs_locations_keys, outputs_skip_keys )
        
        # Create a DataMap for each input and each output.
        for filename, location, skip in input_map_keys:
            input_map_list.append(
              DataMap([
                       tuple(os.path.join(location, filename).split(':')) + (skip,)
                                 for filename, location, skip in zip(
                                     self.parset.getStringVector(filename),
                                     self.parset.getStringVector(location),
                                     self.parset.getBoolVector(skip))
                     ])
            )
            
        for filename, location, skip in output_map_keys:
            output_map_list.append(
              DataMap([
                       tuple(os.path.join(location, filename).split(':')) + (skip,)
                                 for filename, location, skip in zip(
                                     self.parset.getStringVector(filename),
                                     self.parset.getStringVector(location),
                                     self.parset.getBoolVector(skip))
                     ])
            )
        
        # Align the data maps in order to validate them and set the skip values
        # in the same way for each input and output.
        align_data_maps(*(input_map_list+output_map_list))
        
        # Finally, convert everything into individual tasks.
        pipeline_jobs = []
        job_data_product_keys = input_map_keys + output_map_keys
        for idx, job_data_products in enumerate(zip(*(input_map_list+ output_map_list))):
            job = cep_pipeline_job()
            # Clone the parset by creating another instance.
            job_parset = parameterset()
            job_parset.adoptArgv(str(self.parset_as_str).split('\n'))
            job_should_be_skipped = False
            
            # Now replace all input and output information by the (single) data
            # element that should be processed by this task.
            for [job_data_product, job_data_product_key] in zip(job_data_products, job_data_product_keys):
                job_should_be_skipped = job_data_product.skip
                job.host = job_data_product.host
                # We assume that the job will be launched on the node where the
                # data is stored.
                host = 'localhost'
                filename = os.path.basename(job_data_product.file)
                file_location = os.path.dirname(job_data_product.file)
                skip = job_data_product.skip
                # Remember that the key order is determined in a previous zip.
                job_parset.replace(job_data_product_key[0], str([filename]))
                job_parset.replace(job_data_product_key[1], str([host + ":" + file_location]))
                job_parset.replace(job_data_product_key[2], str([skip]))
            
            if job_should_be_skipped :
                # If skip was True for either one of the input/output elements,
                # we should skip this job but increase the job index.
                continue
            
            job.parset_as_dict = job_parset.dict()
            job.command = self.get_command()
            job.name = self.id + "_" + str(idx)
            pipeline_jobs.append(job)
        
        return pipeline_jobs


class pipeline_xml_parser:
    """
    This class can be used to parse a XML input file that should be converted
    into DPU jobs. 
    """
    
    def parse(self, xmlfile):
        """
        Try to parse the XML file into a XML Document object. This will also
        check if the input file is a valid XML file.
        """
        
        try:
            self.xml = minidom.parse(xmlfile).documentElement
            return True
        except ExpatError as e:
            self.parse_error = e
            return False
        
    def get_child_text(self, tagname, node=None):
        """
        Try to find a child with a given tag name and return its (text)
        value. If no parent node was specified, the root node of the XML
        document will be used, i.e. the entire document will be searched.
        If no child with the given tag name can be found, an empty string
        is returned.
        """
        if node == None:
            node = self.xml
        children = node.getElementsByTagName(tagname)
        if len(children) > 0 and len(children[0].childNodes) > 0:
            return children[0].firstChild.data
        else:
            return ''
            
    def get_child_cdata(self, tagname, node):
       """
       For a given node, try to find a descendant with a given tag name and that
       contains a CDATA section. This CDATA, if found, will be returned as a 
       string. Otherwise, an empty string is returned.
       """
       elements = node.getElementsByTagName(tagname)
       result = ''
       for el in elements:
           for child in el.childNodes:
               if child.nodeType == self.xml.CDATA_SECTION_NODE:
                   result = child.data
       return result
       
    def node_to_dict(self, node):
        """
        Convert the XML subtree starting at the specified node into a Python
        dictionary. All branches (element nodes) are used as dictionary keys,
        while the leaves are used as values. Note that the function is
        recursive and, hence, the resulting dictionary itself can contain
        other dictionaries again.
        """
        d = {}
        for child in node.childNodes:
            if child.nodeType == self.xml.ELEMENT_NODE:
                d[child.nodeName] = self.node_to_dict(child)
            if child.nodeType == self.xml.TEXT_NODE and not child.nodeValue.strip() == '':
                return child.nodeValue
        return d
    
       
    def get_inputs(self, pipeline):
        """
        Return the inputs of a pipeline node as a dictionary.
        """
        inputs_node = pipeline.getElementsByTagName("inputs")
        if len(inputs_node) > 0:
            return self.node_to_dict(inputs_node[0])
        else:
            return {}
            
    def get_outputs(self, pipeline):
        """
        Return the outputs of a pipeline node as a dictionary.
        """
        outputs_node = pipeline.getElementsByTagName("outputs")
        if len(outputs_node) > 0:
            return self.node_to_dict(outputs_node[0])
        else:
            return {}
        
    def get_pipeline_block_id(self):
        """
        Return the block id of the pipeline node.
        """
        idtags = self.xml.getElementsByTagName('pipeline_block_id');
        if len(idtags) > 0:
            return idtags[0].firstChild.data
            
    def get_pipelines(self):
        """
        Find all pipelines described in the XML file, convert them into
        pipeline objects that store all the corresponding information
        and return them as a list.
        The function assumes that all pipelines at least have a pipeline_id,
        so it uses this to find pipelines in the XML file.
        """
        pipelines = []
        pipeline_ids = self.xml.getElementsByTagName('pipeline_id')
        for id_elem in pipeline_ids:
            id = id_elem.firstChild.data
            pipeline_node = id_elem.parentNode
            pipeline_obj = pipeline(id)
            pipeline_obj.predecessors_as_str = self.get_child_text('predecessors', pipeline_node)
            pipeline_obj.inputs = self.get_inputs(pipeline_node)
            pipeline_obj.outputs = self.get_outputs(pipeline_node)
            pipeline_obj.parset_as_str = self.get_child_cdata('config', pipeline_node)
            pipelines.append(pipeline_obj)
        return pipelines


class dpu_xml_interface:
    """
    The dpu_xml_interface class starts the DPU XML interface application and
    performs the following steps:
    
    - parse the arguments, make sure that a XML file is passed as argument;
    - parse the XML file and get all the pipeline objects stored in it;
    - determine a suitable order for the pipelines, based on their predecessors;
    - cut the pipeline into individual, independent tasks;
    - submit the these individual tasks in the correct way and order to the DPU;
    - keep polling the job statuses until all jobs have finished.
    
    """
    
    # DPU client object
    dpu = dpu_IO(dpu_name=Env['dpu_name'], dbinfo=('dummy', 'awdummy', 'dummy', 'dummy'))
    
    def usage(self):
        """
        Display usage.
        """
        exit("Usage: %s [options] <xml file>" % sys.argv[0])
        
    def exit(self, error, code=1):
        """
        Print an error and exit.
        """
        print >> sys.stderr, "Error!\n", error
        sys.exit(code)
        
    def parse_arguments(self):
        """
        Parse the input arguments and return the name of the given XML file.
        """
        if len(sys.argv) < 2:
            return self.usage()
            
        xml_filename = sys.argv[1]
        if not os.path.exists(xml_filename):
            self.exit("The specified XML file cannot be found: %s." % xml_filename)
            
        return xml_filename
        
    def order_pipelines(self, pipelines_unsorted):
        """
        Order the pipelines based on the dependecies that are specified with
        the predecessors information. Each pipeline can have one or more 
        predecessors that have to complete before the pipeline itself can run.
        
        We determine the order by iteratively going over all pipelines and
        moving pipelines into a new (ordered) list if they either do not have
        any predecessor or if all their predecessors are already in the new
        list. We keep doing this until the original list is empty, meaning
        that all dependencies were handled correctly.
        If during a single iteration nothing changes, it means we have a
        circular dependency that cannot be met.
        """
        pipelines = []
        pipelines_ids = []
        #TODO: how to run/store multiple independent pipelines?
        while len(pipelines_unsorted) > 0:
            changed = False
            for pipeline in pipelines_unsorted:
                if all(predecessor in pipelines_ids for predecessor in pipeline.get_predecessors()):
                    # The above will also evaluate to True for pipelines with no predecessors.
                    pipelines.append(pipeline)
                    pipelines_ids.append(pipeline.id)
                    pipelines_unsorted.remove(pipeline)
                    changed = True
            if not changed:
                # No changes are made during this iteration, which means that
                # there is a circular dependency.
                self.exit("A circular dependency between some pipelines was found.")
                    
        return pipelines

    def submit_jobs(self, jobs):
        """
        Submit a list of jobs to the DPU and return the DPU keys.
        The jobs parameter should be a list of lists, where the order of the
        lists determine their dependencies. Seen as a matrix, each column
        will be a job that runs the tasks (rows) sequentially. All columns
        are considered to be independent and, therefore, will be submitted
        as independent DPU jobs.
        """
        dpu_jobs = {}
        dpu_job_keys = {'SUBMITTED': [], 'FAILED': [], 'DONE': []}
        
        # Get all "columns" of the jobs matrix.
        for seq_jobs in zip(*jobs):
            # TODO: use one thread per job submission to submit all at once (submitting jobs can take a couple of seconds)
            
            # Map the column to individual tasks and set the right node
            # (i.e. where the data is stored) and the right mode for each task.
            job = map(lambda job: {'DPU_JOBS':[job], 'DPU_NODES':[job.host], 'DPU_MODE':'SEQ'}, seq_jobs)
            
            # Request a DPU key and submit the job.
            key = self.dpu.getkey()
            if self.dpu.submitjobs(key, jobs=job, code=None):
                dpu_job_keys['SUBMITTED'].append(key)
            else:
                print "Error while submitting job:", job.name
                dpu_job_keys['FAILED'].append(key)
        
        return dpu_job_keys
        
    def wait_for_jobs(self, dpu_job_keys):
        """
        This will start a loop that will run until all jobs have finished.
        Once a job finishes, we put the corresponding key in an appropriate list
        depending on the status of the job.
        """
        
        print "Waiting for all jobs to complete..."
        while not len(dpu_job_keys['SUBMITTED']) == 0:
            time.sleep(10)
            for key in dpu_job_keys['SUBMITTED']:
                status = self.dpu.getstatus(key)
                if status.startswith('FINISHED'):
                    dpu_job_keys['SUBMITTED'].remove(key)
                    if status.endswith('0/0/0/0'):
                        dpu_job_keys['DONE'].append(key)
                    else:
                        dpu_job_keys['FAILED'].append(key)
                        
        # TODO: retrieve jobs when done? What to do with logs?
        print dpu_job_keys
        
        
    def main(self):
        """
        Main function that runs all the individual steps.
        """
        
        # Parse the input arguments.
        xmlfile = self.parse_arguments()
        
        # Create a XML parser object and parse the given XML file.
        self.xml_parser = pipeline_xml_parser()
        if not(self.xml_parser.parse(xmlfile)):
            self.exit("Error while parsing the XML file:\n%s" % self.xml_parser.parse_error)
        
        # Get all pipelines from the XML file and determine a correct order.
        pipelines = self.xml_parser.get_pipelines()
        pipelines = self.order_pipelines(pipelines)
        
        # Convert all pipelines into individual tasks.
        jobs = []
        for p in pipelines:
            jobs.append(p.get_tasks())
            
        # Submit the jobs and wait for them to complete.
        dpu_job_keys = self.submit_jobs(jobs)
        self.wait_for_jobs(dpu_job_keys)

        return 0

if __name__ == '__main__':
    sys.exit(dpu_xml_interface().main())
