
function varargout = signal_gui(varargin)
% SIGNAL_GUI Application M-file for signal_gui.fig
%    FIG = SIGNAL_GUI launch signal_gui GUI.
%    SIGNAL_GUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 18-Jul-2002 15:38:29
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

    % TODO: restore signal options from previous experiment.
    % restore_options(fig)
    
    % Wait for callbacks to run and window to be dismissed:
    uiwait(fig);

    % UIWAIT might have returned because the window was deleted using
    % the close box - in that case, return 'cancel' as the answer, and
    % don't bother deleting the window! 
    if ~ishandle(fig)
	    answer = 'cancel';
    else
        if (get(findobj(fig,'Tag','SynthesizedButton'),'Value'))
            % Synthesize data
            
            % NumberOfAntennas = str2num(get(findobj(fig,'Tag','AntennaEdit'),'String'));
            % will be set according to array configuration
            
            sinus=get(findobj(fig,'Tag','SinusRadio'),'Value');
            
            dirpath='data';
            signal_type='synthesized';
          
            snapshot_number         = str2num(get(findobj(fig,'Tag','DataLengthEdit'), 'String'));
            look_dir_phi            = str2num(get(findobj(fig,'Tag','PhiEdit'), 'String'));
            look_dir_theta          = str2num(get(findobj(fig,'Tag','ThetaEdit'), 'String'));
            signal_freq             = str2num(get(findobj(fig,'Tag','SourceFreqEdit'),'String'));
 
            rfi_number              = str2num(get(findobj(fig,'Tag','NumberRFIEdit'), 'String'));
            rfi_phi                 = str2num(get(findobj(fig,'Tag','phiRFIEdit'), 'String'));
            rfi_theta               = str2num(get(findobj(fig,'Tag','thetaRFIEdit'),'String'));
            rfi_ampl                = str2num(get(findobj(fig,'Tag','RFIAmplEdit'),'String'));
            rfi_freq                = str2num(get(findobj(fig,'Tag','RFIfreqEdit'),'String'));
            
            if (size(rfi_phi) ~= rfi_number)
                [rfi_phi,rfi_theta] = randomRFI_main(fig);
            end;
                       
            g = findobj(fig,'Tag','ArrayTypeList');
            array_type = get(g, 'Value');

            % get parameters for selected array type
            params = arrayparams(array_type);
            [px,py] = arraybuilder(array_type, params);

            %
            % Snap to grid if requested to
            %
            null_mask=1;
            GridSize = str2num(get(findobj(fig, 'Tag', 'GridEdit'),'String'));
            SnapToGrid = get(findobj(fig, 'Tag', 'SnapToCheck'),'Value');
            NullGrid = get(findobj(fig, 'Tag', 'NullGridCheck'),'Value');

            xg = (max(px) - min(px)) / (GridSize-1);
            yg = (max(py) - min(py)) / (GridSize-1);
           
            if (SnapToGrid)
                xg=1; % default
                yg=1;
                [px,py,xg,yg,null_mask] = Fitgrid(px,py,GridSize,NullGrid);
            end

            % Only now can we check the number of antennas (could be changed by NullGrid)
            NumberOfAntennas = length(py);
 
            % currently, the RFI signals are randomized as 0 < rfi < rfi_max where rfi_max can be specified
            % Should be more flexible.. TODO
            FreqRFIbase=rfi_freq;
            AmplRFIbase=rfi_ampl;
            
            rfi_freq = FreqRFIbase * rand(1,rfi_number);
            rfi_ampl = AmplRFIbase * rand(1,rfi_number);

            if ~sinus
                AntennaSignals = GenerateData(NumberOfAntennas, snapshot_number, rfi_number, look_dir_phi,look_dir_theta, ...
                    signal_freq, rfi_phi, rfi_theta, px, py, rfi_ampl, rfi_freq);
            else
                % generate a sinus signal for each antenna
                AntennaSignals=zeros(NumberOfAntennas,snapshot_number);
                sinus_type=get(findobj(fig,'Tag','SinusTypePop'),'Value');
                
                fprintf('Selected Sinus type is %d\n',sinus_type);
                
                sig_freq=signal_freq;
                for i=1:NumberOfAntennas
                    if (sinus_type==2)
                        sig_freq=signal_freq/i;
                    elseif (sinus_type==3)
                        sig_freq=signal_freq*rand;
                    end
                    AntennaSignals(i,:)=1*sin(2*pi*[1:1:snapshot_number]/sig_freq);
                end

            end
            
            if NullGrid
                % We have generated a lot of information for 'virtual' antennas
                % remove these by multiplying with null_mask
                for i = 1:snapshot_number
                    AntennaSignals(:,i) = AntennaSignals(:,i) .* (null_mask.');
                end;
            end;

            
        elseif (get(findobj(fig,'Tag','FileButton'),'Value'))
            % Read data from file
	    % Implement me 
        end
        
        % save the signal data so we can use it without having to assign them to the output
        % not used : signal_ampl
        dirpath = 'data';
        save([dirpath '\signal_options.mat'],'look_dir_phi','look_dir_theta','signal_freq','signal_type','array_type', ...
            'rfi_number','rfi_phi','rfi_theta','rfi_freq','rfi_ampl','snapshot_number','NumberOfAntennas','SnapToGrid','NullGrid');            
        save([dirpath '\antenna_signals.mat'],'AntennaSignals');
        save([dirpath '\antenna_config.mat'],'array_type','px','py','GridSize','SnapToGrid','null_mask','NullGrid','xg','yg');
        
        % so, we need to delete the window.   
        handles = guidata(fig);
        delete(fig);    
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

function [phiRFI,thetaRFI] = randomRFI_main(f)
    g = findobj(f,'Tag','NumberRFIEdit');
    i = str2num(get(g, 'String'));
    phiRFI = pi * rand(1,i);
    thetaRFI = pi * rand(1,i);


function [phiRFI,thetaRFI] = randomRFI
    g = findobj('Tag','NumberRFIEdit');
    i = str2num(get(g, 'String'));
    phiRFI = pi * rand(1,i);
    thetaRFI = pi * rand(1,i);


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
function varargout = radiobutton7_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = radiobutton8_Callback(h, eventdata, handles, varargin)
% Synthesized data radio button
    handles.synth = get(h, 'Value');
    if (handles.synth == 1)
        h = findobj('Tag','FileButton');
        set(h, 'Value',0);
    end;
    guidata(h, handles);


% --------------------------------------------------------------------
function varargout = radiobutton9_Callback(h, eventdata, handles, varargin)
% Data from file button
    handles.file = get(h, 'Value');
    if (handles.file == 1)
        h = findobj('Tag','SynthesizedButton');
        set (h, 'Value', 0);
    end;
    guidata(h, handles);


% --------------------------------------------------------------------
function varargout = AntennaEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = SOIAmpEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = SysNoiseEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = RFIAmplEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ChirpAmplEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ChirpFreqEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ChirpPhaseEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = DataLengthEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = OkButton_Callback(h, eventdata, handles, varargin)
    guidata(h, handles);
    uiresume;

% --------------------------------------------------------------------
function varargout = ThetaEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = NumberRFIEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = PhiEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = RFIfreqEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = edit16_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = edit17_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = RandomRFIButton_Callback(h, eventdata, handles, varargin)
    g = findobj('Tag','NumberRFIEdit');
    i = str2num(get(g, 'String'));
    ph = pi * rand(1,i);
    th = pi * rand(1,i);
    i = findobj('Tag','phiRFIEdit');
    set(i,'String',mat2str(ph,2));
    i = findobj('Tag','thetaRFIEdit');
    set(i,'String',mat2str(th,2));

% --------------------------------------------------------------------
function varargout = SourceFreqEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ArrayTypeList_Callback(h, eventdata, handles, varargin)
    g = findobj('Tag','ArrayTypeList');
    array_type = get(g, 'Value');
    % get parameters for selected array type
    params = arrayparams(array_type);
    [x,y] = arraybuilder(array_type, params);
    NumberOfAntennas = length(y);
    f = findobj('Tag','AntennaEdit');
    set(f,'String',num2str(NumberOfAntennas));


% --------------------------------------------------------------------
function varargout = PlotArrayButton_Callback(h, eventdata, handles, varargin)
    g = findobj('Tag','ArrayTypeList');
    array_type = get(g, 'Value');

    % get parameters for selected array type
    params = arrayparams(array_type);
    [x,y] = arraybuilder(array_type, params);

    %
    % Snap to grid if requested to
    %
    SnapToGrid = get(findobj('Tag', 'SnapToCheck'),'Value');
    NullGrid= get(findobj('Tag','NullGridCheck'),'Value');
    null=ones(1,size(x,2));
    if (SnapToGrid)
        old_x=x;
        old_y=y;
        GridSize = str2num(get(findobj('Tag', 'GridEdit'),'String'));
        xg=1; % default
        yg=1;
        [x,y,xg,yg,null] = Fitgrid(x,y,GridSize,NullGrid);
    end
    
    % visualize the array without the nulled antennas
    x = x .* null;
    y = y .* null;
    
    
    figure(2);
    plot(x,y,'s','markersize',2,'markerfacecolor',[0 0 1]),grid
    if (SnapToGrid)
        hold on;
        plot(old_x,old_y,'s','markersize',2,'markerfacecolor','y','markeredgecolor','r')
        hold off
    else
       
    end
    xlabel('x-location (\lambda)'), ylabel('y-location (\lambda)')

    if NullGrid
        title('Array Configuration - grid nulled');
    elseif SnapToGrid
        title('Array Configuration - snap to grid %dx%d',GridSize,GridSize);
    else
        title('Array Configuration');
    end;
    axis equal
    axis square 

% --------------------------------------------------------------------
function varargout = SnapToCheck_Callback(h, eventdata, handles, varargin)

if (get(h, 'Value'))
    set(findobj('Tag','NullGridCheck'),'Enable','on');
else
    set(findobj('Tag','NullGridCheck'),'Enable','off');    
end



% --------------------------------------------------------------------
function varargout = GridEdit_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = NullGridCheck_Callback(h, eventdata, handles, varargin)
        



% --------------------------------------------------------------------
function varargout = SinusRadio_Callback(h, eventdata, handles, varargin)
     if (get(h, 'Value'))
         set(findobj('Tag','SinusTypePop'),'Enable','on');
     else
         set(findobj('Tag','SinusTypePop'),'Enable','off');
     end



% --------------------------------------------------------------------
function varargout = SinusTypePop_Callback(h, eventdata, handles, varargin)

