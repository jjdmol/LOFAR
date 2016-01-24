// Our Resources administration
#include  <GPI_Resources.hxx>
#include  <ErrHdl.hxx>
#include  <GPI/GCF_PortManager.hxx>

// Our static Variable
PVSSuint      GPIResources::_serverPort  = 0;

// Wrapper to read config file
void  GPIResources::init(int &argc, char *argv[])
{
  begin(argc, argv);

  while ( readSection() || generalSection() )
    ;
  end(argc, argv);
}


// Read the config file.
// Our section is [ApiTest] or [ApiTest_<num>], 
PVSSboolean  GPIResources::readSection()
{
  static char line[200];
  
  // Is it our section ? 
  // This will test for [PI] and [PI_<num>]
  if (!isSection("PI"))
    return PVSS_FALSE;

  // Read next entry
  getNextEntry();
  
  // Loop thru section
  while ( (cfgState != CFG_SECT_START) &&  // Not next section
          (cfgState != CFG_EOF) )          // End of config file
  {
    if (!keyWord.icmp("serverPort"))             
      cfgStream >> _serverPort;                  
    else if (!keyWord.icmp("client"))
    {
      CharString hostName;
      char clientName[200];
      *clientName = 0;
      cfgStream >> hostName;
      cfgStream.get(line, 200, '\n');
      sscanf(line, "%s", clientName);
      GCFPortManager::getInstance()->addHost(hostName, clientName);
    }
    else if (!readGeneralKeyWords())            // keywords handled in Resources
    {
      ErrHdl::error(ErrClass::PRIO_WARNING,     // not that bad
                    ErrClass::ERR_PARAM,        // wrong parametrization
                    ErrClass::ILLEGAL_KEYWORD,  // illegal Keyword in Res.
                    keyWord);

      // Signal error, so we stop later
      cfgError = PVSS_TRUE;
    }

    getNextEntry();
  }


  //_counter = counter;
  // So the loop will stop at the end of the file
  return cfgState != CFG_EOF;
}

/** No descriptions */
void GPIResources::setProjectName(const char* projName)
{
  projectName = projName; // will be set in data member of base class
}
