function varargout = subband_gui(varargin)
% SUBBAND_GUI Application M-file for subband_gui.fig
%    FIG = SUBBAND_GUI launch subband_gui GUI.
%    SUBBAND_GUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 18-Jul-2002 11:15:58

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
    NumberSubBands = str2num(get(findobj(fig, 'Tag', 'SubBandEdit'), 'String'));
    SelectedSubBands = [1:NumberSubBands];
    
    g = findobj(fig, 'Tag','SelSubBandEdit');
    set(g, 'String', mat2str(SelectedSubBands));
    
    
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
    
        NumberSubBands = str2num(get(findobj(fig, 'Tag','SubBandEdit'),'String'));
        SelectedSubBands = str2num(get(findobj(fig, 'Tag','SelSubBandEdit'),'String'));
        SubbandFilterLength = str2num(get(findobj(fig, 'Tag','SbFilterLengthEdit'),'String'));
        
        sb_quant_signal            = str2num(get(findobj(fig,'Tag', 'QuantSignalEdit'), 'String'));
        sb_quant_inputfft          = str2num(get(findobj(fig,'Tag', 'QuantInputEdit'), 'String'));
        sb_quant_outputfft         = str2num(get(findobj(fig,'Tag', 'QuantOutputEdit'), 'String'));
        
        TFAavg                  = str2num(get(findobj(fig,'Tag', 'TFAavgEdit'), 'String'));
        TFAfreq                 = str2num(get(findobj(fig,'Tag', 'TFAfreqEdit'), 'String'));
        
        RFIblanking = get(findobj(fig, 'Tag','RFIblankingCheck'),'Value');
       
        % save the parameters to file
        save([dirpath '\subband_options.mat'], 'NumberSubBands','SelectedSubBands','SubbandFilterLength', ...
            'sb_quant_signal','sb_quant_inputfft','sb_quant_outputfft','RFIblanking','TFAavg','TFAfreq'); 
        
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
    g = findobj('Tag','SubBandEdit');
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




% --------------------------------------------------------------------
function varargout = FillButton_Callback(h, eventdata, handles, varargin)
    NumberSubBands = str2num(get(findobj('Tag','SubBandEdit'),'String'));
    SelectedSubBands = [1:NumberSubBands];
    
    g = findobj(fig, 'Tag','SelSubBandEdit');
    set(g, 'String', mat2str(SelectedSubBands));
    

% --------------------------------------------------------------------
function varargout = EvenlyRadio_Callback(h, eventdata, handles, varargin)
    if (get(h, 'Value'))
        set(findobj('Tag','RandomRadio'),'Value',0);
    else
        set(findobj('Tag','EvenlyRadio'),'Value',1);
    end

% --------------------------------------------------------------------
function varargout = RandomRadio_Callback(h, eventdata, handles, varargin)
    if (get(h, 'Value'))
        set(findobj('Tag','EvenlyRadio'),'Value',0);
    else
        set(findobj('Tag','RandomRadio'),'Value',1);
    end
