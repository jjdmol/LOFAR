global HColorMap;
val = get(HColorMap,'Value');
if val == 1
   colormap(gray)
elseif val == 2
    colormap(bone)
elseif val == 3
    colormap(jet)
elseif val == 4
    colormap(hsv)
elseif val == 5
    colormap(hot)
elseif val == 6
   colormap(cool)
elseif val == 7
    colormap(gray)
end
