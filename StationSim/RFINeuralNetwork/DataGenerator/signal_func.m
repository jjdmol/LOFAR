function [s]=signal_func(hello);
global Time;
global Frequency;
global Time_signal;
global Band;
global Ampl;
global signal_name;
global signal_choice;
global Width;
global listbox_signal;
global res;
global s;

signal_choice=0;
signal_choice=hello;
FrameWidth = 0.12; 
LeftBorder = 0.05; 
BottomBorder=0.05; 
TotalWidth = 1 - 3*LeftBorder-FrameWidth ; TotalHeight = 1-2*BottomBorder;
FigSpace = 0.05;
Col1Width = 0.3*TotalWidth;
Col2Width = TotalWidth-Col1Width-FigSpace;
Lin1Height = 0.3*TotalHeight;
Lin2Height= TotalHeight-Lin1Height-FigSpace;
CompWidth=0.05;
CompHeight=0.03;
FrameBorder = 0.01;
LeftFrameBorder = LeftBorder + Col1Width+Col2Width+2*FigSpace;
FrameHeight = TotalHeight;
CompBackGroundColor = [0.5 0.5 0.5];


frm6Pos=[LeftFrameBorder*0.008+0.006 BottomBorder*8.4 FrameWidth*4 FrameHeight/4];
frm6=uicontrol(gcf,...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frm6Pos, ...
   'BackgroundColor', [0.6 0.5 0.5]);

if (signal_choice~=6)
ComPos7=[];
ComPos7=[LeftFrameBorder*0.008+0.1 BottomBorder*11.6 CompWidth*1.3 CompHeight];
Frequency=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos7, ...
   'String','1000');

ComPos9=[];
ComPos9=[LeftFrameBorder*0.008+0.1 BottomBorder*12.3  CompWidth*1.3 CompHeight*0.8];
Frequency_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos9, ...
   'String','Frequency');
end


ComPos8=[];
ComPos8=[LeftFrameBorder*0.22 BottomBorder*11.6 CompWidth*1.3 CompHeight];
Ampl=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos8, ...
   'String','1');

ComPos10=[];
ComPos10=[LeftFrameBorder*0.22 BottomBorder*12.3  CompWidth*1.3 CompHeight*0.8];
Ampl_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos10, ...
   'String','Ampl');

disp(['signal: ' num2str(signal_choice) '']);
ComPos11=[];
ComPos11=[LeftFrameBorder*0.03 BottomBorder*11.6  CompWidth*1.3 CompHeight*1.5];
Var_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos11, ...
   'String',signal_name(signal_choice));

ComPos12=[];
ComPos12=[LeftFrameBorder*0.008+0.1 BottomBorder*10 CompWidth*1.3 CompHeight];
Time=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos12, ...
   'String',get(Time_signal,'String'));

ComPos13=[];
ComPos13=[LeftFrameBorder*0.008+0.1 BottomBorder*10.7 CompWidth*1.3 CompHeight*0.8];
time_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos13, ...
   'String','Time(s)');


ComPos16=[];
ComPos16=[LeftFrameBorder*0.008+0.0175 BottomBorder*9.9 CompWidth*1.35 CompHeight*2.1];
Valide=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos16, ...
   'String','Vision','Callback','Vision');

ComPos17=[];
ComPos17=[LeftFrameBorder*0.49 BottomBorder*11.5 CompWidth*1.5 CompHeight*2];
Addition=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos17, ...
   'String','+','FontSize',18,'Callback',['Addition(' num2str(signal_choice) ')']);

Width=0;
if (signal_choice>2)
ComPos18=[];
ComPos18=[LeftFrameBorder*0.22 BottomBorder*10.7  CompWidth*1.3 CompHeight*0.8];
Width_text=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos18, ...
   'String','Width');

ComPos14=[];
ComPos14=[LeftFrameBorder*0.22 BottomBorder*10 CompWidth*1.3 CompHeight];
Width=uicontrol( ...
   'Style','edit', ...
   'Units','normalized', ...
   'Position',ComPos14, ...
   'String','4');
end

ComPos20=[];
ComPos20=[LeftFrameBorder*0.49 BottomBorder*9.9 CompWidth*1.5 CompHeight*2.1];
Valide=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos20, ...
   'String','-','fontsize',18,'Callback','minus_signal');



