function [h, it] = somui_it(action, arg1)
%SOMUI_IT is a GUI for initialization and training of SOM
%
% [h, it] = somui_it([action], [arg1]) 
%
% ARGUMENTS ([]'s are optional)
%
%  [action]  (string) internal variable
%  [arg1]    (struct) internal argument
%
% RETURNS
%
%  [h]   (handle) handle to IT GUI
%  [it]  (struct) struct  it-struct
%
% [H, IT] = SOMUI_IT([ACTION], [ARG1]) displays a window where the user
% can choose different parameters for initializating and
% training of maps.
%
% When map is trained SOMUI_IT calls SOMUI_VIS with argument
% load_it_structure.
%
% A dummy function SOM_GUI calls this function.
%
% See also SOMUI_VIS, SOMUI_LOAD*, SOMUI_SAVE*, SOMUI_INFO
 
% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 
% Version 1.01 Jukka 180397 -- SOM_DOIT function and button added

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

% handles to certain uicontrol items
global h_st_train;
global h_st_init_map;
global h_st_train_map;
global h_st_init_data;
global h_st_train_data;
global h_pb_train;
global h_pb_close;
global h_et_init_xdim;
global h_et_init_ydim;
global h_pop_init_itype;
global h_pop_init_lattice;
global h_pop_init_shape;
global h_pop_init_neigh;
global h_et_train_alpha;
global h_et_train_inirad;
global h_et_train_finrad;
global h_et_train_epochs;
global h_et_train_wcomp;
global h_pop_train_bs;

global itypestr;
global latticestr;
global shapestr;
global neighstr;
global bsstr;

global ITYPE;
global LATTICE;
global SHAPE;
global NEIGH;

global FIGURENAME;
global FIGURETAG;
global FIGURESIZE;

global LOADMAP_VISIBLE;

% Default variables of size of map
global DEF_XDIM;
global DEF_YDIM;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Check arguments

error(nargchk(0, 2, nargin))  % check no. of input args is correct
error(nargchk(0, 2, nargout)) % check no. of output args is correct

if (nargin < 1)
  action = '';
else
  action = lower(action);
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Default values and Initialiation 

% Some figure constats:
FIGURENAME = 'SOM Toolbox -- Init & Train';
FIGURETAG  = 'somui_it';
FIGURESIZE = [0.09 0.2 0.4 0.6];

% if LOADMAP_VISIBLE is 'on', user may load map from GUI
% if -"-             is 'off', button does not show.
% Search for string 'pb_load_map' in this file
LOADMAP_VISIBLE = 'off';

% Default values for popup menus
% Search for string 'pop_train_itype'
ITYPE	= 2;
LATTICE = 2;
SHAPE	= 1;
NEIGH	= 1;

% Default size is [8 12]
DEF_XDIM = 8;
DEF_YDIM = 12;


% Get the handle to the IT figure == h_figH
set(0, 'ShowHiddenHandles','on');
h_figH = findobj('Type','figure');
h_figH = findobj(h_figH,'flat','Name',FIGURENAME,'Tag',FIGURETAG);
set(0, 'ShowHiddenHandles','off');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This SOMUI_IT should have only one window. Check if there 
%   exists an it-window. If does, get it top,
%   otherwise either do as 'action' says. If there is no
%   'FIGURENAME' window, create one.
%
%   First, read it-struct from UserData

if ~isempty(h_figH)
  it = get(h_figH,'UserData');   % loads it-struct from UserData


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTIONs to be taken
%   ''				bring somui_it on top
%   'delete'			deletes IT GUI
%   'read_mask'			Reads a mask vector to Train frame
%   'slider_1'
%   'slider_001'
%   'som_do_it'			DO SOM -button clicked
%   'initialize'		INITIALIZE-button clicked
%   'train'			TRAIN-button clicked
%   'new_map'			NEW MAP -button clicked
%   'load_map'			LOAD MAP -button clicked
%   'modify_GUI_by_map'
%   'modify_GUI_by_data'
%   'save_map'			SAVE MAP -button clicked
%   'load_data'			LOAD DATA -button clicked
%   <unknown action>		prints warning text
%   <otherwise>			creates a somui_it GUI


  if (strcmp(action,''))                   
    figure(h_figH);
    % Remove next line?
    fprintf(1,'''%s'' figure (%d) is now active\n',FIGURENAME,h_figH);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'delete'

  elseif (strcmp(action,'delete'))
    delete(h_figH);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'read_mask'

  elseif (strcmp(action,'read_mask'))
    vector=somui_sel_var;
    vector=evalin('base',vector)
%    tmp = strcat('[',num2str(vector),']');
    tmp = num2str(vector);
    set(h_et_train_wcomp,'String',tmp);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'slider_1'

  elseif (strcmp(action,'slider_1'))
    local_slider_1(arg1);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'slider_001'

  elseif (strcmp(action,'slider_001'))
    local_slider_001(arg1);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'som_do_it'
%   DO SOM -button clicked;
%
% 1. check that there is data available
% 2. call for som_doit-function

  elseif (strcmp(action,'som_do_it'))
    if (isfield(it,'data_loaded'))
      if (it.data_loaded)
        som_doit(it.data); 
        % DOES NOT MODIFY GUI!!!
      end
    end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'initialize'
%   INITIALIZE-button clicked;
%
% 1. make a warning if user tries to init a map already trained
% 2. read the parameters
% 3. ask user give missing parameters --> 2
% 4. som_init
%
% it-struct is sparse: must contain it.map.train_sequence, it.data.data.
%   'it' itself exists.
%
% Note that variable 'train_sequence', 'comp weights' and 'neighbourhood'
% remain empty until training begins.

  elseif (strcmp(action,'initialize'))

    % warn if user is trying to init a map already trained
    %   'Yes' means that's ok to re-init
    if (isfield(it,'map_loaded'))
      if (it.map_loaded) 
        if (~isempty(it.map.train_sequence))
          btn = questdlg('Do you really want to re-initialize a trained map?', ...
                         'Re-initializating?','Yes','No','No');
        else
          btn = 'Yes';
        end;
      else
        btn = 'Yes';
      end;
    else
      % Never flows here.
      fprintf(1,'Internal bug: no it.map_loaded field.\n'); 
      return;
    end;

    switch btn
      case 'Yes'
        % fetch all arguments: data, msize, parameters

        % check that data is available
        if (isfield(it,'data_loaded'))
          if (~it.data_loaded)
            errordlg({'No data!';'Load it before initialization'}, ...
                     'somui_it error'); 
            return;                             
            % User should LOAD DATA...
          end;
        else
          % Never flows here.
          fprintf(1,'Internal bug: no it.data_loaded field.\n'); 
          return;
        end;

        % fetch dimension and put them to 'msize'.
        xdim = get(h_et_init_xdim,'String');
        ydim = get(h_et_init_ydim,'String');
        if (isempty(xdim) | isempty(ydim))
          errordlg('Map dimension not correct!', ...
                   'somui_it error');
          return;
        end;
        xdim = str2num(xdim);             
        ydim = str2num(ydim);
        msize = [xdim ydim];

        % fetch other parameters
        itype = get(h_pop_init_itype,'Value');
        itype = itypestr{itype};
        lattice = get(h_pop_init_lattice,'Value');
        lattice = latticestr{lattice};
        shape = get(h_pop_init_shape,'Value');
        shape = shapestr{shape};
	% neighbourhood belongs logically to training frame

  
        % call init routine
        it.map = som_init(it.data, msize, itype, lattice, shape);
        it.map_loaded = 1;

        % modify IT UI titles: put a map name to titles
        set(h_st_train,'String','Training (status: <not trained>)');
        tmp = strcat('Map: ',it.map.name);
        set(h_st_init_map,'String',tmp);
        set(h_st_train_map,'String',tmp);

        % modify IT UI titles: put a data name to titles
        if (isempty(it.data.name))
          %% only case: user has given '' for a new data name
          %% strcat doesn't like ''.
          it.data.name = '<no data>';  
        end
        tmp = strcat('Data: ',it.data.name);
        set(h_st_init_data,'String',tmp);
        set(h_st_train_data,'String',tmp);

        % save 'it' struct
        set(h_figH,'UserData',it);

      case 'No'
        %% User has misclicked Initialize-button and doesn't want
        %% to continue to re-initalizing the map
        return;
    end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'train'
%   TRAIN-button clicked;
%
% 1. read the parameters
% 2. ask user give missing parameters --> 1
% 3. call som_trainops to add 'neighbourhood', 'training type' and
%      '...'.
% 4. som_train
% 5. draw a SOMfig if asked
%

  elseif (strcmp(action,'train'))

    % check that user has already initialized a map
    if (isfield(it,'map_loaded'))
      if (~it.map_loaded)
        errordlg({'No Map to train!';'First, initialize a map'},...
                 'Error in training');
        return;
      end;
    else
      % Never flows here.
      fprintf(1,'Internal bug: no it.map_loaded field.\n'); 
      return;
    end;

    % check that data is available
    if (isfield(it,'data_loaded'))
      if (~it.data_loaded)
        errordlg({'No data!';'Load it before initialization'}, ...
                 'somui_it error'); 
        return;                             
        % User should LOAD DATA...
      end;
    else
      % Never flows here.
      fprintf(1,'Internal bug: no it.data_loaded field.\n'); 
      return;
    end;

    % FUTURE:
    %   Fetch the default parameters from the som_train().
    %
    % If at least ones value is missing, som_train uses its defaults

    tmp_use_defaults = 0;
    alpha = get(h_et_train_alpha,'String');
    alpha = str2num(alpha);
    inirad = get(h_et_train_inirad,'String');
    inirad = str2num(inirad);
    finrad = get(h_et_train_finrad,'String');
    finrad = str2num(finrad);
    epochs = get(h_et_train_epochs,'String');
    epochs = str2num(epochs);
    if (isempty(alpha) | isempty(inirad) | isempty(finrad) | ...
        isempty(epochs))
      tmp_use_defaults = 1;
      warndlg({'One or more training parameters missing'; ...
               'Trains with all default parameter values!'}, ...
               'somui_it warning');
    end;

    % Set train options with som_trainops()
    % Belongs to Init frame in GUI
    neigh = get(h_pop_init_neigh,'Value');
    neigh = neighstr{neigh};

    % Train_type;
    bs = get(h_pop_train_bs,'Value');
    bs = bsstr{bs};

    % Mask vector
    weight = get(h_et_train_wcomp,'String');
    if (isempty(weight))
      weight = ones(1, size(it.map.codebook,3));
    else
      if (ischar(weight))
        if (strcmp(weight(1),'['))		% remove [ ... ] if needed
          weight = weight(2:end-1);
        end;
        weight = str2num(weight);		% change to numerals
      end;
      if (size(weight) ~= size(it.map.codebook,3))
        fprintf(1,'Size of the weight array is not correct\n');
        fprintf(1,'Weight Dim should be %d\n',size(it.map.codebook,3));
        return;
      end;
    end;

    % CALL trainops()
    it.map = som_trainops(it.map, neigh, bs, weight);

    % disable GUI for training time
    h_tmp = findobj(h_figH);
    h_tmp = h_tmp(2:end);	% remove the figure itself
    watchon;
    set(h_tmp,'Enable','off');

    drawnow;

    % Call SOM_TRAIN
    if (tmp_use_defaults)
      it.map = som_train(it.map, it.data);
    else
      it.map = som_train(it.map, it.data, epochs, [inirad finrad], alpha, 1);
    end;

    % enable GUI
    set(h_tmp,'Enable','on');
    watchoff;

    drawnow;

    % save it struct
    set(h_figH,'UserData',it);

    % Set the training value string. Matlab's bug: spaces are not printed!
    ltmp = length(it.map.train_sequence);
    tmp = it.map.train_sequence{ltmp}.time;
    tmp = strcat('Training  (',num2str(ltmp),'. training at: ',tmp,')');
    set(h_st_train,'String',tmp);    

    % call somui_vis with new trained map
    vis.map = it.map;
    vis.data = it.data; 
    h_somui_vis = findobj('tag','somui_vis');
    if (isempty(h_somui_vis))
      somui_vis;
    end;
    somui_vis('load_it_structure',vis);
    
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% HELP clicked;
 
  elseif (strcmp(action,'help'))
    helpdlg({'Please, refer to the web site:'; ...
             'http://www.cis.hut.fi/projects/somtoolbox/'},...
             'SOM Toolbox -- Help');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% NEW MAP clicked;
 
  elseif (strcmp(action,'new_map'))
    % reset all init-values. Data and train values may stay as they are.
    % reset both variables and gui values!

    if (it.map_loaded)
      if (~isempty(it.map.train_sequence))
        btn = questdlg('Do you really want to have a new map?', ...
                       'New Map?','Yes','No','Yes');
      else
        btn = 'Yes';
      end;
    else
      btn = 'Yes';
    end;

    switch btn
      case 'Yes'
        it.map_loaded = 0;
        % Set default values for GUI objects:
        set(h_et_init_xdim,'String','');
        set(h_et_init_ydim,'String','');
        set(h_pop_init_itype,'Value', ITYPE);
        set(h_pop_init_lattice,'Value', LATTICE);
        set(h_pop_init_shape,'Value', SHAPE);
        set(h_pop_init_neigh,'Value', NEIGH);
	set(h_st_init_map,'String','Map: <new>');
	set(h_st_train_map,'String','Map: <new>');
	set(h_st_train,'String','Training (status: <no map>)');
	drawnow;

        set(h_figH,'UserData',it);

      case 'No'
        %% Misclicked
        return;
    end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% LOAD MAP clicked;

  elseif (strcmp(action,'load_map'))
    somui_loadmap('create', h_figH);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% LOAD DATA clicked;

  elseif (strcmp(action,'load_data'))
    somui_loaddata('create', h_figH);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% SAVE MAP clicked;

  elseif (strcmp(action,'save_map'))
    if (isfield(it,'map'))
      somui_savemap('create',it.map);
    else
      errordlg('No map to save!', ...
               'somui_it error'); 
      return;                             
    end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == load2
%   somui_loadmap and somui_loaddata() have as their DeleteFcn
%   a call to this ACTION. They return internally a variable 'op',
%   which is 1 for map and 2 for data loading.
%     If no 'op' exist, then user has pressed 'Cancel'.

  elseif (strcmp(action,'load2'))
    if (~isfield(it,'op'))
      return;			% cancelled
    end;
    if (it.op == 1)
      somui_it('modify_gui_by_map');			% modify GUI
    elseif (it.op == 2)
      somui_it('modify_gui_by_data');			% modify GUI
    else
      fprintf(1,'%s: Error in loading ''load2''\n', mfilename);
    end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'modify_gui_by_map'

  elseif (strcmp(action,'modify_gui_by_map'))
    if (isfield(it,'map'))
      it.map_loaded = 1;
      set(h_figH,'UserData',it);
    end;
    tmp = strcat('Map: ',it.map.name);
    set(h_st_init_map,'String',tmp);
    set(h_st_train_map,'String',tmp);
    set(h_st_train,'String','Training (status: <not trained>)');

    % Modify the whole GUI!
%MAKING ERRORS HERE? BUGS?
    set(h_et_init_xdim,'String',num2str(it.map.msize(1)));
    set(h_et_init_ydim,'String',num2str(it.map.msize(2)));
    switch it.map.init_type
      case 'linear',
        tmp = 2;
      case 'random',
        tmp = 1;
      otherwise		% 'unknown'     MAKING ERRORS HERE!?
                        % GUI has no 'unknown' checkings so far
        tmp = 2;
    end;
    set(h_pop_init_itype,'Value',tmp);    
    switch it.map.lattice
      case 'rect',
        tmp = 1;
      case 'hexa',
        tmp = 2;
    end;
    set(h_pop_init_lattice,'Value',tmp);    
    switch it.map.shape
      case 'rect',
        tmp = 1;
      case 'cyl',
        tmp = 2;
      case 'toroid',
        tmp = 3;
    end;
    set(h_pop_init_shape,'Value',tmp);    
    switch it.map.neigh
      case 'gaussian',
        tmp = 1;
      case 'bubble',
        tmp = 2;
      case 'ep',
        tmp = 3;
      case 'catgaussian',
        tmp = 4;
    end;
    set(h_pop_init_neigh,'Value',tmp);    
    switch it.map.train_type
      case 'seq',
        tmp = 1;
      case 'batch',
        tmp = 2;
    end;
    set(h_pop_train_bs,'Value',tmp);    

    if (~isempty(it.map.train_sequence))
      % Read 
      tmpt = it.map.train_sequence{end};
      set(h_et_train_alpha,'String',num2str(tmpt.alpha_ini));
      set(h_et_train_inirad,'String',num2str(tmpt.radius_ini));
      set(h_et_train_finrad,'String',num2str(tmpt.radius_fin));
      set(h_et_train_epochs,'String',num2str(tmpt.epochs));
      tmps = num2str(it.map.mask');
      tmps = strcat('[',tmps,']');
      set(h_et_train_wcomp,'String',tmps);

      % Check this out!
      % FUTURE: ...
    else
      % Default values
      fprintf(1,'Using default values as training parameters\n');
    end;
   

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'modify_gui_by_data'

  elseif (strcmp(action,'modify_gui_by_data'))
    if (isfield(it,'data'))
      it.data_loaded = 1;
      set(h_figH,'UserData',it);
    end;
    tmp = strcat('Data: ',it.data.name);
    set(h_st_init_data,'String',tmp);
    set(h_st_train_data,'String',tmp);
     
    %% Do we need something else


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% COMMAND LINE somui_it with unknown argument

% mfilename is a Matlab function 

  else                                      
    fprintf(1,'%s : Unknown argument ''%s''.\n', ... 
              mfilename,action);

  end  %% end of 'action' section

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% No arguments and no SOMUI_IT window exists.
%    Create one SOMUI_IT UI 
%    Create it-struct

else                                         
  local_create;
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%  end of main %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_CREATE

function local_create

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

% handles to certain uicontrol items
global h_st_train;
global h_st_init_map;
global h_st_train_map;
global h_st_init_data;
global h_st_train_data;
global h_pb_train;
global h_pb_close;
global h_et_init_xdim;
global h_et_init_ydim;
global h_pop_init_itype;
global h_pop_init_lattice;
global h_pop_init_shape;
global h_pop_init_neigh;
global h_et_train_alpha;
global h_et_train_inirad;
global h_et_train_finrad;
global h_et_train_epochs;
global h_et_train_wcomp;
global h_pop_train_bs;

global FIGURENAME;
global FIGURETAG;
global FIGURESIZE;

global ITYPE;
global LATTICE;
global SHAPE;
global NEIGH;

global itypestr;
global latticestr;
global shapestr;
global neighstr;
global bsstr;

global LOADMAP_VISIBLE;

% Default variables of size of map
global DEF_XDIM;
global DEF_YDIM;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Initialization                           

% These popupmenu items must be written twice:
%   * for the GUI ('Rect';'Cyl';...)
%   * for som_xxx() ('rect';'cyl';...)
itypestr   = {'random';'linear'};
latticestr = {'rect';'hexa'};
shapestr   = {'rect';'cyl';'toroid'};
neighstr   = {'gaussian';'bubble';'ep';'cutgaussian'};

bsstr      = {'seq';'batch'};

% FUTURE:
%   Check the data and propose certain training parameters.


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

origUnits = get(0,'Units');
set(0,'Units','normalized');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Uicontrols are done with help of 'guide'. See
%   HELP GUIDE and 'guide' who it works. These controls
%   have been edited.
%
% See 'somui_it.txt' for extra information
%
%  Order of graphic object:
%    SOMUI_IT figure
%    Frames: 
%     init frame:
%     train frame:
%    Push buttons
%
% ATTENTION! This is only the init phase. Lots of other figures
% and so on can be emerged after initialization and they are not
% shown here. You must search them with 'get' and 'findobj' and
% modify with 'set'. 


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Figure

a = figure('Units','normalized', ...
	'Name',FIGURENAME, ...
	'NumberTitle','off', ...
	'Position', FIGURESIZE, ...
	'Tag',FIGURETAG);
h = a;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Frames:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.01 0.01 0.71 0.98], ...
	'Style','frame', ...
	'Tag','fr_main');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.51 0.69 0.47], ...
	'Style','frame', ...
	'Tag','fr_init');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.02 0.69 0.48], ...
	'Style','frame', ...
	'Tag','fr_train');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Init Frame:
%%   st, st, st, st/2 st/2 st/2 pop/2 pop/2 pop2, cb

%	'BackgroundColor',[0.701961 0.701961 0.701961], ...

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.93 0.66 0.04], ...
	'String','Initialization', ...
	'Style','text', ...
	'Tag','st_init_title');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.85 0.85 0.85], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.87 0.66 0.04], ...
	'String','Map: <new>', ...
	'Style','text', ...
	'Tag','st_init_map');
h_st_init_map = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.85 0.85 0.85], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.82 0.66 0.04], ...
	'String','Data: <No data>', ...
	'Style','text', ...
	'Tag','st_init_data');
h_st_init_data = b;

% xdim ydim  /static text + edit

% Horizontal size, in msize the second component [... #columns]
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.77 0.32 0.04], ...
	'String','Horiz. map size:', ...
	'Style','text', ...
	'Tag','st_init_ydim');

[foo, h_et_init_ydim] = local_slider_create(a, [0.37 0.77 0.32 0.04], 1, 100, 12);

% Vertical size, in msize the first component [#rows ...]
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.72 0.32 0.04], ...
	'String','Vert. map size:', ...
	'Style','text', ...
	'Tag','st_init_xdim');

[foo, h_et_init_xdim] = local_slider_create(a, [0.37 0.72 0.32 0.04], 1, 100, 8);

% type, lattice, shape  /static text:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.67 0.32 0.04], ...
	'String','Init type:', ...
	'Style','text', ...
	'Tag','st_init_type');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.62 0.32 0.04], ...
	'String','Lattice:', ...
	'Style','text', ...
	'Tag','st_init_lattice');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.57 0.32 0.04], ...
	'String','Shape:', ...
	'Style','text', ...
	'Tag','st_init_shape');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.52 0.32 0.04], ...
	'String','Neighbourhood:', ...
	'Style','text', ...
	'Tag','st_init_neigh');

% FUTURE: If you want to change the order of Random/Linear...
%   How to find?
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.67 0.32 0.04], ...
	'String',{'Random';'Linear'}, ...
	'Style','popupmenu', ...
	'Value', ITYPE, ...
	'Tag','pop_init_itype');
h_pop_init_itype = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.62 0.32 0.04], ...
	'String',{'Rect';'Hexa'}, ...
	'Style','popupmenu', ...
	'Value', LATTICE, ...
	'Tag','pop_init_lattice');
h_pop_init_lattice = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.57 0.32 0.04], ...
	'String',{'Rect';'Cyl';'Toroid'}, ...
	'Style','popupmenu', ...
	'Value', SHAPE, ...
	'Tag','pop_init_shape');
h_pop_init_shape = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.52 0.32 0.04], ...
	'String',{'Gaussian';'Bubble';'Ep';'Cutgaussian'}, ...
	'Style','popupmenu', ...
	'Value', NEIGH, ...
	'Tag','pop_init_neigh');
h_pop_init_neigh = b;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Train Frame:
%%   st, st, st, st/2 st/2 st/2 st/2 pop/2 pop/2 pop2 pop/2, cb


b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'FontWeight','bold', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.45 0.66 0.04], ...
	'String','Training (status: <no map>)', ...
	'Style','text', ...
	'Tag','st_train_title');
h_st_train = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.85 0.85 0.85], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.40 0.66 0.04], ...
	'String','Map: <new>', ...
	'Style','text', ...
	'Tag','st_train_map');
h_st_train_map = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.85 0.85 0.85], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.35 0.66 0.04], ...
	'String','Data: <No data>', ...
	'Style','text', ...
	'Tag','st_train_data');
h_st_train_data = b; 


% This parameter comes valid after first training although
%   it is written already in the initialization
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment', 'left', ...
	'Position',[0.03 0.30 0.32 0.04], ...
	'String','Training', ...
	'Style','text', ...
	'Tag','st_train_bs');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.30 0.32 0.04], ...
	'String',{'sequential';'batch'}, ...
	'Style','popupmenu', ...
	'Tag','pop_train_bs', ...
	'Value',2);
h_pop_train_bs = b;

% init alpha, radius, fin radius, epochs /static text:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.24 0.32 0.04], ...
	'String','Initial alpha:', ...
	'Style','text', ...
	'Tag','st_train_alpha');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.19 0.32 0.04], ...
	'String','Initial Radius:', ...
	'Style','text', ...
	'Tag','st_train_iniradius');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.14 0.32 0.04], ...
	'String','Final Radius:', ...
	'Style','text', ...
	'Tag','st_train_finradius');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.09 0.32 0.04], ...
	'String','Epochs:', ...
	'Style','text', ...
	'Tag','st_train_epochs');
% FUTURE:
%   read weight variables from a variable in workspace

% alpha, radius, epocs, wcomp  /edit texts

[foo, h_et_train_alpha] = local_slider_create(a, [0.37 0.24 0.32 0.04], ...
                             0, 1, 0.2, 0.01);

[foo, h_et_train_inirad] = local_slider_create(a, [0.37 0.19 0.32 0.04], 1, 100, 4);

[foo, h_et_train_finrad] = local_slider_create(a, [0.37 0.14 0.32 0.04], 0, 10, 1);

[foo, h_et_train_epochs] = local_slider_create(a, [0.37 0.09 0.32 0.04], 0, 100, 6);

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_it(''read_mask'')', ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.04 0.1 0.04], ...
	'String','Read', ...
	'Style','pushbutton', ...
	'Tag','pb_train_wcomp');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.14 0.04 0.18 0.04], ...
	'String','mask:', ...
	'Style','text', ...
	'Tag','st_train_wcomp');
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[1 1 1], ...
	'HorizontalAlignment','left', ...
	'Position',[0.37 0.04 0.32 0.04], ...
	'String','', ...
	'Style','edit', ...
	'Tag','et_train_wcomp');
h_et_train_wcomp = b;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Push Buttons:
%   LOAD DATA, DO SOM, TRAIN, SAVE MAP, CLOSE

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_it(''load_data'')', ...
	'Position',[0.74 0.91 0.23 0.065], ...
	'String','Load Data...', ...
	'Tag','pb_load_data');

% Loading a Map is possible when changing Visible's off -> on
% This is done in the environment variable LOADMAP_VISIBLE
% in the beginning of this file 
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_it(''load_map'')', ...
	'Position',[0.74 0.735 0.23 0.065], ...
	'String','Load Map...', ...
	'Visible', LOADMAP_VISIBLE, ...
	'Tag','pb_load_map');

b = uicontrol('Parent',a, ...
	'BusyAction','cancel', ...
	'Units','normalized', ...
	'Callback','somui_it(''som_do_it'')', ...
	'Position',[0.74 0.82 0.23 0.065], ...
	'String','Do SOM', ...
	'Tag','pb_do_som');

b = uicontrol('Parent',a, ...
	'BusyAction','cancel', ...
	'Units','normalized', ...
	'Callback','somui_it(''initialize'')', ...
	'Position',[0.74 0.65 0.23 0.065], ...
	'String','Initialize', ...
	'Tag','pb_initialize');

% Interruptible lisatyy 1.8.
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Interruptible','off', ...
	'Callback','somui_it(''train'')', ...
	'Position',[0.74 0.43 0.23 0.065], ...
	'String','Train', ...
	'Tag','pb_train');
h_pb_train = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','somui_it(''save_map'')', ...
	'Position',[0.74 0.2 0.23 0.065], ...
	'String','Save Map...', ...
	'Tag','pb_save_map');

b = uicontrol('Parent',a, ...
	'BusyAction','cancel', ...
	'Units','normalized', ...
	'Callback','delete(gcf)', ...
	'Position',[0.74 0.03 0.23 0.065], ...
	'String','Close', ...
	'Tag','pb_close');
h_pb_close = b;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UIMENU: Menus
%
%   Tools
%     Visualization:
%     SOM Demos:
%   Info
%     Info on current Map
%     Info on current Data
%   SOM Toolbox Help
%     Web pages
%     Help dialog
%

b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&Tools', ...
	'Tag','uim_tools');

c = uimenu('Parent',b, ...
	'Callback','somui_vis', ...
	'Label','Visualization...', ...
	'Tag','uim_tools_vis');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''Data preprocessing'',''Preprocessing functions in command line mode'',''/home/info/parvi/tyo/MAT/testing.txt'')', ...
	'Visible','off', ...
	'Label','Data preprocessing...', ...
	'Tag','uim_tools_dp');

c = uimenu('Parent',b, ...
	'Callback','som_demo', ...
	'Separator','on', ...
	'Label','SOM Demos...', ...
	'Tag','uim_tools_demo');

b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&Info', ...
	'Tag','uim_info');

c = uimenu('Parent',b, ...
	'Callback','somui_info', ...
	'Label','Info on current Map', ...
	'Tag','uim_info_map');
% Careful with Tag: see somui_info

c = uimenu('Parent',b, ...
	'Callback','somui_info', ...
	'Label','Info on current Data', ...
	'Tag','uim_info_data');
% Careful with Tag: see somui_info



b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&SOM Toolbox Help', ...
	'Tag','uim_help');

%% Tee somui_mlg2 joka lukee tiedoston argumenttina, muuttaa
%% tiedoston sisallon stringiksi ja sitten kutsuu helpwin(..,..,..)

%% Olemassa somui_mlg, joka tekee oman ikkunan. Tarvitaanko olleskaan?
%% Tehdaan helppifileista somhelp.m, jossa kommenttina kaikki

c = uimenu('Parent',b, ...
	'Callback','helpwin somui_help', ...
	'Visible', 'on', ...
	'Label','Help SOM Toolbox', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','helpwin somui_it', ...
	'Visible', 'on', ...
	'Label','Help somui_it', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''How to Initialize'',''Init'',''/help/howtoinitialize.txt'')', ...
	'Visible', 'off', ...
	'Label','How to Initialize...', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''How to train'',''Train'',''/help/howtotrain.txt'')', ...
	'Visible', 'off', ...
	'Label','How to Train...', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''SOM structs'',''Map and Data structs'',''help/structs.txt'')', ...
	'Separator','on',...
	'Visible', 'off', ...
	'Label','SOM Toolbox structs...', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/'')', ...
	'Label','SOM Toolbox http://www.cis.hut.fi/projects/somtoolbox/', ...
	'Separator', 'on', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/somintro/som.html'')', ...
	'Label','Intro to SOM from web site', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/docu.html'')', ...
	'Label','Intro to SOM Toolbox from web site', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''About SOM Toolbox for Matlab 5'',''Version 1.0beta'',''help/about.txt'')', ...
	'Separator', 'on', ...
	'Visible', 'off', ...
	'Label','About...', ...
	'Tag','uim_about');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL + UIMENU: end of ui graphics
%%
%% it.map.train_sequence and it.data are needed in INITIALIZE
%%
%% NOT when used 'exist('it.map.train')' instead of
%%               'exist(it.map.train)'

it.map_loaded = 0;
it.data_loaded = 0;
set(gcf,'UserData',it);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%  end of local_create %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

function [h1, h2] = local_slider_create(parent, pos, min, max, init, type)

if (nargin == 5)
  type = 1;
end;

h1 = uicontrol('Parent', parent, ...
	'Units','normalized', ...
	'BackgroundColor',[1 1 1], ...
	'Position', [pos(1) pos(2) 0.65*pos(3) pos(4)], ...
	'Style','slider', ...
	'SliderStep',[0.01 0.1], ...
	'Tag','sl_slider_1', ...
	'Min', min, ...
	'Max', max, ...
	'Value', init);

h2 = uicontrol('Parent', parent, ...
	'Units','normalized', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[pos(1)+0.7*pos(3) pos(2) 0.3*pos(3) pos(4)], ...
	'String', init, ...
	'Style','edit', ...
	'Tag','et_slider_1');

ud.h = [h1 h2];
ud.value = init;
set([h1 h2],'UserData',ud);
if (type == 1)
  set([h1 h2],'Callback',...
            ['ud = get(gco,''UserData'');', ...
            'somui_it(''slider_1'',ud.h)']);
else   % type == 0.01
  set([h1 h2],'Callback',...
            ['ud = get(gco,''UserData'');', ...
            'somui_it(''slider_001'',ud.h)']);
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_SLIDER_1
%    The exact value is always read from the edit box
%    ATTENTION! The value in the slider is modified only when
%    user presses RETURN.

function local_slider_1(h)

h1 = h(1);
h2 = h(2);
slmax = get(h1,'max');
slmin = get(h1,'min');

if (gco == h1)			% slider clicked
  curr       = get(h1,'Value');
  tmp        = get(h1,'UserData');
  old        = tmp.value;
  new        = old + sign(curr - old);
  % if bigger jump, that is, clicked or dragged on slider 
  % Magic number 0.011 is little bigger than SliderStep(1)
  if (abs(curr-old) > 0.011 * (slmax-slmin))
    new = round(curr);
  end; 
  tmp.value = new;
  set(h1,'Value',new,'UserData',tmp);
  set(h2,'String',num2str(new),'UserData',tmp);

elseif (gco == h2)		% RETURN pressed in edit box
  tmp = get(h2,'UserData');
  new = str2num(get(h2,'String'));
  if (new > slmax)
    set(h1,'Value', slmax);
    fprintf(1,'Too big value for the slider!\n');
  elseif (new < slmin)
    set(h1,'Value', slmin);
    fprintf(1,'Too small value for the slider!\n');
  else
    set(h1,'Value',new);
    tmp.value = new;
    set([h1 h2],'UserData',tmp);
  end;  
else
  fprintf(1,'Error in local_slider\n');
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_SLIDER_001
%    The exact value is always read from the edit box
%    ATTENTION! The value in the slider is modified only when
%    user presses RETURN.

function local_slider_001(h)

h1 = h(1);
h2 = h(2);
slmax = get(h1,'max');
slmin = get(h1,'min');

if (gco == h1)			% slider clicked
  curr       = get(h1,'Value');
  tmp        = get(h1,'UserData');
  old        = tmp.value;
  new        = old + 0.01*sign(curr - old);
  % if bigger jump, that is, clicked or dragged on slider 
  % Magic number 0.011 is little bigger than SliderStep(1)
  if (abs(curr-old) > 0.011 * (slmax-slmin))
    new = (round(100*curr))/100;
  end; 
  tmp.value = new;
  set(h1,'Value',new,'UserData',tmp);
  set(h2,'String',num2str(new),'UserData',tmp);

elseif (gco == h2)		% RETURN pressed in edit box
  tmp = get(h2,'UserData');
  new = str2num(get(h2,'String'));
  if (new > slmax)
    set(h1,'Value', slmax);
    fprintf(1,'Too big value for the slider!\n');
  elseif (new < slmin)
    set(h1,'Value', slmin);
    fprintf(1,'Too small value for the slider!\n');
  else
    set(h1,'Value',new);
    tmp.value = new;
    set([h1 h2],'UserData',tmp);
  end;  
else
  fprintf(1,'Error in local_slider\n');
end;



