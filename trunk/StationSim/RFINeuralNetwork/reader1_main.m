

%global declarations
%global ch1;
%global ch2;
%global x1;
%global x2;
%global e;
%global nb_spectre;
%global size_spectre;
global echantillonnage;
global freq;
global numero;
global DSP;
global MyData2;
global freq;
%local declarations
MEGA = 1e6;

%initialisation of size_spectre for all sub-functions
size_spectre = 8192;

%=====================================================
% Read in a data file




if (length(freq)==0)
    message='neurone non activé';
    title='Attention';
    msgbox(message,title,'warn')
else
%figure
%plot(mean(DSP_inter(freq,:)))
subplot (HSpecgramFig);
imagesc(DSP(freq*echantillonnage,:))
colormap(1-gray)
set(gcf,'windowbuttondownfcn','reader1_curpt');  
end


%ch1=0;
%ch2=0;
%x1=0;
%x2=0;
%e=0;



%transform matrix index in real world coordonates
%u = size (ch1); u = u(2);
%x = [0 u*e.millisec/1000];
%y = [e.F_low/MEGA  e.F_high/MEGA];

%
%figure(HMainFig);
%if e.nb_voies == 3 
%   subplot(221);
%   imagesc (x,y,20*log10(abs(ch1)));
%   subplot(222);
 %  imagesc (x,y,20*log10(abs(ch2)));
%   subplot(223);
%   imagesc (x,y,20*log10(abs(x1)));
%   subplot(224);  
%   imagesc (x,y,20*log10(abs(x2)));
%else   
%   imagesc (x,y,20*log10(abs(ch1)));
%end


%Pas tant que desactivation curpt programmée!!!!!!
%subplot (HThreeDFig);
%surf (ch1);
%subplot (HSpecgramFig);
%imagesc (x,y,20*log10(DSP_inter));

%format compact;
%e
%nb_spectre
%size_spectre

%clear ch1;
%clear ch2;
%clear x1;
%clear x2;
%clear e;
%clear nb_spectre;
%clear size_spectre;

   
