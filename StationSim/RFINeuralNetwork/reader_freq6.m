global Contrl_freq6;
global Contr_donnee6;


visua=get(Contrl_freq6,'string');
selected = get(Contrl_freq6,'Value');

visua=str2num(visua);

set(Contr_donnee6,'String',selected);
subplot(HTimeFig6);
%t=size_spectre:-1:1;
plot(DSP(visua(selected),:));
%axis ([0 max(max(todraw)) 0 size_spectre]);

[ligne,colonne]=size(DSP);
moy=mean(DSP(visua(selected),:));
   ecart=std(DSP(visua(selected),:));
   %disp('moy = 'num2str(moy) ' ');
   %disp('ecart='num2str(ecart) ' ');
   trimbal=ones(8192,1);
   trimbal(visua(:))=DSP(visua(:),colonne/2);
 
 subplot(HDspFig6);
 plot(10*log10(trimbal));
  
  
subplot (HSpecgramFig6);
cla
imagesc(DSP(visua*echantillonnage,:))
colormap(1-gray)
l1=line([colonne/2 colonne/2],[0.05 length(visua(:))+0.45],'EraseMode','xor');
l2=line([1 ligne],[selected selected],'EraseMode','xor');

subplot(HThreeDFig6)
cla
l3=line([0 1],[1 1]);%Car pas d'intégration.
l4=line([0 1],[ecart/moy ecart/moy],'EraseMode','xor','Color',[1 0 0]);
   %axis ([0 77 0 max(max(todraw))]);
   %axis ([0 77 0 0.00005]);
   %else