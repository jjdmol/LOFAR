function reader5_curpt(action)


global HMainFig5 ;
global HDspFig5 ;
global HTimeFig5;
global HThreeDFig5;
global HSpecgramFig5 ;
global DSP;
global Vecteur_choisi;
global echantillonnage;
global Contrl_Actives5;
global Contrl_donnee5;

[lidsp,coldsp]=size(DSP);

ct=0;
MEGA = 1e6;
%if nargin == 0 % Set the WindowButtonDownFcns
 % set(gcf,'WindowButtonDownFcn','reader5_curpt down')
%if strcmp(action,'down') % Button Down
%  set(gcf,'WindowButtonMotionFcn','reader5_curpt move', ...
%'WindowButtonUpFcn','reader5_curpt up')
%elseif strcmp(action,'move') % Motion action
   

%elseif strcmp(action,'up')
   %set(gcf,'WindowButtonMotionFcn','');   
   
 %  todraw =ones(size_spectre,nb_spectre);
  % todraw = ch1;
  % deltaF = abs ((e.F_high - e.F_low)/MEGA);
  pos=get(HSpecgramFig5,'CurrentPoint')
  col=round(pos(1,1));
  lin=round(pos(1,2));
  if (lin<1) lin=1; end;
  if (lin>coldsp) lin=length(Vecteur_choisi(:,1));
  end;
  set(Contrl_donnee5,'String',lin);
  %if (col<1) col=1; end;
  %if (col>nb_spectre) col=nb_spectre; end;
  %set(Contr2_donnee,'String',num2str(lin));
   subplot(HTimeFig5);
   %t=size_spectre:-1:1;
   ans=Vecteur_choisi((lin),1)*echantillonnage;
   plot(10*log10(DSP(ans,:)));

   %axis ([0 max(max(todraw)) 0 size_spectre]);
   %moy=mean(DSP(lin*echantillonnage,:));
   %ecart=std(DSP(lin*echantillonnage,:));
   %disp('moy = 'num2str(moy) ' ');
   %disp('ecart='num2str(ecart) ' ');

subplot (HSpecgramFig5);
cla
imagesc(DSP(Vecteur_choisi(:,1)*echantillonnage,:))
colormap(1-gray)
l1=line([col col],[1 604],'EraseMode','xor');
l2=line([1 8192],[lin lin],'EraseMode','xor');
%end
%subplot(HThreeDFig2)
%cla
%l3=line([0 1],[1 1]);%Car pas d'intégration.
%l4=line([0 1],[ecart/moy ecart/moy],'EraseMode','xor','Color',[1 0 0]);

%ct=ct+1;
%end
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
