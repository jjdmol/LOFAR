function som_write_data2(sData, filename, dont_care)

% som_write_data(data, filename, [dont_care])
%
% ARGUMENTS ([]'s are optional)
%
%  data        (struct or matrix) data to be written
%  filename    (string) output filename
%  [dont_care] (string) string used to denote missing components (NaNs); 
%               default is 'x'
%
% RETURNS
% 
%  nothing
%
% Any information on data normalization in data struct 'sData' is lost
% with this function; to preserve the information, save the data struct
% as a mat-file ('save datafile sData').
%
% See also SOM_READ_DATA, SOM_WRITE_COD, SOM_READ_COD, SOM_DATA_STRUCT.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco221097, ecco131197

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 3, nargin));  % check no. of input args is correct

if isstruct(sData),
  D = sData.data;
else
  D = sData;
end

[samples dim] = size(D);

if nargin == 2
  dont_care = 'x';
end

% open output file

fid = fopen(filename, 'w');
if fid < 0
  if nargout > 0 
    f = 0; 
    return;
  end;
  error(['Cannot open file ' filename]);
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% write data

% write dimension

fprintf(fid, '%d\n', dim);

% write component names

if isstruct(sData)
  fprintf(fid,'#n ');                     
  fprintf(fid, '%s ', sData.comp_names{:} );
  fprintf(fid,'\n');                      
end

% write data and labels

fprintf(2, '     0 lines written');

f1 = find(sum(isnan(D')));

is_struct = isstruct(sData);
not_is_empty_f1 = ~isempty(f1);

for i = 1:samples

  if ~rem(i, 1000) fprintf(2, '\r%6d', i); end
  
  if not_is_empty_f1 & any(f1==i)
    fprintf(fid, '%s', strrep(sprintf('%g ', D(i,:)), 'NaN', dont_care));
  else
    fprintf(fid, '%g ', D(i,:));
  end

  if is_struct
    fprintf(fid, '%s ', sData.labels{i,:});
  end
  
  fprintf(fid, '\n');
end

fprintf(2,'\r%6d', samples); 

% close file

if fclose(fid)
  error(['Cannot close file ' filename]);
else
  fprintf(2, '\rdata write ok        \n');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%






