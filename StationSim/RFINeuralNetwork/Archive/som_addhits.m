function h=som_addhits(sMap, H, p, type, clr)

%SOM_ADDHITS Draws the hit count on planes in the current fig.
%
% h=som_addhits(sMap, H, [p], [type], [clr])
%
% ARGUMENTS ([]'s are optional) 
%
%  sMap    (struct) map struct (with codebook size n1 x n2 x dim)
%  H       (struct or matrix or vector) data struct, hit 
%           matrix or BMU index vector, see below for details
%  [p]     (vector or string) indicates the subplots which are affected
%           String 'all' (the default) means all the subplots.
%  [type]  (string) visualizing mode ('num' or 'spot'), 
%           default is 'spot' 
%  [clr]   (string or matrix) color (string or RGB vector), 
%           default is 'black'
% RETURNS 
%
%  h       (matrix) handles to the objects.
%
% Adds hits to the visualization made by SOM_SHOW. SOM_ADDHITS acts on the
% current figure and as default it labels all the component 
% planes and u-matrixes. 
%
% Input argument: H
%
% If H is a data struct, the hit matrix is calculated.
% If it is a matrix, the function decides what to do: If H
%
% - has the same size as the map plane, it is assumed to be an
%   already calculated hit matrix and it is visualized as it is.
% - is a vector it is interpreted to be a BMU index vector. The hit 
%   matrix is formed from this just by counting the amount of 
%   each unit in the vector.
%
% If the map is 1-dim this may be ambiguous: sMap is a 5 x 1 map and 
% H a 5 x 1 vector. Is this a BMU index vector or a hit matrix? 
% In this case the function assumes that the vector is a hit matrix,
% but a warning is given.
% 
% Input argument: type
%
% 'num'  Write the hit count as number on each node. 
% 'spot' Show hit count graphically. The area of each spot is 
%        proportional to the proportional amount of hits in the unit.
%
% EXAMPLES
%
%  som_addhits(map,sData);
%       % Counts the hit distribution of sData and draws spots on every plane.
%  H=som_hits(map,sData);som_addhits(map, H, [1 2], 'num', 'w')
%       % Uses an already calculted hit matrix and writes 
%       % hit count in numbers to planes 1 and 2 using white color. 
%
% See also SOM_SHOW, SOM_HITS, SOM_PLANEH, SOM_CLEAR.

% Version 1.0beta Johan 061197 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

%% Init & check %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(2, 5, nargin))    % check no. of input args 
error(nargchk(0, 1, nargout))   % check no. of output args

% Check map and data, exit if error

[datatype, msg]=check(sMap, H);  
 error(msg);

% Check the subplot vector p and  get the handles, exit if error
% Default subplot vector is 'all'

if nargin < 3 | isempty(p)                        % default p
  p='all';
end

[handles,msg,msize,dim]=som_figuredata(p,gcf);    % get handles & check 
error(msg);                                       % if error, exit     
if size(sMap.codebook,3) ~= dim | sMap.msize ~= msize  
  error('Input map struct dimensions do not match with the visualization')
end						  

if nargin < 4 | isempty(type)                     % default type
  type='spot';
end

if nargin < 5 | isempty(clr)                      % default color
  clr='black';
end

%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

switch datatype                     % calculate hits
case 'struct'
  Hits=som_hits(sMap,H);        
case 'hits'
  Hits=H;
case 'bmus'
  Hits=zeros(sMap.msize);
  for i=1:length(H);
    [x,y]=ind2sub(sMap.msize, H(i));
    Hits(x,y)=Hits(x,y)+1;
  end
otherwise  
  error('Internal error: invalid datatype');
end

for i=1:length(handles)              % main loop
  axes(handles(i));
  h_(:,i)=som_planeH(sMap.lattice, Hits, type, clr);
end

%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0
  h=h_;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Subfunction: CHECK                                              %

function [datatype, msg]=check(sMap, H)
%check Checks arguments sMap and H and determines what they are
%
% [datatype, msg]=check(sMap, H)
%
% ARGUMENTS
% 
%  sMap  (struct)        map structure
%  H     (struct|matrix) data struct or BMU index vector
%
% RETURNS
%
%  dataype (string) type of data
%  msg     (string) error message or empty string (no error)

%% Check the number of args %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(2, 2, nargin))    % check no. of input args is correct
error(nargchk(2, 2, nargout))   % check no. of output args is correct

msg=[];                         % default: no error

cr=sprintf('\n');               % CR
yes=1; no=0;
datatype='';                    % default: no datatype

bmuVector=no; hitMatrix=no;     % default: neither vector nor matrix

% 1. Check map structure
% The map should be a struct and have 2-dimensional grid

if ~isa(sMap, 'struct')                   % struct?
  msg = 'Map should be a structure';
  return;
elseif length(sMap.msize) ~= 2            % right dimension?
  msg = 'Can''t visualize!  Map grid dimension should be 2';
  return;
end

% 2. Check data struct
%

if isa(H,'struct'),                            % data is data struct
  if size(H.data,2) ~= size(sMap.codebook,3)   % do data and map match?         
    msg = 'Map dim and data dimension should be the same!';
    return;
  else  
    datatype= 'struct';
  end
  return;
end

% 3. Check data matrix or vector
%

if isvector(H)                            % might be a BMU vector
  bmuVector=yes;
end

if size(H) == sMap.msize                  % might be a hit matrix
  hitMatrix=yes;
end

if ~bmuVector & ~hitMatrix                % not valid type
  msg = 'Hit data has wrong size or invalid components';
  return;
elseif bmuVector & hitMatrix              % ambiguous: warn user
  l1= '   The second input argument could be as well a hit matrix or BMU'
  l2= '   index vector because their dimensions are the same in this case:';
  l3= '   Data is interpreted to be a hit matrix !!';
  warning([l1 cr l2 cr l3]);
  bmuVector=no;
end

if bmuVector & (min(H) < 1 | max(H) > prod(sMap.msize)),
  msg = 'BMU index vector has non-positive or too big indexes';
  return;
end

if bmuVector, datatype = 'bmus'; end;
if hitMatrix, datatype = 'hits'; end;

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
















