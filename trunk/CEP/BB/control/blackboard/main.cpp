#include "blackboard/BlackBoard.h"

#include <DTL.h>

#include <iostream>
#include <string>
#include <typeinfo>

#define DEBUG(x) if(debug) std::cout << x << std::endl;

    //util consts
bool debug(true);

    // wants to use debug:
#include "blackboard/NoSuchBlackBoardComponentException.h"

const char *controlTableName = "controldata";
const char *BlackBoardName = "BlackBoard";
//const char *connectString = "UID=bb;DSN=lofar16";
const char *connectString = "lofar17";

    //forward declarations util functions
void connect(void);
void create(void);
const std::string & getRole(const int rank);
MPIProgramEntry *getObjectThatCanFulfill(const std::string &role);

    //the control data object
class ControlData 
{
   public:
      int rank;
          //      char role[31];
      std::string role;

      ControlData(const int initRank, const std::string& initRole):rank(initRank),role(initRole) 
      {
             //         strncpy(role,initRole,30);
         role[31]='\0';
         
      }
      ControlData():rank(-1),role("<none>")
      {
             //         strcpy(role,"<none>");
      }
};

class RankClause
{
   public:
      int rank;
};


class SelectOnRank
{
   public:
      void operator()(dtl::BoundIOs & position, RankClause &rankParam)
      {
         position[0] << rankParam.rank;
      }
};


    //BEGIN_DTL_NAMESPACE
class ControlBCA {
public:
      void operator() (dtl::BoundIOs &cols, 
                       ControlData  &rowbuf) {

         DEBUG("bind ControlBCA");
         
         cols["rank"] << rowbuf.rank;
         cols["role"] << rowbuf.role;
      }
};
    //END_DTL_NAMESPACE

int main(char ** av, int ac)
{
   connect();
       //<todo>How to handle none existend controldata table?</todo>
   create();

       //<todo>MPI has been init'ed here</todo>
   int rank(2);
   DEBUG("My rank is: " << rank);
   
   std::string role(getRole(rank));

   DEBUG("My role is: " << role);

       /**
          <todo>add code to map the returned role to a implementation
          class.</todo>
       */

       /**
          <todo>if we have more then one role we need to do something
          about primary and secondary roles, or ..</todo>

          <todo>May only the first role be implemented as an
          MPIProgramEntry.</todo>
       */

   try
   {
      MPIProgramEntry *programCode = getObjectThatCanFulfill(role);
       // g, i want metaclasses here.
      DEBUG("about to run...");

      programCode->run();
   }
       //   catch(std::exception &e)
       //   catch(std::runtime_error &e)
   catch(NoSuchBlackBoardComponentException &e)
   {
      DEBUG("failed to create a program...");

      std::cerr << e.what() << std::endl;
   }
   catch(...)
   {
      DEBUG("unhandled");
   }

   return 0;
}

const std::string & getRole(const int rank)
{
   static std::string rc = "<none>";

   DEBUG("accessing control-data..");

   dtl::DBView<ControlData,RankClause> view(controlTableName,ControlBCA(),"WHERE rank = (?)",SelectOnRank());
   DEBUG("creating iterator..");

   dtl::DBView<ControlData,RankClause>::select_iterator iter = view.begin();
   iter.Params().rank = rank;

   DEBUG("looking for rank: " << rank);

   DEBUG("Reading element #" << iter.GetLastCount());
   ControlData dat;

   if (iter != view.end())
   {
      DEBUG("found: " << iter->role);

          //<todo>What if more then onbe row can be returned? can "I" perform more then one role?</todo>
          //<todo>take good care of this former here memory leak</todo>
      rc = iter->role;
   }
   else
   {
      DEBUG("not found!");
   }
   return rc;
}

MPIProgramEntry *getObjectThatCanFulfill(const std::string &role)
{
   MPIProgramEntry * mpipe;

   DEBUG("getting an object");
   
   if(role == BlackBoardName)
   {
      
      mpipe = new BlackBoard();
   }
   else
   {
      NoSuchBlackBoardComponentException *ex = new NoSuchBlackBoardComponentException(role);
      DEBUG("throwing a " << typeid(NoSuchBlackBoardComponentException).name());
      throw *ex;
      
   }

   return mpipe;
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
