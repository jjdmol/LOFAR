function M = som_lininit(D, msize)

%SOM_LININIT Initialize self-organizing map (linearly).
%
% M = som_lininit(data, msize)
% 
% ARGUMENTS
% 
%  data   (matrix) Q x dim data matrix
%  msize  (vector) self-organizing map grid size, k x 1 vector
%          NOTE: the matrix notation of indexes is used
% 
% RETURNS  
%
%  M      (matrix) linearly initialized self-organizing map 
%          codebook matrix, size n1 x ... x nk x dim, where
%          [n1 ... nk] = msize and dim is the dimension of the data 
%          vectors
%
% The vectors of the codebook matrix are linearly initialized 
% in the subspace spanned by k first principal components of the data.
%
% See also SOM_RANDINIT, SOM_INIT.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta ecco 160797, 100997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 2, nargin));  % check no. of input args is correct

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% initialize some variables

% only NaN-free samples are used in the initialization

[n, dim] = size(D);              % no. of samples and data dimension
if dim > 1                       % indices of the samples not having NaN's
  ind = find(~isnan(sum(D')));     
else
  ind = find(~isnan(D));
end
n       = length(ind);           % no. of samples not having NaN's
Data    = D(ind,:);              % matrix of corresponding samples
me      = mean(Data);            % mean of each variable
munits  = prod(msize);           % no. of map units
M       = repmat(me, munits, 1); % initialize codebook to mean value
mdim    = length(msize);         % map grid dimension

if n < dim 
  error('Cannot perform linear initialization; not enough complete samples.');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% compute linear initialization

if dim > 1 & sum(msize > 1) > 1
  % compute principal components using singular value decomposition

  [U, S, V] = svd(((Data - me(ones(n, 1), :)) ./ sqrt(n - 1)), 0);

  % normalize eigenvectors to unit length and multiply them by 
  % corresponding eigenvalues
  
  for i = 1:dim
    Eigvec(:,i) = (V(:,i) / norm(V(:,i))) * S(i,i);
  end
else
  Eigvec = std(Data);
end
  
% initialize codebook vectors

Coords = som_unit_coords(msize, 'rect', 'rect');
Coords = reshape(Coords, munits, mdim);

for n = 1:munits               % compute codebook values
  for d = 1:mdim
    if msize(d) == 1
      xf = 0;
    else
      xf    = 4 * (Coords(n, d) - 1) / (msize(d) - 1) - 2;
    end
    M(n, :) = M(n, :) + xf * Eigvec(:, d)';
  end
end

% reshape codebook

M = reshape(M, [msize dim]);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

