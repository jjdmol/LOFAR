%SOM_DEMO4 Data analysis using the SOM.

% Version 1.0beta juuso 071197 

clf reset;
echo on




clc
%    ==========================================================
%    SOM_DEMO4 - DATA ANALYSIS USING THE SOM
%    ==========================================================

%    In this demo, a real data set from a process is used. 
%    In fact this is the same data set that comes with the 
%    SOM_PAK software package. The demo shows how maps and data
%    sets can be interpreted using labeled samples.

%    For this demo to work at all it has to find the data set
%    files. Below we attempt to load them. If it doesn't succeed, 
%    please make the necessary corrections to the path and try 
%    again. 

%    Warning! Since neither Matlab nor the Toolbox are conservative
%    what comes to memory, running the demo will require quite
%    a lot memory!!

%    Here the SOM_PAK format data files are loaded:

sD1 = som_read_data('ex.data');      % primary data set
sD2 = som_read_data('ex_fts.data');  % faulty samples
sD3 = som_read_data('ex_ndy.data');  % normal day
sD4 = som_read_data('ex_fdy.data');  % faulty day

%    A standard procedure is to preprocess the data so that the 
%    variance of each component is scaled to one. This makes sure 
%    that each component has approximately equal influence in 
%    the training result.

%    Here the primary data set is normalized. 

sD1 = som_normalize_data(sD1,'som_var_norm');

%    Of course other data sets must be preprocessed in the same way.

sPrep = sD1.normalization;
sD2 = som_normalize_data(sD2,sPrep);
sD3 = som_normalize_data(sD3,sPrep);
sD4 = som_normalize_data(sD4,sPrep);

pause % Strike any key to train the map...

%    Here the map is initialized and trained using default 
%    parameters and the sD1 data set.

sMap = som_init(sD1);
sMap = som_train(sMap,sD1);

pause % Strike any key to analyze the map...







clc 
%    VISUAL INSPECTION OF THE MAP
%    ============================

%    The first step in the analysis of the map is visual inspection.
%    This is done by inspecting the U-matrix, component planes and
%    the Sammon's projection. 

f_umat = figure(gcf);
som_show(sMap,0)

figure;
f_comp = gcf;
som_show(sMap,[1:5 -2],'denormalized')
som_addhits(sMap,sD1,6,'spot','red')

drawnow

%    Notice that the values in the colorbars correspond to the
%    denormalized, or original, vector values. Notice also that the
%    sixth subplot is used to show the hit histogram.

%    From the component planes it can be seen that components 'Var1' 
%    and 'Var2' have strong correlation, as do the 'Var3' and 'Var4'.
%    As typical, there are some anomalious regions.

%    Here we calculate the Sammon's projection of the map.
%    It might take a while, so be patient...

S = som_sammon(sMap,3,300,'seconds');
figure; 
f_samm = gcf; 
som_showgrid(sMap,S,'surf')

%    Finally the quality of the map is checked with SOM_QUALITY. 
%    Of course it should be kept in mind that the quality measures
%    only measure how well the map represents the data set used
%    in the qualification process.

qerror = som_quality(sMap,'qe',sD1)
topog_error = som_quality(sMap,'topog',sD1)

pause % Strike any key to label the map...







clc
%    LABELING THE MAP
%    ================

%    If labeled samples are available, the map is can be labeled
%    using the SOM_AUTOLABEL function. The data set sD2 has samples
%    of faulty states. Each sample has a label 'faulty'. Using
%    the 'vote' mode of the SOM_AUTOLABEL function each map unit
%    that is BMU to at least one faulty sample, gets a 'faulty' label.

sMap = som_autolabel(sMap,sD2,'vote');
figure(f_umat)
som_addlabels(sMap,1,'all',[],'white')

%    It can be seen that there is a particular region on the map
%    corresponding to the 'fault' state. 

pause % Strike any key to continue...






clc 
%    TRAJECTORIES
%    ============

%    Trajectories are useful if the data is ordered sequentially 
%    in time. By plotting a trajectory on a labelled map, the state
%    of the process can be followed over time. In process monitoring,
%    for example, when the trajectory goes near the area labelled
%    faulty or dangerous, the operator knows that something should
%    be done.

%    Here the trajectory corresponding to the third data set
%    (the normal day cycle) is plotted as white line, and the
%    trajectory of the fourth data set (faulty day) as 
%    black line.

som_addtraj(sMap,sD3,1,[],'w-')
som_addtraj(sMap,sD4,1,[],'k-')






