global Fresample;
global filter_box;
global HPerioFigPlot;
global Res_Fr;
global resampler;

filter_Selected=get(filter_box,'Value');
S_Fr=str2num(get(Fsample,'String'));
Res_Fr=str2num(get(Fresample,'String'));

switch filter_Selected
    
case 0
    resampler=resample(res,Res_Fr,S_Fr);
case 1
    resampler=resample(res,Res_Fr,S_Fr,b);  
end

subplot(HPerioFigPlot)
periodogram(resampler,hamming(length(resampler)),'onesided',16384,Res_Fr)
    