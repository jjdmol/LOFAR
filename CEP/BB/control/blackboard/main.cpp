#include "blackboard/BlackBoard.h"
#include "control/TopLevelStrategy.h"
#include "knowledge/WorkerWrapper.h"

#include "blackboard/debug.h"

#include <DTL.h>

#include <iostream>
#include <string>
#include <typeinfo>

//#include <mpi2c++/mpi++.h>
#include <mpi.h>

//#define DEBUG(x) if(debug) std::cout << " rank(" << ::rank << "): " << x << std::endl;

    //util vars
int rank(-1);
bool debug(true);

    // wants to use debug:
#include "blackboard/NoSuchBlackBoardComponentException.h"

const char *controlTableName = "controldata";
const char *BlackBoardName = "BlackBoard";
const char *ControllerName = "Controller";
const char *EngineName = "Engine";

const char *connectString = "UID=bb;DSN=lofar16;";

    //forward declarations util functions
void connect(void);
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

int main( int ac, char ** av)
{
  MPI_Init(&ac, &av);
  MPI_Comm      comm = MPI_COMM_WORLD;
  MPI_Comm_rank(comm, &rank);
  //  int rank(MPI::COMM_WORLD.Get_rank());
  DEBUG("My rank is: " << rank);

   connect();
       //<todo>How to handle none existend controldata table?</todo>

       //<todo>MPI has been init'ed here</todo>
   
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

   MPI_Finalize();
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
   else if(role == ControllerName)
   {
     MPIProgramEntry &mpip = TopLevelStrategy::Instance();
     mpipe = & mpip;
   }
   else if(role == EngineName)
   {
     mpipe = new WorkerWrapper();
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
