'''
This programs runs LOFAR regressions tests in a standalone fashion. 
Usage: regression_test_runner.py pipeline_type
Run the regressions test for pipeline_type. Perform the work on host1 and host2 (default localhost)

pipeline_type select any one of: 
  msss_calibratior_pipeline 
  msss_target_pipeline      
  msss_imager_pipeline      

All input, output and temporary files are stored in the working directory:
        /data/scratch/$USER/regression_test_runner/<pipeline_type> 

*** Warning: Directory of target node will be cleared at start of the run ***
'''
import os
import sys
import shutil
import distutils.dir_util
import subprocess
import argparse
from argparse import RawTextHelpFormatter
import fileinput
import ConfigParser

# test for the correct requirements for the pipeline tests
# we need to be able to grab and change installed files for full functionality

def test_environment(lofarroot,pipeline,datadir):

	# test if we started in the correct directory
	if not os.path.isfile(lofarroot + '/lofarinit.sh'):
		print 'Installation not found. Wrong LOFARROOT?: ',lofarroot
		exit()

	# test if the selected pipeline is valid	
	if not os.path.isfile(lofarroot + '/bin/' + pipeline + '.py'):
		print 'Pipeline does not exist in installation.\n Pipeline: ',lofarroot + '/bin/' + pipeline + '.py'
		exit()

	# test if the testdata dir is present (do not test the full tree just the parset)
	if not os.path.isfile(datadir + '/' + pipeline + '.parset'):
		print 'This test is not present in the data directory.\n Pipeline: ',datadir + '/' + pipeline + '.parset'
		exit()


# Clear old data:
# Clear var run directory: remove state, map and logfiles
# Assure working directory exists
# and remove all files in these dirs

def clear_old_data(lofarroot,pipeline,workdir,host0=None,host1=None,host2=None):
	print 'clearing working directories'
	rundir = lofarroot + '/var/run/pipeline/' + pipeline
	shutil.rmtree(rundir,True)
	os.makedirs(rundir)

	if host0 == 'localhost':
		print "clear localhost"
		shutil.rmtree(workdir,True)
		os.makedirs(workdir)
	# special code, relic from the shell script. TODO: necessary?
	if host0 == 'lce072':
		print "clear lce072"
		subprocess.call(['ssh',host0,'rm','-rf',workdir])
		subprocess.call(['ssh',host0,'mkdir','-p',workdir])

	if host1 != None and host1 != 'localhost':
		print "clear host 1"
		print " ".join(['ssh',host1,'rm','-rf',workdir])
		print " ".join(['ssh',host1,'mkdir','-p',workdir])

		subprocess.call(['ssh',host1,'rm','-rf',workdir])
		subprocess.call(['ssh',host1,'mkdir','-p',workdir])
	if host2 != None and host2 != 'localhost':
		print "clear  host2"
		subprocess.call(['ssh',host2,'rm','-rf',workdir])
		subprocess.call(['ssh',host2,'mkdir','-p',workdir])


# Prepare the data and parset to run in a pipeline type depending but static location
# copy input data from data storage to the target host
# copy full input data batch to the target hosts
# TODO: gsmserver can be changed via the parset file. should this be in here as well.
# functionality commented out for the time being 
# To alter values in the default parset file you can specify these values in a configfile called replace_parset_values.cfg
# The old string on left will be replaced with the string on the right.
# example: 
#         [replace]
#         ldb002 = juropa02

def prepare_testdata(lofarroot,pipeline,workdir,testdata,host0=None,host1=None,host2=None,gsmserver=None):
	print 'preparing testdata'
	if host0 == 'localhost':
		distutils.dir_util.mkpath(workdir + '/input_data')
		os.system('cp -r '+testdata+'/input_data/host1/* '+workdir+'/input_data')
		if host2 != None:
			print 'copy from: \n',testdata + '/input_data/host2/','\n to:\n',workdir + '/input_data'
			os.system('cp -r '+testdata+'/input_data/host2/* '+workdir+'/input_data')

	if host1 != None and host1 != 'localhost':
		subprocess.call(['ssh',host1,'mkdir',workdir + '/input_data'])
		os.system('scp -r '+testdata + '/input_data/host1/* ' + host1 + ':' + workdir + '/input_data')

	if host2 != None and host2 != 'localhost':
		subprocess.call(['ssh',host2,'mkdir',workdir + '/input_data'])
		os.system('scp -r '+testdata + '/input_data/host2/* ' + host2 + ':' + workdir + '/input_data')

	parset = testdata + '/' + pipeline + '.parset'
	shutil.copy(parset,workdir)

	print 'edit parset file'
	replacelist = None
	filepath = os.path.dirname(os.path.realpath(__file__)) + '/replace_parset_values.cfg'
	if os.path.isfile(filepath):
		config = ConfigParser.RawConfigParser()
		config.read(filepath)
		replacelist = config.items('replace')
		print 'values to repace:\n',replacelist

	for line in fileinput.input([workdir + '/' + pipeline + '.parset'], inplace=True):
		line = line.replace('host1_placeholder',host1)
		if host2 != None:
			line = line.replace('host2_placeholder',host2)
		line = line.replace('input_path1_placeholder',workdir + '/input_data')
		line = line.replace('input_path2_placeholder',workdir + '/input_data')
		if host1 != 'localhost':
			line = line.replace('output_path1_placeholder',workdir + '/output_data')
			line = line.replace('output_path2_placeholder',workdir + '/output_data')
		else:
			line = line.replace('output_path1_placeholder',workdir + '/output_data/host1')
			line = line.replace('output_path2_placeholder',workdir + '/output_data/host2')
		if replacelist:
			for key,val in replacelist:
				line = line.replace(key,val)
		#if gsmserver != None:
		#	line = line.replace('ldb002',gsmserver)
		sys.stdout.write(line)


# Prepare the pipeline config. Copy the default file and change values if needed
# To alter values in the default pipeline.cfg you can specify these values in a configfile called replace_config_values.cfg
# The old string on left will be replaced with the string on the right.
# You can also add new values. 'section' is a special value to start a new block
# example: 
#         [replace]
#         cep2 = local
#         /lustre/jwork/htb00/htb003/working_dir = /lustre/jhome17/htb00/htb003/regression_test_runner_workdir
#         [add]
#         section = remote
#         method = local 
#         max_per_node = 1

def prepare_pipeline_config(lofarroot,workdir,baseworkdir,username):
	shutil.copy(lofarroot + '/share/pipeline/pipeline.cfg',workdir)
	print 'edit pipeline.cfg file'
	replacelist = None
	addlist = None
	filepath = os.path.dirname(os.path.realpath(__file__)) + '/replace_config_values.cfg'
	if os.path.isfile(filepath):
		config = ConfigParser.RawConfigParser()
		config.read(filepath)
		replacelist = config.items('replace')
		addlist = config.items('add')
		print 'values to repace:\n',replacelist

	for line in fileinput.input([workdir + '/pipeline.cfg'], inplace=True):
		if replacelist:
			for key,val in replacelist:
				line = line.replace(key,val)
		line = line.replace('/data/scratch/' + username, baseworkdir)
		sys.stdout.write(line)

	with open(workdir + '/pipeline.cfg', 'a') as myfile:
		for key,val in addlist:
			if key == 'section':
				myfile.write( '\n['+ val + ']')
			else:
				myfile.write('\n'+key + ' = ' + val)


# Run the pipeline with the prepared data and configs

def run_pipeline(lofarroot,pipeline,workdir):
	print 'running the pipeline'
        command = ['python',lofarroot + '/bin/' + pipeline + '.py',workdir + '/' + pipeline + '.parset','-c',workdir + '/pipeline.cfg','-d']
        print 'command: ',command
	subprocess.call(command)


# Test if the pipeline computed the desired result.
# Copy the reference data and gather the pipeline results on the local node.
# Then run the regression test.

def validate_output(lofarroot,pipeline,workdir,testdata,host0=None,host1=None,host2=None):
	# if the pipeline did not ran on the local node gather the results.
	print 'validating output'
	if host1 != None and host1 != 'localhost':
		distutils.dir_util.mkpath(workdir + '/output_data/host1')
		subprocess.call(['scp','-r',host1 + ':' + workdir + '/output_data/L*',workdir + '/output_data/host1'])
	if host2 != None and host2 != 'localhost':
		distutils.dir_util.mkpath(workdir + '/output_data/host2')
		subprocess.call(['scp','-r',host2 + ':' + workdir + '/output_data/L*',workdir + '/output_data/host2'])

	# prepare the test environment and copy the reference data
	distutils.dir_util.mkpath(workdir + '/target_data/host1')
	distutils.dir_util.copy_tree(testdata + '/target_data/host1',workdir + '/target_data/host1')
	if host2 != None:
		distutils.dir_util.mkpath(workdir + '/target_data/host2')
		distutils.dir_util.copy_tree(testdata + '/target_data/host2',workdir + '/target_data/host2')

	# construct the commands for the tests
	script_path = os.path.dirname(os.path.realpath(__file__))
	commandhost1 = ['python',script_path + '/' + pipeline + '_test.py']
	commandhost2 = ['python',script_path + '/' + pipeline + '_test.py']

	source = os.listdir(workdir + '/target_data/host1')
	source.sort()
	for bla in source:
		commandhost1.append(workdir + '/target_data/host1/' + bla)
	source2 = os.listdir(workdir + '/output_data/host1')
	source2.sort()
	for bla in source2:
		commandhost1.append(workdir + '/output_data/host1/' + bla)
	commandhost1.append('0.0001')
	
	if host2 != None:
		source3 = os.listdir(workdir + '/target_data/host2')
		source3.sort()
		for bla in source3:
			commandhost2.append(workdir + '/target_data/host2/' + bla)
		source4 = os.listdir(workdir + '/output_data/host2')
		source4.sort()
		for bla in source4:
			commandhost2.append(workdir + '/output_data/host2/' + bla)
		commandhost2.append('0.0001')

	# execute the test
	print 'command: ',commandhost1
	subprocess.call(commandhost1)

	if host2 != None:
		print 'command: ',commandhost2
		subprocess.call(commandhost2)

if __name__ == '__main__':
	descriptiontext = "This programs runs LOFAR regressions tests in a standalone fashion.\n"+ \
                       "Usage: regression_test_runner.sh pipeline_type host1 host2\n" + \
                       "Run the regressions test for pipeline_type. Perform the work on host1 and host2\n" + \
                       "\n" + \
                       "pipeline_type select any one of:\n" + \
                       "  msss_calibratior_pipeline\n" + \
                       "  msss_target_pipeline\n" + \
                       "  msss_imager_pipeline\n" + \
                       "\n" + \
                       "All input, output and temporary files are stored in the following directory:\n" + \
                       "     /data/scratch/$USER/regression_test_runner/<pipeline_type> \n" + \
                       "\n" + \
                       "*** Warning: Directory of target node will be cleared at start of the run ***"

	username = os.environ.get('USER')
	homedir = os.environ.get('HOME')
	lofarroot = os.environ.get('LOFARROOT')
	print username, ' ',lofarroot,' ',homedir
	parser = argparse.ArgumentParser(description=descriptiontext, formatter_class=RawTextHelpFormatter)
	parser.add_argument('pipeline',help='give the name of the pipeline to test')
	parser.add_argument('--workdir',help='path of the working directory',default='/data/scratch/'+username+'/regression_test_runner')
	parser.add_argument('--workspace',help='root path of the installation',default=lofarroot)
	parser.add_argument('--controlhost',help='name of the host to run the job on',default='localhost')
	parser.add_argument('--computehost1',help='name of the host to run the job on',default='localhost')
	parser.add_argument('--computehost2',help='optional second host for distributed job tests',default='localhost')
	parser.add_argument('--testdata',help='base directory with the testdata',default='/data/lofar/testdata/regression_test_runner')
	parser.add_argument('--pipelinecfg',help='name of the pipeline config file',default='pipeline.cfg')
	parser.add_argument('--gsmserver',help='optional name of the server of the gsm database.')

	args = parser.parse_args()

	lofarexe = lofarroot + '/bin'

	testdata = args.testdata + '/' + args.pipeline
	print 'directory with testdata: ',args.testdata

	# if running in Jenkins environment $Workspace is defined and pointing to LOFARROOT
	if os.environ.get('WORKSPACE'):
		print 'Running in Jenkins'
		lofarexe = os.environ.get('WORKSPACE') + '/installed/bin'
		lofarroot = os.environ.get('WORKSPACE') + '/installed'

	# Not all pipelines (specifically the imaging pipeline) have all data for two nodes
	# Therefore we test here if the there is a host2 directory in the data dir. 
	# It is now possible to use the pipeline for this case without manual selection of the number
	# of hosts. If no data is present for a second host set it to None
	if not os.path.isdir(testdata + '/input_data/host2'):
		args.computehost2 = None

	script_path = os.path.dirname(os.path.realpath(__file__))
	print 'Running script: ',script_path

	workdir = args.workdir + "/" + args.pipeline

	test_environment(lofarroot,args.pipeline,testdata)
	clear_old_data(lofarroot,args.pipeline,workdir,'localhost',args.computehost1,args.computehost2)
	prepare_testdata(lofarroot,args.pipeline,workdir,testdata,'localhost',args.computehost1,args.computehost2,args.gsmserver)
	prepare_pipeline_config(lofarroot,workdir,args.workdir,username)
	run_pipeline(lofarroot,args.pipeline,workdir)
	validate_output(lofarroot,args.pipeline,workdir,testdata,'localhost',args.computehost1,args.computehost2)
