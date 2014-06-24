package org.astron.util;

import java.io.File;;

/**
 * Title:
 * Description:
 * Copyright:    Copyright (c) 2001
 * Company: ASTRON
 * @author Robbert Dam
 * @version 1.0
 */

/** Collection of handy methods */
public class Utils {

  /* Get the extension of a file in lowercase. */
  public static String getExtension(File f) {
    String ext = null;
    String s = f.getName();
    int i = s.lastIndexOf('.');

    if (i > 0 &&  i < s.length() - 1) {
      ext = s.substring(i+1).toLowerCase();
    }
    return ext;
  }
}