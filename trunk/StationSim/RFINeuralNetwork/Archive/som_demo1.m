%SOM_DEMO1 Self-organizing map for the unit square.

% Version 1.0beta juuso 071197 

clf reset;
figure(gcf)
echo on



clc
%    ==========================================================
%    SOM_DEMO1 - BASIC USAGE (1) - UNIT SQUARE DATA
%    ==========================================================

%    som_init     - Initializes a self-organizing map.
%    som_trainops - Change training algorithm, neighborhood 
%                   function or mask.
%    som_train    - Trains a self-organizing map.
%    som_show     - Visualizes a self-organizing map.
%    som_showgrid - Visualizes a the map grid.

%    SELF-ORGANIZING MAP (SOM):

%    A self-organized map (SOM) is a "map" of the training data, 
%    dense where there is a lot of data and thin where the data 
%    density is low. 

%    The map constitutes of neurons located on a regular map grid. 

som_showgrid({'hexa',[10 15]})
title('Hexagonal SOM grid')

%    Each neuron (marked by circles in the figure) is a model 
%    vector in the data space, and neighboring neurons (connected 
%    with lines in the figure) have similar model vectors.

%    The SOM can be used for data visualization, classification,
%    estimation and a variety of other purposes.

pause % Strike any key to continue...




clc
%    TRAINING DATA: THE UNIT SQUARE
%    ==============================

%    Here 300 data points are created in the unit square:

D = rand(300,2);

%    Here the data points are plotted.

plot(D(:,1),D(:,2),'+r')

pause % Strike any key to train the SOM...



clc
%    INITIALIZE THE SELF_ORGANIZING MAP
%    ==================================

%    The map will be a 2-dimensional grid of size 10 x 10.

msize = [10 10];

%    SOM_INIT initializes the model vectors in the map, and
%    sets the fields of the map structure to default values.
%    Below, the random initialization algorithm is used.

sMap  = som_init(D, msize, 'random');

%    In fact, even the map size can be left for the function 
%    to decide, as below. The size of the map is then based 
%    on the amount of data vectors and the principal 
%    eigenvectors of the data set.

%    sMap = som_init(D);

%    The initialized map and the data set look like this:

som_showgrid(sMap,sMap.codebook)
axis on, hold on
plot(D(:,1),D(:,2),'+r'), hold off

pause % Strike any key to train the SOM...





clc
%    TRAIN THE SELF_ORGANIZING MAP
%    =============================

%    Training is done with the batch-algorithm by default. 
%    Training algorithm can be changed to sequential using 
%    the SOM_TRAINOPS function.

sMap  = som_trainops(sMap,'seq');

%    SOM_TRAIN trains the map with the input data. Training
%    parameters may be given explicitly, or the default 
%    values can be used, as below. 

sMap  = som_train(sMap,D);

pause % Strike any key to visualize the SOM...




clc
%    VISUALIZE THE SELF_ORGANIZING MAP
%    =================================

%    The SOM_SHOWGRID function places the units of the map
%    on given locations and connects neighboring units with lines.

%    Since in this case the data is only 2-dimensional, the units 
%    can be placed to the locations indicated by the model vectors.

%    After the training, the map grid now looks like this:

som_showgrid(sMap,sMap.codebook), 
axis on, hold on
plot(D(:,1),D(:,2),'+r'), hold off

pause % Strike any key to see other visualizations...

%    If the data is higher than 3-dimensional this is not 
%    possible. In this case the visualization of the SOM 
%    is done using function SOM_SHOW which displays the 
%    U-matrix and component planes of the SOM. 

%    The component planes visualization shows what kind of 
%    values the model vectors of the map units have for different 
%    vector components.

figure; 
som_show(sMap,[1 2])

pause % Strike any key to see U-matrix...

%    The U-matrix, or the Unified distance matrix, holds 
%    the distances between neighboring units normalized 
%    between [0,1]. It is used to see the cluster structure
%    of the map. In this case since the data density is uniform
%    there are no clusters, so the U-matrix is not very 
%    informative.

figure; 
som_show(sMap,0)

%    More about visualization is presented in SOM_DEMO3.

echo off
