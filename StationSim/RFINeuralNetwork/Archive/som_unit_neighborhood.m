function Neighbors = som_unit_neighborhood(msize,lattice,shape,maxn)

%SOM_UNIT_NEIGHBORHOOD Indexes of the neighbors of the map units.
%
% Neighbors = som_unit_neighborhood(msize,lattice,shape,maxn)
% 
% ARGUMENTS 
%
%  msize      (vector) dimensions of the map grid (n1, n2, ..., nk)
%              NOTE: the matrix notation of indexes is used
%  lattice    (string) map lattice form ('rect' or 'hexa')
%  shape      (string) map shape ('rect')
%  [maxn]     (scalar) maximum neighbor distance calculated for each unit, 
%              sqrt(sum(msize.^2))+1 by default, but if for example only 
%              1-neighborhoods are needed it is faster to calculate
%              them only. 
%
% RETURNS 
%  
%  Neighbors  (matrix) indexes of neighboring map units, 
%              size munits x munits, where munits is the number 
%              of units in the map (=prod(msize)). In the matrix 
%              each column i contains the minimum neighborhood of unit i
%              that the unit j  belongs to. If the neighborhood 
%              is greater than maxn, the value is Inf.
% 
% EXAMPLES
% 
%    Dist = som_unit_neighborhood(10 5], 'hexa', 'rect')
%    Dist = som_unit_neighborhood([10 5 20], 'rect', 'toroid')
%
% See also SOM_UNIT_DISTANCES, SOM_UNIT_COORDS.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 141097

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments 

error(nargchk(3, 4, nargin));

if length(msize)<1 | any(isnan(msize)), 
  error('NaNs and empty vectors are not valid specifiers for grid size.')
end

if nargin==3 | isempty(maxn) | isnan(maxn), 
  maxn = sqrt(sum(msize.^2))+1;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization

% coordinates of map units when the grid is spread on a plane
Coords = som_unit_coords(msize,lattice,'rect'); 
% remove hexa correction from first column
if strcmp(lattice,'hexa'), Coords(:,1) = Coords(:,1)/sqrt(0.75); end

[munits mdim] = size(Coords); 
Neighbors = zeros(munits,munits)*NaN;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Action

% First calculate the 1-neighborhood of each unit (adjacent units).
% For each unit in the 1-neighborhood it counts that maximum difference
% in coordinates along each coordinate axis is <= 1.

Sh = zeros(size(Coords));
d1 = msize(1);
d2 = msize(2);
c = 1.01; % 1-neighborhood limit (allow for approximations)

for i=1:munits,

  % difference in coordinates
  Dco = Coords - Coords(ones(munits,1)*i,:);
  maxdist0 = max(abs(Dco)');

  switch shape,
  case 'rect',
    n1 = find(maxdist0 <= c & maxdist0 > 0);

  case 'cyl', 
    Sh(:,2)=d2;  maxdist1 = max(abs(Dco+Sh)'); %East    
    Sh(:,2)=-d2; maxdist2 = max(abs(Dco+Sh)'); %West    
    n1 = find((maxdist0 <=c | maxdist1 <=c | maxdist2 <=c) &  maxdist0 > 0);

  case 'toroid',
    Sh(:,1)=0;   Sh(:,2)=d2;  maxdist1 = max(abs(Dco+Sh)'); %East    
    Sh(:,1)=d1;  Sh(:,2)=d2;  maxdist2 = max(abs(Dco+Sh)'); %SouthEast    
    Sh(:,1)=d1;  Sh(:,2)=0;   maxdist3 = max(abs(Dco+Sh)'); %South
    Sh(:,1)=d1;  Sh(:,2)=-d2; maxdist4 = max(abs(Dco+Sh)'); %SouthWest    
    Sh(:,1)=0;   Sh(:,2)=-d2; maxdist5 = max(abs(Dco+Sh)'); %West    
    Sh(:,1)=-d1; Sh(:,2)=-d2; maxdist6 = max(abs(Dco+Sh)'); %NorthWest
    Sh(:,1)=-d1; Sh(:,2)=0;   maxdist7 = max(abs(Dco+Sh)'); %North 
    Sh(:,1)=-d1; Sh(:,2)=d2;  maxdist8 = max(abs(Dco+Sh)'); %NorthEast   
    n1 = find((maxdist0 <=c | maxdist1 <=c | maxdist2 <=c ...
               | maxdist3 <=c | maxdist4 <=c | maxdist5 <=c ...
               | maxdist6 <=c | maxdist7 <=c | maxdist8 <=c) &  maxdist0 > 0);
  otherwise, 
    error (['Unknown shape: ', shape]);
  end

  Neighbors(i,i) = 0;
  Neighbors(i,n1) = 1; 
end

% Then calculate neighborhood distance for each unit using reflexsivity
% of neighborhood: let N1i be the 1-neighborhood set a unit i
% then N2i is the union of all map units belonging to the 1-neighborhood
% of any vector j in N1i (but not in N1i)

n=1; 
while n<=maxn & any(isnan(Neighbors)),
  n=n+1;
  for i=1:munits,
    candidates =  isnan(Neighbors(i,:));  % units not in any neighborhood yet
    if any(candidates), 
      prevneigh = find(Neighbors(i,:)==n-1);% neighborhood (n-1)
      if length(prevneigh)==1,
        N1_of_prevneigh = (Neighbors(prevneigh,:)==1);
      else % union of their N1:s
        N1_of_prevneigh = max(Neighbors(prevneigh,:)==1);
      end
      N1_of_prevneigh(find(isnan(N1_of_prevneigh))) = 0;
      Nn = find(candidates & N1_of_prevneigh); 
      if length(Nn), Neighbors(i,Nn) = n; Neighbors(Nn,i) = n; end
    end
  end  
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Build / clean up the return arguments

Neighbors(find(isnan(Neighbors))) = Inf;

