function som_showtitleButtonDownFcn
%som_showtitleButtonDownFcn Callback of SOM_SHOWTITLE
%
% som_showtitleButtonDownFcn
%
% ARGUMENTS 
%  none
% RETURNS 
%  none
%
% Moves the axis using DRAGRECT command. 
%
% SEE ALSO: DRAGRECT, SOM_SHOWTITLE

% Action

[txt,fig]=gcbo;                     % Get text and figure handles
ax=get(txt,'parent');               % Get axis handle

memunits_fig=get(fig,'units');      % Get figure size in pixels
set(gcf,'units','pixels'); 
pos_fig=get(fig,'position');        

memunits_txt=get(txt,'units');      % Get text field size in pixels
set(txt,'units','pixels');            
text_size=get(txt,'extent');

memunits_ax=get(ax,'units');        % Get axis position in pixels
set(ax,'units','pixels');          
pos_ax=get(ax,'position');

%%% Move text

pos_final=dragrect([pos_ax(1:2) text_size(3:4)]);

%%% Keep the text inside the figure!

pos_final(1)=max(pos_final(1),0);
pos_final(2)=max(pos_final(2),0);
pos_final(1)=min(pos_final(1),pos_fig(3)-text_size(3));
pos_final(2)=min(pos_final(2),pos_fig(4)-text_size(4));

%%% Set new position

new_pos=[pos_final(1:2) pos_ax(3:4)];
set(ax,'position', new_pos);

%%% Set the text on the top of the object stack 

children=get(gcf,'children');
i=find(ismember(children,ax));
new_i=[i 1:i-1 i+1:length(children)];
set(gcf,'children',children(new_i));

set(txt,'position',[0 0 0]);
set(fig,'units',memunits_fig);
set(ax,'units',memunits_ax);
set(txt,'units',memunits_txt);



