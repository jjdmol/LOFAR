function plot_data_tp2(Donnees,Labels)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Fonction plot_data_tp2
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%	Affiche les données de départ , par classe
%
%	Fonction spécifique pour un exemple de trois classes 
%	ou les labels correspondants sont
%
%	'un'
%	'deux'
%	'trois'
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

D1=[];D2=[];D3=[];
for ii=1:size(Donnees,1)
	Label=Labels{ii};
	if strcmp(Label,'un')
		D1=[D1;Donnees(ii,:)];
	elseif strcmp(Label,'deux')
		D2=[D2;Donnees(ii,:)];
	else
		D3=[D3;Donnees(ii,:)];
	end
end
plot(D1(:,1),D1(:,2),'+r',D2(:,1),D2(:,2),'*g',D3(:,1),D3(:,2),'ob');
set(gca, 'YDir','reverse'), axis image
legend('Classe 1','Classe2','Classe3');
return

		
