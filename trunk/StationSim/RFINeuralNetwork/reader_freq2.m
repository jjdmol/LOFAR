global donnee_ben;
global Contrl_freq2;
global echantillonnage;
global numero_classe;
global Contr2_donnee;
global HTimeFig2;
global HDspFig2;
global HThreeDFig2;
global HSpecgramFig2;

selected = get(Contrl_freq2,'Value');
set(Contr2_donnee,'String',selected);

subplot(HTimeFig2);
%t=size_spectre:-1:1;
plot(DSP(donnee_ben{numero_classe}(selected)*echantillonnage,:));
%axis ([0 max(max(todraw)) 0 size_spectre]);

[ligne,colonne]=size(DSP);
moy=mean(DSP(donnee_ben{numero_classe}(selected),:));
   ecart=std(DSP(donnee_ben{numero_classe}(selected),:));
   %disp('moy = 'num2str(moy) ' ');
   %disp('ecart='num2str(ecart) ' ');
   trimbal=ones(ligne,1);
   trimbal(donnee_ben{numero_classe}*echantillonnage)=DSP(donnee_ben{numero_classe}*echantillonnage,colonne/2);
 
 subplot(HDspFig2);
 plot(10*log10(trimbal));
  
  
subplot (HSpecgramFig2);
cla
imagesc(DSP(donnee_ben{numero_classe}*echantillonnage,:))
colormap(1-gray)
l1=line([colonne/2 colonne/2],[0.05 length(donnee_ben{numero_classe})+0.45],'EraseMode','xor');
l2=line([1 ligne],[selected selected],'EraseMode','xor');

subplot(HThreeDFig2)
cla
l3=line([0 1],[1 1]);%Car pas d'intégration.
l4=line([0 1],[ecart/moy ecart/moy],'EraseMode','xor','Color',[1 0 0]);
   %axis ([0 77 0 max(max(todraw))]);
   %axis ([0 77 0 0.00005]);
   %else