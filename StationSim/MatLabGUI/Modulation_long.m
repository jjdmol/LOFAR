function [Signal_modulation]=modulation_long(y,Signal_modulation);

diff=length(Signal_modulation)-length(y);

if diff>0
  
    y=[y' zeros(diff,1)']';
elseif diff<0
    disp(['diff2:'num2str(diff) '']);
    Signal_modulation=[Signal_modulation' zeros(abs(diff),1)']';
  
end

Signal_modulation=Signal_modulation+y;
 
clear diff;
