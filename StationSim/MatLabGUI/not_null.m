function amount = not_null(signals)

amount = 0;

for i = 1:(size(signals, 1))
    amount = amount + (signals(i, 1) ~= 0) ;
end;