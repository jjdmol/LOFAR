function [h, sf] = somui_fig(action, somfigstruct)
%SOMUI_FIG is a visualization window of SOM
%
%  [h, sf] = somui_fig([action], [somfigstruct])
%
% ARGUMENTS ([]'s are optional)
%
%  [action]        internal variable
%  [somfigstruct]  SOM GUI vis structure
%
% RETURNS
%
%  h   (handle)
%  sf  (struct)
%
% SOMUI_FIG(ACTION, SOMFIGSTRUCT) creates a window where SOM
% according to arguments given by the user in the 
% visualization GUI will be plotted. 
%
% Do not use somui_fig from command line. SOMUI_VIS calls it
% internally. After that SOMUI_VIS calls SOMUI_SHOW to draw
% SOM to here created SOMfig window.
%
% When deleting, DeleteFcn calls for SOMUI_VIS to refresh the GUI
%
% See also  SOMUI_VIS, SOMUI_SHOW

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments

if (nargin < 1)
  action = '';
  sf.issfstruct = 0;
elseif (nargin == 1)
  action = lower(action);
  sf.issfstruct = 0;
else
  action = lower(action);
  sf = somfigstruct;
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Some static variable names for FIG window

% No variables



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION to be taken
%   ''		Not enough arguments
%   'help'	Help dialog box
%   'delete'	Routine called when figure will be destroyed
%   'apply'	Draws to the same somfig window
%   'create'	Draws a new somfig window
%   'no_gui'    No GUI wanted

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == ''

if (strcmp(action,''))                   
  fprintf(1,'%s: Internal function. Use somui_it or somui_vis instead.\n', ...
            mfilename);
  return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'help'  

elseif (strcmp(action,'help'))
  helpstr = {'somui_fig(action, somfigstruct)'; ...
             '  Don''t use SOMUI_FIG from command line'; ...
             '  Use somui_it to create a SOM and'; ...
             '  somui_vis to visualize the map'};
  helpdlg(helpstr,'SOM Toolbox -- somui_fig Help');
  return;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'delete' | 'd'
%    Delete the somfig window
%    Clear handle in vis structure
%    Clear somui_vis's list box of controlling SOMfigs

elseif (strcmp(action,'delete') | strcmp(action,'d'))

  sf.h_somui_vis = findobj('tag','somui_vis');     % REMOVE when not needed 
  
  % remove SOMfig from the control list in somui_vis
  tmpvis = get(sf.h_somui_vis,'UserData');
  % remove it and deselect all in control window
  tmpstr = get(findobj(sf.h_somui_vis, 'tag', 'lb_somfig_ctrl'),'String');
  tmpstr = tmpstr(2:end,:);
  set(findobj(sf.h_somui_vis, 'tag', 'lb_somfig_ctrl'),'String',tmpstr,...
            'Value',[]);
  tmpvis.h_somfig = ' ';
  set(sf.h_somui_vis,'UserData',tmpvis);
%  somui_vis('modify_actives');		% call somui_vis() to modify...
  set(findobj(sf.h_somui_vis, 'tag', 'st_active_SOMfig'), ...
      'String', 'SOMfig: <No SOMfig>');
% Tag 'st_active_SOMfig' is referred from somui_fig 874

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'apply'
%    Modification according to parameters in somui_vis
%    Does not create a new window
%    Callback of the button 'DRAW/APPLY' in the visualization GUI
   
elseif (strcmp(action,'apply'))
  h = local_apply(sf);


% QUESTION:
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'create'
%    Create a new somfig window according to parameters
%    in somui_vis.
%    Callback of the button 'DRAW NEW' in the visualization GUI
 
elseif (strcmp(action,'create'))
  [h, sf] = local_create(sf);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'create'
%    Create a new somfig window according to parameters
%    in somui_vis.
%    Callback of the button 'DRAW NEW' in the visualization GUI
 
elseif (strcmp(action,'no_gui'))
  [h, sf] = local_no_gui(sf);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION unknown 
%   command line SOMUI_FIG with unknown argument

% mfilename is a Matlab function 

else                                      
  fprintf(1,'%s : Unknown argument ''%s''.\n', ... 
              mfilename,action);

end  %% end of 'action' section


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%  end of main SOMUI_FIG %%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%% SUBFUNCTIONS  %%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_NO_GUI
% Draw a new figure for SOMUI_SHOW. Doesn't have any GUI info
%
% V 1.01 Jukka 180398

function [h, sfout] = local_no_gui(sfin)
sfout = sfin;
h = figure;



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_CREATE
% Draw a new somui_fig window
%   There can be arbitrary number of SOMfigs on the screen

function [h, sfout] = local_create(sfin)

%  Argument checking and initialization

if (isfield(sfin,'issfstruct'))
  sfout = sfin;
else
  sfout = [];
  h = 0;
  fprintf(1,'somui_fig/local_init: argument not a sfstruct\n');
  return;
end

figuretag  = 'somui_fig';  		% the same for all somfigs

%lb_string = {'New SOMfig'};		% text for listbox 'lb_status'
mapstring = strcat('Map : ', sfout.map.name);
if (~isempty(sfout.map.data_name'))
  mapdatastring = strcat('Map Data : ', sfout.map.data_name);
else
  mapdatastring = 'Map Data : <data>';
end;
if (isfield(sfout,'data'))
  datastring = strcat('Additional Data : ', sfout.data.name);
else
  datastring = 'Additional Data : <no additional data>';
end;

origUnits = get(0,'Units');		% Get the size of the screen;
set(0,'Units','normalized');

% Create a window
%   Here you can modify the default outlook of the window.
%   If you want to modify anything from the command line,
%   you can use 'get' and 'set' functions:
%     get/set(findobj('name','SOMfig*'),...)
%   where * is the number of the SOMfig window.
%
%   The window can be created with or without GUI part
%   according to variabe sf.sf_params.has_somfig_ui.
%
%   Objects		Type	Tag
%   ---------------------------------------
%   Figure		figure	figuretag==somui_fig
%   Frames		frame	fr_[]
%   ...
%   ...

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  Figure

% tmp is 0.02:0.02:0.18 for first 9 SOMfigs
% SOMfigs are displayed in the position of 'base' and 'offset', ie run.
if (isfield(sfout,'sf_name'))
  tmp=sfout.sf_name;
  tmp=str2num(tmp(7:end));
  tmp=0.02*mod(tmp,10);
else
  tmp=0;
end;

a = figure('Units','normalized', ...
	'Color',[0.701961 0.701961 0.701961], ...
	'DeleteFcn', 'somui_fig(''delete'')', ...
	'Name',sfout.sf_name, ...
	'NextPlot','replacechildren', ...
	'NumberTitle','off', ...
	'Position',[0.06+tmp 0.25+tmp 0.5 0.5], ...
	'Tag',figuretag);
sfout.h_somfig = a;
h = a;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% The plot can be done with or without UI box.
%%   Default is to draw UI but it can be switched
%%   off in Options of somui_vis.
%%   sfout.sf_params.has_somfig_ui

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%  UICONTROL: Frames
%%    see, if these are necessary

if (sfout.sf_params.show_gui)

% +0.18
  b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.02 0.96 0.12], ...
	'Style','frame', ...
	'Tag','fr_downbox');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Statistics, Listbox, Buttons: PARAMETERS, CLOSE
%%
%%   drawn only if sfout.sf_params.show_gui == 1

%  if (~isempty(sfout.map.train_sequence))
%    lb_string = getstatus2lb(sfout.map.train_sequence);
%    set(b,'String',lb_string);
%  end

  b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','delete(gcf)', ...
	'Position',[0.77 0.03 0.20 0.07], ...
	'String','Close', ...
	'Tag','pb_close');

  b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.03 0.72 0.03], ...
	'String', datastring, ...
	'Style','text', ...
	'Tag','st_datastring');

%'Position',[0.03 0.12 0.45 0.03], ...
  b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.065 0.72 0.03], ...
	'String', mapdatastring, ...
	'Style','text', ...
	'Tag','st_mapdatastring');

  b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.10 0.72 0.03], ...
	'String',mapstring, ...
	'Style','text', ...
	'Tag','st_mapstring');


% UIMENUS:

b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&Tools', ...
	'Tag','uim_tools');

c = uimenu('Parent',b, ...
	'Callback','somui_it', ...
	'Label','Init and Train...', ...
	'Tag','uim_tools_it');

c = uimenu('Parent',b, ...
	'Callback','somui_vis', ...
	'Label','Visualization...', ...
	'Tag','uim_tools_vis');

% not used in 1.0beta
c = uimenu('Parent',b, ...
	'Callback', 'somui_chtitle;', ...
	'Label','Change Title...', ...
	'Visible', 'off', ...
	'Separator', 'on', ...
	'Tag','uim_tools_tilte');

c = uimenu('Parent',b, ...
	'Callback','', ...
	'Label','Colormap', ...
	'Tag','uim_tools_cm');

d = uimenu('Parent', c, ...
	'Callback','colormap(''copper'')', ...
	'Label','copper', ...
	'Tag','uim_tools_cm');

d = uimenu('Parent', c, ...
	'Callback','colormap(''hot'')', ...
	'Label','hot', ...
	'Tag','uim_tools_cm');

d = uimenu('Parent', c, ...
	'Callback','colormap(''default'')', ...
	'Label','.default', ...
	'Tag','uim_tools_cm');


b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&Info', ...
	'Tag','uim_info');

c = uimenu('Parent',b, ...
	'Callback','somui_info', ...
	'Label','Info on curr &Map', ...
	'Tag','uim_info_map');
% Careful with Tag: see somui_info

c = uimenu('Parent',b, ...
	'Callback','somui_info', ...
	'Label','Info on curr &Data', ...
	'Tag','uim_info_data');
% Careful with Tag: see somui_info

end % if (sfout.sf_params.show_gui)
    % i.e. if user wanted to have ui control


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%% end of local_create %%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% (sub)function LOCAL_APPLY

function local_apply(somfigstruct)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%% end of local_apply %%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% (sub)function GETSTATUS2LB
%%   This function reads IT parameters from
%%   sfout.map.train_sequence and writes them
%%   to listbox in somfig UI.

function st = getstatus2lb(cellstruct)

if (~iscell(cellstruct))
  error('getstatus2lb: Not a cell');
end

trs = cellstruct;

names = fieldnames(trs{1});
for i=1:6                 %% length(names)
  m=char(names(i));
  p=length(m);
  n(i,1:p)=m;
end

for i=1:length(trs)
  a{7*i-6,1} = strcat('Epoch (',trs{i}.time,') :');
  a{7*i-5,1} = strcat(n(1,:),'.... ',num2str(trs{i}.radius_ini));
  a{7*i-4,1} = strcat(n(2,:),'.... ',num2str(trs{i}.radius_fin));
  a{7*i-3,1} = strcat(n(3,:),'..... ',num2str(trs{i}.alpha_ini));
  a{7*i-2,1} = strcat(n(4,:),'........ ',num2str(trs{i}.alpha_type));
  a{7*i-1,1} = strcat(n(5,:),'........ ',num2str(trs{i}.trainlen));
  a{7*i,1}   = strcat(n(6,:),'.......... ',trs{i}.time);
end
st = a;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%% end of local_getstatus2lb %%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%






