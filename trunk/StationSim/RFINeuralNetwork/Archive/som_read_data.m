function sData = som_read_data(filename, varargin)

%SOM_READ_DATA Reads a data file.
%
% sData = som_read_data(filename, dim, [missing])
% sData = som_read_data(filename, [missing])
%
% ARGUMENTS ([]'s are optional)
%
%  filename  (string) input filename
%  dim       (scalar) input data dimension
%  [missing] (string) a string denoting a dont'care value in the input 
%            data file (these are replaced by NaN's in the file read);
%            default is 'x'
%
% RETURNS
% 
%  sData     (struct) data structure
%
% If the argument 'dim' is not given, the data dimension should be 
% present on the first line of the input data. If it isn't, 
% the routine tries to determine data dimension automatically.
%
% Note: if the 'dim' is given, the data file is not allowed to have
% the data dimension on the first line of the input data.
%
% The argument 'missing', if present, must always be a string.
%
% The data struct field 'normalization' is initialized to empty value.
% If desired, function 'som_normalize_data' can be used to normalize
% data, but the normalization method must be specified as its argument.
%
% See also SOM_WRITE_DATA, SOM_READ_COD, SOM_WRITE_COD, 
%          SOM_DATA_STRUCT, SOM_NORMALIZE_DATA.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 170797, 110997, 190997, kk 161097, ecco 221097

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(1, 3, nargin))  % check no. of input args is correct

def_dont_care  = 'x';    % default don't care string
comment_start  = '#';    % the char a SOM_PAK command line starts with
comp_name_line = '#n';   % string used to start a special command line,
                         % which contains names of each component
block_size     = 1000;   % block size used in file read

% open input file

fid = fopen(filename);
if fid < 0
  error(['Cannot open ' filename]); 
end

% process input argumens

if nargin == 1
  dont_care = def_dont_care;
elseif nargin == 2 
  if isstr(varargin{1})
    dont_care = varargin{1};
  else
    dim      = varargin{1};
    dont_care = def_dont_care;
  end
elseif nargin == 3
  dim       = varargin{1};
  dont_care = varargin{2};
end
  
% if the data dimension is not specified, find out what it is

if nargin == 1 | (nargin == 2 & isstr(varargin{1}))

  fpos1 = ftell(fid);             % read first non-comment line
  first_data = [];
  while isempty(first_data)
    first_line               = fgetl(fid);
    if ~isempty(first_line)
      [first_data, first_labs] = scan_line(first_line, dont_care);
    end
    if feof(fid), break; end;
  end

  if isempty(first_data) & feof(fid) % check if the file was empty
    warning([filename ' is an empty file']);
    sData = [];
    return;
  end

  fpos2 = ftell(fid);             % read second non-comment line
  second_data = [];
  while isempty(second_data)
    second_line                = fgetl(fid);
    if feof(fid) break; end;
    [second_data, second_labs] = scan_line(second_line, dont_care);
  end

  l1 = length(first_data);
  if feof(fid)                    % if there was only one data line
    dim = l1;
    fseek(fid, fpos1, -1); 
  else
    l2 = length(second_data);     % if there were more than one data line
    if (l1 == 1 & l2 ~= 1) | (l1 == l2 & l1 == 1 & first_data == 1)
      dim = first_data;
      fseek(fid, fpos2, -1);
    elseif (l1 == l2)
      dim = l1;
      fseek(fid, fpos1, -1);
      warning(['Automatically determined data dimension is ' num2str(dim) ...
	    '; is it correct?']); 
    else
      error(['Invalid header line: ' first_line]);
    end
  end
end 

% check the dimension is valid

if dim < 1 | dim ~= round(dim) 
  error(['Illegal data dimension: ' num2str(dim)]);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% read data

sData      = som_data_struct(zeros(1, dim), filename); 

lnum       = 0;                                    % data vector counter
data_temp  = zeros(block_size, dim);

fprintf(2, '     0 lines read');
while 1;
  line = fgetl(fid);                               % read next line
  if ~isempty(line) & line == -1, break, end;      % is this the end of file? 
  
  [data, labs] = scan_line(line, dont_care, dim);  % process line
  
  if ~isempty(data)

    if length(data) == dim
      lnum = lnum + 1;               % this was a line containing data vector
      data_temp(lnum, 1:dim) = data; % add data to struct
      if ~rem(lnum, block_size)
	fprintf(2,'\r%6d', lnum); 
	data_temp(lnum+1:lnum+block_size, 1:dim) = zeros(block_size, dim);
      end
    else
      error(['Missing vector components on input file line ' num2str(lnum+1)]);
    end

    if ~isempty(labs)                 % insert labels into data struct
      for i = 1:length(labs)
	if labs{i}(1) ~= comment_start
	  labs_temp{lnum,i} = labs{i};
	else
	  break;
	end
      end
    else
      labs_temp{lnum, 1} = '';
    end  
  
  elseif ~isempty(labs)               % if there was no data at all
    if strmatch(comp_name_line, labs{1}) 
      l = length(labs);
      if (l - 1) ~= dim
	error('Illegal number of component names');
      else
	for i = 2:l
	  sData.comp_names{i - 1}  = labs{i};
	end
      end
    end
  end
end

fprintf(2,'\r%6d', lnum); 

% close input file

if fclose(fid) < 0
  error(['Cannot close file ' filename]);
else
  fprintf(2, '\rdata read ok         \n');
end

% adjust data matrix

last       = lnum - rem(lnum, block_size);
sData.data = data_temp(1:lnum, :);

sData.labels     = labs_temp;

sData.comp_names = reshape(sData.comp_names, dim, 1);

return; % all done

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% a subfunction used to process a data line

function [data, labs] = scan_line(line, dont_care, varargin)

% replace don't care strings with NaNs

line = [' ' line ' '];
m    = {};
n    = 0;
ind  = find((line > 8 & line < 14) | line == 32);
i    = 1;
len  = length(ind);

% read tokens of the line into a cell array

while i < len
  start  = ind(i) + 1;
  finish = ind(i + 1) - 1;
  if start <= finish
    n    = n + 1;
    m{n} = line(start:finish);
    if strcmp(m{n}, dont_care), m{n} = 'NaN'; end;
  end
  i = i + 1;
end

if nargin == 3 & varargin{1} <= n
  dim = varargin{1};
else
  dim = n;
end

% if nothing was read, return

data = [];
if n == 0  
  labs = {};
  return;
end

% extract data

i = 1;
while dim >= i 
  temp = eval(m{i}, '[]');
  if ~isempty(temp)
    data(i) = temp;
    i       = i + 1;
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