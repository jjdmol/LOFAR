pragma include once
include 'debug_methods.g'
include 'poly_mep_viewer.g'

const polytracker := function (mep_table_name,ref agent,
                               parent=F, 
                               verbose=1)
{
  self := [ appid='Polytracker' ];
  public := [=];
  
  define_debug_methods(self,public,verbose);
  
  self.mep := read_mep_table(mep_table_name);
  self.data := [=];
  self.agent := ref agent;
  
  self.pv := ref poly_mep_viewer(F,verbose=verbose,unmap=T);

  # clear viewer before starting a new solution  
  whenever self.agent->start_solution do
  {
    self.pv.clear();
    self.data := [=];
    self.pv.topframe()->unmap();
  }
  
  # add data
  whenever self.agent->end_iteration do
  {
    iter_num := $value.iteration_num;
    names := $value.solution.param_names;
    values := $value.solution.param_values;
    # on first entry, populate with actual values from MEP table
    if( !len(self.data) )
    {
      self.data := mep2rec(self.mep,field='actual',col='SIM_VALUES',range=T,subset=names);
      self.data := mep2rec(self.mep,field='perturbed',col='VALUES',range=T,subset=names,rec=self.data);
      # adjust ranges to MHz and [0,1] time
      for( f in field_names(self.data) )
      {
        self.data[f].xrng := [0,1];
        self.data[f].yrng *:= 1e-8;
      }
      newdata := T;
    }
    else
      newdata := F;
    # insert new data into data array
    id := paste('iteration',iter_num);
    for( i in 1:len(names) )
    {
      parm := names[i];
      if( !has_field(self.data,parm) )
        self.dprint(0,'oops, parameter ',parm,' not found in data, ignoring');
      else
      {
        # bug here: data shape is hardwired as [3,2] for now
        self.data[parm].poly[id] := array(values[((i-1)*6)+(1:6)],3,2);
      }
    }
    # send to viewer
    if( newdata )
      self.pv.set_data(self.data,xlab='TIME(normalized)',ylab='FREQ (MHZ)',
                        ids=['actual','perturbed',id],plotcolors=[1,2]);
    else
      self.pv.add_data_id(id,plotcolor=iter_num+1,refresh=T);
    self.pv.topframe()->map();
  }
  
  public.self := ref self; 
  return ref public;  
}
