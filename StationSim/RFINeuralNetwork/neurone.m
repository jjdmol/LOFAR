function neurone(numero)
DSP_inter=[];
Data2=[];
DSP_inter = evalin('base','DSP');
Data2=evalin('base','MyData2');
freq=find(Data2==numero);
if (length(freq)==0)
    message='neurone non activé';
    title='Attention';
    msgbox(message,title,'warn')
else
figure
plot(mean(DSP_inter(freq,:)))
figure
imagesc(DSP_inter(freq,:))
colormap(1-gray)
end
