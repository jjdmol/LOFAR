function som_write_cod(sMap, filename)

%SOM_WRITE_COD Writes a map struct into a file in SOM_PAK format.
%
% som_write_cod(sMap, filename)
%
% ARGUMENTS
%
%  sMap       (struct) self-organizing map struct
%  filename   (string) output file name
%
% RETURNS
%
%  nothing
%
% See also SOM_READ_COD, SOM_WRITE_DATA, SOM_READ_DATA.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 170797, johan 050697, ecco 110997, ecco 221097

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 2, nargin))  % check no. of input args is correct

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize variables

msize = sMap.msize;                                % map grid size
mdim  = size(msize, 2);                            % map grid dimension
dim   = size(sMap.codebook, ndims(sMap.codebook)); % input space dimension

% map dimension check:
% map dimensions higher than 2 are not supported by SOM_PAK

if mdim > 2      
  error('Cannot write maps with higher dimension than two');
end

% codebook and label array modification
% (only this way codebook is written correctly)

msize    = fliplr(msize);
codebook = zeros([msize dim]);
for i = 1:dim
  codebook(:,:,i) = sMap.codebook(:,:,i)';
end
sMap.labels   = sMap.labels';

% open output file

fid = fopen(filename, 'w');
if fid < 0
  error(['Cannot open file ' filename]);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% write map into a file

% write header

fprintf(fid, '%d ', dim);                    % codebook dimension

if strcmp(sMap.lattice, 'hexa')              % lattice type
  fprintf(fid, 'hexa ');
elseif strcmp(sMap.lattice, 'rect')
  fprintf(fid, 'rect ');
end

fprintf(fid, '%d ', msize);                  % map size

% neighborhood type ('ep' and 'cutgauss' are not supported by SOM_PAK; 
% they are converted to 'gaussian')

if strcmp(sMap.neigh, 'gaussian') | strcmp(sMap.neigh,'ep') ...
   | strcmp(sMap.neigh,'cutgauss') % neighborhood type
  fprintf(fid, 'gaussian\n');
elseif strcmp(sMap.neigh, 'bubble')
  fprintf(fid, 'bubble\n');
end

% write the component names as a SOM_PAK command line

fprintf(fid,'#n ');
fprintf(fid, '%s ', sMap.comp_names{:});
fprintf(fid,'\n');

% write codebook

for j = 1:msize(2)
  for i = 1:msize(1)
    fprintf(fid, '%g ', codebook(i, j, :));
    lab = sMap.labels{i,j};
    if exist('lab') & ~isempty(lab)
      fprintf(fid, '%s ', lab{:});
    end
    fprintf(fid, '\n');
  end
end

% close file

if fclose(fid) 
  error(['Cannot close file ' filename]);
else
  fprintf(2, 'map write ok\n');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%






