global res;
global ans;
global list;
global listbox_signal;
global HPerioFig;
global HPerioFilter;
global HPerioFigPlot;
global HPerioFilFig;
global HPerioFigresu;
global listbox_signal;
global HPerioFigresu;


clear res;
clear s;
clear ans;
list={};
indice=0;
count_sig=0;
clear features;
set(listbox_signal,'String',list);
subplot(HPerioFilFig);
cla
subplot(HPerioFig); 
cla
subplot(HPerioFilter);
cla
subplot(HPerioFigresu); 
cla
subplot(HPerioFigPlot);
cla