// this script must reside in the <PVSS project>/scripts directory
main()
{
  // create an enabled flag for CCU_PIC
  dpCreate("PIC_CCU__enabled", "GCFPaPsEnabled");
  dpSet("PIC_CCU__enabled.","autoloaded|TCcuPic");
}
