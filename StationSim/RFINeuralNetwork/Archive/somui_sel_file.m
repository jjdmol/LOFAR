function [filename, path] = somui_sel_file(filter,figname,x,y)
%SOMUI_SEL_FILE  selects a file from disk. A dummy function.
%
%  [filename, path] = somui_sel_file(filter, figname, x, y)
%
% ARGUMENTS
%
%   filter   (string)
%   figname  (string)	A name for the window, 'Load' or 'Save'
%   x        (double)
%   y        (double)
%
% RETURNS
%   filename  (string)	A chosen file name
%   path      (string)	Path name for chosen file
%
% SOMUI_SEL_FILE is a dummy function, does exactly the same as uigetfile.
%
% See also UIGETFILE, SOMUI_SEL_VAR, SOMUI_LOAD, SOMUI_SAVE


% uigetfile: if one parameter used, then all must be used
% [FILENAME, PATHNAME] = UIGETFILE('filterSpec', 'dialogTitle', X, Y)

[filename, path] = uigetfile(filter,figname,x,y);




