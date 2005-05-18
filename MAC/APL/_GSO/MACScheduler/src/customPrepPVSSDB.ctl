// this script must reside in the <PVSS project>/scripts directory
main()
{
  // create an enabled flag for CCU_PIC
  dpCreate("PIC__enabled", "GCFPaPsEnabled");
  dpSet("PIC__enabled.","autoloaded|TCcuPic");
  dpCreate("PIC_Stations__enabled", "GCFPaPsEnabled");
  dpSet("PIC_Stations__enabled.","autoloaded|TCcuPic");
}
