
% Clear out all old data

function reader2(choix_classe)
% Clear out all old figures
% clf;
global numero_classe;
disp(['choix : ' num2str(choix_classe)'']);
numero_classe=choix_classe;
reader1_createfig4;