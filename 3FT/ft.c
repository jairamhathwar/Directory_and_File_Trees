/* Implementation of a file tree composed of directories and files */

#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "dynarray.h"
#include "path.h"
#include "nodeFT.h"
#include "checkerFT.h"
#include "ft.h"
/*
  A File Tree is a representation of a hierarchy of directories and
  files, represented as an AO with 3 state variables:
*/
/* 1. a flag for being in an initialized state (TRUE) or not (FALSE) */
static boolean bIsInitialized;
/* 2. a pointer to the root node in the hierarchy */
static Node_T oNRoot;
/* 3. a counter of the number of nodes in the hierarchy */
static size_t ulCount;
/* --------------------------------------------------------------------
  The FT_traversePath and FT_findNode functions modularize the common
  functionality of going as far as possible down an FT towards a path
  and returning either the node of however far was reached or the
  node if the full path was reached, respectively.
*/
/*
  Traverses the FT starting at the root as far as possible towards
  absolute path oPPath. If able to traverse, returns an int SUCCESS
  status and sets *poNFurthest to the furthest node reached (which may
  be only a prefix of oPPath, or even NULL if the root is NULL).
  Otherwise, sets *poNFurthest to NULL and returns with status:
  * CONFLICTING_PATH if the root's path is not a prefix of oPPath
  * MEMORY_ERROR if memory could not be allocated to complete request
  * NOT_A_DIRECTORY if common node was a file
*/
static int FT_traversePath(Path_T oPPath, Node_T *poNFurthest) {
   int iStatus;
   Path_T oPPrefix = NULL;
   Node_T oNCurr;
   Node_T oNChild = NULL;
   size_t ulDepth;
   size_t i;
   size_t ulChildID;
   assert(oPPath != NULL);
   assert(poNFurthest != NULL);

   /* root is NULL -> won't find anything */
   if(oNRoot == NULL) {
      *poNFurthest = NULL;
      return SUCCESS;
   }
   iStatus = Path_prefix(oPPath, 1, &oPPrefix);
   if(iStatus != SUCCESS) {
      *poNFurthest = NULL;
      return iStatus;
   }
   if(Path_comparePath(Node_getPath(oNRoot), oPPrefix)) {
      Path_free(oPPrefix);
      *poNFurthest = NULL;
      return CONFLICTING_PATH;
   }
   Path_free(oPPrefix);
   oPPrefix = NULL;
   oNCurr = oNRoot;
   ulDepth = Path_getDepth(oPPath);
   for(i = 2; i <= ulDepth; i++) {
      iStatus = Path_prefix(oPPath, i, &oPPrefix);
      if(iStatus != SUCCESS) {
         *poNFurthest = NULL;
         return iStatus;
      }
      if(Node_hasChild(oNCurr, oPPrefix, &ulChildID)) {
         /* go to that child and continue with next prefix */
         Path_free(oPPrefix);
         oPPrefix = NULL;
         iStatus = Node_getChild(oNCurr, ulChildID, &oNChild);
         if(iStatus != SUCCESS) {
            *poNFurthest = NULL;
            return iStatus;
         }
         oNCurr = oNChild;
      }
      else {
         /* oNCurr doesn't have child with path oPPrefix:
            this is as far as we can go */
         break;
      }
   }
   Path_free(oPPrefix);
   *poNFurthest = oNCurr;
   return SUCCESS;
}
/*
  Traverses the FT to find a node with absolute path pcPath. Returns a
  int SUCCESS status and sets *poNResult to be the node, if found.
  Otherwise, sets *poNResult to NULL and returns with status:
  * INITIALIZATION_ERROR if the FT is not in an initialized state
  * BAD_PATH if pcPath does not represent a well-formatted path
  * CONFLICTING_PATH if the root's path is not a prefix of pcPath
  * NO_SUCH_PATH if no node with pcPath exists in the hierarchy
  * MEMORY_ERROR if memory could not be allocated to complete request
 */
static int FT_findNode(const char *pcPath, Node_T *poNResult) {
   Path_T oPPath = NULL;
   Node_T oNFound = NULL;
   int iStatus;
   assert(pcPath != NULL);
   assert(poNResult != NULL);
   if(!bIsInitialized) {
      *poNResult = NULL;
      return INITIALIZATION_ERROR;
   }
   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS) {
      *poNResult = NULL;
      return iStatus;
   }
   iStatus = FT_traversePath(oPPath, &oNFound);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      *poNResult = NULL;
      return iStatus;
   }
   if(oNFound == NULL) {
      Path_free(oPPath);
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   if(Path_comparePath(Node_getPath(oNFound), oPPath) != 0) {
      Path_free(oPPath);
      *poNResult = NULL;
      return NO_SUCH_PATH;
   }
   Path_free(oPPath);
   *poNResult = oNFound;
   return SUCCESS;
}

/* see ft.h for specification */
int FT_insertDir(const char *pcPath) {
   int iStatus;
   Path_T oPPath = NULL;
   Node_T oNFirstNew = NULL;
   Node_T oNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;
   assert(pcPath != NULL);
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;
   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;
   /* find the closest ancestor of oPPath already in the tree */
   iStatus= FT_traversePath(oPPath, &oNCurr);
   if(iStatus != SUCCESS)
   {
      Path_free(oPPath);
      return iStatus;
   }
   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. */
   if(oNCurr == NULL && oNRoot != NULL) {
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }
   ulDepth = Path_getDepth(oPPath);
   if(oNCurr == NULL) /* new root! */
      ulIndex = 1;
   else {
      ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;
      /* oNCurr is the node we're trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       Node_getPath(oNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }
   /* starting at oNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;
      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
         return iStatus;
      }
      /* insert the new node for a directory at this level */
      iStatus = Node_newDir(oPPrefix, oNCurr, &oNNewNode);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         Path_free(oPPrefix);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
         return iStatus;
      }
      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if(oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }
   Path_free(oPPath);
   /* update FT state variables to reflect insertion */
   if(oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}

/* see ft.h for specification*/
int FT_insertFile(const char *pcPath, void *pvContents,
                  size_t ulLength) {
   int iStatus;
   Path_T oPPath = NULL;
   Node_T oNFirstNew = NULL;
   Node_T oNCurr = NULL;
   size_t ulDepth, ulIndex;
   size_t ulNewNodes = 0;
   assert(pcPath != NULL);
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   /* validate pcPath and generate a Path_T for it */
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;
   iStatus = Path_new(pcPath, &oPPath);
   if(iStatus != SUCCESS)
      return iStatus;
   /* find the closest ancestor of oPPath already in the tree */
   iStatus= FT_traversePath(oPPath, &oNCurr);
   if(iStatus != SUCCESS) {
      Path_free(oPPath);
      return iStatus;
   }
   /* no ancestor node found, so if root is not NULL,
      pcPath isn't underneath root. ensures new file would not be the
      ft root*/
   if((oNCurr == NULL && oNRoot != NULL) || Path_getDepth(oPPath) == 1){
      Path_free(oPPath);
      return CONFLICTING_PATH;
   }
   ulDepth = Path_getDepth(oPPath);
   if(oNCurr == NULL) /* new root! */
      ulIndex = 1;
   else {
      ulIndex = Path_getDepth(Node_getPath(oNCurr))+1;
      /* oNCurr is the node we're trying to insert */
      if(ulIndex == ulDepth+1 && !Path_comparePath(oPPath,
                                       Node_getPath(oNCurr))) {
         Path_free(oPPath);
         return ALREADY_IN_TREE;
      }
   }
   /* starting at oNCurr, build rest of the path one level at a time */
   while(ulIndex <= ulDepth) {
      Path_T oPPrefix = NULL;
      Node_T oNNewNode = NULL;
      /* generate a Path_T for this level */
      iStatus = Path_prefix(oPPath, ulIndex, &oPPrefix);
      if(iStatus != SUCCESS) {
         Path_free(oPPath);
         if(oNFirstNew != NULL)
            (void) Node_free(oNFirstNew);
         assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
         return iStatus;
      }
      if(ulIndex == ulDepth) {
        /* insert the new node file for this final level */
        iStatus = Node_newFile(oPPrefix, oNCurr, &oNNewNode, pvContents,
         ulLength);
        if(iStatus != SUCCESS) {
            Path_free(oPPath);
            Path_free(oPPrefix);
            if(oNFirstNew != NULL) (void) Node_free(oNFirstNew);
            assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
            return iStatus;
        }
      }
      
      else {
        /* insert the new node directory for all preceding levels */
        iStatus = Node_newDir(oPPrefix, oNCurr, &oNNewNode);
        if(iStatus != SUCCESS) {
            Path_free(oPPath);
            Path_free(oPPrefix);
            if(oNFirstNew != NULL) (void) Node_free(oNFirstNew);
            assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
            return iStatus;
        }
      }
      /* set up for next level */
      Path_free(oPPrefix);
      oNCurr = oNNewNode;
      ulNewNodes++;
      if(oNFirstNew == NULL)
         oNFirstNew = oNCurr;
      ulIndex++;
   }
   Path_free(oPPath);
   /* update FT state variables to reflect insertion */
   if(oNRoot == NULL)
      oNRoot = oNFirstNew;
   ulCount += ulNewNodes;
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}

/* see ft.h for specification*/
boolean FT_containsDir(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   assert(pcPath != NULL);
   iStatus = FT_findNode(pcPath, &oNFound);
   if(iStatus == SUCCESS) {
    if(!Node_getType(oNFound)) return TRUE; /* type is directory*/
   }
   return FALSE;
}

/* see ft.h for specification*/
boolean FT_containsFile(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   assert(pcPath != NULL);
   iStatus = FT_findNode(pcPath, &oNFound);
   if(iStatus == SUCCESS) {
    if(Node_getType(oNFound)) return TRUE; /* ensures type is file*/
   }
   return FALSE;
}
/* see ft.h for specification*/
int FT_rmDir(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   assert(pcPath != NULL);
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   iStatus = FT_findNode(pcPath, &oNFound);
   if(iStatus != SUCCESS)
       return iStatus;
   if(Node_getType(oNFound)) {
      return NOT_A_DIRECTORY; /* prevents removing file*/
   }
   ulCount -= Node_free(oNFound);
   if(ulCount == 0)
      oNRoot = NULL;
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}
/* see ft.h for specification*/
int FT_rmFile(const char *pcPath) {
   int iStatus;
   Node_T oNFound = NULL;
   assert(pcPath != NULL);
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   iStatus = FT_findNode(pcPath, &oNFound);
   if(iStatus != SUCCESS)
       return iStatus;
   if(!Node_getType(oNFound)) {
      return NOT_A_FILE; /* prevents removing directory*/
   }

   ulCount -= Node_free(oNFound);
   if(ulCount == 0)
      oNRoot = NULL;
   
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}
/* see ft.h for specification*/
int FT_init(void) {
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   if(bIsInitialized)
      return INITIALIZATION_ERROR;
   bIsInitialized = TRUE;
   oNRoot = NULL;
   ulCount = 0;
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}
/* see ft.h for specification*/
int FT_destroy(void) {
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   if(!bIsInitialized)
      return INITIALIZATION_ERROR;
   if(oNRoot) {
      ulCount -= Node_free(oNRoot);
      oNRoot = NULL;
   }
   bIsInitialized = FALSE;
   assert(CheckerFT_isValid(bIsInitialized, oNRoot, ulCount));
   return SUCCESS;
}
/* see ft.h for specification*/
void *FT_getFileContents(const char *pcPath) {
    int iStatus;
    Node_T oNFound = NULL;
    assert(pcPath != NULL);
    
    if(!FT_containsFile(pcPath)) return NULL;
    iStatus = FT_findNode(pcPath, &oNFound);
    if(iStatus != SUCCESS) return NULL;
    return Node_getFileContents(oNFound);
}
/* see ft.h for specification*/
void *FT_replaceFileContents(const char *pcPath, void *pvNewContents,
                             size_t ulNewLength) {
    int iStatus;
    Node_T oNFound = NULL;
    void* oldContents;
    assert(pcPath != NULL);
    if(FT_containsFile(pcPath)) {
        iStatus = FT_findNode(pcPath, &oNFound);
        if(iStatus != SUCCESS) return NULL;
        oldContents = Node_getFileContents(oNFound); 
        iStatus = Node_setFileContents(oNFound, pvNewContents);
        if(iStatus != SUCCESS) {
         return NULL;
        }
        iStatus = Node_setSizeContents(oNFound, ulNewLength);
        if(iStatus != SUCCESS) return NULL;
        return oldContents;
    }
    return NULL;    
}
/* see ft.h for specification*/
int FT_stat(const char *pcPath, boolean *pbIsFile, size_t *pulSize) {
    int iStatus;
    Node_T oNFound = NULL;
    assert(pcPath != NULL);
    assert(pbIsFile!=NULL);
    assert(pulSize!=NULL);
    
    iStatus = FT_findNode(pcPath, &oNFound);
    if (iStatus == SUCCESS) {
        if (Node_getType(oNFound)) {
            *pbIsFile = TRUE;
            *pulSize = Node_getSizeContents(oNFound);
        }
        else {
            *pbIsFile = FALSE;
        }
    }
    return iStatus;
}
/* --------------------------------------------------------------------
  The following auxiliary functions are used for generating the
  string representation of the FT.
*/
/*
  Performs a pre-order traversal of the tree rooted at n,
  inserting each payload to DynArray_T d beginning at index i.
  Returns the next unused index in d after the insertion(s).
  Ensures files are placed before directories.
*/
static size_t FT_preOrderTraversal(Node_T n, DynArray_T d, size_t i) {
   size_t c;
   assert(d != NULL);
   if(n != NULL) {
      (void) DynArray_set(d, i, n);
      i++;
      /* goes through array and orders files first*/
      for(c = 0; c < Node_getNumChildren(n); c++) {
         int iStatus;
         Node_T oNChild = NULL;
         iStatus = Node_getChild(n,c, &oNChild);
         assert(iStatus == SUCCESS);
         if(Node_getType(oNChild))
            i = FT_preOrderTraversal(oNChild, d, i);
      }
      /* then goes through array and orders directories*/
      for(c = 0; c < Node_getNumChildren(n); c++) {
         int iStatus;
         Node_T oNChild = NULL;
         iStatus = Node_getChild(n,c, &oNChild);
         assert(iStatus == SUCCESS);
         if(!Node_getType(oNChild))
            i = FT_preOrderTraversal(oNChild, d, i);
      }
   }
   return i;
}
/*
  Alternate version of strlen that uses pulAcc as an in-out parameter
  to accumulate a string length, rather than returning the length of
  oNNode's path, and also always adds one addition byte to the sum.
*/
static void FT_strlenAccumulate(Node_T oNNode, size_t *pulAcc) {
   assert(pulAcc != NULL);
   if(oNNode != NULL)
      *pulAcc += (Path_getStrLength(Node_getPath(oNNode)) + 1);
}
/*
  Alternate version of strcat that inverts the typical argument
  order, appending oNNode's path onto pcAcc, and also always adds one
  newline at the end of the concatenated string.
*/
static void FT_strcatAccumulate(Node_T oNNode, char *pcAcc) {
   assert(pcAcc != NULL);
   if(oNNode != NULL) {
      strcat(pcAcc, Path_getPathname(Node_getPath(oNNode)));
      strcat(pcAcc, "\n");
   }
}
/*--------------------------------------------------------------------*/
char *FT_toString(void) {
   DynArray_T nodes;

   size_t totalStrlen = 1;
   char *result = NULL;
   if(!bIsInitialized)
      return NULL;
   nodes = DynArray_new(ulCount);

   (void) FT_preOrderTraversal(oNRoot, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &totalStrlen);
   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      return NULL;
   }
   *result = '\0';
   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);
   DynArray_free(nodes);
   return result;
}