function sMap = som_trainops(sMap, varargin)

%SOM_TRAINOPS Set self-organizing map train options.
%
% sMap = som_trainops(sMap, [neigh], [train_type], [mask])
%
% ARGUMENTS ([]'s are optional)
%
%  sMap         (struct) self-orgainzing map structure
%  [neigh]      (string) self-organizing map neighborhood function
%                'gaussian' or 'cutgauss' or 'bubble' or 'ep'
%  [train_type] (string) training algorithm, 'seq' or 'batch', 
%  [mask]       (vector) component mask vector, size dim x 1
%
% RETURNS
%
%  sMap         (struct) updated self-organizing map structure
%
% Notice that the train options may be given in _any_ order, and 
% some of them may be missing.
%
% See also SOM_TRAIN.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 160797, 110997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 4, nargin));  % check no. of input args is correct

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% set struct variables

i = nargin - 1;
while i >= 1
  if strcmp(varargin{i}, 'gaussian') | strcmp (varargin{i}, 'bubble') | ...
	strcmp (varargin{i}, 'ep') | strcmp (varargin{i}, 'cutgauss') 
    sMap.neigh = varargin{i};
  elseif strcmp(varargin{i}, 'seq') | strcmp (varargin{i}, 'batch')
    sMap.train_type = varargin{i};
  elseif ~isstr(varargin{i})
    l   = length(varargin{i});                       % no. of weights
    dim = size(sMap.codebook, ndims(sMap.codebook)); % input space dimension
    if l == dim
      for j = 1:dim
	sMap.mask(j) = varargin{i}(j);
      end
    else
      error('Illegal number of component weights.')
    end
  else
    error(['Illegal train option: ' varargin{i}]);
  end
  i = i - 1;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


