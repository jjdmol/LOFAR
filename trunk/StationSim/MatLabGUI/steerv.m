%
% steer in a direction
%
function a = steerv(px,py,phi,theta)

N1 = size(px,2);
N2 = size(py,2);
%a = zeros(N1,N2);
a = zeros(N1,1);

for n=1:N1
 %  for m=1:N2
      a(n) = exp(-j*2*pi*(px(n)*sin(phi)*cos(theta)+py(n)*sin(theta)));
 %  end
end 

return;