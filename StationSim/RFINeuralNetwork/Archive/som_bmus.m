function [Bmus,Qerrors] = som_bmus(sMap, sData, which_bmus, mask)

%SOM_BMUS Find the best-matching units from the map for the given vectors.
%
% [Bmus, Qerrors] = som_bmus(sMap, sData, [which], [mask])
%
% ARGUMENTS ([]'s are optional)
%
%  sMap     (struct or matrix) self-organizing map structure or the
%            corresponding codebook matrix, size n1 x ... x nk x dim
%  sData    (struct or matrix) data structure, or a matrix of data 
%            vectors, size dlen x dim
%  [which]  (vector) which BMUs are returned, by default only the first
%  [mask]   (vector) mask vector used in distance calculation, by
%            default read from sMap
% RETURNS
%
%  Bmus     (matrix) the requested BMUs for each data vector, 
%            size dlen x length(which)
%  Qerrors  (matrix) the corresponding quantization errors
%
% NOTE: for an unknown vector (all components are NaN's) 
% the bmu=NaN and qerror=NaN.
%
% NOTE: the function returns the linear index into the codebook
% matrix, _not_ the multidimensional matrix subscripts. You can get the
% multidimensional subscripts using function som_ind2sub: 
%   subs = som_ind2sub(sM.msize,bmu);
% Conversion the other way around can be made using som_sub2ind function: 
%   lin_index = som_sub2ind(sM.msize,[i1 i2]);
%
% EXAMPLES
%
%  bmu = som_bmus(sM, [0.3 -0.4 1.0]);
%           % 3-dimensional data, returns BMU for vector [0.3 -0.4 1]
%  bmu = som_bmus(sM, [0.3 -0.4 1.0], [3 5]);
%           % as above, except returns the 3rd and 5th BMUs
%  bmu = som_bmus(sM, [0.3 -0.4 1.0], [], [1 0 1]);
%           % as above, except ignores second component in searching
%  [bmus qerrs] = som_bmus(sM, D);
%           % returns BMUs and corresponding quantization errors 
%           % for each vector in D
%  bmus = som_bmus(sM, sD);
%           % returns BMUs for each vector in sD using the mask in sM
%
% See also SOM_AUTOLABEL, SOM_HITS, SOM_SUB2IND, SOM_IND2SUB.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 071197, 101297 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments and initialize

error(nargchk(1, 4, nargin));  % check no. of input args is correct

% sMap
if isstruct(sMap), M = sMap.codebook; else M = sMap; end
msize = size(M);                      % weight vectors
map_vector_dim = msize(length(msize));% dimension of the weight vectors
msize = msize(1:(length(msize)-1));   % msize = (n1, ...,  nk)
mdim = length(msize);                 % grid dimension
munits = prod(msize);                 % number of map units
M = reshape(M,munits,map_vector_dim); % change the shape of the matrix
if any(any(isnan(M))), 
  error ('Map weight matrix must not have missing components.');
end

% data
if isstruct(sData), D = sData.data; else D = sData; end
[dlen dim] = size(D);
if dim ~= map_vector_dim, 
  error('Data and map dimensions do not match.')
end

% which_bmus
if nargin < 3 | isempty(which_bmus) | any(isnan(which_bmus)), which_bmus = 1; end

% mask
if nargin < 4 | isempty(mask) | any(isnan(mask)), 
  if isstruct(sMap), mask = sMap.mask; else mask = ones(dim,1); end
end
if all(mask == 0), 
  error('All components masked off. BMU search cannot be done.');
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

% The BMU search involves calculating weighted Euclidian distances 
% to all map units for each data vector. Basically this is done as
%   for i=1:dlen, 
%     for j=1:munits, 
%       for k=1:dim
%         Dist(j,i) = Dist(j,i) + mask(k) * (D(i,k) - M(j,k))^2;
%       end
%     end
%   end
% where mask is the weighting vector for distance calculation. However, taking 
% into account that distance between vectors m and v can be expressed as
%   |m - v|^2 = sum_i ((m_i - v_i)^2) = sum_i (m_i^2 + v_i^2 - 2*m_i*v_i)
% this can be made much faster by transforming it to a matrix operation:
%   Dist = (M.^2)*mask*ones(1,d) + ones(m,1)*mask'*(D'.^2) - 2*M*diag(mask)*D'
%
% In the case where there are unknown components in the data, each data
% vector will have an individual mask vector so that for that unit, the 
% unknown components are not taken into account in distance calculation.
% In addition all NaN's are changed to zeros so that they don't screw up 
% the matrix multiplications.

% handle unknown components
unknown = find(sum(isnan(D'))==dim);        % completely unknown vectors
Wmap = (mask*ones(1,dlen)) .* (~isnan(D))'; % mask NaN's out
D(find(isnan(D))) = 0;                      % set NaN's to zeros 

% calculate distances
Dist = sqrt((M.^2)*Wmap + ones(munits,1)*mask'*(D'.^2) - 2*M*diag(mask)*D');

% find the bmus and the corresponding quantization errors
if length(which_bmus) == 1 & which_bmus == 1,
  [Qerrors Bmus] = min(Dist); % a lot faster than sort
else
  [Qerrors Bmus] = sort(Dist);   
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% select return arguments

if munits==1, 
  Bmus = ones(dlen,1);  %one unit only
else 
  Bmus = Bmus(which_bmus,:)'; 
end  
Qerrors = Qerrors(which_bmus,:)';

% completely unknown vectors

if ~isempty(unknown), 
  Bmus(unknown,:) = NaN;
  Qerrors(unknown,:) = NaN;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
