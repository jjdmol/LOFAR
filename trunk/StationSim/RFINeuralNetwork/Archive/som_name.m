function sMD = som_name(sMD, field, contents)

%SOM_NAME Name a field of self-organizing map or data struct.
%
% sMD = som_name(sMD, field, contents)
%
% ARGUMENTS 
%
%  sMD       (struct) self-organizing map or data structure
%  field     (string) field to be named, 'name' or 'comp_names' 
%             or 'data_name'
%  contents  (string or cell array) if field is 'name' or 'data_name', 
%             this is a name string; if field is 'comp_names',
%             this is cell array of strings 
%
% RETURNS
%
%  sMap      (struct) updated self-organizing map or data structure
%
% See also SOM_CREATE, SOM_DATA_STRUCT.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 090997, 110997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(3, 3, nargin));  % check no. of input arguments is correct

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% set field values

ismap = isfield(sMD, 'codebook');         % is this a map or data struct?
if ismap                                  % compute dimension
  dim = size(sMD.codebook, ndims(sMD.codebook));
else
  dim = size(sMD.data, 2);
end

if strcmp(field, 'name')
  sMD.name = contents;
elseif strcmp(field, 'data_name')
  if ismap
    sMD.data_name = contents;
  else
    error('Data struct has no field data_name.')
  end
elseif strcmp(field, 'comp_names')
  if length(contents) == dim
    for i = 1:dim
      sMD.comp_names{i} = contents{i};
    end
  else
    error('Illegal number of component names.');
  end
else 
  error(['Illegal field: ' field]);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



