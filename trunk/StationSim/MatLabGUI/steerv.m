%
% steer in a direction
%
function a = steerv(px,py,phi,theta)

N1 = size(px,2);
N2 = size(py,2);
%a = zeros(N1,N2);
a = zeros(N1,1);

for n=1:N1
   %for m=1:N2
      a(n) = exp(-j*2*pi*(px(n)*sin(theta)*cos(phi)+py(n)*sin(theta)*sin(phi))); 
   %end
end 

return;