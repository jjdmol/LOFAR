# file: ../MeqTrees/MeqExpr.g

#---------------------------------------------------------

# pragma include once
# print '\n\n\n\n\n\n=================================================';
print 'include MeqExpr.g    h05/d08/h09/d10oct2003';

include 'unset.g';
include 'genericClosureFunctions.g';




############################################################################
# MeqExpr: Functions to manipulate symbolic expressions 
#          used in symbex.g and MeqTree.g
############################################################################



MeqExpr := function (name=F) {
    public := [=];
    private := [=];
    genericClosureFunctions(public=public, private=private, name=name,
			    type='MeqExpr', subset='basic');

    private.init := function () {
	wider private;
	return T;
    }

#===========================================================================
# Public interface
#===========================================================================


#=========================================================================
# Unary operation on a (string) expression:

    public.unop := function (unop=F, expr=F, origin=F, trace=F) {
	enc := spaste(unop,'(',expr,')');            # e.g exp(expr)     
	rr := public.test_numeric(expr);
	s := paste('.unop(',enc,'):');
	if (trace) print s,rr;

	# NB: In the following, the ORDER is important!
	if (unop=='abs') {
	    if (rr.is_numeric) return as_string(abs(rr.value));
	} else if (unop=='conj') {
	    if (rr.is_numeric) return as_string(conj(rr.value));
	} else if (unop=='arg') {
	    if (rr.is_numeric) return as_string(arg(rr.value));
	} else if (unop=='real') {
	    if (rr.is_numeric) return as_string(real(rr.value));
	} else if (unop=='imag') {
	    if (rr.is_numeric) return as_string(imag(rr.value));

	} else if (unop=='exp') {
	    if (rr.is_numeric) return as_string(exp(rr.value));
	} else if (unop=='log') {
	    if (rr.is_numeric) return as_string(log(rr.value));
	} else if (unop=='ln') {
	    if (rr.is_zero || rr.is_negative) return 'Inf';
	    if (rr.is_numeric) return as_string(ln(rr.value));

	} else if (unop=='sqrt') {
	    if (rr.is_numeric) return as_string(sqrt(rr.value));
	} else if (unop=='square') {
	    if (rr.is_numeric) return as_string(rr.value^2);
	} else if (unop=='inv') {
	    if (rr.is_numeric) return as_string(1/rr.value);

	} else if (unop=='sin') {
	    if (rr.is_numeric) return as_string(sin(rr.value));
	} else if (unop=='cos') {
	    if (rr.is_numeric) return as_string(cos(rr.value));
	} else if (unop=='tan') {
	    if (rr.is_numeric) return as_string(tan(rr.value));
	} else if (unop=='atan') {
	    if (rr.is_numeric) return as_string(atan(rr.value));
	} else if (unop=='asin') {
	    if (rr.is_numeric) return as_string(asin(rr.value));
	} else if (unop=='acos') {
	    if (rr.is_numeric) return as_string(acos(rr.value));

	} else if (unop=='floor') {
	    if (rr.is_numeric) return as_string(floor(rr.value));
	} else if (unop=='ceiling') {
	    if (rr.is_numeric) return as_string(ceiling(rr.value));

	} else {
	    print '**',s,'not recognised';
	}
	return enc;                           # always!
    }

# Binary operation (+,-,*,/,^) on two (string) expressions

    public.binop := function (e1=F, binop=F, e2=F, origin=F, trace=F) {
	if (trace) print '<<< .binop(',e1,binop,e2,')',origin;
	enc1 := public.enclose(e1, trace=trace);  # enclose '(e1)', if necessary
	if (trace) print '-- enc1:',type_name(enc1);
	enc2 := public.enclose(e2, trace=trace);  # enclose '(e2)', if necessary
	if (trace) print '-- enc2:',type_name(enc2);
	# print '>>> .binop() after enclose()';
	cc2 := split(e2,'');                  # split e2 into chars
	rr1 := public.test_numeric(e1);
	rr2 := public.test_numeric(e2);
	num12 := (rr1.is_numeric && rr2.is_numeric); 
	s := paste('.binop(',enc1,binop,enc2,' num12=',num12,'):');
	if (trace) print s;

	# NB: In the following, the ORDER is important!
	if (any(binop=="+ add")) {
	    if (rr2.is_zero) return e1;
	    if (rr1.is_zero) return e2;
	    if (num12) return as_string(rr1.value+rr2.value);
	    if (cc2[1]=='-') return paste(enc1,'+',enc2);
	    return paste(e1,'+',e2);
	    return paste(enc1,'+',e2);
	} else if (any(binop=="* mult")) {
	    if (rr1.is_zero || rr2.is_zero) return '0';
	    if (rr2.is_one) return e1;
	    if (rr1.is_one) return e2;
	    if (rr2.is_minus) {
		if (rr1.is_minus) return '1';
		return spaste('-',enc1);
	    }
	    if (rr1.is_minus) return spaste('-',enc2);
	    if (num12) return as_string(rr1.value*rr2.value);
	    return paste(enc1,'*',enc2);
	} else if (any(binop=="- sub subt subtract")) {
	    if (rr2.is_zero) return e1;
	    if (rr1.is_zero) return spaste('-',enc2);
	    if (e1==e2) return '0';
	    if (num12) return as_string(rr1.value-rr2.value);
	    return paste(e1,'-',enc2);
	    return paste(enc1,'-',enc2);
	} else if (any(binop=="/ div")) {
	    if (rr1.is_zero) return '0';
	    if (rr2.is_zero) return 'Inf';              #.....?
	    if (rr2.is_one) return e1;
	    if (rr1.is_one) return spaste('1/',enc2);
	    if (rr2.is_minus) {
		if (rr1.is_minus) return '1';
		return spaste('-',enc1);
	    }
	    if (rr1.is_minus) return spaste('-',enc2);
	    if (num12) return as_string(rr1.value/rr2.value);
	    return paste(enc1,'/',enc2);
	} else if (any(binop=="^ pow")) {
	    if (rr1.is_zero) return '0';
	    if (rr1.is_one) return '1';
	    if (rr2.is_zero) return '1';
	    if (rr2.is_one) return e1;
	    if (num12) return as_string(rr1.value^rr2.value);
	    return paste(enc1,'^',enc2);
	} else {
	    print '**',s,'not recognised';
	}
	return F;
    }

#================================================================
#================================================================
# The following functions operate on the .expr and .child fields
# in a given node record (node).
#================================================================
#================================================================

# Check whether the specified node has the required fields.

    public.node := function (ref node=F, trace=F) {
	if (!is_record(node)) val node := [=];
	if (!has_field(node,'name')) node.name := '<dummy>';
	if (!has_field(node,'child')) node.child := "";
	if (!has_field(node,'expr')) node.expr := "";
	if (!has_field(node,'cseqform')) node.cseqform := F;
	## if (!has_field(node,'cseq')) node.cseq := "";
	if (trace) print '\n** mqe.node() ->',node;
	return node;
    }

# Make a node record from a given expr (string):

    public.expr2node := function (expr=F, cseqform=T, trace=F) {
	node := public.node(trace=trace);
	if (!is_string(expr)) expr := '<expr2node>';  # ....?
	node.expr := expr;
	node.cseqform := F;                      # just in case
	public.cseqform (node, cseqform=T, trace=trace);    # always!
	if (!cseqform) public.cseqform (node, cseqform=F, trace=trace);
	if (trace) print '\n** mqe.expr2node() ->',node;
	return node;
    }

# Make sure that the node.expr has the correct cseqform (T/F):
# NB: Note that this also works if 'node' is an expression string.

    public.cseqform := function (ref node=F, cseqform=T, trace=F) {
	# trace := T;
	public.node(node);                       # check the input node
	s := paste('\n** .cseqform(',node.name,cseqform,'): expr=',node.expr,':');
	if (node.cseqform==cseqform) {
	    if (trace) print s,'OK';
	    return T;                            # OK, no nothing
	} else if (cseqform) {	
	    if (trace) print s,'{cname}-> {#n}';
	    return private.child2seqnr (node, trace=trace);
	} else {
	    if (trace) print s,'{#n} -> {cname}';
	    return private.seqnr2child (node, trace=trace);
	}
	return F;
    }

# Replace the child-names {xyz} in the given expression
# with their sequence nrs (in the vector if child-names).
# NB: typical child name: {<classname>[x={x}][b=6]}
# Return a record (rr) with the new expr and the child vector.

    private.child2seqnr := function (ref node=F, trace=F) {
	if (trace) s := paste('     .child2seqnr(',node.name,'):');
	ss := split(node.expr,'');               # split into chars
	expr := "";
	cc := [=];                  
	level := 0;  
	k := 1;
	for (i in ind(ss)) {
	    if (ss[i]=='{') {
		if (level==0) k := i;
		level +:= 1;
	    } else if (ss[i]=='}') {
		level -:= 1;
		if (level==0) {
		    cname := spaste(ss[(k+1):(i-1)]);
		    cc[cname] := cname;
		    iseq := ind(cc)[field_names(cc)==cname];
		    cc[cname] := spaste('{#',iseq,'}');
		    expr := spaste(expr,cc[cname]);
		    next;
		}
	    }
	    if (level==0) expr := spaste(expr,ss[i]);
	    
	}
	# Modify the node record:
	node.expr := expr;                       # the input expression, but with {#n}                  
	node.child := field_names(cc);           # vector of child names in the right order                      
	node.cseqform := T;                      # set the switch
	### for (i in ind(node.child)) node.cseq := [node.cseq,cc[i]];
	if (trace) print s,'->',node;
	return T;
    }

#------------------------------------------------------------------------
# The reverse of .child2seqnr():

    private.seqnr2child := function (ref node=F, trace=F) {
	if (trace) s := paste('     .seqnr2child(',node.name,'):');
	for (i in ind(node.child)) {
	    cseq := spaste('\{\#',i,'\}');
	    s1 := spaste('\'',node.expr,'\' ~ s/',cseq,'/{',node.child[i],'}/g');
	    if (trace) print '   - s1=',s1;
	    node.expr := eval(s1);
	    if (trace) print '   - eval(s1) ->',node.expr;
	}
	node.cseqform := F;                       # set the switch
	if (trace) print s,'->',node;
	return T;
    }

#------------------------------------------------------------------------
# Replace a child (cname) in the expression of the given node (rr) with a new expr.
# Assume that the given expression contains actual child names (not {#n})
# But make sure that node.expr is in {#n} form (cseqform=T).

   public.child2expr := function (ref node=F, cname=F, expr=F, trace=F) {
	public.node(node);                                # check the input node
	if (trace) print '\n  - child2expr(',node.name,'): {',cname,'} ->',expr;

	# Check whether the node actually has a child named 'cname':
	iseq := ind(node.child)[node.child==cname];
	if (len(iseq)==0) {
	    if (trace) print '    -',cname,'not among',len(node.child),'children:',node.child; 
	    return node;
	}

	# enclose (expr) if necessary (do outside?)
	expr := public.enclose(expr);

	# Make sure that node.expr is in cseqform:
	was := node.cseqform;                             # keep for later
	public.cseqform(node, T, trace=trace);

	# OK, replace child cname with expr:
	if (trace>1) print '    - before:',node; 
	for (j in ind(node.child)) {
	    cseq := spaste('\{\#',j,'\}');
	    if (j==iseq) {                                # {#<iseq>} -> expr
		s1 := spaste('\'',node.expr,'\' ~ s/',cseq,'/',expr,'/g');
	    } else {                                      # other {#n} -> their child names
		s1 := spaste('\'',node.expr,'\' ~ s/',cseq,'/{',node.child[j],'}/g');
	    }
	    node.expr := eval(s1);
	}
	node.cseqform := F;
	if (trace>1) print '    - after:',node; 
	# Extract the new node.child vector:
	public.cseqform(node, T, trace=trace);
	if (trace>1) print '    -> T:',node; 

	# Finished:
	# public.cseqform(node, was, trace=trace);  # restore
	return T;
    }


#========================================================================
# Generate C++ code:
# {#n} -> *(values[n-1])

    public.generate_cpp := function (node=F, trace=F) {
	public.cseqform(node, T, trace=trace);
	if (trace) print '\n** .generate_cpp(',node.expr,'):';
	expr := node.expr;
	for (j in ind(node.child)) {
	    cseq := spaste('\{\#',j,'\}');
	    new := spaste('\(\*\(values\[',j-1,'\]\)\)');
	    s1 := spaste('\'',expr,'\' ~ s/',cseq,'/',new,'/g');
	    if (trace) print '-',j,':',s1;
	    expr := eval(s1);
	}
	expr := paste('return',expr);
	if (trace) print '   -> expr =',expr,'\n';
	return expr;
    }


#========================================================================
# Test: evaluate the given (string) expr, using Glish eval().
# Use the child sequence nrs as values.
# NB: This is a very useful little test to see whether the expression in
#     a C++ MeqFunc has been implemented properly.

    public.testeval := function (expr=F, expected=F, trace=F) {
	s := paste('\n** .testeval(',expr,'):');
	if (trace) print s;

	# Convert to a temporary node, and make sure that
	# it is in cseqform ({#n}):
	node := public.expr2node(expr, cseqform=T, trace=trace);

	# Replace the children {#n} in node.expr with numbers (v): 
	expr := node.expr;
	for (i in ind(node.child)) {
	    v := spaste('(',i,'*1.1)');
	    # v := spaste('complex(cos(',i,'),sin(',i,'))');
	    s2 := spaste('\'',expr,'\' ~ s/{#',i,'}/',v,'/g');
	    if (trace>1) print '  -',i,' s2=',s2;
	    expr := eval(s2);
	}

	# OK, evaluate the expression itself:
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
	    if (trace) print '   ->',expr,'->',v;
	}

	# Compare with the expected value, if supplied:
	if (!is_boolean(expected)) {
	    if (v==expected) {
		if (trace) print s,'OK';
	    } else {
		print s,'** ERROR** :',v,'!=',expected;
	    }
	}

	return v;
    }



#----------------------------------------------------------------
# Formatted expr (from symbex.show):

    public.format := function (expr=F) {
	return public.oneliner (expr);
    }

    public.oneliner := function (expr=F) {
	s := "";
	cc := split(expr,'');                          # split into chars
	ss := public.terms (expr, tbreak="+ - * / ^", trace=F);
	if (len(cc)<100 || len(ss)==1) {
	    s := spaste(s,' expr= \'',expr,'\'');
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
	return s;
    }


#====================================================================
# Some helper functions:
#====================================================================

# Helper function to find out if expr is numeric:

    public.test_numeric := function (expr=F, trace=F) {
	# Some special cases (needs a little more thought):
	# if (expr=='Inf') {
	# } else if (expr=='pi') {
	# }
	# 
	rr := [is_numeric=F, is_complex=F, is_zero=F, is_one=F, 
	       is_minus=F, is_negative=F, expr=expr, value=F];
	rr.is_numeric := (rr.expr ~ m/^[0-9i.+-]+$/);  
	if (rr.is_numeric) {
	    rr.value := as_double(rr.expr);
	    rr.is_zero := (rr.value==0);
	    rr.is_one := (rr.value==1);
	    rr.is_minus := (rr.value==-1);
	    rr.is_negative := (rr.value<0);
	    cc := split(rr.expr,'');                     # split into chars
	    if (cc[len(cc)]=='i') {
		rr.is_complex := T;
		rr.value := as_complex(rr.expr);
		rr.is_zero := (abs(rr.value)==0);
		rr.is_one := (real(rr.value)==1 && imag(rr.value)==0);
		rr.is_minus := (real(rr.value)==-1 && imag(rr.value)==0);
		rr.is_negative := (rr.expr ~ m/^-/);    # first char is -
	    }
	}
	return rr;
    }

# Helper function to determine whether the given string expr is zero:

    public.is_zero := function (expr=F, trace=F) {
	if (!is_string(expr)) return F;
	if (expr=='0') return T;
	if (expr=='0+0i') return T;
	return F;
    }
    public.is_unity := function (expr=F, trace=F) {
	if (!is_string(expr)) return F;
	if (expr=='1') return T;
	if (expr=='1+0i') return T;
	# if (as_double(expr)==1) return T;     # could be: '1x'
	return F;
    }

# Helper function to 'round' the given number:

    public.round := function (v) {
	if (is_complex(v) || is_dcomplex(v)) {
	    vr := real(v);
	    vi := imag(v);
	    v := complex(vr,vi);
	} else {
	}
	return F;                          # temporary
	return v;
    }
	 

# Helper function that removes curly brackets

    public.remove_curly_brackets := function (ss) {
	return public.deenclose(ss, bb="{ }");
	#-------------------------------------------------------

	# Old version: removes the open qualifiers too: error!
	curly := ss ~ m/{/;                # if T, contains curly brackets
	if (curly) {
	    ss := ss ~ s/{//;              # strip off '{'
	    ss := ss ~ s/}//;              # strip off '}'
	}
	return ss;
    }		 

#=======================================================================
# Enclose an expression with round brackets, if necessary:

    public.enclose := function (expr=F, always=F, trace=F) {
	#----------------------------
	if (always) return spaste('(',expr,')');
	#----------------------------
	s := paste('.enclose(',expr,'):');

	# if (trace) print '<<<',s;
	rr := public.analyse(expr, unop=T, origin='.enclose()', trace=trace);
	if (is_fail(rr)) {
	    if (trace) print '**',s,'analyse() -> rr=',type_name(rr);
	    print rr;
	    rr := [ok=F];                  # continue, always enclose
	}

	enclose := T;                      # Default: enclose (always safe)
	if (rr.ok) { 
	    if (rr.enclosed) {
		# Do not enclose if already enclosed (...):
		enclose := F;
	    } else if (rr.unop) {
		# Do not enclose if unary operation: sin(...) etc
		enclose := F;
	    } else {
		# Do not enclose if already enclosed in curly brackets {...}:
		rr_curly := public.analyse(expr, bb='{}',  
					   origin='.enclose()', trace=trace);
		if (is_fail(rr_curly)) {
		    if (trace) print '**',s,'analyse() -> rr_curly=',type_name(rr_curly);
		    print rr_curly;
		    break;                           # escape
		}
		if (rr_curly.enclosed) {
		    enclose := F;
		} else {
		    # Do not enclose if 'suitably' numeric:
		    nn := public.test_numeric(expr, trace=trace);
		    if (nn.is_numeric) enclose := F;
		    if (nn.is_negative) enclose := T;   # first char is '-'
		    # if (nn.is_complex) enclose := T;
		}
	    }
	}
	# if (expr=='{DFT}') print '*****',s,'-> enclose=',enclose;
	if (!enclose) return expr;
	return spaste('(',expr,')');
    }

#-----------------------------------------------------------------
# Remove up to nmax layers of 'excess' brackets, if necessary:

    public.deenclose := function (expr=F, bb="( )", 
				  ref n=F, ref reenclose=F, 
				  nmax=10, trace=F) {
	s := paste('** .deenclose(',expr,')');
	if (trace) print s;
	val n := -1;                               # counter
	while ((val n+:=1)<nmax) {                 # safety-limit
	    expr := public.remove_blanks(expr);
	    # if (trace) print '-',n,':',expr;
	    rr := public.analyse(expr, bb=bb, tbreak="+ - * / ^", 
				 origin='.deenclose()', trace=F);
	    if (is_fail(rr)) {
		if (trace) print '**',s,'analyse() -> rr=',type_name(rr);
		print rr;
		break;
	    }
	    if (!rr.enclosed) break;               # not enclosed: escape
	    ncc := len(cc:=split(expr,''));        # split into chars
	    expr := spaste(cc[2:(ncc-1)]);         # re-paste without ()
	}
	val reenclose := (n>0);                    # at least one layer removed 
	if (trace) print s,n,'->',expr;
	return expr;
    }

#-----------------------------------------------------------------
# Break an expression into a vector of (additive) terms:

    public.terms := function (expr=F, tbreak="+ -", trace=F) {
	s := paste('** .terms(',spaste('{',spaste(tbreak),'}'),expr,')');
	expr := public.deenclose(expr);            # remove excess brackets
	rr := public.analyse(expr, tbreak=tbreak, origin='.terms()', trace=F);
	if (is_fail(rr)) {
	    if (trace) print '**',s,'analyse() -> rr=',type_name(rr);
	    print rr;
	    return 'fail';
	}
	if (trace) {
	    print '\n',s;
	    for (s in rr.ss) print ' - term: ',s;
	    print ' ';
	}
	return rr.ss;
    }


#-----------------------------------------------------------------
# Remove any leading and trailing blanks:

    public.remove_blanks := function (expr=F, trace=F) {
	ncc := len(cc:=split(expr,''));            # split into chars
	if (trace) s := paste('** .remove_blanks(',expr,')');
	for (i1 in [1:ncc]) if (cc[i1]!=' ') break;
	for (i2 in [ncc:1]) if (cc[i2]!=' ') break;
	if (i1>1 || i2<ncc) expr := spaste(cc[i1:i2]);        # re-paste
	if (trace) print s,ncc,i1,12,'->',expr,'|';
	return expr;
    }


#=======================================================================
# Analyse the given string (expr), and return the results in a record:
# - count the nr of chars, and give the first and last one.
# - check the pairing of brackets of the given type (bb).
# - split into 'terms', on the specified breaks (tbreak).
# - if only one term:
#   - indicate whether it is enclosed in 'excess' brackets.
#   - check whether it is an unary operation, e.g. sin().
# - etc.

    public.analyse := function (expr=F, bb="( )", tbreak="+ -", unop=F, 
				origin=F, trace=F) {
	# Check (and correct) the bracket specification:
	if (len(bb)!=2) {
	    if (bb=='()' || bb[1]=='(') bb := "( )";
	    if (bb=='{}' || bb[1]=='{') bb := "{ }";
	    if (bb=='[]' || bb[1]=='[') bb := "[ ]";
	}
	cc := split(expr,'');                # split into chars
	if (len(cc)==0) cc := '?';           # just in case
	ncc := len(cc);                      # nr of chars
	ncmax := 50;
	sexpr := spaste(cc[1:min(ncmax,ncc)]);
	if (ncc>ncmax) sexpr := spaste(sexpr,'...');
	s := paste('.analyse(',tbreak,bb,sexpr,'):');
	# print '<<<',s,origin;
	rr := [bb=bb, nchar=ncc, first=cc[1], last=cc[ncc], 
	       enclosed=F, unop=F, ok=T, nn=[0,0], nzz=0, level=0, 
	       tbreak=tbreak, nss=0, ss=""];
	if (ncc<=0 || cc=='?') {
	    print s,origin,'ncc=',ncc,' cc=',cc;
	    return rr;
	}

	if (cc[1]=='+') cc[1] := ' ';        # replace leading '+' with blank char (....)
	zz := "";                            # chars at rr.level=0 
	ss := "";                            # constituent 'terms'
	k := 1;                              # start of 1st term
	for (i in ind(cc)) {
	    c := cc[i];
	    if (c==bb[1]) {                  # opening bracket
		rr.nn[1] +:= 1;
		rr.level +:= 1;
	    } else if (c==bb[2]) {           # closing bracket
		rr.nn[2] +:= 1;
		rr.level -:= 1;
	    } else if (rr.level==0) {        # outside the brackets
		zz := [zz,c];       
		if (any(c==tbreak)) {        # term breaks (e.g. + - * etc)
		    # print '<<< c==tbreak:',i,c,' expr=',expr;
		    if (i>1 && any(cc[i-1]=="+ - * / ^")) {
			# Special case: two binary operators in succession:
			if (c=='+') {        # OK e.g. a*+b etc
			    cc[i] := ' ';    # replace '+' with blank char (....)
			} else if (c=='-') { # OK e.g. a/-b etc
			} else {
			    print s,'WARNING: detected combination:',cc[i-1],c;
			}
			next;                # NOT a break!
		    } else if (len(ss)==0) { # first term
			# print '<<< first term:',i,c,' expr=',expr;
			if (i>1) {
			    ss := spaste(cc[1:(i-1)]);
			    if (ss==' ' || ss=='  ') ss := "";
			}
			# print '>>> first term:',type_name(ss),len(ss),' ss=',ss;
		    } else {                 # another term
			# print '<<< non-first term:',i,c,' expr=',expr;
			ss := [ss,spaste(cc[k:(i-1)])];
			# print '>>> non-first term:',type_name(ss),len(ss),' ss=',ss;
		    }
		    # The term-breaks (+ -) are separate 'terms'
		    # E.g: '(a+b) + (c+d)' gives three 'terms'
		    ss := [ss,spaste(cc[i])];
		    k := i+1;                # start of next term
		    if (k<ncc && cc[k]==' ') k := k+1; 
		}
	    } 
	    if (rr.level<0) rr.ok := F;      # should not happen
	}
	if (k<=i) ss := [ss,spaste(cc[k:i])];  # last term
	rr.nss := len(ss);                   # nr of 'terms'
	rr.nzz := len(zz);                   # nr of chars outside

	if (rr.nss==1) {                     # one term only
	    # NB: The expr can ONLY be enclosed if it is one term!
	    if (rr.first==bb[1] && rr.last==bb[2]) rr.enclosed := T;
	    # Check for unop: sin()/cos()/conj() etc, if unop=T
	    if (unop && rr.nzz>0 && ncc>5) {
		cc5 := spaste(cc[1:5]);
		if (any(cc5=="sqrt( conj( real( imag( asin( acos( atan(")) {
		    rr.unop := T;
		} else {
		    cc4 := spaste(cc[1:4]);
		    if (any(cc4=="sin( cos( tan( exp( log( arg( abs(")) {
			rr.unop := T;
		    } else {
			cc3 := spaste(cc[1:3]);
			if (any(cc3=="ln(")) rr.unop := T;
		    }
		}
	    }
	}
	if (rr.level!=0) rr.ok := F;         # brackets not balanced
	if (trace || !rr.ok) {               # enforce message if not ok
	    print '\n**',s,'ok=',rr.ok,'\n  ',rr;
	    if (rr.ok) for (s in ss) print '   - (',tbreak,') term:',s;
	    print ' ';
	}
	# Attach some fields AFTER printing the trace message: 
	if (is_fail(ss)) print 'ss=',ss;
	rr.ss := ss;                         # the 'terms' themselves
	# print '>>> .analyse() finished: rr=',type_name(rr);
	return rr;
    }

#=======================================================================
# Weed out terms that are zero etc from the given expressions:

    public.weedout := function (expr=F, level=1, trace=F) {
	prefix := spaste(array('..',level));
	s := paste(prefix,'** .weedout(',expr,')');
	if (trace) {print '\n'; print s;}
	if (len(split(expr,''))<=1) return expr;

	# Temporarily strip off any 'excess' layers of brackets (): 
	expr := public.deenclose(expr, reenclose=reenclose);
	if (trace) print prefix,'reenclose=',reenclose,':',expr;

	# Split expr into additive terms:
	aa := public.analyse(expr, tbreak="+ -", origin='.weedout(add)', trace=F);
	if (is_fail(aa)) {
	    if (trace) print '**',s,'analyse() -> aa=',type_name(aa);
	    print aa;
	    return expr;
	}

	ka := len(ssa := "");
	for (i in ind(aa.ss)) {
	    ta := aa.ss[i];
	    # Temporarily strip off any 'excess' layers of brackets ():
	    ta := public.deenclose(ta, reenclose=ta_enclose);
	    nca := len(split(ta,''));           # nr of chars in ta
	    if (any(ta=="+ -")) { 
		if (trace) print prefix,'- include term-break:',ta;
		ssa[ka+:=1] := ta; 
		next;
	    } else if (ta==' ') {
		if (trace) print prefix,'- ignore blank(ed):',ta;
		next;
	    } else if (nca<=1) {                # small nr of chars in ta
		if (ta_enclose) ta := public.enclose(ta);
		if (trace) print prefix,'- include short:',ta,'(nca=',nca,')';
		ssa[ka+:=1] := ta; 
		next;
	    }
	    if (trace) print prefix,'-',nca,ta_enclose,'additive term:',ta;

	    # Split into 'multiplicative' terms:
	    tbreak := "* / ^";                  # term-break chars
	    mm := public.analyse(ta, tbreak=tbreak, origin='.weedout(mult)', trace=F);
	    if (is_fail(mm)) {
		if (trace) print '**',s,'analyse() -> mm=',type_name(mm);
		print mm;
		next;
	    }
	    escape := F;
	    zero := F;
	    unity := F;
	    divisor := F;
	    exponent := F;
	    km := len(ssm := "");
	    ignore := 0;
	    for (j in ind(mm.ss)) {
		tm := mm.ss[j];
		tm_enclose := F;
		if (tm=='/') divisor := T;      # divisor from here on..
		if (tm=='^') exponent := T;     # exponent from here on..
		ncm := len(split(tm,''));       # nr of chars in tm
		if (ignore>0) {                 # set below
		    ignore := max(0,ignore-1);  # decrement counter
		    if (trace) print prefix,'--',ignore,'ignore:',tm;
		    next;                
		} else if (any(tm==tbreak)) {   # e.g. tm='*' etc
		    ssm[km+:=1] := tm; 
		    if (trace) print prefix,'-- include term-break:',tm;
		    next; 
		} 

		# Set switch to (re-)enclose tm in brackets (), if necessary:
		if (j>1) {
		    if (mm.ss[j-1]=='^') tm_enclose := T;   # ^<tm>
		    if (mm.ss[j-1]=='/') tm_enclose := T;   # /<tm>
		}
		# print '----- abs(as_complex(',tm,')=',abs(as_complex(tm)); 
		if (abs(as_complex(tm))>0) tm_enclose := T; # tm is complex nr
		if (trace && tm_enclose) print prefix,'-- tm_enclose=',tm_enclose;

		tm := public.remove_blanks(tm); # remove leading/trailing blanks
		if (any(tm=="0 0+0i")) {
		    if (trace) print prefix,'-- zero:',tm;
		    zero := T;
		    break;                          # all mult.terms irrelevant
		# } else if (any(tm=="1 1.0 1+0i 1.0+0.0i")) {
		#    unity := T;                     # Special case: dealt with below
		} else if (ncm<=1) { 
		    if (tm_enclose) tm := public.enclose(tm);
		    ssm[km+:=1] := tm; 
		    if (trace) print prefix,'-- include short:',tm,'(ncm=',ncm,')';
		    next;                           # next mult.term
		} else {
		    if (trace) print prefix,'--',ncm,tm_enclose,'multiplicative term:',tm;
		    if (level<2) {                  # limit (2) is temporary kludge
			# weed out tm (recursive):
			tm := public.weedout(tm, level=level+1, trace=trace);
		    }
		    if (tm_enclose) tm := public.enclose(tm);
		    ssm[km+:=1] := tm;
		    if (trace) print prefix,'-- include:',tm;
		}

		# Special case: weed out unnecessary unity terms:
		unity := F;                               # DISABLED (for the moment)
		if (unity) {
		    s1 := paste(prefix,'-- (unity)',tm,' ->');
		    ssm[km+:=1] := tm;          # include always, remove if necessary
		    if (j==len(mm.ss)) {                  # 1 is last term
			s1 := paste(s1,'(last term)');
			if (j==1) {
			    if (trace) print s1,'first term also: keep';
			} else if (any(mm.ss[j-1]=="* / ^")) {
			    km -:= 2;
			    if (trace) print s1,'ignore:',mm.ss[(j-1):j];
			} else { 
			    if (trace) print s1,'??';
			}
		    } else if (j==1) {                    # 1 is first term
			s1 := paste(s1,'(first term, but not last)');
			if (mm.ss[j+1]=='/') {
			    if (trace) print s1,'keep:',mm.ss[j],'(followed by /)';
			} else if (mm.ss[j+1]=='^') {
			    ignore := 2;
			    if (trace) print s1,'ignore:',mm.ss[(j+1):(j+2)];
			} else if (mm.ss[j+1]=='*') {
			    km -:= 1;
			    ignore := 1;
			    if (trace) print s1,'ignore:',mm.ss[j:(j+1)];
			} else {
			    if (trace) print s1,'??';
			}
		    } else if (mm.ss[j+1]=='/') {
			km -:= 2;
			if (trace) print s1,'ignore:',mm.ss[(j-1):j],'(followed by /)';
		    } else if (mm.ss[j-1]=='/') {
			km -:= 2;
			if (trace) print s1,'ignore:',mm.ss[(j-1):j];
		    } else if (mm.ss[j+1]=='*') {
			km -:= 1;
			ignore := 1;
			if (trace) print s1,'ignore:',mm.ss[j:(j+1)];
		    } else if (mm.ss[j+1]=='^') {
			km -:= 2;
			ignore := 2;
			if (trace) print s1,'ignore:',mm.ss[(j-1):(j+2)];
		    } else {
			if (trace) print s1;
		    }
		} # end of (unity) case
		
		if (escape) break;
	    } 
	    if (len(ssm)==0) ssm := '1';
	    if (len(ssm)==1 && any(ssm==tbreak)) ssm := '1';

	    # Include the next additive term, unless it is zero:
	    ta := paste(ssm[1:max(1,km)]);
	    if (ta_enclose) ta := public.enclose(spaste(ssm[1:max(1,km)]));
	    if (zero) {
		if (exponent) {
		    if (trace) print prefix,'- zero exponent in:',aa.ss[i],': -> 1';
		    ssa[ka+:=1] := '1';
		} else if (divisor) {
		    print s,'zero divisor in:',aa.ss[i],': -> inf';  
		    ssa[ka+:=1] := 'inf';
		} else {
		    if (trace) print prefix,'- ignore zero term:',aa.ss[i];
		    ka := max(0,ka-1);
		    # If first term, ignore (set to blank) any following '+':
		    if (ka==0 && aa.ss[i+1]=='+') aa.ss[i+1] := ' ';
		}
	    } else {
		if (trace) print prefix,'- include additive term:',ta;
		ssa[ka+:=1] := ta;
	    }
	}
	# Make the output expression (exprout):
	if (len(ssa)==0) ssa := '0';                      # if nothing left
	if (len(ssa)==1 && any(ssa=="+ -")) ssa := '0';   # just in case
	exprout := paste(ssa[1:max(1,ka)]);        
	if (reenclose) exprout := public.enclose(spaste(ssa[1:max(1,ka)]));
	if (trace) print prefix,'exprout=',exprout,'\n';
	return exprout;
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

test_MeqExpr := function () {
    include 'inspect.g';
    # include 'MeqName.g';
    print '\n\n****************************************************';
    print '** test_MeqExpr:';

    # Create global object(s):
    print '\n** Create global objects';
    global mqn, mqe;
    # print '\n',mqn := MeqName('mqn');
    print '\n',mqe := MeqExpr('mqe');
    print ' ';
    # mqe.inspect();

    if (F) {
	print '\n******************\nTest: all';
	expr := '{a} + {b} + {c}';
	for (fname in field_names(mqe)) {
	    if (!is_function(mqe[fname])) next;
	    if (any(fname=="inspect evalexpr")) next;
	    if (fname=='unop') {
		r := mqe.unop('abs',expr);
	    } else if (fname=='binop') {
		r := mqe.binop(expr,'+','4');
	    } else {
		r := mqe[fname](expr);
	    }
	    if (is_fail(r)) {
		print '\n** mqe.',fname,'(expr) -> ** FAILED **';
	    } else {
		print '\n** mqe.',fname,'(expr) ->', r;
	    }
	}
    }

    if (F) {
	print '\n******************\nTest: .enclose()';
	expr := 'abc'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '{abc}'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '-{abc}'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '(abc)'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '-(abc)'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '(abc}'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '(abc)*(cdg)'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '(abc)+(cdg)'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '3.5+5i'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '-3.5+5i'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := 'sin(x)'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := 'conj(x)'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := 'ln(x)'; print '.enclose(',expr,') ->',mqe.enclose(expr);
	expr := '-ln(x)'; print '.enclose(',expr,') ->',mqe.enclose(expr);
    }

    if (F) {
	print '\n******************\nTest: .deenclose()';
	mqe.deenclose('abcd', trace=T); 
	mqe.deenclose('(abcd)', trace=T);
	mqe.deenclose(' (abcd)', trace=T);
	mqe.deenclose('(((abcd)))', trace=T);
	mqe.deenclose('(ab)+(cd)', trace=T);
	mqe.deenclose('((ab)+(cd) )', trace=T);
	mqe.deenclose('(ab)/(cd)', trace=T);
	mqe.deenclose('(ab)^(cd)', trace=T);
	mqe.deenclose('((ab)^(cd))', trace=T);
	mqe.deenclose('(1i*sin({a}))', trace=T);
	print '\n input expr=',expr := '((ab)*(cd))';
	print '\n input expr=',expr := '(ab)*(cd)';
	expr := mqe.deenclose(expr, n=n, reenclose=reenclose, trace=T);
	print 'n=',n,reenclose,' -> expr=',expr;
    }

    if (F) {
	print '\n******************\nTest: .remove_blanks()';
	mqe.remove_blanks('ab  cd', trace=T);
	mqe.remove_blanks('  abcd', trace=T);
	mqe.remove_blanks('abcd  ', trace=T);
    }

    if (F) {
	print '\n******************\nTest: .remove_blanks()';
	for (s1 in ['{abc}','abc','{{abc}}','aa bb']) {
	    s2 := mqe.remove_curly_brackets(s1);
	    print '** .remove_curly_brackets(',s1,') ->',s2,' len=',len(s2);
	}
    }

    if (F) {
	print '\n******************\nTest: .analyse()';
	mqe.analyse('abc+cdg', trace=T);
	mqe.analyse('(abc+cdg)', trace=T);
	mqe.analyse('(abc)+(cdg)', trace=T);
	mqe.analyse('(abc) + (cdg)', trace=T);
	mqe.analyse('{abc+cdg}', trace=T);
	mqe.analyse('{abc}+{cdg}', bb='{}', trace=T);
	mqe.analyse('{b}*cos({HA})', trace=T);
	mqe.analyse('{b} * cos({HA})', trace=T);
	mqe.analyse('{b} * cos({HA})+6', trace=T);
	mqe.analyse('{b}*sin({HA})*sin({DEC})', trace=T);
	mqe.analyse('a/-b', trace=T);
	mqe.analyse('a/+b', trace=T);
	mqe.analyse('a*-b', trace=T);
	mqe.analyse('a^-b', trace=T);
    }

    if (F) {
	print '\n******************\nTest: .terms()';
	tbreak := "+ - * / ^";
	mqe.terms('abc+cdg', tbreak=tbreak, trace=T);
	mqe.terms('(abc+cdg)', tbreak=tbreak, trace=T);
	mqe.terms('(abc)+(cdg)', tbreak=tbreak, trace=T);
	mqe.terms('((abc)+(cdg))', tbreak=tbreak, trace=T);
	mqe.terms('(((abc)+(cdg)))', tbreak=tbreak, trace=T);
	mqe.terms('(abc)*(cdg)', tbreak=tbreak, trace=T);
	mqe.terms('(abc)*(cdg)', tbreak="+ -", trace=T);
	mqe.terms('3*b + 0*a', tbreak=tbreak, trace=T);
    }

    if (F) {
	print '\n******************\nTest: .weedout()';
	# mqe.weedout('0 -3*b + 0*a - t*0 + 1*c + (1+0i) + 0', trace=T);
	# mqe.weedout('-3*b + 2*{e} + 0*{d}', trace=T);
	# mqe.weedout('-3*b + (5+3i) - 7-0.5i', trace=T);
	# mqe.weedout('-3*b + (5+3i) - 0-2i', trace=T);
	# mqe.weedout('-3*b + 0/b + c/0 + a^b + 0^a + g^(0*f)', trace=T);
	# mqe.weedout('(2*a + 3*b)', trace=T);
	# mqe.weedout('0 + 0', trace=T);
	# mqe.weedout('(-3*b + 0*d)/(-0-2i)', trace=T);
	# mqe.weedout('1*b + d*1 + a*1*c', trace=T, ttrace=F);
	# mqe.weedout('e/1*f + e/(1*f)', trace=T, ttrace=F);
	# mqe.weedout('1/h + 1/h*a + d*1/a*1 + s/1 + 1*s/1', trace=T, ttrace=F);
	# mqe.weedout("1^a  3*1^c  b^1", trace=T, ttrace=F);
	# mqe.weedout("+a/-1 a/+1 a*-1 a^-5*b", trace=T, ttrace=F);
	mqe.weedout('((a*b)+(c*d))', trace=T, ttrace=T);
	mqe.weedout('((a*0)+(c*d))', trace=T, ttrace=T);
	mqe.weedout('((a)+(c*d))', trace=T, ttrace=T);
    }

    if (F) {
	print '\n******************\nTest: unop';
	print '   ->',mqe.unop ('conj','-b', trace=T);
	print '   ->',mqe.unop ('conj','{xyz}', trace=T);
	print '   ->',mqe.unop ('conj','0', trace=T);
	print '   ->',mqe.unop ('conj','3+3i', trace=T);
    }
    
    if (F) {
	print '\n******************\nTest: binop';
	for (binop in "+ * / ^ -") {
	    print '\n** binop=',binop;
	    print '   ->',mqe.binop ('-a',binop,'-b', trace=T);
	    print '   ->',mqe.binop ('0',binop,'-b', trace=T);
	    print '   ->',mqe.binop ('-a',binop,'0', trace=T);
	    print '   ->',mqe.binop ('0',binop,'0', trace=T);
	    print '   ->',mqe.binop ('1',binop,'1', trace=T);
	    print '   ->',mqe.binop ('2',binop,'3', trace=T);
	    print '   ->',mqe.binop ('2+2i',binop,'3', trace=T);
	    print '   ->',mqe.binop ('2',binop,'3+3i', trace=T);
	    print '   ->',mqe.binop ('2+2i',binop,'3+3i', trace=T);
	    print '   ->',mqe.binop ('1+0i',binop,'3+3i', trace=T);
	    print '   ->',mqe.binop ('0+1i',binop,'3+3i', trace=T);
	    print '   ->',mqe.binop ('0+0i',binop,'3+3i', trace=T);
	    if (any(binop=="+")) {
		print '   ->',mqe.binop ('a',binop,'a', trace=T);
	    }
	    if (any(binop=="-")) {
		print '   ->',mqe.binop ('-a',binop,'-a', trace=T);
	    }
	    if (any(binop=="/ ^")) {
		print '   ->',mqe.binop ('1',binop,'-b', trace=T);
		print '   ->',mqe.binop ('-a',binop,'1', trace=T);
	    }
	    if (any(binop=="* / -")) {
		print '   ->',mqe.binop ('-1',binop,'-b', trace=T);
		print '   ->',mqe.binop ('-a',binop,'-1', trace=T);
		print '   ->',mqe.binop ('-1',binop,'-1', trace=T);
	    }
	}
    }

    if (F) {
	print '\n******************\nTest: test_numeric';
	ss := "0 5 -5.3 5x x5 1 -1 0+0i 1+0i -1+0i 0+1.2i";
	for (expr in ss) {
	    print mqe.test_numeric (expr, trace=T);
	}
    } 


    if (F) {
	print '\n******************\nTest: testeval';
	expr := '{a}+{cc}-{ab[{k}]}/{cc}';
	node := mqe.expr2node(expr, trace=T);
	mqe.cseqform(node, T, trace=F); 
	print '\n** mqe.cseqform(T) -> node=',node; 
	v := mqe.testeval(node.expr, trace=2); 
	print '\n** mqe.testeval(node.expr) ->',v;
    }

    if (T) {
	print '\n******************\nTest: generate_cpp';
	expr := '{a}+{cc}-{ab[{k}]}/{cc}';
	node := mqe.expr2node(expr, trace=T);
	expr := mqe.generate_cpp(node, trace=T); 
	print '->',expr;
    }

    if (F) {
	print '\n******************\nTest: cseqform';
	expr := '{a}+{cc}-{ab[{k}]}/{cc}';
	node := mqe.expr2node(expr, trace=T);
	for (tf in [F, T, F, T, F, T]) {
	    mqe.cseqform(node, tf, trace=F); 
	    print '\n** mqe.cseqform(',tf,') -> node=',node;
	}
    }

    if (F) {
	print '\n******************\nTest: replace_child';
	expr := '{a}+{cc}-{ab[{k}]}/{cc}';
	node := mqe.expr2node(expr, trace=T);
	mqe.cseqform(node, T, trace=F); 
 	print '\n** mqe.cseqform(T) -> node=',node;
	newexpr := '{xxx}';
	newexpr := '{cc}-{e}';
	cname := 'a';
	mqe.child2expr(node, cname, newexpr, trace=2);
	print '\n** after .child2expr() -> node =',node; 
    }

    return T;
}
# Execute the test-function:
# test_MeqExpr();

############################################################################
