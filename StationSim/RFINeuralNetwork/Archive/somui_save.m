function somui_save(action, figH, type)
%SOMUI_SAVE displays a window for saving a map or data.
%
%  somui_save(action, figh, type)
%
% ARGUMENTS
%   [action]  (string)  action to be taken
%   [figH]    (handle)  calling figure's handle
%   [type]    (integer) default for saving map/data/SOMfig struct
%
% SOMUI_SAVE can save either map struct, data struct or special (for GUI)
% vis struct. Destination can be workspace, SOM_PAK file or normal
% Matlab MAT file.
%
% See also SOM_READ_COD, SOM_READ_DATA, SOMUI_SEL_VAR, SOMUI_SEL_FILE,
%          SOMUI_LOAD

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197, 161197
%   Previous file name is copied to vis struct / Jukka 161197 NOT DONE!

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global FIGURENAME;
global FIGURETAG;

global h_caller;

global h_rb_save_r1;
global h_rb_save_r2;
global h_rb_save_r3;
global h_rb_to_r1;
global h_rb_to_r2;
global h_et_to_edit1;
global h_et_to_edit2;
global h_cb_params_care;
global h_et_params_care;



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Check arguments
% ERROR: These settings come valid every time the GUI is clicked!
%   This can lead easily to errors!

error(nargchk(0, 3, nargin))  % check no. of input args is correct
error(nargchk(0, 0, nargout)) % check no. of output args is correct

if (nargin < 1)
  action = 'create';		% default action is 'create'
end; 
action = lower(action);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization                           

FIGURENAME = 'SOM Toolbox -- Save';
FIGURETAG  = 'somui_save';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTIONs to be taken
%   'local_savefile'
%   'local_savevar'
%   'local_ok'
%   'local_cancel'
%   'local_create'

if (strcmp(action,'save_file'))
  local_sf;

elseif (strcmp(action,'save_var'))
  local_sv;

elseif (strcmp(action,'ok'))
  local_ok;

elseif (strcmp(action,'create'))
  if (nargin < 3)
    type = 3;				% default: Save a SOMfig struct
  end;
  if (nargin < 2)
    figH = gcbf;			% figH is calling figure
  end;
  local_create(figH, type);

else
  if (~exist('action'))
    action = '<None>';
  end;
  fprintf(1,'%s : Unknown argument ''%s''.\n', mfilename, action);
				% mfilename is a Matlab function 
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%  end of main  %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_CREATE

function local_create(h_in, t_in)

global FIGURENAME;
global FIGURETAG;

global h_caller;

global h_rb_save_r1;
global h_rb_save_r2;
global h_rb_save_r3;
global h_rb_to_r1;
global h_rb_to_r2;
global h_et_to_edit1;
global h_et_to_edit2;
global h_cb_params_care;
global h_et_params_care;

origUnits = get(0,'Units');
set(0,'Units','normalized');

h_caller = h_in;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Figure

a = figure('Units','normalized', ...
	'Name',FIGURENAME, ...
	'NumberTitle','off', ...
	'WindowStyle', 'modal', ...
	'Position',[0.2 0.3 0.35 0.28], ...
	'Tag',FIGURETAG);
h = a;

calling_tag = get(h_caller,'tag');
switch calling_tag
  case 'somui_vis'
    set(a,'DeleteFcn','somui_vis(''save2'')');
  case 'somui_it',
    set(a,'DeleteFcn','somui_it(''save2'')');
  otherwise
    fprintf(1,'%s: Error in calling GUI, should be somui_vis or somui_it\n',mfilename);
    set(a,'DeleteFcn','somui_vis(''save2'')');
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Frames:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.01 0.01 0.71 0.98], ...
	'Style','frame', ...
	'Tag','fr_main');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.63 0.69 0.34], ...
	'Style','frame', ...
	'Tag','fr_save');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.27 0.69 0.34], ...
	'Style','frame', ...
	'Tag','fr_to');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.03 0.69 0.23], ...
	'Style','frame', ...
	'Tag','fr_params');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Save Frame:
%   t_in is an argument:
%     1 = map, 2 = data, 3 = SOMfig struct

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.88 0.67 0.08], ...
	'String','Save', ...
	'Style','text', ...
	'Tag','st_save_title');

h_rb_save_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Map', ...
	'Position', [0.03 0.77 0.32 0.1], ... 
	'Tag','rb_save_r1', ...
	'Value', 0);

h_rb_save_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Data', ...
	'Position', [0.03 0.66 0.32 0.1], ...
	'Tag','rb_save_r2', ...
	'Value', 0);

h_rb_save_r3 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','SOMfig struct', ...
	'Position', [0.38 0.77 0.32 0.1], ...
	'Tag','rb_save_r3', ...
	'Value', 0);

% Which radio button is pressed in the default situation
switch t_in
  case 1,
    set(h_rb_save_r1,'Value',1);
  case 2,
    set(h_rb_save_r2,'Value',1);
  case 3,
    set(h_rb_save_r3,'Value',1);
end;

% FUTURE: h_rb_save_r4: Data -- Data Struct

% This procedure has been copied from Matlab's web page,
% Question number xxxx. Executes the mutual exlusive choosing
% of radio buttons.
set(h_rb_save_r1,'UserData',[h_rb_save_r2 h_rb_save_r3]);
set(h_rb_save_r2,'UserData',[h_rb_save_r1 h_rb_save_r3]);
set(h_rb_save_r3,'UserData',[h_rb_save_r1 h_rb_save_r2]);
set([h_rb_save_r1 h_rb_save_r2 h_rb_save_r3],'Callback',...
   ['if(get(gco,''Value'')==1),',...
    'set(get(gco,''UserData''),''Value'',0),',...
    'else,set(gco,''Value'',1),end']);

% negative energy! Hold a mouse (button down) on a radiobutton.
% You see, for example, 2 black of 3 buttons, or 0 of 3 buttons.
% Callback  starts when releasing mouse.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% To Frame:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.51 0.67 0.09], ...
	'String','To', ...
	'Style','text', ...
	'Tag','st_to_title');

h_rb_to_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Workspace', ...
	'Position', [0.03 0.4 0.3 0.1], ...
	'Tag','rb_to_r1', ...
	'Value', 1);

h_rb_to_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','SOM_PAK file', ...
	'Position', [0.03 0.29 0.3 0.1], ...
	'Tag','rb_to_r2', ...
	'Value', 0);

set(h_rb_to_r1,'UserData',[h_rb_to_r2]);
set(h_rb_to_r2,'UserData',[h_rb_to_r1]);
set([h_rb_to_r1 h_rb_to_r2],'Callback',...
   ['if(get(gco,''Value'')==1),',...
      'if(strcmp(get(gco,''Tag''),''rb_to_r1'')),',...
         'set(findobj(''tag'',''et_to_edit1''),''Enable'',''on''),',...
         'set(findobj(''tag'',''pb_to_browse1''),''Enable'',''on''),',...
         'set(findobj(''tag'',''et_to_edit2''),''Enable'',''off''),',...
         'set(findobj(''tag'',''pb_to_browse2''),''Enable'',''off''),',...
      'else,',...
         'set(findobj(''tag'',''et_to_edit1''),''Enable'',''off''),',...
         'set(findobj(''tag'',''pb_to_browse1''),''Enable'',''off''),',...
         'set(findobj(''tag'',''et_to_edit2''),''Enable'',''on''),',...
         'set(findobj(''tag'',''pb_to_browse2''),''Enable'',''on''),',...
      'end,',...
      'set(get(gco,''UserData''),''Value'',0),',...
    'else,',...
       'set(gco,''Value'',1),',...
    'end']);

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment', 'right', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.35 0.4 0.18 0.1], ...
	'String','', ...
	'Style','edit', ...
	'Tag','et_to_edit1');
h_et_to_edit1 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment', 'right', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.35 0.29 0.18 0.1], ...
	'String','', ...
	'Style','edit', ...
	'Enable','off', ...
	'Tag','et_to_edit2');
h_et_to_edit2 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback', 'somui_save(''save_var'');', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.55 0.4 0.14 0.1], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Tag','pb_to_browse1');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback', 'somui_save(''save_file'');', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.55 0.29 0.14 0.1], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Enable','off', ...
	'Tag','pb_to_browse2');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Parameters Frame:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.16 0.67 0.09], ...
	'String','Data writing parameters', ...
	'Style','text', ...
	'Tag','st_params_title');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.03 0.05 0.32 0.1], ...
	'String','Don''t care value:', ...
	'Style','checkbox', ...
	'Value',0,...
	'Tag','cb_params_care');
h_cb_params_care = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment', 'left', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.37 0.05 0.1 0.1], ...
	'String','x', ...
	'Style','edit', ...
	'Enable','off',...
	'Tag','et_params_care');
h_et_params_care = b;

set(h_cb_params_care,'UserData',h_et_params_care);
set(h_cb_params_care,'Callback',...
    ['if (get(gco,''Value'') == 1),',...
       'set(get(gco,''UserData''),''Enable'',''on''),',...
     'else,',...
       'set(get(gco,''UserData''),''Enable'',''off''),',...
     'end']);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Push Buttons
%
%   SAVE, HELP, CANCEL

% FUTURE: maybe save and cancel should change place
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_save(''ok'');', ...
	'Position',[0.74 0.05 0.23 0.15], ...
	'String','Save', ...
	'Tag','pb_save');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','', ...
	'Position',[0.74 0.55 0.23 0.15], ...
	'String','Help', ...
	'Tag','pb_help');

b = uicontrol('Parent',a, ...
	'BusyAction','cancel', ...
	'Units','normalized', ...
	'Callback','delete(gcf)', ...
	'Position',[0.74 0.23 0.23 0.15], ...
	'String','Cancel', ...
	'Tag','pb_cancel');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%  end of local_create  %%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_SV
%    User has clicked BROWSE-button in order to save struct to a
%    variable in Matlab's workspace

function local_sv

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file


global h_et_to_edit1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 

var = somui_sel_var('SOM Select');
if (ischar(var))
  set(h_et_to_edit1,'String',var);
elseif (var == -1)
  fprintf(1, 'Cancel clicked!\n');
elseif (var == -2)
  fprintf(1, 'There is nothing to choose!\n');		% REMOVE?
else
  % Never flows here?
  fprintf(1, 'Variable selection did not work\n');	% REMOVE?
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%% end of local_sv %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_SF
%    User has clicked BROWSE-button in order to save struct to a
%    file in disk

function local_sf

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global h_et_to_edit2;
global h_rb_save_r1;
global h_rb_save_r2;
global h_rb_save_r3;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%   *.cod is for Codebook files
%   *.dat is for data files
%   *.mat is for Matlab files, but user should load them from workspace
%
% Matlab's feature: you can't scroll in a edit window! :-(

tmp = get([h_rb_save_r1 h_rb_save_r2 h_rb_save_r3], 'Value');
filter_index = find([tmp{1} tmp{2} tmp{3}]);

switch filter_index
  case 1,
    filter = '*.cod';
  case 2,
    filter = '*.dat';
  case 3,
    filter = '*.mat';
  otherwise,
    % Never flows here
    fprintf(1, 'Error in somui_save\n');
end;

% [filename, path] = somui_sel_file(filter,'SOM Select',300,200);  % REMOVE?
[filename, path] = uiputfile(filter,'SOM Select',300,200);  
if (ischar(filename))
  tmp = strcat(path,filename);
  set(h_et_to_edit2,'String',tmp);
elseif (filename==0 | path==0)
  fprintf(1, 'Cancel clicked!\n');
else
  % Never flows here, because uigetfile returns [0 0] if
  % user presses 'Cancel' or makes an error
  fprintf(1, 'File selection did not work\n');  
end;



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%% end of local_sf %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_OK
%    User clicks the SAVE button
% 

function local_ok

global FIGURENAME;
global FIGURETAG;

global h_caller;

global h_rb_save_r1;
global h_rb_save_r2;
global h_rb_save_r3;
global h_rb_to_r1;
global h_rb_to_r2;
global h_et_to_edit1;
global h_et_to_edit2;
global h_cb_params_care;
global h_et_params_care;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 

if (get(h_rb_to_r1,'Value') == 1)
  % Save to workspace
  var = get(h_et_to_edit1,'String');		% map, data, somfig
  filename = '';
else
  % Save to file
  % FUTURE: Warn, if user is trying to save over an existing file.
  filename = get(h_et_to_edit2,'String');	% filename
  var = '';
end;

if (isempty(var) & isempty(filename))
  fprintf(1,'Type a variable or file name.\n');
  return;
end;

if (get(h_rb_save_r1,'Value') == 1)
  % Map struct
  % Fetch the map structure from caller's UserData to 'map'
  tmp_userdata = get(h_caller,'UserData');
  if (isfield(tmp_userdata,'map'))
    map = tmp_userdata.map;
  else
    fprintf(1,'Argument not a map struct\n');
    return;
  end;    
  if (isempty(var))
    % save map to file
    som_write_cod(map, filename);
  else
    % save map to the workspace as a variable
    assignin('base',var,map);
  end;
  tmp_userdata.op = 1;				% op == 1 is a map execution
  set(h_caller,'UserData',tmp_userdata);

elseif (get(h_rb_save_r2,'Value') == 1)
  % Data struct
  % Fetch the data structure from caller's UserData to 'data'
  tmp_userdata = get(h_caller,'UserData');
  if (isfield(tmp_userdata,'data'))
    data = tmp_userdata.data;
  else
    fprintf(1,'Argument not a data struct\n');
    return;
  end;    
  % Check the value of 'Don't care' and put it to 'tmp_care'
  %   If tmp_care is empty, then do not use it in som_write_data
  tmp_care = '';
  if (get(h_cb_params_care,'Value') == 1)
    % There is a "don't care" char or string
    tmp_care = get(h_et_params_care,'String');
  end;
  if (isempty(var))
    % save data to file
    if (isempty(tmp_care))
      som_write_data(data, filename);
    else
      som_write_data(data, filename, tmp_care);
    end;
  else
    % save data to the workspace as a variable
    assignin('base',var,data);
  end;
  tmp_userdata.op = 2;				% op == 1 is a data execution
  set(h_caller,'UserData',tmp_userdata);

elseif (get(h_rb_save_r3,'Value') == 1)
  % SOMfig struct
  % Fetch the SOMfig structure from caller's UserData to 'data'
  tmp_userdata = get(h_caller,'UserData');
  if (~isfield(tmp_userdata,'h_somfig'))
    fprintf(1,'Argument not a SOMfig struct\n');
    return;
  end;    
  if (isempty(var))
    % save data to file
    fprintf(1,'%s: Don''t know how to write a SOMfig to a file\n',mfilename);
    return;
  else
    % save data to the workspace as a variable
    assignin('base',var,tmp_userdata);
  end;
  tmp_userdata.op = 3;				% op == 1 is a SOMfig execution
  set(h_caller,'UserData',tmp_userdata);
end;

delete(gcf);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%








