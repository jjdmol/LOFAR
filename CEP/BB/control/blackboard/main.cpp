#include "blackboard/MPIProgramEntry.h"

#include <DTL.h>

#include <iostream>
#include <string>

    //util consts
const char *controlTableName = "controldata";

    //util functions
void connect(void);
void create(void);
const std::string & getRole(const int rank);

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
         std::cout << "##bind ControlBCA##" << std::endl;
         cols["rank"] << rowbuf.rank;
         cols["role"] << rowbuf.role;
      }
};
    //END_DTL_NAMESPACE

int main(char ** av, int ac)
{
   connect();
       //<todo>How to handle none existend controldata table?</todo>
       //   create();

       //<todo>MPI has been init'ed here</todo>
   int rank(2);
   std::cout << "My rank is: " << rank << std::endl;
   
   std::string role(getRole(rank));

   std::cout << "My role is: " << role << std::endl;

   MPIProgramEntry * programCode;

   return 0;
}

const std::string & getRole(const int rank)
{
   static std::string rc = "<none>";
   
   std::cout << "accessing control-data.." << std::endl;

   dtl::DBView<ControlData,RankClause> view(controlTableName,ControlBCA(),"WHERE rank = (?)",SelectOnRank());
   std::cout << "creating iterator.." << std::endl;

   dtl::DBView<ControlData,RankClause>::select_iterator iter = view.begin();
   iter.Params().rank = rank;

   std::cout << "looking for rank: " << rank << std::endl;

   std::cout << "Reading element #" << iter.GetLastCount() << std::endl;
   ControlData dat;
   
   if (iter != view.end())
   {
      std::cout <<"found: " << iter->role << std::endl;
      
          //<todo>What if more then onbe row can be returned? can "I" perform more then one role?</todo>
          //<todo>take good care of this former here memory leak</todo>
      rc = iter->role;
   }
   else
   {
      std::cout <<"not found!" << std::endl;
   }
   return rc;
}

    // get connection
    // <todo>get connection string items from configuration</todo>
void connect()
{
   try {
      std::cout << "Connecting to database.." << std::endl;
      dtl::DBConnection::GetDefaultConnection().Connect("UID=bb;DSN=pg49");
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
      std::cout << "Creating new table.." << std::endl;
      dtl::DBStmt("CREATE TABLE controldata (\
                rank INTEGER,\
                role VARCHAR(30))").Execute();
   }
   catch(dtl::DBException dbex)
   {
      std::cerr << "wow!" << std::endl << dbex.what() << std::endl;
   }
}
