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

%second source
phi2=-2.3362;
theta2=0.89;
t=0:0.001:0.3;
t=t(1:100);
f=50;
y=7200*(sin(2*pi*f*t+0.2)+5);
for i=1:length(x)
    test_vec2(i,:)=y(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi2)*sin(theta2)+py*sin(phi2)*sin(theta2))/Nfft); 
end

%third source
phi3=2.9095;
theta3=0.6701;
t=0:0.001:0.3;
t=t(1:100);
f=53;
z=5400*(cos(2*pi*f*t+0.4)+4);
for i=1:length(x)
    test_vec3(i,:)=z(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi3)*sin(theta3)+py*sin(phi3)*sin(theta3))/Nfft); 
end


%fourth source
phi4=-0.5633;   
theta4=0.2524;
t=0:0.001:0.3;
t=t(1:100);
f=55;
s=1000*sin(2*pi*f*t+0.6)+10000*sin(2*pi*f*2*t+0.3);
for i=1:length(x)
    test_vec4(i,:)=s(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi4)*sin(theta4)+py*sin(phi4)*sin(theta4))/Nfft); 
end

%Addition
test_vec=test_vec1+test_vec3+test_vec4+test_vec2;

figure(1)
subplot(3,1,1)
hold on
tr=0:1/(length(t)-1):1;
plot(tr,abs(test_vec1(:,1)),'b')
plot(tr,abs(test_vec2(:,1)),'r')
plot(tr,abs(test_vec3(:,1)),'m')
plot(tr,abs(test_vec4(:,1)),'black')
title('Plotting the signals in the subband before addition : first antenna')
hold off

subplot(3,1,2)
plot(tr,abs(test_vec(:,1)),'b')
title('Plotting the signals in the subband after addition : first antenna')

subplot(3,1,3)
plot(20*log10(abs(fftshift(fft(test_vec(:,1),64)))),'b')
title('Plotting spectrum in the subband after addition : first antenna')


% fid=fopen('data\test_vectorSTA.txt','w+');
% fprintf(fid,[num2str(size(test_mat,2)) 'x' num2str(size(test_mat,1))   ' \n ']);
% fprintf(fid,'[');
% j=0
% fin=num2str(size(test_mat,1))
% fin_plu=num2str(size(test_mat,2))
% for j=1:92
%     for k=1:300
%         fprintf(fid,'(%e, %e)\t',real(test_mat(j,k)),imag(test_mat(j,k)));
%     end
%     fprintf(fid,'\n');
% end
% fprintf(fid,']');
% fclose(fid);
% test_vec=test_vec.';
% fid2=fopen('data\test_vectorSTA_sin.txt','w+');
% fprintf(fid2,[num2str(size(test_vec,2)) 'x' num2str(size(test_vec,1))   ' \n ']);
% fprintf(fid2,'[');
% j=0;
% k=0;
% fin=num2str(size(test_vec,1))
% fin_plu=num2str(size(test_vec,2))
% for j=1:92
%     for k=1:300
%         fprintf(fid2,'(%e, %e)\t',real(test_vec(j,k)),imag(test_vec(j,k)));
%     end
%     fprintf(fid2,'\n');
% end
% test_vec=test_vec.';
% fprintf(fid2,']');
% fclose(fid2);

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
w2=steerv(px,py,phi2,theta2);
w3=steerv(px,py,phi3,theta3);
w4=steerv(px,py,phi4,theta4);
Vector_det=[w1,w2,w3,w4];


%Second part fo the data to Apply another PHI and theta for testing the Pastd Algorithm
%///////////////////////////////////////////////////////////////////////////////////////////////////////////////
%first source
phi5=-1.1802;    
theta5=0.6511;
t=0:0.001:0.3;
t=t(1:300);
f=50;
x=8000*sin(2*pi*f*t+0.3);
for i=100:length(x)
    test_vec1(i,:)=x(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi5)*sin(theta5)+py*sin(phi5)*sin(theta5))/Nfft); 
end

%second source
phi6=-2.3987;
theta6=0.8199;
t=0:0.001:0.3;
t=t(1:300);
f=80;
y=8200*sin(2*pi*f*t+0.2);
for i=100:length(x)
    test_vec2(i,:)=y(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi6)*sin(theta6)+py*sin(phi6)*sin(theta6))/Nfft); 
end

%third source
phi7=2.7455;
theta7=0.7143;
t=0:0.001:0.3;
t=t(1:300);
f=20;
z=8400*cos(2*pi*f*t+0.4);
for i=100:length(x)
    test_vec3(i,:)=z(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi7)*sin(theta7)+py*sin(phi7)*sin(theta7))/Nfft); 
end


%fourth source
phi8=-1.1563;       
theta8=0.3048;
t=0:0.001:0.3;
t=t(1:300);
f=15;
s=1000*sin(2*pi*f*t+0.6)+10000*sin(2*pi*f*2*t+0.3);
for i=100:300
    test_vec4(i,:)=s(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi8)*sin(theta8)+py*sin(phi8)*sin(theta8))/Nfft); 
end

test_vec=test_vec1+test_vec3+test_vec4+test_vec2;


%Pastd algorithm verification.
numsnaps=200;
BetaPASTd=0.95;
xq=test_vec(101:300,:);
tic
[W, d, snapcnt, vecconv] = pastdev(xq,EvalueSVD,EvectorSVD, numsnaps, 1, BetaPASTd, number_antenna);
figure(4)
plot(vecconv);
title('Convergence of the weigth after Pastd')
%fprintf('\t\tMDL- Pastd - Estimating the number of interfering sources\n');
%rfi_sources_Pastd=mdl(-(sort(-diag(d))),number_antenna,nsnsh);
toc
phi_look=0.3;
theta_look=0.3;
LookingDirection=steerv(px,py,phi_look,theta_look);
%phi_look_RFI=2.9554;
%theta_look_RFI=0.8813;
%RFI=steerv(px,py,phi_look_RFI,theta_look_RFI)
%EVector=[EVector,RFI];

%Apply the Projection for each technique
WeightVector = awe(EVector, Evalue, rfi_sources , length(px),LookingDirection,1);
WeightVector_SVD = awe(EvectorSVD, Evalue,rfi_sources_SVD, length(px),LookingDirection,1);
WeightVector_Pastd= awe(W, Evalue,rfi_sources, length(px),LookingDirection,1);
WeightVector_det=awe(Vector_det, Evalue,rfi_sources, length(px),LookingDirection,1);
    %WeightVector      = awe(Evector, Evalue, LookingDirection, rfi_sources, NumberOfAntennas,LookingDirection,useWeight);

    digwindow=1; 
    relfreq=1;

 % Generate Digital beamforming window 
 digwin = taper(px, py, digwindow, 128);
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
                
             pat_nulled_Pastd(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                    (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(WeightVector_Pastd.')';
         end;
     end;
 end



SteeredPattern = abs(pat_steered)/max(max(abs(pat_steered)));
BeamPattern = abs(pat_nulled)/max(max(abs(pat_nulled)));
BeamPattern_det = abs(pat_nulled_det)/max(max(abs(pat_nulled_det)));
BeamPattern_SVD = abs(pat_nulled_SVD)/max(max(abs(pat_nulled_SVD)));
BeamPattern_Pastd = abs(pat_nulled_Pastd)/max(max(abs(pat_nulled_Pastd)));

figure(5)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi2)*sin(theta2)/Nfft,sin(phi2)*sin(theta2)/Nfft,'*m')
plot(cos(phi3)*sin(theta3)/Nfft,sin(phi3)*sin(theta3)/Nfft,'*black')
plot(cos(phi4)*sin(theta4)/Nfft,sin(phi4)*sin(theta4)/Nfft,'*blue')
%plot(cos(phi_look_RFI)*sin(theta_look_RFI),sin(phi_look_RFI)*sin(theta_look_RFI),'*r')
title('EVD using the first hundred data') 
caxis([-70 0]);
colorbar 

figure(6)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_SVD));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi2)*sin(theta2)/Nfft,sin(phi2)*sin(theta2)/Nfft,'*r')
plot(cos(phi3)*sin(theta3)/Nfft,sin(phi3)*sin(theta3)/Nfft,'*r')
plot(cos(phi4)*sin(theta4)/Nfft,sin(phi4)*sin(theta4)/Nfft,'*r')
%plot(cos(phi_look_RFI)*sin(theta_look_RFI),sin(phi_look_RFI)*sin(theta_look_RFI),'*r')
title('SVD using the first hundred data') 
caxis([-70 0]);
colorbar 

figure(7)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_Pastd));
hold on
plot(cos(phi5)*sin(theta5)/Nfft,sin(phi5)*sin(theta5)/Nfft,'*r')
plot(cos(phi6)*sin(theta6)/Nfft,sin(phi6)*sin(theta6)/Nfft,'*r')
plot(cos(phi7)*sin(theta7)/Nfft,sin(phi7)*sin(theta7)/Nfft,'*r')
plot(cos(phi8)*sin(theta8)/Nfft,sin(phi8)*sin(theta8)/Nfft,'*r')
title('Pastd using 200 hundred snapshot xq(100:300) after having applied SVD or EVD to initialise the System') 
caxis([-70 0]);
colorbar 
hold off

figure(8)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(SteeredPattern));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*b')
plot(cos(phi2)*sin(theta2)/Nfft,sin(phi2)*sin(theta2)/Nfft,'*b')
plot(cos(phi3)*sin(theta3)/Nfft,sin(phi3)*sin(theta3)/Nfft,'*b')
plot(cos(phi4)*sin(theta4)/Nfft,sin(phi4)*sin(theta4)/Nfft,'*b')
hold off
title('Steered Pattern');
caxis([-70 0]);
colorbar 
toc

figure(9)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_det));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi2)*sin(theta2)/Nfft,sin(phi2)*sin(theta2)/Nfft,'*m')
plot(cos(phi3)*sin(theta3)/Nfft,sin(phi3)*sin(theta3)/Nfft,'*black')
plot(cos(phi4)*sin(theta4)/Nfft,sin(phi4)*sin(theta4)/Nfft,'*blue')
%plot(cos(phi_look_RFI)*sin(theta_look_RFI),sin(phi_look_RFI)*sin(theta_look_RFI),'*r')
title('Deterministic using the first hundred data') 
caxis([-70 0]);
colorbar 

%transpose to write in the file

% test_vec=test_vec.';
% EVector=EVector.';
% EvectorSVD=EvectorSVD.';
% W=W.';
% 
% %Write the test vector for Station_sim
% fid2=fopen('data\test_vectorSTA.txt','w+');
% fprintf(fid2,[num2str(size(test_vec,1)) 'x' num2str(size(test_vec,2))   ' \n ']);
% fprintf(fid2,'[');
% j=0;
% size(test_vec,2)
% size(test_vec,1)
% k=0;
% for j=1:size(test_vec,1)
%     for k=1:size(test_vec,2)
%         fprintf(fid2,'(%e, %e)\t',real(test_vec(j,k)),imag(test_vec(j,k)));
%     end
%     fprintf(fid2,'\n');
% end
% fprintf(fid2,']');
% fclose(fid2);
% 
% fid2=fopen('data\Result_EVD.txt','w+');
% fprintf(fid2,[num2str(size(EVector,1)) 'x' num2str(size(EVector,2))   ' \n ']);
% fprintf(fid2,'[');
% j=0;
% k=0;
% for j=1:size(EVector,1)
%     for k=1:size(EVector,2)
%         fprintf(fid2,'(%e, %e)\t',real(EVector(j,k)),imag(EVector(j,k)));
%     end
%     fprintf(fid2,'\n');
% end
% fprintf(fid2,']');
% fclose(fid2);
% 
% fid2=fopen('data\Result_SVD.txt','w+');
% fprintf(fid2,[num2str(size(EvectorSVD,1)) 'x' num2str(size(EvectorSVD,2))   ' \n ']);
% fprintf(fid2,'[');
% j=0;
% k=0;
% for j=1:size(EvectorSVD,2)
%     for k=1:size(EvectorSVD,1)
%         fprintf(fid2,'(%e, %e)\t',real(EvectorSVD(j,k)),imag(EvectorSVD(j,k)));
%     end
%     fprintf(fid2,'\n');
% end
% fprintf(fid2,']');
% fclose(fid2);
% 
% fid2=fopen('data\Result_Pastd.txt','w+');
% fprintf(fid2,[num2str(size(W,2)) 'x' num2str(size(W,1))   ' \n ']);
% fprintf(fid2,'[');
% j=0;
% k=0;
% for j=1:size(W,1)
%     for k=1:size(W,2)
%         fprintf(fid2,'(%e, %e)\t',real(W(j,k)),imag(W(j,k)));
%     end
%     fprintf(fid2,'\n');
% end
% fprintf(fid2,']');
% fclose(fid2);
% 
% 
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




save('data\Extraction.mat','theta1','theta2','theta3','theta4','phi1','phi2','phi3','phi4','BeamPattern','EvectorEVD','N');