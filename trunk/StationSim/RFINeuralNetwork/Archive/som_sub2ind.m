function inds = som_sub2ind(msize,Subs)

%SOM_SUB2IND Linear index from map grid subscripts.
%
% ind = som_sub2ind(msize,Subs)
%
% ARGUMENTS 
%
%  msize  (struct or vector) map struct, or a vector 
%          specifying the map grid size, size 1 x n
%  Subs   (matrix) the subscripts, size m x n, where m is the 
%          number of subscript sets
% 
% RETURNS
%
%  inds   (vector) corresponding linear indexes, size m x 1
%
% EXAMPLES
%
%  ind = som_sub2ind([10 15],[4 5])
%  ind = som_sub2ind(sMap,[4 5])
%  ind = som_sub2ind(sMap.msize,[4 5])
%  inds = som_sub2ind([10 15],[4 5; 3 2; 1 10])
%
% See also SOM_IND2SUB.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 091297

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% action

if isstruct(msize), msize = msize.msize; end

k = [1 cumprod(msize(1:end-1))]';
inds = 1 + (Subs-1)*k;

