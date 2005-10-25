package org.astron.basesim;

import java.util.*;

/**
 * @author Robbert Dam
 * @version 1.0
 */

/** A Reference is a name that refers to some object. This name
 *  can be scoped. A name can be related to a context. A context is the
 *  scope from where the reference was used */
public class Reference {

  static private String delim = ".";
  private String ref;
  private String scope;

  public Reference(String name) {
    ref = new String(name);
  }

  public Reference(String name, String scope) {
    this.scope = scope;
    ref = name;
  }

  public String toString() {
    if (ref.length() == 0) return ref;
    if (ref.substring(0,1).equals(delim)) {
      return scope + ref;
    } else {
      return scope + delim + ref;
    }
  }

  static public String getDelimiter() { return delim; }
  static public void setDelimiter(String d) { delim = d; }

  /** Get name of this reference. */
  public String getName() {
    int i = ref.lastIndexOf(delim);
    if (i == -1) return ref;
    return new String(ref.getBytes(),i+1,ref.length()-i-1);
  }

  public String getScope() { return scope; }

  /** Check if the scope of the specified Reference matches the scope
   *  of this Reference. */
  public boolean inScope(Reference r) {
    StringTokenizer stThat = new StringTokenizer(r.getScope(),delim);
    StringTokenizer stThis = new StringTokenizer(this.scope,delim);

    while (stThis.hasMoreTokens()) {
      if (!stThat.hasMoreTokens()) return false;
      if (!stThat.nextToken().equals(stThis.nextToken())) return false;
    }
    return true;
  }

  /** Compare this reference with another reference. */
  public boolean equals(Reference r) {
    if (toString().equals(r.toString())) return true;
    else return false;
  }
}