/*-------------------------------------------------------------------------*/
/**
  @file		gnuplot_i.cc
  @author	N. Devillard
  @date	Sep 1998
  @version	$Revision$
  @brief	C interface to gnuplot.

  gnuplot is a freely available, command-driven graphical display tool for
  Unix. It compiles and works quite well on a number of Unix flavours as
  well as other operating systems. The following module enables sending
  display requests to gnuplot through simple C calls.
  
*/
/*--------------------------------------------------------------------------*/

/*
	$Id[1]: gnuplot_i.c,v 2.6 2001/10/23 12:38:28 ndevilla Exp $
	$Author$
	$Date$
	$Revision$
 */

/*---------------------------------------------------------------------------
                                Includes
 ---------------------------------------------------------------------------*/

#include <lofar_config.h>
#include <APL/RTCCommon/gnuplot_i.h>
#include <cstring>
#include <unistd.h>
#include <string.h>

// to find Array type in blitz namespace
using namespace blitz;

/*---------------------------------------------------------------------------
                                Defines
 ---------------------------------------------------------------------------*/

/** Maximal size of a gnuplot command */
#define GP_CMD_SIZE     	2048
/** Maximal size of a plot title */
#define GP_TITLE_SIZE   	80
/** Maximal size for an equation */
#define GP_EQ_SIZE      	512
/** Maximal size of a name in the PATH */
#define PATH_MAXNAMESZ       4096

/** Define P_tmpdir if not defined (this is normally a POSIX symbol) */
#ifndef P_tmpdir
#define P_tmpdir "."
#endif

/*---------------------------------------------------------------------------
                            Function codes
 ---------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/**
  @brief	Find out where a command lives in your PATH.
  @param	pname Name of the program to look for.
  @return	pointer to statically allocated character string.

  This is the C equivalent to the 'which' command in Unix. It parses
  out your PATH environment variable to find out where a command
  lives. The returned character string is statically allocated within
  this function, i.e. there is no need to free it. Beware that the
  contents of this string will change from one call to the next,
  though (as all static variables in a function).

  The input character string must be the name of a command without
  prefixing path of any kind, i.e. only the command name. The returned
  string is the path in which a command matching the same name was
  found.

  Examples (assuming there is a prog named 'hello' in the cwd):

  @verbatim
  gnuplot_get_program_path("hello") returns "."
  gnuplot_get_program_path("ls") returns "/bin"
  gnuplot_get_program_path("csh") returns "/usr/bin"
  gnuplot_get_program_path("/bin/ls") returns NULL
  @endverbatim
  
 */
/*-------------------------------------------------------------------------*/
char * gnuplot_get_program_path(const char * pname)
{
    int         i, j, lg;
    char    *   path;
    static char buf[PATH_MAXNAMESZ];

    /* Trivial case: try in CWD */
    snprintf(buf, sizeof buf, "./%s", pname) ;
    if (access(buf, X_OK)==0) {
        snprintf(buf, sizeof buf, ".");
        return buf ;
    }
    /* Try out in all paths given in the PATH variable */
    buf[0] = 0;
    path = getenv("PATH") ;
    if (path!=NULL) {
        for (i=0; path[i]; ) {
            for (j=i ; (path[j]) && (path[j]!=':') ; j++);
            lg = j - i;
            strncpy(buf, path + i, lg);
            if (lg == 0) buf[lg++] = '.';
            buf[lg++] = '/';
            strcpy(buf + lg, pname);
            if (access(buf, X_OK) == 0) {
                /* Found it! */
                break ;
            }
            buf[0] = 0;
            i = j;
            if (path[i] == ':') i++ ;
        }
    } else {
		fprintf(stderr, "PATH variable not set\n");
	}
    /* If the buffer is still empty, the command was not found */
    if (buf[0] == 0) return NULL ;
    /* Otherwise truncate the command name to yield path only */
    lg = strlen(buf) - 1 ;
    while (buf[lg]!='/') {
        buf[lg]=0 ;
        lg -- ;
    }
    buf[lg] = 0;
    return buf ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Opens up a gnuplot session, ready to receive commands.
  @return	Newly allocated gnuplot control structure.

  This opens up a new gnuplot session, ready for input. The struct
  controlling a gnuplot session should remain opaque and only be
  accessed through the provided functions.

  The session must be closed using gnuplot_close().
 */
/*--------------------------------------------------------------------------*/

gnuplot_ctrl * gnuplot_init(const char* display_)
{
    gnuplot_ctrl *  handle ;

    if (getenv("DISPLAY") == NULL) {
        fprintf(stderr, "cannot find DISPLAY variable: is it set?\n") ;
    }
	if (gnuplot_get_program_path("gnuplot")==NULL) {
		fprintf(stderr, "cannot find gnuplot in your PATH");
		return NULL ;
	}

    /* 
     * Structure initialization:
     */
    handle = (gnuplot_ctrl*)malloc(sizeof(gnuplot_ctrl)) ;
    handle->nplots = 0 ;
    gnuplot_setstyle(handle, "points") ;
    handle->ntmp = 0 ;

    if (display_ != NULL) {
      static char cmd[80];
      snprintf(cmd, sizeof cmd, "gnuplot -geometry 480x360 -display %s",display_);

      handle->gnucmd = popen(cmd,"w");
    } else {
      handle->gnucmd = popen("gnuplot -geometry 480x360", "w") ;
    }
    if (handle->gnucmd == NULL) {
        fprintf(stderr, "error starting gnuplot\n") ;
        free(handle) ;
        return NULL ;
    }
    return handle;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Closes a gnuplot session previously opened by gnuplot_init()
  @param	handle Gnuplot session control handle.
  @return	void

  Kills the child PID and deletes all opened temporary files.
  It is mandatory to call this function to close the handle, otherwise
  temporary files are not cleaned and child process might survive.

 */
/*--------------------------------------------------------------------------*/

void gnuplot_close(gnuplot_ctrl * handle)
{
    int     i ;
	
    if (pclose(handle->gnucmd) == -1) {
        fprintf(stderr, "problem closing communication to gnuplot\n") ;
        return ;
    }
    if (handle->ntmp) {
        for (i=0 ; i<handle->ntmp ; i++) {
            remove(handle->to_delete[i]) ;
        }
    }
    free(handle) ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Sends a command to an active gnuplot session.
  @param	handle Gnuplot session control handle
  @param	cmd    Command to send, same as a printf statement.

  This sends a string to an active gnuplot session, to be executed.
  There is strictly no way to know if the command has been
  successfully executed or not.
  The command syntax is the same as printf.

  Examples:

  @code
  gnuplot_cmd(g, "plot %d*x", 23.0);
  gnuplot_cmd(g, "plot %g * cos(%g * x)", 32.0, -3.0);
  @endcode

  Since the communication to the gnuplot process is run through
  a standard Unix pipe, it is only unidirectional. This means that
  it is not possible for this interface to query an error status
  back from gnuplot.
 */
/*--------------------------------------------------------------------------*/

void gnuplot_cmd(gnuplot_ctrl *  handle, const char *  cmd, ...)
{
    va_list ap ;
    char    local_cmd[GP_CMD_SIZE];

    va_start(ap, cmd);
    vsnprintf(local_cmd, sizeof local_cmd, cmd, ap);
    va_end(ap);

    //strcat(local_cmd, "\n");

    fputs(local_cmd, handle->gnucmd) ;
    fflush(handle->gnucmd) ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Change the plotting style of a gnuplot session.
  @param	h Gnuplot session control handle
  @param	plot_style Plotting-style to use (character string)
  @return	void

  The provided plotting style is a character string. It must be one of
  the following:

  - lines
  - points
  - linespoints
  - impulses
  - dots
  - steps
  - errorbars
  - boxes
  - boxeserrorbars
 */
/*--------------------------------------------------------------------------*/

void gnuplot_setstyle(gnuplot_ctrl * h, const char * plot_style) 
{
    if (strcmp(plot_style, "lines") &&
        strcmp(plot_style, "points") &&
        strcmp(plot_style, "linespoints") &&
        strcmp(plot_style, "impulses") &&
        strcmp(plot_style, "dots") &&
        strcmp(plot_style, "steps") &&
        strcmp(plot_style, "errorbars") &&
        strcmp(plot_style, "boxes") &&
        strcmp(plot_style, "boxerrorbars")) {
        fprintf(stderr, "warning: unknown requested style: using points\n") ;
        strcpy(h->pstyle, "points") ;
    } else {
        strcpy(h->pstyle, plot_style) ;
    }
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Sets the x label of a gnuplot session.
  @param	h Gnuplot session control handle.
  @param	label Character string to use for X label.
  @return	void

  Sets the x label for a gnuplot session.
 */
/*--------------------------------------------------------------------------*/

void gnuplot_set_xlabel(gnuplot_ctrl * h, const char * label)
{
    char    cmd[GP_CMD_SIZE] ;

    snprintf(cmd, sizeof cmd, "set xlabel \"%s\"\n", label) ;
    gnuplot_cmd(h, cmd) ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Sets the y label of a gnuplot session.
  @param	h Gnuplot session control handle.
  @param	label Character string to use for Y label.
  @return	void

  Sets the y label for a gnuplot session.
 */
/*--------------------------------------------------------------------------*/

void gnuplot_set_ylabel(gnuplot_ctrl * h, const char * label)
{
    char    cmd[GP_CMD_SIZE] ;

    snprintf(cmd, sizeof cmd, "set ylabel \"%s\"\n", label) ;
    gnuplot_cmd(h, cmd) ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Resets a gnuplot session (next plot will erase previous ones).
  @param	h Gnuplot session control handle.
  @return	void

  Resets a gnuplot session, i.e. the next plot will erase all previous
  ones.
 */
/*--------------------------------------------------------------------------*/

void gnuplot_resetplot(gnuplot_ctrl * h)
{
    int     i ;
    if (h->ntmp) {
        for (i=0 ; i<h->ntmp ; i++) {
            remove(h->to_delete[i]) ;
        }
    }
    h->ntmp = 0 ;
    h->nplots = 0 ;
    return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Plots a 2d graph from a list of doubles.
  @param	handle	Gnuplot session control handle.
  @param	d		Array of doubles.
  @param	n		Number of values in the passed array.
  @param	title	Title of the plot.
  @return	void

  Plots out a 2d graph from a list of doubles. The x-coordinate is the
  index of the double in the list, the y coordinate is the double in
  the list.

  Example:

  @code
    gnuplot_ctrl    *h ;
    double          d[50] ;
    int             i ;

    h = gnuplot_init() ;
    for (i=0 ; i<50 ; i++) {
        d[i] = (double)(i*i) ;
    }
    gnuplot_plot_x(h, d, 50, "parabola") ;
    sleep(2) ;
    gnuplot_close(h) ;
  @endcode
 */
/*--------------------------------------------------------------------------*/

void gnuplot_plot_x(
    gnuplot_ctrl    *   handle,
    double          *   d,
    int                 n,
    const char      *   title
)
{
    int     i ;
	int		tmpfd ;
	int     retval;
    char    name[128] ;
    char    cmd[GP_CMD_SIZE] ;
    char    line[GP_CMD_SIZE] ;


	if (handle==NULL || d==NULL || (n<1)) return ;

    /* Open one more temporary file? */
    if (handle->ntmp == GP_MAX_TMP_FILES - 1) {
        fprintf(stderr,
                "maximum # of temporary files reached (%d): cannot open more",
                GP_MAX_TMP_FILES) ;
        return ;
    }

    /* Open temporary file for output   */
	snprintf(name, sizeof name, "%s/gnuplot-i-XXXXXX", P_tmpdir);
    if ((tmpfd=mkstemp(name))==-1) {
        fprintf(stderr,"cannot create temporary file: exiting plot") ;
        return ;
    }

    /* Store file name in array for future deletion */
    strcpy(handle->to_delete[handle->ntmp], name) ;
    handle->ntmp ++ ;
    /* Write data to this file  */
    for (i=0 ; i<n ; i++) {
		snprintf(line, sizeof line, "%g\n", d[i]);
		retval=write(tmpfd, line, strlen(line));
		(void)retval;
    }
    close(tmpfd) ;

    /* Command to be sent to gnuplot    */
    if (handle->nplots > 0) {
        strcpy(cmd, "replot") ;
    } else {
        strcpy(cmd, "plot") ;
    }
    
    if (title == NULL) {
        snprintf(line, sizeof line, "%s \"%s\" with %s\n", cmd, name, handle->pstyle) ;
    } else {
        snprintf(line, sizeof line, "%s \"%s\" title \"%s\" with %s\n", cmd, name,
                      title, handle->pstyle) ;
    }

    /* send command to gnuplot  */
    gnuplot_cmd(handle, line) ;
    handle->nplots++ ;
    return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Plot a 2d graph from a list of points.
  @param	handle		Gnuplot session control handle.
  @param	x			Pointer to a list of x coordinates.
  @param	y			Pointer to a list of y coordinates.
  @param	n			Number of doubles in x (assumed the same as in y).
  @param	title		Title of the plot.
  @return	void

  Plots out a 2d graph from a list of points. Provide points through a list
  of x and a list of y coordinates. Both provided arrays are assumed to
  contain the same number of values.

  @code
    gnuplot_ctrl    *h ;
	double			x[50] ;
	double			y[50] ;
    int             i ;

    h = gnuplot_init() ;
    for (i=0 ; i<50 ; i++) {
        x[i] = (double)(i)/10.0 ;
        y[i] = x[i] * x[i] ;
    }
    gnuplot_plot1_xy(h, x, y, 50, "parabola") ;
    sleep(2) ;
    gnuplot_close(h) ;
  @endcode
 */
/*--------------------------------------------------------------------------*/

void gnuplot_plot_xy(
    gnuplot_ctrl    *   handle,
	double			*	x,
	double			*	y,
    int                 n,
    const char      *   title
)
{
  int     i ;
  char    cmd[GP_CMD_SIZE] ;
  char    line[GP_CMD_SIZE] ;

  if (handle==NULL || x==NULL || y==NULL || (n<1)) return ;

  /* Command to be sent to gnuplot    */
  if (handle->nplots > 0) {
    strcpy(cmd, "replot") ;
  } else {
    strcpy(cmd, "plot") ;
  }

  if (title == NULL) {
    snprintf(line, sizeof line, "%s \"%s\" with %s\n", cmd, "-", handle->pstyle) ;
  } else {
    snprintf(line, sizeof line, "%s \"%s\" title \"%s\" with %s\n", cmd, "-",
	    title, handle->pstyle) ;
  }

  /* send command to gnuplot  */
  gnuplot_cmd(handle, line) ;

  /* Write data to the pipe  */
  for (i=0 ; i<n; i++) {
    fprintf(handle->gnucmd, "%g %g\n", x[i], y[i]) ;
  }
  fprintf(handle->gnucmd, "e\n"); // end data input

  handle->nplots++ ;
  return ;
}



/*-------------------------------------------------------------------------*/
/**
  @brief	Open a new session, plot a signal, close the session.
  @param	title	Plot title
  @param	style	Plot style
  @param	label_x	Label for X
  @param	label_y	Label for Y
  @param	x		Array of X coordinates
  @param	y		Array of Y coordinates (can be NULL)
  @param	n		Number of values in x and y.
  @return

  This function opens a new gnuplot session, plots the provided
  signal as an X or XY signal depending on a provided y, waits for
  a carriage return on stdin and closes the session.

  It is Ok to provide an empty title, empty style, or empty labels for
  X and Y. Defaults are provided in this case.
 */
/*--------------------------------------------------------------------------*/

void gnuplot_plot_once(
	const char	*	title,
	const char	*	style,
	const char	*	label_x,
	const char	*	label_y,
	double		*	x,
	double		*	y,
	int			n
)
{
	gnuplot_ctrl	*	handle ;

	if (x==NULL || n<1) return ;

	handle = gnuplot_init();
	if (style!=NULL) {
		gnuplot_setstyle(handle, style);
	} else {
		gnuplot_setstyle(handle, "lines");
	}
	if (label_x!=NULL) {
		gnuplot_set_xlabel(handle, label_x);
	} else {
		gnuplot_set_xlabel(handle, "X");
	}
	if (label_y!=NULL) {
		gnuplot_set_ylabel(handle, label_y);
	} else {
		gnuplot_set_ylabel(handle, "Y");
	}
	if (y==NULL) {
		gnuplot_plot_x(handle, x, n, title);
	} else {
		gnuplot_plot_xy(handle, x, y, n, title);
	}
	printf("press ENTER to continue\n");
	while (getchar()!='\n') {}
	gnuplot_close(handle);
	return ;
}




/*-------------------------------------------------------------------------*/
/**
  @brief	Plot a slope on a gnuplot session.
  @param	handle		Gnuplot session control handle.
  @param	a			Slope.
  @param	b			Intercept.
  @param	title		Title of the plot.
  @return	void
  @doc

  Plot a slope on a gnuplot session. The provided slope has an
  equation of the form y=ax+b

  Example:

  @code
    gnuplot_ctrl    *   h ;
    double              a, b ;

    h = gnuplot_init() ;
    gnuplot_plot_slope(h, 1.0, 0.0, "unity slope") ;
    sleep(2) ;
    gnuplot_close(h) ;
  @endcode
 */
/*--------------------------------------------------------------------------*/
    

void gnuplot_plot_slope(
    gnuplot_ctrl    *   handle,
    double              a,
    double              b,
    const char      *   title
)
{
    char    stitle[GP_TITLE_SIZE] ;
    char    cmd[GP_CMD_SIZE] ;

    if (title == NULL) {
        strcpy(stitle, "no title") ;
    } else {
        strncpy(stitle, title, sizeof stitle) ;
        stitle[sizeof stitle - 1] = 0;
    }

    if (handle->nplots > 0) {
        snprintf(cmd, sizeof cmd, "replot %g * x + %g title \"%s\" with %s\n",
                      a, b, title, handle->pstyle) ;
    } else {
        snprintf(cmd, sizeof cmd, "plot %g * x + %g title \"%s\" with %s\n",
                      a, b, title, handle->pstyle) ;
    }
    gnuplot_cmd(handle, cmd) ;
    handle->nplots++ ;
    return ;
}


/*-------------------------------------------------------------------------*/
/**
  @brief	Plot a curve of given equation y=f(x).
  @param	h			Gnuplot session control handle.
  @param	equation	Equation to plot.
  @param	title		Title of the plot.
  @return	void

  Plots out a curve of given equation. The general form of the
  equation is y=f(x), you only provide the f(x) side of the equation.

  Example:

  @code
        gnuplot_ctrl    *h ;
        char            eq[80] ;

        h = gnuplot_init() ;
        strcpy(eq, "sin(x) * cos(2*x)") ;
        gnuplot_plot_equation(h, eq, "sine wave", normal) ;
        gnuplot_close(h) ;
  @endcode
 */
/*--------------------------------------------------------------------------*/

void gnuplot_plot_equation(
    gnuplot_ctrl    *   h,
    const char      *   equation,
    const char      *   title
)
{
    char    cmd[GP_CMD_SIZE];
    char    plot_str[GP_EQ_SIZE] ;
    char    title_str[GP_TITLE_SIZE] ;

    if (title == NULL) {
        strcpy(title_str, "no title") ;
    } else {
        strncpy(title_str, title, sizeof title_str) ;
        title_str[sizeof title_str - 1] = 0;
    }
    if (h->nplots > 0) {
        strcpy(plot_str, "replot") ;
    } else {
        strcpy(plot_str, "plot") ;
    }

    snprintf(cmd, sizeof cmd, "%s %s title \"%s\" with %s\n", 
                  plot_str, equation, title_str, h->pstyle) ;
    gnuplot_cmd(h, cmd) ;
    h->nplots++ ;
    return ;
}

void gnuplot_write_matrix(gnuplot_ctrl* h,
			  Array<double, 2> matrix, bool matrixstyle)
{
  /* Write data to the pipe  */
  if (!matrixstyle) {
    for (int i = 0; i < matrix.extent(firstDim); i++) {
      for (int j = 0; j < matrix.extent(secondDim); j++) {
	fprintf(h->gnucmd, "%d %g\n", j, matrix(i,j));
      }
      fprintf(h->gnucmd, "e\n"); // end data input
    }
  } else {
    for (int i = 0; i < matrix.extent(firstDim); i++) {
      for (int j = 0; j < matrix.extent(secondDim); j++) {
	fprintf(h->gnucmd, "%g ", matrix(i,j));
      }
      fprintf(h->gnucmd, "\n"); // end data input
    }
    fprintf(h->gnucmd, "e\n"); // end data input
  }
  fflush(h->gnucmd);
}

#if 1
void gnuplot_splot(
    gnuplot_ctrl    *   h,
    Array<double, 2>    matrix,
    const char      *   title
)
{
    ofstream outfile;
    outfile.open ("/tmp/temp.dat", ofstream::out);
    for (int x = 0; x < matrix.rows(); ++x)
    {
        for (int y = 0; y < matrix.cols(); ++y)
        {
	    double val = 20*log(matrix(x, y));
	    val = matrix(x,y);
            outfile << (val < 0 ? 0 : val) << ' ';
        }
        outfile << endl;
    }
    outfile.close();
    sync();

#if 0
static int plotcount = 0;

    gnuplot_cmd(h, "set terminal postscript color\n");
    gnuplot_cmd(h, "set output \"skyplot_%d.ps\"\n", plotcount++);
    gnuplot_cmd(h, "set zrange [1e7:3.5e7]\n");
#endif

#if 0
    gnuplot_cmd(h, "set pm3d at s hidden3d 100\n");
    gnuplot_cmd(h, "set style line 100 lt 5 lw 0.5\n");
    gnuplot_cmd(h, "unset hidden3d\n");
    gnuplot_cmd(h, "unset surf\n");
#else
    gnuplot_cmd(h, "set pm3d at s scansforward corners2color c1\n");
    gnuplot_cmd(h, "unset hidden3d\n");
    //gnuplot_cmd(h, "unset surf");
#endif
//     gnuplot_cmd(h, "set pm3d at s scansforward");
//     gnuplot_cmd(h, "set key below");
//     gnuplot_cmd(h, "set border 4095");
    gnuplot_cmd(h, "unset surface");
//     gnuplot_cmd(h, "set samples 25");
//     gnuplot_cmd(h, "set isosamples 20");
//     gnuplot_cmd(h, "set ticslevel 0");
    gnuplot_cmd(h, "set xrange [0:%d]\n", matrix.cols());
    gnuplot_cmd(h, "set yrange [0:%d]\n", matrix.rows());
    gnuplot_cmd(h, "set title '%s'\n", title);
    gnuplot_cmd(h, "set view 130,40\n");

    gnuplot_cmd(h, "splot '/tmp/temp.dat' matrix\n") ;
}

void gnuplot_contour_plot(
    gnuplot_ctrl    *   h,
    Array<double, 2>    matrix,
    const char      *   title
)
{
    ofstream outfile;
    outfile.open ("/tmp/temp.dat", ofstream::out);
    for (int x = 0; x < matrix.rows(); ++x)
    {
        for (int y = 0; y < matrix.cols(); ++y)
        {
            outfile << matrix(x, y) << ' ';
        }
        outfile << endl; 
   }
    outfile.close();

    gnuplot_cmd(h, "set pm3d\n");
//     gnuplot_cmd(h, "set pm3d at s scansforward");
//     gnuplot_cmd(h, "set key below");
//     gnuplot_cmd(h, "set border 4095");
    gnuplot_cmd(h, "unset surface\n");
//    gnuplot_cmd(h, "set contour surface");
//     gnuplot_cmd(h, "set samples 25");
//     gnuplot_cmd(h, "set isosamples 20");
//     gnuplot_cmd(h, "set ticslevel 0");
    gnuplot_cmd(h, "set title '%s'\n", title);
    gnuplot_cmd(h, "set view 60,30\n");
    gnuplot_cmd(h, "splot '/tmp/temp.dat' matrix\n") ;
}

#endif
