// (c) Sylvain JAUME <sylvain at mit.edu> 2005

#include "vtkFileOutputWindow.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <float.h>

#define PEDANTIC_GLOBAL_VAR 0
#define ERROR_GLOBAL_VAR 1
#define WARNING_GLOBAL_VAR 1
#define ASSERT_GLOBAL_VAR 1
#define PRINTOUT_GLOBAL_VAR 1
#define DEBUG_GLOBAL_VAR 1

// not yet defined macros of Sylvain
#define MAIN_MACRO {}
#define START_TIME_MACRO {}
#define END_TIME_MACRO {}

#define USAGE_MACRO( message ) {					\
    PRINTOUT_MACRO( "argc " << argc );					\
    PRINTOUT_MACRO( "usage " << argv[0] << " " << message );}

#ifdef ASSERT_GLOBAL_VAR
#define ASSERT_MACRO( test ) { assert( test ); }
#else
#define ASSERT_MACRO( test )
#endif

#ifdef DEBUG_GLOBAL_VAR
#define DEBUG_MACRO( message ) { std::cout /*<< __FILE__ << " " */	\
      << " " << message << std::endl; }
#else
#define DEBUG_MACRO( message )
#endif

#ifdef PRINTOUT_GLOBAL_VAR
#define PRINTOUT_MACRO( message ) { std::cout /* << __FILE__ << " " */ \
      << " " << message << std::endl; }
#else
#define PRINTOUT_MACRO( message )
#endif

#define CONTACT_MACRO \
  PRINTOUT_MACRO( "Send your comments to Sylvain JAUME <sylvain at mit.edu>" )
#define START_MACRO( message ) { CONTACT_MACRO; \
    PRINTOUT_MACRO( message << " start" ) }
#define MAIN_START_MACRO CONTACT_MACRO;		\
    PRINTOUT_MACRO( argv[0] << " start" );	\
    for ( int m = 1; m < argc; m++ )			\
      { PRINTOUT_MACRO( "argv[" << m << "] " << argv[m] ); } \
    vtkFileOutputWindow *outwin = vtkFileOutputWindow::New();	\
    outwin->SetFileName("logname.txt");				\
    outwin->SetInstance(outwin);
#define MAIN_END_MACRO { PRINTOUT_MACRO( argv[0] << " done" ); }
#define END_MACRO( message ) PRINTOUT_MACRO( message << " done" )
#define READ_MACRO( message ) PRINTOUT_MACRO( "read " << message )
#define WRITE_MACRO( message ) PRINTOUT_MACRO( "write " << message )

#ifdef ERROR_GLOBAL_VAR
#define	ERROR_MACRO( message ) \
  { std::cerr << __FILE__ << " " << __LINE__ << " Error " << message	\
	      << std::endl; }
#else
#define ERROR_MACRO( message )
#endif

#ifdef WARNING_GLOBAL_VAR
#define	WARNING_MACRO( message ) \
  { std::cerr << __FILE__ << " " << __LINE__ << " Warning " << message	\
	      <<  std::endl; }
#else
#define WARNING_MACRO( message )
#endif

#if PEDANTIC_GLOBAL_VAR
#define PEDANTIC_MACRO( message ) DEBUG_MACRO( message )
#else
#define PEDANTIC_MACRO( message )
#endif

#if DEBUG_GLOBAL_VAR
#define PRINTOUT_CELL( message, pts ) {					\
    if ( npts == 1 )							\
      {									\
	PEDANTIC_MACRO( message << " (" << pts[0] << ")" );		\
      }									\
    else if ( npts == 2 )						\
      {									\
	PEDANTIC_MACRO( message << " (" << pts[0] << " " << pts[1] << ")" ); \
      }									\
    else if ( npts == 3 )						\
      {									\
	PEDANTIC_MACRO( message << " ("	<< pts[0] << " " << pts[1] << " " \
			<< pts[2] << ")" );				\
      }									\
    else								\
      {									\
	PEDANTIC_MACRO( message << " npts " << npts );			\
      }									\
									\
    ASSERT_MACRO( npts > 0 );						\
  }
#else
#define PRINTOUT_CELL( pts )
#endif
