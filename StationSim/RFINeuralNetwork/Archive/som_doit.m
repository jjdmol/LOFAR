function [sMap,sData] = som(sData)

%SOM Initialize, train, label and visualize a self-organizing map.
%
% sMap = som(sData)
%
% ARGUMENTS 
%
%  sData  (struct or matrix) data structure or data matrix 
%
% RETURNS
%
%  sMap   (struct) trained self-organized map
%  sData  (struct) normalized data structure
%
% This is the ultimate user-friendly function that initializes, 
% trains, labels and finally visualizes a SOM based on the given data set.
% The function constitutes of function calls to other basic routines
% of the toolbox performed in the usual order. No parameters other 
% than the data set are given.
%
%  [sMap,sData] = som(D)
%
% See also SOM_INIT, SOM_TRAIN, SOM_AUTOLABEL, SOM_SHOW, 
% SOM_PROJECTION, SOM_SHOWGRID.

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta juuso 191297

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Check arguments and initialize

error(nargchk(1, 1, nargin));  % check the number of input arguments

if ~isstruct(sData), 
  sData = som_data_struct(sData); 
  nolabels = 1; 
else
  [llen nofl] = size(sData.labels);
  nolabels = 1; i=1; 
  while i<=llen & nolabels, 
    if ~isempty(sData.labels{i}), 
       j=1; 
       while j<=nofl & nolabels, 
         nolabels = isempty(sData.labels{i,j}); j=j+1;
       end
    end
    i = i+1;
  end
end
[dlen dim] = size(sData.data);

som_info(sData);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Action

% preprocessing (normalization)
if isempty(sData.normalization.inv_params), 
  fprintf(1,'[ Preprocessing ]\n');  
  sData = som_normalize_data(sData);
end

% initialization
fprintf(1,'[ Initializing ]\n');   
sMap = som_init(sData);        

% training
sMap = som_train(sMap,sData);  

% labeling
if ~nolabels,                  
  fprintf(1,'[ Labeling ]\n');
  sMap = som_autolabel(sMap, sData, 'freq');
end

% data projection
fprintf(1,'[ Map projection ]\n');
if dim>=3, odim=3; else odim=dim; end
P = som_projection(sMap,odim,'pca');
P = som_projection(sMap,P,'cca',5);

% visualization: u-matrix, component planes, hits and labels
fprintf(1,'[ Map visualization ]\n');
colormap('hot')                
h = som_show(sMap,[0 1:dim -2],'denormalized'); 
set(h.label(dim+2),'String','Hit histogram')
som_addhits(sMap,sData,dim+2,'spot','red') 
if ~nolabels, som_addlabels(sMap,1,dim+2,'','black'); end
figure
set(gcf,'Name',['Projection of ' sMap.name ' to ' odim 'D']);
som_showgrid(sMap,P,'surf',sMap.codebook(:,:,1))
title('colors according to first component plane')
colormap('hot')                
figure 
set(gcf,'Name',['Profiles of weight vectors (denormalized) of ' sMap.name]);
som_profile(sMap,'PLOT_AXIS_OFF','denormalized')

% info
som_info(sMap); 
fprintf(1,'Average quantization error: %e\n',som_quality(sMap, 'qe', sData));
fprintf(1,'Topographic error:          %4.2f%%\n',100*som_quality(sMap, 'topog', sData));

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
