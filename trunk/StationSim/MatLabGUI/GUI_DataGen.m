global signal_name;
global Fsample;
global Mean_h;
global Var_h;
global s;
global Size;
global S_freq
global count_sig;
global HPerioFigresu;
global HPerioFig;
global HPerioFilter;
global HPerioFigPlot;
global Filter_name;
global filter_box;
global Bandwidth;
global listbox_signal;
global Time_signal;
global Fresample;
global HMainFig_gene1;
global HPerioFilFig;
global res;
global indice;
global list;
global freq2;
global Time_signal 
res=[];
clear res;
clear ans;
clear list;
indice=0;
count_sig=0;
features=struct('Sampling_Freq_signal',{},'total_time',{});
s = struct('Signal_type',{},'Frequency',{},'Ampl',{},'Time',{},'signal_value',{},'TimeWidth',{},'OffsetTime',{},'phase',{},'FreqFin',{},'Type',{});
FrameWidth = 0.12; 
CompBottom=0.86;
CompWidth=0.05;
CompHeight=0.03;
LeftBorder = 0.05; 
BottomBorder=0.05; 
TotalWidth = 1 - 3*LeftBorder-FrameWidth ; TotalHeight = 1-2*BottomBorder;
FigSpace = 0.05;
Col1Width = 0.3*TotalWidth;
Col2Width = TotalWidth-Col1Width-FigSpace;
Lin1Height = 0.3*TotalHeight;
Lin2Height= TotalHeight-Lin1Height-FigSpace;
list=cell(50,1);

FrameBorder = 0.01;
LeftFrameBorder = LeftBorder + Col1Width+Col2Width+2*FigSpace;
FrameHeight = TotalHeight;
CompBackGroundColor = [0.5 0.5 0.5];

MainFigPos_gene = [ 5 35 1017 663];
shh = get(0,'ShowHiddenHandles');
set(0,'ShowHiddenHandles','on')
%Figure principale
HMainFig_gene1 = figure(...
   'Name','DataGenerator', ...
   'handlevisibility','callback',...
   'IntegerHandle','off',...
   'NumberTitle','off',...
   'Tag','MainFig',...
   'position',MainFigPos_gene);

 %=================================
 
frm1Pos=[LeftFrameBorder*0.008 BottomBorder+FrameHeight/2.5 FrameWidth*4.1 FrameHeight/1.665 ];
frm1=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm1Pos, ...
   'BackgroundColor',CompBackGroundColor );

frm2Pos=[LeftFrameBorder*0.606 16.75*BottomBorder FrameWidth*4.1 FrameHeight/8 ];
frm2=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm2Pos, ...
   'BackgroundColor',CompBackGroundColor );

frm6Pos=[LeftFrameBorder*0.615 17.1*BottomBorder FrameWidth*3.99 FrameHeight/12 ];
frm6=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm6Pos, ...
  'BackgroundColor',[0.6 0.5 0.5]);

frm3Pos=[LeftFrameBorder*0.606 BottomBorder+0.23 FrameWidth*4.1 FrameHeight/7 ];
frm3=uicontrol( ...
   'Style','frame', ...
  'Units','normalized', ...
  'Position',frm3Pos, ...
'BackgroundColor', [0.5 0.5 0.5]);

frm4Pos=[LeftFrameBorder*0.008+0.006 BottomBorder+FrameHeight/2+0.18 FrameWidth*4 FrameHeight/5-0.07 ];
frm4=uicontrol( ...
   'Style','frame', ...
  'Units','normalized', ...
  'Position',frm4Pos, ...
   'BackgroundColor', [0.6 0.5 0.5]);

frm5Pos=[LeftFrameBorder*0.008+0.006 BottomBorder+FrameHeight/2+0.30 FrameWidth*4 FrameHeight/5-0.04 ];
frm5=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm5Pos, ...
  'BackgroundColor', [0.6 0.5 0.5]);

frm6Pos=[0.509 BottomBorder*5.9 FrameWidth*4 FrameHeight/8.5 ];
frm6=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm6Pos, ...
   'BackgroundColor', [0.6 0.5 0.5]);

ComPos212=[];
ComPos212=[LeftFrameBorder*0.512 CompBottom-0.11 CompWidth*1.2 CompHeight*0.8];
Time_signal_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos212, ...
   'String','Total Time');

ComPos211=[];
ComPos211=[LeftFrameBorder*0.512 CompBottom-0.14 CompWidth*1.2 CompHeight];
Time_signal=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos211, ...
   'String','1');

ComPos213=[];
ComPos213=[LeftFrameBorder*0.016 BottomBorder*18 CompWidth*8.7 CompHeight];
Text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos213, ...
   'String','Signal - frequency - Ampl(VarNoise) - Time - Width - OffsetTime - Phase - FreqFin(Chirp) - Type','BackgroundColor', [0.6 0.5 0.5]);



ComPos31=[];
ComPos31=[LeftFrameBorder*0.45 CompBottom-0.11 CompWidth*0.73 CompHeight*0.8];
Fs_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos31, ...
   'String','Fs');

ComPos21=[];
ComPos21=[LeftFrameBorder*0.435 CompBottom-0.14 CompWidth*1.2 CompHeight];
Fsample=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos21, ...
   'String','10000');




ComPos6=[];
ComPos6=[LeftFrameBorder*0.03 CompBottom*0.84 CompWidth CompHeight*1.5];
Title_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos6, ...
   'String','Modulated Signal:');


ComPos32=[];
ComPos32=[LeftFrameBorder*0.63 CompBottom*1.02 CompWidth CompHeight];
Gaussian_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos32, ...
   'String','Filter :');


ComPos33=[];
ComPos33=[LeftFrameBorder*0.897 CompBottom*1.045 CompWidth*1.2 CompHeight/1.3];
Filter_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos33, ...
   'String','Bandwidth:');

ComPos34=[];
ComPos34=[LeftFrameBorder*0.9 CompBottom*1.01 CompWidth CompHeight];
Bandwidth=uicontrol( ...
   'Style','Edit', ...
   'Units','normalized', ...
   'Position',ComPos34, ...
   'String','1000');

filtre=cell(4,1);
filtre={'No filter';'FIR_2';'FIR_CLS';'KaiserFilter'};
ComPos35=[];
ComPos35=[LeftFrameBorder*0.73 CompBottom*1.01 CompWidth*2 CompHeight];
Filter_name=uicontrol( ...
   'Style','popupmenu', ...
   'Units','normalized', ...
   'Position',ComPos35, ...
   'String',filtre);

ComPos36=[];
ComPos36=[LeftFrameBorder*0.73 CompBottom*1.045 CompWidth*1.2 CompHeight/1.3];
Filter_text1=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos36, ...
   'String','Filter Type:');

ComPos37=[];
ComPos37=[LeftFrameBorder*1.1 CompBottom*1.01 CompWidth*1.4 CompHeight*1.5];
Filter_text1=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos37, ...
   'String','Apply1','Callback','Filter_Done');

ComPos37=[];
ComPos37=[LeftFrameBorder CompBottom*1.01 CompWidth*1.4 CompHeight*1.5];
Filter_text1=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos37, ...
   'String','Vision','Callback','Vision_filter');

ComPos38=[];
ComPos38=[LeftFrameBorder*0.63 CompBottom*0.38 CompWidth*1.2 CompHeight*1.5];
Gaussian_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos38, ...
   'String','Resampling:');

ComPos39=[];
ComPos39=[LeftFrameBorder*0.72 CompBottom*0.38 CompWidth*1.5 CompHeight*1.5];
filter_box=uicontrol( ...
   'Style','checkbox', ...
   'Units','normalized', ...
   'Position',ComPos39,'String','Filter Designed');

ComPos131=[];
ComPos131=[LeftFrameBorder*0.03 BottomBorder*16.2 CompWidth*8.2 CompHeight*3];
listbox_signal=uicontrol( ...
   'Style','listbox', ...
   'Units','normalized', ...
   'Position',ComPos131, ...
   'String','');


ComPos132=[];
ComPos132=[LeftFrameBorder*0.528 BottomBorder*17.1 CompWidth*1 CompHeight*1.3];
Clear_button=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos132, ...
   'String','Clear','Callback','Clear_gene');


ComPos40=[];
ComPos40=[LeftFrameBorder*0.86 CompBottom-0.49 CompWidth*0.8 CompHeight*0.8];
Fres_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos40, ...
   'String','Fs');

ComPos41=[];
ComPos41=[LeftFrameBorder*0.84 CompBottom-0.52 CompWidth*1.5 CompHeight];
Fresample=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos41, ...
   'String','20000000');

ComPos42=[];
ComPos42=[LeftFrameBorder*1 CompBottom-0.52 CompWidth*1.2 CompHeight*1.5];
bton_apply=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos42, ...
   'String','Apply','Callback','Resampling');

ComPos43=[];
ComPos43=[LeftFrameBorder*1.1 CompBottom-0.52 CompWidth*1.2 CompHeight*1.5];
bton_save=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos43, ...
   'String','Save','Callback','Save_function');


ComPos410=[];
ComPos410=[LeftFrameBorder*0.84 CompBottom-0.555 CompWidth*1.5 CompHeight*0.9];
Size_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos410, ...
   'String','Size(Bytes) :');

ComPos411=[];
ComPos411=[LeftFrameBorder*0.93 CompBottom-0.5542 CompWidth*1.5 CompHeight*0.9];
Size=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos411, ...
   'String','0');



ComPos412=[];
ComPos412=[LeftFrameBorder*1.025 CompBottom-0.555 CompWidth*0.8 CompHeight*0.9];
bton_save=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos412, ...
   'String','Size','Callback','Size_Out');

ComPos415=[];
ComPos415=[LeftFrameBorder*1.1 CompBottom-0.555 CompWidth*1.2 CompHeight];
bton_save=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos415, ...
   'String','GUI Modu','FontSize',7,'Callback','Load_modulation_gui');


signal_name=cell(8,1);
signal_name={'Sin';'Cosin';'rectpulse';'tripulse';'Gaussian';'Square';'Chirp';'Gauss Noise'};
CompLeft =0.8400;
t=0;
for i=1:4
    for j=1:2
        t=t+1;
p_btn = uicontrol(gcf,...
                 'style', 'pushbutton',...
                 'Position',[CompLeft*22+i*70 CompBottom*600-23*j 65 20 ],...
                 'String', signal_name{t},'Callback',[ 'signal_func (' num2str(t) ')']);
end
end

% Define dsp plot
HPerioFilFig= subplot(4,2,2);
 set(gca, ...
   'Units','normalized', ...
   'Position',[0.277 0.03 0.219 0.35]);

HPerioFig = subplot(4,1,2);
 set(gca, ...
    'Units','normalized', ...
    'Position',[0.03 0.03 0.22 0.35]);
%==================================

% Define dsp plot
HPerioFilter= subplot(4,2,1);

 set(gca, ...
   'Units','normalized', ...
   'Position',[0.525 0.44 0.23 0.37]);

HPerioFigresu = subplot(4,2,3);
 set(gca, ...
  'Units','normalized', ...
    'Position',[0.79 0.44 0.203 0.37]);
%==================================

  HPerioFigPlot= subplot(4,3,4);
 set(gca, ...
  'Units','normalized', ...
    'Position',[0.525 0.03 0.46 0.23]);
%==================================

