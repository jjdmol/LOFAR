%clear all
%close all
file_name='data/BF_real_15.txt';
file_name2='data/BF_complex_15.txt';
fid=fopen(file_name,'r');
fid2=fopen(file_name2,'r');
out_real=fscanf(fid,'%f');
out_complex=fscanf(fid2,'%f');
out=complex(out_real,out_complex);
i = 0;
U = 0;
V = 0;
s={};
ind=0;
N=180;
%n=5;%time to wait for in seconds
pat=[];
check=[];
% fol=[];
phi_theta=[];
 time = 0
  for u = 0:N
     u1 = (-1 + 2*u/N);
     for v = 0:N
         v1 = (-1 + 2*v/N);
         if (sqrt(u1^2+v1^2) <= sin(90/180*pi));
             i = i + 1;
             U(i) = u;
             V(i) = v;
             theta = asin(sqrt(u1^2+v1^2));
             phi= atan2(u1,v1);
             S{u+1,v+1}=[phi,theta];
             pat(u+1,v+1) =out(i); 
             %power=20*log10(abs(pat(u+1,v+1)));
             %phi_theta=[phi_theta; phi theta power];
         end;
     end;
end;

%BeamSignals = abs(pat)/max(max(abs(pat)));
BeamPattern=abs(pat);
BeamPattern_scaled = abs(pat)/max(max(abs(pat)));
figure
u=[0:N];
imagesc((-1 + 2*u/N),(-1 + 2*u/N),20*log10(BeamPattern));
hold on
plot(cos(0.2)*sin(0.4),sin(0.2)*sin(0.4),'*b')
plot(cos(0.1)*sin(0.1),sin(0.1)*sin(0.1),'*b')
hold off


fclose(fid);
fclose(fid2);