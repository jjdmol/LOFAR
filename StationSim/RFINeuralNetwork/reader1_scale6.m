subplot (HSpecgramFig6);
val = get(HBtnColorBar6,'UserData');
if (val == 0)
   HColorBar = colorbar;
   BtnState = 1;
   set(HBtnColorBar6,'userData',BtnState6)
elseif val == 1
   delete (HColorBar);
   BtnState = 0;
   set(HBtnColorBar6,'userData',BtnState6)
end
