function sMap = som_create(dim, msize, varargin)

%SOM_CREATE Create self-organizing map structure. 
% 
% sMap = som_create(dim, msize, [lattice], [shape])
%
% ARGUMENTS ([]'s are optional)
%
%  dim              (scalar) input space dimension
%  msize            (vector) map grid size
%                    NOTE: the matrix notation of indices is used
%  [lattice]        (string) map lattice; 'rect' or 'hexa'
%  [shape]          (string) map shape; 'rect' or 'cyl' or 'toroid'
%
% RETURNS   
%
%  sMap             (struct) self-organizing map structure that
%                    consists of fields below
%   .codebook       (matrix) weight vectors, size n1 x n2 x .. x nk x dim
%   .labels         (cell matrix) labels, size n1 x n2 x .. x nk 
%   .msize          (vector) dimensions of the map grid, size k x 1 
%   .init_type      (string) initialization type, 'linear' or 'random'
%   .train_type     (string) training type, 'seq' or 'batch'
%   .lattice        (string) map lattice, 'rect' or 'hexa'
%   .shape          (string) map shape, 'rect' or 'cyl' or 'toroid'
%   .neigh          (string) neighborhood function, 'gaussian' or 
%                    'cutgauss' or 'bubble' or 'ep'
%   .train_sequence (cell array) cell array of structs, size p x 1 
%   .name           (string) map name
%   .data_name      (string) dataset name
%   .normalization  (struct) data normalization struct 
%   .mask           (vector) mask vector used in BMU search, size dim x 1
%   .comp_names     (cell array) names for each component, 
%                    cell array of strings, size dim x 1 
%
% EXAMPLES
%
%  sMap = som_create(4, [3 4]);
%  sMap = som_create(1, 100);
%  sMap = som_create(5, [40 5 18], 'hexa'); 
%  sMap = som_create(3, [10 2], 'hexa', 'rect'); 
%  sMap = som_create(2, 7, '', 'rect');
%
% See also SOM_DATA_STRUCT.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 160797, 100997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 5, nargin))  % check no. of input args is correct

if dim < length(msize)
  error('Map dimension is greater than input space dimension.');
end
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% create structure  
  
sMap = struct('init_type', 'linear', 'train_type', 'batch', 'lattice' ,...
    'hexa', 'shape', 'rect', 'neigh', 'gaussian', 'msize', msize, ...
    'train_sequence', [], 'codebook', [], 'labels', [], ...
    'mask', [], 'data_name', 'unnamed', 'normalization', [], ...
    'comp_names', [], 'name', 'unnamed');

% default values defined here:
%
% init_type   linear   <- this may be changed in som_init
% lattice     hexa
% shape       rect
% neigh       gaussian <- these two may be changed using som_trainops
% train_type  batch    <-

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize structure variables

% codebook

sMap.codebook     = zeros([msize dim]);

% labels

sMap.labels     = cell(msize);

% component default weights

sMap.mask = ones(dim, 1);

% component default names

sMap.comp_names   = cell(dim, 1);
for i = 1:dim
  sMap.comp_names{i} = sprintf('Var%d', i);
end

% map default name

sMap.name = sprintf('SOM %s', datestr(now, 1));

% lattice

if nargin > 2 & ~isempty(varargin{1})
  if strcmp(varargin{1}, 'rect') | strcmp(varargin{1}, 'hexa')
    sMap.lattice = varargin{1};
  else
    error(['Invalid lattice: ' varargin{1}]);
  end
end
  
% shape

if nargin > 3 & ~isempty(varargin{2})
  if strcmp(varargin{2}, 'rect') | strcmp(varargin{2}, 'cyl') ...
     | strcmp(varargin{2}, 'toroid'), 
    sMap.shape = varargin{2};
  else
    error(['Invalid shape: ' varargin{2}]);
  end
end

% normalization struct

sMap.normalization = struct('name', '', 'inv_params', []);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
