function [] = som_showgrid(map, Coords, mode, colors)

%SOM_SHOWGRID Plots the grid structure of the matrix.
%
% som_showgrid(map, [Coords], [mode], [colors])
%
% ARGUMENTS ([]'s are optional)
%  
%  map       (struct or cell-array) a self-organizing map structure 
%             or a cell-array containing information on the size, 
%             lattice and shape of the map. E.g. {[20 10],'hexa','cyl'}.
%  [Coords]  (matrix) a matrix containing the coordinates of nodes
%             size either n1 x ... x nk x dim or n1*...*nk x dim
%  [mode]    (string) plotting mode, either 'surf' or a line color 
%             & style parameter for the plot3 command (e.g. 'r*')
%  [colors]  (vector or matrix) a vector or matrix from which 
%             the colors for the surface are acquired. Meaningful
%             only for 'surf' mode.
% 
% The function is used to visualize the grid-structure of a SOM. 
% Neighboring units are connected to each other and each map unit may be 
% given coordinates separately. Function is typically used to visualize
%
%  1. Sammon's projection of a map: 
%     S = som_sammon(sM,3); 
%     som_showgrid(sM,S) 
%     som_showgrid(sM,S,'surf')
%     som_showgrid({},S)
%  2. map grids (especially those of toroid and cylinder maps): 
%     som_showgrid(sM)                 % the grid of sM
%     som_showgrid({'toroid',[20 10]}) % the grid of a 20x10 toroid map
%
% Using the surf mode the function can also be used to visualize 
% component planes, or any other map unit specific data. This is
% especially useful if used for maps with cylinder or toroid shape:
% 
%  3. map unit specific data on map grid:
%     som_showgrid(sM,[],'surf',D)     % color the surf of sM grid with D
%
% See also SOM_SHOW, SOM_SAMMON.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 091097, 240997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 4, nargin));  % check number of input arguments

% map
if isstruct(map), 
  sM = map;
  msize = sM.msize;
  lattice = sM.lattice;
  shape = sM.shape;
elseif iscell(map),
  n = 1; 
  msize = []; lattice = ''; shape = '';
  for i=1:length(map), 
    if isnumeric(map{i}), msize = map{i}; 
    else
      switch map{i},
      case 'rect', 
        if length(lattice)==0, lattice = map{i}; else shape = map{i}; end
      case 'hexa',   lattice = map{i}; 
      case 'cyl',    shape = map{i}; 
      case 'toroid', shape = map{i}; 
      otherwise,     warning(['Unrecognized parameter: ' map{i}]);
      end
    end
  end
  % default values
  if length(lattice) == 0, lattice = 'rect'; end
  if length(shape) == 0,   shape = 'rect'; end
elseif isempty(map), 
  msize = []; lattice = 'rect'; shape = 'rect';
else
  error ('Illegal first input argument. Enclose it in {}s.');
end
  
% Coords
if nargin < 2 | isempty(Coords), 
  if prod(msize) == 0, 
    error('Size of map must be specified (and greater than zero).');
  end
  Coords = som_unit_coords(msize,lattice,shape);
  
else
  csize = size(Coords); 
  cdim = csize(length(csize));   
  csize = csize(1:(length(csize)-1));
  if isempty(msize), msize = csize; end
  if prod(msize) ~= prod(csize), 
    error('Number of vectors in coordinates matrix does not match the map size.');
  end
  Coords = reshape(Coords,[prod(csize) cdim]);
end

% mode
if nargin < 3 | isempty(mode), mode = 'bo'; end

% colors
if strcmp(mode,'surf'), 
  if nargin < 4 | isempty(colors), 
    % set color as the first component
    colors = Coords(:,1);
  else
    if prod(size(colors)) ~= prod(msize), 
      error ('Number of elements in colors vector does not match the map size.');
    end
    colors = reshape(colors,[msize 1]);
  end
end

% odim
[munits odim] = size(Coords); 
switch odim, 
  case 1, Coords = [Coords, zeros(munits,2)];
  case 2, Coords = [Coords, zeros(munits,1)];
  case 3, %ok
  otherwise, 
    error('Cannot plot higher than 3D data.');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize

if strcmp(mode,'surf'), 
  Coords = reshape(Coords(:,[1 2 3]), [msize 3]);
  colors = reshape(colors,[msize 1]);
else
  % calculate the lines connecting the units
  connect = 1.01; %Change this constant to modify to which neighbors
                  %should the nodes be connected to. Examples:
                  % 1.01 : 4-neighborhood for rect grid, or 6- for hexa grid
                  % 1.42 : 8-neighborhood for rectangular grid
                  % 2.01 : neighbors at distance 2
  Dmatrix = som_unit_distances(msize,lattice,shape);
  Lines = [NaN NaN NaN]; n=0;
  for i=1:munits, 
    for j=(i+1):munits, 
      if Dmatrix(i,j) <= connect, 
        Lines(n+[1:3],:) = [Coords([i j],:); NaN, NaN, NaN];
        n = n+3;      
      end
    end
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

if strcmp(mode,'surf'), 
  surf(Coords(:,:,1),Coords(:,:,2),Coords(:,:,3),reshape(colors,msize)); 
else
  plot3(Coords(:,1),Coords(:,2),Coords(:,3),mode);
  hold on; plot3(Lines(:,1),Lines(:,2),Lines(:,3),'-'); hold off;
end
axis('equal'), axis('off')
if odim<3, view([0 90]); else rotate3d on; end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
