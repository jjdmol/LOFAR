function [Phi_theta]=interpolation_traj2(A,Nfft,long_file)
interp_step=long_file;
[lig,col]=size(A);
Phi=[];
theta=0;
phi=0;
Theta=[];
for i=1:lig-1
Step(i)=(A(i+1,3)-A(i,3))*interp_step;
phi=A(i,1):(A(i+1,1)-A(i,1))/Step(i):A(i+1,1);
theta=A(i,2):(A(i+1,2)-A(i,2))/Step(i):A(i+1,2);
Phi=[Phi phi];
Theta=[Theta theta];
end
Phi_theta=[Phi.' Theta.'];
Phi_theta=Phi_theta(1:long_file,:);