function w = taper( x, y, tapertype, npoints )
%
% w = taper( x, y, tapertype )
%
%  makes a 2D radial taper out of a 1D taper
%      x                - x-location of the elements
%      y                - y-location of the elements
%      tapername        - name of taper (BARTLETT, BLACKMAN, BOXCAR, CHEBWIN, HANNING, KAISER, or TRIANG)
%      npoints          - number of points on the taper
%

switch tapertype
   case 1,  wt = boxcar(npoints);
   case 2,  wt = bartlett(npoints);
   case 3,  wt = blackman(npoints);
   case 4,  wt = chebwin(npoints,20);
   case 5,  wt = chebwin(npoints,40);
   case 6,  wt = hamming(npoints);
   case 7,  wt = hanning(npoints);
   case 8,  wt = kaiser(npoints,3);
   case 9,  wt = kaiser(npoints,10);
   case 10, wt = triang(npoints);
end

r = sqrt( x.^2 + y.^2 );  % weight depends on distance from centre of array
r = r/max(r);
r = round( npoints/2 * r); % finite points on grid
w = wt(npoints/2 + r);