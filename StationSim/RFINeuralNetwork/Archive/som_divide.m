function [V,I]=som_divide(sMap, sData, mode, coord)

%SOM_DIVIDE Divides a dataset according to a given map.
%
% [V,I]=som_divide(sMap, sData, [mode], [coord])
%
% ARGUMENTS ([]'s are optional) 
%
%  sMap     (struct) map struct (codebook size n1 x n2 ... x nk x dim)
%  sData    (struct or matrix) data struct or matrix (size N x dim )
%  [mode]   (string) 'index' or 'coord' or 'class', see below
%  [coord]  (matrix) if mode is given, this is obligatory: 
%            - 'index' K x 1 vector of map node indexes 
%            - 'coord' K x k matrix of map node coordinates   
%            - 'class' n1 x n2 x ... x nk matrix of class numbers
%
% RETURNS
%
% if mode is default (arguments mode and coord are not given):
%  V        (cell array) n1 x n2 cell array of the data vectors 
%            hitting the node  
%  I        (cell array) n1 x n2 cell array of row indexes 
%            of the data vectors
%
% if mode == 'index' and coord is a K x 1 matrix (map node indexes):
%  V        (matrix) K x dim matrix of data vectors 
%            hitting the specified nodes
%  I        (vector) corresponding data row indexes
%   
% if mode == 'coord' and coord is a K x 2 matrix (map node coordinates):
%  V        (matrix) K x dim matrix of the data vectors 
%            hitting the specified nodes
%  I        (vector) corresponding data row indexes
%
% if mode == 'class' and coord is a n1 x n2 matrix of 
% numbers 0...N (that is, the class)
%  V        (cell array) N x 1 cell array of matrixes. 
%            V{K} includes vectors whose BMU has class number 
%            K in the input matrix 'coord'
%  I        (cell array) corresponding data indexes in the cell array
%  (This can be used after using som_manualcluster)
%
% NOTE: if the same node is specified multiple times, only one
% set of hits is returned.
%
% See also SOM_BMU, SOM_HITS, SOM_MANUALCLUSTER.

% Version 1.0beta 260997 Johan
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

%%%% Init & Check %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 

switch nargin
case 2                        % check if no. of input args is correct
  mode='default';             % default mode
case 4
  ;
otherwise
  error('There has to  be 2 or 4 input argumnets');
end

error(nargchk(0, 2, nargout)) % check if no. of output args is correct

if isa(sData,'struct')        % data struct/ matrix ?
  data=sData.data;            % set data
else
  data=sData;             
end

dim=size(sMap.codebook);      % codebook size
msize=dim(1:end-1);           % map grid size
dim=dim(end);                 % map vector dimension

if size(data,2) ~= dim        % check data dimension
  error('Data dimension inconsistent with the map dimension!');
end

N = prod(msize);              % number of model vectors

switch mode                   % check and set mode
case 'default'
  ;                           % nothing to check
case 'coord'
  if size(coord,2)==length(msize);  % K x dim matrix is ok
  else 
    error(['Coordinate matrix is not valid for mode ' mode]); 
  end
case 'index'
  if size(coord,2) == 1;      % index vector is ok       
  else 
    error(['Coordinate matrix is not valid for mode ' mode]); 
  end
case 'class'
  if size(coord) == msize;    % class matrix is ok  
  else 
    error(['Coordinate matrix is not valid for mode ' mode]); 
  end
otherwise                     % error
  error('Unknown operation mode!');
end

%%% Action & Build output according to the mode string output

bmus=som_bmus(sMap,data);           % bmus

switch mode

case 'default'                       % coord is missing 
  I=cell(msize);
  V=cell(msize);
  for index=1:N,                     % output: structure of data
    I{index}=find((bmus==index));
    V{index}=data(I{index},:);
  end

case 'index'                         % coord are the indexes, easy case
    I=find(ismember(bmus,coord));         
    V=data(I,:);
  
case 'coord'                          
  I=[];                              % data vector/index matrix  
  l=length(msize);
  for k=l-1:-1:1,
    r(k)=prod(msize(1:k));           % for transforming coordinates to indexes
  end
  for k=1:size(coord,1),
    cv=num2cell(coord,1);            %
    index=sub2ind(msize,cv{:});      % transform coords to indexes
    bmu_ind=find(ismember(bmus,index));             
    I=[I;bmu_ind];
  end
  V=data(I,:);

case 'class'
  K=max(max(coord));                 % number of classes
  coord=reshape(coord,N,1);          % same order as in bmu indexes!
  for i=1:K,
    N_ind=find(coord == i);          % indexes of the units of class i
    I{i}=find(ismember(bmus,N_ind)); % data indexes       
    V{i}=data(I{i},:);
  end

otherwise                            % this shouldn't happen
  error('Internal error: unknown mode');
end




