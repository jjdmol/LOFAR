function somui_load(action, figH, type)
%SOMUI_LOAD displays a window for loading a map or data.
%
%  somui_load([action],[figh],[type])
%
% ARGUMENTS
%
%  [action] (string)  action to be taken
%  [figH]   (handle)  callingfigure's handle
%  [type]   (integer)  default for loading map/data/SOMfig struct
%
% SOMUI_LOAD internally returns map, data or SOMfig struct to 
% caller's UserData
%
% See also SOM_READ_COD, SOM_READ_DATA, SOMUI_SEL_VAR, SOMUI_SEL_FILE,
%          SOMUI_SAVE

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global FIGURENAME;
global FIGURETAG;

global h_caller;
global op;

global h_rb_load_r1;
global h_rb_load_r2;
global h_rb_load_r3;
global h_rb_from_r1;
global h_rb_from_r2;
global h_et_from_edit1;
global h_et_from_edit2;
global h_rb_params_r1;
global h_rb_params_r2;
global h_et_params_edit2;
global h_cb_params_care;
global h_et_params_care;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments


% GUI cannot return any values but of the first round, 
%   for example, figure's handle. Return values must be
%   done by internal structures

error(nargchk(0, 3, nargin))  % check no. of input args is correct
error(nargchk(0, 0, nargout)) % check no. of output args is correct

if (nargin < 3)
  type = 1;			% default: Load a map struct
end;
if (nargin < 2)
  figH = gcbf;			% figH is calling figure
end;
if (nargin < 1)
  action = 'create';		% default action is 'create'
end; 
action = lower(action);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization                           

FIGURENAME = 'SOM Toolbox -- Load';
FIGURETAG  = 'somui_load';

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTIONs to be taken
%   'local_loadfile'
%   'local_loadvar'
%   'local_ok'
%   'local_create'

if (strcmp(action,'load_file'))
  local_lf;

elseif (strcmp(action,'load_var'))
  local_lv;

elseif (strcmp(action,'ok'))		% LOAD!
  local_ok;

elseif (strcmp(action,'create'))
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

function local_create(h_in, t_in);

global FIGURENAME;
global FIGURETAG;

global h_caller;
global op;

global h_rb_load_r1;
global h_rb_load_r2;
global h_rb_load_r3;
global h_rb_from_r1;
global h_rb_from_r2;
global h_et_from_edit1;
global h_et_from_edit2;
global h_rb_params_r1;
global h_rb_params_r2;
global h_et_params_edit2;
global h_cb_params_care;
global h_et_params_care;

origUnits = get(0,'Units');
set(0,'Units','normalized');

h_caller 	= h_in;
op 		= 0;		% setting 'error' value. Will be 
				% changed in local_ok

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Figure

% load2 points to a part of somui_vis or somui_it loading instructions        

a = figure('Units','normalized', ...
	'Name',FIGURENAME, ...
	'NumberTitle','off', ...
	'WindowStyle', 'modal', ...
	'Position',[0.2 0.3 0.35 0.33], ...
	'Tag',FIGURETAG);
h = a;

calling_tag = get(h_caller,'tag');
switch calling_tag
  case 'somui_vis'
    set(a,'DeleteFcn','somui_vis(''load2'')');
  case 'somui_it',
    set(a,'DeleteFcn','somui_it(''load2'')');
  otherwise
    fprintf(1,'%s: Error in calling GUI, should be somui_vis or somui_it\n',mfilename);
    set(a,'DeleteFcn','somui_vis(''load2'')');
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
	'Position',[0.02 0.71 0.69 0.27], ...
	'Style','frame', ...
	'Tag','fr_load');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.42 0.69 0.28], ...
	'Style','frame', ...
	'Tag','fr_from');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.03 0.69 0.38], ...
	'Style','frame', ...
	'Tag','fr_params');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Load Frame:
%   t_in is an argument:
%     1 = map, 2 = data, 3 = SOMfig struct

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.91 0.67 0.06], ...
	'String','Load', ...
	'Style','text', ...
	'Tag','st_load_title');

h_rb_load_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Map', ...
	'Position', [0.03 0.82 0.32 0.08], ... 
	'Tag','rb_load_r1', ...
	'Value', 0);

h_rb_load_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Data', ...
	'Position', [0.03 0.73 0.32 0.08], ...
	'Tag','rb_load_r1', ...
	'Value', 0);

h_rb_load_r3 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','SOMfig struct', ...
	'Position', [0.38 0.82 0.32 0.08], ...
	'Tag','rb_load_r1', ...
	'Value', 0);

% Which radio button is pressed in the default situation
switch t_in
  case 1,
    set(h_rb_load_r1,'Value',1);
  case 2,
    set(h_rb_load_r2,'Value',1);
  case 3,
    set(h_rb_load_r3,'Value',1);
end;

% FUTURE: h_rb_load_r4: Data -- Data Struct

% This procedure has been copied from Matlab's web page,
% Question number xxxx. Executes the mutual exlusive choosing
% of radio buttons.
set(h_rb_load_r1,'UserData',[h_rb_load_r2 h_rb_load_r3]);
set(h_rb_load_r2,'UserData',[h_rb_load_r1 h_rb_load_r3]);
set(h_rb_load_r3,'UserData',[h_rb_load_r1 h_rb_load_r2]);
set([h_rb_load_r1 h_rb_load_r2 h_rb_load_r3],'Callback',...
   ['if(get(gco,''Value'')==1),',...
    'set(get(gco,''UserData''),''Value'',0),',...
    'else,set(gco,''Value'',1),end']);

% negative energy! Hold a mouse (button down) on a radiobutton.
% You see, for example, 2 black of 3 buttons, or 0 of 3 buttons.
% Callback  starts when releasing mouse.

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% From Frame:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.63 0.67 0.06], ...
	'String','From', ...
	'Style','text', ...
	'Tag','st_from_title');

h_rb_from_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Workspace', ...
	'Position', [0.03 0.54 0.3 0.08], ...
	'Tag','rb_from_r1', ...
	'Value', 1);

h_rb_from_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','SOM_PAK file', ...
	'Position', [0.03 0.45 0.3 0.08], ...
	'Tag','rb_from_r2', ...
	'Value', 0);

set(h_rb_from_r1,'UserData',[h_rb_from_r2]);
set(h_rb_from_r2,'UserData',[h_rb_from_r1]);
set([h_rb_from_r1 h_rb_from_r2],'Callback',...
   ['if(get(gco,''Value'')==1),',...
      'if(strcmp(get(gco,''Tag''),''rb_from_r1'')),',...
         'set(findobj(''tag'',''et_from_edit1''),''Enable'',''on''),',...
         'set(findobj(''tag'',''pb_from_browse1''),''Enable'',''on''),',...
         'set(findobj(''tag'',''et_from_edit2''),''Enable'',''off''),',...
         'set(findobj(''tag'',''pb_from_browse2''),''Enable'',''off''),',...
      'else,',...
         'set(findobj(''tag'',''et_from_edit1''),''Enable'',''off''),',...
         'set(findobj(''tag'',''pb_from_browse1''),''Enable'',''off''),',...
         'set(findobj(''tag'',''et_from_edit2''),''Enable'',''on''),',...
         'set(findobj(''tag'',''pb_from_browse2''),''Enable'',''on''),',...
      'end,',...
      'set(get(gco,''UserData''),''Value'',0),',...
    'else,',...
      'set(gco,''Value'',1),',...
    'end']);

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment', 'right', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.35 0.54 0.18 0.08], ...
	'String','', ...
	'Style','edit', ...
	'Tag','et_from_edit1');
h_et_from_edit1 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment', 'right', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.35 0.45 0.18 0.08], ...
	'String','', ...
	'Style','edit', ...
	'Enable','off', ...
	'Tag','et_from_edit2');
h_et_from_edit2 = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_load(''load_var'');', ... 
	'Position',[0.55 0.54 0.14 0.08], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Tag','pb_from_browse1');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_load(''load_file'');', ... 
	'Position',[0.55 0.45 0.14 0.08], ...
	'String','Browse', ...
	'Style','pushbutton', ...
	'Enable','off', ...
	'Tag','pb_from_browse2');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Parameters Frame:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.34 0.67 0.06], ...
	'String','Data reading parameters (files)', ...
	'Style','text', ...
	'Tag','st_params_title');

h_rb_params_r1 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Status line', ...
	'Position', [0.03 0.24 0.32 0.08], ...
	'Tag','rb_params_r1', ...
	'Value', 1);

h_rb_params_r2 = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Style','RadioButton', ...
	'String','Data Dimension:', ...
	'Position', [0.03 0.15 0.32 0.08], ...
	'Tag','rb_params_r2', ...
	'Value', 0);

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Enable','off', ...
	'HorizontalAlignment', 'left', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.37 0.15 0.1 0.08], ...
	'String','guess', ...
	'Style','edit', ...
	'Tag','et_params_edit2');
h_et_params_edit2 = b;

% Radio buttons and en/disabling the edit box of Data dimension:
%   The inner in-clause is for en/disabling, the rest for radio buttons
set(h_rb_params_r1,'UserData',[h_rb_params_r2]);
set(h_rb_params_r2,'UserData',[h_rb_params_r1]);
set([h_rb_params_r1 h_rb_params_r2],'Callback',...
   ['if(get(gco,''Value'')==1),',...
      'if(strcmp(get(gco,''Tag''),''rb_params_r2'')),',...
         'set(findobj(''tag'',''et_params_edit2''),''Enable'',''on''),',...
      'else,',...
         'set(findobj(''tag'',''et_params_edit2''),''Enable'',''off''),',...
      'end,',...
      'set(get(gco,''UserData''),''Value'',0),',...
    'else,',...
      'set(gco,''Value'',1),',...
    'end']);

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.03 0.05 0.32 0.08], ...
	'String','Don''t care value:', ...
	'Style','checkbox', ...
	'Tag','h_cb_params_care');
h_cb_params_care = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Enable','off', ...
	'HorizontalAlignment', 'left', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.37 0.05 0.1 0.08], ...
	'String','x', ...
	'Style','edit', ...
	'Tag','h_et_params_care');
h_et_params_care = b;

% If 'Data dimension' is chosen, then enable the edit box for
%   typing the dimension. If not chosen, disable it.
set(h_cb_params_care,'UserData',h_et_params_care);
set(h_cb_params_care,'Callback',...
    ['if (get(gco,''Value'')==1),', ...
      'set(get(gco,''UserData''),''Enable'',''on''),', ...
      'else, set(get(gco,''UserData''),''Enable'',''off''), end']);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Push Buttons
%
%   LOAD, HELP, CANCEL

% FUTURE: maybe load and cancel should change place
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_load(''ok'');', ...
	'Position',[0.74 0.05 0.23 0.12], ...
	'String','Load', ...
	'Tag','pb_load');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','', ...
	'Position',[0.74 0.4 0.23 0.12], ...
	'String','Help', ...
	'Tag','pb_help');

b = uicontrol('Parent',a, ...
	'BusyAction','cancel', ...
	'Units','normalized', ...
	'Callback','delete(gcf)', ...
	'Position',[0.74 0.2 0.23 0.12], ...
	'String','Cancel', ...
	'Tag','pb_cancel');


% kk
% TESTING
%
%waitfor(a,'userdata');
%fprintf(1,'skdjfkslj\n\t\tsdfk\n');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%  end of local_create  %%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_LV
%    User has clicked BROWSE-button in order to choose a
%    variable from Matlab's workspace

function local_lv

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global h_et_from_edit1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 

var = somui_sel_var('SOM Select');
if (ischar(var))
  set(h_et_from_edit1,'String',var);
elseif (var == -1)
  fprintf(1, 'Cancel clicked!\n');
elseif (var == -2)
  fprintf(1, 'There is nothing to choose!\n');		% REMOVE?
else
  % Never flows here?
  fprintf(1, 'Variable selection did not work\n');	% REMOVE?
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%% end of local_lv %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_LF
%    User has clicked BROWSE-button in order to choose a
%    file from disk

function local_lf

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

global h_et_from_edit2;
global h_rb_load_r1;
global h_rb_load_r2;
global h_rb_load_r3;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
%   *.cod is for Codebook files
%   *.dat is for data files
%   *.mat is for Matlab files, but user should load them from workspace
%
% Matlab's *nasty* feature: you can't scroll in a edit window! :-(

tmp = get([h_rb_load_r1 h_rb_load_r2 h_rb_load_r3], 'Value');
filter_index = find([tmp{1} tmp{2} tmp{3}]);
% DIDN't WORK
%   filter_index = find(get([h_rb_load_r1 h_rb_load_r2 ...
%                            h_rb_load_r3], 'Value'))

switch filter_index
  case 1,
    filter = '*.cod';
  case 2,
    filter = '*.dat';
  case 3,
    % FUTURE:
    %    filter = '*.mat';
    filter = '*.cod';
    fprintf(1, 'Choose ''Map'' or ''Data'' in Load Frame.\n');
  otherwise,
    % Never flows here
    fprintf(1, 'Choose a correct combination in Load and From Frames.\n');    
end;

[filename, path] = somui_sel_file(filter,'SOM Select',300,200);
if (ischar(filename))
  tmp = strcat(path,filename);
  set(h_et_from_edit2,'String',tmp);
elseif (filename==0 | path==0)
  fprintf(1, 'Cancel clicked!\n');
else
  % Never flows here, because uigetfile returns [0 0] if
  % user presses 'Cancel' or makes an error
  fprintf(1, 'File selection did not work\n');  
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%% end of local_lf %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_OK
%    User clicks the LOAD button
% 

function local_ok

global FIGURENAME;
global FIGURETAG;

global h_caller;
global op;			% default value 0 given in local_create

global h_rb_load_r1;
global h_rb_load_r2;
global h_rb_load_r3;
global h_rb_from_r1;
global h_rb_from_r2;
global h_et_from_edit1;
global h_et_from_edit2;
global h_rb_params_r1;
global h_rb_params_r2;
global h_et_params_edit2;
global h_cb_params_care;
global h_et_params_care;


%TESTING:
%set(gcf,'UserData','sksdj');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% 

if (get(h_rb_from_r1,'Value') == 1)
  % Load from workspace
  var = get(h_et_from_edit1,'String');		% map, data, somfig
  filename = '';
else
  % Load from file
  filename = get(h_et_from_edit2,'String');	% filename
  var = '';
end;

if (isempty(var) & isempty(filename))
  fprintf(1,'Type a variable or file name.\n');
  return;
end;

if (get(h_rb_load_r1,'Value') == 1)
  % Map struct
  if (isempty(var))
    % read map from file
    map = som_read_cod(filename);
    if (~exist('map'))
      fprintf(1,'File reading %s did not succeed\n', filename);
      return;
    end;
  else
    % read map from variable
    % first check if variable exists
    found = local_check_var(var);
    if (found == 1)
      map = evalin('base',var);
    else
      fprintf(1,'Variable %s does not exist\n', var);
      return;
    end;
    if (~isfield(map,'codebook'))
      fprintf(1,'Variable %s is not a map structure\n', var);
      return;
    end;
  end;
  tmp = get(h_caller,'UserData');
  tmp.map = map;
  tmp.op = 1;		% return value, op == 1 means map loading operation
%FUTURE:
%  tmp.last.mapfile = ... 
%  Copy the name for the next Load-window
  set(h_caller,'UserData',tmp);
  fprintf(1,'Successful map reading\n');

elseif (get(h_rb_load_r2,'Value') == 1)
  % Data struct					
  if (isempty(var))
    % read data from file
    if (get(h_rb_params_r1,'Value') == 1)
      % Status line in the SOM_PAK file
      arg1 = 'dim_in_data';
    else 					
      % User gives the data dimension, that is in arg1
      tmp_edit2 = get(h_et_params_edit2,'String');
      if (~isempty(tmp_edit2))
        if (strcmp(tmp_edit2,'guess'))
          arg1 = 'dim_in_data';
        else
          arg1 = str2num(tmp_edit2);	% arg1 == <user given dimension>
        end;
      else
        fprintf(1,'Give the data dim or read with status line\n');
        return;
      end;
    end;
    if (get(h_cb_params_care,'Value') == 1)
      % User gives a mark for holes in the data
      if (isempty(get(h_et_params_care,'String')))
        arg2 = 'x';
      else
        arg2 = get(h_et_params_care,'String');
      end;
    else 
      arg2 = '';
    end;
    if (isempty(arg2))
      data = som_read_data(filename, arg1);
    else
      data = som_read_data(filename, arg1, arg2);
    end;
    % makes an error in som_read_dat if fails
  else
    % reads data struct from variable
    found = local_check_var(var);
    if (found == 1)
      data = evalin('base',var);
    else
      fprintf(1,'Variable %s does not exist\n', var);
      return;
    end;
    if (~isfield(data,'data'))
      fprintf(1,'Variable %s is not a data struct, use SOM_DATA_STRUCT\n', var);
      return;
    end;
  end;
  tmp = get(h_caller,'UserData');
  tmp.data = data;
  tmp.op = 2;		% return value, op == 2 means data loading operation
  set(h_caller,'UserData',tmp);
  fprintf(1,'Successful data reading\n');

elseif (get(h_rb_load_r3,'Value') == 1)
  % SOMfig struct
  if (isempty(var))
    fprintf(1,'Cannot read SOMfig struct from file. Use LOAD\n');
    return;
  end;
  % read somfig from variable
  found = local_check_var(var);
  if (found == 1)
    sf = evalin('base',var);
  else
    fprintf(1,'Variable %s does not exist\n', var);
    return;
  end;
  if (~isfield(sf,'sf_params'))
    fprintf(1,'Variable %s is not a SOMfig structure\n', var);
    return;
  end;
  sf.op = 3;		% return value, op == 3 means SOMfig loading operation
  set(h_caller,'UserData',sf);
  fprintf(1,'Successful SOMfig reading\n');
end;

delete(gcf);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_CHECK_VAR

function f_out = local_check_var(varin)

tmpvars = evalin('base','who');
f_out = 0;
for i = 1:length(tmpvars)
  if (strcmp(tmpvars{i},varin))
    f_out = 1;
    i = length(tmpvars);
  end;
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



