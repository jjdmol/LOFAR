OutputSignal=OutputSignal';
[ligPoly,colPoly]=size(OutputSignal)
step_poly=str2num(get(Int_time,'String'))
fin_poly=round(colPoly/step_poly)-1;
step_poly0=1;
step_poly1=step_poly;
int_poly=[];
resu_poly=[];

if step_poly==1
    resu_poly=OutputSignal;
else
for i=1:fin_poly
    int_poly=OutputSignal(:,[step_poly0,step_poly1]);
    resu_poly=[resu_poly;int_poly];
    step_poly0=step_poly1;
    step_poly1=step_poly1+step_poly;
end
end
set(0,'CurrentFigure',HMainFig_gene)
subplot(PolyphasePlot)
imagesc(20*log10(abs(resu_poly')))