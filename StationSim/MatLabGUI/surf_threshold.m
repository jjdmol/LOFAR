if I==[];
    message='You must apply a threshold';
    title='Warning';
    msgbox(message,title,'Warn')
else
figure('Name','Threshold Surf Plot')
[li_thres,co_thres]=size(I);
hold on
surf(20*log10(Threshold_modu)*ones(li_thres,co_thres),'MarkerEdgeColor',[1 0 0]);
surf(20*log10(abs(I)));
brighten(0.8)
shading interp;
hold off
end