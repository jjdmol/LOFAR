%SOM_DEMO3 Self-organizing map visualization.

% Version 1.0beta juuso 071197 

clf reset;
figure(gcf)
echo on




clc
%    ==========================================================
%    SOM_DEMO3 - VISUALIZATION
%    ==========================================================

%    The SOM Toolbox offers two basic visualization functions: 
%    SOM_SHOW and SOM_SHOWGRID. Of these the SOM_SHOW uses
%    patch objects and is the primary visualization tool. 
%    The SOM_SHOWGRID uses SURF and PLOT commands. It was 
%    originally intended for visualization of the Sammon's 
%    projection, but it can also be used to 3D visualization
%    of the map.    

%    There's an important limitation that visualization functions have:
%    while the SOM Toolbox otherwise supports N-dimensional map grids, 
%    visualization only works for 1- and 2-dimensional map grids!!!

%    The visualization GUI tool, SOMUI_VIS uses the SOM_SHOW
%    and associated functions for visualization.

pause % Strike any key to continue...





clc
%    DEMO DATA
%    =========

%    Here we build the data used in this demo. It'll consist
%    of random vectors in three unit-cubes the centers of which
%    are at [0, 0, 0], [2 1 1] and [1 2 1].

%data
c1 = [0 0 0]; 
cube1 = (rand(100,3)-0.5) + c1(ones(100,1),:);
c2 = [2 1 1]; 
cube2 = (rand(100,3)-0.5) + c2(ones(100,1),:);
c3 = [1 2 1]; 
cube3 = (rand(100,3)-0.5) + c3(ones(100,1),:);
data = [cube1; cube2; cube3];

%labels
labels = cell(300,1);
for i=1:100,   labels{i,1} = 'first'; end
for i=101:200, labels{i,1} = 'second'; end
for i=201:300, labels{i,1} = 'third'; end

%data structure
sData = som_data_struct(data,'Train data',labels,{'X','Y','Z'});
plot3(data(:,1),data(:,2),data(:,3),'r+')
rotate3d on

pause % Strike any key to train the map...

%    Here we'll use just the default parameters to train the map.

sMap = som_init(sData);
sMap = som_train(sMap,sData);

pause % Strike any key to learn about SOM_SHOW...






clc
%    SOM_SHOW
%    ========

%    The SOM_SHOW function is used to visualization the unified 
%    distance matrix and the component planes of the SOM. 

%    The component planes visualizations shows what kind of 
%    values the model vectors of the map units have for different 
%    vector components. The U-matrix, or the Unified distance matrix, 
%    holds the distances between neighboring units normalized 
%    between [0,1]. It is used to see the cluster structure
%    of the map. 

%    Here the U-matrix and all three component planes of the
%    map are shown:

som_show(sMap)

%    Notice that the names of the components are shown as the titles
%    of the subplots. 

%    In the U-matrix figure (top left subplot) there appears to be
%    three clusters, separated from each other by high values
%    of the U-matrix. The values of the U-matrix are in fact 
%    calculated for points between map units only and the values
%    in the map units (marked with the little markers) are calculated
%    as the mean of the surrounding values. 

pause % Strike any key to continue...

%    The SOM_SHOW function has a few handy parameters:
%     - components can be shown in any order, multiple times, 
%       or not at all
%     - the vector values may be shown denormalized, if the 
%       function SOM_NORMALIZE_DATA has been used to normalize the
%       data
%     - the colorbars may be set horizontally or vertically
%     - the size of each map unit can be controlled

%    Here the 3rd component plane and the U-matrix are shown 
%    so that the size of each unit is in proportion to the height
%    of the hit histogram for the data: 

H = som_hits(sMap,sData); % calculate hit histogram
som_show(sMap,[3 0],'',[],H)

%    Notice that giving an empty value ('' or []) informes the 
%    function to use default values.

pause % Strike any key to continue...






clc
%    AUXILIARY FUNCTIONS OF SOM_SHOW
%    ===============================

%    The SOM_SHOW has many auxiliary functions. Most of them
%    add different kinds of marks on the map units.

%     som_addlabels      - Writes labels of map units on the map.
%     som_addhits        - Shows the number of hits in each map unit.
%     som_addtraj        - Shows a trajectory on the map.
%     som_clear          - Removes labels, hits and/or trajectories
%                          from the figure.
%     som_show_handles   - Gets the handles left to the figure 
%                          by the functions above.
%     som_recolorbar     - Reconfigures the colorbars after a 
%                          COLORMAP command.
%     som_manualclassify - Tool for manual clustering of the map.

%     Here the map is labeled with labels in the data vectors:

sMap = som_autolabel(sMap,sData,'vote');

%     Here the labels of the map are added to all subplots
%     of the figure.

som_addlabels(sMap)

pause % Press any key to add hit histogram...

%     Here the figures are cleared of labels.

som_clear('lab') 

%     Here hit histogram for the first cluster is added to the
%     second subplot. The size of the 'spot' denotes the amount
%     of hits in each unit.

H1 = som_hits(sMap,cube1);
som_addhits(sMap,H1,2,'spot','white')

pause % Press any key to add trajectory...

%     Here a trajectory (temporally ordered set of input vectors)
%     is defined, and added to the first subplot.

trajdata = [c1; c2; c3];  % remember that c1, c2 and c3 
                          % respond to centers of the first, 
                          % second and third cube
som_addtraj(sMap,trajdata,1)

pause % Press any key to change the colormap...

%     Matlab offers flexibility in the colormaps. Using the 
%     COLORMAP function, the colormap may be changed. There
%     are several useful colormaps readily available, for 
%     example 'hot' and 'jet'. The default number of colors 
%     in the colormaps is 64. However, it is often advantageous 
%     to use less colors in the colormap. This way the 
%     components planes visualization become easier to 
%     interpret.

%     Here the three component planes are visualized using 
%     the 'hot' colormap and only three colors. 

colormap(hot(3));
som_show(sMap,[1 2 3])

pause % Press any key to change the colorbar labels...

%     The function SOM_RECOLORBAR can be used to reconfigure
%     the labels beside the colorbar. 

%     Here the colorbar of the first subplot is labeled using 
%     labels 'small', 'medium' and 'big' at values 0, 1 and 2. 
%     For the colorbar of the second subplot, values are calculated
%     for the borders between colors. 

som_recolorbar(1,{[0 1 2]},'',{{'small','medium','big'}});
som_recolorbar(2,'border','');

pause % Press any key to learn about SOM_SHOWGRID...





clc
%    SOM_SHOWGRID
%    ============

%    SOM_SHOWGRID function was originally built to visualize the 
%    Sammon's projection of a map, but it can be applied to many
%    other tasks as well. 

%    Here we calculate and plot the Sammon's projection of the map.
%    Notice that calculating the Sammon's projection is 
%    a calculation intensive process, so results after 10 seconds 
%    might not be that great... Better results can be achieved by 
%    iterating the algorithm longer.

S2 = som_sammon(sMap,2,10,'seconds');
som_showgrid(sMap,S2)

pause % Press any key to continue...

%    The SOM_SHOWGRID function has two display modes. The first
%    shows the map grid with lines and circles. The second 
%    uses the SURF command, and is usually the better option
%    in three dimensions. 

%    Any matrix of unit coordinates can be given to SOM_SHOWGRID
%    instead of the Sammon's projection. Since the data is in
%    this case only 3-dimensional, we can plot the map using it.

%    Here the default mode of SOM_SHOWGRID is used on the left,
%    and the 'surf' mode on the right. Note that you can rotate
%    the pictures by dragging the mouse pointer on top of the them.

S3 = sMap.codebook;
subplot(1,2,1)
som_showgrid(sMap,S3)
subplot(1,2,2)
som_showgrid(sMap,S3,'surf')

pause % Press any key to change the colors...

%    In 'surf' mode also the color of the surface can be controlled. 
%    For example below the surface is colored according to the
%    hit histogram of the 2nd cube.

clf
H2 = som_hits(sMap,cube2); 
colormap(hot(20))
som_showgrid(sMap,S3,'surf',H2)
shading('interp')

pause % Press any key to visualize map shapes...

%    Using the SOM_UNIT_COORDS function to provide unit coordinates, 
%    the SOM_SHOWGRID can be used to visualize the map grids. 

%    Here the grids of the three possible map shapes (rectangular,
%    cylinder and toroid) are visualized. 

subplot(2,2,1)
som_showgrid({[10 5],'hexa','rect'});
subplot(2,2,2)
som_showgrid({[10 5],'rect','cyl'});
subplot(2,2,3)
som_showgrid({[10 5],'hexa','toroid'});

%    Finally, this is how the map could be visualized if its 
%    shape was toroid.

Co = som_unit_coords(sMap.msize,'rect','toroid');
subplot(2,2,4)
som_showgrid({sMap.msize,'rect','toroid'},Co,'surf')


echo off
