% SOM Toolbox
% Version 1.0beta, 19-Dec-97
%
% SOM Toolbox version 1.0beta, Copyright (C) 1997 by Esa Alhoniemi,
% Johan Himberg, Kimmo Kiviluoto, Jukka Parviainen and Juha Vesanto.
%
% SOM Toolbox comes with ABSOLUTELY NO WARRANTY; for details
% see License.txt in the program package. This is free software, 
% and you are welcome to redistribute it under certain conditions; 
% see License.txt for details.
%
% All function names start with 'som_'
% except those in relation to the gui which start with 'somui_'
%
% Map and data structures
%
%   map struct           try 'help som_create'
%   data struct          try 'help som_data_struct'
%
%   som_create           - create self-organizing map structure
%   som_data_struct      - create a self-organizing map data struct
%   som_info             - print out information about map or data struct
%   som_name             - name some fields in map or data struct
%
% Initialization and training
%
%   som_init             - initialize map
%     som_randinit          - random initialization
%     som_lininit           - initialization using pricipal components
%   som_trainops         - set map train options
%   som_train            - train map
%     som_seqtrain          - train using sequential algorithm
%     som_batchtrain        - train using batch algorithm
%
% SOM_PAK compatibility
%
%   som_read_cod         - reads a SOM_PAK format codebook file
%   som_read_data        - reads a SOM_PAK format data file
%   som_write_cod        - writes a SOM_PAK format codebook file
%   som_write_data       - writes a SOM_PAK format data file
%
% Tools 
%
%   som_bmus             - calculates BMUs for given data vectors
%   som_hits             - calculates the number of hits in each map unit
%   som_autolabel        - automatically labels the SOM based on given data
%   som_quality          - quality measures for the map
%   som_normalize_data   - normalizes the given data set
%   som_denormalize_data - inverts the normalization
%   som_divide           - divides a dataset according to the given map
%   som_manualclassify   - tool for clustering SOM manually
%
% Visualization
%
%   som_show             - shows given component planes and/or u-matrix
%     som_plane            - show a component plane (2D)
%     som_plane3           - show a component plane (3D)
%     som_planeU           - show a u-matrix
%       som_umat             - calculates U-matrix
%   som_addlabels        - adds labels of map units on top of map
%     som_planeL           - writes labels on map
%   som_addhits          - as addlabels, but shows the number of hits
%     som_planeH           - writes a hit-matrix (or any matrix) on map
%   som_addtraj          - as addlabels, but shows trajectory
%     som_planeT           - draws a trajectory on map
%   som_clear            - clears hit marks, labels or trajectories
%   som_recolorbar       - reconfigures the colorbars on a som_show figure
%   
%   som_profile          - shows the model vectors of a given map 
%
%   som_showgrid         - shows the map grid, or its projection to 2/3D space
%     som_sammon           - calculates Sammon's projection 
%     som_cca              - calculates CCA projection
%     som_pca              - calculates PCA projection 
%
% General 
% 
%   som_gui              - starts the graphical user interface tool
%     somui_it           - initialization and training GUI
%     somui_vis          - visualization GUI
%   som_doit             - push-the-button-and-go function for using the toolbox
%   som_demo             - GUI for demos
%     som_demo1          - basic usage of the SOM Toolbox, part I
%     som_demo2          - basic usage of the SOM Toolbox, part II
%     som_demo3          - SOM visualization
%     som_demo4          - data analysis using the SOM
%

