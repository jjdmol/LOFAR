# file: ../MeqTrees/MeqArray.g

#---------------------------------------------------------

# pragma include once
# print '\n\n\n\n\n\n=================================================';
print 'include MeqExpr.g    h10oct2003';

include 'unset.g';
include 'genericClosureFunctions.g';
include 'MeqTree.g';




############################################################################
# MeqArray: Functions to manipulate arrays (vectors, mattrices) of
#             MeqExpr objects (symbolic expressions) 
############################################################################



MeqArray := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqArray', subset='*');

    private.init := function () {
	wider private;
	
	private.mqn := MeqName(public.type());
	private.mqe := MeqExpr(public.type());
	private.mqt := MeqTree(public.type());

	private.state.name := "";      # element names
	private.state.expr := "";      # their math expressions
	private.state.type := F;       # used in .tree()
	
	return T;
    }

#===========================================================================
# Public interface
#===========================================================================

    public.expr := function () {return public.state.expr}
    public.name := function () {return public.state.name}

    public.oneliner := function (expr=F) {
	s := "";
	return s;
    }

    private.show := function (opt=F) {
	ss := "";
	expr := private.state.expr;
	name := private.state.name;
	ss := paste(ss,'\n name[',shape(name),'] =',name);
	ss := paste(ss,'\n expr[',shape(expr),'] =',expr);
	for (i in ind(name)) {
	    s := paste('-',i,':',spaste(name[i]));
	    s := spaste(s,' = \'',spaste(expr[i]),'\'');
	    ss := paste(ss,'\n',s);
	}
	ss := paste(ss,'\n');
	return ss;
    }

    private.summary := function () {
	ss := "";
	expr := private.state.expr;
	s := paste(type_name(expr),'[',shape(expr),']');
	ss := paste(ss,'\n',s);
	return ss;
    }

#============================================================================
# Turn the (elements of the) expression array into (nodes of) a MeqTree:

    public.tree := function (name=F, trace=T) {
	trace := F;
	if (!is_string(name)) name := private.its.name;
	mqt := MeqTree(name);
	for (i in ind(private.state.name)) {         # all elements
	    s1 := spaste('{',spaste(private.state.name[i]),'}');
	    if (private.state.expr[i]==s1) {
		# The node should NOT be its own child
		# Or have itself as child? 
		# How do we deal with circularity in general?
		mqt.node (private.state.name[i], expr='-1', trace=trace);
	    } else {
		mqt.node (private.state.name[i], expr=private.state.expr[i], trace=trace);
	    }
	}
	return mqt;
    }

#====================================================================
# A symbex array (1D vector or 2D matrix) consists of a properly shaped 
# string array (exprout) of Glish expressions for the various elements. 
# The expessions can contain symbex nodes ({name}). 
# The result (exprout) can be fed into a matrix operation function 
# (like matprod() or dirprod()).
# The matrix dimensions are taken from the input expr (string) array,
# which contain the expressions for the various elements.

    public.vector := function (name='V', qual=F, expr=F, dim=F, suffix=F, type=F, trace=F) {
	wider private;
	s := paste('.vector(',name,dim,' suffix=',suffix,'):');
	if (trace) print s,' expr[',shape(expr),']=',expr;

	if (is_string(suffix)) dim := len(suffix);    # e.g. "I Q U V"

	if (is_string(expr)) {                        # expr given
	    dim := len(expr);
	    expr::shape := dim;                       # just in case
	} else if (is_boolean(dim)) {
	    return F;                                 # error message...?
	} else {                                      # automatic names
	    expr := array('{}',dim);
	}

	# The default element suffixes are numbers [1,2,3,...]
	if (!is_string(suffix)) suffix := as_string([1:dim]);
	if (len(suffix)!=dim) suffix := as_string([1:dim]);

	exprout := rep('0',dim);
	nameout := rep('-',dim);
	for (i in ind(expr)) {
	    nameout[i] := spaste(name,'_',suffix[i]);
	    if (!is_boolean(qual)) {
		nameout[i] := private.mqn.nodename(nameout[i], qual);
	    }
	    if (expr[i]=='{}') {
		exprout[i] := spaste('{',nameout[i],'}');
	    } else {
		exprout[i] := expr[i];
	    }
	}
	if (trace) print '   ->',type_name(exprout),'[',len(exprout),']:',exprout;
	private.state.expr := exprout;
	private.state.name := nameout;
	private.state.type := type;
	return private.state.expr;
    }

#==================================================================================
# Make a (nxm) matrix:

    public.matrix := function (name='M', qual=F, expr=F, dim=F, type=F, trace=F) {
	wider private;
	s := paste('.matrix(',name,qual,dim,'):');
	ndim := len(dim);
	if (trace) print s,' ndim=',ndim,' expr[',shape(expr),']=',expr;

	if (is_string(expr)) {                        # expr given
	    if (len(dim)!=2) dim := shape(expr);      # shape not specified
	    if (len(expr)!=prod(dim)) dim := shape(expr); # mismatch
	    expr::shape := dim;
	} else if (ndim!=2) {
	    return F;                                 # error message?
	} else {                                      # automatic names
	    expr := array('{}',dim[1],dim[2]);
	}
	if (trace) print '    dim=',dim,expr;

	exprout := array('0',dim[1],dim[2]);
	nameout := array('-',dim[1],dim[2]);
	for (i in [1:dim[1]]) {
	    # print 'i=',i;
	    for (j in [1:dim[2]]) {
		# print 'j=',j,':',expr[i,j];
		nameout[i,j] := spaste(name,'_',i,j);
		if (!is_boolean(qual)) {
		    nameout[i,j] := private.mqn.nodename(nameout[i,j], qual);
		}
		if (expr[i,j]=='{}') {
		    print 's1=',s1 := spaste('{',spaste(nameout[i,j]),'}');
		    exprout[i,j] := s1;
		} else {
		    exprout[i,j] := expr[i,j];
		}
	    }
	}
	if (trace) print '   ->',type_name(exprout),'[',len(exprout),']:',exprout;
	private.state.expr := exprout;
	private.state.name := nameout;
	private.state.type := type;
	return private.state.expr;
    }


#=================================================================
# Some special matrices:
#=================================================================

# Unit matrix:

    public.unit_matrix := function (name='U', qual=F, dim=2, type=F, trace=F) {
	wider private;
	s := paste('.unit_matrix(',name,dim,type,'):');
	if (is_boolean(dim)) {
	    return F;                                 # error message...?
	} else {                                      # automatic names
	    dim := dim[1];                            # just in case
	    if (is_boolean(type)) {
		expr := array('1',dim); 
	    } else if (type=='complex') {
		expr := array('1+0i',dim);
	    } else {
		expr := array('1',dim);
	    }
	}
	if (trace) print s,' expr[',shape(expr),']=',expr;
	return public.diagonal_matrix (name, qual=qual, expr=expr, type=type, trace=trace);
    }

# Diagonal matrix:

    public.diagonal_matrix := function (name='D', qual=F, expr=F, dim=2, type=F, trace=F) {
	wider private;
	s := paste('.diagonal_matrix(',name,dim,type,'):');
	if (trace) print s,' expr[',shape(expr),']=',expr;

	if (is_string(expr)) {                        # expr given
	    dim := len(expr);
	    expr::shape := dim;                       # just in case
	} else if (is_boolean(dim)) {
	    return F;                                 # error message...?
	} else {                                      # automatic names
	    dim := dim[1];                            # just in case
	    expr := array('{}',dim);                  # nodes with automatic names
	}

	ss := array('0',dim,dim);
	for (i in [1:dim]) ss[i,i] := expr[i];
	return public.matrix(name, qual=qual, expr=ss, type=type, trace=trace);
    }

# Tensor:

    public.tensor := function (name=F, qual=F, expr=F, dim=F, trace=F) {
	wider private;
	s := paste('.tensor(',name,dim,'):');
	if (trace) print s,' expr[',shape(expr),']=',expr;
	print s,'** NOT IMPLEMENTED YET! **';
	return F;
    }

#-----------------------------------------------------------------
# Transpose a matrix (string array):

    public.transpose := function(trace=F) {
	wider private;
	for (fname in "name expr") {
	    private.state[fname] := private.transpose (private.state[fname]);
	}
	return private.state.expr;
    }

    private.transpose := function(m) {
	dm := shape(m);
	if (len(dm)==2) { 
	    mt := array('0',dm[2],dm[1]);
	    for (i in [1:dm[1]]) {mt[,i] := m[i,]};
	    return mt;
	} 
	return F;
    }

#====================================================================
# Matrix/vector product: Generates a node for each element.
# Given by expression (string) arrays (e1,e2) of the correct shape.
# Return a properly shaped expression array that can be fed into
# a matrix operation function (like matprod() or dirprod()).
# The output is a vector if e2 is a vector.

    public.matprod := function (name=F, e1=F, e2=F, qual=F, suffix=F, trace=F) {
	d1 := shape(e1);	d2 := shape(e2);

	if (is_boolean(suffix)) {
	    suffix := as_string([1:d1[1]]);
	} else if (len(suffix)<d1[1]) {
	    suffix := as_string([1:d1[1]]);
	}

	s := paste('.matprod(',name,d1,d2,'):');
	if (trace) print s,'(input) suffix=',suffix;
	if (trace) print '-',d1,'e1=',e1;
	if (trace) print '-',d2,'e2=',e2;

	if (len(d1)==1) e1::shape := d1 := [1,d1];
	if (len(d2)==1) e2::shape := d2 := [d2,1];
	if (d1[2] != d2[1]) {
	    print s,'not commensurate',d1,d2;
	    return F;
	}
	s := paste('.matprod(',name,'):');
	if (trace) print s,' d1=',d1,' d2=',d2;

	rr := [exprout="", nameout=""];                    # output record
	exprout := array('0',d2[2],d1[1]);
	for (i in [1:d1[1]]) {
	    for (j in [1:d2[2]]) {
		if (trace) print '< i=',i,' j=',j,':';
		s3 := '0';
		for (k in [1:d1[2]]) {
		    if (trace) print '-- k=',k,':',e1[i,k],':',e2[k,j];
		    s4 := private.mqe.binop (e1[i,k],'*',e2[k,j], trace=trace);
		    if (trace) print '-- s4 =',type_name(s4);
		    if (is_fail(s4)) print s4;
		    s3 := private.mqe.binop (s3,'+',s4, trace=trace);
		    if (trace) print '-- s3 =',type_name(s3);
		    if (is_fail(s3)) print s3;
		}
		# Construct the node name:
		if (d2[2]==1) {
		    node := spaste(name,'_',suffix[i]);
		} else {
		    node := spaste(name,'_',i,j);
		}
		if (!is_boolean(qual)) node := private.mqn.nodename(node, qual);
		exprout[j,i] := spaste('{',node,'}');
		if (trace) print '> i=',i,' j=',j,':',exprout[j,i];
		if (!is_string(s3)) s3 := 'ffail';
		# r := private.mqt.node (node, expr=s3, hold=T, origin=s, trace=trace);
		# if (is_fail(r)) print r;
	    }
	}
	# r := private.mqt.flush_node_temp (trace=trace);  # flush the node buffer
	# if (is_fail(r)) print r;
        # The output is a vector if e2 is a vector.
	if (d2[2]==1) exprout::shape := d1[1]; 
	if (trace) print s,'exprout=',exprout;
	return exprout;                    # return expression (string) vector
    }


# Direct (Kronecker) product: Generates a node for each element.
# Given by expression (string) arrays (e1,e2) of the correct shape.
# Return a properly shaped expression array that can be fed into
# a matrix operation function (like matprod() or dirprod()).

    public.dirprod := function (name=F, e1=F, e2=F, qual=F, trace=F) {
	d1 := shape(e1);
	d2 := shape(e2);
	s := paste('.dirprod(',name,qual,d1,d2,'):');
	if (trace) print s,'(input)';
	if (len(d1)==1) e1::shape := d1 := [1,d1];
	if (len(d2)==1) e2::shape := d2 := [d2,1];
	if (d1[2] != d2[1]) {
	    print s,'not commensurate',d1,d2;
	    return F;
	}
	s := paste('.dirprod(',name,qual,d1,d2,'):');
	if (trace) print s,'(modified)';

	exprout := array('0', d1[1]*d2[1], d1[2]*d2[2]);
	for (i in [1:d1[1]]) {
	    ii := [(1+2*(i-1)):(2*i)];             # vector
	    for (j in [1:d1[2]]) {
		if (private.mqe.is_zero(e1[i,j])) next;          
		# s3 := spaste(e1[i,j]);
		s3 := e1[i,j];
		jj := [(1+2*(j-1)):(2*j)];         # vector
		for (i1 in ind(ii)) {
		    for (j1 in ind(jj)) {
			if (private.mqe.is_zero(e2[i1,j1])) next;          
			s5 := private.mqe.unop ('conj',e2[i1,j1]);
			s4 := private.mqe.binop (s3,'*',s5);
			node := spaste(name,'_',ii[i1],jj[j1]); # node name
			if (!is_boolean(qual)) node := private.mqn.nodename(node, qual);
			exprout[ii[i1],jj[j1]] := spaste('{',node,'}');
			if (trace) print '-',i,j,i1,j1,node,':',s4;
			private.mqt.node (node, expr=s4, hold=T, origin=s, trace=trace);
		    }
		}
	    }
 	}
	r := private.mqt.flush_node_temp (trace=trace);  # flush the node buffer
	if (is_fail(r)) print r;
	if (trace) print s,'exprout=',exprout;
	return exprout;                    # return expression (string) vector
    }

#------------------------------------------------------------------------
# Vector in-product: of 2 expression (string) vectors (e1,e2) of the same length.
# The output is an expression. Like .binop(), it does NOT produce a new node.

    public.inprod := function (e1=F, e2=F, trace=F) {
	d1 := shape(e1);
	d2 := shape(e2);
	s := paste('.inprod(',d1,d2,'):');
	if (len(d1)!=1 || len(d2)!=1 || d1!=d2) {
	    print s,'not commensurate',d1,d2;
	    return F;
	}
	exprout := '0';
	for (i in [1:d1]) {
	    exprout := private.mqe.binop (exprout,'+',private.mqe.binop (e1[i],'*',e2[i]));
	}
	if (trace) print s,'exprout=',exprout;
	return exprout;                    # return expression (string)
    }


# Binop should work on arrays, just like Glish.
# However, there is a difference with matprod etc in terms of (named) node generation!
# Obviously, we have not yet thought deeply enough.... 

    public.binop_multiple := function (e1=F, binop=F, e2=F, origin=F, trace=F) {
	s := paste('** .binop_multiple(',type_name(e1),len(e1),binop,type_name(e2),len(e2),'):');
	print s,'not implemented yet';
	return F;
	#-------------------------------------------------------------------
	if (!is_string(e1) || !is_string(e2)) {
	    print s,'type mismatch';
	    return F;
	} else if (len(e1)==len(e2)) {        # OK, same length
	} else if (len(e1)==1) {	
	    e1 := rep(e1,len(e2));            # make e1 same length as e2
	} else if (len(e2)==1) {
	    e2 := rep(e2,len(e1));            # make e2 same length as e1
	} else {
	    print s,'length mismatch';
	    return F;
	}
	# OK, do the actual binop:
	ss := "";
	for (i in ind(e1)) {
	    ss[i] := private.mqe.binop (e1=e1[i], binop=binop, e2=e2[i], 
					origin=origin, trace=trace);
	}
	return ss;
    }



#===========================================================================
# Finished:
#===========================================================================

    private.init();
    return public;
}





############################################################################
############################################################################
############################################################################
# test-function:
############################################################################

test_MeqArray := function () {
    include 'inspect.g';
    # include 'MeqName.g';
    print '\n\n****************************************************';
    print '** test_MeqArray:';

    # Create global object(s):
    print '\n** Create global objects';
    global mqn, mqa;
    # print '\n',mqn := MeqName('mqn');
    print '\n',mqa := MeqArray('mqa');
    print ' ';
    # mqa.inspect();

    if (F) {
	print '\n******************\nTest: all';
	expr := '{a} + {b} + {c}';
	for (fname in field_names(mqa)) {
	    if (!is_function(mqa[fname])) next;
	    if (any(fname=="inspect evalexpr")) next;
	    if (fname=='unop') {
		r := mqa.unop('abs',expr);
	    } else if (fname=='binop') {
		r := mqa.binop(expr,'+','4');
	    } else {
		r := mqa[fname](expr);
	    }
	    if (is_fail(r)) {
		print '\n** mqa.',fname,'(expr) -> ** FAILED **';
	    } else {
		print '\n** mqa.',fname,'(expr) ->', r;
	    }
	}
    }

#---------------------------------------------------------------

    qual := F;                  # default value (ignored)
    # qual := [a=T, b=[2], c=[d=T]];
    qual := [a=T, b=[2]];


    if (F) {
	print '\n******************\nTest: vector';
	v := mqa.vector('V', qual=qual, dim=4, trace=F);
	# v := mqa.vector('V', qual=qual, suffix="I Q U V", trace=F);
	# v := mqa.vector('V', qual=qual, expr="1 {} 3 pi 5", trace=F);
	# mqa.mqt().node ('g', qual=qual, expr='e', trace=F);
	# v := mqa.vector('V', qual=qual, expr="1 2 {g} pi 5", trace=F);
	print 'v=',v;
	mqa.show(trace=T);
	mqa.summary(trace=T);
	mqt := mqa.tree(trace=T);
	mqt.show(trace=T);
    }
    
    if (T) {
	print '\n******************\nTest: matrix';
	# print 'm=',m := mqa.unit_matrix(dim=4, trace=F);
	# print 'm=',m := mqa.unit_matrix(dim=4, type='complex', trace=F);
	# print 'm=',m := mqa.matrix('A', qual=qual, dim=[2,2], trace=F);
	print 'm=',m := mqa.matrix('T', qual=F, dim=[2,3], trace=F);
	# print 'm=',m := mqa.diagonal_matrix('M', qual=qual, dim=4, trace=F);
	# print 'm=',m := mqa.diagonal_matrix('M', qual=qual, expr="1 {g} {} 4", trace=F);
	# print 'm=',m := mqa.matrix('A', qual=qual, expr="0 7*0 {} {a}+{b}", dim=[2,2], trace=F);
	# print 'm1=',m1 := mqa.mqt().weedout(m, trace=T);
	mqa.show(trace=T);
	mqa.summary(trace=T);
	# mqt := mqa.tree(trace=T);
	# mqt.show(trace=T);
    }

    if (F) {
	print '\n******************\nTest: transpose';
	print 'm=',m := mqa.matrix('T', qual=F, dim=[2,3], trace=F);
	mqa.show('before',trace=T);
	mqa.transpose();
	mqa.show('transposed',trace=T);
    }

    if (F) {
	print '\n******************\nTest: matrix product';
	qual := F;
	print '\n A=',A := mqa.matrix('A', qual=qual, dim=[2,2], trace=F);
	print '\n B=',B := mqa.matrix('B', qual=qual, dim=[2,2], trace=F);
	print '\n matprod: AmB=',AB := mqa.matprod('AmB', A, B);
	print '\n dirprod: AdB=',AB := mqa.dirprod('AdB', A, B);
    }
    
    if (F) {
	print '\n******************\nTest: H_conversion';
	Slin := mqa.prefab('Stokes_matrix_linear', trace=T);
	Scir := mqa.prefab('Stokes_matrix_circular', trace=T);
	if (F) {
	    H := mqa.prefab('H_matrix', trace=T);
	    HH := mqa.dirprod('HH', H, H);
	    mqa.mqt().evaluate(HH, trace=T);
	    print 'S=',S := mqa.matprod('S', HH, Slin);
	    mqa.mqt().evaluate(S, trace=T);                       # transposed!?
	    print 'Scir=',Scir;
	} else {
	    Hinv := mqa.prefab('H_inverse_matrix', trace=T);
	    HH := mqa.dirprod('HH', Hinv, Hinv);
	    mqa.mqt().evaluate(HH, trace=T);
	    print 'S=',S := mqa.matprod('S', HH, Scir);
	    mqa.mqt().evaluate(S, trace=T);                       # transposed!?
	    print 'Slin=',Slin;
	}
    }



    #--------------------------------------------------------------
    # Finished: common actions:
    #--------------------------------------------------------------
    if (T && common_finish) {
	# mqa.mqt().range(trace=T);
	# mqa.mqt().show(opt=[full=T, cseqform=F], trace=T);
	# mqa.mqt().show('before .evaluate()', opt=[type='range'], trace=T);
	# mqa.mqt().evaluate(trace=T);
	# mqa.mqt().show('after .evaluate()', opt=[type='range'], trace=T);
	# mqa.mqt().range(trace=T);
	# mqa.mqt().show(opt=[type='child'], trace=T);
	# mqa.mqt().inspect();
	## mqa.mqt().print();
    }


    return T;
}
# Execute the test-function:
# test_MeqArray();

############################################################################
