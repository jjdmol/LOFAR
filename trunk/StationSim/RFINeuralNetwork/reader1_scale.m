subplot (HSpecgramFig);
val = get(HBtnColorBar,'UserData');
if (val == 0)
   HColorBar = colorbar;
   BtnState = 1;
   set(HBtnColorBar,'userData',BtnState)
elseif val == 1
   delete (HColorBar);
   BtnState = 0;
   set(HBtnColorBar,'userData',BtnState)
end
