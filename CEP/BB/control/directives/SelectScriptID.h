#ifndef SELECTSCRIPTID_H_HEADER_INCLUDED_C0B1C703
#define SELECTSCRIPTID_H_HEADER_INCLUDED_C0B1C703

//##ModelId=3F4DC73102EE
class SelectScriptID
{
   public:
      void operator()(dtl::BoundIOs & position,
		      scriptSelectClause &selectParam)
      {
         position[0] << selectParam.id;
      }
};



#endif /* SELECTSCRIPTID_H_HEADER_INCLUDED_C0B1C703 */
