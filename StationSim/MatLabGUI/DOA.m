function a = DOA(px,py,Phi_theta,Nfft)
% computes the direction of arrival of a signal
N1 = size(px,2);
N2 = size(py,2);
%a = zeros(N1,N2);
for n=1:N1
   %for m=1:N2
   %a(n) = exp(-j*2*pi*(px(n)*sin(phi)*cos(theta)+py(n)*sin(theta))); 
   %a(n) = -j*2*pi*(px(n)*sin(phi)*cos(theta)+py(n)*sin(theta)); 
   a(:,n) = (-2*pi*(px(n).* sin(Phi_theta(:,2)).* cos(Phi_theta(:,1)) + py(n).* sin(Phi_theta(:,2)).* sin(Phi_theta(:,1))));
   %end
end 
return;