function U = som_umat(M, varargin)

%SOM_UMAT Compute unified distance matrix of self-organizing map.
%
% U = som_umat(M, [lattice], [shape], [mode])
%
% ARGUMENTS ([]'s are optional)
%
%  M          (struct or matrix) self-organizing map struct or the 
%               the corresponding codebook matrix, size n1 x n2 x dim
%  [lattice]  (string) map lattice, 'rect' or 'hexa', default is 'rect'
%  [shape]    (string) map shape, 'rect' or 'cyl' or 'toroid', 
%              default is 'rect'
%  [mode]     (string) method used for determining unit center value,
%              'mean' or 'median', default is 'mean'
% RETURNS
%
%  U          (matrix)  u-matrix of the self-organizing map, 
%              size (2*n1-1 x 2*n2-1)
%
% This function calculates and returns the unified distance matrix
% of a SOM. For example a case of 5x1 -sized map:
%            m(1) m(2) m(3) m(4) m(5)
% where m(i) denotes one map unit. The u-matrix is a 9x1 vector:
%    u(1) u(1,2) u(2) u(2,3) u(3) u(3,4) u(4) u(4,5) u(5) 
% where u(i,j) is the distance between map units m(i) and m(j)
% and u(k) is the mean (or the median) of the surrounding values, 
% e.g. u(3) = (u(2,3) + u(3,4))/2. Examples of usage:
%
%  U = som_umat(sM);  
%  U = som_umat(sM.codebook(:,:,[1 3:5]),sM.lattice,sM.shape,'median');
%  U = som_umat(rand(10,10,4),'hexa','rect'); 
% 
% NOTE: the dimension of the map grid may be 2 at most.
% NOTE: the mask field of the M matrix is ignored. 
%
% See also: SOM_SHOW, SOM_PLANEU.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso / ecco 210797, ecco 190997, juuso 260997,
%  ecco 130199

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 4, nargin));  % check no. of input arguments is correct

% default values
lattice = 'rect';
shape = 'rect';
m = 1;

% M
if isstruct(M), % som struct
  lattice = M.lattice;
  shape = M.shape;
  M = M.codebook;
end
if length(size(M)) > 3
  error('Cannot compute u-matrices for map whose dimensions are higher than 2.');
end

% shape, lattice, mode
for i=1:(nargin-1),
  switch varargin{i},
  case 'rect',   if i==1, lattice = 'rect'; else shape = 'rect'; end
  case 'hexa',   lattice = 'hexa';
  case 'cyl',    shape = 'cyl';
  case 'toroid', shape = 'toroid';
  case 'mean',   m = 1;
  case 'median', m = 2;
  otherwise, 
    error(['Unrecognized input argument: ', varargin{i}]); 
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize variables

si = size(M);
y  = si(1);
if length(si) == 2
  x = 1;
  dim = si(2); 
else
  x   = si(2);
  dim = si(3);
end
  
if x == 1 &  y == 1,
  warning('Only one codebook vector');
  U = [];
  return;
end

ux = 2 * x - 1; 
uy = 2 * y - 1;
U  = zeros(uy, ux);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% u-matrix computation

if strcmp(lattice, 'rect'), % rectangular lattice
  if ~strcmp(shape,'rect'), 
    warning(['Using rectangular shape instead of ', shape, '.']);
  end
  
  for j=1:y, for i=1:x,
      if i<x, 
	dx = sum((M(j,i,:) - M(j,i+1,:)).^2); % horizontal
	U(2*j-1,2*i) = sqrt(dx);
      end 
      if j<y, 
	dy = sum((M(j,i,:) - M(j+1,i,:)).^2); % vertical
	U(2*j,2*i-1) = sqrt(dy);
      end
      if j<y & i<x,
	dz1 = sum((M(j,i,:) - M(j+1,i+1,:)).^2); % diagonals
	dz2 = sum((M(j+1,i,:) - M(j,i+1,:)).^2);
	U(2*j,2*i) = (sqrt(dz1)+sqrt(dz2))/(2 * sqrt(2));
      end
    end
  end

elseif strcmp(lattice, 'hexa') % haexagonal lattice

  if ~strcmp(shape,'rect'), 
    warning(['Using rectangular shape instead of ', shape, '.']);
  end

  for j=1:y, for i=1:x,
      if i<x,
	dx = sum((M(j,i,:) - M(j,i+1,:)).^2); % horizontal
	U(2*j-1,2*i) = sqrt(dx);
      end

      if j<y, % diagonals
	dy = sum((M(j,i,:) - M(j+1,i,:)).^2);
	U(2*j,2*i-1) = sqrt(dy);	
	
	if rem(j,2)==0 & i<x,
	  dz= sum((M(j,i,:) - M(j+1,i+1,:)).^2); 
	  U(2*j,2*i) = sqrt(dz);
	elseif rem(j,2)==1 & i>1,
	  dz = sum((M(j,i,:) - M(j+1,i-1,:)).^2); 
	  U(2*j,2*i-2) = sqrt(dz);
	end
      end
    end
  end
  
end

if strcmp(lattice, 'rect') & (uy == 1 | ux == 1),
  % in 1-D case, mean is equal to median 

  ma = max([ux uy]);
  for i = 1:2:ma,
    if i>1 & i<ma, U(i) = mean([U(i-1) U(i+1)]); 
    elseif i==1, U(i) = U(i+1); 
    else U(i) = U(i-1); % i==ma
    end
  end    

elseif strcmp(shape, 'rect') & strcmp(lattice, 'rect')

  for j=1:2:uy, for i=1:2:ux,
      if i>1 & j>1 & i<ux & j<uy,    % middle part of the map
      a = [U(j,i-1) U(j,i+1) U(j-1,i) U(j+1,i)];        
    elseif j==1 & i>1 & i<ux,        % upper edge
      a = [U(j,i-1) U(j,i+1) U(j+1,i)];
    elseif j==uy & i>1 & i<ux,       % lower edge
      a = [U(j,i-1) U(j,i+1) U(j-1,i)];
    elseif i==1 & j>1 & j<uy,        % left edge
      a = [U(j,i+1) U(j-1,i) U(j+1,i)];
    elseif i==ux & j>1 & j<uy,       % right edge
      a = [U(j,i-1) U(j-1,i) U(j+1,i)];
    elseif i==1 & j==1,              % top left corner
      a = [U(j,i+1) U(j+1,i)];
    elseif i==ux & j==1,             % top right corner
      a = [U(j,i-1) U(j+1,i)];
    elseif i==1 & j==uy,             % bottom left corner
      a = [U(j,i+1) U(j-1,i)];
    elseif i==ux & j==uy,            % bottom right corner
      a = [U(j,i-1) U(j-1,i)];
    else
      a = 0;
    end
    if m == 1, U(j,i) = mean(a); else U(j,i) = median(a); end
  end, end

elseif strcmp(shape, 'rect') & strcmp(lattice, 'hexa')

  for j=1:2:uy, for i=1:2:ux,
    if i>1 & j>1 & i<ux & j<uy,      % middle part of the map
      a = [U(j,i-1) U(j,i+1)];
      if rem(j-1,4)==0, a = [a, U(j-1,i-1) U(j-1,i) U(j+1,i-1) U(j+1,i)];
      else a = [a, U(j-1,i) U(j-1,i+1) U(j+1,i) U(j+1,i+1)]; end       
    elseif j==1 & i>1 & i<ux,        % upper edge
      a = [U(j,i-1) U(j,i+1) U(j+1,i-1) U(j+1,i)];
    elseif j==uy & i>1 & i<ux,       % lower edge
      a = [U(j,i-1) U(j,i+1)];
      if rem(j-1,4)==0, a = [a, U(j-1,i-1) U(j-1,i)];
      else a = [a, U(j-1,i) U(j-1,i+1)]; end
    elseif i==1 & j>1 & j<uy,        % left edge
      a = U(j,i+1);
      if rem(j-1,4)==0, a = [a, U(j-1,i) U(j+1,i)];
      else a = [a, U(j-1,i) U(j-1,i+1) U(j+1,i) U(j+1,i+1)]; end
    elseif i==ux & j>1 & j<uy,       % right edge
      a = U(j,i-1);
      if rem(j-1,4)==0, a=[a, U(j-1,i) U(j-1,i-1) U(j+1,i) U(j+1,i-1)];
      else a = [a, U(j-1,i) U(j+1,i)]; end
    elseif i==1 & j==1,              % top left corner
      a = [U(j,i+1) U(j+1,i)];
    elseif i==ux & j==1,             % top right corner
      a = [U(j,i-1) U(j+1,i-1) U(j+1,i)];
    elseif i==1 & j==uy,             % bottom left corner
      if rem(j-1,4)==0, a = [U(j,i+1) U(j-1,i)];
      else a = [U(j,i+1) U(j-1,i) U(j-1,i+1)]; end
    elseif i==ux & j==uy,            % bottom right corner
      if rem(j-1,4)==0, a = [U(j,i-1) U(j-1,i) U(j-1,i-1)];
      else a = [U(j,i-1) U(j-1,i)]; end
    else
      a=0;
    end
    if m == 1, U(j,i) = mean(a); else U(j,i) = median(a); end
end, end

end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% normalization between [0,1]

% U = U - min(min(U)); 
% ma = max(max(U)); if ma > 0, U = U / ma; end
