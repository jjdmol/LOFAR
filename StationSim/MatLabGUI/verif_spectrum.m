close all 
clear all
dirpath = 'data';
number_antenna=92;
load('data/configuration.mat');
dirpath='C:\users\Dromer\Home\Astron\Matlab_Dat_Files\';
[px,py]=reader_array([dirpath Filename_array],NAntennas);
Nfft=1;

%first source
phi1=-1.2847;    
theta1=0.6239;
t=0:0.001:0.3;
t=t(1:100);
f=50;
x=8000*(sin(2*pi*f*t+0.6));
for i=1:length(x)
    test_vec1(i,:)=x(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi1)*sin(theta1)+py*sin(phi1)*sin(theta1))/Nfft); 
end


%Second source
phi_look=0.3;
theta_look=0.3;
t=0:0.001:0.3;
t=t(1:100);
f=50;
x=8000*(sin(2*pi*f*t+0.6));
for i=1:length(x)
    test_vec2(i,:)=x(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi_look)*sin(theta_look)+py*sin(phi_look)*sin(theta_look))/Nfft); 
end

%Addition
test_vec=test_vec1+test_vec2;

figure(1)
subplot(3,1,1)
hold on
tr=0:1/(length(t)-1):1;
plot(tr,abs(test_vec1(:,1)),'b')
plot(tr,abs(test_vec2(:,1)),'r')
title('Plotting the signals in the subband before addition : first antenna')
hold off

subplot(3,1,2)
plot(tr,abs(test_vec(:,1)),'b')
title('Plotting the signals in the subband after addition : first antenna')

subplot(3,1,3)
plot(20*log10(abs(fftshift(fft(test_vec(:,1),64)))),'b')
title('Plotting spectrum in the subband after addition : first antenna')

%Selection of a part of the data to apply STA bloc
xq=test_vec(1:100,:);
figure(2)
plot(abs(xq))
title('Time Series corresponding to the subband generated, first hundred samples'); 
te=size(xq,2);
%i=sqrt(-1);
%xq_test=[1+i,2-i,3*i,4].';
%xq=repmat(xq_test,1,100);

%Apply EVD
fprintf('ACM\n');
[ EvectorEVD, EvalueEVD,Cor ] = acm(xq.',number_antenna,te, 0.2, 0.4, px, py, 1, 1,0.25);
[maxi,index]=max(diag(EvalueEVD));
nsnsh=size(xq,1);
fprintf('\t\tMDL- EVD - Estimating the number of interfering sources\n');

for i = 1:size(EvectorEVD,3);
rfi_sources(i,:)=mdl(-sort(diag(-squeeze(EvalueEVD(:,:,i)))),number_antenna,nsnsh);
end
EVector=[];
for i=0:rfi_sources-1
EVector(:,i+1)=EvectorEVD(:,index-i);
%EVector(:,i+1)=(1/EvectorEVD(1,i+1))*EvectorEVD(:,i+1);
Evalue=EvalueEVD(:,index-i);
end   

%Single value decomposition verification
[EvectorSVD,EvalueSVD]=svd(xq.');
EvalueSVD=EvalueSVD-1;
sz=size(diag(EvalueSVD),1);
d=power(diag(EvalueSVD),2);
fprintf('\t\tMDL- SVD - Estimating the number of interfering sources\n');
rfi_sources_SVD=mdl(-sort(-d),number_antenna,nsnsh);

%Plotting the EigenVlaue of the 2 techniques EVD, SVD 
figure(3)
hold on
plot(-sort(-diag(EvalueEVD)),'*b');
plot(-sort(-d),'*r');
hold off
title('EigenValues using EVD, SVD')
legend('EigenValue using EVD','EigenValue using SVD');


%Deterministic nulling :
w1=steerv(px,py,phi1,theta1);
Vector_det=[w1];

%steering Vector
LookingDirection=steerv(px,py,phi_look,theta_look);

%projection 
WeightVector = awe(EVector, Evalue, rfi_sources , length(px),LookingDirection,1);
WeightVector_SVD = awe(EvectorSVD, Evalue,rfi_sources_SVD, length(px),LookingDirection,1);
WeightVector_det=awe(Vector_det, Evalue,rfi_sources, length(px),LookingDirection,1);

relfreq=1;
%BeamPattern obtention
% Generate Digital beamforming window 
relfreq=1;
 N=120;
 phi_theta=[];
 pat_steered=[];
 pat_nulled=zeros(N,N,size(WeightVector,2)); 
 fin=N*N;
 i=0;
 time = 0
  for u = 0:N
     u1 = -1 + 2*u/N;
     for v = 0:N
         v1 = -1 + 2*v/N;
         if (sqrt(u1^2+v1^2) <= sin(90/180*pi));
             i = i + 1;
             U(i) = u;
             V(i) = v;
             theta = asin(sqrt(u1^2+v1^2));
             phi = atan2(u1,v1);
             phi_theta=[phi_theta; phi theta];
             S{u+1,v+1}=[phi,theta];
             if mod(i,1000)==0
                 disp(['Status : ' num2str(i) ' - ' num2str(fin)]);
             end

             pat_steered(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                      (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(LookingDirection.')';
           
             pat_nulled(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                    (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(WeightVector.')';
                
             pat_nulled_SVD(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                    (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(WeightVector_SVD.')';
                
             pat_nulled_det(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                    (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(WeightVector_det.')';

         end;
     end;
 end



SteeredPattern = abs(pat_steered)/max(max(abs(pat_steered)));
BeamPattern = abs(pat_nulled)/max(max(abs(pat_nulled)));
BeamPattern_det = abs(pat_nulled_det)/max(max(abs(pat_nulled_det)));
BeamPattern_SVD = abs(pat_nulled_SVD)/max(max(abs(pat_nulled_SVD)));

figure(5)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi_look)*sin(theta_look),sin(phi_look)*sin(theta_look),'*b')
title('EVD using the first hundred data') 
caxis([-70 0]);
colorbar 

figure(6)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_SVD));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi_look)*sin(theta_look),sin(phi_look)*sin(theta_look),'*b')
title('SVD using the first hundred data') 
caxis([-70 0]);
colorbar 

figure(7)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(SteeredPattern));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi_look)*sin(theta_look),sin(phi_look)*sin(theta_look),'*b')
hold off
title('Steered Pattern');
caxis([-70 0]);
colorbar 

figure(8)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_det));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi_look)*sin(theta_look),sin(phi_look)*sin(theta_look),'*b')
title('Deterministic using the first hundred data') 
caxis([-70 0]);
colorbar 

[Y1,Y2]=plotspectrum3(test_vec.',px,py,phi_look,theta_look,WeightVector_SVD)
[Y3,Y4]=plotspectrum3(test_vec.',px,py,phi_look,theta_look,WeightVector_det)

figure
subplot(3,1,1)
plot(20*log10(abs(fftshift(fft(Y1,64)))),'b')
title('Plotting spectrum in the subband inner product : Steering vector')

subplot(3,1,2)
plot(20*log10(abs(fftshift(fft(Y2,64)))),'b')
title('Plotting spectrum in the subband inner product : WeigthVector EVD')

subplot(3,1,3)
plot(20*log10(abs(fftshift(fft(Y4,64)))),'b')
title('Plotting spectrum in the subband inner product : deterministic WeigthVector')

