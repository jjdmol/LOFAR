#include <DTL.h>

#include <iostream>
#include <string>
#include <typeinfo>

#define DEBUG(x) if(debug) std::cout << x << std::endl;

    //util consts
bool debug(true);

const char *controlTableName = "controldata";

const char *connectString = "UID=bb;DSN=lofar16;";

    //forward declarations util functions
void connect(void);
void create(void);

int main(char ** av, int ac)
{
   connect();
       //<todo>How to handle none existend controldata table?</todo>
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

    //create the control data, it should allready exist
void create()
{
   try {
      DEBUG("Creating new table..");
      dtl::DBStmt("CREATE TABLE controldata (\
                rank INTEGER,\
                role VARCHAR(30),\
                master INTEGER)").Execute();
   }
   catch(dtl::DBException dbex)
   {
      std::cerr << "wow!" << std::endl << dbex.what() << std::endl;
   }
}
