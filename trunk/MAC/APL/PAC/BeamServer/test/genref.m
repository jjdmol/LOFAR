
%
% station local coordinates for 7 timesteps
% 7 x 3 matrix
%
lmn_t=[ 0 0 1; 0 1 0; 1 0 0; sin(pi/4) sin(pi/4) 0; sin(pi/4) 0 sin(pi/4); 0 sin(pi/4) sin(pi/4); sqrt(1/3) sqrt(1/3) sqrt(1/3) ]
lmn_t(4,:)
size(lmn_t)

%
% antenna locations 4 x 3 matrix
%
pos=[ 0 0 0; 0 0 1; 0 1 0; 1 0 0]
size(pos)

%
% create spectral window specification (2 subbands)
% starting at 10MHz
% 2 vector
%
freq=[0:1];
freq*=256e3;
freq+=10e6

%
% speed of light
%
c=299792458.0;

%
% calculate weights dimensions 7 timesteps, 4 signals, 2 subbands
%
M0 = zeros(7,4);
M0 = lmn_t(:,2) * pos(:,1)' - lmn_t(:,1) * pos(:,2)' - lmn_t(:,3) * pos(:,3)';
weights_1 = M0*exp(2*pi*i*freq(1)/c)
weights_2 = M0*exp(2*pi*i*freq(2)/c)
factor=2*pi*i/c;
weights_3 = M0*exp(factor*freq(1))
weights_4 = M0*exp(factor*freq(2))

sum(sum(weights_1 - weights_3))
sum(sum(weights_2 - weights_4))


