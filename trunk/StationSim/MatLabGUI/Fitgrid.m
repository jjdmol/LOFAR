function [x_new,y_new,x_grid, y_grid, null_mask] = fitgrid( x, y, n_gridpoints, null_fill )

%
% [ x_new y_new ] = fitgrid( x, y, n_gridpoints )
%
%  fits a set of points (x,y) on a grid of n_points by n_points
%
%      x              - array with x-coordinates
%      y              - array with y coordinates
%      n_gridpoints   - number of points on grid in one direction
% 
%  W.Cazemier, G.Hampson 7/8/2000

% Adapted to support 'null filling'. An experiment to evaluate fft
% beamforming for use in LOFAR
% P.C. Broekema, july 2002

max_x = max(x);
min_x = min(x);

max_y = max(y);
min_y = min(y);


% transform to the range 0 - (n_gridpoints-1)
if (max_x-min_x ~= 0)
    x_new = (x - min_x) * (n_gridpoints-1) / (max_x - min_x);
else 
    x_new = min_x;
end;

if (max_y - min_y ~= 0)
    y_new = (y - min_y) * (n_gridpoints-1) / (max_y - min_y);
else 
    y_new = min_y;
end;

% round to integer points

x_new = round(x_new);
y_new = round(y_new);

null_mask=ones(1,size(x_new,2));
fprintf('2\n');
if null_fill
    % since matrix can't be indexed from 0, antennas are numbered from 1
    % null_mask is an array which indicates the _real_ antennas
    null_mask = zeros(1, n_gridpoints^2);
    if (size(x_new,2) > size(y_new,2))
        guard = size(y_new,2);
    else
        guard = size(x_new,2);
    end;
    for i = 1:guard
        index = x_new(i) + (y_new(i) * n_gridpoints) + 1;
        null_mask(index) = 1;
    end;

    % now we generate a new x_new and y_new array containing all gridpoints as antennas
    sparse_range = n_gridpoints^2;
    x_temp = zeros(1,sparse_range);
    y_temp = zeros(1,sparse_range);
    for i = 1:sparse_range
        x_temp(i) = mod(i-2, n_gridpoints);
        y_temp(i) = (i-2)/n_gridpoints;
    end;
    x_new=x_temp;
    y_new=y_temp;
end;

% transform back
x_new =  (max_x - min_x) / (n_gridpoints-1) * x_new + min_x;
y_new =  (max_y - min_y) / (n_gridpoints-1) * y_new + min_y;

x_grid = (max_x - min_x) / (n_gridpoints-1);
y_grid = (max_y - min_y) / (n_gridpoints-1);