

%global declarations
%global ch1;
%global ch2;
%global x1;
%global x2;
%global e;
%global nb_spectre;
%global size_spectre;
global echantillonnage;
global DSP;
global Indique6;
%local declarations
MEGA = 1e6;

%initialisation of size_spectre for all sub-functions
size_spectre = 8192;

%=====================================================
% display in DSP plot
subplot (HSpecgramFig6);
imagesc(DSP(Indique6*echantillonnage,:))
colormap(1-gray)
set(gcf,'windowbuttondownfcn','reader1_curpt6');  



