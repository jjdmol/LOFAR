function varargout = bf_demo(varargin)
% BF_DEMO Application M-file for bf_demo.fig
%    FIG = BF_DEMO launch bf_demo GUI.
%    BF_DEMO('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 27-Sep-2002 11:30:52

datapath='./data';

if (exist(datapath) == 0)
    mkdir(datapath);
end


if nargin == 0  % LAUNCH GUI

	fig = openfig(mfilename,'reuse');

	% Use system color scheme for figure:
	set(fig,'Color',get(0,'defaultUicontrolBackgroundColor'));

	% Generate a structure of handles to pass to callbacks, and store it. 
	handles = guihandles(fig);
	guidata(fig, handles);
    
    status=0;
    save('data/demo_status.mat','status');

    set(findobj(fig,'Tag','StatusBox'),'String','Initialized. Start experiment by plotting a beam pattern.');
    
	if nargout > 0
		varargout{1} = fig;
	end

elseif ischar(varargin{1}) % INVOKE NAMED SUBFUNCTION OR CALLBACK

	try
		if (nargout)
			[varargout{1:nargout}] = feval(varargin{:}); % FEVAL switchyard
		else
			feval(varargin{:}); % FEVAL switchyard
		end
	catch
		disp(lasterr);
	end

end


%| ABOUT CALLBACKS:
%| GUIDE automatically appends subfunction prototypes to this file, and 
%| sets objects' callback properties to call them through the FEVAL 
%| switchyard above. This comment describes that mechanism.
%|
%| Each callback subfunction declaration has the following form:
%| <SUBFUNCTION_NAME>(H, EVENTDATA, HANDLES, VARARGIN)
%|
%| The subfunction name is composed using the object's Tag and the 
%| callback type separated by '_', e.g. 'slider2_Callback',
%| 'figure1_CloseRequestFcn', 'axis1_ButtondownFcn'.
%|
%| H is the callback object's handle (obtained using GCBO).
%|
%| EVENTDATA is empty, but reserved for future use.
%|
%| HANDLES is a structure containing handles of components in GUI using
%| tags as fieldnames, e.g. handles.figure1, handles.slider2. This
%| structure is created at GUI startup using GUIHANDLES and stored in
%| the figure's application data using GUIDATA. A copy of the structure
%| is passed to each callback.  You can store additional information in
%| this structure at GUI startup, and you can change the structure
%| during callbacks.  Call guidata(h, handles) after changing your
%| copy to replace the stored original so that subsequent callbacks see
%| the updates. Type "help guihandles" and "help guidata" for more
%| information.
%|
%| VARARGIN contains any extra arguments you have passed to the
%| callback. Specify the extra arguments by editing the callback
%| property in the inspector. By default, GUIDE sets the property to:
%| <MFILENAME>('<SUBFUNCTION_NAME>', gcbo, [], guidata(gcbo))
%| Add any extra arguments after the last argument, before the final
%| closing parenthesis.



% --------------------------------------------------------------------
function varargout = ArraySelector_Callback(h, eventdata, handles, varargin)
    array_type = get(h, 'Value');
    % get parameters for selected array type
    params = arrayparams(array_type);
    [x,y] = arraybuilder(array_type, params);
    NumberOfAntennas = length(y);
    f = findobj('Tag','NumberElements');
    set(f,'String',num2str(NumberOfAntennas));
 

% --------------------------------------------------------------------
function varargout = PlotArrayButton_Callback(h, eventdata, handles, varargin)
    g = findobj('Tag','ArraySelector');
    array_type = get(g, 'Value');

    % get parameters for selected array type
    params = arrayparams(array_type);
    [px,py] = arraybuilder(array_type, params);

    figure;
    plot(px,py,'s','markersize',2,'markerfacecolor',[0 0 1]),grid
    xlabel('x-location (\lambda)'), ylabel('y-location (\lambda)')
    title('Array Configuration');
    axis equal
    axis square 


% --------------------------------------------------------------------
function varargout = NumberElements_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ShowNormalShape_Callback(h, eventdata, handles, varargin)
    g = findobj('Tag','ArraySelector');
    array_type = get(g, 'Value');

    % get parameters for selected array type
    params = arrayparams(array_type);
    [px,py] = arraybuilder(array_type, params);
    
    NullGrid=0;
        
    % save the configuration, so we can use genepattern and plotpattern
    save('data/antenna_config.mat' ,'px','py','NullGrid');
    
    % dummy to create an 'empty' file
    dummy=0;
    save('data/antenna_signals.mat','dummy');
 
    % save the output options
    beam_top = 1;
    beam_3d  = 1;
    ad_beam_3d = 1;
    ad_beam_top = 1;

    beam_side=0;
    beam_contour=0;
    bf_power=0;
    bf_side=0;
    bf_3dplot=0;
    bf_diffplot=0;
    ad_beam_side=0;
    ad_beam_contour=0;
    rfi_mit_power=0;
    signal_eigen_sv=0;
    signal_eigen_acm=0;
    signal_spectrum=0;
   
    save('data/output_options.mat','beam_side','beam_top','beam_contour','beam_3d',...
        'signal_eigen_sv','signal_eigen_acm','signal_spectrum','bf_power','bf_3dplot','bf_side','bf_diffplot',...
        'ad_beam_side','ad_beam_top','ad_beam_contour','ad_beam_3d','rfi_mit_power');
    
    % now use the station_gui elements to plot the beam.
    genepattern;
    plotpattern_demo;

    status=1;
    
    save('data/demo_status.mat','status');
    set(findobj('Tag','StatusBox'),'String','Array beam pattern plotted. Now select a looking direction and plot the beam pattern when steered in that direction.');
    

% --------------------------------------------------------------------
function varargout = PointPhi_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = PointTheta_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ShowSteeredShape_Callback(h, eventdata, handles, varargin)

    if(exist('data/demo_status.mat'))
        load('data/demo_status.mat')
        if (status > 0)

            load('data/antenna_config.mat');
        
            look_dir_phi    = str2num(get(findobj('Tag','PointPhi'), 'String'));
            look_dir_theta  = -str2num(get(findobj('Tag','PointTheta'), 'String'));
            rfi_phi=look_dir_phi;
            rfi_theta=look_dir_theta;
            rfi_number=1;
    
            WeightVector=steerv(px,py,look_dir_phi, look_dir_theta);
            LookingDirection=WeightVector;
    
            save('data/signal_options.mat','rfi_phi','rfi_theta','rfi_number','look_dir_phi','look_dir_theta');
            save('data/rfi_eigen.mat','WeightVector','LookingDirection');
            genebeamnulls;
            plotpatternrfi_demo(1);
            
            status = 2;
            save('data/demo_status.mat','status');
            set(findobj('Tag','StatusBox'),'String','Steered beam pattern plotted. Introduce some RFI sources and plot the adapted beam pattern to finish the experiment.');
        else
            set(findobj('Tag','SteerWarn'),'String','First plot the normal beam');
        end
    end
    

% --------------------------------------------------------------------
function varargout = ShowAdaptedShape_Callback(h, eventdata, handles, varargin)

    if (exist('data/demo_status.mat'))
        load('data/demo_status.mat')
    end
    if (status > 1)

        load('data/antenna_config.mat');

        BFstrat=1;
        rfi_strat=3;
        rfi_value=-1;
    
        save('data/bf_options.mat','BFstrat','rfi_strat','rfi_value');
    
        look_dir_phi    = str2num(get(findobj('Tag','PointPhi'), 'String'));
        look_dir_theta  = -str2num(get(findobj('Tag','PointTheta'), 'String'));
    
        rfi_phi         = str2num(get(findobj('Tag','RFIphi'), 'String'));
        rfi_theta       = -str2num(get(findobj('Tag','RFItheta'), 'String'));
    
        if (length(rfi_phi) == length(rfi_theta))
        
            set(findobj('Tag','NumberSources'),'String',length(rfi_theta));
            rfi_number=length(rfi_phi);
    
            LookingDirection=steerv(px,py,look_dir_phi, look_dir_theta);
            NumberOfAntennas=length(py);
            snapshot_number=100;
            signal_freq=0.9 ;
            rfi_ampl=ones(rfi_number)*2;
            rfi_freq=ones(rfi_number);
        
            AntennaSignals = GenerateData(NumberOfAntennas, snapshot_number, rfi_number, look_dir_phi,look_dir_theta, ...
                signal_freq, rfi_phi, rfi_theta, px, py, rfi_ampl, rfi_freq);

            save('data/antenna_signals.mat','AntennaSignals'); 
            save('data/signal_options.mat','rfi_phi','rfi_theta','rfi_number','look_dir_phi','look_dir_theta','snapshot_number', ...
                'NumberOfAntennas');
            save('data/rfi_eigen.mat','LookingDirection');
            eigensystem;
            genebeamnulls;
            plotpatternrfi_demo(2);
            
            set(findobj('Tag','StatusBox'),'String','Experiment concluded.');

            
            % experiment is complete. Remove stale files.
            %delete('data/*.mat');
        else
            set(findobj('Tag','RFIwarn'),'String','!');
        
        end
    else
        set(findobj('Tag','RFIwarn'),'String','!!!');
    end
   

% --------------------------------------------------------------------
function varargout = RFIphi_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = RFItheta_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = NumberSources_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = RandomButton_Callback(h, eventdata, handles, varargin)

    rfi_number=str2num(get(findobj('Tag','NumberSources'),'String'));
    
    phiRFI = pi * (rand(1,rfi_number) - 0.5);
    thetaRFI = pi * (rand(1,rfi_number) - 0.5);
 
    g = findobj('Tag','NumberSources');
    i = str2num(get(g, 'String'));
    
    ph = pi * (rand(1,i) - 0.5);
    th = pi * (rand(1,i) - 0.5);

    i = findobj('Tag','RFIphi');
    set(i,'String',mat2str(ph,2));
    i = findobj('Tag','RFItheta');
    set(i,'String',mat2str(th,2));



% --------------------------------------------------------------------
function varargout = StatusBox_Callback(h, eventdata, handles, varargin)

