function h=som_planeH(lattice, m, type, clr, points)

%SOM_PLANEH Writes a hit histogram or any matrix on the current axis
%
% h=som_planeH(lattice, m, type, [clr], [points])
%
% ARGUMENTS ([]'s are optional) 
%
%  lattice   (string) lattice type, 'hexa' or 'rect'
%  m         (matrix) number of hits in each node, size n1 x n2
%  type      (string) how to show the hits, 'num' or 'spot'
%  [clr]     (string or matrix) color string or RGB vector. 
%             Default is 'black'.
%  [points]  (double) text size. Default is 10. This argument 
%             has no effect if type=='spot'.
%                 
%  RETURNS      
%
%   h        (matrix) object handles to every TEXT/PATCH object.
%  
%  The texts are centered on the nodes. 
%  The objects are tagged with the string 'Hit'
%  
%  See also  SOM_PLANE, SOM_ADDHITS, SOM_CLEAR

% Version 1.0beta Johan 260997 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

% Check number of arguments

error(nargchk(3, 5, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

%% Check & init %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

memhold=ishold;               % remember the hold state

if nargin < 4 | isempty(clr),            
  clr='black';                % default color
end

if nargin < 5 | isempty(points),
  points=10;                  % default pointsize
end

xdim=size(m,1);                               % dimension 
ydim=size(m,2); 

x=repmat(1:xdim,1,ydim);                      % coordinate vectors            
y=reshape(repmat(1:ydim,xdim,1),1,xdim*ydim);

switch lattice                                
case 'hexa'
  y=y+.5;
  t=find(rem(x,2));                           % hexa coordinates
  y(:,t)=y(:,t)-0.5;
case 'rect'
  ;                                           % rect coordinates
otherwise
  error(['Lattice' lattice ' not implemented!']);
end

%%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

switch type                                     
case 'num'                                    % 'num'  mode
  str=num2str(reshape(m,xdim*ydim,1));
  h_=text(y,x,str);
  set(h_,'Tag','Hit');                        % set tags
  set(h_, 'color', clr)
  set(h_,'fontunit','points');
  set(h_, 'fontsize', points);
  set(h_, 'horizontalalignment', 'center');
case 'spot'                                   % 'spot' mode
  mx=max(max(m));                             % The value of the
  Size=sqrt(m./mx);       % calculate size    % plane has to be within
  Size(Size == 0)=eps;    % area proportial   % caxis. Otherwise it alters
                          % to amount of hits % the colormap.
  hold on;                                    %  
  h_=som_plane(lattice, ones(size(m))*mean(caxis), Size);
    set(h_,'Tag','Hit');                      % set tags
  set(h_, 'facecolor', clr);
  set(h_,'edgecolor','none');                 % no edges
otherwise
  error('Argument type may have values { num | spot }');
end

%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if nargout>0, h=h_; end                  % give output only if necessary

%% Clean %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if ~memhold
  hold off;                              % reset hold state
end
