# file: ../MeqTrees/MeqName.g

#---------------------------------------------------------

# pragma include once
# print '\n\n\n\n\n\n=================================================';
print 'include MeqName.g    d02/do3/h05/h07/d08/h09oct2003';

include 'unset.g';
include 'genericClosureFunctions.g';





############################################################################
# MeqName: Collection of MeqNode names and its functions:
############################################################################

# Changing the internal list must be done explicitly (e.g. append=T).
# Results are given back as new MeqName objects.
# Most functions can also be used on names given as argument.


MeqName := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqName');

    private.init := function () {
	wider private;
	public.classnames("");
	public.names("");
	return T;
    }

    public.MeqSpigots := function (pattern="*") {
    }
    public.MeqParms := function (pattern="*") {
    }
    public.MeqFuncs := function (pattern="*") {
    }

#===========================================================================
# Public interface:
#===========================================================================
# Make a new MeqName object, in various ways:
# Used to return derived sets of names.

    public.new := function (names=F, history=F, trace=F) {
	new := MeqName();
	new.names(names, append=F);
	if (is_string(history)) new.history(history);
	if (trace) print new.summary('created new MeqName');
	return new;
    } 

# Access to its internal set of node-names: 

    public.names := function (names=F, append=F, trace=F) {
	wider private;
	if (trace) print '.names(',type_name(names),len(names),append,')';
	if (!has_field(private.state,'names')) private.state.names := "";
	if (is_string(names)) {
	    if (append) {
		# Append to existing internal nodenames
		private.state.names := [private.state.names,names];
	    } else {
		private.state.names := names;
		private.makemap(trace=trace);        # -> private.map
	    }
	}
	return private.state.names;
    } 
    public.clear := function () {return public.names("", append=F)}


# Access to the list of valid class-names.
# These are obtained 'somehow', probably from MeqServer.

    public.classnames := function (names=F, append=F) {
	wider private;
	if (!has_field(private.state,'classnames')) private.state.classnames := "";
	if (is_string(names)) {
	    if (!append) private.state.classnames := "";
	    private.state.classnames := [private.state.classnames,names];
	}
	return private.state.classnames;
    } 
    public.is_classname := function (name=F) {
	if (any(name==private.state.classnames)) return T;
	public.message(notrec=name);
	return F;
    }

# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()!

    private.summary := function () {
	ss := "";
	n := len(nn := public.names());
	ss := paste(ss,'\n-',n,'names in the list');
	if (n>0) ss := paste(ss,' (first =',nn[1],')');
	if (F) {
	    n := len(nn := public.classnames());
	    ss := paste(ss,'\n-',n,'valid classnames');
	    if (n>0) ss := paste(ss,' (first =',nn[1],')');
	}
	return ss;
    }

# Format part of the contents of this object:
# NB: Called by genericClosureFunction .show()!

    private.show := function (opt=[=]) {
	if (!has_field(opt,'what')) opt.what := 'names';
	if (opt.what=='classnames') n := len(nn := public.classnames());
	if (opt.what=='names') n := len(nn := public.names());
	ss := "";
	for (i in ind(nn)) {
	    ss := paste(ss,'\n-',i,':',nn[i]);
	}
	return ss;
    } 

#===============================================================================
# Construct a legal node-name: <classname>[qual1=value][qual2=..][qual3=..][...]
# Note that qualifiers arre appended in order of the qual record.
# A qualifier may be parametrized. Parameters are enclosed: e.g. [s1={s1}].
# In addition, expressions are allowed: [s2={s1}+1]
# Parameters are filled in with actual values by .fillin(), see below.
# If a qualifier has multiple values, multiple nodenames will be produced. 

    public.nodename := function (classname='<classname>', qual=[=], append=F, trace=F) {

	# Convenience: classname may be the output record from .dissect():
	if (is_record(classname)) {
	    if (!has_field(classname,'classname')) {
		return F;                    # error
	    } else if (!has_field(classname,'qual')) {
		return F;                    # error
	    } else {	 
		qual := classname.qual;
		classname := classname.classname;
	    }
	}

	if (trace) print '\n** .nodename(',classname,qual,'):';
	qual := public.qual(qual);
	substitute := F;                     # see below
	# The node-name starts with a class-name (e.g. 'mqe_predict')
	s := spaste(classname);
	# Followed by qualifiers: .{<qname>=[<value>]}
	for (qname in field_names(qual)) {
	    vv := qual[qname];
	    if (len(vv)>1) {                 # vv is multiple
		vv := T;                     # parametrized qualifier {<qname>}
		substitute := T;             # see below
	    }
	    s := private.append_qual(s, qname, vv[1]);
	}

	# Fill in with multiple values, if required:
	if (substitute) {
	    s := public.fillin (qual, name=s, trace=trace);
	}

	# Append the new nodename to the internal list, if required:
	if (append) public.names(s, append=T);
	if (trace) {
	    for (i in ind(s)) print '           ->',i,':',s[i];
	}
	return s;                            # return the new nodename
    }

# Check the qual record:

    public.qual := function (qual=F) {
	wider private;

	# Initialise private.qual with some default quals:
	if (!has_field(private,'qual')) {
	    qq := [=];	    
	    qq.s1 := [s='{s1}'];              # station s1
	    qq.s2 := [s='{s2}'];              # station s2
	    qq.s12 := [s1=T, s2=T];           # ifr (s1,s2)
	    qq.c := [c=T];                    # source component c
	    qq.cs1 := [c=T, s='{s1}'];        # station s1, source component c
	    qq.cs2 := [c=T, s='{s2}'];        # station s2, source component c
	    qq.cs12 := [c=T, s1=T, s2=T];     # ifr (s1,s2), source component c
	    private.qual := qq;
	}

	# Get the specified qual record:
	if (is_record(qual)) {
	    return qual;                      # assume ok
	} else if (!is_string(qual)) {
	    return [=];                       # error?
	} else if (!has_field(private.qual,qual)) {
	    return [=];                       # error?
	} else {
	    return private.qual[qual];        # predefined qual record
	}
    }

    public.quals := function (trace=T) {
	public.qual();                        # initialise
	s := paste('\n** Predefined qualifier records:');
	ss := s;
	if (trace) print s;
	for (fname in field_names(private.qual)) {
	    s1 := sprintf('%-8s',fname);
	    s := paste(' -',s1,'-> ',private.qual[fname]);
	    ss := paste(ss,'\n',s);
	    if (trace) print s;
	}
	ss := paste(ss,'\n');
	if (trace) print ' ';
	return ss;
    }

#-------------------------------------------------------------------------------
# Append a qualifier string {<qname>[=<v>]} to nadename string s:
# NB: If the value (v) is T, the qualifier field is appended
#     as a 'parameter', i.e. without a value: {<name>=}.
#     This can later be replaced by an actual value for the same
#     keyword. This is useful for building trees.
# NB: The parametrization may be nested by specyfying:
#     <qname>={<qname2>=}

    private.append_qual := function (s=F, qname=F, v=F, trace=F) {
	if (is_boolean(v) && !v) return s;       # v=F: ignore..........??   <---
	s := spaste(s,'[',qname,'=');            # opening bracket etc
	if (is_boolean(v)) {                     # v=T: parameter
	    if (F) {
		# OK, do nothing                 # -> [<qname>=]
	    } else {
		s := spaste(s,'{',qname,'}');    # -> [<qname>={<qname>}]: preferred
	    }
	} else if (is_record(v)) {
	    for (fname in field_names(v)) {
		s := private.append_qual(s, fname, v[fname]);
	    }
	} else if (is_numeric(v)) {
	    s := spaste(s,v[1]);                 # <qname>=<v>
	} else if (is_string(v)) {
	    if (v[1]=='+') {                     # obsolete...?
		s := spaste(s,'{',qname,'}+1');  # make expression
	    } else if (v[1]=='-') {              # obsolete...?
		s := spaste(s,'{',qname,'}-1');  # make expression
	    } else {
		s := spaste(s,v[1]);
	    }
	}
	s := spaste(s,']');            # closing bracket
	if (trace) print '** .append_qual(',qname,v,') ->',s;
	return s;
    }

#=====================================================================
# Generate specific node name(s) by filling in the parametrized qualifiers 
# in  the (internal or external) nodenames with the values given in the 
# fields of the qual record.
#    e.g. qual := [a="xx yy", b=4, d=[1:3], g='*']
# Multiple values are allowed (expansion).
# The order of qualifiers in qual determines the expansion order.
# Return the result directly, or in a new MeqName object.
# In any case, do NOT modify the internal names.

    public.fillin := function (qual=[=], name=F, trace=F) {
	external := (is_string(name));          # T if external name supplied
	if (external) {                         # work on external name(s)
	    hist := paste('.fillin(',qual,'): from:',name);
	} else {
	    name := public.names();             # get the internal name(s)
	    hist := paste('.fillin(',qual,'): from:',public.label());
	}
	if (trace) print '\n**',hist;

	n := len(name);                         # original length
	if (n>0) {
	    # Fill in the qualifier values one by one:
	    # NB: The name-vector may be expanded in the process! 
	    for (qname in field_names(qual)) {
		vv := qual[qname];
		name := private.fillin (name, qname, vv, trace=trace);
	    }
	}

	# Finished:
	if (trace) print '** .fillin(): expanded: from',n,'to',len(name),'name(s) \n';

	# Return the modified external name(s) as a string vector:
	# This is more convenient since the modification of external names
	# is mostly used in loops with one or a few names at a time.
	if (external) return name;           

	# Return the modified internal name(s) in a new MeqName object:
	return public.new(name, history=hist, trace=trace);
    }

# Fill in occurences of the parameter {<qname>} in the given name(s)
# with the value(s) vv, while evaluating any expressions if vv is numeric. 
# If vv is multiple, the vector (name) is expanded.

    private.fillin := function (name=F, qname=F, vv=F, trace=F) {
	if (is_boolean(vv)) return name;                 # no value specified, ignore
	if (trace) print '\n   private.fillin(): ',private._.format(vv,qname);
	if (!is_string(vv) && !is_numeric(vv)) return name;   # error...

	ss := "";                                        # output string vector
	for (i in ind(name)) {                           # name may be vector
	    s1 := spaste('\'',name[i],'\' ~ m/{',qname,'}/');
	    if (trace) print '    - s1=',s1;
	    if (eval(s1)) {                              # found {<qname>}
		if (trace) print '     - expand (vv is',type_name(vv),'):',name[i];

		if (is_string(vv)) {                     # string: just replace globally
		    for (v in vv) {
			s2 := spaste('\'',name[i],'\' ~ s/{',qname,'}/',v,'/g');
			if (trace) print '    - s2=',s2;
			s4 := eval(s2);                  # substitute
			ss := [ss,s4];                   # append to output vector
			if (trace) print '      - string v =',v,': ->',s4;
		    }

		} else {                                 # numeric: evaluate expression
		    rr := public.dissect(name[i]);       # -> [classname=.., qual=..]
		    count := F;

		    for (v in vv) {                      # for each value
			qual := rr.qual;                 # to be modified
			for (fname in field_names(rr.qual)) {
			    expr := qual[fname];         # e.g. '{<qname>}+1'
			    s6 := spaste('\'',expr,'\' ~ m/{',qname,'}/');
			    if (trace) print '    - s6=',s6;
			    if (eval(s6)) {              # found {<qname>}
				s7 := spaste('\'',expr,'\' ~ s/{',qname,'}/',v,'/g');
				if (trace) print '    - s7=',s7;
				s3 := eval(s7);          # {c}+1  ->  3+1
				s8 := eval(s3);          # 3+1    ->  4
				if (is_fail(s8)) {
				    print '** FAILED **  eval(',s3,')';
				    print s8;
				} else {
				    qual[fname] := s8;   # numeric!
				    if (trace) print '      - qual =',qual,' s8=',type_name(s8),s8;
				}
			    }
			}
			# Make a new nodename with the modified qual record:
			# NB: s4 will be multiple if qual fields have multiple values!
			#     so this automatically supports vector expressions
			s4 := public.nodename(rr.classname, qual, append=F);
			if (trace) print '      - s4=',s4;
			ss := [ss,s4];                   # append to output vector
			count +:= 1;                     # count the number of new names
		    }

		    if (count==0) {
			print '      - ** ERROR ** (count=0):',name[i];
			ss := [ss,name[i]];              # copy the unchanged name
		    }
		}

		
	    } else {                             # {<qname>} not in name[1]
	 	if (trace) print '      - copy:',name[i];
		ss := [ss,name[i]];              # just copy it to the output vector
	    }
	}

	# Return the new (and possibly expanded) vector of names:
	return ss;
    }


#=========================================================================
# Check the given (or internal) nodenames for parametrized qualifiers 
# [<qname>={..}]. Returns a boolean vector.

    public.parametrized := function (name=F, trace=T) {
	wider private;
	if (trace) print '\n** .parametrized():';
	internal := !is_string(name);
	if (internal) name := public.names();
	ff := rep(F,len(name));               # True if name parametrized
	for (i in ind(name)) {
	    # Check for the occurence of '{'':
	    s1 := spaste('\'',name[i],'\' ~ m/\{/');
	    ff[i] := eval(s1);  
	    if (trace && ff[i]) print '-',i,': parametrized qualifier in: ',name[i];
	}
	if (trace) print '** .parametrized():',len(ff[ff]),'/',len(ff);
	# r := any(ff);                         # -> T if any names are parametrized
	return ff;
    }


#=========================================================================
# Dissect a given node name into classname and qualifiers:
# Return in a record. NB: The qualifier values are strings!

    public.dissect := function (name=F, trace=F) {
	ss := split(name,'[');                 # split on opening brackets ([)
	rr := [classname=ss[1], qual=[=]];     # output record
	if (len(ss)>1) {
	    for (j in [2:len(ss)]) {
		s2 := split(ss[j],']');        # split on closing brackets (])
		s3 := split(s2[1],'=');        # split on '='
		if (len(s3)==1) {              # i.e. '=]' (parametrized qualifier)   
		    # NB: Semi-obsolete: should not really happen...
		    # Make the equivalent {} parameter expression
		    rr.qual[s3[1]] := spaste('{',s3[1],'}');
		} else {
		    v := s3[2];                # qualifier value (string!)
		    if (as_integer(v)!=0) {
			rr.qual[s3[1]] := as_integer(v);
		    } else if (any(v=="0")) {
			rr.qual[s3[1]] := 0;   # assume integer
		    } else {
			rr.qual[s3[1]] := v;   # assume string
		    }
		}
	    }
	}
	if (trace) {
	    print '** .dissect(',name,') ->',rr;
	    s := public.nodename(rr, append=F, trace=trace);
	    if (s==name) {
		print '      OK: .nodename(.dissect()) is circular';
	    } else {
		print '      not OK(??): .nodename(.dissect()) not circular! ->',s;
	    }
	}
	return rr;
    }


#=========================================================================
# Return a new MeqName object with a subset of the internal node-names:
# If external, use a temporary MeqName object: 

    public.subset := function (classname='*', qual=[=], trace=F) {
	hist := paste('.subset() of:',public.label(),':',classname,qual);
	if (trace) print '\n**',hist;
	# Make sure that private.map record exists and is up to date
	private.makemap(trace=trace);

	if (classname=='spigot') classname := 'mqs_spigot';
	if (classname=='parm') classname := 'mqp_parm';

	name := public.names();
	nsv := len(sv := rep(T,len(name)));

	for (fname in field_names(private.map)) {
	    if (trace) print '-',fname,':',private.map[fname];
	    if (fname=='classname') {
		sv1 := rep(F,nsv);
		for (s in classname) {
		    if (s=='*') {
			sv1 := rep(T,nsv);
		    } else {
			sv1 |:= (private.map.classname==s);
		    }
		    if (trace) print '  - classname =',s,'->',sv1;
		}
		sv &:= sv1;

	    } else if (has_field(qual,fname)) {     # Selection specified:
		sv1 := rep(F,nsv);
		for (v in qual[fname]) {            # may be vector
		    v1 := spaste(v);                # make string
		    if (v1=='*') {
			sv1 := rep(T,nsv);
			sv1 &:= !(private.map[fname]=='-');
			sv1 &:= !(private.map[fname]=='?');
		    } else {
			sv1 |:= (private.map[fname]==v1);
		    }
		    if (trace) print '  -',fname,'=',v1,'->',sv1;
		}
		sv &:= sv1;

	    } else {
		# No selection specified for this qualifier: ignore
	    }
	    if (trace) print '  sv =',sv;
	}
	return public.new(name[sv], history=hist, trace=trace);
    }

# Dissect the internal names, and fill private.map:
# This is used to obtain subsets (see .subset()).

    private.makemap := function (trace=F) {
	wider private;
	private.map := [classname=""];
	if (trace) print '\n** private.makemap():';
	name := public.names();
	for (i in ind(name)) {
	    rr := public.dissect (name[i]);
	    if (trace) print '-',i,':',name[i],'->',rr;
	    private.map.classname[i] := rr.classname;
	    for (fname in field_names(rr.qual)) {
		if (!has_field(private.map,fname)) {
		    private.map[fname] := "";
		    if (i>1) private.map[fname] := rep('-',i-1);
		}
		# Qualifier values from dissect() may not be strings.
		# They should be converted for the purpose of private.map
		private.map[fname][i] := paste(rr.qual[fname]);
	    }
	}
	if (trace) {
	    for (fname in field_names(private.map)) {
		v := private.map[fname];
		print '-',fname,type_name(v),'[',len(v),']:',v;
	    }
	    print ' ';
	}
	return T;
    }



#===========================================================================
# Private functions:
#===========================================================================



#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}




############################################################################
# test-function:
############################################################################

test_MeqName := function () {
    include 'inspect.g';
    print '\n\n****************************************************';
    print '** test_MeqName:';

    # Create global object(s):
    print '\n** Create global objects';
    global mqn;
    print '\n',mqn := MeqName('mqn');
    if (T) {
	mqn.classnames("AA BB CC");
	mqn.names("aa bb cc dd");
    }
    print ' ';

    if (F) {
	# mqn.inspect('mqn');
	# print 'mqn.summary() ->',mqn.summary();
	# print 'mqn.show(classnames) ->',mqn.show(opt[what='classnames']);
	# print 'mqn.show() ->',mqn.show();
	# print 'mqn.clear() ->',mqn.clear();
	# print 'mqn.summary() ->',mqn.summary();
    }

    if (F) mqt.test();

    if (F) {
	print '\n******************\nTest: circularity of .dissect()';
	print '** s=',s := mqn.nodename('mqp_parm', [a=T, b='{b}', c='{c}+1', d='{d}-1'], append=T, trace=T);
	print '** .dissect(',s,') ->',rr := mqn.dissect(s, trace=T);
	print '** s1=',s1 := mqn.nodename(rr, append=F, trace=T);
	# print '** s1=',s1 := mqn.nodename(rr.classname, rr.qual, append=F, trace=T);
	print '** (s==s1) ->',(s==s1);
    }

    if (F) {
	mqn.clear();
	# print mqn.nodename('mqp_parm', [a=T, b=2, c='xxx', d=F], append=T, trace=T);
	# print mqn.nodename('mqp_parm', [a=T, b=[1:4]], append=T, trace=T);
	# mqn.nodename('mqp_parm', [s=[s2=T]], append=T);
	# mqn.nodename('mqp_parm', [s=[s1=[s2=T]]], append=T);
	# mqn.nodename('mqp_parm', "a bb ccc ddd", append=T);
	mqn.nodename('mqp_parm', [a=T, b='{b}', c='{c}+1', d='{d}-1'], append=T, trace=T);
	# mqn.nodename('mqp_parm', [a=T, b='+', c='-'], append=T, trace=T);
	mqn.show(trace=T);
	print '** .parametrized() ->',mqn.parametrized(trace=T);

	# mqn1 := mqn.fillin([c=[1:3]], trace=T);
	# mqn1 := mqn.fillin([c=[1:3], d=[1:3]], trace=T);
	mqn1 := mqn.fillin([a="xx yy", b=[-1:1], c=[1:3], d=[1:3]], trace=F);	
	# mqn1 := mqn.fillin([a="xx yy"], trace=T);
	# mqn1 := mqn.fillin([a="xx yy", c=[1:2], d=[1:2]], trace=F);

	print '** .parametrized() ->',mqn1.parametrized(trace=T);
	mqn.show(trace=T);
	# print '** .check() ->',mqn.check(trace=T);
    }
	
    if (F) {
	print mqn.nodename('mqp_parm', [a=T, b=2, c='xxx', d=F], append=T);
	# mqn1 := mqn.fillin([a="xx yy", c='yyy', d=[2:4]], trace=T);
	mqn1 := mqn.fillin([a=[-4:-6]], trace=T);
	mqn1.show('after substitute', trace=T);
    }

    if (F) {
	print s1 := mqn.nodename('mqp_parm', [a=T, b=2, c='xxx', d=F], append=T);
	print mqn.fillin([a=[-4:-6]], s1, trace=T);
	mqn.show('after substitute', trace=T);
    }

    if (F) {
	print s1 := mqn.nodename('mqp_parm', [a=T, b=2, c='xxx', d=F], append=T);
	# mqn.private().dissect(trace=T);
	# print '** .check() ->',mqn.check(trace=T);
	# print mqn.summary();
	mqn2 := mqn1.subset('*', [a=[-3:-5]], trace=T);
	mqn2.show('after subset', trace=T);
    }

    if (T) {
	mqn.quals();
    }

    return T;
}
# Execute the test-function:
# test_MeqName();

############################################################################
