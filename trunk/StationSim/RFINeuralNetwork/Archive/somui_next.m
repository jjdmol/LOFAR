function next_free = somui_next
%SOMUI_NEXT returns the next free # for naming SOMfig#
%
%  next_free = somui_next()
%
% RETURNS
%
%   next_free  (char)  Next free number for the name of SOMfig

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 


all_somfigs = findobj('tag','somui_fig');
numb_somfigs = length(all_somfigs);
if (numb_somfigs == 0)
  next_free = num2str(1);             %% 'SOMfig1'
else
  sfnames = get(all_somfigs,'name');
  sfnames = cellstr(sfnames);         %% if not converted, falls in case of 1
  for i=1:numb_somfigs          
    tmp = char(sfnames{i}(7:end));    %% remove SOMfig from 'name'
    tmp = str2num(tmp);
    sfnumbers(i,1) = tmp;
  end
  next_free = max(sfnumbers) + 1;     %% doesn't use holes
  next_free = num2str(next_free);
end;
