function  H=som_planeU(lattice, m, c)

%SOM_PLANEU Draws a 2D unified distance matrix
%
% h=som_planeU(lattice, m, [c])
% 
% ARGUMENTS ([]'s are optional)
%
%  lattice  (string) grid lattice, 'hexa' or 'rect'
%  m        (matrix) component plane, size n1 x n2, for example
%            map.codebook(:,:,d), or any n1 x n2 matrix in general
%  [c]      (scalar) size scaling of the unit markers, default=0.3
%
%  h        (struct)
%   .plane  (matrix) object handles to nodes (PATCH)
%   .unit   (matrix) object handles to markers (PATCH)
%
% The matrix is oriented in the figure as when printed in the command
% window. Axes are set to the 'ij' mode with equal spacing and turned 
% off. The node (a,b) has coordinates (a,b) in the axes, except on the 
% even numbered rows on the hexa plane where the coordinates 
% are (a,b+0.5). 
%
% See also SOM_PLANE, SOM_PLANE3, SOM_SHOW

% Version 1.0beta Johan 250997 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

%% Check arguments %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(2, 3, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

%%% Init %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargin == 2 | isempty(c), 
  c=0.2;                      % default unit marker size
end

m=m'; 
[xdim,ydim]=size(m);          % map dims
n=xdim*ydim;                  % amount of the units

% Sets x and y columns for hexa and rect for drawing a node with FILL. %%

square=[[-.5 -.5];[-.5 .5];[.5 .5];[.5 -.5]];

hexagon=[[0 0.6667];...
  [0.5 0.3333];...
  [0.5 -0.3333];...
  [0 -0.6667];...
  [-0.5 -0.3333];...
  [-0.5 0.3333]];

%%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% matrix of 6xn nodes for FILL %

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
  error(['Lattice ' lattice 'not implemented!']);
end

% counts matrices x and y which 'moves' nodes to correct positions %

x=repmat(repmat(1:xdim,l,1),1,ydim);
y=reshape(repmat(1:ydim,l*xdim,1),l,n);

% The tricky part for making the desired look & proper scaling
% for the map. Command view([0 90]) shows the map in 2D properly oriented

switch lattice
case 'hexa'
  t2=find(ismember(y(1,:),[2:2:ydim]'));   % create the matrix
  t3=find(ismember(y(1,:),[3:4:ydim]'));
  x(:,t2)=x(:,t2)+0.5;                      
  x(:,t3)=x(:,t3)+1;
  x=(x+nx)/2+.5;y=(y+ny)/2+.5;
case 'rect'  
  x=(x+nx)/2+.5;y=(y+ny)/2+.5;
end

%%% Draw the map! %

h_som_plane=fill(x,y,m(:)');  

%%% Draw the units! %

xdim=round(xdim/2);ydim=round(ydim/2); % calculate unit coordinates
hold on;                                        
h_unit=som_plane(lattice, ones(ydim, xdim)*NaN, c);    % draw units,
set(h_unit,'facecolor',get(gcf,'Color'));              % color NaN
set(h_unit,'edgecolor','none');                        % doesn't affect scalig
set(h_unit,'linewidth',1);                             

% graphical things (som_plane function used takes care of the rest!)

set(h_som_plane,'edgecolor','none');                     
set(gca,'Tag','Umatrix');     % this is necessary for indentification

hold off;

%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0, 
   H.plane=h_som_plane; 
   H.unit=h_unit;
end                           % set output only, if it is needed









