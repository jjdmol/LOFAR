function reader1_curpt(action)

global Contrl_freq;
global ch2;
global x1;
global x2;
global e;
global nb_spectre;
global size_spectre;
global HMainFig ;
global HDspFig ;
global HTimeFig ;
global HThreeDFig;
global HSpecgramFig ;
global DSP;
global freq;
global echantillonnage;
global Contr_donnee;
global Contr_donnee;

MEGA = 1e6;
if nargin == 0 % Set the WindowButtonDownFcns
  set(gcf,'WindowButtonDownFcn','reader1_curpt down')
elseif strcmp(action,'down') % Button Down
  set(gcf,'WindowButtonMotionFcn','reader1_curpt move', ...
'WindowButtonUpFcn','reader1_curpt up')
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
 

set(Contr_donnee,'String',lin);
  %if (col>nb_spectre) col=nb_spectre; end;
   subplot(HTimeFig);
   %t=size_spectre:-1:1;
   plot(10*log10(DSP(freq(lin)*echantillonnage,:)));
   %axis ([0 max(max(todraw)) 0 size_spectre]);
    moy=mean(DSP(lin*echantillonnage,:));
   ecart=std(DSP(lin*echantillonnage,:));
   %disp('moy = 'num2str(moy) ' ');
   %disp('ecart='num2str(ecart) ' ');
   trimbal=zeros(8192,1);
   trimbal(freq*echantillonnage)=10*log10(DSP(freq*echantillonnage,col));
 
 subplot(HDspFig);
 plot(trimbal);
  
  
subplot (HSpecgramFig);
cla
imagesc(DSP(freq*echantillonnage,:))
colormap(1-gray)
l1=line([col col],[1 604],'EraseMode','xor');
l2=line([1 8192],[lin lin],'EraseMode','xor');

subplot(HThreeDFig)
cla
l3=line([0 1],[1 1]);%Car pas d'intégration.
l4=line([0 1],[ecart/moy ecart/moy],'EraseMode','xor','Color',[1 0 0]);

   %axis ([0 77 0 max(max(todraw))]);
   %axis ([0 77 0 0.00005]);
   %else
  %error('Invalid input to curpt.')
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
