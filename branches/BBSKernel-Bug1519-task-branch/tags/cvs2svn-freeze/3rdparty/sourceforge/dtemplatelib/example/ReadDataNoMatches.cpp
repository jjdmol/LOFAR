#include "ReadDataNoMatches.h"

// should return an empty vector of Example objects as we're feeding the view
// a query that should return no matches
vector<Example> ReadDataNoMatches()
{
 vector<Example> results;

 // as INT_VALUE < INT_VALUE is never true, no rows will be returned by the query
 DBView<Example> view("DB_EXAMPLE", DefaultBCA<Example>(), 
   "WHERE INT_VALUE < INT_VALUE" + exampleOrderBy);

 DBView<Example>::select_iterator read_it = view.begin();
 for ( ; read_it != view.end(); ++read_it)
 {
  results.push_back(*read_it);
 }

 return results;

}
