clear all
close all
load('data/antenna_config.mat');
long_file=500;
Nfft=256;
bandwidth=1000000;
total_time=long_file/bandwidth*Nfft;
%Trajectory generation
%mode 1
%disp('Trajectory Computation : Mode 1...');
% matrice(:,:,1)=[0,0,0;0.3,0.7,0.3;0.9,0.9,1];
% matrice(:,:,2)=[1,1,0;0.3,0.3,0.1;1.2,0.9,1];
% matrice(:,:,3)=[2.5,0.5,0;2.8,0.7,0.3;1.2,1.2,1];
% matrice(:,:,4)=[1.6,1.3,0;2.1,1.24,0.5;2.6,1.2,1];
% for i=1:4
% [phi_theta]=interpolation_traj2(squeeze(matrice(:,:,i)),Nfft,long_file);
% Phi_theta(:,:,i)=phi_theta;
% end

%mode 2
disp('Trajectory Computation : Mode 2...');
matrice(:,:,1)=[0,0, 0,0;...  
   0.05,0.01,5,0.1;...
   0.06,0.06,2,0.5;...
   0.07,0.04,3,1];
matrice(:,:,2)=[0.2,0.5, 5,0;...  
   0.5,0.6,3,0.5;...
   0.06,0.06,10,0.6;...
   0.5,0.04,1,1];
matrice(:,:,3)=[0 , 1, 8,0;...  
   1.5,1.5,0.2,0.1;...
   0.5,0.5,0.5,0.7;...
   0.07,0.04,0.30,1];
matrice(:,:,4)=[1.2,1.3, 0,0;...  
   0,0,3,0.2;...
   0.5,0.5,7,0.4;...
   0.45,0.45,2.5,1];
matrice(:,:,5)=[-0.7,0.7, 0,0;...  
   1,1,3,0.2;...
   0.5,0.5,0.35,0.4;...
   0.45,0.45,0.25,1];


for i=1:size(matrice,3);
phi_theta=interpolation(squeeze(matrice(:,:,i)),Nfft,total_time,bandwidth);
phi_theta=phi_theta(1:long_file,:);
Phi_theta(:,:,i)=phi_theta;
end


%plotting trajectory for each sources
tr=0:1/long_file:1;
tr=tr(1:long_file);
for i=1:4
    figure
    hold on
    plot(tr,squeeze(Phi_theta(:,1,i)),'r')
    plot(tr,squeeze(Phi_theta(:,2,i)),'b')
    legend('Phi','Theta')
    Ylabel('radian')
    title(['trajecory for source :' num2str(i)]);
    hold off
end


Nfft=1;
%Generating the sources the motion
disp('Generating the source : 1...');
step=1/long_file;
t=0:step:1;
%first source
f=50;
x=8000*sin(2*pi*f*t+0.3);
x=x(1:long_file);
for i=1:length(x)
    test_vec1(i,:)=x(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(Phi_theta(i,1,1))*sin(Phi_theta(i,2,1))+py*sin(Phi_theta(i,1,1))*sin(Phi_theta(i,2,1)))/Nfft); 
end

%second source
disp('Generating the source : 2...');
f=80;
y=5200*sin(2*pi*f*t+0.2);
y=y(1:long_file);
for i=1:length(x)
    test_vec2(i,:)=y(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(Phi_theta(i,1,2))*sin(Phi_theta(i,2,2))+py*sin(Phi_theta(i,1,2))*sin(Phi_theta(i,2,2)))/Nfft); 
end

%third source
disp('Generating the source : 3...');
f=20;
z=8400*cos(2*pi*f*t+0.4);
z=z(1:long_file);
for i=1:length(x)
    test_vec3(i,:)=z(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(Phi_theta(i,1,3))*sin(Phi_theta(i,2,3))+py*sin(Phi_theta(i,1,3))*sin(Phi_theta(i,2,3)))/Nfft); 
end

disp('Generating the source : 4...');
f=15;
s=1000*sin(2*pi*f*t+0.6)+10000*sin(2*pi*f*2*t+0.3);
s=s(1:long_file);
for i=1:length(x)
    test_vec4(i,:)=s(i)*exp(-1*sqrt(-1)*2*pi*(px*cos(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4))+py*sin(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4)))/Nfft); 
end

%Plotting the signals in the subband before addition
% figure(5)
% hold on
% plot(tr,abs(test_vec1),'b')
% plot(tr,abs(test_vec2),'r')
% plot(tr,abs(test_vec3),'m')
% plot(tr,abs(test_vec4),'black')
% title('Plotting the signals in the subband before addition')
% hold off

%Addition
test_vec=test_vec1+test_vec3+test_vec4+test_vec2;


%Verification EVD, SVD:
snap_length=100;
xq=test_vec(1:snap_length,:);
figure(6);
plot(abs(xq));
title('Time Series corresponding to the subband generated, first hundred samples'); 
number_antenna=length(px);

%EVD:
fprintf('ACM\n');
[ EvectorEVD, EvalueEVD,Cor ] = acm(xq.',number_antenna,snap_length, 0.2, 0.4, px, py, 1, 1);
[maxi,index]=max(diag(EvalueEVD));
nsnsh=size(xq,1);
fprintf('\t\tMDL- EVD - Estimating the number of interfering sources\n');
for i = 1:size(EvectorEVD,3);
    rfi_sources(i,:)=mdl(-sort(diag(-squeeze(EvalueEVD(:,:,i)))),number_antenna,nsnsh);
end
EVector=[];
for i=0:rfi_sources-1
EVector(:,i+1)=EvectorEVD(:,index-i);
Evalue=EvalueEVD(:,index-i);
end



%Single value deomposition verification
[EvectorSVD,EvalueSVD]=svd(xq.');
EvalueSVD=EvalueSVD-1;
sz=size(diag(EvalueSVD),1);
d=power(diag(EvalueSVD),2);
fprintf('\t\tMDL- SVD - Estimating the number of interfering sources\n');
rfi_sources_SVD=mdl(-sort(-d),number_antenna,nsnsh);

%rfi_sources=4;
%rfi_sources_SVD=4;
%plotting of the EigenValues using EVD, SVD
figure(7)
hold on
plot(-sort(-diag(EvalueEVD)),'*b');
plot(-sort(-d),'*r');
hold off
title('EigenValues using EVD, SVD')
legend('EigenValue using EVD','EigenValue using SVD');

 

%Projection and steering Vector
phi_look=squeeze(Phi_theta(1,1,5));
theta_look=squeeze(Phi_theta(1,2,5));
LookingDirection=steerv(px,py,phi_look,theta_look);
WeightVector = awe(EVector, 1, LookingDirection,rfi_sources , length(px),LookingDirection,1);
WeightVector_SVD = awe(EvectorSVD, 1, LookingDirection,rfi_sources_SVD, length(px),LookingDirection,1);


%Beampattern Obtention
N=100;
phi_theta=[];
phi_theta=[];
pat_steered=zeros(N,N);
pat_nulled=zeros(N,N);
pat_nulled_SVD=zeros(N,N);
relfreq=1;
i=0;
fin=N*N;
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

         end;
     end;
 end



SteeredPattern = abs(pat_steered)/max(max(abs(pat_steered)));
BeamPattern = abs(pat_nulled)/max(max(abs(pat_nulled)));
BeamPattern_SVD = abs(pat_nulled_SVD)/max(max(abs(pat_nulled_SVD)));


%Plotting of the trajectory for EVD during snap_length
figure(8)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
for i=1:snap_length
hold on
plot(cos(Phi_theta(i,2,1))*sin(Phi_theta(i,1,1))/Nfft,sin(Phi_theta(i,2,1))*sin(Phi_theta(i,1,1))/Nfft,'*b')
plot(cos(Phi_theta(i,1,2))*sin(Phi_theta(i,2,2))/Nfft,sin(Phi_theta(i,1,2))*sin(Phi_theta(i,2,2))/Nfft,'*r')
plot(cos(Phi_theta(i,1,3))*sin(Phi_theta(i,2,3))/Nfft,sin(Phi_theta(i,1,3))*sin(Phi_theta(i,1,3))/Nfft,'*m')
plot(cos(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4))/Nfft,sin(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4))/Nfft,'*black')
plot(cos(phi_look)*sin(theta_look),sin(phi_look)*sin(theta_look),'*w')
end
hold off
title('EVD') 
caxis([-70 0])
colorbar
mov1(1) = getframe(gcf);
%Plotting of the trajectory for SVD during snap_length
figure(9)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_SVD));
for i=1:snap_length
hold on
plot(cos(Phi_theta(i,2,1))*sin(Phi_theta(i,1,1))/Nfft,sin(Phi_theta(i,2,1))*sin(Phi_theta(i,1,1))/Nfft,'*b')
plot(cos(Phi_theta(i,1,2))*sin(Phi_theta(i,2,2))/Nfft,sin(Phi_theta(i,1,2))*sin(Phi_theta(i,2,2))/Nfft,'*r')
plot(cos(Phi_theta(i,1,3))*sin(Phi_theta(i,2,3))/Nfft,sin(Phi_theta(i,1,3))*sin(Phi_theta(i,1,3))/Nfft,'*m')
plot(cos(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4))/Nfft,sin(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4))/Nfft,'*black')
plot(cos(phi_look)*sin(theta_look),sin(phi_look)*sin(theta_look),'*w')
end
hold off
title('SVD')
caxis([-70 0])
colorbar

%Pastd algorithm verification.
numsnaps=200;
end_file=3   %long_file-(numsnaps+snap_length)
for t=1:end_file
BetaPASTd=0.95;
xq=test_vec(snap_length+t:numsnaps+snap_length+t,:);
tic
if t==1
[W, d, snapcnt, vecconv] = pastdev(xq,EvalueEVD,EvectorEVD, numsnaps, 1, BetaPASTd, number_antenna);
else
[W, d, snapcnt, vecconv] = pastdev(xq,EvalueEVD,W,numsnaps, 1, BetaPASTd, number_antenna);
end
figure(10)
plot(vecconv);
title('Convergence of the weigth after Pastd')
phi_look=squeeze(Phi_theta(t+1,1,5));
theta_look=squeeze(Phi_theta(t+1,2,5));
LookingDirection=steerv(px,py,phi_look,theta_look);
WeightVector_Pastd= awe(W, Evalue, LookingDirection,rfi_sources, length(px),LookingDirection,1);
t
N=100;
phi_theta=[];
phi_theta=[];
pat_steered=zeros(N,N);
pat_nulled=zeros(N,N);
pat_nulled_SVD=zeros(N,N);
relfreq=1;
i=0;
fin=N*N;
disp([' wait for ' num2str(fin) ]); 
 time = 0
  for u = 0:N
     u1 = -1 + 2*u/N;
     for v = 0:N
         v1 = -1 + 2*v/N;
         if (sqrt(u1^2+v1^2) <= sin(90/180*pi));
             i = i + 1;
             if mod(i,1000)==0
                 disp(['Status : ' num2str(i) ' - ' num2str(fin)]);
             end
             U(i) = u;
             V(i) = v;
             theta = asin(sqrt(u1^2+v1^2));
             phi = atan2(u1,v1);
             phi_theta=[phi_theta; phi theta];
             S{u+1,v+1}=[phi,theta];
                
             pat_nulled_Pastd(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                    (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(WeightVector_Pastd.')';
         end;
     end;
 end
 
BeamPattern_Pastd = abs(pat_nulled_Pastd)/max(max(abs(pat_nulled_Pastd)));
figure(9+t)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_Pastd));
hold on
j=t+100;
plot(cos(Phi_theta(j,2,1))*sin(Phi_theta(j,1,1))/Nfft,sin(Phi_theta(j,2,1))*sin(Phi_theta(j,1,1))/Nfft,'*b')
plot(cos(Phi_theta(j,1,2))*sin(Phi_theta(j,2,2))/Nfft,sin(Phi_theta(j,1,2))*sin(Phi_theta(j,2,2))/Nfft,'*r')
plot(cos(Phi_theta(j,1,3))*sin(Phi_theta(j,2,3))/Nfft,sin(Phi_theta(j,1,3))*sin(Phi_theta(j,1,3))/Nfft,'*m')
plot(cos(Phi_theta(j,1,4))*sin(Phi_theta(j,2,4))/Nfft,sin(Phi_theta(j,1,4))*sin(Phi_theta(j,2,4))/Nfft,'*black')
plot(cos(phi_look)*sin(theta_look),sin(phi_look)*sin(theta_look),'*w')
hold off
title('Pastd using amoving window of 100 snapshot after EVD')
caxis([-70 0])
colorbar
mov1(t+1) = getframe(gcf);
end
movie2avi(mov1,'C:\Documents and Settings\dromer\Desktop\Pattern_adapted')
close all

 