
% Clear out all old data

function reader1(choix)
% Clear out all old figures
% clf;
global numero;
disp(['choix : ' num2str(choix)'']);
numero=choix;
reader1_createfig2;