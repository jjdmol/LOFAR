global HSpecgramFig2;
subplot (HSpecgramFig2);
val = get(HBtnColorBar2,'UserData');
if (val == 0)
   HColorBar2 = colorbar;
   BtnState = 1;
   set(HBtnColorBar2,'userData',BtnState)
elseif val == 1
   delete (HColorBar2);
   BtnState = 0;
   set(HBtnColorBar2,'userData',BtnState)
end
