function reader2_curpt(action)

global Contrl_freq2;
global ch2;
global x1;
global x2;
global e;
global nb_spectre;
global size_spectre;
global HMainFig2 ;
global HDspFig2 ;
global HTimeFig2 ;
global HThreeDFig2;
global HSpecgramFig2 ;
global DSP;
global donnee_ben;
global echantillonnage;
global numero_classe;
global Contr2_donnee;
ct=0;
MEGA = 1e6;
if nargin == 0 % Set the WindowButtonDownFcns
  set(gcf,'WindowButtonDownFcn','reader2_curpt down')
elseif strcmp(action,'down') % Button Down
  set(gcf,'WindowButtonMotionFcn','reader2_curpt move', ...
'WindowButtonUpFcn','reader2_curpt up')
elseif strcmp(action,'move') % Motion action
   

elseif strcmp(action,'up')
   set(gcf,'WindowButtonMotionFcn','');
 %  todraw =ones(size_spectre,nb_spectre);
  % todraw = ch1;
  % deltaF = abs ((e.F_high - e.F_low)/MEGA);
  pos=get(gca,'CurrentPoint')
  col=round(pos(1,1));
  lin=round(pos(1,2));
  if (lin<1) lin=1; end;
  if (lin>size_spectre) lin=length(freq); end;
  %if (col<1) col=1; end;
  %if (col>nb_spectre) col=nb_spectre; end;
  set(Contr2_donnee,'String',num2str(lin));
   subplot(HTimeFig2);
   %t=size_spectre:-1:1;
   plot(10*log10(DSP(donnee_ben{numero_classe}(lin)*echantillonnage,:)));
   %axis ([0 max(max(todraw)) 0 size_spectre]);
   moy=mean(DSP(lin*echantillonnage,:));
   ecart=std(DSP(lin*echantillonnage,:));
   %disp('moy = 'num2str(moy) ' ');
   %disp('ecart='num2str(ecart) ' ');
trimbal=zeros(8192,1);
trimbal(donnee_ben{numero_classe}*echantillonnage)=10*log10(DSP(donnee_ben{numero_classe}*echantillonnage,col));
subplot(HDspFig2);
plot(trimbal);
   
subplot (HSpecgramFig2);
cla
imagesc(DSP(donnee_ben{numero_classe}*echantillonnage,:))
colormap(1-gray)
l1=line([col col],[1 604],'EraseMode','xor');
l2=line([1 8192],[lin lin],'EraseMode','xor');

subplot(HThreeDFig2)
cla
l3=line([0 1],[1 1]);%Car pas d'intégration.
l4=line([0 1],[ecart/moy ecart/moy],'EraseMode','xor','Color',[1 0 0]);

ct=ct+1;
end
%if (subpl == 204)
   %   todraw = ch1;
   %elseif (subpl == 206)todraw =ones(size_spectre,nb_spectre);
   %   todraw = ch2;
   %elseif (subpl == 208)
   %   todraw = x1;
   %elseif (subpl == 210)
   %   todraw = x2;
   %else
   %   todraw = ch1;
   %end;
