function sMap = som_read_cod(filename)

%SOM_READ_COD Reads a SOM_PAK format codebook file.
%
% sMap = som_read_cod(filename)
%
% ARGUMENTS   
%
%  filename (string) input filename
%
% RETURNS     
%
%  sMap     (struct) a self-organizing map structure.
%
% The first line of the file must be the header line in SOM_PAK format.
%
% Empty lines and lines starting with a '#' are ignored, except 
% the ones starting with '#n'. The strings after '#n' are read to 
% field comp_names of the map structure.
%
% See also SOM_WRITE_COD, SOM_READ_DATA, SOM_WRITE_DATA, SOM_CREATE.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Johan 030697, ecco 170797, ecco 100997, ecco 221097

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 1, nargin))  % check no. of input args is correct

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize variables

lnum           = 0;    % codebook vector counter
comment_start  = '#';  % the char a SOM_PAK command line starts with
comp_name_line = '#n'; % string used to start a special command line,
                       % which contains names of each component

% open input file

fid = fopen(filename);
if fid < 0, error(['Cannot open ' filename]); end

% read header line

line = fgetl(fid);
[dim, header] = scan_line(line, 8);
if size(header, 2) ~= 4
  error([ 'Header ' line ' not valid']); 
end                                 

% create map struct and set its fields according to header line

msize           = [str2num(header{3}) str2num(header{2})];
sMap            = som_create(dim, msize);
sMap.name       = filename;
sMap.shape      = 'rect';
sMap.train_type = 'seq';
sMap.init_type  = 'unknown';
sMap.data_name  = 'unknown';
munits          = prod(msize);
sMap.lattice    = header{1};
sMap.neigh      = header{4};
codebook        = zeros(munits, dim);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% read codebook from the file

fprintf(2, '     0 lines read');
while 1; 
  line = fgetl(fid);                        % read next line
  if ~isempty(line) & line == -1 break; end % is this the end-of-file?

  [data, labs] = scan_line(line, dim);      % process line

  if ~isempty(data)
    if length(data) == dim
      lnum = lnum + 1;         % this was a line containing a codebook vector
      codebook(lnum,:) = data; % add data to struct
      if ~rem(lnum, 1000)
	fprintf(2,'\r%6d', lnum); 
      end
    else
      error(['Missing vector components on codebook file line ' num2str(lnum+1)]);
    end
      
    if ~isempty(labs)                 % insert labels into map struct        
      for i = 1:length(labs)
	if labs{i}(1) ~= comment_start
	  sMap.labels{lnum}{i} = labs{i};
	else
	  break;
	end
      end
    else
      sMap.labels{lnum} = cell(1);
    end

  elseif ~isempty(labs)               % if there was no data at all
    if strmatch(comp_name_line, labs{1}) 
      l = length(labs);
      if (l - 1) ~= dim
	error('Illegal number of component names');
      else
	for i = 2:l
	  sMap.comp_names{i - 1} = labs{i};
	end
      end
    end
  end
end

fprintf(2,'\r%6d', lnum); 

% close the input file

if fclose(fid) < 0
  error(['Cannot close file ' filename]); 
else
  fprintf(2, '\rmap read ok         \n');
end

% check the number of lines read was correct

if lnum ~= munits
  error(['Illegal number of map units: ' num2str(lnum) ' (should be ' num2str(munits) ').']);
end

% reshape to SOM Toolbox format 

codebook      = reshape(codebook, [fliplr(msize) dim]);
sMap.codebook = zeros([msize dim]);
for i = 1:dim
  sMap.codebook(:, :, i) = codebook(:, :, i)';
end
sMap.labels   = reshape(sMap.labels, fliplr(msize))';

return; % all done

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% a subfunction used to process a codebook line

function [data, labs] = scan_line(line, dim)

line = [' ' line ' '];
m    = {};
n    = 0;
ind  = find((line > 8 & line < 14) | line == 32);
i    = 1;
len  = length(ind);

% read tokens into a cell array

while i < len
  start  = ind(i) + 1;
  finish = ind(i + 1) - 1;
  if start <= finish
    n = n + 1;
    m{n} = line(start:finish);
  end
  i = i + 1;
end

% if nothing was read, return

if n == 0  
  data = [];
  labs = {};
  return;
end

% extract data 

data = [];
i = 1;
while i <= n & i <= dim
  temp = eval(m{i}, '[]');
  if ~isempty(temp)
    data(i) = temp;
    i = i + 1;
  else
    break;
  end
end

% extract labels

if n == i - 1
  labs = {};
else
  j = 1;
  while i <= n
    labs{j} = m{i};
    i = i + 1;
    j = j + 1;
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
