function sMap = som_init(sData, varargin)

%SOM_INIT Create and initialize a self-organizing map.
%
% sMap = som_init(sData, [msize], [init_type], [lattice], [shape])
%
% ARGUMENTS ([]'s are optional) 
%
%  sData        (struct or matrix) data structure or data matrix, 
%                size dlen x dim
%  [msize]      (vector) map dimensions, 1 x k vector of doubles
%                NOTE: the matrix notation of indexes is used
%                by default, a heuristic formula is used to determine
%                the number of map units and the two principal eigenvalues
%                of the data set are used to determine the actual
%                sidelengths of the 2-dimensional map grid
%  [init_type]  (string) initialization type, 'linear' or 'random', 
%                default is 'linear'
%  [lattice]    (string) map lattice type, 'rect' or 'hexa', 
%                default is 'hexa'
%  [shape]      (string) map shape, 'rect' or 'cyl' or 'toroid',
%                default is 'rect'
%
% RETURNS
%
%  sMap         (struct) initialized self-organizing map structure
%
% EXAMPLES
%
%  D = rand(1000, 4);    % or D = som_data_struct(rand(1000,4));
%  sMap = som_init(D);
%  sMap = som_init(D, [10 5]);
%  sMap = som_init(D, 20, 'random');
%  sMap = som_init(D, 20, 'linear', 'hexa', 'rect');
%  sMap = som_init(D, [4 6 7], [], 'rect');
%
% See also SOM_CREATE, SOM_RANDINIT, SOM_LININIT, SOM_TRAIN.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% ecco 160797, 100997


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 5, nargin));  % check no. of input arguments is correct

% check are we given a matrix or a struct

if (isstruct(sData))
  D = sData.data;
else
  D = sData;
end

% init type

if nargin > 2 & ~isempty(varargin{2})
  init_type = varargin{2}; 
else
  init_type = 'linear';
end

% lattice type

if nargin > 3 & ~isempty(varargin{3}) 
  lattice = varargin{3};
else
  lattice = 'hexa';
end

[samples dim] = size(D);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% if the map size is not given, a 2-D map is made

if nargin == 1 | isempty(varargin{1})
  ind           = find(~isnan(sum(D'))); % indices of samples not having NaNs 
  if length(ind) < dim
    error('Cannot automatically determine map size; not enough complete samples.')
  end
  
  munits = 5 * samples ^ .543;  % this is just one way to make a guess...

  % initialize xdim/ydim ratio using principal components of the input
  % space; the ratio equals to ratio of two largest eigenvalues
  % NOTE the special case of toroid map with hexagonal lattice, which 
  % is handled later

  if dim == 1
    msize     = ceil(munits);     % 1-D data -> 1-D map
  else
    me        = mean(D(ind, :));      
    n         = size(D(ind, :), 1);       
    [U, S, V] = svd(((D(ind, :) - me(ones(n, 1), :)) ./ sqrt(n - 1)), 0);
    ratio     = S(1, 1) / S(2, 2);
    % in hexagonal lattice, the sidelengths are not directly 
    % proportional to the number of units since the units on the 
    % y-axis are squeezed together by a factor of sqrt(0.75)
    if strcmp(lattice,'hexa'), 
      msize(2)  = min(munits, round(sqrt(munits / ratio * sqrt(0.75))));
    else
      msize(2)  = min(munits, round(sqrt(munits / ratio)));
    end
    msize(1)  = round(munits / msize(2));

    % if actual dimension of the data is 1, make the map 1-D
    
    if min(msize) == 1, msize = [1 max(msize)]; end;

  end
else
  msize = varargin{1}; % the map size was given as an argument
  mdim  = length(msize);
  if mdim > dim
    error('Input space dimension is smaller than map codebook dimension.');
  elseif mdim == 1,
    msize = [1 msize];
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% set some variables

% set map shape

if nargin > 4 & ~isempty(varargin{4}) 
  shape = varargin{4};
else
  shape = 'rect';
end

% a special case: if the map is toroid with hexa lattice, 
% size along first axis must be even

if strcmp(lattice,'hexa') & strcmp(shape,'toroid') & mod(msize(1),2), 
  msize(1) = msize(1) + 1;
end

munits = prod(msize);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% create map struct

sMap           = som_create(dim, msize, lattice, shape);
sMap.init_type = init_type;

if (isstruct(sData))
  sMap.data_name = sData.name;
  for i = 1:dim
    sMap.comp_names{i} = sData.comp_names{i};
  end
else
  sMap.data_name = inputname(1);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% call initialization routine

if strcmp(init_type, 'random')             % random initialization
  sMap.codebook  = som_randinit(D, msize);
elseif strcmp(init_type, 'linear')         % linear initialization
  sMap.codebook  = som_lininit(D, msize);
else
  error(['Illegal initialization type: ' init_type]);
end

% make sure the codebook doesn't include any NaNs

if any(isnan(reshape(sMap.codebook, munits * dim, 1)))
  error('Map initialization failed; there may be a column of NaNs in the data.');
end

% add data normalization information to map struct

if isstruct(sData)
  sMap.normalization = sData.normalization;
end
  
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

