function som_info(sMD)

%SOM_INFO Diplays information of a self-organizing map or data structure.
% 
% som_info(sMD)
%
% ARGUMENTS
%
%  sMD  (struct) self-organizing map or data struct
%
% Prints out information about the given structure.
%
% See also SOM_CREATE, SOM_DATA_STRUCT.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 160797, 110997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 1, nargin))  % check no. of input args is correct

if ~isstruct(sMD)
  error('Input argument is not a struct.')
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% print struct information

ismap = isfield(sMD, 'codebook');   % is this a map or data struct?

if ismap
  mdim = length(sMD.msize);
  dim  = size(sMD.codebook, ndims(sMD.codebook));
  l    = length(sMD.train_sequence);

  fprintf(1,'The struct %s is a self-organizing map struct.\n\n',inputname(1));
  fprintf(1,'Map name                              : %s\n', sMD.name);
  fprintf(1,'Data used in intialization/training   : %s\n\n', sMD.data_name);
  fprintf(1,'Shape (rect/cyl/toroid)               : %s\n', sMD.shape);
  fprintf(1,'Lattice type (rect/hexa)              : %s\n', sMD.lattice);
  fprintf(1,'Neighborhood type                     : %s\n\n', sMD.neigh);
  fprintf(1,'Map grid dimension                    : %d\n', mdim);
  fprintf(1,'Map grid size                         : ');
  for i = 1:mdim - 1
    fprintf(1,'%d x ',sMD.msize(i));
  end
  fprintf(1,'%d\n\n', sMD.msize(mdim));
  fprintf(1,'Codebook dimension                    : %d\n', dim);
  for i = 1:dim,
    fprintf(1,'Component %3d name                    : %s\n', i, sMD.comp_names{i});
  end
  if strcmp(sMD.init_type, 'unknown')
    fprintf(1, '\nThe map has been produced using SOM_PAK program package.\n\n');
  else
    fprintf(1,'\nInitialization type (random/linear)   : %s\n', sMD.init_type);
    fprintf(1,'Training type       (seq/batch)       : %s\n\n', sMD.train_type);
    if l > 0
      fprintf(1,'The map has been trained %d time(s)\n', l);
      fprintf(1,'Last training at                      : %s\n', sMD.train_sequence{l}.time);
    else
      fprintf(1,'The map has been initialized, but not trained.\n');
    end
  end
else
  dim      = size(sMD.data, 2);
  samples  = size(sMD.data, 1);
  ind      = find(~isnan(sum(sMD.data'))); 
  complete = size(sMD.data(ind,:),1);
  partial  = samples - complete;
  values   = prod(size(sMD.data));
  missing  = sum(sum(isnan(sMD.data))); 
  
  fprintf(1,'The struct %s is a data struct.\n\n',inputname(1));
  fprintf(1,'Data name              : %s\n', sMD.name);
  fprintf(1,'Sample vector dimension: %d\n', dim);
  fprintf(1,'Number of data vectors : %d\n', samples);
  fprintf(1,'Complete data vectors  : %d\n', complete);
  fprintf(1,'Partial data vectors   : %d\n', partial);  
  fprintf(1,'Complete values        : %d of %d (%d%%)\n\n', ...
      values-missing, values, 100 * (values - missing) / values); 

  for i = 1:dim,
    fprintf(1,'Component %3d name: %s\n', i, sMD.comp_names{i});
    missing  = sum(isnan(sMD.data(:,i)));
    complete = samples - missing;
    fprintf(1,' complete values:  %d of %d (%d%%)\n', complete, ...
	samples, 100 * complete / samples);
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%