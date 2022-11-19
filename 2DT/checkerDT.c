/* Identifies all or nearly all of the buggy DTs' issues and reports
them to stderr with a corresponding message. The implementation of these
checker functions thoroughly exercise checks of every invariant of the 
data structures' internal representations and their interfaces' stated 
restrictions.*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "checkerDT.h"
#include "dynarray.h"
#include "path.h"


/* see checkerDT.h for specification */
boolean CheckerDT_Node_isValid(Node_T oNNode) {
   Node_T oNParent;
   Path_T oPNPath;
   Path_T oPPPath;

   /* Sample check: a NULL pointer is not a valid node */
   if(oNNode == NULL) {
      fprintf(stderr, "A node is a NULL pointer\n");
      return FALSE;
   }

   /* Sample check: parent's path must be the longest possible
      proper prefix of the node's path */
   oNParent = Node_getParent(oNNode);
   if(oNParent != NULL) {
      oPNPath = Node_getPath(oNNode);
      oPPPath = Node_getPath(oNParent);

      if(Path_getSharedPrefixDepth(oPNPath, oPPPath) !=
         Path_getDepth(oPNPath) - 1) {
         fprintf(stderr, "P-C nodes don't have P-C paths: (%s) (%s)\n",
                 Path_getPathname(oPPPath), Path_getPathname(oPNPath));
         return FALSE;
      }
   }

   return TRUE;
}

/* Performs a pre-order traversal of the tree rooted at oNNode.
   Increments the dirCount for every node checked in tree.
   Returns FALSE and prints message to stderr if a broken invariant is 
   found returns TRUE otherwise. */
static boolean CheckerDT_treeCheck(Node_T oNNode, size_t *dirCount) {
   size_t ulIndex;
   int nodeComparison;
   int iStatus;
   
   if(oNNode!= NULL) {
      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerDT_Node_isValid(oNNode))
         return FALSE;

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         Node_T oNChildPrev = NULL;
         iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
         
         if(iStatus != SUCCESS) {
         fprintf(stderr, 
         "getNumChildren claims more children than getChild returns\n");
         return FALSE;
         }

         if (ulIndex != 0) {
            /* compare current node to previous node */
            (void) Node_getChild(oNNode, ulIndex-1, &oNChildPrev);
            /* if same paath, report duplicate path*/
            nodeComparison = Path_comparePath(Node_getPath(oNChild), 
               Node_getPath(oNChildPrev));
            if(nodeComparison == 0) {
               fprintf(stderr, "Duplicate path detected in tree\n");
               return FALSE;
            }
            /* report if lexicographically misordered*/
            if(nodeComparison < 0) {
               fprintf(stderr, "Children not in lexicographic order\n");
               return FALSE;
            }
         }
          /* counter incremented for size of tree */
         *dirCount=(*dirCount) + 1;

         /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
         if(!CheckerDT_treeCheck(oNChild, dirCount))
            return FALSE;
      }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerDT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {
   /* initialize counter to 1 for root*/
   size_t counter = 1;

   /* Sample check on a top-level data structure invariant:
      if the DT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }   
   if(oNRoot == NULL) return TRUE;

   /* compare absolute ulCount to counter variable from treeCheck */
   if(CheckerDT_treeCheck(oNRoot, &counter)) {
      if (counter != ulCount) {
         fprintf(stderr, "Total number of directories do not match \n");
         return FALSE;
      }
      /* only if all invariants are properly checked for return TRUE*/
      return TRUE;
   }
   return FALSE;
}