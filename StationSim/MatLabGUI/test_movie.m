close all 
clear all
dirpath = 'data';
number_antenna=92;

%Load trajectory and antenna configuration
load([dirpath '/antenna_signals.mat']);
[px,py]=create_lofar;
load([dirpath '/trajectory_phi_theta.mat']);
figure(20)
surf(20*log10(abs((squeeze(SelectedSubBandSignals(1,:,:))))));
shading interp
view(0,90)

%Select one subband and plot the first hundredd data selected for the EVD, SVD
subband=7;
x_data=SelectedSubBandSignals(:,:,subband).';
snap_length=100;
xq=x_data(1:snap_length,:);
figure(1)
plot(20*log10(abs(xq)));
title('Time Series corresponding to the subband generated, first hundred samples'); 

%Apply EigenValue decomposition
fprintf('ACM\n');
[ EvectorEVD, EvalueEVD,Cor ] = acm(xq.',number_antenna,snap_length, 0.2, 0.4, px, py, 1, 1,1/snap_length);
[maxi,index]=max(diag(EvalueEVD));
nsnsh=size(xq,1);
fprintf('\t\tMDL- EVD - Estimating the number of interfering sources\n');
for i = 1:size(EvectorEVD,3);
Evalues_int=-sort(diag(-squeeze(EvalueEVD(:,:,i))))
rfi_sources(i,:)=mdl(-sort(diag(-squeeze(EvalueEVD(:,:,i)))),number_antenna,nsnsh)
end
rfi_sources=20;
%[xq_test EvectorEVD(:,index)*(xq_test(1)/EvectorEVD(1,index))]
EVector=[];
for i=0:rfi_sources-1
EVector(:,i+1)=EvectorEVD(:,index-i);
%EVector(:,i+1)=(1/EvectorEVD(1,i+1))*EvectorEVD(:,i+1);
Evalue=EvalueEVD(:,index-i);
end
EVectorEVD=EVector;

%Single value deomposition verification
[EvectorSVD,EvalueSVD]=svd(xq.');
%EvalueSVD=EvalueSVD-1;
d=power(diag(EvalueSVD),2);
fid = fopen('values.txt','w')
G=[Evalues_int d];

for i=1:size(G,1)
    for j=1:size(G,2)
    fprintf(fid,'%g\t',G(i,j));
    end
    fprintf(fid,'\n');
end

fclose(fid)
fprintf('\t\tMDL- SVD - Estimating the number of interfering sources\n');
rfi_sources_SVD=mdl(-sort(-d),number_antenna,nsnsh);
rfi_sources_SVD=20;

figure(2)
hold on
plot(-sort(-diag(EvalueEVD)),'*b');
plot(-sort(-d),'*r');
hold off
title('EigenValues using EVD, SVD')
legend('EigenValue using EVD','EigenValue using SVD');

%Steering Vector and Projection
 phi_look=Phi_theta(snap_length,1,3);
 theta_look=Phi_theta(snap_length,2,3);
LookingDirection=steerv(px,py,Phi_theta(snap_length,1,3),Phi_theta(snap_length,2,3));
LookingDirection_vrai=LookingDirection;
%w=awe(vector, value, nrfi, nant,d,useWeight)
WeightVector = awe(EVector, EvalueEVD,rfi_sources, length(px),LookingDirection,1);
WeightVector_SVD = awe(EvectorSVD, EvalueSVD,rfi_sources_SVD, length(px),LookingDirection,1);

%Beampattern obtention
N=120;
phi_theta=[];
phi_theta=[];
pat_steered=zeros(N,N);
pat_nulled=zeros(N,N);
pat_nulled_SVD=zeros(N,N);
relfreq=1;
fin=N*N;
i=0;
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


%plot beampattern and trajectory corresponding to the angle input by the data generator  
figure(3)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
hold on
plot(cos(Phi_theta(1:snap_length,2,1)).*sin(Phi_theta(1:snap_length,1,1)),sin(Phi_theta(1:snap_length,2,1)).*sin(Phi_theta(1:snap_length,1,1)),'*b')
plot(cos(Phi_theta(1:snap_length,1,2)).*sin(Phi_theta(1:snap_length,2,2)),sin(Phi_theta(1:snap_length,1,2)).*sin(Phi_theta(1:snap_length,2,2)),'*r')
%plot(cos(Phi_theta(i,1,3))*sin(Phi_theta(i,2,3)),sin(Phi_theta(i,1,3))*sin(Phi_theta(i,1,3)),'*m')
%plot(cos(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4)),sin(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4)),'*black')
%plot(cos(phi_look_RFI)*sin(theta_look_RFI),sin(phi_look_RFI)*sin(theta_look_RFI),'*r')
title('EVD') 
caxis([-70 0])
colorbar

figure(4)
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_SVD));
for i=1:snap_length
hold on
plot(cos(Phi_theta(1:snap_length,2,1)).*sin(Phi_theta(1:snap_length,1,1)),sin(Phi_theta(1:snap_length,2,1)).*sin(Phi_theta(1:snap_length,1,1)),'*b')
plot(cos(Phi_theta(1:snap_length,1,2)).*sin(Phi_theta(1:snap_length,2,2)),sin(Phi_theta(1:snap_length,1,2)).*sin(Phi_theta(1:snap_length,2,2)),'*r')
%plot(cos(Phi_theta(i,1,3))*sin(Phi_theta(i,2,3)),sin(Phi_theta(i,1,3))*sin(Phi_theta(i,1,3)),'*m')
%plot(cos(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4)),sin(Phi_theta(i,1,4))*sin(Phi_theta(i,2,4)),'*black')
%plot(cos(phi_look_RFI)*sin(theta_look_RFI),sin(phi_look_RFI)*sin(theta_look_RFI),'*r')
end
title('SVD') 
caxis([-70 0])
colorbar

%Pastd algorithm verification.
numsnaps=100;
BetaPASTd=0.95;
t=0;
Moving_Window=100; 
Offset=100; %Start the Pastd algorithm at this index 
for ki=Offset+1:Offset+2%size(SelectedSubBandSignals,2)
xq_1=x_data(ki-Moving_Window:ki,:);
[W, ValuePastd, snapcnt, vecconv] = pastdev(xq_1,EvalueSVD,EvectorSVD, numsnaps, 1, BetaPASTd, number_antenna);
figure(5)
plot(vecconv);
title('Convergence of the weigth after Pastd')
%phi_look_RFI=2.9554;
%theta_look_RFI=0.8813;
%RFI=steerv(px,py,phi_look_RFI,theta_look_RFI)
%EVector=[EVector,RFI];
LookingDirection=steerv(px,py,Phi_theta(ki,1,3),Phi_theta(ki,2,3));

%Projection 
 WeightVector_Pastd= awe(W, EvalueSVD,rfi_sources, length(px),LookingDirection,1);
 digwindow=1; 
 relfreq=1;

 %Generate Digital beamforming window  
 digwin = taper(px, py, digwindow, 128);
 N=120;
 check=[];
 phi_theta=[];
 pat_steered=[];
 % pat_nulled=zeros(N,N,size(WeightVector,2)); 
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
                
             pat_nulled_Pastd(u+1,v+1) = exp(-1*sqrt(-1)*2*pi*relfreq* ...
                    (px*sin(theta)*cos(phi)+py*sin(phi)*sin(theta)))*(WeightVector_Pastd.')';
         end;
     end;
 end

SteeredPattern = abs(pat_steered)/max(max(abs(pat_steered)));
BeamPattern_Pastd = abs(pat_nulled_Pastd)/max(max(abs(pat_nulled_Pastd)));
if (mod(ki,2)==0)
t=t+1;
figure
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_Pastd));
hold on
plot(cos(Phi_theta(ki,2,1))*sin(Phi_theta(ki,1,1)),sin(Phi_theta(ki,2,1))*sin(Phi_theta(ki,1,1)),'*b')
plot(cos(Phi_theta(ki,1,2))*sin(Phi_theta(ki,2,2)),sin(Phi_theta(ki,1,2))*sin(Phi_theta(ki,2,2)),'*r')

% plot(cos(Phi_theta(snap_length/2+t,2,1))*sin(Phi_theta(snap_length/2+t,1,1)),sin(Phi_theta(snap_length/2+t,2,1))*sin(Phi_theta(snap_length/2+t,1,1)),'*b')
% plot(cos(Phi_theta(snap_length/2+t,1,2))*sin(Phi_theta(snap_length/2+t,2,2)),sin(Phi_theta(snap_length/2+t,1,2))*sin(Phi_theta(snap_length/2+t,2,2)),'*r')
%plot(cos(Phi_theta(snap_length/2+t,1,3))*sin(Phi_theta(snap_length/2+t,2,3)),sin(Phi_theta(snap_length/2+t,1,3))*sin(Phi_theta(snap_length/2+t,1,3)),'*m')
%plot(cos(Phi_theta(snap_length/2+t,1,4))*sin(Phi_theta(snap_length/2+t,2,4)),sin(Phi_theta(snap_length/2+t,1,4))*sin(Phi_theta(snap_length/2+t,2,4)),'*black')
title('Pastd') 
colorbar
caxis([-70 0])
hold off
mov1(t) = getframe(gcf);
plotspectrum3(SelectedSubBandSignals(:,ki-Moving_Window:ki,subband),px,py,Phi_theta(ki,1,3),Phi_theta(ki,2,3),WeightVector_Pastd);
mov2(t) = getframe(gcf);
end
% figure
% u=[0:N];(snap_length/2+t,2,3)),sin(Phi_theta(snap_length/2+t,1,3))*sin(Phi_theta(snap_length/2+t,1,3)),'*r')
% plot(cos(Phi_theta(snap_length/2+t,1,4))*sin(Phi_theta(snap_length/2+t,2,4)),sin(Phi_theta(snap_length/2+t,1,4))*sin(Phi_theta(snap_length/2+t,2,4)),'*r')
% hold off
% title('Steered Pattern');

%mov2(i) = getframe(gcf);
end
movie2avi(mov1,'C:\Documents and Settings\dromer\Desktop\Pattern_adapted_AM')
movie2avi(mov2,'C:\Documents and Settings\dromer\Desktop\Subband_adapted_AM')
%movie2avi(mov2,'PatternSteered_trajectory')
%for i=1:99
 %   close figure(i)
 %end
Phi_theta_verif=Phi_theta;
 save('data/weigth_vector.mat','WeightVector','WeightVector_SVD','px','py','LookingDirection_vrai','phi_look','theta_look','WeightVector_Pastd','Phi_theta_verif','xq','EvectorEVD','EvalueEVD');