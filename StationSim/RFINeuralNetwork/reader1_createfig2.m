global HBtnColorBar;
global HColorMap;
global HMainFig ;
global HDspFig ;
global HTimeFig ;
global HThreeDFig ;
global HSpecgramFig ;
global Contrl_freq;
global fichier;
global echantillonnage;
global freq;
global Contr_donnee;
global MyData2;
%global numero;
%==================================
%define general aspects
FrameWidth = 0.12; 

LeftBorder = 0.05; BottomBorder=0.05; 
TotalWidth = 1 - 3*LeftBorder-FrameWidth ; TotalHeight = 1-2*BottomBorder;
FigSpace = 0.05;
Col1Width = 0.3*TotalWidth;
Col2Width = TotalWidth-Col1Width-FigSpace;
Lin1Height = 0.3*TotalHeight;
Lin2Height= TotalHeight-Lin1Height-FigSpace;

FrameBorder = 0.01;
LeftFrameBorder = LeftBorder + Col1Width+Col2Width+2*FigSpace;
FrameHeight = TotalHeight;
CompBackGroundColor = [0.5 0.5 0.5];



%create figure 1 and set its attributs
MainFigPos = [ 5 35 1017 663];
shh = get(0,'ShowHiddenHandles');
set(0,'ShowHiddenHandles','on')
%Figure principale
HMainFig = figure(...
   'Name','Spectrogramme', ...
   'handlevisibility','callback',...
   'IntegerHandle','off',...
   'NumberTitle','off',...
   'Tag','MainFig',...
   'position',MainFigPos);
     
 %==================================
 % Set up main axes of MainFig
 axes( ...
    'Units','normalized', ...
    'Position',[LeftBorder BottomBorder TotalWidth TotalHeight]);
 
 set(HMainFig,'defaultaxesposition',[LeftBorder BottomBorder TotalWidth TotalHeight])
 
 %==================================
 % Define 3D subplot properties
 HThreeDFig = subplot(2,2,1);
 set(gca, ...
    'Units','normalized', ...
    'Position',[LeftBorder BottomBorder+Lin2Height+FigSpace Col1Width Lin1Height]);
 %==================================
 % Define time plot
 HTimeFig = subplot(2,2,2);
 set(gca, ...
    'Units','normalized', ...
    'Position',[LeftBorder+Col1Width+FigSpace BottomBorder+Lin2Height+FigSpace Col2Width Lin1Height], ...
    'XTick',[],'YTick',[], ...
    'Box','on');
%==================================
 % Define dsp plot
 HDspFig = subplot(2,2,3);
 set(gca, ...
    'Units','normalized', ...
    'Position',[LeftBorder BottomBorder Col1Width Lin2Height], ...
    'XTick',[],'YTick',[], ...
    'Box','on');
%==================================
 % Define specgram plot
 HSpecgramFig = subplot(2,2,4);
 set(gca, ...
    'Units','normalized', ...
    'Position',[LeftBorder+Col1Width+FigSpace  BottomBorder Col2Width Lin2Height], ...
    'XTick',[],'YTick',[], ...
    'Box','on');
 
 %====================================
 % Information for all buttons (and menus)
 CompHeight = 0.03;

 % Spacing between the button and the next command's label
 CompSpace = 0.02;
 
%====================================
% The CONSOLE frame

frmPos=[LeftFrameBorder BottomBorder FrameWidth FrameHeight ];
h=uicontrol( ...
   'Style','frame', ...
   'Units','normalized', ...
   'Position',frmPos, ...
   'BackgroundColor',CompBackGroundColor );

% First Frame Component attributs

CompLeft = LeftFrameBorder + FrameBorder;
CompBottom = FrameHeight - FrameBorder - CompHeight;
CompWidth = FrameWidth - 2*FrameBorder;

CompPos7 = [CompLeft CompBottom*1.05 CompWidth CompHeight];
hand1 = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos7, ...
   'Horiz','left', ...
   'String','Choix donnee :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');


ComPos8=[];
ComPos8=[CompLeft+0.7*FrameWidth-FrameBorder CompBottom*1.06 CompWidth*0.3 CompHeight*0.7];
Contr_donnee=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos8, ...
   'String','rien');

% Affichage label 
CompPos = [CompLeft CompBottom CompWidth CompHeight];
h = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'Horiz','left', ...
   'String','Neurone :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');

% Map Menu label 
CompBottom = CompBottom - CompHeight - CompSpace;
CompWidth = 0.4*FrameWidth - FrameBorder;
CompPos = [CompLeft CompBottom CompWidth CompHeight];
h = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'Horiz','left', ...
   'String','Coul. :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');
CompPos9 = [CompLeft*0.06 CompBottom*0.78 CompWidth*4.7 CompHeight*0.75];
h = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos9, ...
   'Horiz','left', ...
   'String','Profil de la bande de fr�quence � t :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');

CompPos5 = [CompLeft*0.45 CompBottom*0.78 CompWidth*6.7 CompHeight*0.75];
h = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos5, ...
   'Horiz','left', ...
   'String','Plan Temps-fr�quence des donn�es s�lectionn�es :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');


CompPos6 = [CompLeft*0.45 CompBottom*1.175 CompWidth*4 CompHeight*0.75];
h = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos6, ...
   'Horiz','left', ...
   'String','Profil temporelle de la donnee :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');


% Map Menu
CompWidth = 0.6*FrameWidth - FrameBorder;

CompPos = [CompLeft+0.4*FrameWidth-FrameBorder CompBottom CompWidth CompHeight];
CompStr='Default|Bone|Jet|Hsv|Hot|Cool|Gray';
callbackStr='reader1_setmapcolor';
HColorMap=uicontrol( ...
   'Style','popup', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'String',CompStr, ...
   'Callback',callbackStr);

% Brigthness label 
CompBottom = CompBottom - CompHeight - CompSpace;
CompWidth = 0.4*FrameWidth - FrameBorder;
CompPos = [CompLeft CompBottom CompWidth CompHeight];
h = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'Horiz','left', ...
   'String','Dyn. :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');

% Brigthness Btn1
CompWidth = 0.3*FrameWidth - FrameBorder;
CompPos = [CompLeft+0.4*FrameWidth-FrameBorder CompBottom CompWidth CompHeight];
CompStr='+';
callbackStr='reader1_setbrightnessup';
HBtnPlus=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'String',CompStr, ...
   'Callback',callbackStr);

% Brigthness Btn2
CompWidth = 0.3*FrameWidth - FrameBorder;
CompPos = [CompLeft+0.7*FrameWidth-FrameBorder CompBottom CompWidth CompHeight];
CompStr='-';
callbackStr='reader1_setbrightnessdown';
HBtnMoins=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'String',CompStr, ...
   'Callback',callbackStr);


% Colorbar btn
CompBottom = CompBottom - CompHeight - CompSpace;
CompWidth = FrameWidth - 2*FrameBorder;
CompPos = [CompLeft CompBottom CompWidth CompHeight];
BtnState = 0;
CompStr='Voir Echelle';
callbackStr='reader1_scale';
HBtnColorBar=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'String',CompStr, ...
   'UserData',BtnState,...
   'Callback',callbackStr);


% Environnement label
CompBottom = CompBottom - CompHeight - CompSpace;
CompPos = [CompLeft CompBottom CompWidth CompHeight];
h = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'Horiz','left', ...
   'String','Environnement :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');


CompPos4 = [CompLeft CompBottom*0.85 CompWidth CompHeight*1.2];
h = uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',CompPos4, ...
   'Horiz','left', ...
   'String','Frequence (en lin�aire) :', ...
   'Interruptible','off', ...
   'BackgroundColor',CompBackGroundColor , ...
   'ForegroundColor','white');


% Save Btn
CompWidth = 0.5*FrameWidth - FrameBorder;
CompPos = [CompLeft+0.5*FrameWidth-FrameBorder CompBottom*0.95 CompWidth CompHeight];
CompStr='Save';
callbackStr='reader1_save';
HBtnSave=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',CompPos, ...
   'String',CompStr, ...
   'Callback',callbackStr);

%==================================
%Appel pour la DSP
Indique=[];
freq=find(MyData2==numero);
Indique1=1:length(freq);
Indique=[Indique1' freq*echantillonnage];
callback_str='reader_freq';
ComPos2=[];
ComPos2=[CompLeft+0.1*FrameWidth-FrameBorder 0.2*CompBottom CompWidth*2 CompHeight*14];
Contrl_freq=uicontrol( ...
   'Style','listbox', ...
   'Units','normalized', ...
   'Position',ComPos2, ...
   'String',num2str(Indique) , ...
   'Callback',callback_str);
%set(Contrl_freq,'Selected','On');
%Appel pour la DSP
ComPos3=[];
ComPos3=[CompLeft+0.5*FrameWidth-FrameBorder 1.32*CompBottom CompWidth*0.8 CompHeight*0.7];
Contrl_neurone=uicontrol( ...
   'Style','text', ...
   'Units','normalized', ...
   'Position',ComPos3, ...
   'String',num2str(numero+1));

if length(fichier)>0
    ComPos5=[];
    callbackStr='reader1_createfig3';
    ComPos5=[CompLeft+0.2*FrameWidth-FrameBorder 0.1*CompBottom CompWidth*1.5 CompHeight*1.3];
 Contrl_Acti=uicontrol( ...
   'Style','pushbutton', ...
   'Units','normalized', ...
   'Position',ComPos5, ...
   'String','Activation','Callback',callbackStr);   
end


reader1_main;