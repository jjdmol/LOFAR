function [x,y]=create_lofar 

%LOFAR array of 7 circles with logarithmic radius 
   params = arrayparams(20);
   ne = params(1,5);
   angoff = 0;

   ci = params(1,1);   % initial spacing
   cf = params(1,2);   % final spacing
   M = 7;
   f = 10^(log10(cf/ci)/(M-2));
   xloc = ci;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end

   x = [0];
   y = [0];
   angoff = [0:24/M:24]*params(2,1);
  
   for i = 1:M
      x = [x xloc(i)*sin(([0:360/ne:360-360/ne]+angoff(i))*pi/180)];
      y = [y xloc(i)*cos(([0:360/ne:360-360/ne]+angoff(i))*pi/180)];
   end
   