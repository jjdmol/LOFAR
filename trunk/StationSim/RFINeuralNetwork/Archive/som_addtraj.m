function h=som_addtraj(sMap, T, p, lab, clr, pnt)

%SOM_ADDTRAJ Draws a trajectory on planes in the current figure.
%
% h=som_addtraj(sMap, T, [p], [lab], [clr], [pnt])
%
% ARGUMENTS ([]'s are optional)
%
%  sMap   (struct) map structure
%  T      (matrix or struct) data matrix, BMU index vector or data struct
%  [p]    (vector or string) indicates the subplots which are 
%          affected. String 'all' (the default) means all subplots.
%  [lab]  (cell array or string) cell array of labels for data 
%          vectors, see below
%  [clr]  (string) line color & style, default: 'k-'
%  [pnt]  (scalar) point size, default: 10
%
% RETURNS 
%
%  h       (struct) with the following fields
%   .line: (vector) 1 x P vector of line objects
%   .text: (matrix) N x P matrix of text objects
%           (P number of subplots, N number of data points) 
%
% 
% Adds a trajectory to the visualization made by SOM_SHOW. 
% SOM_ADDTRAJ acts on the current figure and as default it labels all 
% the component planes and u-matrixes. 
%
% The function finds best matching units for data vectors and 
% connects these units with a line. Data has to be specified as 
% data struct, data matrix or BMU index vector.  
%
% Any vector is interpreted to be a BMU index vector 
% and a matrix N x dim is interpreted to be a data matrix. This 
% may be ambiguous if the map dimension == length(T). In this 
% case the vector is assumed to be a data matrix with one vector only.
%
% Each hit is labelled with lab, which may be a cell array of strings, 
% one string for each data point. There are two special strings: 
% 'order' gives labels '1'...'N' in the order of appearance and 'none'
% leaves the points without labels. 
% 
% If T is a data struct, the default value for lab is the first label 
% in the data struct and if T is a data matrix or a BMU index vector,  
% default is 'none'.
%
% If one node gets multiple hits the lablel marks are distributed 
% randomly on the node. (This will be done more cleverly in the future 
% versions, if possible.)
%
% EXAMPLES
%
%  som_addtraj(map,[1 100 12 34 35 2],'all','order');
%       % draws a trajectory through specified nodes on all the planes using
%       % black color and labels the data points 1,..,6.
%  som_addtraj(map, sData, 10, [], 'y:', 20)
%       % draws trajectory according to sData on plane numer 10, 
%       % labels data points according to vector labels in sData. 
%       % Uses yellow dotted line and 20 points text font.
%  
% See also SOM_SHOW, SOM_PLANET, SOM_BMUS, SOM_CLEAR. 

% Version 1.0beta Johan 061197 

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

%% Check & init %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(2, 6, nargin))       % check no. of input args
error(nargchk(0, 1, nargout))      % check no. of output args

[datatype, msg]=check(sMap,T);     % check map and data, see subfunction
error(msg);                        % exit if error

switch datatype                    % calculate bmus
case { 'struct', 'matrix'}
  bmus=som_bmus(sMap,T);
case 'bmus'
  bmus=T;
otherwise
  error('Internal error: unknown datatype');
end

if size(bmus,1) < size(bmus,2)     % bmus has to be a Nx1 vector
  bmus=bmus';
end

nb=length(bmus);                   % number of bmus


if nargin < 6 | isempty(pnt)       % default pointsize
  pnt=10;
end

if nargin < 5 | isempty(clr)       % default color
  clr='k-';
end

txtclr='ymcrgbwk';
txtclr=txtclr(ismember(txtclr,clr)); % drop out line style from txtclr

if nargin < 4 | isempty(lab)       % default labeling
  switch datatype
  case 'struct'              % data struct: default label is the original one
    lab=T.labels;
    for i=1:size(lab,1)
      lab{i}=lab{i,1};      % first label only
    end
  case { 'matrix', 'bmus'}         % bmu index or data matrix: no labeling
    lab= 'none';                                 
  otherwise                        % this shouldn't happen
    error('Internal error: unknown datatype');
  end
end

if ischar(lab)                     % special strings for lab
  switch lab
  case 'none'
    lab=cell(nb,1);                % empty labels
  case 'order'
    lab=cellstr(num2str([1:nb]')); % ordinal number labeling
  otherwise
    error('Labels are invalid');
  end
end

if nargin < 3 | isempty(p)         % default p is 'all'
  p='all';
end

[handles,msg,msize,dim]=som_figuredata(p,gcf);    % get handles & check 
error(msg);                                       % if error, exit
if size(sMap.codebook,3) ~= dim | sMap.msize ~= msize      
  error('Input map struct dimensions do not match with the visualization')
end						  

%%%%%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

[i,j]=ind2sub(sMap.msize, bmus);   % logical coordinates of bmus

for k=1:length(handles)            % main loop
  axes(handles(k));
  memhold=ishold;                  % remember hold state
  hold on;
  h_=som_planeT(sMap.lattice, [i j], clr, 'o', lab, pnt, txtclr); % draw 
  h_p(:,k)=h_.line; h_txt(:,k)=h_.text;
  if ~memhold                      % restore hold state
    hold off;
  end
end

%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0                                    % output only if necessary
  h.line=h_p;,
  h.text=h_txt;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Subfunction: CHECK                                              %

function [datatype, msg]=check(sMap, T)
%check Checks arguments sMap and T and determines what they are
%
% [datatype, msg]=check(sMap, T)
%
% ARGUMENTS
% 
%  sMap  (struct)        map structure
%  T     (struct|matrix) data struct or BMU index vector
%
% RETURNS
%
%  dataype (string) type of data

%% Check the number of args %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(2, 2, nargin))    % check no. of input args is correct
error(nargchk(2, 2, nargout))   % check no. of output args is correct

msg=[];                         % default: no error

cr=sprintf('\n');               % CR
yes=1; no=0;
datatype='';                    % default: no datatype

bmuVector=no; dataMatrix=no;    % default: neither vector nor matrix

% 1. Check map structure
% The map should be a struct and have 2-dimensional grid

if ~isa(sMap, 'struct')                 % struct?
  msg= 'Map should be a structure';
  return;
elseif length(sMap.msize) ~= 2          % right size?
  msg= 'Can''t visualize!  Map grid dimension should be 2';
  return;
end

% 2. Check data struct
%

if isa(T, 'struct'),                            % data is data struct
  if size(T.data,2) ~= size(sMap.codebook,3)    % data dimension ok? 
    msg= 'Map dim and data dimension should be the same!';
    return;
  else  
    datatype= 'struct';
  end
  return;
end

% 3. Check data matrix / BMU vector
%

if isvector(T)                             % might be a BMU vector
  bmuVector=yes;
end

if size(T,2) == size(sMap.codebook,3)      % might be a data matrix
  dataMatrix=yes;
end

if ~bmuVector & ~dataMatrix                % not valid type
  msg = 'Second argument has wrong size, invalid components or is wrong type.';
  return;
elseif bmuVector & dataMatrix              % ambiguous: warn user
  l1= '   The second input argument could be as well a data matrix or BMU';
  l2= '   index vector because their dimensions are the same in this case:';
  l3= '   It is interpreted to be a data vector!';
  warning([l1 cr l2 cr l3]);
  bmuVector=no;
end

if bmuVector & (min(T) < 1 | max(T) > prod(sMap.msize)),
   msg = 'BMU index vector has non-positive or too big indexes';
   return;
end

if bmuVector, datatype = 'bmus'; end;
if dataMatrix, datatype = 'matrix'; end;

%% Subfunction: ISVECTOR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function t=isvector(v)
% ISVECTOR checks if a matrix is a vector or not
%
%  t=isvector(v)
% 
%  ARGUMENTS
%
%  v (matrix) 
%
%  OUTPUT 
%
%  t (logical)
%
%
%  Performs operation ndims(v) == 2 & min(size(v)) ==1
%

t=(ndims(v) == 2 & min(size(v)) == 1) & ~ischar(v);












