function h=som_planeT(lattice, coordinates, clr, marker, lab, points, txtclr)
%
%SOM_PLANET Draws a trajectory on the current axis.
%
% h=som_planeT(lattice, coordinates, [clr], [points])
%
% ARGUMENTS ([]'s are optional) 
%
%  lattice      (string) grid lattice, 'hexa' or 'rect'
%  coordinates  (matrix) Nx2 matrice of grid coordinates ij.
%  [clr]        (string) line color & style, default: 'k-'
%  [marker]     (string) marker, default: 'o'
%  [lab]        (string or cell array) string or string array NxK or 
%                cell array of strings, default is Nx1 cell array 
%                of empty strings
%  [points]     (scalar) text size, default: 10 pts 
%  [txtclr]     (string) text color
%
% RETURNS      
%
%   h           (struct) object handles 
%    .line       (matrix) line       
%    .text       (matrix) text
%
% This function draws a trajectory on the current axis.
%	    
% See also SOM_PLANE, SOM_PLANEU, SOM_ADDTRAJ, SOM_CLEAR.

% Version 1.0beta Johan 061197 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

%%% Check the number of arguments

error(nargchk(2, 7, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

%%%% Init & check %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

i=coordinates(:,1);y=i;                   % xy: graphical coords
j=coordinates(:,2);x=j;                   % ij: logical

switch lattice                            % check lattice
case{'rect'}             
  ;
case{'hexa'}                              % hexa transform
  t=find(~(rem(y,2)));
  x(t)=x(t)+.5;
otherwise
  error(['Lattice' lattice ' not implemented!']);
end

if nargin < 3 | isempty(clr)             % default color
  clr='black';
end

if nargin < 4 | isempty(marker)          % default marker
  marker='o';
end

if nargin < 5 | isempty(lab)             % default labs are none
  lab=cell(size(coordinates,1),1)
end

if nargin < 6 | isempty(points)          % default pointsize 
  points=10;
end

if nargin < 7 | isempty(points)          % default text color 
  txtclr='black';
end

%%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

h_p=plot(x,y,clr);                 % line
set(h_p,'Tag','Traj');             % tags etc.
set(h_p,'Linewidth',2);       
set(h_p,'marker',marker);

h_txt=som_planeL(lattice,[i j], lab, txtclr, points); % labels
set(h_txt,'Tag','Traj');           % tags

for i=1:size(h_txt,1),             % distribute labels a bit
  g=get(h_txt(i),'position');      % this will be made cleverer in later versions
  g([1 2])=g([1 2])+(rand(1,2)-.5)*0.6;
  set(h_txt(i),'position',g);
end

%%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0, 
  h.line=h_p;
  h.text=h_txt;
end                  % give output only if necessary










