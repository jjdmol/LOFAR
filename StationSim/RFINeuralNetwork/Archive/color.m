close all

[flin,fcol]=size(DSP);
DSP_Log=10*log10(DSP);
Masque=(0)*ones(flin,fcol);
int=length(freq);
for i=1:int
      Masque(freq(i)*echantillonnage,:)= DSP_Log(freq(i)*echantillonnage,:);     
  end
Cdata=[];
Cdata(:,:,2)=Masque; 
Cdata(:,:,1)=0;
Cdata(:,:,3)=0;
Cdata(:,:,2)=Cdata(:,:,2)+abs(min(min(Cdata(:,:,2))));
Cdata(:,:,2)=Cdata(:,:,2)/(max(max(Cdata(:,:,2))));

Cdata(find(Masque==0))=0;
imagesc(DSP_Log)
hold on
colormap(1-gray)
alphamap('rampup',0.00001);
I2=imagesc(Cdata);
set(I2,'AlphaData',Masque)
%set(I2,'CDataMapping','scaled')
hold off