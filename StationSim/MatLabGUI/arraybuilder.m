function [x, y] = arraybuilder(arraytype, params)
%
%  Function which builds a particular type of array
%
% params(1,x) % size
% params(2,x) % angle
%

switch arraytype
   case 1
   %
   % Compact THEA  
   %
%if 0
   sc=4;
   x = [ 0:1:3 ];  
   x = [ x x x x ]*sc;
   y = ones(1,4);
   y = [ 0*y 1*y 2*y 3*y ]*sc;
   x = x-6;  % centre the array
   y = y-6;
   %else
%x = [0 4 2.5 3 3.5 4 4.5 5 5.5 0   0   0 0   0 0   0];
%y = [0 4 0   0   0 0   0 0   0 2.5 3 3.5 4 4.5 5 5.5];

%x = [0 0.5 1 1.5 2 2.5 3 0   0 0   0 0   0 0 6 6];
%y = [0   0 0   0 0   0 0 2 2.5 3 3.5 4 4.5 5 4 0];
%  end

   case 2
   %
   % Compact THEA, but now with triangular lattice structure
   %
   sc=4;
   x = [ 0:1:3 ];  
   x = [ x 0.5:1:3.5 ];  
   x = [ x 0:1:3 ];  
   x = [ x 0.5:1:3.5 ]*sc;  
   y = ones(1,4);
   y = [ 0*y 1*y 2*y 3*y ]*sc;
   x = x-6;
   y = y-6;

   xb = 4*[0:3];
   x = [xb xb+.5 xb+1 xb+1.5];
   x = [xb xb+1 xb+2 xb+3];
   x = [xb xb+2 xb+4 xb+6];
   yb = 4*[0:-.5:-1.5];
   y = [yb yb+4 yb+8 yb+12];

   case 3
   %
   % 16 element array in one ring 
   %
   ne = params(1,5);
   angoff = params(2,1);
   ringsize = params(1,1);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);

   case 4
      %
      % 16 element array with two rings of 11 and 5
      %
      ne = 5;
      angoff = params(2,2);
      ringsize = params(1,2);
      x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);
      y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);
      ne = 11;
      angoff = params(2,1);  
      ringsize = params(1,1);
      x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];
      y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];

   case 5
   %
   % 16 element array with two rings of 8 elements 
   %
   ne = params(1,5);
   angoff = params(2,2);
   ringsize = params(1,2);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);
   angoff = params(2,1); 
   ringsize = params(1,1);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];

   case 6
   %
   % 16 element array with two rings of 9 and 7
   %
   ne = 7;
   angoff = params(2,2);
   ringsize = params(1,2);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   ne = 9;
   angoff = params(2,1); 
   ringsize = params(1,1);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];

   case 7
   %
   % 16 element array with four rings of 7, 5, 3, 1
   %
   ne = 1;
   angoff = params(2,4);
   ringsize = params(1,4);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   ne = 3;
   angoff = params(2,3); 
   ringsize = params(1,3);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   ne = 5;
   angoff = params(2,2);
   ringsize = params(1,2);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   ne = 7;
   angoff = params(2,1); 
   ringsize = params(1,1);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];


   case 8
   %
   % 16 element array with three rings of 11, 4, 1
   %
   ne = 1;
   ne = 25;
   angoff = params(2,3);
   ringsize = params(1,3);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
 %  ne = 4;
   angoff = params(2,2); 
   ringsize = params(1,2);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
 %  ne = 11;
   angoff = params(2,1);
   ringsize = params(1,1);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];

   case 9
   %
   % Y shaped array (VLA)
   %
   ne = 1;
   angoff = 0;
   ringsize = 0;
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   ne = 3;
   angoff = params(2,5);
   ringsize = params(1,5);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,4);
   ringsize = params(1,4);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,3);
   ringsize = params(1,3);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,2);
   ringsize = params(1,2);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,1);
   ringsize = params(1,1);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];

   case 10
   %
   % Spiral shaped array (3 arms)
   %
   ne = 1;
   angoff = 0;
   ringsize = 0;
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   ne = 3;
   angoff = params(2,5);
   ringsize = params(1,5);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,4);
   ringsize = params(1,4);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,3);
   ringsize = params(1,3);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,2);
   ringsize = params(1,2);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,1);
   ringsize = params(1,1);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];


   ne = 5;
   angoff = params(2,5);
   ringsize = params(1,5);
   x = [0 ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [0.5 ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,4);
   ringsize = params(1,4);
%   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
%   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,3);
   ringsize = params(1,3);
%   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
%   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];

   x = [0  0   -3    3  -5 5]
   y = [.5 5 -4.5 -4.5  1  1]

   x = [0  0   -3    3  -5 5]
   y = [.5 5 -4.5 -4.0  1.5  1]

   x = [0  0   -3    3  -5 5]
   y = [.5 5 -4.0 -3.5  1.5  1]

   x = [0  0   -3    3  -4.5 4.5]
   y = [.5 5 -4.0 -3.5  1.5  1]


   case 11
   %
   % Spiral shaped array (4 arms)
   %
   ne = 4;
   angoff = params(2,4);
   ringsize = params(1,4);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180); 
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);
   angoff = params(2,3);
   ringsize = params(1,3);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,2);
   ringsize = params(1,2);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   angoff = params(2,1);
   ringsize = params(1,1);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];

   case 12
   %
   % logarithmic array (4x4)
   %
   ci = params(1,1);   % initial spacing
   cf = params(1,2);   % final spacing
   M = 4;
   f = 10^(log10(cf/ci)/(M-2));
   xloc(1) = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   x = [xloc xloc xloc xloc];
   y = [xloc(4)*ones(1,4) xloc(3)*ones(1,4) xloc(2)*ones(1,4) xloc(1)*ones(1,4)];

   case 13
   %
   % logarithmic circle
   %
   ci = params(2,1);   % initial spacing
   cf = params(2,2);   % final spacing
   M = params(1,5);    % number of elements
   f = 10^(log10(cf/ci)/(M-2));
   xloc(1) = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   x = params(1,1)*sin(xloc*pi/180);
   y = params(1,1)*cos(xloc*pi/180);

   case 14
   %
   % logarithmic spiral
   %
   ci = params(1,1);   % initial spacing
   cf = params(1,2);   % final spacing
   M = 16;
   f = 10^(log10(cf/ci)/(M-2));
   xloc(1) = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   xloc = xloc+params(1,3);
   rotang = (0:params(2,1):15*params(2,1))*pi/180;
   x = xloc.*sin(rotang);
   y = xloc.*cos(rotang);

   case 15
   %
   % two logarithmic circle
   %
   ci = params(2,1);   % initial spacing
   cf = params(2,2);   % final spacing
   M = 9;
   f = 10^(log10(cf/ci)/(M-2));
   xloc = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   xloc = xloc + params(2,5);
   x = params(1,1)*sin(xloc*pi/180);
   y = params(1,1)*cos(xloc*pi/180);

   ci = params(2,3);   % initial spacing
   cf = params(2,4);   % final spacing
   M = 7;
   f = 10^(log10(cf/ci)/(M-2));
   xloc = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   x = [x params(1,2)*sin(xloc*pi/180)];
   y = [y params(1,2)*cos(xloc*pi/180)];

   case 16
   %
   % three logarithmic circle
   %
   ci = params(2,1);   % initial spacing
   cf = params(2,2);   % final spacing
   M = 9;
   f = 10^(log10(cf/ci)/(M-2));
   xloc = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   xloc = xloc + params(2,5);
   x = params(1,1)*sin(xloc*pi/180);
   y = params(1,1)*cos(xloc*pi/180);

   ci = params(2,3);   % initial spacing
   cf = params(2,4);   % final spacing
   M = 6;
   f = 10^(log10(cf/ci)/(M-2));
   xloc = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   x = [x params(1,2)*sin(xloc*pi/180)];
   y = [y params(1,2)*cos(xloc*pi/180)];

   x = [x .3];  % center element
   y = [y .3];

   case 17
   ne = 5;
   angoff = params(2,1);
   ringsize = params(1,1);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring

   angoff = params(2,2);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring

   angoff = params(2,3);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring

   ne = 3;
   ringsize = params(1,2);
   xr = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   yr = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring

   x = [x+xr(1) x+xr(2) x+xr(3)];
   y = [y+yr(1) y+yr(2) y+yr(3)];

   case 18
   %
   % THEA Tile
   %
   sc=.5;
   xx = [ 0:1:7 ];  
   x = [ xx xx xx xx xx xx xx xx ]*sc;
   yy = ones(1,8);
   y = [ 0*yy 1*yy 2*yy 3*yy 4*yy 5*yy 6*yy 7*yy]*sc;


   xx = [ 0:1:2];  
   x = [ xx xx xx ]*sc;
   yy = ones(1,3)
   y = [ 0*yy 1*yy 2*yy ]*sc;
   x = x - .5;
   y = y - .5;

   xx = [ 0:1:4]; 
   x = [ xx xx xx xx xx ]*sc;
   yy = ones(1,5);
   y = [ 0*yy 1*yy 2*yy 3*yy 4*yy ]*sc;
   x = x - 1;
   y = y - 1;


   xx = [ 0:1:3 ];  
   x = [ xx xx xx xx ]*sc;
   yy = ones(1,4);
   y = [ 0*yy 1*yy 2*yy 3*yy ]*sc;
   x = x - .75;
   y = y - .75;


   case 19
   %
   % LOFAR array of 5 circles with logarithmic radius 
   %
   ne = params(1,5);
   angoff = 0;

   ci = params(1,1);   % initial spacing
   cf = params(1,2);   % final spacing
   M = 5;
   f = 10^(log10(cf/ci)/(M-2));
   xloc = ci;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end

   x = [];
   y = [];
   angoff = [0:24/5:24]*0;
   for i = 1:M
      x = [x xloc(i)*sin(([0:360/ne:360-360/ne]+angoff(i))*pi/180)];
      y = [y xloc(i)*cos(([0:360/ne:360-360/ne]+angoff(i))*pi/180)];
   end

   
   case 20
   %
   % LOFAR array of 7 circles with logarithmic radius 
   %
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
   
   case 21
   %
   % logarithmic circle
   %
   ci = params(2,1);   % initial spacing
   cf = params(2,2);   % final spacing
   M = params(1,5);    % number of elements
   f = 10^(log10(cf/ci)/(M-2));
   xloc(1) = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   xr = params(1,1)*sin(xloc*pi/180);
   yr = params(1,1)*cos(xloc*pi/180);

   ci = params(2,3);   % initial spacing
   cf = params(2,4);   % final spacing
   M = params(1,4);
   f = 10^(log10(cf/ci)/(M-2));
   xloc = 0;  % initial location
   for m=1:M-1
      xloc(m+1) = xloc(m) + ci*f^(m-1);
   end
   xr = [xr params(1,2)*sin((params(2,5)+xloc)*pi/180)];
   yr = [yr params(1,2)*cos((params(2,5)+xloc)*pi/180)];


   %
   % Triangular lattice in middle
   %
   xb = 4*[0:5];
   x = [xb xb-.5 xb-1 xb-1.5 xb-2 xb-2.5];
   yb = [0:.5:2.5];
   y = [yb yb+4 yb+8 yb+12 yb+16 yb+20];

   x(1) = NaN;
   x(6) = NaN;
   x(36) = NaN;
   x(31) = NaN;
   xx = [];
   yy = [];
   for i = 1:length(x)
      if isfinite(x(i)) 
         xx = [xx x(i)];
         yy = [yy y(i)];
      end
   end
   x = [xx-8.75 xr];
   y = [yy-11.25 yr];

   case 22

   ne = 7;
   angoff = params(2,4);
   ringsize = params(1,4);
   x = ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   y = ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180);  % inner ring
   ne = 13;
   angoff = params(2,3); 
   ringsize = params(1,3);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   ne = 19;
   angoff = params(2,2);
   ringsize = params(1,2);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];
   ne = 25;
   angoff = params(2,1); 
   ringsize = params(1,1);
   x = [x ringsize*sin(([0:360/ne:360-360/ne]+angoff)*pi/180)];       % outer ring
   y = [y ringsize*cos(([0:360/ne:360-360/ne]+angoff)*pi/180)];

   case 23
       x = [0.1];
       y = [0.1];
end


