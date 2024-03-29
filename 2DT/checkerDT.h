/* Functions used in checkerDT.c and their corresponding
parameters they take in.*/

#ifndef CHECKER_INCLUDED
#define CHECKER_INCLUDED

#include "nodeDT.h"


/*
   Returns TRUE if oNNode represents a directory entry
   in a valid state, or FALSE otherwise.
*/
boolean CheckerDT_Node_isValid(Node_T oNNode);

/*
   Returns TRUE if the hierarchy is in a valid state or FALSE
   otherwise.  The data structure's validity is based on a boolean
   bIsInitialized indicating whether the DT is in an initialized
   state, a Node_T oNRoot representing the root of the hierarchy, and
   a size_t ulCount representing the total number of directories in
   the hierarchy.
*/
boolean CheckerDT_isValid(boolean bIsInitialized,
                          Node_T oNRoot,
                          size_t ulCount);

#endif
