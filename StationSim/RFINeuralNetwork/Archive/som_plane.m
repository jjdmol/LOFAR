function h=som_plane(lattice, m, sp)

%SOM_PLANE Draws a 2D-component plane on the current axis.
%
% h=som_plane(lattice, m, sp) 
%
% ARGUMENTS ([]'s are optional)
%
%  lattice  (string) grid lattice, 'hexa' or 'rect'
%  m        (matrix) component plane, size n1 x n2, for example
%            map.codebook(:,:,d), or any n1 x n2 matrix in general
%  [sp]     (matrix or scalar) n1 x n2 matrix gives an individual size 
%            scaling for each node. A scalar gives the same size 
%            for each node. Default is 1.
% RETURNS   
%
%  h        (vector) array of object handles to every PATCH object,
%            size n1*n2 x 1
%
% The matrix is oriented in the figure as when printed in the command
% window. Axes are set to the 'ij' mode with equal spacing and turned off.
% The node (a,b) has coordinates (a,b) in the axes, except on the even 
% numbered rows on the hexa plane where the coordinates are (a,b+0.5). 
%
% See also SOM_PLANE3, SOM_PLANEU, SOM_SHOW.

% Version 1.1beta Johan 061197, modification 121297
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             
%

%%% Check arguments %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(2, 3, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

%%% Init %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargin == 2 | isempty(sp),  
  sp=1;                           % default value for sp  
end  

m=m';sp=sp';                      % proper orientation for computing

[xdim,ydim]=size(m);
n=xdim*ydim;

%if n==3 
%warning('Visualization problems with 1x3 and 3x1 maps. See documentation.');
%end

% Sets x and y columns for hexa and rect for drawing a node with FILL. %%

square=[[-.5 -.5];[-.5 .5];[.5 .5];[.5 -.5]];

% This is not actually correct, but otherwise we would not get
% the node coordinates even.

hexagon=[[0 0.6667];...
  [0.5 0.3333];...
  [0.5 -0.3333];...
  [0 -0.6667];...
  [-0.5 -0.3333];...
  [-0.5 0.3333]];

% matrix of 6xn nodes for FILL 

switch lattice
case 'hexa'
  nx=repmat(hexagon(:,1),1,n);
  ny=repmat(hexagon(:,2),1,n);
  l=6;
case 'rect'
  nx=repmat(square(:,1),1,n);
  ny=repmat(square(:,2),1,n);
  l=4;
otherwise
  error([ 'Lattice ' lattice ' not implemented!']);
end

%%%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% counts matrices x and y which 'moves' nodes to the correct positions 

x=repmat(repmat(1:xdim,l,1),1,ydim);
y=reshape(repmat(1:ydim,l*xdim,1),l,n);

if size(sp) == [1 1]              % set and check sp
  ;
elseif size(sp) == size(m)
  sp=repmat(sp(:)',l,1);
else
  error('Size matrix (sp) has wrong size.');
end

% Size zero would cause division by zero. eps is as good (node disappears)
% I treid first NaN, it works well otherwise, but the node is 
% then not on the axis and axis tight command etc. may work oddly.

sp(sp==0)=eps;                    

% A messy thing for making a hexagonal lattice.
% Command view([0 90]) shows the map in 2D properly oriented

switch lattice
case 'hexa'
  t=find(rem(y(1,:),2)); 
  x(:,t)=x(:,t)-.5; 
  x=(x./sp+nx).*sp+.5; y=(y./sp+ny).*sp;  % scale with sp
case 'rect' 
  x=(x./sp+nx).*sp; y=(y./sp+ny).*sp;     % scale with sp
end

%% Draw the map! 

tmp=fill(x,y,m(:)');

%% Set axes properties  
  
axis('ij');                                % matrix mode

switch lattice
case 'rect'
  lelim=-.50; rilim=.50; uplim=-.50; lolim=.50; % axis limits
  axis('equal');                           % this corrects the squares
case 'hexa'
  lelim=-.50; rilim=1.00; uplim=-.67; lolim=.67; % axis limits
  set(gca,'DataAspectRatio',[0.9015 1 1]); % this corrects the hexagons
end

set(gca,'XaxisLocation','Top');            % axis orientation
set(gca,'Tag','Componentplane');           % tag is necessary for som_show 

axis('off');                               % axis off & tight
axis([1+lelim xdim+rilim 1+uplim ydim+lolim]);
%%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0, h=tmp; end                   % Set h only, 
                                           % if there really is output













