function varargout = channel_gui(varargin)
% CHANNEL_GUI Application M-file for channel_gui.fig
%    FIG = CHANNEL_GUI launch channel_gui GUI.
%    CHANNEL_GUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 15-Jul-2002 13:41:36

if nargin == 0  % LAUNCH GUI

	fig = openfig(mfilename,'reuse');
    
	% Use system color scheme for figure:
	set(fig,'Color',get(0,'defaultUicontrolBackgroundColor'));

	% Generate a structure of handles to pass to callbacks, and store it. 
	handles = guihandles(fig);
	guidata(fig, handles);
    
	if nargout > 0
		varargout{1} = fig;
	end
    
    % set default value for selected sub band array
    NumberChannels = str2num(get(findobj(fig, 'Tag', 'ChannelEdit'), 'String'));
    SelectedChannels = [1:NumberChannels];
    
    g = findobj(fig, 'Tag','SelChannelEdit');
    set(g, 'String', mat2str(SelectedChannels));
    
    
    % Wait for callbacks to run and window to be dismissed:
    uiwait(fig);

    % UIWAIT might have returned because the window was deleted using
    % the close box - in that case, return 'cancel' as the answer, and
    % don't bother deleting the window! 
    if ~ishandle(fig)
	    answer = 'cancel';
    else
        dirpath = 'data';
        load([dirpath '\signal_options.mat']);
        load([dirpath '\antenna_signals.mat']);
    
        NumberChannels = str2num(get(findobj(fig,'Tag','ChannelEdit'),'String'));
        SelectedChannels = str2num(get(findobj(fig,'Tag','SelChannelEdit'),'String'));
        ChannelFilterLength = str2num(get(findobj(fig, 'Tag','SbFilterLengthEdit'),'String'));
        ch_quant_signal            = str2num(get(findobj(fig,'Tag', 'QuantSignalEdit'), 'String'));
        ch_quant_inputfft          = str2num(get(findobj(fig,'Tag', 'QuantInputEdit'), 'String'));
        ch_quant_outputfft         = str2num(get(findobj(fig,'Tag', 'QuantOutputEdit'), 'String'));

        TFAavg                  = str2num(get(findobj(fig,'Tag', 'TFAavgEdit'),'String'));
        TFAfreq                 = str2num(get(findobj(fig,'Tag', 'TFAfreqEdit'),'String'));
        
        CH_RFIblanking = get(findobj(fig, 'Tag','RFIblankingCheck'),'Value');
        
        % save the parameters to file
        save([dirpath '\channel_options.mat'], 'NumberChannels','SelectedChannels','ChannelFilterLength', ...
            'ch_quant_signal','ch_quant_inputfft','ch_quant_outputfft','CH_RFIblanking','TFAavg','TFAfreq');
        
        % so, we need to delete the window.
        handles = guidata(fig);
        delete(fig);
    end;
    
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
function varargout = edit1_Callback(h, eventdata, handles, varargin)
% number of sub bands




% --------------------------------------------------------------------
function varargout = edit2_Callback(h, eventdata, handles, varargin)
% array of selected sub bands
    g = findobj('Tag','ChannelEdit');
    set(g, 'String', size(str2num(get(h,'String')),2));
    


% --------------------------------------------------------------------
function varargout = OkButton_Callback(h, eventdata, handles, varargin)
    uiresume;
    


% --------------------------------------------------------------------
function varargout = RFIblankingCheck_Callback(h, eventdata, handles, varargin)
  if (get(h,'Value'))
     set(findobj('Tag','TFAavgEdit'),'Enable','on');
     set(findobj('Tag','TFAfreqEdit'),'Enable','on');     
  else
     set(findobj('Tag','TFAavgEdit'),'Enable','off');
     set(findobj('Tag','TFAfreqEdit'),'Enable','off');           
  end



% --------------------------------------------------------------------
function varargout = SbFilterLengthEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = QuantSignalEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = QuantInputEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = QuantOutputEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = TFAavgEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = TFAfreqEdit_Callback(h, eventdata, handles, varargin)

