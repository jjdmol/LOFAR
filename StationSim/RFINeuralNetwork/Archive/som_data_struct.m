function sData = som_data_struct(data, varargin)

%SOM_DATA_STRUCT Create a self-organizing map data struct.
%
% sData = som_data_struct(data, [name], [labels], [comp_names], ...
%                         [normalization])
%
% ARGUMENTS ([]'s are optional) 
%
%  data             (scalar) data matrix, size Q x dim
%  [name]           (string) dataset name
%  [labels]         (cell matrix) labels for each data vector, 
%                    size Q x m
%  [comp_names]     (cell array) data component names       
%  [normalization]  (string) name of the normalization method: 
%                    - som_var_norm  variance normalization
%                    - som_lin_norm  normalization to unit cube
%                    - som_hist_norm histogram normalization
%                    - som_unit_norm vector length normalization
%                    default: som_var_norm
% RETURNS   
%
%  sData            (struct) a data structure which consists of 
%                    fields below
%   .data           (matrix) data matrix, size Q x dim
%   .name           (string) dataset name
%   .labels         (cell matrix) labels for each data vector size Q x m 
%   .comp_names     (cell array) name for each component, 
%                    cell array of strings, size dim x 1 
%   .normalization  (struct) structure with the following fields:
%    .name            (string) name of the normalization method
%    .inv_params      (matrix) matrix of the parameters for the inverse
%                      transform; if data has not yet been normalized
%                      inv_params = [] 
%
% EXAMPLES
%
%  D      = rand(8, 4);
%  labs   = cell(8, 1); labs{1, 1} = 'first_label';
%  cnames = {'first', 'second', 'third', 'fourth'}
%
%  sData  = som_data_struct(D); 
%  sData  = som_data_struct(D, 'my_data', labs, cnames);
%  sData  = som_data_struct(D, [], [], cnames,'som_hist_norm');
%
% See also SOM_CREATE.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 071197

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 5, nargin));  % check no. of input arguments is correct

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% create data struct

sData = struct('data', [], 'name', '', 'labels' , [], 'comp_names', [],...
    'normalization', '');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize variables

[samples dim] = size(data);

% data

sData.data = data;

% dataset name

if nargin > 1 & ~isempty(varargin{1})
  sData.name = varargin{1};
else
  if ~isempty(inputname(1))
    sData.name = inputname(1);
  else 
    sData.name = 'unknown';
  end
end

% labels

if nargin > 2 & ~isempty(varargin{2})
  s = size(varargin{2});
  if iscell(varargin{2}) & s(1) == samples
    sData.labels = varargin{2};
    %sData.labels = cell(s);
    %for i = 1:samples
    %  for j = 1:s(2)
    %    if iscell(varargin{2}{i, j}),
    %      sData.labels{i, j} = varargin{2}{i, j};
    %    else
    %      sData.labels{i, j} = {varargin{2}{i, j}};
    %    end
    %  end
    %end
  else
    error('Invalid labels');
  end
else
  sData.labels = cell(samples, 1); 
end

% component names

sData.comp_names = cell(dim, 1);
if nargin > 3 & ~isempty(varargin{3})
  if iscell(varargin{3}) & length(varargin{3}) == dim
    for i = 1:dim
      sData.comp_names{i} = varargin{3}{i};
    end
  else
    error('Invalid component names.')
  end
else
  for i = 1:dim
    sData.comp_names{i} = sprintf('Var%d', i);
  end
end

% normalization

if nargin == 5
  if strcmp(varargin{4},'som_var_norm') ...
	| strcmp(varargin{4},'som_lin_norm') ...
	| strcmp(varargin{4},'som_hist_norm')...
	| strcmp(varargin{4},'som_unit_norm')
    method = varargin{4};
  else
    error(['Unknown normalization method: ' varargin{4}]);
  end
else
  method = 'som_var_norm'; % default normalization method
end

sData.normalization = struct('name', method, 'inv_params', []);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



