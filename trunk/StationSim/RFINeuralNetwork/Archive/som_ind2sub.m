function Subs = som_ind2sub(msize,inds)

%SOM_IND2SUB Map grid subscripts from linear index.
%
% Subs = som_ind2sub(msize,inds)
%
% ARGUMENTS 
%
%  msize  (struct or vector) map struct, or a vector of size 1 x n 
%          specifying the map grid size
%  inds   (vector) linear subscripts of map units, size m x 1
% 
% RETURNS
%
%  Subs   (matrix) the corresponding subscripts, size m x n
%
% EXAMPLES
%
%  sub = som_ind2sub([10 15],44)
%  sub = som_ind2sub(sMap,44)
%  sub = som_ind2sub(sMap.msize,44)
%  Subs = som_ind2sub([10 15],[44 13 91]')
%
% See also SOM_SUB2IND.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 091297

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

if isstruct(msize), msize = msize.msize; end

n = length(msize); 
k = [1 cumprod(msize(1:end-1))]; 
inds = inds - 1;
for i = n:-1:1, 
  Subs(:,i) = floor(inds/k(i))+1; 
  inds = rem(inds,k(i)); 
end

