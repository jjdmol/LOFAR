function h=som_show(sMap, comps, scale, colorbardir, M)

%SOM_SHOW Shows the selected component planes and the u-matrix.
%
% h=som_show(sMap, [comps], [scale], [colorbardir], [M])
%
% ARGUMENTS ([]'s are optional)
%
%  sMap           (struct) map structure, codebook size n1 x n2 x dim
%  [comps]        (vector or string) vector of component indices or 
%                  string 'all', default: 'all'
%  [scale]        (string) whether to show normalized or denormalized
%                  data values on the slider, 'denormalized' or 'normalized', 
%                  default is 'normalized'
%  [colorbardir]  (string) colorbar direction, 'horiz' or 'vert' 
%                  default is: 'vert'.
%  [M]            (matrix) data matrix specifying the size of each 
%                  node, size n1 x n2, default=1.
%                              
% RETURNS 
%
%   h             (struct) struct with the following fields:
%    .plane        (vector) handles to the axes objecets (subplots)
%    .colorbar     (vector) handles to the colorbars (colorbars of empty
%                            planes exists but are invisible}
%    .label        (vector) handles to the axis labels
%
% This function shows the component planes and the u-matrix of sMap
% to the current figure.
%
% The default is that the u-matrix is shown first and then the
% component planes according to their order. 
% 
% The optional argument comps may be used to specify the planes which
% are to be shown - and their order in the figure. The u-matrix is
% referred using the values zero (calculate using all the components) 
% or -1 (calculate using only the comoponents specified at the comp).
% A blank plane may be created by inclunding value -2 to the comp vector
% The empty plane is convenient for labeling, hit marking etc. 
%
% The optional argument scale defines whether or not the component plane
% values on the colorbars are to be denormalizedd or not.
% Value 'normalized' shows the planes simply using the original codebook values
% in the map struct - and 'denormalized' denormalizes these values according 
% to the preprocessing information stored in the map struct.
% However, U-matrixes are _always_ computed using the values in the 
% codebook of the map. 
%
% Label n below the colorbar means 'normalized' and d 'denormalized'.
%
% The optional argument M scales the size of each node and it is
% supposed to be a hit matrix (output of som_hits). The node with
% maximal amount of hits gets size scale 1 (normal) and the scaling
% factor for each node is proportional to the ratio between its hits 
% and the maximum. (Of course, any properly sized matrix can be 
% given here to scale the size.)
%
% EXAMPLES
% 
%  map=som_init(data); map=som_train(map,data); som_show(map);
%      % init - teach - visualize
%  som_show(map,[1:6 9 0 -1 -2],'denormalized','vert', som_hits(map,data))
%      % Visualize planes 1...6 and 9; the unit size shows 
%      % the proportional amount of hits in each unit.
%      % show the U-matrix calculated using all the components and 
%      % the u-matrix calculated using components 1...6 and 9. The last
%      % plane is an empty grid.
%      % Use denormalized values in the colorbars. 
%
% See also SOM_ADDHITS, SOM_ADDLABELS, SOM_ADDTRAJ, SOM_HITS, SOM_CLEAR, 
%          COLORMAP, SOM_RECOLORBAR.
%          

% Copyright (c) 1997-1998 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

% Version 1.0beta Johan 231097, bug fix 121297, added som_showtitle 100298 

%% Check arguments %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(1, 5, nargin))  % check no. of input args
error(nargchk(0, 2, nargout)) % check no. of output args

msg=checkMap(sMap);           % see subfunction
error(msg);

d=size(sMap.codebook,3);      % numb. of components
s=[size(sMap.codebook,1) size(sMap.codebook,2)]; % size of the map

if nargin < 5 | isempty(M),
  M=ones(s);                   % default is "normal size" for all nodes
end

if nargin < 4 | isempty(colorbardir)
  colorbardir = 'vert';
elseif ~ischar(colorbardir)
  error('Colorbar direction should be a string.');
end

switch colorbardir              % check colorbardirection
case { 'vert', 'horiz'}
  ;
otherwise
  error('Colorbar direction should be ''vert'',''horiz''');
end

if nargin < 3 | isempty(scale) % default descaling mode
  scale= 'normalized';
end  

switch scale                   % scaled/original ?
case { 'normalized','denormalized'}
  ;
otherwise
  error('Invalid scaling mode argument');
end

if nargin < 2 | isempty(comps)  
  comps='all';                 % default is all components and umatrix
end
  
if ~isvector(comps)            % check the size of comps
  error('The second argument has to be a vector or string ''all''');
end

if isstr(comps)                % check if comps is string 'all'
  switch(comps)
  case 'all',
    comps=[0:d];
  otherwise
    error('The second argument has to be a vector or string ''all''');
  end
end

comps=round(comps);
if min(comps)<-2 | max(comps)>d % check if comps is out of range 
  error('Invalid component plane indexes!');
end  

if s ~= size(M)                % check the size of M
  error('The size of the last argument is inconsistent with the first one.')
end

%% Initialization %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
 
n=length(comps);                  % the number of subfigures
c=setdiff(unique(comps),[0 -1 -2]);  % get the unique component indexes
c=c(~isnan(c));                   
                                  % estimate the suitable dimension for
y=ceil(sqrt(n));                  % subplots
x=ceil(n/y);

M=sqrt(M./max(max(M)));           % scale according to the hit histogram

%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if sum(comps == 0)                % U-matrix (all) requested?              
  if d>1                          % calculate U-matrix with all comps
    umat0=som_umat(sMap);          
  else                            
    umat0=ones(s)*NaN;            % som_umat can't compute, set to NaN.
    warning('Didn''t compute u-matrix: must be at least two  components!');
  end
end

if sum(comps == -1)               % U-matrix (selected) requested?
  if length(c)>1 ,                % calculate U-matrix with selected comps
    umat1=som_umat(sMap.codebook(:,:,c),sMap.lattice,sMap.shape); 
  else
    umat1=ones(s)*NaN;            % som_umat can't compute, set to NaN
    warning('Didn''t compute u-matrix: must be at least two components!');
  end
end

clf;                              % clear figure

for i=1:n,                        % main loop
  h_axes(i,1)=subplot(x,y,i);     % open a new subplot
switch comps(i)
  case -2 
    tmp_h=som_plane(sMap.lattice,zeros(sMap.msize),1);
    h_label(i,1)=xlabel('empty');
    set(tmp_h,'facecolor','none');
  case 0
    som_planeU(sMap.lattice, umat0);
    h_label(i,1)=xlabel('U-matrix (all components)');
  case -1 
    som_planeU(sMap.lattice, umat1);
    h_label(i,1)=xlabel('U-matrix (shown components)');
  otherwise 
    handle_box=som_plane(sMap.lattice, sMap.codebook(:,:,comps(i)), M);
    h_label(i,1)=xlabel(strrep(sMap.comp_names{comps(i)},'_','\_'));
  end
  
%%% Adjust axis ratios to optimal (only 2D!) and put the
%%% title (xlabel) as close to axis as possible
  
  text('string','Cardinalité associées aux neurones','verticalalignment','top');
  set(gca,'plotboxaspectratio',[s(2) s(1) s(1)]);
  
  h_colorbar(i,1)=colorbar(colorbardir);           % colorbars
  %if comps(i)==-2
    set(findobj(h_colorbar(i)),'Visible','off');   % turn off the colorbar of empty
    %end                                              % grid
end  
  
% set window name
set(handle_box,'FaceColor','b');
set(gcf,'Name',[ 'Map name: ' sMap.name ' Data name: ' sMap.data_name]);

%% Set axes handles to the UserData field (for som_addxxx functions
%% and som_recolorbar) 
%% set component indexes and normalization struct for som_recolorbar

SOM_SHOW.subplotorder=h_axes;
SOM_SHOW.msize=sMap.msize;
SOM_SHOW.dim=size(sMap.codebook,3); 
SOM_SHOW.comps=comps;
SOM_SHOW.normalization=sMap.normalization;

set(gcf,'UserData', SOM_SHOW);

% set text interp to none for title

set(h_label,'interpreter','none');

%h_colorbar=som_recolorbar('all','auto', scale);   %refresh colorbars

% set movable text to lower corner

som_showtitle(sMap);

%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 

if nargout > 0
  h.plane=h_axes; h.colorbar=h_colorbar; h.label=h_label;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Subfunction: CHECKMAP                                         %

function msg=checkMap(sMap)

error(nargchk(1, 1, nargin))  % check no. of input args
error(nargchk(1, 1, nargout)) % check no. of output args

msg=[];                       % default is no error

% The map should be a struct and have 2-dimensional grid

if ~isa(sMap, 'struct')                    % struct?
  msg= 'Map should be a structure.';
elseif length(size(sMap.codebook)) ~= 3    % right size?
  msg= 'Can''t visualize! Map grid dimension should be 2';
end

%% Subfunction: ISVECTOR %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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

t=(ndims(v) == 2 & min(size(v)) == 1);




