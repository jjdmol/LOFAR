function [quality] = som_quality(sMap, q_type, varargin)

%SOM_QUALITY Calculate a given quality measure for the map.
%
% quality = som_quality(sMap, q_type, [q_arg1, g_arg2, ...])
%
% ARGUMENTS ([]'s are optional)
%
%  sMap          (struct or matrix) som structure or the weight 
%                 vector matrix, size n1 x ... x nk x dim
%  q_type        (string) which quality measure is calculated, 
%                 'qe', 'energy' or 'topog', default is 'qe'
%  [q_arg1,
%   q_arg, ...]  (varies) some quantization measures need parameters 
%                 which are given with these arguments. Typically a 
%                 data set (struct or matrix) is given first.
% RETURNS
%
%  quality       (scalar) the quality of the map 
%
% The issue of SOM quality is a complicated one. Typically two
% evaluation criterias are used: resolution and topology preservation.
% If the dimension of the data set is higher than the dimension of the 
% map grid, these usually become contradictory goals. In this function
% three different kinds of quality measures have been implemented: 
%
%  'qe'     : The average quantization error for a given data set.
%             Argument: a data set
%  'topog'  : Topographic error, the proportion of all data vectors
%             for which first and second BMUs are not adjacent units.
%             Argument: a data set
%  'energy' : Kernel error, or the sum of distances from given   
%             data vectors to all map units weighted by the neighborhood
%             kernel about the BMU of the vector. In a sense the 
%             energy function of the SOM.
%             Arguments: 1. a data set 
%                        2. radius of the neighborhood kernel
%
% Please notice that when when calculating BMUs of data vectors, the
% mask of the given map is used.
%
% EXAMPLES
%
%  qe = som_quality(sMap,'qe',sData);   % quantization error
%  te = som_quality(M,'topog',D);       % topographic error
%  en = som_quality(sMap,'energy',D,3); % kernel error
%
% See also SOM_BMUS. 

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 141097, 220997
 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

% input arguments
if nargin < 2, error('Not enough input arguments.'); end

% sMap
if ~isstruct(sMap), 
  M = sMap;
  si = size(M); 
  sMap = som_create(si(length(si)), si(1:(length(si)-1)), 'rect', 'rect');
  sMap.codebook = M;
end

% quality measure arguments
switch q_type, 
case 'qe', 
  if nargin<3, 
    error('A data set needed to calculate average quantization error.'); 
  end
case 'energy', 
  if nargin<4, 
    error('A data set and neighborhood radius are needed to calculate kernel error.'); 
  end
case 'topog', 
  if nargin<3, 
    error('A data set needed to calculate topographic error.'); 
  end
otherwise,
  error(['Unknown quality measure: ', q_type]);
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

switch q_type, 
case 'qe', quality = meanqerror(sMap,varargin{1});
case 'energy', quality = kernelerror(sMap,varargin{1},varargin{2});
case 'topog', quality = topographic_error(sMap,varargin{1});
end

return;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% subfunctions 

%%%% MEAN QUANTIZATION ERROR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [error] = meanqerror(sMap,sData)

[bmus qerrors]= som_bmus(sMap,sData,1,sMap.mask);

inds = find(~isnan(qerrors));
if length(inds) == 0, warning('Empty data set.'); error = 0;
else error = sum(qerrors(inds)); end
error = mean(qerrors(inds));
%%%%

%%%% KERNEL ERROR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [error] = kernelerror(sMap,D,radius)

if isstruct(D), D = D.data; end
[dlen dim] = size(D);

munits = prod(sMap.msize);
M = reshape(sMap.codebook,[munits dim]);
mask = sMap.mask;
Unitdist = som_unit_distances(sMap.msize, sMap.lattice, sMap.shape); 
Unitdist = Unitdist.^2;
radius = radius^2;

qerrors = zeros(dlen,1)*NaN;
for i = 1:dlen, 
  % find BMU
  x = D(i,:);                                % sample vector
  known = ~isnan(x);                         % its known components
  Dx = M(:,known) - x(ones(munits,1),known); % difference between vector and all map units
  dist = sqrt(mask(known)'*(Dx'.^2));        % distance to each map unit
  if ~isempty(dist),
    [qe bmu] = min(dist);
    % neighborhood function 
    % notice that the elements of Unitdist and radius have been squared!
    switch sMap.neigh, 
    case 'bubble', 
      h = (Unitdist(bmu,:) <= radius);
    case 'gaussian', 
      h = exp( - Unitdist(bmu,:)/(2*radius));
    case 'cutgauss', % cut gaussian
      h = exp( - (Unitdist(bmu,:))/(2*radius)) .* (Unitdist(bmu,:) <= radius);
    case 'ep', % epanechnikov
      h = (1 - Unitdist(bmu,:)/radius) .* (Unitdist(bmu,:) <= radius);
    end
    qerrors(i) = h*dist';
  end
end

inds = find(~isnan(qerrors));
if length(inds) == 0, warning('Empty data set.'); error = 0;
else error = sum(qerrors(inds)); end
%%%%

%%%% TOPOGRAPHIC ERROR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [error] = topographic_error(sMap, sData)

Bmus = som_bmus(sMap, sData, [1 2], sMap.mask);
Neighbors = som_unit_neighborhood(sMap.msize,sMap.lattice,sMap.shape,1);

if isstruct(sData), [dlen dim] = size(sData.data); 
else [dlen dim] = size(sData); end

error = 0; n=0;
for i=1:dlen, 
  if ~isnan(Bmus(i,1)), 
    error = error + (Neighbors(Bmus(i,1),Bmus(i,2)) >= 2); 
    n = n+1;
  end
end

if n==0, warning('Empty data set.'); 
else error = error/n; end
%%%%
