function h=som_showtitle(txt)
%SOM_SHOWTITLE Adds a movable map info text or any other text to current figure
%
% h=som_showtitle(txt)
%
% ARGUMENTS ([]'s are optional)
% 
%  txt (map struct, string or double)
%         
% RETURNS 
%
%  h (vector) handles to existing axis objects created by this function 
%
% This function sets a text to the current figure. If txt is a map struct
% the map name, data name and msize are wirtten to the lower left corner
% If txt is a string, it's written as it is to the same place. 
% If txt is a double, the font size of all text objects created by this
% function are changed to the pointsize txt.
%
% If no input argument is given the function only returns the handles
% to all existing axis objects.
%
% The texts may be dragged to a new location at any tim using mouse. 
% Note, that the current axes will be the parent of the text object 
% after dragging.
%
% String 'Info' is set to the Tag property field of the objects. 
% 
% EXAMPLES
%
% som_show(map);som_showtitle(16);
%  % sets the point size of the map info to 16
% som_showtitle('Faa');som_showtitle('Foo');som_showtitle(20);
%  % sets movable texts Faa nad Foo to the current figure
%  % and changes their fonts to pointsize 20
% delete(som_showtitle);
%  % deletes all objects created by function som_showtitle from the 
%  % current figure
% 
% See also SOM_SHOW, SOM_SHOWTITLEBUTTONDOWFCN          

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/             

% Version 1.0beta Johan 150197

%% Check arguments %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

error(nargchk(0, 1, nargin))  % check no. of input args
error(nargchk(0, 1, nargout)) % check no. of output args

%% Init %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Get the handles to the existing Info-axes objects

h_infotxt=findobj(gcf,'tag','Info','type','text');
h_infoax=findobj(gcf,'tag','Info','type','axes');

%% Action %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% If no arguments are given, return the old axes handles

if nargin == 0 | nargout > 1 
  h=h_infoax;
else

% If txt is a struct assume that it is a map:
% Write map name, data name and map size to the figure
  
  if isstruct(txt)
    map=txt;
    name=map.name;                % get name
    if isempty(name)
      name='[no name]'
    end
    
    data=map.data_name;           % get data name
    if isempty(data)
      data='[no name]';
    end
    
    msize=num2str(map.msize(1:2));  % get size
  
    %% set text string and put it into figure with moving capability
  
    text=[ 'Map: ' name ', Data: ' data ', Size: ' msize];
    [t,h]=movetext(text);
    h_infoax=[h;h_infoax];
    
  elseif ischar(txt)              % string: write a string as it is
    [t,h]=movetext(txt);
    h_infoax=[h; h_infoax];
  else                            % other: change font size  
    set(h_infotxt,'fontunits','points');
    set(h_infotxt,'fontsize',txt);
  end
end

%% Build output %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
if nargout>0     % output only if necessary
  h=h_infoax;
end
%%% SUBFUNCTIONS %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%  

function [t,h]=movetext(txt)
% Moves the text. See also SOM_SHOWTITLEBUTTONDOWNFCN
%
%
initpos=[0.05 0.05 0.01 0.01];   

memaxes = gca;                   % Memorize the gca

%% Create new axis on the lower left corner.
%% This will be the parent for the text object

h = axes('position',initpos,'units','normalized');
set(h,'visible','off');          % hide axis

t = text(0,0,txt);               % write text 
set(t,'tag','Info');             % set tag
set(h,'tag','Info');             % set tag

set(t,'verticalalignment','bottom');  % set text alignment
set(t,'horizontalalignment','left');

%% Set ButtonDownFcn

set(t,'buttondownfcn','som_showtitleButtonDownFcn') 

axes(memaxes);                   % Reset original gca


