%clear all
%close all
N=180;
i = 0;
U = 0;
V = 0;
 %n=5;%time to wait for in seconds
dirpath = 'data';
NumberOfAntennas=92;
load([dirpath '/antenna_config.mat']);
 S={};
phase=[];
check=[];
% fol=[];
pow=[];
steervect=steerv(px,py,0.2,0.4);
steerRFI=steerv(px,py,2.5830,0.8238);
steerRFI1=steerv(px,py,-2.9709,0.7128);
RFI=[steerRFI,steerRFI1];
Evalue=0;
phi_theta=[];
%WeightVector      = awe(Evector, Evalue, LookingDirection, rfi_sources, NumberOfAntennas,LookingDirection,useWeight);
w=awe(RFI, Evalue, steervect, 2, NumberOfAntennas,steervect,1);
 time = 0
  for u = 0:N
     u1 = -1 + 2*u/N;
     for v = 0:N
         v1 = -1 + 2*v/N;
         if (sqrt(u1^2+v1^2) <= sin(90/180*pi));
             i = i + 1;
            % disp(['loop : ' num2str(i) '-' num2str(N*N) ' ' ]); 
             U(i) = u;
             V(i) = v;
             theta = asin(sqrt(u1^2+v1^2));
             phi = atan2(u1,v1);
             S{u+1,v+1}=[phi,theta];
             phase(u+1,v+1)=exp(-1*sqrt(-1)*2*pi* ...
                      (px*cos(phi)*sin(theta)+py*sin(phi)*sin(theta)))*(steervect.')';
             phase_nulled(u+1,v+1)=exp(-1*sqrt(-1)*2*pi* ...
                      (px*cos(phi)*sin(theta)+py*sin(phi)*sin(theta)))*(w.')';
             %power=20*log10(abs(sum(phase(i,:))));
             %pow(u+1,v+1)=power;
             %phi_theta=[phi_theta;phi theta];
         end;
     end;
end;
u=[0:N];
BeamPattern1=[];
BeamPattern1=abs(phase);
BeamPattern1_nulled=abs(phase_nulled);
BeamPattern_scaled1 = abs(phase)/max(max(abs(phase)));
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern_scaled1+0.5));
figure
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern1_nulled+0.5));
hold on
plot(cos(0.2)*sin(0.4),sin(0.2)*sin(0.4),'*r')
plot(cos(0.1)*sin(0.1),sin(0.1)*sin(0.1),'*r')
hold off
% % phase=phase.';
% % fid=fopen('data\phi_theta.txt','w+');
% % fprintf(fid,[num2str(size(phi_theta,2)) ' ' num2str(size(phi_theta,1))   ' \n ']);
% % fprintf(fid,'[');
% % fprintf(fid,'%e %e\n',phi_theta.');
% % fprintf(fid,']');
% % fclose(fid);

% fid=fopen('data\test_vectorEssai.txt','w+');
% fprintf(fid,[num2str(size(phase,2)) 'x' num2str(size(phase,1))   ' \n ']);
% fprintf(fid,'[');
% j=0
% fin=num2str(size(phase,1))
% fin_plu=num2str(size(phase,2))
% for j=1:92
%     for k=1:25445
%         fprintf(fid,'(%e, %e)\t',real(phase(j,k)),imag(phase(j,k)));
%     end
%     fprintf(fid,'\n');
% end
% fprintf(fid,']');
% fclose(fid);



