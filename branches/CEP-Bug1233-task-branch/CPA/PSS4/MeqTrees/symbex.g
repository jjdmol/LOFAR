# symbex.g: Symbolic expressions

# Copyright (C) 1996,1997,1998
# Associated Universities, Inc. Washington DC, USA.
#
# This library is free software; you can redistribute it and/or modify it
# under the terms of the GNU Library General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
#
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
# License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with this library; if not, write to the Free Software Foundation,
# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
#
# Correspondence concerning AIPS++ should be addressed as follows:
#        Internet email: aips2-request@nrao.edu.
#        Postal address: AIPS++ Project Office
#                        National Radio Astronomy Observatory
#                        520 Edgemont Road
#                        Charlottesville, VA 22903-2475 USA
#
# $Id$
#
#---------------------------------------------------------

# pragma include once
print '\n\n\n\n\n\n=================================================';
print 'include symbex.g    d03/h05/h07/d08/h09/d10oct2003';

include 'genericClosureFunctions.g';
include 'MeqTree.g';


symbex := function (name='sbx') {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, 
			    name=name, type='symbex');


    private.init := function () {
	wider private;

	# The resulting MeqTrees are kept in a separate object:
	private.mqt := MeqTree(public.type());

	# Functions to handle expression (strings) are in MeqExpr:
	private.mqe := MeqExpr(public.type());

	# Functions to deal with node-names
	private.mqn := MeqName(public.type());

	return T;
    }


#====================================================================
# Public interface:
#====================================================================

    public.mqe := function () {return ref private.mqe}

    public.mqn := function () {return ref private.mqn}

    public.mqt := function () {return ref private.mqt}
    public.clear := function () {return private.mqt.clear()}


#====================================================================
# A symbex array (1D vector or 2D matrix) consists of a properly shaped 
# string array (exprout) of Glish expressions for the various elements. 
# The expessions can contain symbex nodes ({name}). 
# The result (exprout) can be fed into a matrix operation function 
# (like matprod() or dirprod()).
# The matrix dimensions are taken from the input expr (string) array,
# which contain the expressions for the various elements.

    public.vector := function (name='V', qual=F, expr=F, dim=F, suffix=F, type=F, trace=F) {
	s := paste('.vector(',name,dim,' suffix=',suffix,'):');
	if (trace) print s,' expr[',shape(expr),']=',expr;
	if (is_string(suffix)) dim := len(suffix);    # e.g. "I Q U V"
	if (is_string(expr)) {                        # expr given
	    dim := len(expr);
	    expr::shape := dim;                       # just in case
	} else if (is_boolean(dim)) {
	    return F;                                 # error message...?
	} else {                                      # automatic names
	    expr := array('{}',dim);                  # see .expr2out()
	}
	if (!is_string(suffix)) suffix := as_string([1:dim]);
	if (len(suffix)!=dim) suffix := as_string([1:dim]);
	exprout := rep('0',dim);
	for (i in ind(expr)) {
	    node := spaste(name,'_',suffix[i]);
	    exprout[i] := private.expr2out (expr=expr[i], node=node, qual=qual,
					    value=F, type=type, trace=trace);
	}
	if (trace) print '   ->',type_name(exprout),'[',len(exprout),']:',exprout;
	return exprout;
    }


# Make a (nxm) matrix:

    public.matrix := function (name='M', qual=F, expr=F, dim=F, type=F, trace=F) {
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
	exprout := expr;
	for (i in [1:dim[1]]) {
	    for (j in [1:dim[2]]) {
		node := spaste(name,'_',i,j);         # automatic node name
		exprout[i,j] := private.expr2out (expr=expr[i,j], node=node, qual=qual,
						  value=F, type=type, trace=trace);
	    }
	}
	if (trace) print '   ->',type_name(exprout),'[',len(exprout),']:',exprout;
	return exprout;
    }

# Helper function to deal with the elements of the input expr arrays of the
# functions .vector() and .matrix().
# The elements of the input expr can have the following syntax:
# - {}             -> make nodes with automatic names (e.g. {G_23})
# - {name}         -> make node with the given name (and dummy initial value)
# - {numeric}      -> make with automatic name (e.g. {G_23}), and the given value
# - numeric        -> not a new node, just a number
# - non-numeric    -> regular Glish expression assumed (may have {nodes}).

    private.expr2out := function (expr=F, node=F, qual=F, value=F, type=F, trace=F) {
	if (trace) print paste('.expr2out(',expr,node,qual,value,type,')');
	cc := split(expr,'');                  # split into chars
	if (cc[1]=='{') {
	    if (!is_boolean(qual)) node := private.mqn.nodename(node, qual);
	    exprout := spaste('{',node,'}');
	    if (expr=='{}') {
		# {}: new node with the provided name, and a dummy value
		value := private.mqt.dummy_value (value=value, type=type);
		private.mqt.node (node, expr=spaste(value), origin='expr2out', trace=trace); 
	    } else {
		# {xyz}: something enclosed by curly brackets: new node 
		dummy := private.mqe.expr2node(expr);
		private.mqe.cseqform (dummy, cseqform=T, trace=F);
		for (s in dummy.child) {
		    v := as_double(s);
		    if (v!=0 || s=='0') {
			if (trace) print '---',s,'= numeric =',v;
			# {numeric}: new node with this default value
			private.mqt.node (node, expr=s, origin='expr2out', trace=trace); 
		    } else {
			if (trace) print '---',s,'= non-numeric';
			# {xyz}: new node with this name
			private.mqt.node (node, expr=expr, origin='expr2out', trace=trace); 
		    }
		}
	    }
	} else {
	    # Assume Glish expression. No new node.
	    exprout := expr;
	    # Should we make sure that the child nodes exist?
	    if (T) {
		dummy := private.mqe.expr2node(expr);
		private.mqe.cseqform (dummy, cseqform=T, trace=F);
		private.mqt.ensure_nodes(dummy.child, trace=trace);
	    }
	}
	return exprout;
    }

# Some special matrices:

    public.unit_matrix := function (dim=2, type=F, trace=F) {
	s := paste('.unit_matrix(',dim,type,'):');
	if (trace) print s;
	ss := ['1',rep('0',dim)];
	if (type=='complex') ss := ['1+0i',rep('0',dim)];
	ss := array(ss,dim,dim);
	if (trace) print '   ->',type_name(ss),'[',len(ss),']:',ss;
	return ss;
    }

    public.diagonal_matrix := function (name='D', qual=F, expr=F, dim=2, type=F, trace=F) {
	s := paste('.diagonal_matrix(',name,dim,type,'):');
	if (trace) print s,' expr[',shape(expr),']=',expr;
	if (is_string(expr)) {                        # expr given
	    dim := len(expr);
	    expr::shape := dim;                       # just in case
	} else {                                      # automatic names
	    if (is_boolean(dim)) return F;            # error message...?
	    dim := dim[1];                            # just in case
	    expr := array('{}',dim);                  # nodes with automatic names
	}
	ss := array('0',dim,dim);
	for (i in [1:dim]) ss[i,i] := expr[i];
	return public.matrix(name, qual=qual, expr=ss, type=type, trace=trace);
    }

    public.tensor := function (name=F, qual=F, expr=F, dim=F, trace=F) {
	s := paste('.tensor(',name,dim,'):');
	if (trace) print s,' expr[',shape(expr),']=',expr;
	print s,'** NOT IMPLEMENTED YET! **';
	return F;
    }

# Transpose an matrix (string array):

    public.transpose := function(m) {
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
	d1 := shape(e1);
	d2 := shape(e2);
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
		r := private.mqt.node (node, expr=s3, hold=T, origin=s, trace=trace);
		if (is_fail(r)) print r;
	    }
	}
	r := private.mqt.flush_node_temp (trace=trace);  # flush the node buffer
	if (is_fail(r)) print r;
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

#=====================================================================
# Convert Jones matric from linear to circular version (v.v.):

    public.lincir := function (expr=F, mode='lin2cir', trace=F) {
	s := paste('.lin2cir(',type_name(expr),shape(expr),'):');
	if (trace) print s,expr;
	if (len(expr)!=4) return F;                 # not a Jones matrix
	expr::shape := [2,2];                       # just in case                        
	H := public.prefab('H_matrix', trace=trace);
	Hinv := public.prefab('H_inverse_matrix', trace=trace);
	if (mode=='lin2cir') {                      # linear -> circular
	    HJ := public.matprod('HJ', H, expr);
	    HJH := public.matprod('HJH', HJ, Hinv);
	} else {                                    # circular -> linear
	    HJ := public.matprod('HJ', Hinv, expr);
	    HJH := public.matprod('HJH', HJ, H);
	}
	if (trace) {
	    private.mqt.evaluate(HJ, trace=F);
	    private.mqt.evaluate(HJH, trace=F);
	}
	return HJH;
    }


#====================================================================
# Make some standard qualifier records:

#====================================================================
# Make some standard expressions:

    public.prefab := function (prefab=F, name=F, qual=F, expr=F, dim=F, type=F, ij=F, trace=F) {
	s := paste('symbex.prefab(',prefab,name,expr,dim,type,ij,'):');
	# trace := T;
	if (trace) print '\n**',s;
	r := F;

	# Check the qualifier record:
	qual := private.mqn.qual(qual);

	if (prefab=='test') {
	    if (is_boolean(name)) name := 'Test';
	    r := 'no test at the moment';

	} else if (prefab=='Stokes_matrix_linear') {
	    if (is_boolean(name)) name := 'Slin';
	    # NB: The factor 0.5 is correct, but can be applied elsewhere too!
	    # expr := "0.5 0 0 0.5   0.5 0 0 -0.5   0 0.5 0.5 0   0 0.5i -0.5i 0";
	    expr := "1 0 0 1   1 0 0 -1   0 1 1 0   0 1i -1i 0";
	    r := public.matrix(name, qual=qual, expr=expr, dim=[4,4]);
	} else if (prefab=='Stokes_matrix_circular') {
	    if (is_boolean(name)) name := 'Scir';
	    # NB: The factor 0.5 is correct, but can be applied elsewhere too!
	    # expr := "0.5 0 0 0.5   0 0.5 0.5 0   0 0.5i -0.5i 0   0.5 0 0 -0.5";
	    expr := "1 0 0 1   0 1 1 0   0 1i -1i 0   1 0 0 -1";
	    r := public.matrix(name, qual=qual, expr=expr, dim=[4,4]);

	} else if (prefab=='H_matrix') {
	    if (is_boolean(name)) name := 'Hmat';
	    expr := "1/sqrt(2) 1/sqrt(2) 1i/sqrt(2) -1i/sqrt(2)";
	    r := public.matrix(name, qual=qual, expr=expr, dim=[2,2]);
	} else if (prefab=='H_inverse_matrix') {
	    if (is_boolean(name)) name := 'Hinv';
	    expr := "1/sqrt(2) -1i/sqrt(2) 1/sqrt(2) 1i/sqrt(2)";
	    r := public.matrix(name, qual=qual, expr=expr, dim=[2,2]);

	} else if (prefab=='unit_matrix') {
	    if (is_boolean(name)) name := 'Unit';
	    r := public.unit_matrix(dim=dim);

	} else if (prefab=='diagonal_matrix') {
	    if (is_boolean(name)) name := 'Mdiag';
	    r := public.diagonal_matrix(name, qual=qual, expr=expr, type=type);

	} else if (prefab=='rotation_matrix') {
	    if (is_boolean(name)) name := 'Mrot';
	    if (is_string(expr)) expr := [arot=expr];       # string -> record
	    if (!is_record(expr)) expr := [=];              # empty record
	    if (!has_field(expr,'arot')) expr.arot := "{arot}";
	    if (len(expr.arot)==1) expr.arot := [expr.arot,expr.arot];
	    expr := [spaste('cos(',expr.arot[1],')'),spaste('-sin(',expr.arot[2],')'),
		     spaste('sin(',expr.arot[1],')'),spaste('cos(',expr.arot[2],')')];
	    r := public.matrix(name, qual=qual, expr=expr, dim=[2,2]);

	} else if (prefab=='phase_rotation_matrix') {
	    if (is_boolean(name)) name := 'Prot';
	    if (is_string(expr)) expr := [prot=expr];       # string -> record
	    if (!is_record(expr)) expr := [=];              # empty record
	    if (!has_field(expr,'prot')) expr.prot := "{prot}";
	    if (len(expr.prot)==1) expr.prot := [expr.prot,spaste('-',expr.prot)];
	    expr := [spaste('exp(1i*',expr.prot[1],')'),
		     spaste('exp(1i*',expr.prot[2],')')];
	    r := public.diagonal_matrix(name, qual=qual, expr=expr, dim=[2,2]);
	    
	} else if (prefab=='ellipticity_matrix') {
	    if (is_boolean(name)) name := 'Mell';
	    if (is_string(expr)) expr := [aell=expr];       # string -> record
	    if (!is_record(expr)) expr := [=];              # empty record
	    if (!has_field(expr,'aell')) expr.aell := "{aell}";
	    if (len(expr.aell)==1) expr.aell := [expr.aell,spaste('-',expr.aell)];
	    expr := [spaste('cos(',expr.aell[1],')'),spaste('-1i*sin(',expr.aell[2],')'),
		     spaste('1i*sin(',expr.aell[1],')'),spaste('cos(',expr.aell[2],')')];
	    r := public.matrix(name, qual=qual, expr=expr, dim=[2,2]);

	} else if (prefab=='IQUV') {
	    if (is_boolean(name)) name := 'LSM';
	    r := public.vector(name, qual=qual, expr=expr, suffix="I Q U V");

	} else if (prefab=='Vis_XY') {
	    if (is_boolean(name)) name := 'V';
	    r := public.vector(name, qual=qual, expr=expr, suffix="XX XY YX YY", type='complex');
	} else if (prefab=='Vis_RL') {
	    if (is_boolean(name)) name := 'V';
	    r := public.vector(name, qual=qual, expr=expr, suffix="RR RL LR LL", type='complex');

	} else if (prefab=='JijJones') {            
	    if (!is_record(expr)) expr := [=];              # empty record
	    # if (!has_field(expr,'GJones')) expr.aell := "{aell}";
	    Ji := public.matrix('Ji', expr=F, dim=[2,2], type='complex');
	    Jj := public.matrix('Jj', expr=F, dim=[2,2], type='complex');
	    r := public.dirprod('Jij', Ji, Jj);

	} else if (prefab=='JJones') {            
	    if (is_boolean(name)) name := 'Ji';
	    r := public.matrix(name, qual=qual, expr=expr, dim=[2,2], type='complex');

	} else if (prefab=='GijJones') {            
	    Gi := public.prefab('GJones', 'Gi', qual=private.mqn.qual('s1'));
	    Gj := public.prefab('GJones', 'Gi', qual=private.mqn.qual('s2'));
	    r := public.dirprod('Gij', Gi, Gj, qual=private.mqn.qual('s12'));

	} else if (prefab=='GJones') {
	    if (is_boolean(name)) name := 'Gi';
	    # Position-independent gain (uv-plane effect):
	    if (is_boolean(expr) || len(expr)==2) {
		r := public.diagonal_matrix(name, qual=qual, expr=expr, type='complex');
	    } else {        
                # include (off-diagonal) cross-talk terms            
		r := public.matrix(name, qual=qual, expr=expr, dim=[2,2], type='complex');
	    }

	} else if (prefab=='EJones') {
	    if (is_boolean(name)) name := 'Ei';
	    # Position-dependent gain (image-plane effect):
	    if (is_boolean(expr) || len(expr)==2) {
		r := public.diagonal_matrix(name, qual=qual, expr=expr, type='complex');
	    } else {        
                # include (off-diagonal) cross-talk terms            
		r := public.matrix(name, qual=qual, expr=expr, dim=[2,2], type='complex');
	    }

	} else if (prefab=='TJones') {            
	    if (is_boolean(name)) name := 'Ti';
	    # Atmosphere: assume phase rotation only:
	    r := public.prefab('phase_rotation_matrix', name=name, qual=qual, expr=expr);

	} else if (prefab=='IJones') {            
	    if (is_boolean(name)) name := 'Ii';
	    # Ionospheric phase (excl Faraday rotation, see FJones):
	    r := public.prefab('phase_rotation_matrix', name=name, qual=qual, expr=expr);

	} else if (prefab=='DJones') {
	    if (is_boolean(name)) name := 'Di';
	    Ell := public.prefab('ellipticity_matrix', expr=expr, trace=F);
	    Rot := public.prefab('rotation_matrix', expr=expr, trace=F);
	    r := public.matprod(name, Ell, Rot);

	} else if (prefab=='FJones') {
	    if (is_boolean(name)) name := 'Fi';
	    r := public.prefab('rotation_matrix', name=name, qual=qual, expr=expr, trace=F);

	} else if (prefab=='PJones') {
	    if (is_boolean(name)) name := 'Pi';
	    # Perpendicular dipoles:
	    r := public.prefab('rotation_matrix', name=name, qual=qual, expr=expr, trace=F);

	} else if (prefab=='KJones') {
	    if (is_boolean(name)) name := 'Ki';
	    r1 := public.prefab('xyz', trace=trace); 
	    r2 := public.prefab('lmn', trace=trace); 
	    r := private.rmerge(r1,r2);
	    r.HA := private.mqt.node('HA', expr='1.0'); 
	    r.u := private.mqt.node('u', qual=private.mqn.qual('cs12'), 
				    expr=spaste(r.dx,'*sin(',r.HA,') + ',
						r.dy,'*cos(',r.HA,')')); 
	    r.v := private.mqt.node('v', qual=private.mqn.qual('cs12'),  
				    expr=spaste('-',r.dx,'*cos(',r.HA,')*sin(',r.DEC,') + ',
						r.dy,'*sin(',r.HA,')*sin(',r.DEC,') + ',
						r.dz,'*cos(',r.DEC,')')); 
	    r.w := private.mqt.node('w', qual=private.mqn.qual('cs12'), 
				    expr=spaste(r.dx,'*cos(',r.HA,')*cos(',r.DEC,') - ',
						r.dy,'*sin(',r.HA,')*cos(',r.DEC,') + ',
						r.dz,'*sin(',r.DEC,')')); 
	    expr := spaste('exp(pi*2i*(',r.u,'*',r.lk,'+',r.v,'*',r.mk,'+',r.w,'*',r.nk,'))');
	    expr := paste(expr,'/ sqrt(',r.nk,')');
	    r := public.diagonal_matrix(name, qual=F, expr=rep(expr,2), dim=2);

	} else if (prefab=='KijJones') {
	    if (is_boolean(name)) name := 'Kij';
	    Ki := public.prefab('KJones', 'Ki', trace=F);
	    Kj := public.prefab('KJones', 'Kj', trace=F);
	    r := public.dirprod('Jij', Ki, Kj);

	} else if (prefab=='DFT_kernel') {
	    if (is_boolean(name)) name := 'DFT';
	    r := public.prefab('uvw', trace=trace);
	    expr := spaste('exp(pi*2i*(',r.u,'*',r.lk,'+',r.v,',*',r.mk,'+',r.w,'*',r.nk,'))/',r.nk);
	    r.DFT := private.mqt.node(name, qual=qual, expr=expr);

	} else if (prefab=='lmn') {
	    r := [=];
	    r.RAk := private.mqt.node('RAk', qual=private.mqn.qual('c'), expr='1.2');        # RA of source k (rad)
	    r.DECk := private.mqt.node('DECk', qual=private.mqn.qual('c'), expr='1.1');
	    r.RA := private.mqt.node('RA', qual=qual, expr='1.0');            # RA of fringe-stopping centre (rad)
	    r.DEC := private.mqt.node('DEC', qual=qual, expr='1.0');
	    r.lk := private.mqt.node('lk', qual=private.mqn.qual('c'),      # direction cosine (rad)
				     expr=spaste(r.RAk,'-',r.RA)); 
	    r.mk := private.mqt.node('mk', qual=private.mqn.qual('c'), 
				     expr=spaste(r.DECk,'-',r.DEC));
	    r.nk := private.mqt.node('nk', qual=private.mqn.qual('c'), 
				     expr=spaste('sqrt(1-(',r.lk,'^2)-(',r.mk,'^2))'));

	} else if (prefab=='xyz') {
	    # Station position coordinates:
	    r := [=];
	    r.x1 := private.mqt.node('xi', qual=private.mqn.qual('s1'), expr='1.0');        # x: in the meridian plane
	    r.x2 := private.mqt.node('xi', qual=private.mqn.qual('s2'), expr=spaste(r.x1,'+5'));
	    r.y1 := private.mqt.node('yi', qual=private.mqn.qual('s1'), expr='1.0');        # y: towards the East
	    r.y2 := private.mqt.node('yi', qual=private.mqn.qual('s2'), expr=spaste(r.y1,'+1000'));
	    r.z1 := private.mqt.node('zi', qual=private.mqn.qual('s1'), expr='0.0');        # z: towards the north pole
	    r.z2 := private.mqt.node('zi', qual=private.mqn.qual('s2'), expr=spaste(r.z1,'-1'));
	    r.dx := private.mqt.node('dx', qual=private.mqn.qual('s12'), expr=spaste(r.x2,'-',r.x1));     # baseline 
	    r.dy := private.mqt.node('dy', qual=private.mqn.qual('s12'), expr=spaste(r.y2,'-',r.y1));     # baseline 
	    r.dz := private.mqt.node('dz', qual=private.mqn.qual('s12'), expr=spaste(r.z2,'-',r.z1));     # baseline 

	} else if (prefab=='uvw') {
	    # See Thompson, Moran and Swenson, page 86;
	    r1 := public.prefab('xyz', qual=qual, trace=trace); 
	    r2 := public.prefab('lmn', trace=trace);  
	    r := private.rmerge(r1,r2);
	    r. HA := private.mqt.node('HA', expr='1.0'); 
	    private.mqt.node('u', qual=private.mqn.qual('cs12'), 
			     expr=spaste(r.dx,'*sin(',r.HA,') + ',
					 r.dy,'*cos(',r.HA,')')); 
	    private.mqt.node('v', qual=private.mqn.qual('cs12'), 
			     expr=spaste('-',r.dx,'*cos(',r.HA,')*sin(',r.DEC,') + ',
					 r.dy,'*sin(',r.HA,')*sin(',r.DEC,') + ',
					 r.dz,'*cos(',r.DEC,')')); 
	    private.mqt.node('w', qual=private.mqn.qual('cs12'), 
			     expr=spaste(r.dx,'*cos(',r.HA,')*cos(',r.DEC,') - ',
					 r.dy,'*sin(',r.HA,')*cos(',r.DEC,') + ',
					 r.dz,'*sin(',r.DEC,')')); 

	} else if (prefab=='xyz2uvw') {
	    # See Thompson, Moran and Swenson, page 86;
	    r := [HA='1.0', DEC='1.0'];                       # temporary
	    # private.mqt.node('HA', expr='1.0');             # -> {HA}
	    expr := array('0',3,3);
	    expr[1,] := [spaste('sin(',r.HA,')'), 
			 spaste('cos(',r.HA,')'), 
			 spaste('0')]; 
	    expr[2,] := [spaste('-cos(',r.HA,')*sin(',r.DEC,')'), 
			 spaste('sin(',r.HA,')*sin(',r.DEC,')'),
			 spaste('cos(',r.DEC,')')]; 
	    expr[3,] := [spaste('cos(',r.HA,')*cos(',r.DEC,')'),
			 spaste('sin(',r.HA,')*cos(',r.DEC,')'),
			 spaste('sin(',r.DEC,')')]; 
	    r := public.matrix(prefab, expr);

	} else if (prefab=='Mifr') {
	    if (is_boolean(name)) name := 'Mij';
	    r := public.diagonal_matrix(name, qual=qual, expr=F, dim=4);

	} else if (prefab=='Aifr') {
	    if (is_boolean(name)) name := 'Aij';
	    r := public.vector(name, qual=qual, expr=expr, suffix="XX XY YX YY", type='complex');

	} else {
	    print s,'not recognised:',prefab;
	    return F;
	}

	# Finished:
	if (trace) print '\n   symbex.prefab(',prefab,') ->',type_name(r),shape(r),':',r;
	return r;
    }

# Merge two records by copying the fields of r2 to r1:

    private.rmerge := function (r1=[=], r2=[=], override=T, trace=T) {
	if (trace) print '\n** .rmerge(',type_name(r1),len(r1),type_name(r2),len(r2),'):';
	if (!is_record(r1)) r1 := [=];
	if (!is_record(r2)) return r1;
	for (fname in field_names(r2)) {
	    if (trace) print '-',fname,'=',r2[fname];
	    if (has_field(r1,fname)) {
		print '\n** .rmerge(): name clash for field:',fname,'\n';
		r1[fname] := r2[fname];
	    } else {
		r1[fname] := r2[fname];
	    }
	}
	return r1;
    }


#====================================================================
# Specific trees:
#====================================================================

#--------------------------------------------------------------------
# A MeqCondeq node generates condition equations for a solver:
# It has two children: 'predicted' and 'measured'
# Default args are for WSRT:

# Alternative 1:
#    define with:  qual := [s1='{s1}', s2='({s1}+1):({smax}-1)'];
#    fillin with:  qual := [smax=3, s1=[1:3]]
# Note the order of the fillin qualifiers (smax first)
# Does not work because MeqName.fillin() does one qualifier at a time.
# Could be made to work by changing the order, but that might affect
# the way in which the ifrs are generated.

# Alternative 2:
#    define with:  qual := [s1=T, s2=T]
#    fillin with:  qual := [s1=[1:3], s2='({s1}+1):({smax}-1)']
# In other words, put expressions in the fillin qual!!
# This is very attractive....

    public.MeqCondeq := function (name='MeqCondeq', suffix="XX XY YX YY",
				  qual=[s1='{s1}', s2='({s1}+1):3'],
				  predicted='predict', measured='data', 
				  smax=3, trace=F) {
	input := [qual=qual, suffix=suffix, predicted=predicted, measured=measured];
	funcname := private._.onentry('.MeqCondeq()', input=input, trace=trace);
	private.mqt.clear();       # Clear the internal MeqTree object

	s1 := public.vector (predicted, suffix=suffix, qual=[s1=T, s2=T]);
	s2 := public.vector (measured, suffix=suffix, qual=[s1=T, s2=T]);
	for (i in ind(suffix)) {
	    expr := paste(s2[i],'-',s1[i]);
	    public.vector (name, suffix=suffix[i], qual=qual, expr=expr);
	}

	# Create a new MeqTree object, and copy the internal one:
	mqt := private.mqt.new(name, copy=T);
	if (trace) mqt.show(opt=[full=T], trace=T);
	return private._.onexit(funcname, mqt, trace=trace);
    }

#-----------------------------------------------------------------------
# A MeqSink node puts data back into the datasource.
# (It is the counterpart of the MeqSpigot node) 

# This the one we need for our first experiment: just copy uv-data
# from MeqSpigots to MeqSinks, controlled by the MeqSinks.

    public.MeqSink := function (name='MeqSink', suffix="XX XY YX YY",
			       qual=[s1='{s1}', s2='({s1}+1):3'],
			       spigot='spigot', smax=3, trace=F) {
	input := [qual=qual, suffix=suffix, spigot=spigot];
	funcname := private._.onentry('.MeqSink()', input=input, trace=trace);
	private.mqt.clear();       # Clear the internal MeqTree object

	s1 := public.vector (spigot, suffix=suffix, qual=[s1=T, s2=T]);
	for (i in ind(suffix)) {
	    expr := paste(s1[i]);
	    public.vector (name, suffix=suffix[i], qual=qual, expr=expr);
	}

	# Create a new MeqTree object, and copy the internal one:
	mqt := private.mqt.new(name, copy=T);
	if (trace) mqt.show(opt=[full=T], trace=T);
	return private._.onexit(funcname, mqt, trace=trace);
    }

#-----------------------------------------------------------------------------
# A MeqWsum node calculated the weighted sum of its children:

    public.predict := function (name='predict', pol='XXYY', trace=F) {
	input := [=];
	funcname := private._.onentry('.predict()', input=input, trace=trace);
	private.mqt.clear();       # Clear the internal MeqTree object

	if (pol=='RRLL') {
	    suffix := "RR RL LR LL";
	    S := public.prefab('Stokes_matrix_circular','S');
	} else {
	    suffix := "XX XY YX YY";
	    S := public.prefab('Stokes_matrix_linear','S');
	}

	I := public.vector ('LSM', suffix="I Q U V", qual=private.mqn.qual('c'));
	SI := public.matprod('SI', S, I, suffix=suffix, qual=private.mqn.qual('c'));

	Ji := public.matrix('Jones', qual=private.mqn.qual('cs1'), dim=[2,2], type='complex');
	Jj := public.matrix('Jones', qual=private.mqn.qual('cs2'), dim=[2,2], type='complex');
	Jij := public.dirprod('Jij', Ji, Jj, qual=private.mqn.qual('cs12'));

	public.matprod(name, Jij, SI, suffix=suffix, qual=private.mqn.qual('cs12'));

	print '\n** .bypass(SI) -> absum =',absum := private.mqt.bypass(SI, trace=F);
	print '\n** .bypass(Jij) -> absum =',absum := private.mqt.bypass(Jij, trace=F);

	# Create a new MeqTree object, and copy the internal one:
	mqt := private.mqt.new(name, copy=T);
	if (trace) mqt.show(opt=[full=T], trace=T);
	return private._.onexit(funcname, mqt, trace=trace);
    }


#-----------------------------------------------------------------------------
# A MeqWsum node calculated the weighted sum of its children:

    public.MeqWsum := function (name='MeqWsum', suffix="XX XY YX YY",
				qual=F, child=F, trace=F) {
	input := [qual=qual, suffix=suffix];
	funcname := private._.onentry('.MeqCondeq()', input=input, trace=trace);
	private.mqt.clear();       # Clear the internal MeqTree object

	s1 := public.vector ('predict', suffix=suffix, qual=[s1=T, s2=T, c=T]);
	I := public.prefab('IQUV');
	public.mqt().show(opt=[full=T, cseqform=F], trace=T);
	mqt := public.mqt().MeqWsum(trace=T);

	# Create a new MeqTree object, and copy the internal one:
	return private._.onexit(funcname, mqt, trace=trace);
    }


#====================================================================
    private.init();
    return ref public;
}


#====================================================================
#====================================================================
# To be done:
#   matrix groupings (?)
#   bypass (automatic?) check whether same result
#   domain (2D: freq/time)
#   range  (3D: freq/time/parm)  n+1
#   simulation (mnsparm)
#   automatic experiment
#   derivatives (difference/analytic)  -> range
#   complex numbers
#   solve()
#   display results

# NB: complete shadow?


#################################################################################
# Test-program:
#################################################################################


test_symbex := function (name="") {
    private := [=];
    public := [=];

    private.mqn := MeqName('test_symbex');

    print '\n\n********************************************************** test_symbex():';
    sbx := symbex();
    print '\n** sbx =\n',sbx;
    sbx.mqt().clear();
    common_finish := T;         # if T, perform common finishing actions

    qual := F;                  # default value (ignored)
    # qual := [a=T, b=[2], c=[d=T]];
    qual := [a=T, b=[2]];

#-----------------------------------------------------------------
# Some helper functions:


#-----------------------------------------------------------------


    if (F) {
	print '\n******************\nTest: vector';
	# v := sbx.vector('V', qual=qual, dim=4, trace=F);
	v := sbx.vector('V', qual=qual, suffix="I Q U V", trace=F);
	# v := sbx.vector('V', qual=qual, expr="1 2 3 pi 5", trace=F);
	# sbx.mqt().node ('g', qual=qual, expr='e', trace=F);
	# v := sbx.vector('V', qual=qual, expr="1 2 {g} pi 5", trace=F);
	print 'v=',v;
    }
    
    if (F) {
	print '\n******************\nTest: matrix';
	# print 'm=',m := sbx.unit_matrix(dim=4, trace=F);
	# print 'm=',m := sbx.matrix('A', qual=qual, dim=[2,2], trace=F);
	# print 'm=',m := sbx.diagonal_matrix('M', qual=qual, dim=4, trace=F);
	# print 'm=',m := sbx.diagonal_matrix('M', qual=qual, expr="1 {g} {} 4", trace=F);
	print 'm=',m := sbx.matrix('A', qual=qual, expr="{0} 7*0 {} {a}", dim=[2,2], trace=F);
	# print 'm1=',m1 := sbx.mqt().weedout(m, trace=T);
	# sbx.mqt().weedout(trace=T);
    }

    if (F) {
	print '\n******************\nTest: matrix product';
	qual := F;
	print '\n A=',A := sbx.matrix('A', qual=qual, dim=[2,2], trace=F);
	print '\n B=',B := sbx.matrix('B', qual=qual, dim=[2,2], trace=F);
	print '\n matprod: AmB=',AB := sbx.matprod('AmB', A, B);
	print '\n dirprod: AdB=',AB := sbx.dirprod('AdB', A, B);
    }
    
    if (F) {
	print '\n******************\nTest: H_conversion';
	Slin := sbx.prefab('Stokes_matrix_linear', trace=T);
	Scir := sbx.prefab('Stokes_matrix_circular', trace=T);
	if (F) {
	    H := sbx.prefab('H_matrix', trace=T);
	    HH := sbx.dirprod('HH', H, H);
	    sbx.mqt().evaluate(HH, trace=T);
	    print 'S=',S := sbx.matprod('S', HH, Slin);
	    sbx.mqt().evaluate(S, trace=T);                       # transposed!?
	    print 'Scir=',Scir;
	} else {
	    Hinv := sbx.prefab('H_inverse_matrix', trace=T);
	    HH := sbx.dirprod('HH', Hinv, Hinv);
	    sbx.mqt().evaluate(HH, trace=T);
	    print 'S=',S := sbx.matprod('S', HH, Scir);
	    sbx.mqt().evaluate(S, trace=T);                       # transposed!?
	    print 'Slin=',Slin;
	}
    }

    if (T) {
	print '\n******************\nTest: Prefabricated expressions';
	# r := sbx.prefab('IQUV', trace=T);
	# r := sbx.prefab('IQUV', expr="10 0 0 0", trace=T);
	# r := sbx.prefab('IQUV', expr="{10} {0} {0} {0}", trace=T);
	# r := sbx.prefab('Vis_XY', trace=T);
	# r := sbx.prefab('Vis_RL', trace=T);
	# r := sbx.prefab('Mifr', trace=T);
	# r := sbx.prefab('Aifr', trace=T);
	# r := sbx.prefab('GJones', trace=T);
	# r := sbx.prefab('GJones', expr="{1+0i} {1-0i}", trace=T);
	# r := sbx.prefab('GijJones', qual=[c=T], trace=T);
	# r := sbx.prefab('DJones', trace=T);
	# r := sbx.prefab('DJones', expr=[arot="{p}",aell="{q}"], trace=T);
	# r := sbx.prefab('JJones', expr=[arot="{p}",aell="{q}"], trace=T);
	# r := sbx.prefab('KJones', trace=T);
	# r := sbx.prefab('KijJones', trace=T);
	# r := sbx.prefab('DFT_kernel', trace=T);
    }

    if (F) {
	print '\n******************\nTest: Jones matrices';
	# print 'Ji=', Ji := sbx.unit_matrix (dim=2, type='complex', trace=F);
	# print 'Jj=', Jj := sbx.unit_matrix (dim=2, type='complex', trace=F);
	print 'Gi=', Gi := sbx.prefab('GJones','Gi', trace=F);
	print 'Gj=', Gj := sbx.prefab('GJones','Gj', trace=F);
	print 'Di=', Di := sbx.prefab('DJones','Di', expr=[aell='{delli}', arot='{droti}'], trace=F);
	print 'Dj=', Dj := sbx.prefab('DJones','Dj', expr=[aell='{dellj}', arot='{drotj}'], trace=F);
	print 'Fi=', Fi := sbx.prefab('FJones','Fi', expr=[arot='{froti}'], trace=F);
	print 'Fj=', Fj := sbx.prefab('FJones','Fj', expr=[arot='{frotj}'], trace=F);
	print 'DFi=', DFi := sbx.matprod('DFi', Di, Fi);
	print 'DFj=', DFj := sbx.matprod('DFj', Dj, Fj);
	print 'GDFi=', GDFi := sbx.matprod('GDFi', Gi, DFi);
 	print 'GDFj=', GDFj := sbx.matprod('GDFj', Gj, DFj);
	# print '\n** .bypass(DFi) -> absum =',sbx.bypass(DFi, trace=F);
	# print '\n** .bypass(DFj) -> absum =',sbx.bypass(DFj, trace=F);
	# print 'Jij=',Jij := sbx.dirprod('Jij', Ji, Jj);
    }

    if (F) {
	print '\n******************\nTest: Jones chain';
	print 'Ji=', Ji := sbx.unit_matrix (dim=2, type='complex', trace=T);
	print 'Gi=', Gi := sbx.prefab('GJones','Gi', trace=F);
	print 'Ji*Gi -> Ji=', Ji := sbx.matprod('Ji', Ji, Gi);
	if (F) {
	    print 'Di=', Di := sbx.prefab('DJones','Di', expr=[aell='{delli}', arot='{droti}'], trace=T);
	    print 'Ji*Di -> Ji=', Ji := sbx.matprod('Ji', Ji, Di);
	}
	if (T) {
	    print 'Fi=', Fi := sbx.prefab('FJones','Fi', expr=[arot='{froti}'], trace=F);
	    print 'Ji*Fi -> Ji=', Ji := sbx.matprod('Ji', Ji, Fi, trace=F);
	}
	# print 'Jij=',Jij := sbx.dirprod('Jij', Ji, Jj);
	sbx.mqt().show(trace=T);
	sbx.mqt().weedout(trace=T, ttrace=F);
    }

    if (F) {
	print '\n******************\nTest: KJones';
	# r := sbx.prefab('xyz', trace=T);
	# r := sbx.prefab('lmn', trace=T);
	# r := sbx.prefab('uvw', trace=T);
	print 'Ki=', Ki := sbx.prefab('KJones', 'Ki', trace=F);
	# print 'Kj=', Kj := sbx.prefab('KJones', 'Kj', trace=F);
	# print 'Kij=', Kij := sbx.prefab('KijJones', trace=F);
	# r := sbx.prefab('DFT_kernel', trace=F);
    }

    if (F) {
	print '\n******************\nTest: xyz2uvw';
	sbx.prefab('xyz', trace=T);
	xyz := "{xi} {yi} {zi}"
	sbx.prefab('lmn', trace=T);               # -> {DEC}
	sbx.mqt().node ('HA', expr='1.1');              # -> {HA}
	M := sbx.prefab('xyz2uvw', trace=T);      # 3x3 matrix
	sbx.mqt().node ('u', expr=sbx.inprod(M[1,], xyz));
	sbx.mqt().node ('v', expr=sbx.inprod(M[2,], xyz));
	sbx.mqt().node ('w', expr=sbx.inprod(M[3,], xyz));
    }

    if (F) {
	print '\n******************\nTest: LOFAR Jij';
	print 'Ei=', Ei := sbx.prefab('EJones','Ei', trace=F);
	print 'Ej=', Ej := sbx.prefab('EJones','Ej', trace=F);
	# print 'Pi=', Pi := sbx.prefab('PJones','Pi', trace=F);
	# print 'Pj=', Pj := sbx.prefab('PJones','Pj', trace=F);
	print 'Fi=', Fi := sbx.prefab('FJones','Fi', expr=[arot='{froti}'], trace=F);
	print 'Fj=', Fj := sbx.prefab('FJones','Fj', expr=[arot='{frotj}'], trace=F);
	print 'EFi=', EFi := sbx.matprod('EFi', Ei, Fi);
	print 'EFj=', EFj := sbx.matprod('EFj', Ej, Fj);
	print 'Jij=',Jij := sbx.dirprod('Jij', EFi, EFj);
	# r := sbx.prefab('KJones', trace=T);
	# r := sbx.prefab('KijJones', trace=T);
    }

    if (F) {
	print '\n******************\nTest: LOFAR H(t,f)';
	Jij := sbx.prefab('JijJones', trace=T);
	I := sbx.prefab('IQUV');
	S := sbx.prefab('Stokes_matrix_linear','S');
	SI := sbx.matprod('SI', S, I, suffix="XX XY YX YY");
	print 'SI=',SI;
	V := sbx.matprod('V', Jij, SI, suffix="XX XY YX YY");
	print '\n** .bypass(SI) -> absum =',sbx.bypass(SI, trace=F);
	print '\n** .bypass(Jij) -> absum =',sbx.bypass(Jij, trace=F);
    }

    if (F) {
	print '\n******************\nTest: WSRT';
	K := sbx.prefab('KijJones');
	I := sbx.prefab('IQUV');
	S := sbx.prefab('Stokes_matrix_linear','S');
	SI := sbx.matprod('SI', S, I, suffix="XX XY YX YY");
	VP := sbx.matprod('VP', K, SI, suffix="XX XY YX YY");
	if (T) {
	    Jij := sbx.prefab('JijJones', trace=T);
	    # Jij := sbx.prefab('GijJones', trace=T);
	    V := sbx.matprod('V', Jij, VP, suffix="XX XY YX YY");
	    print '\n** .bypass(Jij) -> absum =',sbx.bypass(Jij, trace=F);
	    # print '.bypass(VP) -> absum =',sbx.bypass(VP, trace=F);
	}
	print '\n** .bypass(SI) -> absum =',sbx.bypass(SI, trace=F);
	print '\n** .bypass(K) -> absum =',sbx.bypass(K, trace=F);
    }

    if (F) {
	print '\n******************\nTest: Stokes times I';
	I := sbx.prefab('IQUV');
	S := sbx.prefab('Stokes_matrix_linear','S');
	# S := sbx.prefab('Stokes_matrix_circular','S');
	SI := sbx.matprod('SI', S, I);
	print 'SI=',SI;
    }


    if (F) {
	print '\n******************\nTest: MeqCondeq/MeqSink';
	# mqt1 := sbx.MeqCondeq(suffix='XY', trace=T);
	mqt1 := sbx.MeqSink(suffix='XY', trace=T);
	mqt2 := mqt1.fillin(qual=[smax=3, s1=[1:2]], trace=T);
	# mqt2.show(trace=T);
    }

    if (T) {
	print '\n******************\nTest: predict';
	mqt1 := sbx.predict(pol='XXYY', trace=T);
	# mqt1 := sbx.predict(pol='RRLL', trace=T);
	# mqt1.print(opt=[full=T, copies=2]);
	mqt1.generate_cpp(trace=T);
    }

    if (F) {
	print '\n******************\nTest: LSM';
	mqt1 := sbx.LSM(trace=T);
	# mqt1.print(opt=[full=T, copies=2]);
    }

    if (F) {
	print '\n******************\nTest: MeqWsum';
	mqt1 := sbx.MeqWsum(suffix='XY', trace=T);
    }



    #--------------------------------------------------------------
    # Finished: common actions:
    #--------------------------------------------------------------
    if (T && common_finish) {
	# sbx.mqt().range(trace=T);
	# sbx.mqt().show(opt=[full=T, cseqform=F], trace=T);
	# sbx.mqt().show('before .evaluate()', opt=[type='range'], trace=T);
	# sbx.mqt().evaluate(trace=T);
	# sbx.mqt().show('after .evaluate()', opt=[type='range'], trace=T);
	# sbx.mqt().range(trace=T);
	# sbx.mqt().show(opt=[type='child'], trace=T);
	# sbx.mqt().inspect();
	## sbx.mqt().print();
    }
    return ref sbx;
}

#====================================================================
sbx := test_symbex();                 # execute test_program
#====================================================================


