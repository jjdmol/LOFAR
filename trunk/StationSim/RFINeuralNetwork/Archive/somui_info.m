function somui_info(arg1)
%SOMUI_INFO gives information on map or data struct
%
%  somui_info(arg1)
%
% ARGUMENTS:
%
%  arg1  (struct)  map or data
%
% SOMUI_INFO(ARG1) gives information on map or data struct
% This function is called from uimenus of somui_it and somui_vis.
% Similar function in SOM package is SOM_INFO
%
% Information is displayed in a window
%
% See also SOM_INFO, SOMUI_FIG, SOMUI_VIS

% Copyright (c) 1997 by the SOM toolbox programming team.
% http://www.cis.hut.fi/projects/somtoolbox/

% Version 1.0beta Jukka 071197 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

global mapstring;
global datastring;

error(nargchk(0, 1, nargin))  % check no. of input args is correct
error(nargchk(0, 0, nargout)) % check no. of output args is correct

% LOOK OUT: search for a tag of the calling object
%   uim_info_map, uim_info_data, uim_info_vis

if (nargin==0)
  tmp_ud = get(gcbf,'UserData');
  if (isempty(tmp_ud))
    fprintf(1,'%s: No UserData - no info on Map or Data\n',mfilename);
    return;
  end;
  if (isstruct(tmp_ud))
    if strcmp(get(gcbo,'tag'),'uim_info_map')
      if (isfield(tmp_ud,'map'))
        arg1 = tmp_ud.map;
      else
        fprintf(1,'%s: Map struct not loaded\n',mfilename);
        return;        
      end;
    elseif strcmp(get(gcbo,'tag'),'uim_info_data')
      if (isfield(tmp_ud,'data'))
        arg1 = tmp_ud.data;
      else
        fprintf(1,'%s: Data struct not loaded\n',mfilename);
        return;
      end;      
    elseif strcmp(get(gcbo,'tag'),'uim_info_vis')
      arg1 = tmp_ud;	% ATT. arg1.map, arg1.data and other variables!
    end;
  else   % not a struct
    fprintf(1,'%s: UserData not a SOM map or data struct\n',mfilename);
    return;   
  end;

  mapstring 	= '*';
  datastring 	= '*';
%  visstring	= '#';

%  if (isfield(arg1,'map')			% vis.map
%    visstring  = local_getvis2lb(arg1);
%  elseif (isfield(arg1,'lattice'))		& map.lattice
  if (isfield(arg1,'lattice'))			% map.lattice
    mapstring  = local_getmap2lb(arg1); 
  elseif (isfield(arg1,'data'))			% data.data
    datastring = local_getdata2lb(arg1);
  end;

else
  mapstring 	= arg1;
  datastring	= '#';
end;

local_create;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)functions LOCAL_CREATE

function local_create

global mapstring;
global datastring;

origUnits = get(0,'Units');
set(0,'Units','normalized');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% UICONTROL: Figure

a = figure('Color',[0.8 0.8 0.8], ...
	'Units','normalized', ...
	'MenuBar','none', ...
	'Name','SOM Toolbox -- Info', ...
	'NumberTitle','off', ...
	'Position',[0.1 0.55 0.35 0.33], ...
	'Tag','somui_info', ...
	'WindowStyle','modal');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Frames:

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.01 0.14 0.98 0.85], ...
	'Style','frame', ...
	'Tag','fr_main');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% UICONTROL: Buttons

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'Callback','delete(gcf)', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'Position',[0.8 0.01 0.19 0.12], ...
	'String','Close', ...
	'Tag','pb_cancel');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%

if (strcmp(datastring,'#'))
  tmp_str = 'SOMfig structure:';
  tmp_lb  = fieldnames(mapstring);
elseif (strcmp(mapstring,'*'))
  tmp_str = 'Data structure:';
  tmp_lb  = datastring;
else
  tmp_str = 'Map structure:';
  tmp_lb  = mapstring;
end;

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'BackgroundColor',[0.8 0.8 0.8], ...
	'HorizontalAlignment','left', ...
	'FontWeight','bold', ...
	'Position',[0.02 0.88 0.96 0.1], ...
	'String', tmp_str, ...
	'Style','text', ...
	'Tag','st_info_title');

b = uicontrol('Parent',a, ...
	'Units','normalized', ...
	'FontName','Courier', ...
	'BackgroundColor',[0.701961 0.701961 0.701961], ...
	'HorizontalAlignment','left', ...
	'Position',[0.02 0.15 0.96 0.72], ...
	'String', tmp_lb, ...
	'Style','listbox', ...
	'Tag','lb_info');


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%  end of LOCAL_CREATE  %%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function local_getmap2lb
%   Reads a map structure and prints it to ready 
%   for listbox. arg == map structure
%   Font in listbox is changed to Courier
%
% See also SOM_INFO

function mapstring = local_getmap2lb(arg)

mdim = length(arg.msize);
dim  = size(arg.codebook, ndims(arg.codebook));
l    = length(arg.train_sequence);

mapstring = cell(12+mdim,1);

mapstring{1}= strcat('Map name .........: ', arg.name);
mapstring{2}= strcat(' Data name .......: ', arg.data_name);
mapstring{3}= strcat('Shape ............: ', arg.shape);
mapstring{4}= strcat(' Lattice .........: ', arg.lattice);
mapstring{5}= strcat(' Neighborhood ....: ', arg.neigh);
mapstring{6}= strcat('Map grid dim .....: ', num2str(mdim));

tmp = num2str(arg.msize(1));
for i = 2:mdim
  tmp = strcat(tmp,'x', num2str(arg.msize(i)));
end
mapstring{7}= strcat(' Map grid size:...: ', tmp);
mapstring{8}= strcat('Codebook dim .....: ', num2str(dim));

for i = 1:dim,
  mapstring{8+i} = strcat(' #',num2str(i),' name .........: ', arg.comp_names{i});
end

if strcmp(arg.init_type, 'unknown')
  mapstring{9+dim}= 'The map has been produced using SOM_PAK program package.';
else
  mapstring{9+dim}= strcat('Init type ........: ', arg.init_type);
  mapstring{10+dim}= strcat('Training type ....: ', arg.train_type);

  if l > 0
    mapstring{11+dim}= strcat('# map trained ....: ', num2str(l));
    mapstring{12+dim}= strcat(' Last training ...: ', arg.train_sequence{l}.time);
  else
    mapstring{11+dim}= 'The map has been initialized,';
    mapstring{12+dim}= ' but not trained.';
  end
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%% end of local_getmap2lb  %%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% (sub)function local_getdata2lb
%   Reads a data structure and prints it to ready 
%   for listbox. arg == data structure
%   Font in listbox is changed to Courier
%
% See also SOM_INFO

function datastring = local_getdata2lb(arg)

dim      = size(arg.data, 2);
samples  = size(arg.data, 1);
ind      = find(~isnan(sum(arg.data'))); 
complete = size(arg.data(ind,:),1);
partial  = samples - complete;
values   = prod(size(arg.data));
missing  = sum(sum(isnan(arg.data))); 
  
datastring = cell(6+2*dim,1);

datastring{1}= strcat('Data name .........: ', arg.name);
datastring{2}= strcat('Sample vector dim .: ', num2str(dim));
datastring{3}= strcat('# data vectors ....: ', num2str(samples));
datastring{4}= strcat('Complete data vect : ', num2str(complete));
datastring{5}= strcat('Partial data vect .: ', num2str(partial));  

tmp1 = num2str(values-missing);
tmp2 = num2str(100*(values-missing)/values);
tmp = 'Complete values ...:';
tmp = strcat(tmp,tmp1,' of ',num2str(values),' (',tmp2,'%)');

datastring{6}= tmp;

for i=1:dim,
  datastring{5+2*i}= strcat('# ',num2str(i),' name ...........:', ...
                       arg.comp_names{i});

  missing  = sum(isnan(arg.data(:,i)));
  complete = samples - missing;
  tmp = strcat(num2str(complete),' of ',num2str(samples), ...
                 ' (',num2str(100*complete/samples),'%)');

  datastring{6+2*i}= strcat(' complete values ..: ', tmp);  
end;



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%% end of local_getdata2lb  %%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
