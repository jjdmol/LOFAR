function h=som_plane3(lattice, m, sp, z)

%SOM_PLANE3 Draws a 2D component plane in 3D.
%
% h=som_plane3(m, lattice, [sp], [z]) 
%
% ARGUMENTS ([]'s are optional)
%
%  lattice  (string) grid lattice, 'hexa' or 'rect'
%  m        (matrix) component plane, size n1 x n2, for example
%            map.codebook(:,:,d), or any n1 x n2 matrix in general
%  [sp]     (matrix or scalar) n1 x n2 matrix gives an individual size 
%            scaling for each node. A scalar gives the same size 
%            for each node. Default is 1.
%  [z]      (matrix) the z-axis value, size n1 x n2, z=m by default
%
% RETURNS   
%
%  h        (vector) array of object handles to every PATCH object, 
%            size n1*n2 x 1
%
% The matrix is oriented in the figure as when printed in the command
% window. Axes are set to the 'ij' mode with equal spacing and turned 
% off. The node (a,b) has coordinates (a,b) in the axes, except on 
% the even numbered rows on the hexa plane where the coordinates 
% are (a,b+0.5). 
%
% The main difference to som_plane-function is that the FILL3-function 
% is used - instead of FILL - to create the patches.
%
% See also SOM_PLANE, SOM_PLANEU, SOM_SHOW.

% Version 1.0beta Johan 0601197 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             
%

%%% Check arguments %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(2, 4, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

%%% Init %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 

if nargin < 3 | isempty(sp),  
  sp=1;                           % default value for sp  
end

if nargin < 4 | isempty(z),  
  z=m;                            % default value for z
end

m=m';sp=sp';z=z';                 % proper orientation for computing

[xdim,ydim]=size(m);
n=xdim*ydim;

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

% matrix of 6xn nodes for FILL %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

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

sp(sp==0)=eps;                    % see som_plane 

% information for z-axis %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if size(z) == size(m)
  z=repmat(z(:)',l,1);
else
  error('Z-coordinate matrix (z) has wrong size.');
end

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

tmp=fill3(x,y,z,m(:)');

%% Set axes properties  
  
axis('ij');                                % matrix mode

switch lattice
case 'rect'
  axis('equal');                           % this corrects the squares
case 'hexa'
  set(gca,'DataAspectRatio',[0.9015 1 1]); % this corrects the hexagons
end

set(gca,'XaxisLocation','Top');            % axis orientation
set(gca,'Tag','Componentplane');           % tag is necessary for som_show 
axis('off');                               % axis off & tight
axis('tight');

%%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0, h=tmp; end                   % Set h only, 
                                           % if there really is output












