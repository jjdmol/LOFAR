function [DSPresu,mmi1]=min_median2Clip(DSP_Dmoy,resu)
DSP=[];
close all
%DSP=10*log10(DSP_Dmoy);
resu=resu(1:8192);
sb = 50; % nombre de sous bandes
ns = 2; % Noise sigma 
%prendre 70 bandes pour 16384;
%prendre 64 bandes pour 8192;

nssb = 64; % nombre de sous bandes à créer pour le calcul des minima
[li,col]=size(DSP_Dmoy);
col=col-1;
decfac = ceil(col/nssb);
%sp1 = sp(:,55); % prendre un spectre au hazard
sp1=[]
for i=1:col
moyenne=mean(resu{i});
sp1=[sp1 moyenne];
end
%sum(DSP,1)/li; % prendre un spectre
sp1(decfac*nssb+1:end) = []; % pour faire un reshape sur x canaux, il faut que x*nssb soit un entier,
                             % x étant le nombre de cannaux par sous bande
ssp1 = reshape (sp1,decfac,[]);
min1 = min (ssp1);  % dans chaque slot des 315 cannaux (env 1 MHz) prendre le minimum.

mmi1= interp(min1,decfac); %le filtre median n'est pas necessaire, car interp réalise déja un filtrage


figure;
plot(10*log10(sp1)); %pour vérif
hold on;
plot(10*log10(mmi1),'r');
title('Mean of the power level in each time-bin weighted by the transfer function of the receiver'); 
Ylabel('power : dB');
Xlabel('Frequency channel');

%plot (sp1,'r');

%seuil2 = mmi1+2*ns; % ajouter ns sigma de bruit pour fixer le seuil
%plot(seuil2,'g');

%ob=zeros(sb,li);
%calcul du nombres de cannaux occupés dans toute la bande
%for i=1:li
  %  taux_occ=sp(1:decfac*nssb,i)-seuil2'; % fabriquer la différence entre le spectre et le seuil
   % for j=1:sb
    %    ob(j,i) = numel(taux_occ(taux_occ((j-1)*ceil(col/sb)+1:j*(ceil(/sb)-1))>0))/(ceil(col/sb)-1)*100; % trouver le nombre d'elements dont le signe est positif. Occupied Bins = ob
    %end
    %end

%figure;
%plot (ob);
%figure;
%hist (ob);
%if (~exist ('mean_ob')) mean_ob = ones(1,1); end
%if (~exist ('std_ob')) std_ob = ones(1,1); end

    
%std_ob = [std_ob ; std(ob)]
%mean_ob = [mean_ob; mean(ob)]
%figure;
%contourf (ob,5);
%set(gca,'xtick',[6:6:800]);
%set(gca,'CLim',[0 100]);
%colorbar;

%Normalisation du spectre intégré sur 6s.
resul=[];
seuil=[];
figure
for i=1:col
        resul(i)=sp1(i)/mmi1(i);
       % seuil(i)=2*ns/mmi1(i)+1;
end

plot(10*log10(resul));
title('Power level mean for each time bin after transfer function inversion');
Ylabel('power : dB');
Xlabel('Frequency channel');
%recherche des emetteurs à bandes étroites se distinguant
%du bruit.
%t=[];
%DSP2=resu;
%fin=col-2;
%for i=3:fin
 %   av=i+2;
 %   der=i-2;
 %   if (DSP2(av)<seuil & DSP2(der)<seuil & DSP2(i)>seuil)
 %       DSP2(i)=seuil;
 %       t=[t i];
 %end
 %end
%figure
%plot(DSP2)
%hold on 
%plot(seuil,'g')
%hold off
DSPresu=ones(li,col);
%Normalisation par rapport à la réponse en fréquence du détecteur
fonc_inst=mmi1;
S=repmat(fonc_inst,li,1);
DSPresu=DSP_Dmoy(:,1:col)./S;
figure
colormap(1-gray)
imagesc(10*log10(DSPresu'))
title('Time-frequency plane');
Ylabel('Frequency channel');
Xlabel('Time-bin');
%On considère que les bandes emettrices repérée sur
%l'intégration sur 6s sont présentes sur les spectres instantanés
%Reperage des points au dessus du seuil car log >1
whos
%for i=1:length(t)
   % a=t(i);
    %for j=1:li
       % DSP1(j,a)=0;
        %end
    %end
DSP=DSPresu';
%Visualisation
%figure
%imagesc(10*log10(DSP_Dmoy'))
%colormap(1-gray)

%figure
%imagesc(10*log10(DSP'))
%colormap(1-gray)

%figure 
%imagesc(DSP1')
%colormap(1-gray)

%moyennageDSP