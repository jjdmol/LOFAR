%SOM_DEMO2 Self-organizing map for the unit cube.

% Version 1.0beta juuso 071197 

clf reset;
figure(gcf)
echo on



clc
%    ==========================================================
%    SOM_DEMO2 - BASIC USAGE (2) - UNIT CUBE DATA
%    ==========================================================

%    som_data_struct    - Create a data struct.
%    som_name           - Give names to vector components.
%    som_bmus           - Find BMUs from the map.
%    som_hits           - Calculate hit histogram for the map.
%    som_quality        - Quality measures for the map.

%    In SOM_DEMO1 the Self-Organizing Map (SOM) was 
%    introduced and its basic usage was presented using 
%    2-dimensional data from the unit square.

%    In SOM_DEMO2 the basic functions are presented further
%    using 3-dimensional data from the unit cube. This represents
%    a more usual case where the dimension of the map grid is lower 
%    than the dimension of the data set. In this case the 
%    map cannot follow perfectly the data set any more but
%    must find a balance between two goals: 

%      - data representation accuracy
%      - data set topology representation accuracy

pause % Strike any key to continue...




clc
%    TRAINING DATA: THE UNIT CUBE
%    ============================

%    Here 500 data points are created in the unit cube:

D = rand(500,3);

%    Here the data points are plotted.

plot3(D(:,1),D(:,2),D(:,3),'+r')
rotate3d on

%    The ROTATE3D command enables you to rotate the picture 
%    by dragging the pointer above the picture with the leftmost
%    mouse button pressed down.

%    The data struct of the Toolbox enables naming of the 
%    vector components using the SOM_NAME function. 
%    Here the data set is inserted into a data struct:

sData = som_data_struct(D);

%    Here the vector components are given names which will 
%    be used in visualization: 

sData = som_name(sData,'comp_names',{'x','y','z'});

pause % Strike any key to train the SOM...





clc
%    INITIALIZE AND TRAIN THE SOM
%    ============================

%    In SOM_DEMO1 the random algorithm was used for initialization.
%    Usually, however, it is better to use the linear initialization
%    algorithm. SOM_INIT uses it by default. Here it is also
%    left to the SOM_INIT to decide the size of the map:

sMap  = som_init(sData);
sMap0 = sMap;             % make a copy for further analysis

%    The initialized map and the data set look like this:

subplot(1,2,1), som_showgrid(sMap,sMap.codebook)
axis on, hold on
plot3(sData.data(:,1),sData.data(:,2),sData.data(:,3),'+r') 
hold off

pause % Strike any key to train the SOM...

%    In SOM_DEMO1 the sequential algorithm was used to train the
%    map. By default the batch algorithm is used, because it 
%    is much faster and because the results are typically just as
%    good as with sequential algorithm. 

sMap  = som_train(sMap,sData);

%    After training the map looks like this. 

subplot(1,2,2), som_showgrid(sMap,sMap.codebook), axis on

pause % Strike any key to continue...






clc
%    BEST-MATCHING UNITS (BMU)
%    =========================

%    An important concept with Self-Organizing Maps is the 
%    Best-Matching Unit (BMU). The BMU of a data vector is 
%    the unit on the map whose model vector best resembles
%    the data vector. In practise the similarity is measured
%    as the minimum distance between data vector and each 
%    model vector on the map. 

%    The SOM Toolbox has a special function for calculating
%    BMUs: SOM_BMUS. SOM_BMUS returns the index of the unit 
%    in the reshaped map codebook:
%       reshape(sMap.codebook,[prod(sMap.msize) dim])
%    where dim is the dimension of the input vectors.

%    Here the BMU is searched for the vector [1 1 1]

bmu = som_bmus(sMap,[1 1 1])

%    To get the actual grid coordinates, the IND2SUB function 
%    can be used. Here the corresponding unit is marked with a 
%    black 'BMU' text in the figure. You can rotate the figure 
%    to see better where the BMU is.

[i1, i2] = ind2sub(sMap.msize,bmu);
co = sMap.codebook(i1,i2,:);
text(co(1),co(2),co(3),'BMU'), hold off

%    A hit histogram of a data set on a map is calculated
%    by calculating for each map unit the number of data vectors
%    in the set for which is has been the BMU. Hit histograms
%    can be used to compare data sets with each other.

pause % Strike any key to analyze map quality...






clc
%    SELF-ORGANIZING MAP QUALITY
%    ===========================

%    As stated in the beginning, the maps have two primary 
%    quality properties:
%      - data representation accuracy
%      - data set topology representation accuracy

%    The former is usually measured using average quantization error
%    between data vectors and their BMUs on the map.
%    For the latter several measures have been proposed, e.g. the 
%    topographic error measure: percentage of data vectors for which 
%    the first- and second-BMUs are not adjacent units. 

%    Both measures have been implemented in the SOM_QUALITY function.
%    Here are the quality measures for the trained map: 

som_quality(sMap,'qe',sData)       % quantization error
som_quality(sMap,'topog',sData)    % topographic error

pause % Strike any key to calculated quality of the untrained map...

%    Here are the quality measures for the initial map: 

som_quality(sMap0,'qe',sData)      % quantization error
som_quality(sMap0,'topog',sData)   % topographic error

%    As can be seen, by folding the SOM has reduced the 
%    average quantization error, but on the other hand
%    the topology representation capability has suffered.
%    By using a larger final neighborhood radius in 
%    the training, the map becomes stiffer and preserves
%    the topology of the data set better.

pause % Strike any key to visualize the map...






clf
clc
%    VISUALIZING THE SELF_ORGANIZING MAP
%    ===================================

%    Typically the SOM_SHOW function is used for
%    visualization. It visualizes the U-matrix and the 
%    component planes of the map. 
%    Here the U-matrix and the component planes of the figure
%    are visualized:

som_show(sMap)

%    Notice that the names of the components are included
%    as the titles of the subplots.

pause % Strike any key to continue...

%    The figure made by the SOM_SHOW function is built of
%    small patch-objects corresponding to the units of the 
%    map. By controlling the size of the objects additional
%    information can be visualized. 

%    Here the SOM_HITS function is used to calculate 
%    the hit histogram of the training data set and the 
%    SOM_SHOW function is used to visualize it. The 
%    size of the nodes corresponds to the height of the
%    histogram in each node, and the color to the value
%    of component 1 (the x coordinate).

H = som_hits(sMap, sData);
som_show(sMap,1,'','',H)

%    SOM_SHOW has many auxiliary function using which 
%    different kinds of information can be added to the figure.
%    The functions are: som_addhits, som_addlabels, som_addtraj,
%    som_clear, som_recolorbar, som_show_colorbars and 
%    som_show_handles. 

%    SOM visualization is introduced in more detail in SOM_DEMO3.


echo off
