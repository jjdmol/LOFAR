function h_t = somui_text(pos, align, fsize, tag, string, h_parent, debug)
%SOMUI_TEXT writes a title to SOMUI_FIG's SOMfig
%
%  somui_text(pos, align, fsize, tag, string, h_parent, debug)
%
% ARGUMENTS
%   
%   pos	        (vector) axis position, box for the text
%   align       (number) magic number for horizontal and vertical
%                        alignment, see below
%   fsize       (number) normalized fontsize
%   tag         (char) tag for the text
%   string      (string) text string
%   [h_parent]	(handle) handle to parent figure to be operated		
%   [debug]     (integer) if 1, shows the axis
%
% RETURNS
%   
%   h_t  (handle) handle to text object
%
% SOMUI_TEXT is called from SOMUI_SHOW to create a text 
% object into SOMfig. 
%
% Text object parameters:	
%   VerticalAlignment: 		bottom, middle, top
%   HorizontalAlignment: 	left, center, right
%
% NOTE! Matlab 5.1 has still problems in GUI services, for example,
% uicontrol(...,'style','text','horizontalaligment','center',...)
% does not work.
% 
% See also SOMUI_SHOW

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(5, 7, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

parent = gcf;		% default h_parent argument
visible = 'off';	% default debug argument

if (nargin >= 6)
  parent = h_parent;
end;
if (nargin == 7)
  if debug==1
    visible = 'on';
  end;
end;

h_axes = axes('Parent', parent, ...
	'Position', pos, ...
	'Visible', visible);

switch align
  case 1,
	% Horizontal: left, Vertical: bottom: Text starts from
	% left bottom corner
	align1 = 'left';
	align2 = 'bottom';
	position = [0 0];
  case 2,
	% Horizontal: center, Vertical: bottom: Text is justified, 
	% on bottom line
        align1 = 'center';
        align2 = 'bottom';
        position = [0.5 0];
  case 3,
	% Horizontal: center, Vertical: middle: Text is justified and
	% and in middle
        align1 = 'center';
        align2 = 'middle';
        position = [0.5 0.5];
  case 4,
	% Horizontal: right, Vertical: bottom: Text ends to right 
	% bottom corner
	align1 = 'right';
	align2 = 'bottom';
	position = [1 0];
end;

h_t = text('Parent', h_axes, ...
	'FontUnits','normalized', ...
	'FontSize', fsize, ...
	'HorizontalAlignment', align1, ...
	'VerticalAlignment', align2, ...
	'Position', position, ...
	'String', string, ...
	'Tag', tag);

% EOF
