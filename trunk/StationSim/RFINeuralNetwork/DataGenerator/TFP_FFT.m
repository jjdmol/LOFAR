MainFigPos_Perio = [ 5 35 250 150];
shh = get(0,'ShowHiddenHandles');
set(0,'ShowHiddenHandles','on')


HMainFig_Perio = figure(...
   'Name','FFT', ...
   'handlevisibility','callback',...
   'IntegerHandle','off',...
   'NumberTitle','off',...
   'Tag','MainFig2',...
   'position',MainFigPos_Perio);

frm1PosPerio=[LeftFrameBorder*0.008 BottomBorder FrameWidth*8 FrameHeight ];
frm1Perio=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm1PosPerio, ...
   'BackgroundColor',CompBackGroundColor );

frm1PosPerio=[LeftFrameBorder*0.045 BottomBorder*2 FrameWidth*7.5 FrameHeight*0.9 ];
frm1Perio=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm1PosPerio, ...
   'BackgroundColor',[0.6 0.5 0.5] );

ComPos50=[];
ComPos50=[LeftFrameBorder*0.2 CompBottom*0.7 CompWidth*5 CompHeight*6];
Freq_Res_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos50, ...
   'String','Frequency resolution');

ComPos51=[];
ComPos51=[LeftFrameBorder*0.2 CompBottom*0.52 CompWidth*5 CompHeight*5];
Freq_Res=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos51, ...
   'String','1000');

ComPos52=[];
ComPos52=[LeftFrameBorder*0.68 CompBottom*0.7 CompWidth*5 CompHeight*6];
Int_Time_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos52, ...
   'String','Integration Time');

ComPos53=[];
ComPos53=[LeftFrameBorder*0.68 CompBottom*0.52 CompWidth*5 CompHeight*5];
Int_Time=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos53, ...
   'String','1');


ComPos55=[];
ComPos55=[LeftFrameBorder*0.4 CompBottom*1.06 CompWidth*6 CompHeight*3];
Apply_Freq_Res=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos55, ...
   'String','FFT','BackgroundColor',CompBackGroundColor);

if indice_reference==1
Callback_choice='DSPHolland_integrate_reference';
else
Callback_choice='DSPHolland_integrate';   
end
ComPos54=[];
ComPos54=[LeftFrameBorder*0.38 CompBottom*0.25 CompWidth*8 CompHeight*5.3];
Apply_Freq_Res=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos54, ...
   'String','Apply','Callback',Callback_choice);

