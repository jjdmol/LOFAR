function [Hits] = som_hits(sMap, sData)

%SOM_HITS Calculate the number of hits from the data to each map unit.
%
% Hits = som_hits(sMap, sData)
%
% ARGUMENTS ([]'s are optional)
%
%  sMap   (struct or matrix) self-organizing map structure or 
%          the weight vector matrix, size n1 x ... x nk x dim
%  sData  (struct or matrix) data structure or matrix of 
%          data vectors, size dlen x dim
%
% RETURNS
%
%  Hits   (matrix) the number of hits in each map unit, 
%          size n1 x n2 x ... x nk 
%
% EXAMPLES
% 
%  H = som_hits(sMap,sData);
%       % hit matrix of sData on sMap using the mask vector in sMap
%  H = som_hits(sMap.codebook,D);
%       % hit matrix of sData on sMap using default mask vector
%  som_show(sMap, 1, som_hits(sMap, sData)); 
%       % visualizing the hit matrix
%
% See also SOM_AUTOLABEL, SOM_BMUS.    

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 220997

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% check arguments

error(nargchk(2, 2, nargin));  % check no. of input args is correct

% sMap
if isstruct(sMap), msize = size(sMap.codebook); else msize = size(sMap); end
map_vector_dim = msize(length(msize));% dimension of the weight vectors
msize = msize(1:(length(msize)-1));   % msize = (n1, ...,  nk)
mdim = length(msize);                 % grid dimension
munits = prod(msize);                 % number of map units

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

% calculate BMUs
bmus = som_bmus(sMap,sData,1);

% for each unit, check how many hits it got
Hits = zeros(msize);
for i=1:munits, Hits(i) = sum(bmus == i); end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

