clear all
close all

%Settings
dirpath = 'data';
number_antenna=92;
load([dirpath '/antenna_config.mat']);
Nfft=1;

%first source
phi1=-1.2847;    
theta1=0.6239;
t=0:0.001:0.3;
t=t(1:100);
f=50;
x=8000*sin(2*pi*f*t+0.3);
for i=1:length(x)
    test_vec1(i,:)=x(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi1)*sin(theta1)+py*sin(phi1)*sin(theta1))/Nfft); 
end


%second source
phi2=-2.3362;
theta2=0.89;
t=0:0.001:0.3;
t=t(1:100);
f=80;
y=5200*sin(2*pi*f*t+0.2);
for i=1:length(x)
    test_vec2(i,:)=y(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(phi2)*sin(theta2)+py*sin(phi2)*sin(theta2))/Nfft); 
end

%Addition of the 2 signals
test_vec=test_vec1+test_vec2;


xq=test_vec(1:100,:);
figure(1)
plot(abs(xq))
title('Time Series corresponding to the subband generated, first hundred samples'); 
te=size(xq,2);
%i=sqrt(-1);
%xq_test=[1+i,2-i,3*i,4].';
%xq=repmat(xq_test,1,100);

%Apply EVD
fprintf('ACM\n');
[ EvectorEVD, EvalueEVD,Cor ] = acm(xq.',number_antenna,te, 0.2, 0.4, px, py, 1, 1);
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
%Plotting the EigenVlaue of the 2 techniques EVD, SVD 
figure(2)
hold on
plot(-sort(-diag(EvalueEVD)),'*b');
legend('EigenValue using EVD');

%Input the steering direction
phi_look=0.3;
theta_look=0.3;
LookingDirection=steerv(px,py,phi_look,theta_look);
%Projection
WeightVector = awe(EVector, Evalue, LookingDirection,rfi_sources , length(px),LookingDirection,1);

 

 % Generate Digital beamforming window   
 digwindow=1; 
 relfreq=1;
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

         end;
     end;
 end



SteeredPattern = abs(pat_steered)/max(max(abs(pat_steered)));
BeamPattern = abs(pat_nulled)/max(max(abs(pat_nulled)));

figure(3)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*r')
plot(cos(phi2)*sin(theta2)/Nfft,sin(phi2)*sin(theta2)/Nfft,'*r')
%plot(cos(phi_look_RFI)*sin(theta_look_RFI),sin(phi_look_RFI)*sin(theta_look_RFI),'*r')
title('EVD using the first hundred data') 
caxis([-70 0]);
colorbar 


figure(4)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(SteeredPattern));
hold on
plot(cos(phi1)*sin(theta1)/Nfft,sin(phi1)*sin(theta1)/Nfft,'*b')
plot(cos(phi2)*sin(theta2)/Nfft,sin(phi2)*sin(theta2)/Nfft,'*b')
hold off
title('Steered Pattern');
caxis([-70 0]);
colorbar 
save('data\Extraction_2source.mat','theta1','phi1','theta2','phi2','BeamPattern','EvectorEVD','N','Nfft');



