import os
import sys
import shutil
import distutils.dir_util
import subprocess
import argparse
import fileinput
import ConfigParser

# test for the correct requirements for the pipeline tests
def test_environment(lofarroot,pipeline,datadir):

	# test if we started in the correct directory
	if not os.path.isfile(lofarroot + '/lofarinit.sh'):
		print 'Installation not found. Wrong LOFARROOT?: ',lofarroot
		exit()

	# test if the selected pipeline is valid	
	if not os.path.isfile(lofarroot + '/bin/' + pipeline + '.py'):
		print 'Pipeline does not exist in installation.\n Pipeline: ',pipeline
		exit()

	# test if the testdata dir is present (do not test the full tree just the parset)
	if not os.path.isfile(datadir + '/' + pipeline + '.parset'):
		print 'This test is not present in the data directory.\n Pipeline: ',datadir + '/' + pipeline + '.parset'
		exit()

def clear_old_data(lofarroot,pipeline,workdir,host0=None,host1=None,host2=None):
	print 'clearing working directories'
	rundir = lofarroot + '/var/run/pipeline/' + pipeline
	shutil.rmtree(rundir,True)
	os.makedirs(rundir)
	#subprocess.call(['mkdir','-p', rundir])

	if host0 == 'localhost':
		print "clear localhost"
		shutil.rmtree(workdir,True)
		os.makedirs(workdir)
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

def prepare_testdata(lofarroot,pipeline,workdir,testdata,host0=None,host1=None,host2=None,gsmserver=None):
	print 'preparing testdata'
	#host2dir = testdata + '/input_data/host2'
	if host0 == 'localhost':
		#distutils.dir_util.copy_tree(testdata + '/input_data/host1',workdir + '/input_data')
		#shutil.copytree(testdata + '/input_data/host1',workdir + '/input_data')
		distutils.dir_util.mkpath(workdir + '/input_data')
		os.system('cp -r '+testdata+'/input_data/host1/* '+workdir+'/input_data')
		if host2 != None:
			print 'copy from: \n',testdata + '/input_data/host2/','\n to:\n',workdir + '/input_data'
			#shutil.copytree(testdata + '/input_data/host2',workdir + '/input_data/host2')
			os.system('cp -r '+testdata+'/input_data/host2/* '+workdir+'/input_data')
			#distutils.dir_util.copy_tree(testdata + '/input_data/host2',workdir + '/input_data')


	if host1 != None and host1 != 'localhost':
		subprocess.call(['ssh',host1,'mkdir',workdir + '/input_data'])
		os.system('scp -r '+testdata + '/input_data/host1/* ' + host1 + ':' + workdir + '/input_data')

	if host2 != None and host2 != 'localhost':
		subprocess.call(['ssh',host2,'mkdir',workdir + '/input_data'])
		os.system('scp -r '+testdata + '/input_data/host2/* ' + host2 + ':' + workdir + '/input_data')
		# subprocess scp cant find the path for whatever reason...
		#subprocess.call(['scp','-r',testdata + '/input_data/host2/*',host2 + ':' + workdir + '/input_data'])

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

def prepare_pipeline_config(lofarroot,workdir):
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
		sys.stdout.write(line)

	with open(workdir + '/pipeline.cfg', 'a') as myfile:
		for key,val in addlist:
			if key == 'section':
				myfile.write( '\n['+ val + ']')
			else:
				myfile.write('\n'+key + ' = ' + val)

def run_pipeline(lofarroot,pipeline,workdir): #,testdata):
	print 'running the pipeline'
	#shutil.copy(testdata + '/pipeline.cfg',workdir)
        command = ['python',lofarroot + '/bin/' + pipeline + '.py',workdir + '/' + pipeline + '.parset','-c',workdir + '/pipeline.cfg','-d']
        print 'command: ',command
	#shutil.copy(testdata + '/'+pipeline + '/' + pipeline + '.parset',workdir)
	subprocess.call(command)

def validate_output(lofarroot,pipeline,workdir,testdata,host0=None,host1=None,host2=None):
	print 'validating output'
	if host1 != None and host1 != 'localhost':
		distutils.dir_util.mkpath(workdir + '/output_data/host1')
		subprocess.call(['scp','-r',host1 + ':' + workdir + '/output_data/L*',workdir + '/output_data/host1'])
	if host2 != None and host2 != 'localhost':
		distutils.dir_util.mkpath(workdir + '/output_data/host2')
		subprocess.call(['scp','-r',host2 + ':' + workdir + '/output_data/L*',workdir + '/output_data/host2'])

	distutils.dir_util.mkpath(workdir + '/target_data/host1')
	distutils.dir_util.copy_tree(testdata + '/target_data/host1',workdir + '/target_data/host1')
	if host2 != None:
		distutils.dir_util.mkpath(workdir + '/target_data/host2')
		#shutil.copystat(testdata + '/target_data/host1',workdir + '/target_data')
		distutils.dir_util.copy_tree(testdata + '/target_data/host2',workdir + '/target_data/host2')

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

	print 'command: ',commandhost1
	#subprocess.call(['python',script_path + '/' + pipeline + '_test.py',workdir + '/target_data/*',workdir + 'output_data/*','0.0001'])
	subprocess.call(commandhost1)

	if host2 != None:
		print 'command: ',commandhost2
		subprocess.call(commandhost2)

if __name__ == '__main__':
	username = os.environ.get('USER')
	homedir = os.environ.get('HOME')
	lofarroot = os.environ.get('LOFARROOT')
	print username, ' ',lofarroot,' ',homedir
	parser = argparse.ArgumentParser()
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

	# if no data is present for a second host set it to None
	if not os.path.isdir(testdata + '/input_data/host2'):
		args.computehost2 = None

	script_path = os.path.dirname(os.path.realpath(__file__))
	print 'Running script: ',script_path

	workdir = args.workdir + "/" + args.pipeline

	test_environment(lofarroot,args.pipeline,testdata)
	clear_old_data(lofarroot,args.pipeline,workdir,'localhost',args.computehost1,args.computehost2)
	prepare_testdata(lofarroot,args.pipeline,workdir,testdata,'localhost',args.computehost1,args.computehost2,args.gsmserver)
	prepare_pipeline_config(lofarroot,workdir)
	run_pipeline(lofarroot,args.pipeline,workdir)
	validate_output(lofarroot,args.pipeline,workdir,testdata,'localhost',args.computehost1,args.computehost2)
