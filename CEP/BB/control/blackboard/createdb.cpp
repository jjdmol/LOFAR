#include <DTL.h>

#include <iostream>
#include <string>
#include <typeinfo>

#include <getopt.h>
#define DEBUG(x) if(debug) std::cout << x << std::endl;

    //util consts
bool debug(true);

const char *deploymentTableName = "deploymentdata";
const char *scriptsTableName = "scripts";

std::string dbuserid("bb");
std::string dsn("lofar16");

const char *connectString;

    //forward declarations util functions
void connect(void);
void create(void);

int main( int ac, char ** av)
{
  int c;
  static struct option long_options[] =
  {
    {"dbuser", 1, 0, 'u'},
    {"dsn", 1, 0, 'd'},
    {0, 0, 0, 0}
  };

  DEBUG("searching for options");

  while((c = getopt_long (ac, av, "u:d:",
		   long_options, NULL) ) != -1)
  {
    switch (c)
    {
    case 'u':
      dbuserid = optarg;
      std::cout << "dbuserid set to " << optarg << std::endl;
      break;
    case 'd':
      dsn = optarg;
      std::cout << "dsn set to " << optarg << std::endl;
      break;
    }
  }

  std::string connectStringString = "UID=" + dbuserid + ";DSN=" + dsn + ";";

  connectString = connectStringString.c_str();

  DEBUG("connectString: " << connectString);

  connect();
       //<todo>How to handle none existend deploymentdata table?</todo>
  create();
  return 0;
}

    // get connection
    // <todo>get connection string items from configuration</todo>
void connect()
{
   try {
      DEBUG("Connecting to database..");
      dtl::DBConnection &dbcon = dtl::DBConnection::GetDefaultConnection();
      DEBUG("Got default connection..");
      dbcon.Connect(connectString);
      DEBUG("connection started");
   }
   catch(dtl::DBException dbex)
   {
      std::cerr << "wow!" << std::endl << dbex.what() << std::endl;
   }
}

    //create the deployment data, it should allready exist
void create()
{
   try {
      DEBUG("Creating new table..");
      dtl::DBStmt("CREATE TABLE deploymentdata (\
                rank INTEGER,\
                role VARCHAR(30),\
                master INTEGER)").Execute();
      dtl::DBStmt("CREATE TABLE scripts (\
                id text,\
                text text)").Execute();
   }
   catch(dtl::DBException dbex)
   {
      std::cerr << "wow!" << std::endl << dbex.what() << std::endl;
   }
}
