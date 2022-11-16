/*--------------------------------------------------------------------*/
/* checkerFT.c                                                        */
/* Author:                                                            */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>  
#include "checkerFT.h"
#include "dynarray.h"
#include "path.h"

/* see checkerFT.h for specification */
boolean CheckerFT_Node_isValid(Node_T oNNode) {
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

/*
   Performs a pre-order traversal of the tree rooted at oNNode.
   Returns FALSE if a broken invariant is found and
   returns TRUE otherwise.

   You may want to change this function's return type or
   parameter list to facilitate constructing your checks.
   If you do, you should update this function comment.
*/
static boolean CheckerFT_treeCheck(Node_T oNNode, size_t *nodeCount) {
   size_t ulIndex;
   int prevStatus;
   int nodeComparison;
   int iStatus;
   
   if(oNNode!= NULL) {
      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerFT_Node_isValid(oNNode))
         return FALSE;

      /* temp variable count. change return type to whatever count. use temp variable to store whatever it's returning*/
      /* in for loop, counter increment for size of tree compare to other value. if different, print error. else, chill */

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         Node_T oNChildPrev = NULL;
         iStatus = Node_getChild(oNNode, ulIndex, &oNChild);
         if (iStatus == NOT_A_DIRECTORY) {
            *nodeCount = (*nodeCount)+1;
         }
         else if(iStatus != SUCCESS) {
            fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         else {
            if (ulIndex != 0) {
            prevStatus = Node_getChild(oNNode, ulIndex-1, &oNChildPrev);
            if(prevStatus != SUCCESS) {
               fprintf(stderr, "getNumChildren claims more children than getChild returns\n");
               return FALSE;
               }

            nodeComparison = Path_comparePath(Node_getPath(oNChild), Node_getPath(oNChildPrev));
            if(nodeComparison == 0) {
               fprintf(stderr, "Duplicate path detected in tree\n");
               return FALSE;
               }
            if(nodeComparison < 0) {
               fprintf(stderr, "Children not in lexicographic order\n");
               return FALSE;
               }
            }
            *nodeCount=(*nodeCount) + 1;

            /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
            if(!CheckerFT_treeCheck(oNChild, nodeCount))
               return FALSE;
         }
      }
   }
   return TRUE;
}

/* see checkerFT.h for specification */
boolean CheckerFT_isValid(boolean bIsInitialized, Node_T oNRoot,
                          size_t ulCount) {
   size_t counter = 1;

   /* Sample check on a top-level data structure invariant:
      if the FT is not initialized, its count should be 0. */
   if(!bIsInitialized)
      if(ulCount != 0) {
         fprintf(stderr, "Not initialized, but count is not 0\n");
         return FALSE;
      }

   /* compare counter with other value (how many nodes should be there)
      2 if statements */
   if(oNRoot == NULL) return TRUE;
   if(getType(oNRoot)) return FALSE;

   if(CheckerFT_treeCheck(oNRoot, &counter)) {
      if (counter != ulCount) {
         fprintf(stderr, "Total number of nodes do not match \n");
         return FALSE;
      }
      return TRUE;
   }
   return FALSE;
}