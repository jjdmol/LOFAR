function h=som_planeL(lattice, coordinates, string, clr, points)

%SOM_PLANEL Writes label(s) on the current axes.
%
%  h=som_planeL(lattice, coordinates, string, [clr], [points])
%
% ARGUMENTS ([]'s are optional) 
%
%  lattice      (string) grid lattice, 'hexa' or 'rect'
%  coordinates  (matrix) Nx2 matrix of grid coordinates ij 
%  string       (string or string array or cell array) the labels, 
%                either a single string, or a string array (size
%                Nxl) or a cell array of strings (length N) 
%  [clr]        (string) text color, default: 'black'
%  [points]     (scalar) text size, default: 10 pts 
%
%  RETURNS      
%
%  h            (matrix) object handles to every TEXT object.
%
%  You may label one cell with multiple labels, multiple cells 
%  with one label or multiple cells with one label. 
%  The text objects are tagged with the string 'Lab'  
%  
%  EXAMPLES
%  
%   Ex 1: som_planeL('hexa',[1 1],'XYZ') 
%            labels node (1,1) as XYZ
%   Ex 2: som_planeL('hexa',[[1 1];[1 2]],'XYZ') 
%            labels (1,1) and (1,2) as 'XYZ' 
%   Ex 3: som_planeL('hexa',[[1 1];[1 2]],{'AB';'XYZ'} 
%            labels (1,1) as AB and (1,2) as XYZ 
%   Ex 4: som_planeL('hexa',[[1 1];[1 2];[1 3]],{'AB';'XYZ'}
%            causes an error
%	    
% See also SOM_PLANE, SOM_PLANEU, SOM_ADDLABELS, SOM_CLEAR.

% Version 1.0beta Johan 250997 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             
%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Check the number of arguments

error(nargchk(3, 5, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%% Init & check 

if nargin < 5 | isempty(points)          % default pointsize 
  points=10;
end

if nargin < 4 | isempty(clr)             % default color
  clr='black';
end
                                         
y=coordinates(:,1);
x=coordinates(:,2);                      % obs. the ij-coordinates

switch lattice                           % check lattice
case{'rect'}             
  ;
case{'hexa'}                             % hexa transform
  t=find(~(rem(y,2)));
  x(t)=x(t)+.5;
otherwise
  error(['Lattice' lattice ' not implemented!']);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% 
%%% Action 

h_=text(x,y,string);                     % write labels 

set(h_,'Tag','Lab');                     % labels are tagged and centered
set(h_,'HorizontalAlignment','center');  
set(h_,'Interpreter','none');            % no TeX here

set(h_,'FontSize',points);               % change the pointsize and color
set(h_,'FontUnit','points');
set(h_,'Color',clr);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%% Build output 

if nargout>0, h=h_; end                  % give output only if necessary








