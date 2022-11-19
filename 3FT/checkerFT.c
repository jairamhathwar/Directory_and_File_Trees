/* Identifies and validates the file tree module. The implementation of
these checker functions thoroughly exercise checks of every invariant of
the data structures' internal representations and their interfaces' 
stated restrictions */

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

/* Checks whether there are adjacent children nodes of the parent oNNode
(oNChild and oNChildPrev) by passing ulIndex and if oNChildPrev is same
as the passed in type of oNChild, performs validation checks for 
duplicate paths and lexicographic order. 
Note: the ordering of files before directories appears in the toString
method in ft.c*/
static boolean checkNodeCompare(Node_T oNNode, Node_T oNChild, 
   Node_T oNChildPrev, size_t ulIndex, boolean type) {
   int prevStatus;
   int nodeComparison;

   if (ulIndex != 0) {
      prevStatus = Node_getChild(oNNode, ulIndex-1, 
         &oNChildPrev);

      /* compare current node to previous node, staying 
      consistent with the type */
      if((prevStatus == NOT_A_DIRECTORY && type == TRUE) || 
         prevStatus == SUCCESS && type == FALSE) {
         nodeComparison = Path_comparePath(Node_getPath(oNChild), 
            Node_getPath(oNChildPrev));
         /* if same path, report duplicate path*/
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
   }
   return TRUE;
}


/* Performs a pre-order traversal of the tree rooted at oNNode. 
   Increments the nodeCount for every node in tree.
   Returns FALSE and prints message to stderr if a broken invariant is 
   found and returns TRUE otherwise. */
static boolean CheckerFT_treeCheck(Node_T oNNode, size_t *nodeCount) {
   size_t ulIndex;
   int iStatus;
   
   if(oNNode!= NULL) {
      /* Sample check on each node: node must be valid */
      /* If not, pass that failure back up immediately */
      if(!CheckerFT_Node_isValid(oNNode))
         return FALSE;

      /* Recur on every child of oNNode */
      for(ulIndex = 0; ulIndex < Node_getNumChildren(oNNode); ulIndex++)
      {
         Node_T oNChild = NULL;
         Node_T oNChildPrev = NULL;
         iStatus = Node_getChild(oNNode, ulIndex, &oNChild);

         /* if it's a file, then perform file checks. ordering of files
         first then directories handled in toString method of ft.c*/
         if (iStatus == NOT_A_DIRECTORY) {
            if(!checkNodeCompare(oNNode, oNChild, oNChildPrev, 
               ulIndex, TRUE)) return FALSE;
            *nodeCount = (*nodeCount)+1;
         }

         /* if other broken invariant detected return FALSE*/
         else if(iStatus != SUCCESS) {
            fprintf(stderr, 
         "getNumChildren claims more children than getChild returns\n");
            return FALSE;
         }

         /*if it's a directory then perform directory checks*/
         else {
            if(!checkNodeCompare(oNNode, oNChild, oNChildPrev, 
               ulIndex, FALSE)) return FALSE;
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