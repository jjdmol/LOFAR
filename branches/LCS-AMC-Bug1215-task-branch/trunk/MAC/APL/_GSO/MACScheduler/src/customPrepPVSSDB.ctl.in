// this script must reside in the <PVSS project>/scripts directory
main()
{
  // create an enabled flag for CCU_PIC
  dpCreate("PIC__enabled", "GCFPaPsEnabled");
  dpSet("PIC__enabled.","autoloaded|TCcuPic");
  dpCreate("PIC_Stations__enabled", "GCFPaPsEnabled");
  dpSet("PIC_Stations__enabled.","autoloaded|TCcuPic");
  dpCreate("PIC_CEP__enabled", "GCFPaPsEnabled");
  dpSet("PIC_CEP__enabled.","autoloaded|TCcuCep");
  dpCreate("GSO__enabled", "GCFPaPsEnabled");
  dpSet("GSO__enabled.","autoloaded|TCcuPic");
  dpCreate("VIC__enabled", "GCFPaPsEnabled");
  dpSet("VIC__enabled.","autoloaded|TCcuPic");
  
  dpDelete("GSO_MACScheduler");
}
