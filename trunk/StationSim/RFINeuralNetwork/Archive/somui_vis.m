function [h] = somui_vis(action, arg1)
%SOMUI_VIS is a GUI for visualization of SOM
%
%  [h] = somui_vis([action], [arg1])
%
% ARGUMENTS
%
%   action  (string)  internal variable
%   arg1    (struct)  normally a 'vis' struct
%
% RETURNS
%
%   h   (handle) handle to VIS GUI
%
% VARIABLES
%
%   h_figH  (handle) to VIS window.
%   vis     (struct) somui_vis's UserData
%   h_...:	handles ...
%   UPPERCASE:	default parameters of the GUI
%
% SOMUI_VIS(ACTION,ARG1) displays a window where the user
% can choose different visualizating actions for the map.
%
% See also SOMUI_IT, SOMUI_FIG, SOMUI_SHOW

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 
% Version 1.1 Jukka 180398


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Declare global variables and constants of this file

% handles for listbox and string objects
%   values given in the subfunction local_create_vis
%   used in local_set_values

global h_lb_somfig_ctrl;		% VIS SOMfig control list box
global h_lb_use_comps;			% VIS Use components' list box
global h_et_onfigure_traj;
global h_et_onfigure_hits;
global h_et_onfigure_labels;
global h_pop_show_cb;

global h_st_active_map;
global h_st_active_data;
global h_st_active_somfig;

global h_cb_onfigure_traj;
global h_cb_onfigure_hits;
global h_cb_onfigure_labels;
global h_cb_show_ti;
global h_cb_show_ax;
global h_cb_show_gui;
global h_cb_show_cn;
global h_pop_show_cb;
global h_cb_draw_new;

% default CONSTANTS for GUI
%   values given just below in the initialization phase

global DRAW_NEW;
global TRAJ;
global HITS;
global LABELS;
global SHOW_TI;
global SHOW_AX;
global SHOW_GUI;
global SHOW_CN;
global SHOW_CB;

% other variables/constants

global h_all_somfigs;			% FUTURE: to the vis struct ?
global all_somfig_names;
global FIGURENAME;
global FIGURETAG;
global FIGURESIZE;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Check arguments

error(nargchk(0, 2, nargin))  % check no. of input args is correct
error(nargchk(0, 1, nargout)) % check no. of output args is correct

if (nargin < 1)
  action = '';
else
  action = lower(action);
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialization                           
%   Initialize variables and 'DEFAULS'
%

% DEFAULT values for 'vis' structure
%   Default values can only be changed here. 
%   These defaults are loaded each time a new map is to be drawn.
%
% These defaults are read in the subfunction local_add_defaults
%
% FUTURE?: can be changed in Options menu during the program
%
% See somui_vis.txt for the vis struct.

DRAW_NEW		= 1;
TRAJ			= 0;
HITS			= 0;
LABELS			= 0;
SHOW_TI			= 1;
SHOW_AX			= 0;
SHOW_GUI		= 1;
SHOW_CN			= 1;
SHOW_CB			= 1;		% popupmenu: 1,2

% Some other constats:
FIGURENAME		= 'SOM Toolbox -- Visualization';
FIGURETAG		= 'somui_vis';
FIGURESIZE		= [0.385 0.15 0.4 0.65];

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Get the handle to the VIS figure == h_figH

set(0, 'ShowHiddenHandles','on');
h_figH = findobj('Type','figure');
h_figH = findobj(h_figH, 'flat', 'Name', FIGURENAME, 'Tag', FIGURETAG);
set(0, 'ShowHiddenHandles','off');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% This SOMUI_VIS should have only one window. Check if there 
%   exists a somui_vis window. If does, get it top,
%   otherwise either do as 'action' says. If there is no
%   somui_vis window, create one.

if ~isempty(h_figH)
  vis = get(h_figH,'UserData');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTIONs to be taken
%   ''				bring somui_vis on top
%   'load_it_structure'		called from somui_it
%   'make_somfig_active'	called when double clicked the list box
%				of somfig control frame
%   'delete_somui_vis'		called when somui_vis will be deleted
%   'somfig_ctrl_[]'		somfig control frame, push buttons
%   'use_comps_[]'		use components frame, ...
%   'reset_camera'		reset view angle to ORIG_CAMERA
%   'load_data'			load data -button
%   'load_map'			load map -button
%   'save_map'			save map -button
%   'apply'			apply -button
%   'draw'			draw new -button
%   <unknown action>		prints warning text
%   <otherwise>			creates a somui_vis GUI

  if (strcmp(action,''))                   
    figure(h_figH);
    % REMOVE?:
    % fprintf(1,'''%s'' figure (%d) is now active\n',FIGURENAME,h_figH);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'load_it_structure'
%   somui_it calls this action after training has been executed
%   arg1 is a struct
%
% Creates vis structure
%
% Default, picks all component planes active from map.comp_names.
%
% FUTURE: local_load_in(op, arg1)

  elseif (strcmp(action,'load_it_structure'))
    if (nargin ~= 2)
      errordlg({'somui_vis(''load_it_structure'') needs'; ...
                'one it structure as the second argument.'}, ...
		'error in somui_it/vis');
      return;
    end
    if (~isstruct(arg1))
      errordlg('The second argument must be an it structure', ...
	       'error in somui_it/vis');
      return;
    end
    vis.map = arg1.map;			
    vis.data = arg1.data;
    vis = local_add_defaults(vis);	% reads DEFAULT values
    set(h_figH,'UserData',vis);
    somui_vis('use_comps_refresh');
    somui_vis('use_comps_select_all');	% pick all component planes

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  ACTION == 'make_somfig_active'
%    somui_vis calls this action when user double clicks
%    an item in the list box in somfig control frame.
%
%  FUTURE: Select + Deselect buttons into one line,
%    a new button ACTIVATE, which can be used instead of double click
%  FUTURE: maybe a single click would do the same?
%
%  Check the value of the listbox. If more than one chosen,
%    ignore because only one figure's params can be shown in vis GUI.
%    The old values are saved (?) to the current SOMfig. Pop the 
%    requested SOMfig up and read its params to vis.

  elseif (strcmp(action, 'make_somfig_active'))
    ind_chosen = get(h_lb_somfig_ctrl, 'Value');
    all_somfig_names = get(h_lb_somfig_ctrl, 'String');
    sf_name = all_somfig_names{ind_chosen};
    h_sf = findobj('name', sf_name);
    if (~isempty(h_sf))
      if (length(h_sf) == 1)
        figure(h_sf);
        sf = get(h_sf,'UserData');
        vis = local_set_values(sf);	% reads SOMfigs params to VIS GUI
        set(h_figH, 'UserData', vis);
        somui_vis('somfig_ctrl_refresh');
        set(h_lb_somfig_ctrl,'Value',1);	% 1st of list box is active
      else
        fprintf(1,'Multiple SOMfigs cannot be opened!\n');
      end;
    else
      fprintf(1,'SOMfig did not exist!\n');	
    end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION chosen by button clicking.
%
% ACTION ==			BUTTON clicked:
%    				  Left Side:
%   somfig_ctrl_...		    SOMfig Control:
%				      SELECT ALL
%				      DESELECT
%				      REFRESH
%				      DELETE SOMFIGS
%   use_comps_...		    Component planes:
%				      SELECT ALL
%				      DESELECT
%				  Rigth Side:
%   load_data			    LOAD DATA
%   load_map			    LOAD MAP
%   save_map			    SAVE MAP
%   draw			    DRAW/APPLY
%   helpbtn			    HELP
%   delete_somui_vis		    CLOSE
%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'somfig_ctrl_...'
%
%   clicked in the SOMfig Control frame
%     somfig_ctrl_select_all	SELECT ALL -button
%     somfig_ctrl_deselect	DESELECT -button
%     somfig_ctrl_refresh	REFRESH -button
%     somfig_ctrl_delete	DELETE SOMFIGS -button

  elseif (strcmp(action,'somfig_ctrl_select_all'))
    somui_vis('somfig_ctrl_refresh');
    ind_e = size(get(h_lb_somfig_ctrl,'String'));
    set(h_lb_somfig_ctrl,'Value',[1:ind_e(1)]);

  elseif (strcmp(action,'somfig_ctrl_deselect'))
    somui_vis('somfig_ctrl_refresh');
    set(h_lb_somfig_ctrl,'Value',[]);

  % searchs for all figures' names and put them in the listbox.
  % All SOMfigs have the first six letters of their name 'SOMfig'
  %
  % FUTURE: each SOMfig sends its name to this list
  %   so that refreshing will be useless 
  %
  % Variables:
  %
  % h_all_figs		== handle array to all figures on screen
  % all_fig_names 	== string array of all figure names on screen
  % ind_all_somfigs 	== double array of indices to SOMfigs
  % h_all_somfigs (G)	== handle array to all SOMfigs on screen
  % all_somfig_names (G)== string array of all SOMfig names on screen 

  elseif (strcmp(action,'somfig_ctrl_refresh'))
    h_all_figs 		= findobj('type','figure');
    all_fig_names 	= get(h_all_figs,'name'); 
    ind_all_somfigs 	= strmatch('SOMfig', all_fig_names);  
    h_all_somfigs	= h_all_figs(ind_all_somfigs);    
    all_somfig_names 	= all_fig_names(ind_all_somfigs); 

    set(h_lb_somfig_ctrl,'String', all_somfig_names);
    vis.h_all_somfigs 	= h_all_somfigs;
    vis.all_somfig_names = all_somfig_names;
  % BUG ERROR:
    if (~isempty(h_all_somfigs))
      vis.h_somfig	= h_all_somfigs(1);	% 1st is active ???
    else
      vis.h_somfig	= '';
    end;
    set(h_figH, 'UserData', vis);

  elseif (strcmp(action,'somfig_ctrl_delete'))
    ind_to_be_deleted = get(h_lb_somfig_ctrl,'Value');
    e = size(get(h_lb_somfig_ctrl,'String'));
    % special treatment of indexing:
    if (ind_to_be_deleted == e(1) & ind_to_be_deleted ~= 1)
      set(h_lb_somfig_ctrl,'Value',ind_to_be_deleted-1);
    end
    h_to_be_deleted = h_all_somfigs(ind_to_be_deleted);
    delete(h_to_be_deleted);
    somui_vis('somfig_ctrl_refresh');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'use_comps_...'
%
%   clicked in the Use Component(s) frame
%     use_comps_select_all	SELECT ALL -button
%     use_comps_deselect	DESELECT -button
%
%   use_comps_refresh		internal
%
%   FUTURE: edit text, so that component order can be given

  elseif (strcmp(action,'use_comps_select_all'))
    e = size(get(h_lb_use_comps,'String'));
    set(h_lb_use_comps,'Value',[1:e(1)]);

    % FUTURE: Tool for manipulating component names and order
    % FUTURE: et_nums = num2str([1:e(1)]);
    %         et_text = strcat('[',et_nums,']');
    %         set(h_et_use_comps,'String',et_text);

  elseif (strcmp(action,'use_comps_deselect'))
    set(h_lb_use_comps,'Value',[]);

    % FUTURE: set(h_et_use_comps,'String',[]);
   
  elseif (strcmp(action,'use_comps_refresh'))
    % FUTURE: same procedure in local_set_values
    if (exist('vis'))
      if (isfield(vis,'map'))
        tmp = vis.map.comp_names;
        if ~isempty(tmp)
          tmpstring = cell(size(tmp) + [1 0]);
          tmpstring{1} = 'U-Matrix';
          tmpstring(2:end, :) = vis.map.comp_names;
          set(h_lb_use_comps,'String',tmpstring);
        end;
      end;
    end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'load'
%   LOAD-button clicked;

  elseif (strcmp(action,'load'))
    somui_load('create', h_figH);
    % When the Load window is exiting, it calls load2
    %   Otherwise, if the code of load2 would be here,
    %   the programs continues right after initing the somui_load
    %   window and nothing would work
    % FUTURE: waitfor()

  elseif (strcmp(action,'load2'))
    if (~isfield(vis, 'op'))
      return;			% cancelled
    end;
    if (vis.op == 0) 
      % Cancel clicked
      fprintf(1,'Loading Operation is Cancelled (%d)\n', vis.op);
    elseif (vis.op == 1)
      % Map struct
      fprintf(1,'Loading Operation of SOM map (%d)\n', vis.op);
      vis = local_add_defaults(vis);
      set(h_figH,'UserData',vis);
      tmp = local_set_values(vis);		% used to be ..values2
      somui_vis('use_comps_refresh');
      somui_vis('use_comps_select_all');
    elseif (vis.op == 2)  
      % Data struct   
      fprintf(1,'Loading Operation of Data (%d)\n', vis.op);
    elseif (vis.op == 3)  
      % SOMfig struct
      fprintf(1,'Loading Operation of SOMfig (%d)\n', vis.op);
      tmp = local_set_values(vis);		% used to be ..values2
      somui_vis('use_comps_refresh');
      somui_vis('use_comps_select_all');
    else
      fprintf(1,'%s: Error in loading ''load2''\n', mfilename);
    end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'save'
%   SAVE clicked;
%   Saves the current map, data or vis struct to workspace, 
%   Matlab MAT file or to the SOM_PAK file

  elseif (strcmp(action,'save'))
    somui_save('create', h_figH, 3);

  elseif (strcmp(action,'save2'))
    if (~isfield(vis, 'op'))
      return;			% cancelled
    end;
    if (vis.op == 0)
      % Cancel clicked
      fprintf(1,'Saving Operation is Cancelled (%d)\n', vis.op);
    elseif (vis.op == 1)
      % Map struct
      fprintf(1,'Saving Operation of SOM map (%d)\n', vis.op);
    elseif (vis.op == 2)
      % Data struct   
      fprintf(1,'Saving Operation of Data (%d)\n', vis.op);
    elseif (vis.op == 3)
      % SOMfig struct
      fprintf(1,'Saving Operation of SOMfig (%d)\n', vis.op);
    else
      fprintf(1,'%s: Error in saving ''save2''\n', mfilename);
    end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'draw'
%   DRAW/APPLY -button clicked
%
%   Launch a new SOMfig figure (somui_fig) if needed and calls 
%   somui_show to draw SOM.
%
%   After drawing SOM, check if either trajectory, labels or hits
%   are to be drawn or cleared away.
%   
%   Criteria for drawing a new/existing SOMfig is the component
%   planes from list box 'Use Components / U-matrix': 
%   if chosen, draw; in none, check T/H/L.
%
%   FUTURE: see if 'nothing' has changed. Then don't draw again,
%   if user wants SOMfig to an existing figure.
%
% SEE ALSO: somui_fig, somui_show
%
%   somui_fig creates a figure with UI controls
%   somui_show acts like som_show drawing maps

  elseif (strcmp(action,'draw'))
    if (isfield(vis, 'sf_params'))
      something_drawn = 0;		% for detecting if drawn
      vis = local_read_values(vis);	% read all strings etc up to date
      if (~isempty(vis.use_comps))	% draw/apply comp planes
        sf = local_create_sf(vis);
        if (vis.sf_params.draw_new)	% user wants new window

% FUTURE:
%          if (~exist('h_new_dlg'))	% process window ... --> subfunction
%            h_new_dlg = somui_new_dlg('Jukka');
%            set(h_new_dlg,'Visible','on');
%          else
%            if (~ishandle(h_new_dlg))
%              h_new_dlg = somui_new_dlg('Jukka');
%              set(h_new_dlg,'Visible','on');
%            else
%              set(h_new_dlg,'Visible','on');            
%            end;
%          end;

          sf.sf_name = strcat('SOMfig', somui_next);	% Create a name

          if (sf.sf_params.show_gui==0)			%% 180398
            [h_sf, sf] = somui_fig('no_gui', sf);	%% 180398
            som_show(sf.map);				%% 180398
            return;					%% 180398
	  else						%% 180398
            [h_sf, sf] = somui_fig('create', sf);		
	 	 % SOMUI_FIG creates a new SOMfig window
	  end;    					%% 180398

          vis.h_somfig = h_sf;				% active handle
          set(h_figH, 'UserData', vis);			


        else				% user uses current window
          if (ishandle(vis.h_somfig))	
            h_sf = vis.h_somfig;

% FUTURE
%            if (~exist('h_new_dlg'))	% process window ... --> subfunction
%              h_new_dlg = somui_new_dlg('Jukka');
%              set(h_new_dlg,'Visible','on');
%            else
%              if (~ishandle(h_new_dlg))
%                h_new_dlg = somui_new_dlg('Jukka');
%                set(h_new_dlg,'Visible','on');
%              else
%                set(h_new_dlg,'Visible','on');            
%              end;
%            end;

            figure(h_sf);
            delete(findobj(gcf,'type','axes'));
            delete(findobj(gcf,'type','text'));
          else
            warning('Invalid handle, drawing a new SOMfig');
            set(h_cb_draw_new,'Value',1);
            vis.sf_params.draw_new = 1;
            set(h_figH, 'UserData', vis);
            somui_vis('draw');
            return;
          end;
        end;

        % Now we have an old or new SOMfig window and it is active
        [tmp_h, tmp_hb] = somui_show(sf, h_sf);		
		% SOMUI_SHOW draws a SOM into the existing window
        somui_vis('somfig_ctrl_refresh');		% Refresh the control
        figure(h_sf);
        something_drawn = 1;
      end;			% if (~isempty(vis.use_comps))

      vis = get(h_figH, 'UserData');

      % check if there is no SOMfig
      tmp = ishandle(vis.h_somfig);
      if (isempty(tmp))
        fprintf(1,'%s: There must be an active SOMfig for T/H/L\n',mfilename);
        return;
      elseif (tmp == 0)
        fprintf(1,'%s: Invalid SOMfig handle for T/H/L\n',mfilename);
        return;
      end;

      % Not in use in 1.0beta

      % Now we have an active SOMfig where T/H/L can be drawn

      if (vis.sf_params.traj)
        if (~isempty(vis.traj_data)) 
          tmp = str2num(vis.traj_data);
          h_tmp = som_addtraj(vis.map, vis.data, tmp);
        else
          h_tmp = som_addtraj(vis.map, vis.data);
        end;
        vis.h_traj = h_tmp;
        set(h_figH, 'UserData', vis);
        something_drawn = 1;
      else
        if (isfield(vis,'h_traj'))
          set(vis.h_traj.line, 'Visible', 'off');
          set(vis.h_traj.text, 'Visible', 'off');
        end;
      end;

      % Show Hits
      if (vis.sf_params.hits)

% Options?!

        if (~isempty(vis.hits_data))
          tmp = str2num(vis.hits_data);
          h_tmp = som_addhits(vis.map, vis.data, tmp);
        else
          h_tmp = som_addhits(vis.map, vis.data);
        end;
        vis.h_hits = h_tmp;
        set(h_figH, 'UserData', vis);
        something_drawn = 1;
      else 
        som_clear('Hit');
      end;

      % Show labels
      if (vis.sf_params.labels)
        if (~isempty(vis.labels_data))
          tmp = str2num(vis.labels_data);
          h_tmp = som_addlabels(vis.map, tmp);
        else
          h_tmp = som_addlabels(vis.map);
        end;
        if (~isempty(h_tmp))		% if no labels -> empty label handle
          vis.h_labels = h_tmp;
        end;
        set(h_figH, 'UserData', vis);
        something_drawn = 1;
      else
        if (isfield(vis,'h_labels'))
          set(vis.h_labels, 'Visible', 'off');
        end;
      end;

      if (~something_drawn)
        fprintf(1,'%s: Choose component plane(s) to be drawn\n', mfilename);
      end;
    else
      fprintf(1,'%s: Missing map or data. Load map first\n', mfilename);
    end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'delete_somui_vis'
%   CLOSE-button clicked   

  elseif (strcmp(action, 'delete_somui_vis'))
    if (isfield(vis,'h_somfig'))
      if (~isempty(vis.h_somfig))
        btn = questdlg({'There are still SOMfigs open.'; ...
                        'Do you want to delete somui_vis GUI?'}, ...
			'Deleting somui_vis GUI', ...
                        'Delete','Delete All','Cancel','Cancel');
      else
        btn = 'Delete';
      end;
    else
      btn = 'Delete';
    end;

    switch btn
      case 'Delete'
        delete(gcf);
        fprintf(1,'%s: somui_vis GUI closed!\n',mfilename);
        return;
      case 'Delete All'
        delete(vis.h_all_somfigs);
        delete(gcf);
        fprintf(1,'%s: somui_vis GUI and all SOMfigs closed!\n',mfilename);
        return;
      case 'Cancel'
        return;
    end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION == 'modify_actives'

  elseif (strcmp(action, 'modify_actives'))
    local_modify_actives(vis);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION unknown
%   COMMAND LINE argument unknown
%

  else                                      
    fprintf(1,'%s : Unknown argument ''%s''.\n', mfilename, action);
				% mfilename is a Matlab function 

  end;  %% end of 'action' section

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% ACTION --> create somui_vis
%   No arguments *and* no SOMUI_VIS window exists.
%   Create one SOMUI_VIS UI and vis structure.

else                         
  local_create_vis;
  somui_vis('somfig_ctrl_refresh');	% modify, if SOMfigs exist
end;                

if (exist('vis'))
  local_modify_actives(vis);	% Modify the map, data and active SOMfig info
end;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%% end of main %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%









%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%  SUBFUNCTIONS LOCAL_...  %%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_MODIFY_ACTIVES
 
function local_modify_actives(visin)

global h_st_active_map;
global h_st_active_data;
global h_st_active_somfig;

if (isfield(visin,'map'))
  tmp = strcat('Map: ',visin.map.name);
  set(h_st_active_map,'String',tmp);
end;
if (isfield(visin,'data'))
  tmp = strcat('Data: ',visin.data.name);
  set(h_st_active_data,'String',tmp);
end;
if (isfield(visin,'h_somfig'))
  tmp = ishandle(visin.h_somfig);
  if (~isempty(tmp))
    if (tmp ~= 0)
      tmp = get(visin.h_somfig,'Name');
      tmp = strcat('SOMfig: ',tmp);
      set(h_st_active_somfig,'String',tmp);
    end;
  else
    set(h_st_active_somfig,'String','SOMfig: <No SOMfig>');      
  end;
end;



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% (sub)function LOCAL_CREATE_VIS
%%    Creates GUI without any user information
%%    Default values given in the begin of this file
%%    Creates vis-struct.
%%
%%    REMOVE...
%%    Definetely the first time when this function
%%    is called. So, there is nothing before
%%    this one. No structures in User Data, for example.

function local_create_vis;

% declare globals:

global h_lb_somfig_ctrl;
global h_lb_use_comps;
global h_et_onfigure_traj;
global h_et_onfigure_hits;
global h_et_onfigure_labels;
global h_et_camera;
global h_st_active_map;
global h_st_active_data;
global h_st_active_somfig;

global h_cb_onfigure_traj;
global h_cb_onfigure_hits;
global h_cb_onfigure_labels;
global h_cb_show_ti;
global h_cb_show_ax;
global h_cb_show_gui;
global h_cb_show_cn;
global h_pop_show_cb;
global h_cb_draw_new;

global h_somfigs;
global somfig_names;
global FIGURENAME;
global FIGURETAG;
global FIGURESIZE;

global DRAW_NEW;
global TRAJ;
global HITS;
global LABELS;
global SHOW_TI;
global SHOW_AX;
global SHOW_GUI;
global SHOW_CN;
global SHOW_CB;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Initialization

origUnits = get(0,'Units');
set(0,'Units','normalized');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Uicontrols have been done with help of 'guide'. See
%   HELP GUIDE and 'guide' who it works. These controls
%   have been edited by hands. Don't fetch this file
%   to 'guide' in order to modify GUI - 'guide' will
%   remove all other code.
%
% See 'somui_vis.txt' for extra information
%
% ATTENTION! This is only the init phase. Lots of other figures
%   and so on can be emerged after initialization and they are not
%   shown here. You must search them with 'get' and 'findobj' and
%   modify with 'set'. 
%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%  UICONTROL
%    Order of graphic object families:
%      1. SOMUI_VIS figure
%      2. Frames: big frame in the left, then all frames from top to bottom
%      3. Push Buttons: in the right from top to bottom
%      4. 5. 6. 7. ...
%
%    Variables to be saved in vis-struct:
%      h_somui_vis		handle to SOMfig
%      h_lb_somfig_ctrl		handle to list box object in somfig
%                                 control frame
%      h_lb_use_comps		handle to list box object in use
%				  components frame
%      h_et_onfigure_traj	handle to edit text object of trajectory
%      h_et_onfigure_hits	handle to edit text object of hits
%      h_et_onfigure_labels	handle to edit text object of labels
%      h_et_camera		handle to edit text object of camera
%
%    From command line you can use 'set(h_... ,'tag','...')'
%    in order to modify particular UICONTROL unit.
%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Figure

% TIP: Here you can change the default position of the GUI

%FIGURESIZE=[0.385 0.15 0.4 0.65], ...

a = figure('Units','normalized', ...
	'Name',FIGURENAME, ...
	'NumberTitle','off', ...
	'Position', FIGURESIZE, ...
	'Tag',FIGURETAG);
h = a;
vis.h_somui_vis = a;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Frames

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.01 0.01 0.71 0.98], ...
	'Style','frame', ...
	'Tag','fr_left_big');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.835 0.69 0.145], ...
	'Style','frame', ...
	'Tag','fr_current');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.59 0.69 0.24], ...
	'Style','frame', ...
	'Tag','fr_use_comps');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.4 0.69 0.185], ...
	'Style','frame', ...
	'Tag','fr_add');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.21 0.69 0.185], ...
	'Style','frame', ...
	'Tag','fr_show');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.02 0.02 0.69 0.185], ...
	'Style','frame', ...
	'Tag','fr_somfig_ctrl');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Pushbuttons: In the right side buttons
%   LOAD DATA, LOAD MAP, SAVE MAP, APPLY, DRAW NEW,
%   PRINT, HELP, CLOSE


b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''load'')', ...
	'Position',[0.74 0.91 0.23 0.06], ...
	'String','Load...', ...
	'Tag','pb_load');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''save'')', ...
	'Position',[0.74 0.84 0.23 0.06], ...
	'String','Save...', ...
	'Tag','pb_save');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''draw'')', ...
	'Position',[0.74 0.60 0.23 0.06], ...
	'String','Draw/Apply', ...
	'Tag','pb_draw_new');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''delete_somui_vis'');', ...
	'Position',[0.74 0.04 0.23 0.06], ...
	'String','Close', ...
	'Tag','pb_close');



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Current Information 
%
% More current information from the menu bar 'Info'
%
% Frame: fr_draw_on_grid
%   text: Draw on GRID/SAMMON 'something' as 'something'
%   checkboxes: COMPONENT PLANE, U-MATRIX, MATRIX
%   pop-up menus: 3 x HEIGHT, COLOR, SIZE
%
%   Changes in checkboxes are read immediately to vis-struct by
%   'Callback'-function.
%   
%   Default values can be changed in the beginning of this
%   file 'DEFAULTS'.
%
%   SEE ALSO: somui_fig
%
%   

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.945 0.66 0.03], ...
	'String', 'Current map, data and SOMfig:', ...
	'Style','text', ...
	'Tag','st_draw');

h_st_active_map = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment','left', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'Position',[0.03 0.91 0.66 0.03], ...
	'String','Map: <No map>', ...
	'Style','text', ...
	'Tag','st_active_map');

h_st_active_data = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment','left', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'Position',[0.03 0.875 0.66 0.03], ...
	'String','Data: <No data>', ...
	'Style','text', ...
	'Tag','st_active_data');

% somui_fig calls this tag 'st_active_SOMfig'
h_st_active_somfig = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'HorizontalAlignment','left', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'Position',[0.03 0.84 0.66 0.03], ...
	'String','SOMfig: <No SOMfig>', ...
	'Style','text', ...
	'Tag','st_active_SOMfig');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Use Component(s)
%
% Frame: fr_use_comps
%   text: Use component(s)
%   editbox: listing component planes
%   buttons: SELECT ALL, DESELECT
%   checkboxes: USE MULTIPLE WINDOWS, USE SAME COLOR SCALE
%
% The strings for the list box are read in ...

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.795 0.66 0.03], ...
	'String','Use component plane(s) / U-matrix', ...
	'Style','text', ...
	'Tag','st_use_comps');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','', ...
	'BackgroundColor',[1 1 1], ...
	'Max',2, ...
	'Position',[0.03 0.60 0.33 0.19], ... 
	'Style','listbox', ...
	'Tag','lb_use_comps', ...
	'Value',[]);
h_lb_use_comps = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.draw_new = tmp2;set(gcf,''UserData'',tmp)', ... 
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.75 0.33 0.04], ...
	'String','Draw to new window', ...
	'Style','checkbox', ...
	'Value', DRAW_NEW, ...
	'Tag','cb_draw_new');
h_cb_draw_new = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''use_comps_select_all'')', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.645 0.33 0.04], ...
	'String','Select All', ...
	'Tag','pb_use_comps_sa');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''use_comps_deselect'')', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.60 0.33 0.04], ...
	'String','Deselect all', ...
	'Tag','pb_use_comps_ds');



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Add [trajectory, hits, labels]
%
% Frame: fr_add
%   text: On the figure show...
%   checkboxes: Trajectory, Hits, Labels
%

% Not used in 1.0beta:
%	'String','Add T/H/L on the map with options', ...
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.55 0.66 0.03], ...
	'String','Add T/H/L on the map', ...
	'Style','text', ...
	'Tag','st_onfigure');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.traj = tmp2;set(gcf,''UserData'',tmp)', ... 
	'Position',[0.03 0.505 0.33 0.04], ...
	'String','Trajectory', ...
	'Style','checkbox', ...
	'Tag','cb_onfigure_traj', ...
	'Value',TRAJ);
h_cb_onfigure_traj = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.hits = tmp2;set(gcf,''UserData'',tmp)', ... 
	'Position',[0.03 0.46 0.33 0.04], ...
	'String','Hits', ...
	'Style','checkbox', ...
	'Tag','cb_onfigure_hits', ...
	'Value',HITS);
h_cb_onfigure_hits = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','',...
	'Position',[0.37 0.46 0.33 0.04], ...
	'String','Options for Hits...', ...
	'Enable', 'off', ...
	'Tag','pb_onfigure_hits_opt');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.labels = tmp2;set(gcf,''UserData'',tmp)', ... 
	'Position',[0.03 0.415 0.33 0.04], ...
	'String','Labels', ...
	'Style','checkbox', ...
	'Tag','cb_onfigure_labels', ...
	'Value',LABELS);
h_cb_onfigure_labels = b;

% Not used in 1.0beta
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.37 0.505 0.33 0.04], ...
	'String','', ...
	'Style','edit', ...
	'Visible', 'off', ...
	'Tag','et_onfigure_traj');
h_et_onfigure_traj = b;

% Not used in 1.0beta
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.37 0.46 0.33 0.04], ...
	'String','', ...
	'Style','edit', ...
	'Visible', 'off', ...
	'Tag','et_onfigure_hits');
h_et_onfigure_hits = b;

% Not used in 1.0beta
b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[1 1 1], ...
	'Position',[0.37 0.415 0.33 0.04], ...
	'String','', ...
	'Style','edit', ...
	'Visible', 'off', ...
	'Tag','et_onfigure_labels');
h_et_onfigure_labels = b;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Show in SOMfig:
%
% Frame: fr_show

show_cob_string   = {'A Colorbar each axis'; ...
                     'No Colorbars'};

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.36 0.66 0.03], ...
	'String','Show in SOMfig:', ...
	'Style','text', ...
	'Tag','st_show');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.show_ti = tmp2;set(gcf,''UserData'',tmp)', ... 
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.03 0.315 0.33 0.04], ...
	'String','Title', ...
	'Style','checkbox', ...
	'Tag','cb_show_ti', ...
	'Value',SHOW_TI);
h_cb_show_ti = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.show_ax = tmp2;set(gcf,''UserData'',tmp)', ... 
	'Position',[0.03 0.27 0.33 0.04], ...
	'String','Axis', ...
	'Style','checkbox', ...
	'Tag','cb_show_ax', ...
	'Value',SHOW_AX);
h_cb_show_ax = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.show_gui = tmp2;set(gcf,''UserData'',tmp)', ... 
	'Position',[0.37 0.315 0.33 0.04], ...
	'String','GUI in SOMfig', ...
	'Style','checkbox', ...
	'Tag','cb_show_gui', ...
	'Value', SHOW_GUI);
h_cb_show_gui = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.show_cn = tmp2;set(gcf,''UserData'',tmp)', ... 
	'Position',[0.37 0.27 0.33 0.04], ...
	'String','Component Name', ...
	'Style','checkbox', ...
	'Tag','cb_show_cn', ...
	'Value',SHOW_CN);
h_cb_show_cn = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','tmp=get(gcf,''UserData'');tmp2=get(gcbo,''Value'');tmp.sf_params.show_cb = tmp2;set(gcf,''UserData'',tmp)', ... 
	'Position',[0.03 0.225 0.33 0.04],...
	'String',show_cob_string, ...
	'Style','popupmenu', ...
	'Tag','pop_show_cb', ...
	'Value',SHOW_CB);
h_pop_show_cb = b;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: SOMfigure Control
%
% Frame: fr_somfig_ctrl
%   text
%   editbox: listing current SOMfigs
%   buttons: SELECT ALL, DESELECT, REFRESH, DELETE
%


b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.03 0.17 0.66 0.03], ...
	'String','SOMfigure Control', ...
	'Style','text', ...
	'Tag','st_somfig_ctrl');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','if (strcmp(get(gcf,''SelectionType''),''open'')), somui_vis(''make_somfig_active''); end;', ...
	'BackgroundColor',[1 1 1], ...
	'Max',2, ...
	'Position',[0.03 0.035 0.33 0.13], ... 
	'String','', ...
	'Style','listbox', ...
	'Tag','lb_somfig_ctrl', ...
	'Value',1);
h_lb_somfig_ctrl = b;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''somfig_ctrl_select_all'')', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.125 0.33 0.04], ...
	'String','Select All', ...
	'Tag','pb_somfig_ctrl_sa');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''somfig_ctrl_deselect'')', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.08 0.33 0.04], ...
	'String','Deselect All', ...
	'Tag','pb_somfig_ctrl_ds');

%b = uicontrol('Parent',a, ...
%	'Units','normalized', ...
%	'Callback','somui_vis(''somfig_ctrl_refresh'')', ...
%	'BackgroundColor',[0.701961 0.701961 0.701961], ...
%	'Position',[0.37 0.035 0.33 0.04], ...
%	'String','Refresh', ...
%	'Tag','pb_somfig_ctrl_ref');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','somui_vis(''somfig_ctrl_delete'')', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.37 0.035 0.33 0.04], ...
	'String','Delete selected SOMfigs', ...
	'Tag','pb_somfig_ctrl_de');



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Active components of vis

% CHANGED
%map	'Position',[0.03 0.08 0.67 0.025], ...
%data	'Position',[0.03 0.05 0.67 0.025], ...
%somfig	'Position',[0.03 0.02 0.67 0.025], ...
% somfig was unvisible

% CHANGE: to its own frame up!


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UIMENU: Menus
%
% If you want to make default menus invisible,
% use 'figure's MenuBar property
%

b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&Tools', ...
	'Tag','uim_tools');

c = uimenu('Parent',b, ...
	'Callback','somui_it', ...
	'Label','Init and Train...', ...
	'Tag','uim_tools_it');

c = uimenu('Parent',b, ...
	'Callback','som_demo', ...
	'Separator','on', ...
	'Label','SOM Demos...', ...
	'Tag','uim_tools_demo');

%c = uimenu('Parent',b, ...
%	'Callback','', ...
%	'Label','Label...', ...
%	'Tag','uim_tools_xxx');

b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&Info', ...
	'Tag','uim_info');

c = uimenu('Parent',b, ...
	'Callback','somui_info', ...
	'Label','Info on current &Map...', ...
	'Tag','uim_info_map');
% Careful with Tag: see somui_info

c = uimenu('Parent',b, ...
	'Callback','somui_info', ...
	'Label','Info on current &Data...', ...
	'Tag','uim_info_data');
% Careful with Tag: see somui_info

c = uimenu('Parent',b, ...
	'Callback','somui_info', ...
	'Label','Info on current &VIS structure...', ...
	'Enable','off', ...
	'Tag','uim_info_vis');
% Careful with Tag: see somui_info


b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','SOM f&unctions', ...
	'Tag','uim_somhelp');

c = uimenu('Parent',b, ...
	'Callback',['figure;', ...
                'tmp=get(findobj(''tag'',''somui_vis''),''UserData'');', ...
                'som_show(tmp.map)'], ...
	'Label','s&om_show(map)', ...
	'Tag','');


b = uimenu('Parent',a, ...
	'Callback','', ...
	'Label','&SOM Toolbox Help', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/'');', ...
	'Label','SOM Toolbox http://www.cis.hut.fi/projects/somtoolbox/', ...
	'Tag','uim_tools_demo');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''How to use GUI'',''Init'',''/home/info/parvi/tyo/MAT/testing.txt'')', ...
	'Label','How to use &GUI...', ...
	'Enable', 'off', ...
	'Separator', 'on', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''How to use SOM fcns from GUI '',''Train'',''/home/info/parvi/tyo/MAT/testing.txt'')', ...
	'Enable', 'off', ...
	'Label','How to use SOM &fcns from GUI...', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''How to use SOM fcns from cmd line'',''Train'',''/home/info/parvi/tyo/MAT/testing.txt'')', ...
	'Enable', 'off', ...
	'Label','How to use SOM fcns from &command line...', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','somui_mlg(''SOM structs'',''Map and Data structs'',''/home/info/parvi/tyo/MAT/testing.txt'')', ...
	'Enable', 'off', ...
	'Separator','on',...
	'Label','SOM Toolbox &structs...', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/somintro/som.html'');', ...
	'Separator','on',...
	'Label','&Intro to the SOM from web site', ...
	'Tag','uim_help');

c = uimenu('Parent',b, ...
	'Callback','web(''http://www.cis.hut.fi/projects/somtoolbox/docu.html'');', ...
	'Label','Intro to SOM &Toolbox from web site', ...
	'Tag','uim_help');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Create VIS struct -- DON'T CREATE
%   See somui_vis.txt for vis struct

vis.isvisstruct = 1;
set(gcf,'UserData',vis);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%% end of local_create_vis %%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%




%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function LOCAL_ADD_DEFAULTS
%   Adds the default values of somui_vis GUI
%   to the vis structure. 
%
%     User can change default values in the 
%   beginning of this file, where global variables are
%   announced for the first time.
%
%     There is no map or data structs available at this moment
%   if new SOMfig is to be drawn, i.e., there is only visin.map,
%   visin.data, visin.map_loaded and visin.data_loaded existing.

function visout = local_add_defaults(visin);

global DRAW_NEW;
global TRAJ;	
global HITS;	
global LABELS;
global SHOW_TI;			
global SHOW_AX;			
global SHOW_GUI;		
global SHOW_CN;			
global SHOW_CB;			

visout = visin;			% containing at least map and data structs

visout.sf_params.draw_new	= DRAW_NEW;
visout.sf_params.traj 		= TRAJ;
visout.sf_params.hits 		= HITS;
visout.sf_params.labels 	= LABELS;
visout.sf_params.show_ti 	= SHOW_TI;  
visout.sf_params.show_ax 	= SHOW_AX;
visout.sf_params.show_gui 	= SHOW_GUI;
visout.sf_params.show_cn 	= SHOW_CN;
visout.sf_params.show_cb 	= SHOW_CB;
visout.title 			= '';		% visin.map.name;
visout.traj_data 		= '';		% options
visout.hits_data 		= '';		% options
visout.labels_data 		= '';		% options
visout.sf_name			= '';		% use 'somui_next()'
visout.use_comps		= [];		% which one is better
						% default value
visout.comps			= visout.use_comps;
visout.msize			= visin.map.msize;	% see: som_addhits()
visout.normalization		= visin.map.normalization;
		
%visout.use_comps		= [0 1:length(visin.map.comp_names)];
%visout.h_somfig				% s('somfig_ctrl_refresh')
%visout.h_all_somfigs				% - " -
%visout.all_somfig_names			% - " -

visout.h_somui_it		= findobj(findobj('style','figure'),...
					'tag','somui_it');
visout.h_somui_vis		= findobj(findobj('style','figure'),...
					'tag','somui_vis');

%FUTURE:
%visout.M			= [];		% See somui_show()


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%% end of local_add_defaults %%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% (sub)function LOCAL_READ_VALUES

function visout = local_read_values(visin)

global h_lb_somfig_ctrl;
global h_lb_use_comps;
global h_et_onfigure_traj;
global h_et_onfigure_hits;
global h_et_onfigure_labels;
global h_et_camera;

visout 			= visin;

visout.traj_data 	= get(h_et_onfigure_traj,'String');
visout.hits_data 	= get(h_et_onfigure_hits,'String');
visout.labels_data 	= get(h_et_onfigure_labels,'String');
visout.use_comps 	= get(h_lb_use_comps,'Value') - 1;
visout.comps		= visout.use_comps;
% FEATURE: use_comps: if the first component is 0, it
%   means U-matrix. Shifting with -1 we take U-matrix with.

% FUTURE: 
% visout.M

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%% end of local_read_values %%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% (sub)function LOCAL_SET_VALUES
%%    Called after a 'context switch', a new SOMfig will be loaded
%%  into somui_vis GUI.

function visout = local_set_values(sfin)

global h_lb_somfig_ctrl;
global h_lb_use_comps;
global h_et_onfigure_traj;
global h_et_onfigure_hits;
global h_et_onfigure_labels;
global h_st_active_map;
global h_st_active_data;
global h_st_active_somfig;

global h_cb_onfigure_traj;
global h_cb_onfigure_hits;
global h_cb_onfigure_labels;
global h_cb_show_ti;
global h_cb_show_ax;
global h_cb_show_gui;
global h_cb_show_cn;
global h_pop_show_cb;
global h_cb_draw_new;

visout = sfin;

set(h_et_onfigure_traj,		'String', visout.traj_data);
set(h_et_onfigure_hits,		'String', visout.hits_data);
set(h_et_onfigure_labels,	'String', visout.labels_data);  

if (isfield(visout,'map'))
  tmp = visout.map.comp_names;
  if ~isempty(tmp)
    tmpstring = cell(size(tmp) + [1 0]);
    tmpstring{1} = 'U-Matrix';
    tmpstring(2:end, :) = visout.map.comp_names;
  end;
else
  tmpstring = '';
end;
  
set(h_lb_use_comps,		'String', tmpstring);
set(h_lb_use_comps,		'Value', visout.use_comps + 1); 
	% shifting with +1. use_comps is of size(map.comp_names) + 1.
	% The first component is, if zero, means U-Matrix.
	% Matlab's vector indexing begins from 1, that's why shifting.

set(h_cb_draw_new,		'Value', visout.sf_params.draw_new);
set(h_cb_onfigure_traj,		'Value', visout.sf_params.traj);
set(h_cb_onfigure_hits,		'Value', visout.sf_params.hits);
set(h_cb_onfigure_labels,	'Value', visout.sf_params.labels);
set(h_cb_show_ti,		'Value', visout.sf_params.show_ti);
set(h_cb_show_ax,		'Value', visout.sf_params.show_ax);
set(h_cb_show_gui,		'Value', visout.sf_params.show_gui);
set(h_cb_show_cn,		'Value', visout.sf_params.show_cn);
set(h_pop_show_cb,		'Value', visout.sf_params.show_cb);

drawnow;				% if necessary?


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%% end of local_set_values %%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% (sub)function LOCAL_CREATE_SF

function sfout = local_create_sf(visin)

sfout = visin;
sfout.issfstruct = 1;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%% end of local_create_sf %%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%





