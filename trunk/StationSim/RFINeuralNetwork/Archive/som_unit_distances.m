function Dist = som_unit_distances(msize,lattice,shape)

%SOM_UNIT_DISTANCES Calculates distances between unit locations on the grid.
%
% Dist = som_unit_distances(msize,lattice,shape)
% 
% ARGUMENTS
%
%  msize    (vector) dimensions of the map grid (n1, n2, ..., nk)
%            NOTE: the matrix notation of indexes is used
%  lattice  (string) map lattice form ('rect' or 'hexa')
%  shape    (string) map shape ('rect' or 'cyl' or 'toroid')
%
% RETURNS 
%  
%  Dist     (matrix) distances between map units, 
%            size munits x munits, where munits is the number of 
%            units in the map (=prod(msize)) 
% EXAMPLES
%
%    Dist = som_unit_distances([10 5], 'hexa', 'rect')
%    Dist = som_unit_distances([10 5 20], 'rect', 'toroid')
%
% NOTE: the distances are between units locations on the grid, 
% _not_ between unit weight vectors.
%
% See also SOM_UNIT_COORDS, SOM_UNIT_NEIGHBORHOOD.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 141097, 110997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments 

error(nargchk(3, 3, nargin));

if length(msize)<1 | any(isnan(msize)), 
  error('NaNs and empty vectors are not valid specifiers for grid size.')
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization

% coordinates of map units when the grid is spread on a plane
Coords = som_unit_coords(msize,lattice,'rect'); 

[munits mdim] = size(Coords); 
Dist = zeros(munits,munits);
if strcmp(lattice,'hexa'), c1 = sqrt(0.75); else c1 = 1; end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Action

Sh = zeros(size(Coords));
d1 = msize(1)*c1;
d2 = msize(2);

% calculate euclidian distance from each location to each other location
for i=1:munits, 
  % difference in coordinates on the spread grid
  Dco = Coords - Coords(ones(munits,1)*i,:); 
  switch shape,
  case 'rect', % rectangular shape
    Dist(i,:) = sqrt(sum(Dco'.^2));

  case 'cyl', % cylinder shape, shift to East and West
    D(1,:) = sqrt(sum(Dco'.^2));
    Sh(:,2)=d2;  D(2,:) = sqrt(sum((Dco+Sh)'.^2)); %East    
    Sh(:,2)=-d2; D(3,:) = sqrt(sum((Dco+Sh)'.^2)); %West    
    Dist(i,:) = min(D);

  case 'toroid', % toroid shape, shift to lots of directions
    D(1,:) = sqrt(sum(Dco'.^2));
    Sh(:,1)=0;   Sh(:,2)=d2;  D(2,:) = sqrt(sum((Dco+Sh)'.^2)); %East    
    Sh(:,1)=d1;  Sh(:,2)=d2;  D(3,:) = sqrt(sum((Dco+Sh)'.^2)); %SouthEast    
    Sh(:,1)=d1;  Sh(:,2)=0;   D(4,:) = sqrt(sum((Dco+Sh)'.^2)); %South
    Sh(:,1)=d1;  Sh(:,2)=-d2; D(5,:) = sqrt(sum((Dco+Sh)'.^2)); %SouthWest    
    Sh(:,1)=0;   Sh(:,2)=-d2; D(6,:) = sqrt(sum((Dco+Sh)'.^2)); %West    
    Sh(:,1)=-d1; Sh(:,2)=-d2; D(7,:) = sqrt(sum((Dco+Sh)'.^2)); %NorthWest
    Sh(:,1)=-d1; Sh(:,2)=0;   D(8,:) = sqrt(sum((Dco+Sh)'.^2)); %North 
    Sh(:,1)=-d1; Sh(:,2)=d2;  D(9,:) = sqrt(sum((Dco+Sh)'.^2)); %NorthEast   
    Dist(i,:) = min(D);

  otherwise, 
    error (['Unknown shape: ', shape]);
  end
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Build / clean up the return arguments

%no need
