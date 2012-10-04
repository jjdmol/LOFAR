.. _framework-quickstart:

CEP quick-start (including example run)
==========================================


This section provides some quick notes on getting started with the pipeline system. More details are available in subsequent sections of this chapter. The first chapter detail all the steps needed to start an imaging pipeline from scratch until checking the output meta-data. (test data INCLUDED!!)



Setting up the environment and directories
-------------------------------------------
The pipelines and framework you will be using are exactly the same
as the automated central processing system. To allow usage on your 
own data some configuration has to be done. Keep in mind most of these steps have to be performed only a single time

*Step by Step:*

1. Log in on lfe001, the head-node of the cep1 cluster. 
   The pipelines should only be started on this cluster: The resource usage can be large and might interfere with observation!.
   
2. Load an environment: 

	a. ``use LofIm`` for the latest development version
   
3. Create directories:

	a. ``cexec lce: "mkdir /data/scratch/USERNAME`` Create a personal directory on the computation nodes.
			**Fill in your own user-name.**
	b. ``mkdir -p /home/USERNAME/pipeline/runtime_directory`` Create in your home a directory for runtime files.
	c. ``mkdir /home/USERNAME/pipeline/config_files`` Create a directory for the config files.
	d. ``mkdir /home/USERNAME/pipeline/parset_files`` Create a directory for the parset files.
      

4. Copy the configuration files to your own config dir:

	a. Because you are using the use command to set your environment you need to find out the location of these files. They might be create new each day. ``which msss_calibrator_pipeline.py`` Results for instance in. 		``/opt/cep/LofIm/daily/Fri/lofar_build/install/gnu_opt/bin/msss_calibrator_pipeline.py`` The config files are located relative from the ``install`` directory in ``gnu_opt/share/pipeline``
	b. Copy the ``pipeline.cfg`` and ``tasks.cfg`` files to your own configuration directory.
		``cp /opt/cep/lofar/lofar_versions/LOFAR-Release-1_3-latest/lofar_build/install/gnu_opt/share/pipeline/*.cfg /home/USERNAME/pipeline/config_files``
	c. ``cp /home/klijn/cep1.clusterdesc /home/USERNAME/pipeline/config_files/cep1.clusterdesc`` Copy the cluster description file to your config dir. It is currently located in a home directory
	d. This copy action will change the dynamic nature of the files. If you want to be sure that you have the bleeding edge software perform this copy step and the next adaptation step again. 
	
5. Adapt the configuration files so they point to your own directories:

	a. Open your own version pipeline.cfg with your editor of choice.
	b. Observe that the first entry lofarroot points to a daily build -or- a release version. This is the reasoning behind the dynamic nature of the configuration files. And a possible copy if you want to use the latest version.
	c. Change the runtime_directory entry to ``/home/USERNAME/pipeline/runtime_directory/``
		**THIS RUNTIME_DIRECTORY MUST BE ACCESSIBLE FROM ALL NODES**
	d. Change the working_directory entry to ``/data/scratch/USERNAME``
		**THIS WORKING_DIRECTORY CAN --NOT-- EVER, FOR ANY REASON, BE ON A GLOBAL SHARE. EVER**
	e. Change the clusterdesc entry to ``/home/USERNAME/pipeline/config/cep1.clusterdesc``
	f. Change to task_files entry to ``[/home/USERNAME/pipeline/config/tasks.cfg]``

.. code-block::	none 

	#Example pipeline.cfg	
	[DEFAULT]
	lofarroot = /opt/cep/LofIm/daily/Fri/lofar_build/install/gnu_opt
	casaroot = /opt/cep/LofIm/daily/Fri/casacore
	pyraproot = /opt/cep/LofIm/daily/Fri/pyrap
	hdf5root = /opt/cep/hdf5
	wcsroot = /opt/cep/wcslib
	pythonpath = /opt/cep/LofIm/daily/Fri/lofar_build/install/gnu_opt/lib/python2.6/dist-packages
	runtime_directory = /home/klijn/pipeline/runtime_directory
	recipe_directories = [%(pythonpath)s/lofarpipe/recipes]
	working_directory = /data/scratch/klijn
	task_files = [/home/klijn/pipeline/config/tasks.cfg]

	[layout]
	job_directory = %(runtime_directory)s/jobs/%(job_name)s

	[cluster]
	clusterdesc = /home/klijn/pipeline/config/cep1.clusterdesc

	[deploy]
	engine_ppath = %(pythonpath)s:%(pyraproot)s/lib:/opt/cep/pythonlibs/lib/python/site-packages
	engine_lpath = %(lofarroot)s/lib:%(casaroot)s/lib:%(pyraproot)s/lib:%(hdf5root)s/lib:%(wcsroot)s/lib

	[logging]
	log_file = %(runtime_directory)s/jobs/%(job_name)s/logs/%(start_time)s/pipeline.log

6. Run a short template run of the imaging pipeline:

	1. use LofIm
	2. ``cp /data/scratch/klijn/*.parset /home/USERNAME/pipeline/parset_files/out.parset`` copy the test parametersets file to your own parset directory.
	3. `` msss_imager_pipeline.py /data/scratch/klijn/out.parset --config ~/pipeline/config_files/pipeline.cfg --job test1 -d`` details: 

		a. ``msss_imager_pipeline.py`` the imaging pipeline executable
		b. ``/home/USERNAME/pipeline/parset_files/out.parset`` the settings for the pipeline
		c. ``--config ~/pipeline/config_files/pipeline.cfg`` the configuration to use
		d. ``--job test1`` a self chosen name allows distinguishing between runs
		e. ``-d`` turn on debugging information prints. The default settings of the pipeline is almost silent. This settings allows some sense of progress. 
		f. The pipeline should now perform a simple imaging run of a msss like observation.
		
	4. The resulting image can be found at lce001:/data/scratch/USERNAME/test1/awimage_cycle_0

7. Additional information:

	1. The pipeline remembers progress: And will not redo work already done. 
	2. ``cd /home/USERNAME/pipeline/runtime_directory/jobs/test1`` Go to the runtime_directory for the started/finished run. At this location you can find the logs, partial parset, mapfiles (internal datamember) and the statefile.
	3. deleting this state file will reset the pipeline and allows running from the start. (You could also rename your job)
	4. In the parset directory additional parsets will become available. Currently a full mom_parset.parset is provided. It contains ALL settings that are set from outside the pipeline framework.
	
8. TODO:

	1. A description of parameter set entries
	2. How-to use your own data
	3. How-to change the executables (task.cfg file changes)
	4. How-to use your own build of the offline processing framework 
	


