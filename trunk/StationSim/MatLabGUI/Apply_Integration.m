[ligPoly,colPoly]=size(OutputSignal)
step_poly=str2num(get(Int_time,'String'))
fin_poly=round(colPoly/step_poly)-1;
if fin_poly<1
message=['Integration Time must be equal or less than ' colPoly ''];
    title='Warning';
    msgbox(message,title,'warn')
end
step_poly0=1;
step_poly1=step_poly;
int_poly=[];
resu_poly=[];

if step_poly==1
    resu_poly=OutputSignal';
else
for i=1:fin_poly
    int_poly=OutputSignal(:,step_poly0:step_poly1);
    int_poly=mean(int_poly');
    resu_poly=[resu_poly;int_poly];
    step_poly0=step_poly1
    step_poly1=step_poly1+step_poly
end
end
set(0,'CurrentFigure',HMainFig_gene)
set(real_time,'string',1/freq_real*step_poly);
set(real_fr,'String',freq_real)
subplot(PolyphasePlot)
imagesc(20*log10(abs(resu_poly')))