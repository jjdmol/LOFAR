function h=som_addlabels(sMap, lab_n, p, points, clr)

%SOM_ADDLABELS Writes labels on the current figure.
%
% h=som_addlabels(sMap, [lab_n], [p], [points], [clr])
%
% ARGUMENTS    
%           
%  sMap      (struct) map structure
%  [lab_n]   (matrix or string) vector or string 'all' (the default) 
%             that indicates which of multiple labels to print
%  [p]       (matrix or string) vector or string 'all' (the default) 
%             that indicates the subplots which are affected.
%  [points]  (scalar) font size, default: 10 points
%  [clr]     (string) font color, default: 'black'
%          
% RETURNS      
%
%  h         (cell array) n1 x n2 x length(p) cell array of 
%             object handles to every TEXT object.
%
% Labels the visualiztion made by SOM_SHOW. SOM_ADDLABELS works 
% on the current figure and as default it labels all the component 
% planes and u-matrixes. 
%
% Argument lab_n can be used to select a group of labels to be 
% shown. Eg. [1:3] will show three first labels in each node.  
%
% The texts are centered on the nodes.
% The cell array h may be transformed to a vector by command cat(1,h{:}); 
%
% EXAMPLES
%
%  som_addlabels(map);
%       % adds all labels from map to the figure 
%  som_addlabels(map,[1 3 5], [1 2], [], 'w')
%       % adds 1st, 3rd and 5th labels from map to 1st and 2nd 
%       % subplot, uses default point size and white color
%
% See also SOM_SHOW, SOM_LABEL, SOM_CLEAR.

% Version 1.0beta Johan 061197 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

%% Init & check    %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(1, 5, nargin))      % check no. of input args
error(nargchk(0, 1, nargout))     % check no. of output args

msg=checkMap(sMap);               % see subfunction
error(msg);                       % exit if error

if nargin < 2 | isempty(lab_n)    % default lab_n
  lab_n='all';
end

if ischar(lab_n)                  % check string 'all'
  switch lab_n
  case 'all'                      
    t=sMap.labels(:);             % find max amount of labels
    for i=1:size(t,1)
      n(i)=length(t{i});
    end
    lab_n=1:max(n);
  otherwise
    error('String argument for the label number vector has to be ''all''.');
  end
end

mxl=max(lab_n);                   % ind is a logical vector which   
ind=logical(zeros(1,mxl));        % selects the labels to be shown
ind(lab_n)=1;                     % mxl is the max. index to be shown

if nargin < 3 | isempty(p)        % default subplot vector p
  p='all';
end

[handles,msg,msize,dim]=som_figuredata(p,gcf);
error(msg);                                            % get handles & check 
if size(sMap.codebook,3) ~= dim | sMap.msize ~= msize  % if error, exit
  error('Input map struct dimensions do not match with the visualization')
end						  

if nargin < 4 | isempty(points)   % default point size
  points=10;
end

if nargin < 5 | isempty(clr)      % default color
  clr='black';
end

%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

for n=1:length(handles)              
  axes(handles(n));
  for i=1:sMap.msize(1),                   % Main loops for ij-coordinates 
    for j=1:sMap.msize(2),
      if ~isempty(sMap.labels{i,j})        % check cells
	labs=sMap.labels{i,j};             % this structure deals with
      else                                 % the empty cells
	labs='';
      end
      l=min(length(labs),mxl);             % select the length
      if ~isempty(sMap.labels{i,j}(ind(1:l)))
	h_{i,j,n}=som_planeL(sMap.lattice,[i j], ...  % draw the labels
	    sMap.labels{i,j}(ind(1:l)), clr, points);
      end
    end
  end
end


%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0
  h=h_;
end

%% Subfunctions %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function msg=checkMap(sMap)

error(nargchk(1, 1, nargin))  % check no. of input args
error(nargchk(1, 1, nargout)) % check no. of output args

msg=[];                       % default is no error

% The map should be a struct and have 2-dimensional grid

if ~isa(sMap, 'struct')                         % struct?
  msg= 'Map should be a structure';
  return
elseif length(sMap.msize) ~= 2                  % right size?
  msg= 'Can''t visualize!  Map grid dimension should be 2';
  return;
end


