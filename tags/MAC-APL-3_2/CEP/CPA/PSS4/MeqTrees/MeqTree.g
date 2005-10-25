# file: ../MeqTrees/MeqTree.g

#---------------------------------------------------------

# pragma include once
# print '\n\n\n\n\n\n=================================================';
print 'include MeqTree.g    h07/d08/h09/d10oct2003';

include 'unset.g';
include 'genericClosureFunctions.g';
include 'MeqExpr.g';
include 'MeqName.g';


############################################################################
# MeqTree: Glish record that contains a generic or specific tree 
#          in which each child is a named record etc
############################################################################



MeqTree := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqTree');


    private.init := function () {
	wider private, public;

	# Functions to handle expression (strings) are in MeqExpr:
	private.mqe := MeqExpr('MeqTree');

	# Functions to deal with node-names
	private.mqn := MeqName('MeqTree');

	# Since a MeqTree object can be a node in a tree, it has a number
	# of required fields with the same name and function of node fields
	# with the same name:
	public.index := 0;

	public.clear();
	return T;
    }


#===========================================================================
# Public interface:
#===========================================================================  

# Re-initialise the internal node repository:

    public.clear := function (trace=F) {
	wider private;
	if (trace) print '\n** .clear()\n';
	private.node := [=];
	private.node_temp := [=];                # temporary node buffer 
	public.consis();                         # initialises other vectors 
	return T;
    }
    
# Check whether any of the node names are (still) parametrized:

    public.parametrized := function (trace=T) {
	wider private;
	ff := field_names(private.node);
	private.parametrized := private.mqn.parametrized(ff);
	return private.parametrized;
    }

#=============================================================================
# Summary of the contents of this object: 
# NB: Called by genericClosureFunction .summary()

    private.summary := function () {
	ss := "";
	s := paste('contains a total of',len(private.node),'nodes');
	ss := paste(ss,'\n',s);
	root := public.root();
	for (i in ind(root)) {
	    s := spaste ('- root ',i,': ',root[i]);
	    ss := paste(ss,'\n',s);
	}
	return ss;
    }

#====================================================================
# Fill in the 'parametrized' qualifiers with the given values in 
# the fields of the record qual. This will usually lead to a larger
# nr of nodes. The result is returned in a new MeqTree object.

    public.fillin := function (qual=[=], trace=F) {
	funcname := private._.onentry('fillin()', input=[qual=qual], 
				      trace=trace);

	mqt := public.new();                      # create output MeqTree object

	for (name in public.root()) {
	    if (trace) {
		s := paste('\n root:',name);
		private._.message(s, trace=trace);
	    }
	    private.fillin (mqt, name, qual, trace=trace);
	}

	# Check the consistency of new MeqTree object:
	mqt.consis(trace=trace); 

	# Check whether some node-names are still parametrized:
	ff := mqt.parametrized(trace=trace);

	# Finished: return new MeqTree object:
	if (trace) mqt.show(trace=trace);
	return private._.onexit(funcname, mqt, trace=trace);
    }

# Recursive function that does the actual work:

    private.fillin := function (ref mqt=F, name=F, qual=[=], level=1, trace=F) {

	# Produce a vector of one or more specific node names:
	names := private.mqn.fillin(qual, name);
	if (trace) {
	    s := paste('node:',name,'->',len(names),'new node name(s)');
	    private._.message(s, indent=level, trace=trace);
	}
	
	# For each of these new node-names:
	for (j in ind(names)) {
	    rr := private.node[name];  
	    rr.name := names[j];                  # new node name
	    append := T;

	    # Modify any child-names, using the qual-record (qq.qual)
	    # from the dissected node-name:
	    if (len(rr.child)>0) {
		qq := private.mqn.dissect(names[j]); # -> classname/qual
		if (trace) {
		    s := paste(j,': dissect -> qq =',qq);
		    private._.message(s, indent=level+1, trace=trace);
		}
		
		for (i in ind(rr.child)) {        # child node names
		    cnew := private.mqn.fillin(qq.qual, rr.child[i]);
		    if (trace) {
			s := paste('rr.child[',i,'] =',rr.child[i],'->',cnew);
			private._.message(s, indent=level+2, trace=trace);
		    }
		    if (len(cnew)==1) {           # OK, expected
			# Recursive: 
			private.fillin (mqt, rr.child[i], qq.qual, 
					level=level+3, trace=trace);
			rr.child[i] := cnew;      # new child name
		    } else {                      # Should not happen
			s := paste('len(cnew)=',len(cnew),':',cnew);
			private._.message(s, indent=level+2, error=T);
			rr.child[i] := spaste('error_',rr.child[i]);    #...??
			# append := F;
		    }
		}
	    }
	    
	    # Append new node to the output mqt, if required:
	    if (append) mqt.append(rr, trace=F);
	}
	return T;
    }

#====================================================================
# Check whether any of the node names are still parametrized:

    public.parametrized := function (trace=F) {
	funcname := private._.onentry('parametrized()', trace=trace);
	names := field_names(private.node);
	r := private.mqn.parametrized(names, trace=trace);
	if (r) {
	    if (trace) print '\n** .parametrized(): some';
	} else {
	    if (trace) print '\n** .parametrized(): OK, none\n';
	}
	return private._.onexit(funcname, r, trace=trace);
    }
    

#====================================================================
# Create a new node. 
# If hold=T, keep the new node in the temp-buffer until later. This is to allow 
# expressions in which one of the inputs (e.g. matrices) is overwritten.
    
    public.node := function (name=F, qual=F, expr=F, aux=[=], hold=F, origin=F, trace=F) {
	wider private;
	s := paste('\n** .node(',name,qual,expr,aux,hold,'): ');
	if (trace) print s,' origin=',origin;
	
	# Check the input type:
	if (is_string(name)) {
	    # Normal, proceed
	} else if (public.same_type(name)) {
	    # MeqTree (with one tree only?)
	} else if (is_record(name)) {
	    # Assume node record?
	} else {
	    # Error
	}

	# Append qualifiers[..][..] to the node-name, if specified:
	if (!is_boolean(qual)) name := private.mqn.nodename(name, qual);

	# Initialise a new node-record in the temp-buffer:
	node := private.mqe.expr2node(expr);
	node.name := name;
	node.aux := aux;
	if (F) {
	    # Extract the child names {<cname>} from expr, and convert to {#n}
	    # NB: Done already by private.mqe.expr2node()
	    private.mqe.cseqform(node, T, trace=trace);
	}
	private.node_temp[name] := node;

	# Two modes:
	if (hold) {                           # keep in the temp-buffer
	    if (trace) print '** .node(hold=',hold,'):',private.node_temp[name],'\n';
	    # Return the node {name} as an expression, 
	    # for compatibility with .matrix etc (see prefab) 
	    exprout := spaste('{',name,'}');
	} else {                              # flush the temp-buffer
	    exprout := public.flush_node_temp (trace=trace);
	    if (is_fail(exprout)) print exprout;
	}
	return exprout;
    }

# Helper function to flush the temporary node buffer:

    public.flush_node_temp := function (trace=F) {
	wider private;
	# trace := T;
	tnames := field_names(private.node_temp);
	s := paste('.flush_node_temp()');
	if (trace) print '\n***********\n<<< start of:',s,len(tnames),tnames;
	if (len(tnames)<=0) return T;                # not needed

	for (name in tnames) {
	    # Gives segmentation fault...!
	    # private.node_consis(private.node_temp[name], new=T, trace=trace);
	}
	# Check whether any of the nodes in private.node_temp exists already:
	for (name in tnames) {
	    if (trace) print '\n- check existence of node:',name;
	    if (!has_field(private.node,name)) {
		# Node (name) does not yet exist: create.
		print '** create new node {',name,'}';
	    } else if (!is_record(private.node[name])) {
		# Existing node, but invalid (??): overwrite
		s1 := type_name(private.node[name]);
		print '** overwrite INVALID (?!) existing node {',name,'} of type:',s1;
	    } else {
		# Node already exists: overwrite it with the one from temp.
		# But first substitute all references to it in node_temp (!)
		# with the expression (expr) of the original node: 
		expr := private.node[name].expr;
		print '** substitute refs to existing node {',name,'} with its expr:',expr;
		for (i in ind(private.node_temp)) {
		    if (trace) print '\n <<< temp {',private.node_temp[i].name,'}:',private.node_temp[i].expr;
		    if (T) {
			private.mqe.child2expr (private.node_temp[i], 
						cname=name, expr=expr, trace=F); 
		    } else {
			rr := private.substitute_with_expr (private.node_temp[i], 
							    name=name, expr=expr, trace=F);
			private.node_temp[i] := rr;
			
		    }
		    if (trace) print '>>> substituted:   ',private.node_temp[i].expr;
		}
	    }
	    if (trace) print '- \nfinished checking existence of node',name;
	}
	# print s,'after exist-check\n';

	# Copy the nodes from private.node_temp to private.node:
	exprout := "";
	for (name in tnames) {
	    exprout := [exprout,spaste('{',name,'}')];
	    if (has_field(private.node,name)) {
		print '** overwrite existing node {',name,'}';
	    }
	    private.node[name] := private.node_temp[name];
	    private.mqe.cseqform(private.node[name], T, trace=trace);
	}
	private.node_temp := [=];             # reset the temp buffer
	# print s,'after copying, node_temp=',private.node_temp;

	# NB: Call node_consis AFTER resetting the node_temp buffer
	for (name in tnames) {
	    if (trace) print '-- node_consis():',name;
	    private.node_consis(private.node[name], new=T, trace=trace);
	}
	public.consis(trace=trace);
	if (trace) print '>>>',s,'finished:',exprout,'\n**********\n';
	return exprout;
    }

# Helper function to substitute all references to {<name>} in the expression of 
# the given node with the given expression. 
# NB: This is similar to what is done in .bypass() below, but the expressions
#     in the temp_nodes are not yet in {#n} form. 

    private.substitute_with_expr := function (ref node=F, name=F, expr=F, trace=F) {
	if (!is_record(node)) return F;
	s := paste('\n  ** .substitute_with_expr(',node.name,name,expr,')');
	if (trace) print s;

	# Check whether node.expr contains the string {<name>}:
        # NB: Glish eval() only works on global variables:
	global symbex_temp;
	s1 := spaste('symbex_temp ~ m/{',name,'}/');
	symbex_temp := node.expr;               # copy to global
	if (!eval(s1)) return F;                # not matched
	
	# Debugging only:
	if (trace) ss := private.mqe.terms (node.expr, trace=T);

	# Substitute all occurences of {<name>} with (<expr>):
	expr := private.mqe.enclose(expr);      # enclose () if necessary
	s2 := spaste('symbex_temp ~ s/{',name,'}/',expr,'/');
	expr := eval(s2);                       # substitute
	if (is_fail(expr)) {                    # problem..
	    print 'failed: eval(',s2,')';
	    print expr;
	    return F;
	} 
	# OK, replace the node.expr with the new version:
	if (trace) print '  eval(',s2,') ->',expr;
	node.expr := expr;
	# NB: do NOT call private.node_consis(node) here!!
	return T;                               # return T if any substituted
    }

#================================================================================
# Generate dummy values for leaf-objects, when not specified:

    public.dummy_value := function (value=F, type=F, init=F, trace=F) {
	wider private;
	if (!has_field(private,'dumval')) {
	    private.dumval := [ampl=1.0, arg=0.0];
	}
	if (is_boolean(type)) type := 'real';
	if (!is_boolean(value)) {
	    if (type=='real') value := as_double(value);
	    if (type=='complex') value := as_complex(value);
	} else if (type=='real') {
	    private.dumval.ampl +:= 0.1;
	    value := private.dumval.ampl;
	} else if (type=='complex') {
	    private.dumval.ampl +:= 0.1;
	    private.dumval.arg +:= pi/10;
	    value := complex(cos(private.dumval.arg),sin(private.dumval.arg));
	    value *:= private.dumval.ampl;
	} else {
	    print '.dummy_value: type not recognised:',type;
	    value := 987;
	}
	return value;
    }

#-------------------------------------------------------------------------------
# Helper function to check the internal consistency of a node:

    private.node_consis := function (ref rr=F, new=F, trace=F) {
	wider private;
	if (trace) print '** .node_consis()',type_name(rr);
	if (!is_record(rr)) return F;
	s := paste('** .node_consis(',rr.name,'):');
	if (trace) print s;

	ff := field_names(private.node);
	rr.index := ind(ff)[ff==rr.name];     # its index in private.node

	# Some fields are only set in the beginning:
	if (new) {                          
	    rr.label := '<node-label>';
	    rr.is_leaf := F;                  # T if leaf node (parm, spigot, ..)
	    rr.evaluate := T;                 # if T, evaluate again
	    rr.bypassed := F;                 # T if bypassed (see .bypass())
	    rr.range := F;                    # result (can be numeric or tf_xxx...)
	    rr.ncall := 0;                    # statistics
	    rr.neval := 0;                    # statistics
	    rr.nerr := 0;                     # statistics
	}
	if (trace) print s,'rr=\n',rr;

	# Extract the child-names from expr into a vector rr.child, 
	# and replace them with sequence nrs (in cc.child):
	private.mqe.cseqform (rr, T, trace=trace);
	
	# Make sure that each child name points to an existing field in private.node:
	public.ensure_nodes(rr.child, trace=trace);
	if (trace) print s,len(rr.child),'child(ren):',rr.child;
	rr.is_leaf := (len(rr.child)==0);     # no children: is tree-leaf 

	# Finished:
	if (trace) print rr,'\n';
	return T;
    }

#-------------------------------------------------------------------------
# Make sure that each name points to an existing field in private.node:
	
    public.ensure_nodes := function (name=F, trace=F) {
	for (cname in name) {
	    if (!has_field(private.node,cname)) {    # field does not exist yet
		value := public.dummy_value();       # type=real (complex?)
		expr := spaste(value);               # non-zero dummy value
		public.node(cname, expr=expr, trace=trace);
		print paste(' place-holder node {',cname,'} expr =',expr);
	    } else if (trace) {                      # Field exists, do nothing?
		print paste('** ensure_nodes:',cname,'exists');
	    }
	}
	return T;
    }
	

#====================================================================
# Make a new MeqTree object:

    public.new := function (name=F, history=F, copy=F, trace=F) {
	new := MeqTree(name);
	if (copy) new.insert(private.node, clear=T);  # copy the current tree(s)
	if (is_string(history)) new.history(history);
	if (trace) print new.summary('created new MeqTree');
	return new;
    } 


# Split into a record of MeqTree objects with one tree each.
# Each tree has all the nodes it needs:

    public.split := function (trace=F) {
	rr := [=];                                    # output record
	for (rname in public.root()) {                # for all root-name(s)
	    rr[rname] := public.new(rname, trace=trace);
	    rr[rname].insert(public.tree(rname), clear=T);
	}
	if (trace) for (tt in rr) tt.show(trace=T);
	return rr;                                    # record of single-tree MeqTree objects
    }

# Remove nodes that have neither parents nor children:

    public.remove_orphans := function (trace=F) {
	wider private;
    }

# Remove the named nodes from private.node.
# If new=T, return the result in a new MeqTree object. 

    public.delete := function (name=F, new=F, trace=F) {
	trace := T;
	wider private;
	if (trace) print '\** .delete(',name,'):';
	if (!is_string(name)) return F;
	nn[1] := len(name);
	nn[2] := len(private.node);
	for (i in ind(name)) {
	    name[i] := private.mqe.remove_curly_brackets(name[i]);
	}
	for (fname in name) {
	    if (!has_field(private.node,fname)) {
		print '- field not in private.node:',fname;
	    } else {
		private.node[fname] := F;             # mark for delete
	    }
	}
	nn[3] := len(private.node);
	rr := [=];
	for (fname in field_names(private.node)) {
	    if (is_boolean(private.node[fname])) next;
	    rr[fname] := private.node[fname];
	}
	private.node := rr;
	nn[4] := len(private.node);
	nn[5] := nn[3]-nn[2];                         # should be zero!
	nn[6] := nn[2]-(nn[4]+nn[1]);                 # should be zero!
	if (trace) print '   -> nn=',nn,'\n';
	public.consis();
	return T;
    }

# Get a tree (subset of private.node), starting at the specified root(name):

    public.tree := function (root=F, trace=F) {
	tree := [=];                                  # output record
	if (!is_string(root)) {
	    # error (take the first?)
	} else if (!has_field(private.node,root)) {
	    # error
	} else {
	    private.tree(tree, private.node[root]);   # collect nodes recursively
	}
	return tree;                              
    } 

    private.tree := function (ref tree=F, rr=F, trace=F) {
	for (cname in rr.child) {                     # all child nodes
	    private.tree(tree, private.node[cname]);  # recursive
	}
	tree[rr.name] := rr;                          # parent node
	return T;
    } 

# Just append/overwrite a node to private.node:
# No further checks...

    public.append := function (node=F, trace=T) {
	wider private;
	if (trace) {
	    print '\n** .append(',node.name,'):\n    ',node,'\n';
	}
	private.node[node.name] := node;
	return node.name;
    }

# Insert a tree (record of nodes) by simply appending/overwriting fields:

    public.insert := function (tree=F, clear=F, merge=F, trace=F) {
	wider private;
	if (clear) public.clear();                   # clear first
	if (public.same_type(tree)) {                # input is a MeqTree object
	    root := tree.root();
	    if (len(root)==1) {                      # contains only one tree
		private.node[root] := tree;          # attach MeqTree object
	    } else if (len(root)>1) {                # contains multiple trees
		rr := tree.split();                  # split into MeqTree objects (?)
		for (cc in rr) {
		    rname := cc.root();
		    private.node[cc.root()] := cc;   # attach MeqTree object 
		    private.node[rname] := cc.tree(rname); 
		} 
	    }
	    if (merge) public.merge(trace=trace);    # merge the MeqTrees

	} else {                                     # assume that tree is a tree
	    private.merge (tree, trace=trace);
	}
	return T;
    } 

# Merge with any MeqTree objects that are attached:

    public.merge := function (trace=T) {
	ss := public.MeqTree();                      # names of nodes that are MeqTree objects
	if (trace) print '\n** .merge():',len(ss),'MeqTree(s):',ss;
	for (tname in ss) {
	    if (trace) print ' -',tname;
	    rr := private.node[tname];               # assume MeqTree object
	    private.merge(rr.tree(tname), trace=trace);
	}
	return T;
    }

# Merge a tree, i.e. a record of nodes:
# Append/override named nodes in private.node.

    private.merge := function (tree=F, trace=F) {
	wider private;
	for (fname in field_names(tree)) {
	    if (trace) print '   -',fname;
	    private.node[fname] := tree[fname];  # append/override node
	}
	public.consis(trace=trace);
	return T;
    }

#========================================================================
# Find the name(s) of the root node(s) of the tree(s):
# A root node is one without parents.

    public.root := function (single=F, trace=F) {
	sv := (private.node_has_parent==0);      # sv=T for orphan nodes
	s := paste('.root nodes:',len(sv[sv]),'/',len(sv));

        # If single=T, return only the subset that has no children. 
	if (single) {
	    # Assume that MeqTree objects are not single:
	    sv &:= !private.node_is_MeqTree;     # exclude MeqTree objects
	    for (i in ind(private.node)) {
		# Include (sv=T) orphan nodes without children: 
		if (sv[i]) sv[i] := (len(private.node[i].child)==0);
	    }
	    s := paste('.single nodes:',len(sv[sv]),'/',len(sv));
	}

	if (!any(sv)) {
	    print '\n**',s,': no such nodes! \n';
	    return "";
	}
	if (trace) print s;
	return field_names(private.node)[sv];
    }

# Find the name(s) of the node(s) that are MeqTree object(s):

    public.MeqTree := function (trace=F) {
	sv := private.node_is_MeqTree;
	s := paste('.MeqTree:',len(sv[sv]),'/',len(sv));
	if (!any(sv)) {
	    print '**',s,': no MeqTree objects';
	    return "";
	}
	if (trace) print s;
	return field_names(private.node)[sv];
    }

# Check the internal consistency of the tree(s):
# Also update the overall vectors.
# NB: A node is the start-point of a tree if it has no parents:

    public.consis := function (trace=F) {
	wider private;
	if (trace) print '\n** .consis():';

	# Initialise the overall bookkeeping vectors:
	n := len(private.node);
	if (n==0) {
	    private.node_is_node := [];         
	    private.node_is_MeqTree := [];
	    private.node_is_child := [];
	    private.node_has_parent := [];
	} else {
	    private.node_is_node := rep(F,n);
	    private.node_is_MeqTree := rep(F,n);
	    private.node_is_child := rep(0,n);
	    private.node_has_parent := rep(0,n);
	}

	# First pass through the nodes:
	for (i in ind(private.node)) {
	    rr := ref private.node[i];            # convenience
	    rr.index := i;                        # MeqTree objects too!                             

	    if (public.same_type(rr)) {           # MeqTree object
		private.node_is_MeqTree[i] := T;
		if (trace) print '   -',i,':',rr.label();

	    } else {
		private.node_is_node[i] := T;
		# check/update node
		# NB: This also ensures that all children exist!
		private.node_consis(private.node[i]); 
	    }
	}

	# Second pass through the nodes:
	for (i in ind(private.node)) {
	    if (!private.node_is_node[i]) next;   # not a node, ignore
	    rr := ref private.node[i];            # convenience
	    if (trace) print '  -',i,':',rr.name,': children=',rr.child;
	    for (cname in rr.child) {            
		j := private.node[cname].index;   # its index in private.node
		private.node_is_child[j] +:= 1;   # is child at least once
		private.node_has_parent[j] +:= 1; # has at least one parent
	    }
	}

	# Third pass through the nodes:
	if (F) {
	    for (i in ind(private.node)) {
		if (!private.node_is_node[i]) next;   # not a node, ignore
		rr := ref private.node[i];            # convenience
	    }
	}

	if (trace) print ' ';
	return T;
    }

#====================================================================
# Evaluate a named node recursively (like .getResult()):

    public.evaluate := function (name=F, domain=F, trace=F) {
	wider private;

	# Check the overall tree consistency
	public.consis();   

	s := paste('.evaluate(',private.its.name,'):');
	if (trace) print '\n***',s,' domain=',domain; 

	# If no start-name specified, use the root(s):
	if (is_boolean(name)) {
	    name := public.root();
	    if (is_boolean(name)) return F;    
	}

	# Glish eval() works on GLOBAL variables only:
	global MeqTree_eval;
	MeqTree_eval := [=];                         # see private.evaluate()
	for (i in ind(private.node)) {
	    if (public.same_type(private.node[i])) {
		print s,i,': node is MeqTree object.....!!?';
	    } else {
		MeqTree_eval[i] := F;                # create a slot
		# Do at least one evaluation of each field (node):
		private.node[i].evaluate := T;       # .....
		# The node index nr is used in private.evaluate()!!
		private.node[i].index := i;          # just in case
	    }
	}

	# OK, evaluate, starting at the named field(s):
	for (i in ind(name)) {
	    fname := name[i];
	    fname := private.mqe.remove_curly_brackets (name[i]);
	    private.evaluate(private.node[fname], domain, trace=trace);
	    s1 := private.format_range(private.node[fname]);
	    if (trace) print '  ** evaluate MeqTree:',s1;
	}
	return T;
    }

# Recursive function that does the actual evaluation:

    private.evaluate := function (ref rr=F, domain=F, level=1, trace=F) {
	prefix := private.prefix(level);
	s := paste(prefix,'.evaluate(',rr.name,'):');
	if (trace) print s,'entry';

	# Check the Glish expression:
	if (!is_string(rr.expr)) {
	    rr.nerr +:= 1;                         # book-keeping
	    print s,'rr.expr not a string, but:',type_name(rr.expr);
	    return F;
	}

	# Evaluate if necessary:
	global MeqTree_eval;
	rr.ncall +:= 1;                            # book-keeping
	if (!rr.evaluate) {                        # no value yet
	    if (trace) print s,'already evaluated';
	    MeqTree_eval[rr.index] := rr.range;    # just in case

	} else {
	    rr.neval +:= 1;                        # book-keeping

	    # Make sure that all its children have values:
	    for (cname in rr.child) {
		v := private.evaluate(private.node[cname], domain, 
				      level=level+1, trace=trace);
		if (is_fail(v)) print v;
		if (is_boolean(v)) return F;       # problem
	    }

	    # Evaluate its own expression:
	    expr := rr.expr;
	    for (i in ind(rr.child)) {
		j := private.node[rr.child[i]].index;
		s1 := spaste('\'',expr,'\' ~ s/{\\\#',i,'}/MeqTree_eval[',j,']/g');
		if (trace) print '-',i,' s1=',s1;
		expr := eval(s1);
		if (trace) print '      eval() ->',expr;
	    }

	    v := eval(expr);
	    sv := as_string(v);
	    if (is_fail(v)) {                      # ....failed....
		print '\n   **',s,'-> failed (!!) ***\n';
		print v;
	    } else if (any(sv=="nan nannani")) {
		print '\n   **',s,'-> (not a number):',sv,' (!!) ***\n';
	    } else if (any(sv=="inf")) {
		print '\n   **',s,'-> (infinite):',sv,' (!!) ***\n';
	    } else {                               # OK, successful
		if (trace) print '-',s,'->',v;
	    }
	    MeqTree_eval[rr.index] := eval(expr);
	    
	    private.range (rr, value=v, trace=F);  # update
	    rr.evaluate := F;                      # avoid extra work
	}

	# Finished: 
	return private.range(rr);                  # return NUMERIC value...(?)
    }


#===========================================================================
# Helper function: Get/set the NUMERIC value (range) of the given node (rr):
# rr.range can be numeric, or any of a number of tf_classes (e.g. tf_range)

    public.range := function (value=unset, txt=F, trace=F) {
	s := paste('** .range(value=',value,'):');
	if (trace) print '\n\n**',s,txt;
	for (rr in private.node) {
	    v := private.range (rr=rr, value=value, trace=trace);
	}
	if (trace) print ' ';
	return T;
    }

    private.range := function (ref rr=F, value=unset, trace=F) {
	wider private;
	s := paste('** .range(',rr.name,'):');
	s := spaste(s,' (range=',type_name(rr.range),')');
	if (is_record(rr.range)) {
	    # tf_range, tf_polc, tf_result .....
	    # if (!is_unset(value)) ....
	    # value := ...;
	} else {
	    if (!is_unset(value)) rr.range := value;
	    value := rr.range;
	}
	if (trace) {
	    s := spaste(s,'=',type_name(value),'[',shape(value),']');
	    if (len(value)==1) s := paste(s,'=',value);
	    print s;
	}
	return value;
    }

    private.format_range := function (rr=F, trace=F) {
	s := paste('** .range(',rr.name,'):');
	s := spaste(s,' (range=',type_name(rr.range),')');
	value := private.range(rr);
	s := spaste(s,'=',type_name(value),'[',shape(value),']');
	if (len(value)==1) s := paste(s,'=',value);
	if (trace) print s;
	return s;
    }



#========================================================================
# Bypass a named node by replacing all references to it by other nodes 
# with its own internal expression.
# Multiple nodes are bypassed if name is a vector/array. 

    public.bypass := function (name=F, trace=F) {
	wider private;
	funcname := private._.onentry('bypass()', input=[name=name], 
				      trace=trace);
	name::shape := len(name);                         # make 1D
	s := paste('** .bypass(',name,'):');

	public.consis();                                  # just in case

	# Store the evaluated values for comparison afterwards
	private.compareval(init=T, trace=trace);
	if (trace) public.show('before bypass', opt=[full=T], trace=T);

	for (bname in name) {                        
	    # In some cases, the names are still encased in brackets:
	    bname := private.mqe.remove_curly_brackets(bname);
	    if (trace) print s,bname;
	    if (!has_field(private.node, bname)) {
		print s,'no such node:',bname;
	    } else if (!has_field(private.node[bname],'is_leaf')) {
		print s, bname,': MeqParm objects cannot be bypassed!'; 
	    } else if (private.node[bname].is_leaf) {
		print s, bname,': leaf (end-point) nodes cannot be bypassed!'; 
	    } else {
		# Replace the {#n} in the expr of node[bname] with its child names:
		# was := private.node[bname].cseqform;            # keep for later
		node := private.node[bname]; 
		private.mqe.cseqform(node, F, trace=trace); 
		expr := private.mqe.enclose(node.expr);         # enclose () if necessary
		if (trace) print '\n--- replace:',bname,'with:',expr,':'; 
		for (i in ind(private.node)) {
		    if (!private.node_is_node[i]) {
			if (trace) print ' --',i,': not a node'; 
		    } else {
			private.mqe.child2expr (private.node[i], 
						bname, expr, trace=trace);
			private.node_consis(private.node[i]);   # adjust (?)
		    }
		}
		# private.mqe.cseqform(private.node[bname], was, trace=trace); 
		private.node[bname].bypassed := T;              # set switch (useful?)
	    }
	}
	if (trace) public.show('before delete', opt=[full=T], trace=T);
	public.delete(name, trace=trace);
	public.consis();                                        # done by .delete()
	if (trace) public.show('after bypass', opt=[full=T], trace=T);

	# Check whether the new tree evaluates to the same numbers:
        # The sum of the absolute diff (should be zero)
	absum := private.compareval(trace=trace);

	return private._.onexit(funcname, absum, trace=trace);
    }


#-------------------------------------------------------------------------
# Compare the evaluated results (e.g. before and after bypass).

    private.compareval := function (init=F, evaluate=T, trace=F) {
	wider private;
	if (trace) print '\n** .compareval(init=',init,evaluate,'):';
	if (evaluate) public.evaluate(trace=F);
	if (init || !has_field(private,'rrange')) private.rrange := [=];
	absum := 0;
	fmt := spaste('%',public.maxchar(),'s');
	for (fname in field_names(private.node)) {
	    new := private.node[fname].range;
	    s := spaste('  - {',sprintf(fmt,fname),'} = ');
	    s := paste(s,sprintf('%-25s',paste(new)));
	    if (init) {
		# Do nothing, just store (below)
	    } else if (!has_field(private.rrange,fname)) {
		print s,'no old value';
	    } else {
		diff := (new - private.rrange[fname]);
		absum +:= abs(diff);
		if (absum!=0) {
		    if (!trace) print '\n** .compareval():  ** problem detected **';
		    trace := T;
		}
		if (trace) {
		    ## s := paste(s,sprintf('min=%.3g',mm[1]));
		    s := paste(s,sprintf(' diff = %-6s',paste(diff)));
		    s := paste(s,sprintf(' absum = %-6s',paste(absum)));
		    if (trace) print s;
		}
	    }
	    private.rrange[fname] := new;      # keep for comparison
	}
	# Give the overall verdict:
	if (trace) {
	   if (init) {
	       print ' ';
	   } else {
	       s := paste('   .compareval() -> absum=',absum);
	       if (absum==0) s := paste(s,':   ** OK **');
	       if (absum!=0) s := paste(s,':   ** something wrong! (should be zero) **');
	       print s,'\n';
	   }
	}
	return absum;
    }

# Helper function to determine the max nr of chars in a vector of strings:

    public.maxchar := function (s=F, trace=F) {
	if (!is_string(s)) s := field_names(private.node);
	n := 0; 
	for (s1 in s) n := max(n,len(split(s1,'')));
	return n;
    }


#========================================================================
# Show the given (name) tree, or all:

# Overwrite the function called by the generic function public.show()

    private.show := function (opt=[=]) {
	if (!has_field(opt,'what')) opt.what := 'expr';
	ss := public.summary();
	s := private.show_what (name=F, opt=opt, trace=F);
	return paste(ss,'\n',s);
    }

# Function copied/adapted from symbex.g

    private.show_what := function (name=F, opt=[=], trace=F) {
	wider private;

	ss := "";                                  # accumulator string
	public.consis();              
	if (is_boolean(name)) {                    # no start-node(s)
	    name := public.root();                 # use root node(s)
	    if (is_boolean(name)) return F;    
	}
	private.show_what_node := [=];             # see .show_recursive()

	# Do for all specified (named) trees:
	for (i in ind(name)) {                     # start-node names
	    if (!has_field(private.node,name[i])) next;
	    rr := private.node[name[i]];             # start-node
	    # s := paste('\n \n - root: (',name[i],'):');
	    s := spaste('\n \n - root ',i,':');
	    if (trace) print s;
	    ss := spaste(ss,'\n',s);
	    ss := private.show_recursive(rr=rr, ss=ss, opt=opt, trace=trace);
	}
	if (trace) print ' ';
	ss := paste(ss,'\n');

	if (T) ss := paste(ss,private.show_node_reuse(trace=trace));

	# Finished:
	if (trace) print ' ';
	return paste(ss,'\n');
    }

# Show the nodes of a tree (recursively):

    private.show_recursive := function (rr=F, ss=F, level=1, cseq=F, opt=[=], trace=F) {
	wider private;
	if (!has_field(opt,'what')) opt.what := 'expr';
	if (!has_field(opt,'full')) opt.full := F;
	if (!has_field(opt,'cseqform')) opt.cseqform := T;
	is_node := T;
	if (public.same_type(rr)) {            # node is MeqTree object
	    s := paste('object:',rr.label());
	    is_node := F;
	# } else if (rr.bypassed) {
	#     s := paste('(bypassed)');
	} else {
	    private.mqe.cseqform(rr, opt.cseqform, trace=F);
	    s := private.oneliner(rr, type=opt.what, cseq=cseq);
	}
	prefix := private.prefix(level);
	s := paste(prefix,s);
	if (trace) print s;
	ss := paste(ss,'\n',s);

	if (is_node) {
	    for (i in ind(rr.child)) {
		cname := rr.child[i];
		# Only show each node once:
		if (!has_field(private.show_what_node,cname)) {  # Only show each node once
		    private.show_what_node[cname] := F;          # Inhibit next time
		} else if (!opt.full) {                      
		    next;
		}
		ss := private.show_recursive(rr=private.node[cname], 
					     ss=ss, level=level+1, cseq=i, 
					     opt=opt, trace=trace);
		if (is_fail(ss)) print ss;
	    }
	}
	return ss;
    }

# Prefix formatter:

    private.prefix := function (level=1) {
	prefix := '  .  ';
	if (level>1) prefix := spaste(prefix,rep('. ',level-1));
	return prefix;
    }

# Formatted line of info:

    private.oneliner := function (rr=F, type='expr', cseq=F, ncmax=50) {
	s := spaste('node {',rr.name,'}:');
	if (rr.is_leaf) s := spaste('leaf {',rr.name,'}:');

	if (type=='expr') {
	    if (cseq>0) s := spaste('{#',cseq,'} ',s);        # child seq nr
	    # expr := rr.expr;
	    cc := split(rr.expr,'');                          # split into chars
	    ss := private.mqe.terms (rr.expr, tbreak="+ - * / ^", trace=F);
	    if (len(cc)<ncmax || len(ss)==1) {
		s := spaste(s,' expr= \'',rr.expr,'\'');
	    } else {
		s := spaste(s,' expr=');
		concat := F;
		for (s1 in ss) {                              # terms
		    cc1 := split(s1,'');
		    cc2 := ' '; if (len(cc1)>1) cc2 := spaste(cc1[1:2]);
		    cc3 := ' '; if (len(cc1)>2) cc3 := spaste(cc1[1:3]);
		    # concat := F;      # .....disable concat....                    
		    if (concat) {
			s := spaste(s,s1);
			concat := F;
		    } else if (s1==' + ' || s1==' - ') {
			s := spaste(s,s1);                    # append +/-
		    } else if (s1==' * ' || s1=='*') {
			s := spaste(s,s1);                    # append *
			concat := T;                          # append the next term
		    } else if (cc1[1]=='*' || cc2==' *') {
			s := spaste(s,s1);
		    } else {
			s := paste(s,'\n             ',s1);
		    }
		}
	    }

	} else if (type=='range') {
	    v := private.range(rr);
	    s := spaste(s,' (',type_name(rr.range),')');
	    s := paste(s,'=',type_name(v),shape(v));
	    if (len(v)==1) s := paste(s,'=',v);
	} else if (type=='child') {
	    s :=  paste(s,' child=',rr.child);
	    s := spaste(s,' leaf=',rr.is_leaf);
	} else {
	    s := paste(s,'not recognised:',type);
	}
	return s;
    }

# Format the re-use count (see .show_what()):

    private.show_node_reuse := function (trace=F) {
	ss := paste('\n * multiple use of nodes:');
	if (trace) print ss;
	count := 0;
	for (i in ind(private.node)) {
	    if (private.node_is_child[i]>1) {
		count +:= 1;
		s := paste('   -',private.node_is_child[i],'times:');
		s := spaste(s,' node {',private.node[i].name,'}');
		ss := spaste(ss,'\n',s);
		if (trace) print s;
	    }
	}
	if (count==0) return "";
	return ss;
    }

#========================================================================
# Generate C++ code:

    public.generate_cpp := function (name=F, trace=F) {
	if (trace) print '\n** .generate_cpp(',name,'):';
	if (!is_string(name)) name := public.root();
	rr := [=];
	for (fname in name) {
	    node := private.node[fname];
	    cc := private.mqn.dissect(node.name);
	    cname := cc.classname;
	    if (!has_field(rr,cname)) {
		rr[cname] := private.mqe.generate_cpp(node);
		if (trace) print '- classname =',cname,': expr =\n  ',rr[cname];
	    }
	}
	return rr;
    }

#=====================================================================
# Weed out terms that are zero etc from the given expression(s):

    public.weedout := function (expr=F, trace=F, ttrace=F) {
	wider private;
	if (trace) print '\n ** .weedout(',type_name(expr),shape(expr),'):';
	if (is_string(expr)) {
	    for (i in ind(expr)) {
		ss[i] := private.mqe.weedout (expr[i], trace=ttrace);
		if (trace) print '  - expr[',i,']:',expr[i],'\n            ->',ss[i];
	    }
	    ss::shape := shape(expr);
	    if (trace) print ' ->',type_name(ss),shape(ss);
	    return ss;

	} else if (is_record(expr)) {
	    # Assume expr is node... (not implemented yet)

	} else {
	    # Weed out the expressions in all nodes:
	    # Store the evaluated values for comparison afterwards
	    private.compareval(init=T, trace=trace);
	    for (i in ind(private.node)) {
		rr := private.node[i];
		expr := private.mqe.weedout(rr.expr, trace=ttrace);
		expr := private.mqe.deenclose(expr);                  #...??...
		if (trace) {
		    print '-',i,': weedout node {',rr.name,'}: expr=',rr.expr;
		    print '           ->',expr;
		}
		private.node[i].expr := expr;
	    }
	    # Check whether the new tree evaluates to the same numbers:
	    absum := private.compareval(trace=trace);
	}
	return F;
    }


#===========================================================================
# Some special cases:
#===========================================================================

#---------------------------------------------------------------------------
# Return a new MeqTree which is the weighted sum of all root nodes:

    public.wsum := function (trace=T) {
	child := public.root();
	new := public.new('wsum', copy=T);
	# public.node := function (name=F, qual=F, expr=F, aux=[=], hold=F, origin=F, trace=F) {
	expr := "";
	for (i in ind(child)) {
	    if (i>1) expr := spaste(expr,'+');
	    expr := spaste(expr,'{',child[i],'}');
	}
	new.node ('wsum', expr=expr);
	if (trace) new.show(trace=T);
	return new;
    }


#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}






############################################################################
# test-function:
############################################################################

test_MeqTree := function () {
    include 'inspect.g';
    print '\n\n****************************************************';
    print '** test_MeqTree:';

    # Create global object(s):
    print '\n** Create global objects';
    global mqn, mqt;
    print '\n',mqn := MeqName('mqn');
    print '\n',mqt := MeqTree('mqt');
    print ' ';

    if (F) {
	print '\n******************\nTest: node';
	mqt.node ('a', expr='pi');
	mqt.node ('b', expr='0');
	mqt.node ('u', expr='{1}');
	mqt.node ('z', expr='{0}');
    }

    if (F) {
	print '\n******************\nTest: bypass a node';
	mqt.node ('ss', expr='({a}^2)*{c}+{b}/2-3*{c}', trace=F);
	# mqt.node ('a', expr='[1]', trace=F);
	# mqt.node ('b', expr='pi', trace=F);
	mqt.node ('c', expr='4*{d}+1', trace=F);
	mqt.show(opt=[full=T, cseqform=T], trace=T);
	mqt.show(opt=[full=T, cseqform=F], trace=T);
	# mqt.bypass('a', trace=T);        # bypass leaf
	mqt.bypass('c', trace=T);              # bypass node
    }
	
    if (F) {
	print '\n******************\nTest: polynomial';
	mqt.node ('poly', expr='({a}^2)+{b}/2-3*{c}');
	mqt.node ('a', expr='[1:5]');      # multiple values allowed!
	mqt.node ('b', expr='pi');
	mqt.node ('c', expr='sin(3.7)');
	mqt.node ('d', expr='30');
	mqt.evaluate(trace=T);
    }

    if (F) {
	print '\n******************\nTest: trees';
	mqt.node ('poly1', expr='({a}^2)+{b}/2-3*{c}');
	mqt.node ('poly2', expr='2*({a}^2)+1*{c}');
	mqt.show('initial',trace=T);
	print '\n** mqt.root() ->',mqt.root();
	print '\n** mqt.MeqTree() ->',mqt.MeqTree();
	print '\n** mqt.merge() ->',mqt.merge(trace=T);
	rr := mqt.split(trace=T);
	mqt.insert(rr[2]);
	mqt.show('after .insert()',trace=T);
	mqt.merge();
	mqt.show('after .merge()',trace=T);
    }

    if (T) {
	print '\n******************\nTest: tree duplication';
	name := mqn.nodename('poly', [s=T, x=T]);
	# print 'cname=',cname := mqn.nodename('b', [s=5]);
	print 'a=',a := mqn.nodename('a');
	print 'b=',b := mqn.nodename('b', [s=T, x=T]);
	print 'c=',c := mqn.nodename('c', [x=T]);
	mqt.node (name, expr=spaste('({',a,'}^2)+{',b,'}/2-3*{',c,'}'));
	mqt.show('before',opt=[full=T], trace=T);
	mqt1 := mqt.fillin([s=[1:2], x=[-1:-2]], trace=T);
	# mqt1 := mqt.fillin([x=[1:2]], trace=T);
	mqt1.show('after', opt=[full=T], trace=T);
	# mqt1.evaluate(trace=T);
    }

    if (F) {
	print '\n******************\nTest: tree duplication';
	# predict := mqn.nodename('predict', [s1='{s1}', s2='{s1}+(1:3)', c=T]);
	# predict := mqn.nodename('predict', [s1='{s1}', s2='{s1}+(1:3)', c=T]);
	predict := mqn.nodename('predict', [s1='{s1}', s2='(({s1}+1):3)', c=T]);
	j1 := mqn.nodename('jones', [s='{s1}', c=T]);
	j2 := mqn.nodename('jones', [s='{s2}', c=T]);
	# j2 := mqn.nodename('jones', [s='{s1}-1', c=T]);
	mqt.node (predict, expr=spaste('{',j1,'} * conj({',j2,'})'));
	mqt.show('before',trace=T);
	# qual := [s1=[1:2], c=10];
	# qual := [s1=[1:2], s2=[4:5], c=10];
	qual := [s2=[4:5], s1=[1:4], c=[10]];
	# qual [s2=[4:5], c=10];
	# qual [s2=[4:5], c=10];
	print '\n** mqt.fillin(',qual,') -> mqt1';
	mqt1 := mqt.fillin(qual, trace=F);
	mqt1.show('after', opt=[full=T], trace=T);
	# mqt1.evaluate(trace=T);
    }

    if (F) {
	print '\n******************\nTest: qualified node';
	print 'child =',s := mqt.node ('child', qual=[c=T], expr='30');
	print 'parent =',mqt.node ('parent', qual=[c=T], expr=spaste(s,' + 5'));
    }

    if (F) {
	print '\n******************\nTest: wsum';
	I := sbx.prefab('IQUV');
	mqt.show ('before', opt=[full=T, cseqform=F], trace=T);
	mqt1 := mqt.wsum(trace=T);
	mqt1.show ('after', opt=[full=T], trace=T);
    }


    #--------------------------------------------------------------
    # Finished: common actions:
    #--------------------------------------------------------------
    if (T) {
	# mqt.range(trace=T);
	# mqt.show(opt=[full=T], trace=T);
	# mqt.save();                 # includes .show()
	# mqt.show('before .evaluate()', opt=[what='range'], trace=T);
 	# mqt.evaluate(trace=T);
	# mqt.show('after .evaluate()', opt=[what='range'], trace=T);
	# mqt.range(trace=T);
	# mqt.show('child', trace=T);
	# mqt.inspect();
	## mqt.print();
    }
    return T;
}
# Execute the test-function:
# test_MeqTree();

############################################################################
