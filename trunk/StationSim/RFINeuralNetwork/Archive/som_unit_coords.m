function Coords = som_unit_coords(msize,lattice,shape)

%SOM_UNIT_COORDS Locations of units on the SOM grid. 
%
% Coord = som_unit_coords(msize,lattice,shape)
% 
% ARGUMENTS 
%
%  msize      (vector) dimensions of the map grid (n1, n2, ..., nk)
%              NOTE: the matrix notation of indexes is used
%  lattice    (string) map lattice form ('rect' or 'hexa')
%  shape      (string) map shape ('rect' or 'cyl' or 'toroid')
%
% RETURNS 
%  
%  Coords     (matrix) coordinates of map units, with the coordinate
%              of the first unit at (1, ..., 1). Size munits x mdim, 
%              where munits is the number of units in the 
%              map (=prod(msize)) and mdim is 
%              the dimension of the grid (=length(msize))
% EXAMPLES
% 
%    Coords = som_unit_coords([10 5], 'hexa', 'rect')
%    Coords = som_unit_coords([10 5 20], 'rect', 'toroid')
%
% NOTE: In the case of hexagonal lattice, the 
% coordinates of units along the second axis are _not_ integers, 
% but multiples of sqrt(0.75). This is done to make distances
% of a unit to all its six neighbors equal. In addition the
% second coordinates of every other row are shifted by 0.5 to 
% positive direction. Therefore the coordinates returned
% by som_unit_coords for 'hexa' lattice _cannot_ be used
% as indices to the matrix.
%
% See also SOM_UNIT_DISTANCES.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 141097, 061097, 110997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments 

error(nargchk(3, 3, nargin));

if length(msize)<1 | any(isnan(msize)), 
  error('NaNs and empty vectors are not valid specifiers for grid size.')
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization

munits = prod(msize);
mdim = length(msize);
Coords = zeros(munits,mdim);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Action

% build a matrix of coordinates so that for each map unit
% its coordinates are placed in the Coords matrix

switch(lattice),
case 'rect', 
  for i=1:munits, 
    ind = i - 1;
    for d = 1:mdim, 
      Coords(i,d) = rem(ind,msize(d));
      ind = (ind - Coords(i,d))/msize(d);
    end
  end

case 'hexa', 
  if mdim > 2, 
    error('You can only use hexa lattice with 1- or 2-dimensional maps.');
  end
  if mdim==1, msize = [msize, 1]; mdim = 2; Coords = [Coords, Coords]; end
  for i=1:munits, 
    ind = i - 1;
    Coords(i,1) = rem(ind,msize(1));
    ind = (ind - Coords(i,1))/msize(1);
    Coords(i,2) = rem(ind,msize(2));
    if mod(Coords(i,1),2) == 1, Coords(i,2) = Coords(i,2) + 0.5; end
  end

otherwise, error(['Unknown lattice: ', lattice]);
end

if strcmp(shape,'rect'), 

  if strcmp(lattice,'hexa'), 
    % this correction is made to make distances to all 
    % neighboring units equal
    Coords(:,1) = Coords(:,1)*sqrt(0.75); 
  end
  % make the coordinates of units begin from (1, ... ,1)
  Coords = Coords + 1;

else

  % to make cylinder/toroid the coordinates must lie in 3D space, at least
  [munits mdim] = size(Coords);
  if     mdim == 1, Coords = [Coords ones(munits,2)];
  elseif mdim == 2, Coords = [Coords ones(munits,1)];
  end  
  mdim = 3;

  % Bend the coordinates to a circle in the plane formed by 2nd 
  % and 3rd axis. Notice that the angle to which the last coordinates
  % are bended is _not_ 360 degrees, because that would be equal to 
  % the angle of the first coordinates (0 degrees).
  angle = 2*pi*msize(2)/(msize(2)+1);
  Coords(:,[2 3]) = bend(Coords(:,2),Coords(:,3),angle,strcmp(lattice,'hexa'));

  switch shape
  case 'cyl', % ok
  case 'toroid', 
    % Bend the coordinates to a circle in the plane formed by 1st and
    % 2nd axis. 
    angle = 2*pi*msize(1)/(msize(1)+1);
    Coords(:,[1 3]) = bend(Coords(:,1),Coords(:,3),angle,0);
    % NOTE: if lattice is 'hexa', the msize(1) should be even, otherwise 
    % the bending the upper and lower edges of the map do not match 
    % to each other
    if strcmp(lattice,'hexa') & mod(msize(1),2)==1, 
      warning('Map size along first coordinate is not even.');
    end
  otherwise, error(['Unknown shape: ', shape]);    
  end

end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Build / clean up the return arguments

% if you want an [n1 n2 ... nk mdim] matrix 
%   reshape(Coords,[msize mdim]);

return;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% a subfunction used to process bend the data to a circle 

function C = bend(cx,cy,angle,xishexa)

dx = max(cx) - min(cx);
if dx ~= 0, 
  if xishexa, 
    % in case of hexagonal lattice it must be taken into account that
    % coordinates of every second row are +0.5 off to the right
    dx = dx-0.5; 
  end
  cx = angle*(cx - min(cx))/dx; 
end    
C(:,1) = (cy - min(cy)+1) .* cos(cx);
C(:,2) = (cy - min(cy)+1) .* sin(cx);


