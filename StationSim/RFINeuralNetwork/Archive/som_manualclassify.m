function selection=som_manualclassify(msize,lattice,map,preselection,lw)

%SOM_MANUALCLASSIFY Selects manually sets of nodes on a plane.
%
% class = som_manualclassify(msize, lattice, map, [preselection], [lw])
%
% ARGUMENTS ([]'s are optional) 
%
%  msize           (matrix) map grid size ([n1 n2])
%  lattice         (string) grid lattice, 'hexa' or 'rect'
%  map             (scalar or matrix) number of colors or a colormap
%  [preselection]  (matrix) numbers 0...N-1, size n1 x n2
%  [lw]            (scalar) line width, default: 2
%
% RETURNS
%
%  class           (matrix) class selections 0...N. size n1 x n2 
%
% This function draws a msize-sized grid of node borders on the current axis
% and opens a new figure for a color selection palette.
%
% The idea is to draw these borders on an existing plane of the 
% same size and type. The facecolor of the nodes is 'none', so 
% the original ones are not overdrawn. (This function may be
% used on an SOM_SHOW figure, too. Then you have to get the axis 
% handle, lattice and msize  with function SOM_FIGUREDATA.)
%
% By clicking a color on the color palette, you may choose the color
% which is given for the node borders on the plane simply by clicking 
% the nodes. Now, you may classify the nodes.
%
% RETURN terminates the operation and gives the output to the 
% workspace. The output argument is a matrix which returns the 
% colors selescted, that is, the classes of the nodes.
%
% The input argument 'preselection' initializes the border colors. 
% The default initialization is zero (not selected). 
%
% SOM_MANUALCLASSIFY can be used together with SOM_DIVIDE. 
% 
% The object created by this function are deleted when you press 
% RETURN. You can relaunch the som_manualclassify from the point you 
% pressed RETURN by specifying the same colormap and giving the 
% previous output ('selection') as the input argument 'preselection'.
%
% NOTE: Only one som_manualcluster may be launched at the time. 
%
% See also SOM_DIVIDE, SOM_CLEAR.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

% Version 1.0beta Johan 230997 

%%% Check arguments %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(3, 5, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

global MAP;
MAP=map;                      % colormap, global for this and subfunction CALL

n=prod(msize);                % amount of nodes

if size(MAP)==[1 1]           % check colormap
  if MAP<1
    error('There has to be at last one color!')
  end
  MAP=jet(MAP+1);
  MAP(1,:)=[0 0 0];           % double: default map is jet, 
elseif size(MAP,2)==3         % but 0-color is black
  ;
else                          
  error('Input argument map has to be 1 x 1 or N x 3')
end

if nargin < 5 | isempty(lw)                 % default line width
  lw=2;
end

%% check and preprocess the input argument 'preselection' 

if nargin < 4 | isempty(preselection)       % default preselection 
  preselection=0;                           % is 0 (not selected)
end

if size(preselection) == [1 1]              % check preselection size
  preselection=repmat(preselection,msize);  % make msize matrix if [1 1]
elseif size(preselection) == msize
  ; 
else
  error('Preselction matrix size must be msize');     % wrong size
end

preselection=preselection';                 % transform to a vector 
preselection=preselection(:);               % for later use

if (size(MAP,1)-1 < max(preselection)) | (min(preselection) < 0)
  error('Preselection values have to be in interval 0..amount of colors-1');
end

%%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

memhold=ishold;                             % remember drwing mode
hold on;

if ~isempty(findobj('Tag','Sel'));          % old objects left: abort!
  error('Old SOM_MANUALCLUSTER objects exists, see: SOM_CLEAR');  
else
  h=som_plane(lattice,ones(msize(1),msize(2))*NaN,0.83) % draw the borders
  set(h,'Tag','Sel')                                    % tag them
  
  for i=1:n,                                % set preselected colors
    set(h(i),'userdata', preselection(i));
    set(h(i),'edgecolor', MAP(preselection(i)+1,:));
  end
  
  f=selectColorFig(MAP);                    % see below
  f_str=num2str(f);                         
  set(h,'facecolor','none');
  set(h,'linewidth',lw);
  set(h,'buttondownfcn',[ 'call(' f_str ');' ]);
end

%% Wait for user input and RETURN %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

disp('Use mouse to select color and nodes.') ;
disp('Press return when finished.');
pause; 

%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

selection=get(h,'userdata');               
selection=reshape(cat(1,selection{:}),msize(2),msize(1))';

%% Clean %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

som_clear('sel');          

%%%%%%%%%%%%%% Subfunctions %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Open a color selection window
%% The UserData property of this figure knows the selection color index

function f=selectColorFig(map);      

l=size(map,1);

% Open a new figure, tag the objects etc.

f=figure;                           
set(f,'Tag','Sel');
set(f,'name','SOM_MANUALCLUSTER: Color Selection');
h_plane=som_plane('rect',ones(1,l),0.8);    % draw colors  
axis('on');
h_ax=gca;                                   % remember axis

% Write the button-down-functions which change the UserData-property 
% according to the selection

for i=1:l,                                  
  set(h_plane(i),'facecolor',map(i,:));
  set(h_plane(i),'buttondownfcn', ... 
      [ 'set(' num2str(f) ', ''Userdata'',' num2str(i-1) '); ']);
end

set(f,'userdata',0);                        % texts etc.
set(h_ax,'plotboxaspectratio',[l 1 1]);
set(h_ax,'xtick',[1:l+1]);
set(h_ax,'xticklabel',num2str([0:l]'));
set(h_ax,'Yticklabel','');
title('Selection colors, 0 = not selected'); 


% This changes the color of the node border %%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function call(h_f)

global MAP;

i=get(h_f,'userdata');                    % Get the selection color 

if get(gcbo,'userdata') == i;             % toggle color 
  set(gcbo,'edgecolor',MAP(1,:));
  set(gcbo,'userdata',0);
else 
  set(gcbo,'edgecolor', MAP(i+1,:));
  set(gcbo,'userdata',i);
end







