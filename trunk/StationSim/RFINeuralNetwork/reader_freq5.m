global Contrl_Actives5;
global Contrl_donnee5;
global Vecteur_choisi;

[ligne,colonne]=size(DSP);

visua5=get(Contrl_Actives5,'string');
selected5 = get(Contrl_Actives5,'Value');
visua5=str2num(visua5);
set(Contrl_donnee5,'String',selected5);
subplot(HTimeFig5);
%t=size_spectre:-1:1;
plot(DSP(visua5(selected5,2),:));




subplot (HSpecgramFig5);
cla
imagesc(DSP(Vecteur_choisi(:,1)*echantillonnage,:))
colormap(1-gray)
l1=line([colonne/2 colonne/2],[0.05 length(Vecteur_choisi(:,1))+0.45],'EraseMode','xor');
l2=line([1 ligne],[selected5 selected5],'EraseMode','xor');

