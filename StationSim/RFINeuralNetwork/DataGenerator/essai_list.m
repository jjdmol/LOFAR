t=1;
list_modu=cell(100,1);
for i=1:length(Modulated_signal)
    list_modu{t}=['' Modulated_signal(i).Modula_signal ''];
    for j=1:length(Modulated_signal(i).feature_modulat)
        t=t+1;
        list_modu{t}=['   ' Modulated_signal(i).feature_modulat(j).name ' -  ' num2str(Modulated_signal(i).feature_modulat(j).Carrier_freq) '  -  'num2str(Modulated_signal(i).feature_modulat(j).modulation_Ampl) ' - ' num2str(Modulated_signal(i).feature_modulat(j).modulation_param) '' ];
    end 
    t=t+1;
end
set(List_modulation,'String',list_modu)