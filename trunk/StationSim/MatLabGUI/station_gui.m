function varargout = station_gui(varargin)
% STATION_GUI Application M-file for station_gui.fig
%    FIG = STATION_GUI launch station_gui GUI.
%    STATION_GUI('callback_name', ...) invoke the named callback.

% Last Modified by GUIDE v2.0 25-Jul-2002 14:49:41

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


% The sub-band splitter and the channel splitter don't differ in functionality
% Since they both use the same code, both can use the same GUI.
function varargout = SubBandButton_Callback(h, eventdata, handles, varargin)
    subband_gui;
%   g = findobj('Tag','SubBandEdit');
%   text=['#Subbands: ',num2str(NumberSubBands)];
%   set(g, 'String', text);


function varargout = ChannelButton_Callback(h, eventdata, handles, varargin)
    channel_gui; 


% --------------------------------------------------------------------
function varargout = BeamFormerButton_Callback(h, eventdata, handles, varargin)
    beamformerGUI ;
    

% --------------------------------------------------------------------
function varargout = SignalButton_Callback(h, eventdata, handles, varargin)
  % open new window containing options for the input signal
  % including which antennae to use, signal source and noise level
  % signal_fig = openfig('signal_gui.fig','reuse');
  AntennaSignals = signal_gui;


% --------------------------------------------------------------------
function varargout = OutputButton_Callback(h, eventdata, handles, varargin)
    output_gui;



% --------------------------------------------------------------------
function varargout = AddWeightButton_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = File_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Print_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = Close_Callback(h, eventdata, handles, varargin)
  % close all gui windows
  close all

% --------------------------------------------------------------------
function varargout = RunButton_Callback(h, eventdata, handles, varargin)
    % generate pattern
    dirpath='data';
    load([dirpath '\signal_options.mat']);
    load([dirpath '\output_options.mat']);
    
    fprintf('Generating pattern\n');
    tic;
    if (beam_side | beam_top | beam_contour | beam_3d)
        genepattern;
        fprintf('Plotting pattern\n');
        % plot pattern
        plotpattern;
    end;
    
    if (get(findobj('Tag', 'SubbandEnableCheck'),'Value'))
        fprintf('Subband splitter\n');
        SubbandSplitter;
        load([dirpath '\subband_options.mat'])
        if (RFIblanking & rfi_mit_power)
            fprintf('\tPlotting RFI blanking results\n');
            load([dirpath '\antenna_signals']);
            RFImitResults(AntennaSignals,SelectedSubBands,DFTSystemResponse,NumberSubBands,SubbandFilterLength, ...
                SelectedSubBandSignals, NumberOfAntennas, size(SelectedSubBandSignals),FlaggingCube,sb_quant_signal,...
                sb_quant_inputfft,sb_quant_outputfft); 
        end
    end

    % Calculating the eigen system on a large (nulled) grid may fill the memory to overflow
    if (get(findobj('Tag','BFenableCheck'),'Value'));
        fprintf('Generating Eigen system\n');
        eigensystem;    
        fprintf('Plotting spectrum\n');
        plotspectrum;


        % Check if we need to use the sub-band splitter
        % Keeping the used files up to date is up to the user.

        fprintf('Applying weight vector to subbands\n');
        
        load([dirpath '\antenna_signals.mat']);
        load([dirpath '\rfi_eigen.mat']);
        load([dirpath '\bf_options.mat']);
        clear('Evector','Evalue');
 
        % initialize the Beamformed signals according to selected beamformer and whether or not the 
        % sub band splitter is enabled. 
        if (BFstrat==1) 
            if (get(findobj('Tag', 'SubbandEnableCheck'),'Value'))
                BFSignals=zeros(size(SelectedSubBandSignals,1),size(SelectedSubBandSignals,2));
            else
                BFSignals=zeros(size(AntennaSignals,1),size(AntennaSignals,2));
            end
        elseif (BFstrat==2)
            if (get(findobj('Tag', 'SubbandEnableCheck'),'Value'))
                % FFT beamformer and sub band splitter doesn't work very well
                BFSignals=zeros(size(arrayadaptation(SelectedSubBandSignals(1,1,1))));
            else
                BFSignals=zeros(size(arrayadaptation(AntennaSignals(1,1,1)),1));
            end
        end
        
        if (get(findobj('Tag', 'SubbandEnableCheck'),'Value'))
            % TODO : Use a real algorithm for selecting a subband.
            Subband=size(SelectedSubBandSignals,3)/2;   
                % now we apply the weightvector to each subband.
            if (BFstrat == 1)
                for i=1:size(SelectedSubBandSignals,2)
                    BFSignals(:,i)=SelectedSubBandSignals(:,i,Subband) .* WeightVector;
                end
            elseif (BFstrat == 2)
                BFSignals = arrayadaptation(SelectedSubBandSignals);
            end
            fprintf('Done\n');
            save([dirpath '\antenna_signals.mat'],'AntennaSignals','SelectedSubBandSignals','BFSignals');
            %clear('AntennaSignals','SelectedSubBandSignals','BeamPattern','BFSignals');
        else
            if (BFstrat == 1)
                for i=1:size(AntennaSignals,2)
                    BFSignals(:,i)=AntennaSignals(:,i) .* WeightVector;
                end
            elseif (BFstrat == 2)
                BFSignals=arrayadaptation(AntennaSignals);
            end
            save([dirpath '\antenna_signals.mat'],'AntennaSignals','BFSignals');
            %clear('AntennaSignals','BeamPattern','BFSignals');
        end
        
        if (bf_power | bf_3dplot | bf_side)
            fprintf('Plotting beamformer results\n');
            plotBFfigures;
        end 
        
        if (ad_beam_top | ad_beam_side | ad_beam_contour | ad_beam_3d)
            % plot pattern without RFI, one beam (no subband splitter).
            fprintf('Generating pattern with rfi nulls\n');
            genebeamnulls;
            fprintf('Plotting new pattern\n');    
            plotpatternrfi;
        end; 
    end;
    if (get(findobj('Tag', 'ChannelEnableCheck'),'Value'))
        fprintf('Channel splitter\n');
        ChannelSplitter(get(findobj('Tag','BFenableCheck'),'Value'));
        load([dirpath '\channel_options.mat'])
        if (CH_RFIblanking & rfi_mit_power)
            fprintf('\tPlotting RFI blanking results\n');
            load([dirpath '\antenna_signals']);
            RFImitResults(BFSignals,SelectedChannels,DFTSystemResponse,NumberChannels,ChannelFilterLength, ...
                SelectedChannelSignals, NumberOfAntennas, size(SelectedChannelSignals),FlaggingCube,ch_quant_signal,ch_quant_inputfft,...
                ch_quant_outputfft); 
        end
    end
    toc;
end
    
% --------------------------------------------------------------------
function varargout = SubbandEnableCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = ChannelEnableCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = BFenableCheck_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = DFTtestButton_Callback(h, eventdata, handles, varargin)
    DFTFilterBankTest;
