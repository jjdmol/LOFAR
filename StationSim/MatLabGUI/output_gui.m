function varargout = output_gui(varargin)
% OUTPUT_GUI Application M-file for output_gui.fig
%    FIG = OUTPUT_GUI launch output_gui GUI.
%    OUTPUT_GUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 19-Mar-2003 11:59:17

if nargin == 0  % LAUNCH GUI

	fig = openfig(mfilename,'reuse');


	% Generate a structure of handles to pass to callbacks, and store it. 
	handles = guihandles(fig);
	guidata(fig, handles);

	if nargout > 0
		varargout{1} = fig;
	end
    
    %restore the previous options from file
    restore_options(fig);
    
    % Wait for callbacks to run and window to be dismissed:
    uiwait(fig);

    % save all ouput options to config file
    dirpath = 'data';
    
    beam_side   = get(findobj(fig,'Tag','BeamSideCheck'), 'Value');
    beam_top    = get(findobj(fig,'Tag','BeamTopCheck'), 'Value');
    beam_contour= get(findobj(fig,'Tag','BeamContourCheck'), 'Value');
    beam_3d     = get(findobj(fig,'Tag','Beam3dCheck'), 'Value');
 
    ad_beam_side   = get(findobj(fig,'Tag','AdBeamSideCheck'), 'Value');
    ad_beam_top    = get(findobj(fig,'Tag','AdBeamTopCheck'), 'Value');
    ad_beam_contour= get(findobj(fig,'Tag','AdBeamContourCheck'), 'Value');
    ad_beam_3d     = get(findobj(fig,'Tag','AdBeam3dCheck'), 'Value');
        
    signal_eigen_sv = get(findobj(fig,'Tag','SignalEigenSteeringCheck'),'Value');
    signal_eigen_acm= get(findobj(fig,'Tag','SignalEigenACMCheck'),'Value');
    Polyphase_spectrum = get(findobj(fig,'Tag','SignalSpectrumCheck'),'Value');
    OutputSignal = get(findobj(fig,'Tag','Output_spectrum'),'Value');
    
    
    
    bf_power = get(findobj(fig,'Tag','BFpowerCheck'),'Value');
    bf_3dplot = get(findobj(fig,'Tag','BF3dCheck'),'Value');
    bf_side  = get(findobj(fig,'Tag','BFsideCheck'),'Value');
    bf_diffplot = get(findobj(fig,'Tag','DiffPlotCheck'),'Value');
    
    rfi_mit_power = get(findobj(fig,'Tag','RFImitPowerCheck'),'Value');
    
    save([dirpath '/output_options.mat'],'beam_side','beam_top','beam_contour','beam_3d',...
        'signal_eigen_sv','signal_eigen_acm','Polyphase_spectrum','bf_power','bf_3dplot','bf_side','bf_diffplot',...
        'ad_beam_side','ad_beam_top','ad_beam_contour','ad_beam_3d','rfi_mit_power','OutputSignal');
    h=get(findobj('Tag','StationSimGUI'));
    set(findobj(h.Children,'tag','OutputButton'),'BackgroundColor',[0.11 0.36 0.59]);
    % file saved, close the gui
    handles = guidata(fig);
    delete(fig);    
    
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

function restore_options(fig)
    dirpath = 'data';
    if exist([dirpath '/output_options.mat']);
        load([dirpath '/output_options.mat']);
        set(findobj(fig,'Tag','Beam3dCheck'), 'Value', beam_3d);

        set(findobj(fig,'Tag','AdBeamSideCheck'), 'Value',ad_beam_side);
        set(findobj(fig,'Tag','AdBeamTopCheck'), 'Value', ad_beam_top);
        set(findobj(fig,'Tag','AdBeamContourCheck'), 'Value', ad_beam_contour);
        set(findobj(fig,'Tag','AdBeam3dCheck'), 'Value', ad_beam_3d);

        set(findobj(fig,'Tag','SignalEigenSteeringCheck'),'Value',signal_eigen_sv);
        set(findobj(fig,'Tag','SignalEigenACMCheck'),'Value',signal_eigen_acm);
        set(findobj(fig,'Tag','SignalSpectrumCheck'),'Value',Polyphase_spectrum);
        set(findobj(fig,'Tag','Output_spectrum'),'Value',OutputSignal);

        set(findobj(fig,'Tag','BFpowerCheck'),'Value',bf_power);
        set(findobj(fig,'Tag','BF3dCheck'),'Value',bf_3dplot);
        set(findobj(fig,'Tag','BFsideCheck'),'Value',bf_side);
        set(findobj(fig,'Tag','DiffPlotCheck'),'Value',bf_diffplot);
  
        set(findobj(fig,'Tag','RFImitPowerCheck'),'Value',rfi_mit_power);    
    end;    
    
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
function varargout = BeamSideCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = BeamTopCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Beam3dCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = SignalEigenSteeringCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = BeamContourCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = SignalEigenACMCheck_Callback(h, eventdata, handles, varargin)


% --------------------------------------------------------------------
function varargout = SignalSpectrumCheck_Callback(h, eventdata, handles, varargin)


% --------------------------------------------------------------------
function varargout = ChannelCheck_Callback(h, eventdata, handles, varargin)



% --------------------------------------------------------------------
function varargout = OkButton_Callback(h, eventdata, handles, varargin)
   uiresume; 

   
% --------------------------------------------------------------------
function varargout = AdBeamSideCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = AdBeamTopCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = AdBeamContourCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = AdBeam3dCheck_Callback(h, eventdata, handles, varargin)

% --------------------------------------------------------------------
function varargout = BFpowerCheck_Callback(h, eventdata, handles, varargin)


% --------------------------------------------------------------------
function varargout = BF3dCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = BFsideCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = BFavgCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = RFImitPowerCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = DiffPlotCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Output_spectrum_Callback(h, eventdata, handles, varargin)

