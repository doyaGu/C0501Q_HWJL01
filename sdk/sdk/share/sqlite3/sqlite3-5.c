/************** Begin file wherecode.c ***************************************/
/*
** 2015-06-06
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This module contains C code that generates VDBE code used to process
** the WHERE clause of SQL statements.
**
** This file was split off from where.c on 2015-06-06 in order to reduce the
** size of where.c and make it easier to edit.  This file contains the routines
** that actually generate the bulk of the WHERE loop code.  The original where.c
** file retains the code that does query planning and analysis.
*/
/* #include "sqliteInt.h" */
/************** Include whereInt.h in the middle of wherecode.c **************/
/************** Begin file whereInt.h ****************************************/
/*
** 2013-11-12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains structure and macro definitions for the query
** planner logic in "where.c".  These definitions are broken out into
** a separate source file for easier editing.
*/

/*
** Trace output macros
*/
#if defined(SQLITE_TEST) || defined(SQLITE_DEBUG)
/***/ int sqlite3WhereTrace;
#endif
#if defined(SQLITE_DEBUG) \
    && (defined(SQLITE_TEST) || defined(SQLITE_ENABLE_WHERETRACE))
# define WHERETRACE(K,X)  if(sqlite3WhereTrace&(K)) sqlite3DebugPrintf X
# define WHERETRACE_ENABLED 1
#else
# define WHERETRACE(K,X)
#endif

/* Forward references
*/
typedef struct WhereClause WhereClause;
typedef struct WhereMaskSet WhereMaskSet;
typedef struct WhereOrInfo WhereOrInfo;
typedef struct WhereAndInfo WhereAndInfo;
typedef struct WhereLevel WhereLevel;
typedef struct WhereLoop WhereLoop;
typedef struct WherePath WherePath;
typedef struct WhereTerm WhereTerm;
typedef struct WhereLoopBuilder WhereLoopBuilder;
typedef struct WhereScan WhereScan;
typedef struct WhereOrCost WhereOrCost;
typedef struct WhereOrSet WhereOrSet;

/*
** This object contains information needed to implement a single nested
** loop in WHERE clause.
**
** Contrast this object with WhereLoop.  This object describes the
** implementation of the loop.  WhereLoop describes the algorithm.
** This object contains a pointer to the WhereLoop algorithm as one of
** its elements.
**
** The WhereInfo object contains a single instance of this object for
** each term in the FROM clause (which is to say, for each of the
** nested loops as implemented).  The order of WhereLevel objects determines
** the loop nested order, with WhereInfo.a[0] being the outer loop and
** WhereInfo.a[WhereInfo.nLevel-1] being the inner loop.
*/
struct WhereLevel {
  int iLeftJoin;        /* Memory cell used to implement LEFT OUTER JOIN */
  int iTabCur;          /* The VDBE cursor used to access the table */
  int iIdxCur;          /* The VDBE cursor used to access pIdx */
  int addrBrk;          /* Jump here to break out of the loop */
  int addrNxt;          /* Jump here to start the next IN combination */
  int addrSkip;         /* Jump here for next iteration of skip-scan */
  int addrCont;         /* Jump here to continue with the next loop cycle */
  int addrFirst;        /* First instruction of interior of the loop */
  int addrBody;         /* Beginning of the body of this loop */
  int iLikeRepCntr;     /* LIKE range processing counter register */
  int addrLikeRep;      /* LIKE range processing address */
  u8 iFrom;             /* Which entry in the FROM clause */
  u8 op, p3, p5;        /* Opcode, P3 & P5 of the opcode that ends the loop */
  int p1, p2;           /* Operands of the opcode used to ends the loop */
  union {               /* Information that depends on pWLoop->wsFlags */
    struct {
      int nIn;              /* Number of entries in aInLoop[] */
      struct InLoop {
        int iCur;              /* The VDBE cursor used by this IN operator */
        int addrInTop;         /* Top of the IN loop */
        u8 eEndLoopOp;         /* IN Loop terminator. OP_Next or OP_Prev */
      } *aInLoop;           /* Information about each nested IN operator */
    } in;                 /* Used when pWLoop->wsFlags&WHERE_IN_ABLE */
    Index *pCovidx;       /* Possible covering index for WHERE_MULTI_OR */
  } u;
  struct WhereLoop *pWLoop;  /* The selected WhereLoop object */
  Bitmask notReady;          /* FROM entries not usable at this level */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  int addrVisit;        /* Address at which row is visited */
#endif
};

/*
** Each instance of this object represents an algorithm for evaluating one
** term of a join.  Every term of the FROM clause will have at least
** one corresponding WhereLoop object (unless INDEXED BY constraints
** prevent a query solution - which is an error) and many terms of the
** FROM clause will have multiple WhereLoop objects, each describing a
** potential way of implementing that FROM-clause term, together with
** dependencies and cost estimates for using the chosen algorithm.
**
** Query planning consists of building up a collection of these WhereLoop
** objects, then computing a particular sequence of WhereLoop objects, with
** one WhereLoop object per FROM clause term, that satisfy all dependencies
** and that minimize the overall cost.
*/
struct WhereLoop {
  Bitmask prereq;       /* Bitmask of other loops that must run first */
  Bitmask maskSelf;     /* Bitmask identifying table iTab */
#ifdef SQLITE_DEBUG
  char cId;             /* Symbolic ID of this loop for debugging use */
#endif
  u8 iTab;              /* Position in FROM clause of table for this loop */
  u8 iSortIdx;          /* Sorting index number.  0==None */
  LogEst rSetup;        /* One-time setup cost (ex: create transient index) */
  LogEst rRun;          /* Cost of running each loop */
  LogEst nOut;          /* Estimated number of output rows */
  union {
    struct {               /* Information for internal btree tables */
      u16 nEq;               /* Number of equality constraints */
      Index *pIndex;         /* Index used, or NULL */
    } btree;
    struct {               /* Information for virtual tables */
      int idxNum;            /* Index number */
      u8 needFree;           /* True if sqlite3_free(idxStr) is needed */
      i8 isOrdered;          /* True if satisfies ORDER BY */
      u16 omitMask;          /* Terms that may be omitted */
      char *idxStr;          /* Index identifier string */
    } vtab;
  } u;
  u32 wsFlags;          /* WHERE_* flags describing the plan */
  u16 nLTerm;           /* Number of entries in aLTerm[] */
  u16 nSkip;            /* Number of NULL aLTerm[] entries */
  /**** whereLoopXfer() copies fields above ***********************/
# define WHERE_LOOP_XFER_SZ offsetof(WhereLoop,nLSlot)
  u16 nLSlot;           /* Number of slots allocated for aLTerm[] */
  WhereTerm **aLTerm;   /* WhereTerms used */
  WhereLoop *pNextLoop; /* Next WhereLoop object in the WhereClause */
  WhereTerm *aLTermSpace[3];  /* Initial aLTerm[] space */
};

/* This object holds the prerequisites and the cost of running a
** subquery on one operand of an OR operator in the WHERE clause.
** See WhereOrSet for additional information 
*/
struct WhereOrCost {
  Bitmask prereq;     /* Prerequisites */
  LogEst rRun;        /* Cost of running this subquery */
  LogEst nOut;        /* Number of outputs for this subquery */
};

/* The WhereOrSet object holds a set of possible WhereOrCosts that
** correspond to the subquery(s) of OR-clause processing.  Only the
** best N_OR_COST elements are retained.
*/
#define N_OR_COST 3
struct WhereOrSet {
  u16 n;                      /* Number of valid a[] entries */
  WhereOrCost a[N_OR_COST];   /* Set of best costs */
};

/*
** Each instance of this object holds a sequence of WhereLoop objects
** that implement some or all of a query plan.
**
** Think of each WhereLoop object as a node in a graph with arcs
** showing dependencies and costs for travelling between nodes.  (That is
** not a completely accurate description because WhereLoop costs are a
** vector, not a scalar, and because dependencies are many-to-one, not
** one-to-one as are graph nodes.  But it is a useful visualization aid.)
** Then a WherePath object is a path through the graph that visits some
** or all of the WhereLoop objects once.
**
** The "solver" works by creating the N best WherePath objects of length
** 1.  Then using those as a basis to compute the N best WherePath objects
** of length 2.  And so forth until the length of WherePaths equals the
** number of nodes in the FROM clause.  The best (lowest cost) WherePath
** at the end is the chosen query plan.
*/
struct WherePath {
  Bitmask maskLoop;     /* Bitmask of all WhereLoop objects in this path */
  Bitmask revLoop;      /* aLoop[]s that should be reversed for ORDER BY */
  LogEst nRow;          /* Estimated number of rows generated by this path */
  LogEst rCost;         /* Total cost of this path */
  LogEst rUnsorted;     /* Total cost of this path ignoring sorting costs */
  i8 isOrdered;         /* No. of ORDER BY terms satisfied. -1 for unknown */
  WhereLoop **aLoop;    /* Array of WhereLoop objects implementing this path */
};

/*
** The query generator uses an array of instances of this structure to
** help it analyze the subexpressions of the WHERE clause.  Each WHERE
** clause subexpression is separated from the others by AND operators,
** usually, or sometimes subexpressions separated by OR.
**
** All WhereTerms are collected into a single WhereClause structure.  
** The following identity holds:
**
**        WhereTerm.pWC->a[WhereTerm.idx] == WhereTerm
**
** When a term is of the form:
**
**              X <op> <expr>
**
** where X is a column name and <op> is one of certain operators,
** then WhereTerm.leftCursor and WhereTerm.u.leftColumn record the
** cursor number and column number for X.  WhereTerm.eOperator records
** the <op> using a bitmask encoding defined by WO_xxx below.  The
** use of a bitmask encoding for the operator allows us to search
** quickly for terms that match any of several different operators.
**
** A WhereTerm might also be two or more subterms connected by OR:
**
**         (t1.X <op> <expr>) OR (t1.Y <op> <expr>) OR ....
**
** In this second case, wtFlag has the TERM_ORINFO bit set and eOperator==WO_OR
** and the WhereTerm.u.pOrInfo field points to auxiliary information that
** is collected about the OR clause.
**
** If a term in the WHERE clause does not match either of the two previous
** categories, then eOperator==0.  The WhereTerm.pExpr field is still set
** to the original subexpression content and wtFlags is set up appropriately
** but no other fields in the WhereTerm object are meaningful.
**
** When eOperator!=0, prereqRight and prereqAll record sets of cursor numbers,
** but they do so indirectly.  A single WhereMaskSet structure translates
** cursor number into bits and the translated bit is stored in the prereq
** fields.  The translation is used in order to maximize the number of
** bits that will fit in a Bitmask.  The VDBE cursor numbers might be
** spread out over the non-negative integers.  For example, the cursor
** numbers might be 3, 8, 9, 10, 20, 23, 41, and 45.  The WhereMaskSet
** translates these sparse cursor numbers into consecutive integers
** beginning with 0 in order to make the best possible use of the available
** bits in the Bitmask.  So, in the example above, the cursor numbers
** would be mapped into integers 0 through 7.
**
** The number of terms in a join is limited by the number of bits
** in prereqRight and prereqAll.  The default is 64 bits, hence SQLite
** is only able to process joins with 64 or fewer tables.
*/
struct WhereTerm {
  Expr *pExpr;            /* Pointer to the subexpression that is this term */
  int iParent;            /* Disable pWC->a[iParent] when this term disabled */
  int leftCursor;         /* Cursor number of X in "X <op> <expr>" */
  union {
    int leftColumn;         /* Column number of X in "X <op> <expr>" */
    WhereOrInfo *pOrInfo;   /* Extra information if (eOperator & WO_OR)!=0 */
    WhereAndInfo *pAndInfo; /* Extra information if (eOperator& WO_AND)!=0 */
  } u;
  LogEst truthProb;       /* Probability of truth for this expression */
  u16 eOperator;          /* A WO_xx value describing <op> */
  u16 wtFlags;            /* TERM_xxx bit flags.  See below */
  u8 nChild;              /* Number of children that must disable us */
  WhereClause *pWC;       /* The clause this term is part of */
  Bitmask prereqRight;    /* Bitmask of tables used by pExpr->pRight */
  Bitmask prereqAll;      /* Bitmask of tables referenced by pExpr */
};

/*
** Allowed values of WhereTerm.wtFlags
*/
#define TERM_DYNAMIC    0x01   /* Need to call sqlite3ExprDelete(db, pExpr) */
#define TERM_VIRTUAL    0x02   /* Added by the optimizer.  Do not code */
#define TERM_CODED      0x04   /* This term is already coded */
#define TERM_COPIED     0x08   /* Has a child */
#define TERM_ORINFO     0x10   /* Need to free the WhereTerm.u.pOrInfo object */
#define TERM_ANDINFO    0x20   /* Need to free the WhereTerm.u.pAndInfo obj */
#define TERM_OR_OK      0x40   /* Used during OR-clause processing */
#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
#  define TERM_VNULL    0x80   /* Manufactured x>NULL or x<=NULL term */
#else
#  define TERM_VNULL    0x00   /* Disabled if not using stat3 */
#endif
#define TERM_LIKEOPT    0x100  /* Virtual terms from the LIKE optimization */
#define TERM_LIKECOND   0x200  /* Conditionally this LIKE operator term */
#define TERM_LIKE       0x400  /* The original LIKE operator */
#define TERM_IS         0x800  /* Term.pExpr is an IS operator */

/*
** An instance of the WhereScan object is used as an iterator for locating
** terms in the WHERE clause that are useful to the query planner.
*/
struct WhereScan {
  WhereClause *pOrigWC;      /* Original, innermost WhereClause */
  WhereClause *pWC;          /* WhereClause currently being scanned */
  char *zCollName;           /* Required collating sequence, if not NULL */
  char idxaff;               /* Must match this affinity, if zCollName!=NULL */
  unsigned char nEquiv;      /* Number of entries in aEquiv[] */
  unsigned char iEquiv;      /* Next unused slot in aEquiv[] */
  u32 opMask;                /* Acceptable operators */
  int k;                     /* Resume scanning at this->pWC->a[this->k] */
  int aEquiv[22];            /* Cursor,Column pairs for equivalence classes */
};

/*
** An instance of the following structure holds all information about a
** WHERE clause.  Mostly this is a container for one or more WhereTerms.
**
** Explanation of pOuter:  For a WHERE clause of the form
**
**           a AND ((b AND c) OR (d AND e)) AND f
**
** There are separate WhereClause objects for the whole clause and for
** the subclauses "(b AND c)" and "(d AND e)".  The pOuter field of the
** subclauses points to the WhereClause object for the whole clause.
*/
struct WhereClause {
  WhereInfo *pWInfo;       /* WHERE clause processing context */
  WhereClause *pOuter;     /* Outer conjunction */
  u8 op;                   /* Split operator.  TK_AND or TK_OR */
  int nTerm;               /* Number of terms */
  int nSlot;               /* Number of entries in a[] */
  WhereTerm *a;            /* Each a[] describes a term of the WHERE cluase */
#if defined(SQLITE_SMALL_STACK)
  WhereTerm aStatic[1];    /* Initial static space for a[] */
#else
  WhereTerm aStatic[8];    /* Initial static space for a[] */
#endif
};

/*
** A WhereTerm with eOperator==WO_OR has its u.pOrInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
struct WhereOrInfo {
  WhereClause wc;          /* Decomposition into subterms */
  Bitmask indexable;       /* Bitmask of all indexable tables in the clause */
};

/*
** A WhereTerm with eOperator==WO_AND has its u.pAndInfo pointer set to
** a dynamically allocated instance of the following structure.
*/
struct WhereAndInfo {
  WhereClause wc;          /* The subexpression broken out */
};

/*
** An instance of the following structure keeps track of a mapping
** between VDBE cursor numbers and bits of the bitmasks in WhereTerm.
**
** The VDBE cursor numbers are small integers contained in 
** SrcList_item.iCursor and Expr.iTable fields.  For any given WHERE 
** clause, the cursor numbers might not begin with 0 and they might
** contain gaps in the numbering sequence.  But we want to make maximum
** use of the bits in our bitmasks.  This structure provides a mapping
** from the sparse cursor numbers into consecutive integers beginning
** with 0.
**
** If WhereMaskSet.ix[A]==B it means that The A-th bit of a Bitmask
** corresponds VDBE cursor number B.  The A-th bit of a bitmask is 1<<A.
**
** For example, if the WHERE clause expression used these VDBE
** cursors:  4, 5, 8, 29, 57, 73.  Then the  WhereMaskSet structure
** would map those cursor numbers into bits 0 through 5.
**
** Note that the mapping is not necessarily ordered.  In the example
** above, the mapping might go like this:  4->3, 5->1, 8->2, 29->0,
** 57->5, 73->4.  Or one of 719 other combinations might be used. It
** does not really matter.  What is important is that sparse cursor
** numbers all get mapped into bit numbers that begin with 0 and contain
** no gaps.
*/
struct WhereMaskSet {
  int n;                        /* Number of assigned cursor values */
  int ix[BMS];                  /* Cursor assigned to each bit */
};

/*
** Initialize a WhereMaskSet object
*/
#define initMaskSet(P)  (P)->n=0

/*
** This object is a convenience wrapper holding all information needed
** to construct WhereLoop objects for a particular query.
*/
struct WhereLoopBuilder {
  WhereInfo *pWInfo;        /* Information about this WHERE */
  WhereClause *pWC;         /* WHERE clause terms */
  ExprList *pOrderBy;       /* ORDER BY clause */
  WhereLoop *pNew;          /* Template WhereLoop */
  WhereOrSet *pOrSet;       /* Record best loops here, if not NULL */
#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
  UnpackedRecord *pRec;     /* Probe for stat4 (if required) */
  int nRecValid;            /* Number of valid fields currently in pRec */
#endif
};

/*
** The WHERE clause processing routine has two halves.  The
** first part does the start of the WHERE loop and the second
** half does the tail of the WHERE loop.  An instance of
** this structure is returned by the first half and passed
** into the second half to give some continuity.
**
** An instance of this object holds the complete state of the query
** planner.
*/
struct WhereInfo {
  Parse *pParse;            /* Parsing and code generating context */
  SrcList *pTabList;        /* List of tables in the join */
  ExprList *pOrderBy;       /* The ORDER BY clause or NULL */
  ExprList *pResultSet;     /* Result set. DISTINCT operates on these */
  WhereLoop *pLoops;        /* List of all WhereLoop objects */
  Bitmask revMask;          /* Mask of ORDER BY terms that need reversing */
  LogEst nRowOut;           /* Estimated number of output rows */
  u16 wctrlFlags;           /* Flags originally passed to sqlite3WhereBegin() */
  i8 nOBSat;                /* Number of ORDER BY terms satisfied by indices */
  u8 sorted;                /* True if really sorted (not just grouped) */
  u8 okOnePass;             /* Ok to use one-pass algorithm for UPDATE/DELETE */
  u8 untestedTerms;         /* Not all WHERE terms resolved by outer loop */
  u8 eDistinct;             /* One of the WHERE_DISTINCT_* values below */
  u8 nLevel;                /* Number of nested loop */
  int iTop;                 /* The very beginning of the WHERE loop */
  int iContinue;            /* Jump here to continue with next record */
  int iBreak;               /* Jump here to break out of the loop */
  int savedNQueryLoop;      /* pParse->nQueryLoop outside the WHERE loop */
  int aiCurOnePass[2];      /* OP_OpenWrite cursors for the ONEPASS opt */
  WhereMaskSet sMaskSet;    /* Map cursor numbers to bitmasks */
  WhereClause sWC;          /* Decomposition of the WHERE clause */
  WhereLevel a[1];          /* Information about each nest loop in WHERE */
};

/*
** Private interfaces - callable only by other where.c routines.
**
** where.c:
*/
SQLITE_PRIVATE Bitmask sqlite3WhereGetMask(WhereMaskSet*,int);
SQLITE_PRIVATE WhereTerm *sqlite3WhereFindTerm(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  int iCur,             /* Cursor number of LHS */
  int iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  Index *pIdx           /* Must be compatible with this index, if not NULL */
);

/* wherecode.c: */
#ifndef SQLITE_OMIT_EXPLAIN
SQLITE_PRIVATE int sqlite3WhereExplainOneScan(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  int iLevel,                     /* Value for "level" column of output */
  int iFrom,                      /* Value for "from" column of output */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
);
#else
# define sqlite3WhereExplainOneScan(u,v,w,x,y,z) 0
#endif /* SQLITE_OMIT_EXPLAIN */
#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
SQLITE_PRIVATE void sqlite3WhereAddScanStatus(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  int addrExplain                 /* Address of OP_Explain (or 0) */
);
#else
# define sqlite3WhereAddScanStatus(a, b, c, d) ((void)d)
#endif
SQLITE_PRIVATE Bitmask sqlite3WhereCodeOneLoopStart(
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  int iLevel,          /* Which level of pWInfo->a[] should be coded */
  Bitmask notReady     /* Which tables are currently available */
);

/* whereexpr.c: */
SQLITE_PRIVATE void sqlite3WhereClauseInit(WhereClause*,WhereInfo*);
SQLITE_PRIVATE void sqlite3WhereClauseClear(WhereClause*);
SQLITE_PRIVATE void sqlite3WhereSplit(WhereClause*,Expr*,u8);
SQLITE_PRIVATE Bitmask sqlite3WhereExprUsage(WhereMaskSet*, Expr*);
SQLITE_PRIVATE Bitmask sqlite3WhereExprListUsage(WhereMaskSet*, ExprList*);
SQLITE_PRIVATE void sqlite3WhereExprAnalyze(SrcList*, WhereClause*);





/*
** Bitmasks for the operators on WhereTerm objects.  These are all
** operators that are of interest to the query planner.  An
** OR-ed combination of these values can be used when searching for
** particular WhereTerms within a WhereClause.
*/
#define WO_IN     0x0001
#define WO_EQ     0x0002
#define WO_LT     (WO_EQ<<(TK_LT-TK_EQ))
#define WO_LE     (WO_EQ<<(TK_LE-TK_EQ))
#define WO_GT     (WO_EQ<<(TK_GT-TK_EQ))
#define WO_GE     (WO_EQ<<(TK_GE-TK_EQ))
#define WO_MATCH  0x0040
#define WO_IS     0x0080
#define WO_ISNULL 0x0100
#define WO_OR     0x0200       /* Two or more OR-connected terms */
#define WO_AND    0x0400       /* Two or more AND-connected terms */
#define WO_EQUIV  0x0800       /* Of the form A==B, both columns */
#define WO_NOOP   0x1000       /* This term does not restrict search space */

#define WO_ALL    0x1fff       /* Mask of all possible WO_* values */
#define WO_SINGLE 0x01ff       /* Mask of all non-compound WO_* values */

/*
** These are definitions of bits in the WhereLoop.wsFlags field.
** The particular combination of bits in each WhereLoop help to
** determine the algorithm that WhereLoop represents.
*/
#define WHERE_COLUMN_EQ    0x00000001  /* x=EXPR */
#define WHERE_COLUMN_RANGE 0x00000002  /* x<EXPR and/or x>EXPR */
#define WHERE_COLUMN_IN    0x00000004  /* x IN (...) */
#define WHERE_COLUMN_NULL  0x00000008  /* x IS NULL */
#define WHERE_CONSTRAINT   0x0000000f  /* Any of the WHERE_COLUMN_xxx values */
#define WHERE_TOP_LIMIT    0x00000010  /* x<EXPR or x<=EXPR constraint */
#define WHERE_BTM_LIMIT    0x00000020  /* x>EXPR or x>=EXPR constraint */
#define WHERE_BOTH_LIMIT   0x00000030  /* Both x>EXPR and x<EXPR */
#define WHERE_IDX_ONLY     0x00000040  /* Use index only - omit table */
#define WHERE_IPK          0x00000100  /* x is the INTEGER PRIMARY KEY */
#define WHERE_INDEXED      0x00000200  /* WhereLoop.u.btree.pIndex is valid */
#define WHERE_VIRTUALTABLE 0x00000400  /* WhereLoop.u.vtab is valid */
#define WHERE_IN_ABLE      0x00000800  /* Able to support an IN operator */
#define WHERE_ONEROW       0x00001000  /* Selects no more than one row */
#define WHERE_MULTI_OR     0x00002000  /* OR using multiple indices */
#define WHERE_AUTO_INDEX   0x00004000  /* Uses an ephemeral index */
#define WHERE_SKIPSCAN     0x00008000  /* Uses the skip-scan algorithm */
#define WHERE_UNQ_WANTED   0x00010000  /* WHERE_ONEROW would have been helpful*/
#define WHERE_PARTIALIDX   0x00020000  /* The automatic index is partial */

/************** End of whereInt.h ********************************************/
/************** Continuing where we left off in wherecode.c ******************/

#ifndef SQLITE_OMIT_EXPLAIN
/*
** This routine is a helper for explainIndexRange() below
**
** pStr holds the text of an expression that we are building up one term
** at a time.  This routine adds a new term to the end of the expression.
** Terms are separated by AND so add the "AND" text for second and subsequent
** terms only.
*/
static void explainAppendTerm(
  StrAccum *pStr,             /* The text expression being built */
  int iTerm,                  /* Index of this term.  First is zero */
  const char *zColumn,        /* Name of the column */
  const char *zOp             /* Name of the operator */
){
  if( iTerm ) sqlite3StrAccumAppend(pStr, " AND ", 5);
  sqlite3StrAccumAppendAll(pStr, zColumn);
  sqlite3StrAccumAppend(pStr, zOp, 1);
  sqlite3StrAccumAppend(pStr, "?", 1);
}

/*
** Argument pLevel describes a strategy for scanning table pTab. This 
** function appends text to pStr that describes the subset of table
** rows scanned by the strategy in the form of an SQL expression.
**
** For example, if the query:
**
**   SELECT * FROM t1 WHERE a=1 AND b>2;
**
** is run and there is an index on (a, b), then this function returns a
** string similar to:
**
**   "a=? AND b>?"
*/
static void explainIndexRange(StrAccum *pStr, WhereLoop *pLoop, Table *pTab){
  Index *pIndex = pLoop->u.btree.pIndex;
  u16 nEq = pLoop->u.btree.nEq;
  u16 nSkip = pLoop->nSkip;
  int i, j;
  Column *aCol = pTab->aCol;
  i16 *aiColumn = pIndex->aiColumn;

  if( nEq==0 && (pLoop->wsFlags&(WHERE_BTM_LIMIT|WHERE_TOP_LIMIT))==0 ) return;
  sqlite3StrAccumAppend(pStr, " (", 2);
  for(i=0; i<nEq; i++){
    char *z = aiColumn[i] < 0 ? "rowid" : aCol[aiColumn[i]].zName;
    if( i>=nSkip ){
      explainAppendTerm(pStr, i, z, "=");
    }else{
      if( i ) sqlite3StrAccumAppend(pStr, " AND ", 5);
      sqlite3XPrintf(pStr, 0, "ANY(%s)", z);
    }
  }

  j = i;
  if( pLoop->wsFlags&WHERE_BTM_LIMIT ){
    char *z = aiColumn[j] < 0 ? "rowid" : aCol[aiColumn[j]].zName;
    explainAppendTerm(pStr, i++, z, ">");
  }
  if( pLoop->wsFlags&WHERE_TOP_LIMIT ){
    char *z = aiColumn[j] < 0 ? "rowid" : aCol[aiColumn[j]].zName;
    explainAppendTerm(pStr, i, z, "<");
  }
  sqlite3StrAccumAppend(pStr, ")", 1);
}

/*
** This function is a no-op unless currently processing an EXPLAIN QUERY PLAN
** command, or if either SQLITE_DEBUG or SQLITE_ENABLE_STMT_SCANSTATUS was
** defined at compile-time. If it is not a no-op, a single OP_Explain opcode 
** is added to the output to describe the table scan strategy in pLevel.
**
** If an OP_Explain opcode is added to the VM, its address is returned.
** Otherwise, if no OP_Explain is coded, zero is returned.
*/
SQLITE_PRIVATE int sqlite3WhereExplainOneScan(
  Parse *pParse,                  /* Parse context */
  SrcList *pTabList,              /* Table list this loop refers to */
  WhereLevel *pLevel,             /* Scan to write OP_Explain opcode for */
  int iLevel,                     /* Value for "level" column of output */
  int iFrom,                      /* Value for "from" column of output */
  u16 wctrlFlags                  /* Flags passed to sqlite3WhereBegin() */
){
  int ret = 0;
#if !defined(SQLITE_DEBUG) && !defined(SQLITE_ENABLE_STMT_SCANSTATUS)
  if( pParse->explain==2 )
#endif
  {
    struct SrcList_item *pItem = &pTabList->a[pLevel->iFrom];
    Vdbe *v = pParse->pVdbe;      /* VM being constructed */
    sqlite3 *db = pParse->db;     /* Database handle */
    int iId = pParse->iSelectId;  /* Select id (left-most output column) */
    int isSearch;                 /* True for a SEARCH. False for SCAN. */
    WhereLoop *pLoop;             /* The controlling WhereLoop object */
    u32 flags;                    /* Flags that describe this loop */
    char *zMsg;                   /* Text to add to EQP output */
    StrAccum str;                 /* EQP output string */
    char zBuf[100];               /* Initial space for EQP output string */

    pLoop = pLevel->pWLoop;
    flags = pLoop->wsFlags;
    if( (flags&WHERE_MULTI_OR) || (wctrlFlags&WHERE_ONETABLE_ONLY) ) return 0;

    isSearch = (flags&(WHERE_BTM_LIMIT|WHERE_TOP_LIMIT))!=0
            || ((flags&WHERE_VIRTUALTABLE)==0 && (pLoop->u.btree.nEq>0))
            || (wctrlFlags&(WHERE_ORDERBY_MIN|WHERE_ORDERBY_MAX));

    sqlite3StrAccumInit(&str, db, zBuf, sizeof(zBuf), SQLITE_MAX_LENGTH);
    sqlite3StrAccumAppendAll(&str, isSearch ? "SEARCH" : "SCAN");
    if( pItem->pSelect ){
      sqlite3XPrintf(&str, 0, " SUBQUERY %d", pItem->iSelectId);
    }else{
      sqlite3XPrintf(&str, 0, " TABLE %s", pItem->zName);
    }

    if( pItem->zAlias ){
      sqlite3XPrintf(&str, 0, " AS %s", pItem->zAlias);
    }
    if( (flags & (WHERE_IPK|WHERE_VIRTUALTABLE))==0 ){
      const char *zFmt = 0;
      Index *pIdx;

      assert( pLoop->u.btree.pIndex!=0 );
      pIdx = pLoop->u.btree.pIndex;
      assert( !(flags&WHERE_AUTO_INDEX) || (flags&WHERE_IDX_ONLY) );
      if( !HasRowid(pItem->pTab) && IsPrimaryKeyIndex(pIdx) ){
        if( isSearch ){
          zFmt = "PRIMARY KEY";
        }
      }else if( flags & WHERE_PARTIALIDX ){
        zFmt = "AUTOMATIC PARTIAL COVERING INDEX";
      }else if( flags & WHERE_AUTO_INDEX ){
        zFmt = "AUTOMATIC COVERING INDEX";
      }else if( flags & WHERE_IDX_ONLY ){
        zFmt = "COVERING INDEX %s";
      }else{
        zFmt = "INDEX %s";
      }
      if( zFmt ){
        sqlite3StrAccumAppend(&str, " USING ", 7);
        sqlite3XPrintf(&str, 0, zFmt, pIdx->zName);
        explainIndexRange(&str, pLoop, pItem->pTab);
      }
    }else if( (flags & WHERE_IPK)!=0 && (flags & WHERE_CONSTRAINT)!=0 ){
      const char *zRange;
      if( flags&(WHERE_COLUMN_EQ|WHERE_COLUMN_IN) ){
        zRange = "(rowid=?)";
      }else if( (flags&WHERE_BOTH_LIMIT)==WHERE_BOTH_LIMIT ){
        zRange = "(rowid>? AND rowid<?)";
      }else if( flags&WHERE_BTM_LIMIT ){
        zRange = "(rowid>?)";
      }else{
        assert( flags&WHERE_TOP_LIMIT);
        zRange = "(rowid<?)";
      }
      sqlite3StrAccumAppendAll(&str, " USING INTEGER PRIMARY KEY ");
      sqlite3StrAccumAppendAll(&str, zRange);
    }
#ifndef SQLITE_OMIT_VIRTUALTABLE
    else if( (flags & WHERE_VIRTUALTABLE)!=0 ){
      sqlite3XPrintf(&str, 0, " VIRTUAL TABLE INDEX %d:%s",
                  pLoop->u.vtab.idxNum, pLoop->u.vtab.idxStr);
    }
#endif
#ifdef SQLITE_EXPLAIN_ESTIMATED_ROWS
    if( pLoop->nOut>=10 ){
      sqlite3XPrintf(&str, 0, " (~%llu rows)", sqlite3LogEstToInt(pLoop->nOut));
    }else{
      sqlite3StrAccumAppend(&str, " (~1 row)", 9);
    }
#endif
    zMsg = sqlite3StrAccumFinish(&str);
    ret = sqlite3VdbeAddOp4(v, OP_Explain, iId, iLevel, iFrom, zMsg,P4_DYNAMIC);
  }
  return ret;
}
#endif /* SQLITE_OMIT_EXPLAIN */

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
/*
** Configure the VM passed as the first argument with an
** sqlite3_stmt_scanstatus() entry corresponding to the scan used to 
** implement level pLvl. Argument pSrclist is a pointer to the FROM 
** clause that the scan reads data from.
**
** If argument addrExplain is not 0, it must be the address of an 
** OP_Explain instruction that describes the same loop.
*/
SQLITE_PRIVATE void sqlite3WhereAddScanStatus(
  Vdbe *v,                        /* Vdbe to add scanstatus entry to */
  SrcList *pSrclist,              /* FROM clause pLvl reads data from */
  WhereLevel *pLvl,               /* Level to add scanstatus() entry for */
  int addrExplain                 /* Address of OP_Explain (or 0) */
){
  const char *zObj = 0;
  WhereLoop *pLoop = pLvl->pWLoop;
  if( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0  &&  pLoop->u.btree.pIndex!=0 ){
    zObj = pLoop->u.btree.pIndex->zName;
  }else{
    zObj = pSrclist->a[pLvl->iFrom].zName;
  }
  sqlite3VdbeScanStatus(
      v, addrExplain, pLvl->addrBody, pLvl->addrVisit, pLoop->nOut, zObj
  );
}
#endif


/*
** Disable a term in the WHERE clause.  Except, do not disable the term
** if it controls a LEFT OUTER JOIN and it did not originate in the ON
** or USING clause of that join.
**
** Consider the term t2.z='ok' in the following queries:
**
**   (1)  SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.x WHERE t2.z='ok'
**   (2)  SELECT * FROM t1 LEFT JOIN t2 ON t1.a=t2.x AND t2.z='ok'
**   (3)  SELECT * FROM t1, t2 WHERE t1.a=t2.x AND t2.z='ok'
**
** The t2.z='ok' is disabled in the in (2) because it originates
** in the ON clause.  The term is disabled in (3) because it is not part
** of a LEFT OUTER JOIN.  In (1), the term is not disabled.
**
** Disabling a term causes that term to not be tested in the inner loop
** of the join.  Disabling is an optimization.  When terms are satisfied
** by indices, we disable them to prevent redundant tests in the inner
** loop.  We would get the correct results if nothing were ever disabled,
** but joins might run a little slower.  The trick is to disable as much
** as we can without disabling too much.  If we disabled in (1), we'd get
** the wrong answer.  See ticket #813.
**
** If all the children of a term are disabled, then that term is also
** automatically disabled.  In this way, terms get disabled if derived
** virtual terms are tested first.  For example:
**
**      x GLOB 'abc*' AND x>='abc' AND x<'acd'
**      \___________/     \______/     \_____/
**         parent          child1       child2
**
** Only the parent term was in the original WHERE clause.  The child1
** and child2 terms were added by the LIKE optimization.  If both of
** the virtual child terms are valid, then testing of the parent can be 
** skipped.
**
** Usually the parent term is marked as TERM_CODED.  But if the parent
** term was originally TERM_LIKE, then the parent gets TERM_LIKECOND instead.
** The TERM_LIKECOND marking indicates that the term should be coded inside
** a conditional such that is only evaluated on the second pass of a
** LIKE-optimization loop, when scanning BLOBs instead of strings.
*/
static void disableTerm(WhereLevel *pLevel, WhereTerm *pTerm){
  int nLoop = 0;
  while( pTerm
      && (pTerm->wtFlags & TERM_CODED)==0
      && (pLevel->iLeftJoin==0 || ExprHasProperty(pTerm->pExpr, EP_FromJoin))
      && (pLevel->notReady & pTerm->prereqAll)==0
  ){
    if( nLoop && (pTerm->wtFlags & TERM_LIKE)!=0 ){
      pTerm->wtFlags |= TERM_LIKECOND;
    }else{
      pTerm->wtFlags |= TERM_CODED;
    }
    if( pTerm->iParent<0 ) break;
    pTerm = &pTerm->pWC->a[pTerm->iParent];
    pTerm->nChild--;
    if( pTerm->nChild!=0 ) break;
    nLoop++;
  }
}

/*
** Code an OP_Affinity opcode to apply the column affinity string zAff
** to the n registers starting at base. 
**
** As an optimization, SQLITE_AFF_BLOB entries (which are no-ops) at the
** beginning and end of zAff are ignored.  If all entries in zAff are
** SQLITE_AFF_BLOB, then no code gets generated.
**
** This routine makes its own copy of zAff so that the caller is free
** to modify zAff after this routine returns.
*/
static void codeApplyAffinity(Parse *pParse, int base, int n, char *zAff){
  Vdbe *v = pParse->pVdbe;
  if( zAff==0 ){
    assert( pParse->db->mallocFailed );
    return;
  }
  assert( v!=0 );

  /* Adjust base and n to skip over SQLITE_AFF_BLOB entries at the beginning
  ** and end of the affinity string.
  */
  while( n>0 && zAff[0]==SQLITE_AFF_BLOB ){
    n--;
    base++;
    zAff++;
  }
  while( n>1 && zAff[n-1]==SQLITE_AFF_BLOB ){
    n--;
  }

  /* Code the OP_Affinity opcode if there is anything left to do. */
  if( n>0 ){
    sqlite3VdbeAddOp2(v, OP_Affinity, base, n);
    sqlite3VdbeChangeP4(v, -1, zAff, n);
    sqlite3ExprCacheAffinityChange(pParse, base, n);
  }
}


/*
** Generate code for a single equality term of the WHERE clause.  An equality
** term can be either X=expr or X IN (...).   pTerm is the term to be 
** coded.
**
** The current value for the constraint is left in register iReg.
**
** For a constraint of the form X=expr, the expression is evaluated and its
** result is left on the stack.  For constraints of the form X IN (...)
** this routine sets up a loop that will iterate over all values of X.
*/
static int codeEqualityTerm(
  Parse *pParse,      /* The parsing context */
  WhereTerm *pTerm,   /* The term of the WHERE clause to be coded */
  WhereLevel *pLevel, /* The level of the FROM clause we are working on */
  int iEq,            /* Index of the equality term within this level */
  int bRev,           /* True for reverse-order IN operations */
  int iTarget         /* Attempt to leave results in this register */
){
  Expr *pX = pTerm->pExpr;
  Vdbe *v = pParse->pVdbe;
  int iReg;                  /* Register holding results */

  assert( iTarget>0 );
  if( pX->op==TK_EQ || pX->op==TK_IS ){
    iReg = sqlite3ExprCodeTarget(pParse, pX->pRight, iTarget);
  }else if( pX->op==TK_ISNULL ){
    iReg = iTarget;
    sqlite3VdbeAddOp2(v, OP_Null, 0, iReg);
#ifndef SQLITE_OMIT_SUBQUERY
  }else{
    int eType;
    int iTab;
    struct InLoop *pIn;
    WhereLoop *pLoop = pLevel->pWLoop;

    if( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0
      && pLoop->u.btree.pIndex!=0
      && pLoop->u.btree.pIndex->aSortOrder[iEq]
    ){
      testcase( iEq==0 );
      testcase( bRev );
      bRev = !bRev;
    }
    assert( pX->op==TK_IN );
    iReg = iTarget;
    eType = sqlite3FindInIndex(pParse, pX, IN_INDEX_LOOP, 0);
    if( eType==IN_INDEX_INDEX_DESC ){
      testcase( bRev );
      bRev = !bRev;
    }
    iTab = pX->iTable;
    sqlite3VdbeAddOp2(v, bRev ? OP_Last : OP_Rewind, iTab, 0);
    VdbeCoverageIf(v, bRev);
    VdbeCoverageIf(v, !bRev);
    assert( (pLoop->wsFlags & WHERE_MULTI_OR)==0 );
    pLoop->wsFlags |= WHERE_IN_ABLE;
    if( pLevel->u.in.nIn==0 ){
      pLevel->addrNxt = sqlite3VdbeMakeLabel(v);
    }
    pLevel->u.in.nIn++;
    pLevel->u.in.aInLoop =
       sqlite3DbReallocOrFree(pParse->db, pLevel->u.in.aInLoop,
                              sizeof(pLevel->u.in.aInLoop[0])*pLevel->u.in.nIn);
    pIn = pLevel->u.in.aInLoop;
    if( pIn ){
      pIn += pLevel->u.in.nIn - 1;
      pIn->iCur = iTab;
      if( eType==IN_INDEX_ROWID ){
        pIn->addrInTop = sqlite3VdbeAddOp2(v, OP_Rowid, iTab, iReg);
      }else{
        pIn->addrInTop = sqlite3VdbeAddOp3(v, OP_Column, iTab, 0, iReg);
      }
      pIn->eEndLoopOp = bRev ? OP_PrevIfOpen : OP_NextIfOpen;
      sqlite3VdbeAddOp1(v, OP_IsNull, iReg); VdbeCoverage(v);
    }else{
      pLevel->u.in.nIn = 0;
    }
#endif
  }
  disableTerm(pLevel, pTerm);
  return iReg;
}

/*
** Generate code that will evaluate all == and IN constraints for an
** index scan.
**
** For example, consider table t1(a,b,c,d,e,f) with index i1(a,b,c).
** Suppose the WHERE clause is this:  a==5 AND b IN (1,2,3) AND c>5 AND c<10
** The index has as many as three equality constraints, but in this
** example, the third "c" value is an inequality.  So only two 
** constraints are coded.  This routine will generate code to evaluate
** a==5 and b IN (1,2,3).  The current values for a and b will be stored
** in consecutive registers and the index of the first register is returned.
**
** In the example above nEq==2.  But this subroutine works for any value
** of nEq including 0.  If nEq==0, this routine is nearly a no-op.
** The only thing it does is allocate the pLevel->iMem memory cell and
** compute the affinity string.
**
** The nExtraReg parameter is 0 or 1.  It is 0 if all WHERE clause constraints
** are == or IN and are covered by the nEq.  nExtraReg is 1 if there is
** an inequality constraint (such as the "c>=5 AND c<10" in the example) that
** occurs after the nEq quality constraints.
**
** This routine allocates a range of nEq+nExtraReg memory cells and returns
** the index of the first memory cell in that range. The code that
** calls this routine will use that memory range to store keys for
** start and termination conditions of the loop.
** key value of the loop.  If one or more IN operators appear, then
** this routine allocates an additional nEq memory cells for internal
** use.
**
** Before returning, *pzAff is set to point to a buffer containing a
** copy of the column affinity string of the index allocated using
** sqlite3DbMalloc(). Except, entries in the copy of the string associated
** with equality constraints that use BLOB or NONE affinity are set to
** SQLITE_AFF_BLOB. This is to deal with SQL such as the following:
**
**   CREATE TABLE t1(a TEXT PRIMARY KEY, b);
**   SELECT ... FROM t1 AS t2, t1 WHERE t1.a = t2.b;
**
** In the example above, the index on t1(a) has TEXT affinity. But since
** the right hand side of the equality constraint (t2.b) has BLOB/NONE affinity,
** no conversion should be attempted before using a t2.b value as part of
** a key to search the index. Hence the first byte in the returned affinity
** string in this example would be set to SQLITE_AFF_BLOB.
*/
static int codeAllEqualityTerms(
  Parse *pParse,        /* Parsing context */
  WhereLevel *pLevel,   /* Which nested loop of the FROM we are coding */
  int bRev,             /* Reverse the order of IN operators */
  int nExtraReg,        /* Number of extra registers to allocate */
  char **pzAff          /* OUT: Set to point to affinity string */
){
  u16 nEq;                      /* The number of == or IN constraints to code */
  u16 nSkip;                    /* Number of left-most columns to skip */
  Vdbe *v = pParse->pVdbe;      /* The vm under construction */
  Index *pIdx;                  /* The index being used for this loop */
  WhereTerm *pTerm;             /* A single constraint term */
  WhereLoop *pLoop;             /* The WhereLoop object */
  int j;                        /* Loop counter */
  int regBase;                  /* Base register */
  int nReg;                     /* Number of registers to allocate */
  char *zAff;                   /* Affinity string to return */

  /* This module is only called on query plans that use an index. */
  pLoop = pLevel->pWLoop;
  assert( (pLoop->wsFlags & WHERE_VIRTUALTABLE)==0 );
  nEq = pLoop->u.btree.nEq;
  nSkip = pLoop->nSkip;
  pIdx = pLoop->u.btree.pIndex;
  assert( pIdx!=0 );

  /* Figure out how many memory cells we will need then allocate them.
  */
  regBase = pParse->nMem + 1;
  nReg = pLoop->u.btree.nEq + nExtraReg;
  pParse->nMem += nReg;

  zAff = sqlite3DbStrDup(pParse->db, sqlite3IndexAffinityStr(v, pIdx));
  if( !zAff ){
    pParse->db->mallocFailed = 1;
  }

  if( nSkip ){
    int iIdxCur = pLevel->iIdxCur;
    sqlite3VdbeAddOp1(v, (bRev?OP_Last:OP_Rewind), iIdxCur);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    VdbeComment((v, "begin skip-scan on %s", pIdx->zName));
    j = sqlite3VdbeAddOp0(v, OP_Goto);
    pLevel->addrSkip = sqlite3VdbeAddOp4Int(v, (bRev?OP_SeekLT:OP_SeekGT),
                            iIdxCur, 0, regBase, nSkip);
    VdbeCoverageIf(v, bRev==0);
    VdbeCoverageIf(v, bRev!=0);
    sqlite3VdbeJumpHere(v, j);
    for(j=0; j<nSkip; j++){
      sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, j, regBase+j);
      assert( pIdx->aiColumn[j]>=0 );
      VdbeComment((v, "%s", pIdx->pTable->aCol[pIdx->aiColumn[j]].zName));
    }
  }    

  /* Evaluate the equality constraints
  */
  assert( zAff==0 || (int)strlen(zAff)>=nEq );
  for(j=nSkip; j<nEq; j++){
    int r1;
    pTerm = pLoop->aLTerm[j];
    assert( pTerm!=0 );
    /* The following testcase is true for indices with redundant columns. 
    ** Ex: CREATE INDEX i1 ON t1(a,b,a); SELECT * FROM t1 WHERE a=0 AND b=0; */
    testcase( (pTerm->wtFlags & TERM_CODED)!=0 );
    testcase( pTerm->wtFlags & TERM_VIRTUAL );
    r1 = codeEqualityTerm(pParse, pTerm, pLevel, j, bRev, regBase+j);
    if( r1!=regBase+j ){
      if( nReg==1 ){
        sqlite3ReleaseTempReg(pParse, regBase);
        regBase = r1;
      }else{
        sqlite3VdbeAddOp2(v, OP_SCopy, r1, regBase+j);
      }
    }
    testcase( pTerm->eOperator & WO_ISNULL );
    testcase( pTerm->eOperator & WO_IN );
    if( (pTerm->eOperator & (WO_ISNULL|WO_IN))==0 ){
      Expr *pRight = pTerm->pExpr->pRight;
      if( (pTerm->wtFlags & TERM_IS)==0 && sqlite3ExprCanBeNull(pRight) ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+j, pLevel->addrBrk);
        VdbeCoverage(v);
      }
      if( zAff ){
        if( sqlite3CompareAffinity(pRight, zAff[j])==SQLITE_AFF_BLOB ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
        if( sqlite3ExprNeedsNoAffinityChange(pRight, zAff[j]) ){
          zAff[j] = SQLITE_AFF_BLOB;
        }
      }
    }
  }
  *pzAff = zAff;
  return regBase;
}

/*
** If the most recently coded instruction is a constant range contraint
** that originated from the LIKE optimization, then change the P3 to be
** pLoop->iLikeRepCntr and set P5.
**
** The LIKE optimization trys to evaluate "x LIKE 'abc%'" as a range
** expression: "x>='ABC' AND x<'abd'".  But this requires that the range
** scan loop run twice, once for strings and a second time for BLOBs.
** The OP_String opcodes on the second pass convert the upper and lower
** bound string contants to blobs.  This routine makes the necessary changes
** to the OP_String opcodes for that to happen.
*/
static void whereLikeOptimizationStringFixup(
  Vdbe *v,                /* prepared statement under construction */
  WhereLevel *pLevel,     /* The loop that contains the LIKE operator */
  WhereTerm *pTerm        /* The upper or lower bound just coded */
){
  if( pTerm->wtFlags & TERM_LIKEOPT ){
    VdbeOp *pOp;
    assert( pLevel->iLikeRepCntr>0 );
    pOp = sqlite3VdbeGetOp(v, -1);
    assert( pOp!=0 );
    assert( pOp->opcode==OP_String8 
            || pTerm->pWC->pWInfo->pParse->db->mallocFailed );
    pOp->p3 = pLevel->iLikeRepCntr;
    pOp->p5 = 1;
  }
}


/*
** Generate code for the start of the iLevel-th loop in the WHERE clause
** implementation described by pWInfo.
*/
SQLITE_PRIVATE Bitmask sqlite3WhereCodeOneLoopStart(
  WhereInfo *pWInfo,   /* Complete information about the WHERE clause */
  int iLevel,          /* Which level of pWInfo->a[] should be coded */
  Bitmask notReady     /* Which tables are currently available */
){
  int j, k;            /* Loop counters */
  int iCur;            /* The VDBE cursor for the table */
  int addrNxt;         /* Where to jump to continue with the next IN case */
  int omitTable;       /* True if we use the index only */
  int bRev;            /* True if we need to scan in reverse order */
  WhereLevel *pLevel;  /* The where level to be coded */
  WhereLoop *pLoop;    /* The WhereLoop object being coded */
  WhereClause *pWC;    /* Decomposition of the entire WHERE clause */
  WhereTerm *pTerm;               /* A WHERE clause term */
  Parse *pParse;                  /* Parsing context */
  sqlite3 *db;                    /* Database connection */
  Vdbe *v;                        /* The prepared stmt under constructions */
  struct SrcList_item *pTabItem;  /* FROM clause term being coded */
  int addrBrk;                    /* Jump here to break out of the loop */
  int addrCont;                   /* Jump here to continue with next cycle */
  int iRowidReg = 0;        /* Rowid is stored in this register, if not zero */
  int iReleaseReg = 0;      /* Temp register to free before returning */

  pParse = pWInfo->pParse;
  v = pParse->pVdbe;
  pWC = &pWInfo->sWC;
  db = pParse->db;
  pLevel = &pWInfo->a[iLevel];
  pLoop = pLevel->pWLoop;
  pTabItem = &pWInfo->pTabList->a[pLevel->iFrom];
  iCur = pTabItem->iCursor;
  pLevel->notReady = notReady & ~sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur);
  bRev = (pWInfo->revMask>>iLevel)&1;
  omitTable = (pLoop->wsFlags & WHERE_IDX_ONLY)!=0 
           && (pWInfo->wctrlFlags & WHERE_FORCE_TABLE)==0;
  VdbeModuleComment((v, "Begin WHERE-loop%d: %s",iLevel,pTabItem->pTab->zName));

  /* Create labels for the "break" and "continue" instructions
  ** for the current loop.  Jump to addrBrk to break out of a loop.
  ** Jump to cont to go immediately to the next iteration of the
  ** loop.
  **
  ** When there is an IN operator, we also have a "addrNxt" label that
  ** means to continue with the next IN value combination.  When
  ** there are no IN operators in the constraints, the "addrNxt" label
  ** is the same as "addrBrk".
  */
  addrBrk = pLevel->addrBrk = pLevel->addrNxt = sqlite3VdbeMakeLabel(v);
  addrCont = pLevel->addrCont = sqlite3VdbeMakeLabel(v);

  /* If this is the right table of a LEFT OUTER JOIN, allocate and
  ** initialize a memory cell that records if this table matches any
  ** row of the left table of the join.
  */
  if( pLevel->iFrom>0 && (pTabItem[0].jointype & JT_LEFT)!=0 ){
    pLevel->iLeftJoin = ++pParse->nMem;
    sqlite3VdbeAddOp2(v, OP_Integer, 0, pLevel->iLeftJoin);
    VdbeComment((v, "init LEFT JOIN no-match flag"));
  }

  /* Special case of a FROM clause subquery implemented as a co-routine */
  if( pTabItem->viaCoroutine ){
    int regYield = pTabItem->regReturn;
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pTabItem->addrFillSub);
    pLevel->p2 =  sqlite3VdbeAddOp2(v, OP_Yield, regYield, addrBrk);
    VdbeCoverage(v);
    VdbeComment((v, "next row of \"%s\"", pTabItem->pTab->zName));
    pLevel->op = OP_Goto;
  }else

#ifndef SQLITE_OMIT_VIRTUALTABLE
  if(  (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
    /* Case 1:  The table is a virtual-table.  Use the VFilter and VNext
    **          to access the data.
    */
    int iReg;   /* P3 Value for OP_VFilter */
    int addrNotFound;
    int nConstraint = pLoop->nLTerm;

    sqlite3ExprCachePush(pParse);
    iReg = sqlite3GetTempRange(pParse, nConstraint+2);
    addrNotFound = pLevel->addrBrk;
    for(j=0; j<nConstraint; j++){
      int iTarget = iReg+j+2;
      pTerm = pLoop->aLTerm[j];
      if( pTerm==0 ) continue;
      if( pTerm->eOperator & WO_IN ){
        codeEqualityTerm(pParse, pTerm, pLevel, j, bRev, iTarget);
        addrNotFound = pLevel->addrNxt;
      }else{
        sqlite3ExprCode(pParse, pTerm->pExpr->pRight, iTarget);
      }
    }
    sqlite3VdbeAddOp2(v, OP_Integer, pLoop->u.vtab.idxNum, iReg);
    sqlite3VdbeAddOp2(v, OP_Integer, nConstraint, iReg+1);
    sqlite3VdbeAddOp4(v, OP_VFilter, iCur, addrNotFound, iReg,
                      pLoop->u.vtab.idxStr,
                      pLoop->u.vtab.needFree ? P4_MPRINTF : P4_STATIC);
    VdbeCoverage(v);
    pLoop->u.vtab.needFree = 0;
    for(j=0; j<nConstraint && j<16; j++){
      if( (pLoop->u.vtab.omitMask>>j)&1 ){
        disableTerm(pLevel, pLoop->aLTerm[j]);
      }
    }
    pLevel->op = OP_VNext;
    pLevel->p1 = iCur;
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);
    sqlite3ReleaseTempRange(pParse, iReg, nConstraint+2);
    sqlite3ExprCachePop(pParse);
  }else
#endif /* SQLITE_OMIT_VIRTUALTABLE */

  if( (pLoop->wsFlags & WHERE_IPK)!=0
   && (pLoop->wsFlags & (WHERE_COLUMN_IN|WHERE_COLUMN_EQ))!=0
  ){
    /* Case 2:  We can directly reference a single row using an
    **          equality comparison against the ROWID field.  Or
    **          we reference multiple rows using a "rowid IN (...)"
    **          construct.
    */
    assert( pLoop->u.btree.nEq==1 );
    pTerm = pLoop->aLTerm[0];
    assert( pTerm!=0 );
    assert( pTerm->pExpr!=0 );
    assert( omitTable==0 );
    testcase( pTerm->wtFlags & TERM_VIRTUAL );
    iReleaseReg = ++pParse->nMem;
    iRowidReg = codeEqualityTerm(pParse, pTerm, pLevel, 0, bRev, iReleaseReg);
    if( iRowidReg!=iReleaseReg ) sqlite3ReleaseTempReg(pParse, iReleaseReg);
    addrNxt = pLevel->addrNxt;
    sqlite3VdbeAddOp2(v, OP_MustBeInt, iRowidReg, addrNxt); VdbeCoverage(v);
    sqlite3VdbeAddOp3(v, OP_NotExists, iCur, addrNxt, iRowidReg);
    VdbeCoverage(v);
    sqlite3ExprCacheAffinityChange(pParse, iRowidReg, 1);
    sqlite3ExprCacheStore(pParse, iCur, -1, iRowidReg);
    VdbeComment((v, "pk"));
    pLevel->op = OP_Noop;
  }else if( (pLoop->wsFlags & WHERE_IPK)!=0
         && (pLoop->wsFlags & WHERE_COLUMN_RANGE)!=0
  ){
    /* Case 3:  We have an inequality comparison against the ROWID field.
    */
    int testOp = OP_Noop;
    int start;
    int memEndValue = 0;
    WhereTerm *pStart, *pEnd;

    assert( omitTable==0 );
    j = 0;
    pStart = pEnd = 0;
    if( pLoop->wsFlags & WHERE_BTM_LIMIT ) pStart = pLoop->aLTerm[j++];
    if( pLoop->wsFlags & WHERE_TOP_LIMIT ) pEnd = pLoop->aLTerm[j++];
    assert( pStart!=0 || pEnd!=0 );
    if( bRev ){
      pTerm = pStart;
      pStart = pEnd;
      pEnd = pTerm;
    }
    if( pStart ){
      Expr *pX;             /* The expression that defines the start bound */
      int r1, rTemp;        /* Registers for holding the start boundary */

      /* The following constant maps TK_xx codes into corresponding 
      ** seek opcodes.  It depends on a particular ordering of TK_xx
      */
      const u8 aMoveOp[] = {
           /* TK_GT */  OP_SeekGT,
           /* TK_LE */  OP_SeekLE,
           /* TK_LT */  OP_SeekLT,
           /* TK_GE */  OP_SeekGE
      };
      assert( TK_LE==TK_GT+1 );      /* Make sure the ordering.. */
      assert( TK_LT==TK_GT+2 );      /*  ... of the TK_xx values... */
      assert( TK_GE==TK_GT+3 );      /*  ... is correcct. */

      assert( (pStart->wtFlags & TERM_VNULL)==0 );
      testcase( pStart->wtFlags & TERM_VIRTUAL );
      pX = pStart->pExpr;
      assert( pX!=0 );
      testcase( pStart->leftCursor!=iCur ); /* transitive constraints */
      r1 = sqlite3ExprCodeTemp(pParse, pX->pRight, &rTemp);
      sqlite3VdbeAddOp3(v, aMoveOp[pX->op-TK_GT], iCur, addrBrk, r1);
      VdbeComment((v, "pk"));
      VdbeCoverageIf(v, pX->op==TK_GT);
      VdbeCoverageIf(v, pX->op==TK_LE);
      VdbeCoverageIf(v, pX->op==TK_LT);
      VdbeCoverageIf(v, pX->op==TK_GE);
      sqlite3ExprCacheAffinityChange(pParse, r1, 1);
      sqlite3ReleaseTempReg(pParse, rTemp);
      disableTerm(pLevel, pStart);
    }else{
      sqlite3VdbeAddOp2(v, bRev ? OP_Last : OP_Rewind, iCur, addrBrk);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
    }
    if( pEnd ){
      Expr *pX;
      pX = pEnd->pExpr;
      assert( pX!=0 );
      assert( (pEnd->wtFlags & TERM_VNULL)==0 );
      testcase( pEnd->leftCursor!=iCur ); /* Transitive constraints */
      testcase( pEnd->wtFlags & TERM_VIRTUAL );
      memEndValue = ++pParse->nMem;
      sqlite3ExprCode(pParse, pX->pRight, memEndValue);
      if( pX->op==TK_LT || pX->op==TK_GT ){
        testOp = bRev ? OP_Le : OP_Ge;
      }else{
        testOp = bRev ? OP_Lt : OP_Gt;
      }
      disableTerm(pLevel, pEnd);
    }
    start = sqlite3VdbeCurrentAddr(v);
    pLevel->op = bRev ? OP_Prev : OP_Next;
    pLevel->p1 = iCur;
    pLevel->p2 = start;
    assert( pLevel->p5==0 );
    if( testOp!=OP_Noop ){
      iRowidReg = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_Rowid, iCur, iRowidReg);
      sqlite3ExprCacheStore(pParse, iCur, -1, iRowidReg);
      sqlite3VdbeAddOp3(v, testOp, memEndValue, addrBrk, iRowidReg);
      VdbeCoverageIf(v, testOp==OP_Le);
      VdbeCoverageIf(v, testOp==OP_Lt);
      VdbeCoverageIf(v, testOp==OP_Ge);
      VdbeCoverageIf(v, testOp==OP_Gt);
      sqlite3VdbeChangeP5(v, SQLITE_AFF_NUMERIC | SQLITE_JUMPIFNULL);
    }
  }else if( pLoop->wsFlags & WHERE_INDEXED ){
    /* Case 4: A scan using an index.
    **
    **         The WHERE clause may contain zero or more equality 
    **         terms ("==" or "IN" operators) that refer to the N
    **         left-most columns of the index. It may also contain
    **         inequality constraints (>, <, >= or <=) on the indexed
    **         column that immediately follows the N equalities. Only 
    **         the right-most column can be an inequality - the rest must
    **         use the "==" and "IN" operators. For example, if the 
    **         index is on (x,y,z), then the following clauses are all 
    **         optimized:
    **
    **            x=5
    **            x=5 AND y=10
    **            x=5 AND y<10
    **            x=5 AND y>5 AND y<10
    **            x=5 AND y=5 AND z<=10
    **
    **         The z<10 term of the following cannot be used, only
    **         the x=5 term:
    **
    **            x=5 AND z<10
    **
    **         N may be zero if there are inequality constraints.
    **         If there are no inequality constraints, then N is at
    **         least one.
    **
    **         This case is also used when there are no WHERE clause
    **         constraints but an index is selected anyway, in order
    **         to force the output order to conform to an ORDER BY.
    */  
    static const u8 aStartOp[] = {
      0,
      0,
      OP_Rewind,           /* 2: (!start_constraints && startEq &&  !bRev) */
      OP_Last,             /* 3: (!start_constraints && startEq &&   bRev) */
      OP_SeekGT,           /* 4: (start_constraints  && !startEq && !bRev) */
      OP_SeekLT,           /* 5: (start_constraints  && !startEq &&  bRev) */
      OP_SeekGE,           /* 6: (start_constraints  &&  startEq && !bRev) */
      OP_SeekLE            /* 7: (start_constraints  &&  startEq &&  bRev) */
    };
    static const u8 aEndOp[] = {
      OP_IdxGE,            /* 0: (end_constraints && !bRev && !endEq) */
      OP_IdxGT,            /* 1: (end_constraints && !bRev &&  endEq) */
      OP_IdxLE,            /* 2: (end_constraints &&  bRev && !endEq) */
      OP_IdxLT,            /* 3: (end_constraints &&  bRev &&  endEq) */
    };
    u16 nEq = pLoop->u.btree.nEq;     /* Number of == or IN terms */
    int regBase;                 /* Base register holding constraint values */
    WhereTerm *pRangeStart = 0;  /* Inequality constraint at range start */
    WhereTerm *pRangeEnd = 0;    /* Inequality constraint at range end */
    int startEq;                 /* True if range start uses ==, >= or <= */
    int endEq;                   /* True if range end uses ==, >= or <= */
    int start_constraints;       /* Start of range is constrained */
    int nConstraint;             /* Number of constraint terms */
    Index *pIdx;                 /* The index we will be using */
    int iIdxCur;                 /* The VDBE cursor for the index */
    int nExtraReg = 0;           /* Number of extra registers needed */
    int op;                      /* Instruction opcode */
    char *zStartAff;             /* Affinity for start of range constraint */
    char cEndAff = 0;            /* Affinity for end of range constraint */
    u8 bSeekPastNull = 0;        /* True to seek past initial nulls */
    u8 bStopAtNull = 0;          /* Add condition to terminate at NULLs */

    pIdx = pLoop->u.btree.pIndex;
    iIdxCur = pLevel->iIdxCur;
    assert( nEq>=pLoop->nSkip );

    /* If this loop satisfies a sort order (pOrderBy) request that 
    ** was passed to this function to implement a "SELECT min(x) ..." 
    ** query, then the caller will only allow the loop to run for
    ** a single iteration. This means that the first row returned
    ** should not have a NULL value stored in 'x'. If column 'x' is
    ** the first one after the nEq equality constraints in the index,
    ** this requires some special handling.
    */
    assert( pWInfo->pOrderBy==0
         || pWInfo->pOrderBy->nExpr==1
         || (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0 );
    if( (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)!=0
     && pWInfo->nOBSat>0
     && (pIdx->nKeyCol>nEq)
    ){
      assert( pLoop->nSkip==0 );
      bSeekPastNull = 1;
      nExtraReg = 1;
    }

    /* Find any inequality constraint terms for the start and end 
    ** of the range. 
    */
    j = nEq;
    if( pLoop->wsFlags & WHERE_BTM_LIMIT ){
      pRangeStart = pLoop->aLTerm[j++];
      nExtraReg = 1;
      /* Like optimization range constraints always occur in pairs */
      assert( (pRangeStart->wtFlags & TERM_LIKEOPT)==0 || 
              (pLoop->wsFlags & WHERE_TOP_LIMIT)!=0 );
    }
    if( pLoop->wsFlags & WHERE_TOP_LIMIT ){
      pRangeEnd = pLoop->aLTerm[j++];
      nExtraReg = 1;
      if( (pRangeEnd->wtFlags & TERM_LIKEOPT)!=0 ){
        assert( pRangeStart!=0 );                     /* LIKE opt constraints */
        assert( pRangeStart->wtFlags & TERM_LIKEOPT );   /* occur in pairs */
        pLevel->iLikeRepCntr = ++pParse->nMem;
        testcase( bRev );
        testcase( pIdx->aSortOrder[nEq]==SQLITE_SO_DESC );
        sqlite3VdbeAddOp2(v, OP_Integer,
                          bRev ^ (pIdx->aSortOrder[nEq]==SQLITE_SO_DESC),
                          pLevel->iLikeRepCntr);
        VdbeComment((v, "LIKE loop counter"));
        pLevel->addrLikeRep = sqlite3VdbeCurrentAddr(v);
      }
      if( pRangeStart==0
       && (j = pIdx->aiColumn[nEq])>=0 
       && pIdx->pTable->aCol[j].notNull==0
      ){
        bSeekPastNull = 1;
      }
    }
    assert( pRangeEnd==0 || (pRangeEnd->wtFlags & TERM_VNULL)==0 );

    /* Generate code to evaluate all constraint terms using == or IN
    ** and store the values of those terms in an array of registers
    ** starting at regBase.
    */
    regBase = codeAllEqualityTerms(pParse,pLevel,bRev,nExtraReg,&zStartAff);
    assert( zStartAff==0 || sqlite3Strlen30(zStartAff)>=nEq );
    if( zStartAff ) cEndAff = zStartAff[nEq];
    addrNxt = pLevel->addrNxt;

    /* If we are doing a reverse order scan on an ascending index, or
    ** a forward order scan on a descending index, interchange the 
    ** start and end terms (pRangeStart and pRangeEnd).
    */
    if( (nEq<pIdx->nKeyCol && bRev==(pIdx->aSortOrder[nEq]==SQLITE_SO_ASC))
     || (bRev && pIdx->nKeyCol==nEq)
    ){
      SWAP(WhereTerm *, pRangeEnd, pRangeStart);
      SWAP(u8, bSeekPastNull, bStopAtNull);
    }

    testcase( pRangeStart && (pRangeStart->eOperator & WO_LE)!=0 );
    testcase( pRangeStart && (pRangeStart->eOperator & WO_GE)!=0 );
    testcase( pRangeEnd && (pRangeEnd->eOperator & WO_LE)!=0 );
    testcase( pRangeEnd && (pRangeEnd->eOperator & WO_GE)!=0 );
    startEq = !pRangeStart || pRangeStart->eOperator & (WO_LE|WO_GE);
    endEq =   !pRangeEnd || pRangeEnd->eOperator & (WO_LE|WO_GE);
    start_constraints = pRangeStart || nEq>0;

    /* Seek the index cursor to the start of the range. */
    nConstraint = nEq;
    if( pRangeStart ){
      Expr *pRight = pRangeStart->pExpr->pRight;
      sqlite3ExprCode(pParse, pRight, regBase+nEq);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeStart);
      if( (pRangeStart->wtFlags & TERM_VNULL)==0
       && sqlite3ExprCanBeNull(pRight)
      ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+nEq, addrNxt);
        VdbeCoverage(v);
      }
      if( zStartAff ){
        if( sqlite3CompareAffinity(pRight, zStartAff[nEq])==SQLITE_AFF_BLOB){
          /* Since the comparison is to be performed with no conversions
          ** applied to the operands, set the affinity to apply to pRight to 
          ** SQLITE_AFF_BLOB.  */
          zStartAff[nEq] = SQLITE_AFF_BLOB;
        }
        if( sqlite3ExprNeedsNoAffinityChange(pRight, zStartAff[nEq]) ){
          zStartAff[nEq] = SQLITE_AFF_BLOB;
        }
      }  
      nConstraint++;
      testcase( pRangeStart->wtFlags & TERM_VIRTUAL );
    }else if( bSeekPastNull ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, regBase+nEq);
      nConstraint++;
      startEq = 0;
      start_constraints = 1;
    }
    codeApplyAffinity(pParse, regBase, nConstraint - bSeekPastNull, zStartAff);
    op = aStartOp[(start_constraints<<2) + (startEq<<1) + bRev];
    assert( op!=0 );
    sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase, nConstraint);
    VdbeCoverage(v);
    VdbeCoverageIf(v, op==OP_Rewind);  testcase( op==OP_Rewind );
    VdbeCoverageIf(v, op==OP_Last);    testcase( op==OP_Last );
    VdbeCoverageIf(v, op==OP_SeekGT);  testcase( op==OP_SeekGT );
    VdbeCoverageIf(v, op==OP_SeekGE);  testcase( op==OP_SeekGE );
    VdbeCoverageIf(v, op==OP_SeekLE);  testcase( op==OP_SeekLE );
    VdbeCoverageIf(v, op==OP_SeekLT);  testcase( op==OP_SeekLT );

    /* Load the value for the inequality constraint at the end of the
    ** range (if any).
    */
    nConstraint = nEq;
    if( pRangeEnd ){
      Expr *pRight = pRangeEnd->pExpr->pRight;
      sqlite3ExprCacheRemove(pParse, regBase+nEq, 1);
      sqlite3ExprCode(pParse, pRight, regBase+nEq);
      whereLikeOptimizationStringFixup(v, pLevel, pRangeEnd);
      if( (pRangeEnd->wtFlags & TERM_VNULL)==0
       && sqlite3ExprCanBeNull(pRight)
      ){
        sqlite3VdbeAddOp2(v, OP_IsNull, regBase+nEq, addrNxt);
        VdbeCoverage(v);
      }
      if( sqlite3CompareAffinity(pRight, cEndAff)!=SQLITE_AFF_BLOB
       && !sqlite3ExprNeedsNoAffinityChange(pRight, cEndAff)
      ){
        codeApplyAffinity(pParse, regBase+nEq, 1, &cEndAff);
      }
      nConstraint++;
      testcase( pRangeEnd->wtFlags & TERM_VIRTUAL );
    }else if( bStopAtNull ){
      sqlite3VdbeAddOp2(v, OP_Null, 0, regBase+nEq);
      endEq = 0;
      nConstraint++;
    }
    sqlite3DbFree(db, zStartAff);

    /* Top of the loop body */
    pLevel->p2 = sqlite3VdbeCurrentAddr(v);

    /* Check if the index cursor is past the end of the range. */
    if( nConstraint ){
      op = aEndOp[bRev*2 + endEq];
      sqlite3VdbeAddOp4Int(v, op, iIdxCur, addrNxt, regBase, nConstraint);
      testcase( op==OP_IdxGT );  VdbeCoverageIf(v, op==OP_IdxGT );
      testcase( op==OP_IdxGE );  VdbeCoverageIf(v, op==OP_IdxGE );
      testcase( op==OP_IdxLT );  VdbeCoverageIf(v, op==OP_IdxLT );
      testcase( op==OP_IdxLE );  VdbeCoverageIf(v, op==OP_IdxLE );
    }

    /* Seek the table cursor, if required */
    disableTerm(pLevel, pRangeStart);
    disableTerm(pLevel, pRangeEnd);
    if( omitTable ){
      /* pIdx is a covering index.  No need to access the main table. */
    }else if( HasRowid(pIdx->pTable) ){
      iRowidReg = ++pParse->nMem;
      sqlite3VdbeAddOp2(v, OP_IdxRowid, iIdxCur, iRowidReg);
      sqlite3ExprCacheStore(pParse, iCur, -1, iRowidReg);
      sqlite3VdbeAddOp2(v, OP_Seek, iCur, iRowidReg);  /* Deferred seek */
    }else if( iCur!=iIdxCur ){
      Index *pPk = sqlite3PrimaryKeyIndex(pIdx->pTable);
      iRowidReg = sqlite3GetTempRange(pParse, pPk->nKeyCol);
      for(j=0; j<pPk->nKeyCol; j++){
        k = sqlite3ColumnOfIndex(pIdx, pPk->aiColumn[j]);
        sqlite3VdbeAddOp3(v, OP_Column, iIdxCur, k, iRowidReg+j);
      }
      sqlite3VdbeAddOp4Int(v, OP_NotFound, iCur, addrCont,
                           iRowidReg, pPk->nKeyCol); VdbeCoverage(v);
    }

    /* Record the instruction used to terminate the loop. Disable 
    ** WHERE clause terms made redundant by the index range scan.
    */
    if( pLoop->wsFlags & WHERE_ONEROW ){
      pLevel->op = OP_Noop;
    }else if( bRev ){
      pLevel->op = OP_Prev;
    }else{
      pLevel->op = OP_Next;
    }
    pLevel->p1 = iIdxCur;
    pLevel->p3 = (pLoop->wsFlags&WHERE_UNQ_WANTED)!=0 ? 1:0;
    if( (pLoop->wsFlags & WHERE_CONSTRAINT)==0 ){
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_STEP;
    }else{
      assert( pLevel->p5==0 );
    }
  }else

#ifndef SQLITE_OMIT_OR_OPTIMIZATION
  if( pLoop->wsFlags & WHERE_MULTI_OR ){
    /* Case 5:  Two or more separately indexed terms connected by OR
    **
    ** Example:
    **
    **   CREATE TABLE t1(a,b,c,d);
    **   CREATE INDEX i1 ON t1(a);
    **   CREATE INDEX i2 ON t1(b);
    **   CREATE INDEX i3 ON t1(c);
    **
    **   SELECT * FROM t1 WHERE a=5 OR b=7 OR (c=11 AND d=13)
    **
    ** In the example, there are three indexed terms connected by OR.
    ** The top of the loop looks like this:
    **
    **          Null       1                # Zero the rowset in reg 1
    **
    ** Then, for each indexed term, the following. The arguments to
    ** RowSetTest are such that the rowid of the current row is inserted
    ** into the RowSet. If it is already present, control skips the
    ** Gosub opcode and jumps straight to the code generated by WhereEnd().
    **
    **        sqlite3WhereBegin(<term>)
    **          RowSetTest                  # Insert rowid into rowset
    **          Gosub      2 A
    **        sqlite3WhereEnd()
    **
    ** Following the above, code to terminate the loop. Label A, the target
    ** of the Gosub above, jumps to the instruction right after the Goto.
    **
    **          Null       1                # Zero the rowset in reg 1
    **          Goto       B                # The loop is finished.
    **
    **       A: <loop body>                 # Return data, whatever.
    **
    **          Return     2                # Jump back to the Gosub
    **
    **       B: <after the loop>
    **
    ** Added 2014-05-26: If the table is a WITHOUT ROWID table, then
    ** use an ephemeral index instead of a RowSet to record the primary
    ** keys of the rows we have already seen.
    **
    */
    WhereClause *pOrWc;    /* The OR-clause broken out into subterms */
    SrcList *pOrTab;       /* Shortened table list or OR-clause generation */
    Index *pCov = 0;             /* Potential covering index (or NULL) */
    int iCovCur = pParse->nTab++;  /* Cursor used for index scans (if any) */

    int regReturn = ++pParse->nMem;           /* Register used with OP_Gosub */
    int regRowset = 0;                        /* Register for RowSet object */
    int regRowid = 0;                         /* Register holding rowid */
    int iLoopBody = sqlite3VdbeMakeLabel(v);  /* Start of loop body */
    int iRetInit;                             /* Address of regReturn init */
    int untestedTerms = 0;             /* Some terms not completely tested */
    int ii;                            /* Loop counter */
    u16 wctrlFlags;                    /* Flags for sub-WHERE clause */
    Expr *pAndExpr = 0;                /* An ".. AND (...)" expression */
    Table *pTab = pTabItem->pTab;
   
    pTerm = pLoop->aLTerm[0];
    assert( pTerm!=0 );
    assert( pTerm->eOperator & WO_OR );
    assert( (pTerm->wtFlags & TERM_ORINFO)!=0 );
    pOrWc = &pTerm->u.pOrInfo->wc;
    pLevel->op = OP_Return;
    pLevel->p1 = regReturn;

    /* Set up a new SrcList in pOrTab containing the table being scanned
    ** by this loop in the a[0] slot and all notReady tables in a[1..] slots.
    ** This becomes the SrcList in the recursive call to sqlite3WhereBegin().
    */
    if( pWInfo->nLevel>1 ){
      int nNotReady;                 /* The number of notReady tables */
      struct SrcList_item *origSrc;     /* Original list of tables */
      nNotReady = pWInfo->nLevel - iLevel - 1;
      pOrTab = sqlite3StackAllocRaw(db,
                            sizeof(*pOrTab)+ nNotReady*sizeof(pOrTab->a[0]));
      if( pOrTab==0 ) return notReady;
      pOrTab->nAlloc = (u8)(nNotReady + 1);
      pOrTab->nSrc = pOrTab->nAlloc;
      memcpy(pOrTab->a, pTabItem, sizeof(*pTabItem));
      origSrc = pWInfo->pTabList->a;
      for(k=1; k<=nNotReady; k++){
        memcpy(&pOrTab->a[k], &origSrc[pLevel[k].iFrom], sizeof(pOrTab->a[k]));
      }
    }else{
      pOrTab = pWInfo->pTabList;
    }

    /* Initialize the rowset register to contain NULL. An SQL NULL is 
    ** equivalent to an empty rowset.  Or, create an ephemeral index
    ** capable of holding primary keys in the case of a WITHOUT ROWID.
    **
    ** Also initialize regReturn to contain the address of the instruction 
    ** immediately following the OP_Return at the bottom of the loop. This
    ** is required in a few obscure LEFT JOIN cases where control jumps
    ** over the top of the loop into the body of it. In this case the 
    ** correct response for the end-of-loop code (the OP_Return) is to 
    ** fall through to the next instruction, just as an OP_Next does if
    ** called on an uninitialized cursor.
    */
    if( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
      if( HasRowid(pTab) ){
        regRowset = ++pParse->nMem;
        sqlite3VdbeAddOp2(v, OP_Null, 0, regRowset);
      }else{
        Index *pPk = sqlite3PrimaryKeyIndex(pTab);
        regRowset = pParse->nTab++;
        sqlite3VdbeAddOp2(v, OP_OpenEphemeral, regRowset, pPk->nKeyCol);
        sqlite3VdbeSetP4KeyInfo(pParse, pPk);
      }
      regRowid = ++pParse->nMem;
    }
    iRetInit = sqlite3VdbeAddOp2(v, OP_Integer, 0, regReturn);

    /* If the original WHERE clause is z of the form:  (x1 OR x2 OR ...) AND y
    ** Then for every term xN, evaluate as the subexpression: xN AND z
    ** That way, terms in y that are factored into the disjunction will
    ** be picked up by the recursive calls to sqlite3WhereBegin() below.
    **
    ** Actually, each subexpression is converted to "xN AND w" where w is
    ** the "interesting" terms of z - terms that did not originate in the
    ** ON or USING clause of a LEFT JOIN, and terms that are usable as 
    ** indices.
    **
    ** This optimization also only applies if the (x1 OR x2 OR ...) term
    ** is not contained in the ON clause of a LEFT JOIN.
    ** See ticket http://www.sqlite.org/src/info/f2369304e4
    */
    if( pWC->nTerm>1 ){
      int iTerm;
      for(iTerm=0; iTerm<pWC->nTerm; iTerm++){
        Expr *pExpr = pWC->a[iTerm].pExpr;
        if( &pWC->a[iTerm] == pTerm ) continue;
        if( ExprHasProperty(pExpr, EP_FromJoin) ) continue;
        if( (pWC->a[iTerm].wtFlags & TERM_VIRTUAL)!=0 ) continue;
        if( (pWC->a[iTerm].eOperator & WO_ALL)==0 ) continue;
        testcase( pWC->a[iTerm].wtFlags & TERM_ORINFO );
        pExpr = sqlite3ExprDup(db, pExpr, 0);
        pAndExpr = sqlite3ExprAnd(db, pAndExpr, pExpr);
      }
      if( pAndExpr ){
        pAndExpr = sqlite3PExpr(pParse, TK_AND, 0, pAndExpr, 0);
      }
    }

    /* Run a separate WHERE clause for each term of the OR clause.  After
    ** eliminating duplicates from other WHERE clauses, the action for each
    ** sub-WHERE clause is to to invoke the main loop body as a subroutine.
    */
    wctrlFlags =  WHERE_OMIT_OPEN_CLOSE
                | WHERE_FORCE_TABLE
                | WHERE_ONETABLE_ONLY
                | WHERE_NO_AUTOINDEX;
    for(ii=0; ii<pOrWc->nTerm; ii++){
      WhereTerm *pOrTerm = &pOrWc->a[ii];
      if( pOrTerm->leftCursor==iCur || (pOrTerm->eOperator & WO_AND)!=0 ){
        WhereInfo *pSubWInfo;           /* Info for single OR-term scan */
        Expr *pOrExpr = pOrTerm->pExpr; /* Current OR clause term */
        int j1 = 0;                     /* Address of jump operation */
        if( pAndExpr && !ExprHasProperty(pOrExpr, EP_FromJoin) ){
          pAndExpr->pLeft = pOrExpr;
          pOrExpr = pAndExpr;
        }
        /* Loop through table entries that match term pOrTerm. */
        WHERETRACE(0xffff, ("Subplan for OR-clause:\n"));
        pSubWInfo = sqlite3WhereBegin(pParse, pOrTab, pOrExpr, 0, 0,
                                      wctrlFlags, iCovCur);
        assert( pSubWInfo || pParse->nErr || db->mallocFailed );
        if( pSubWInfo ){
          WhereLoop *pSubLoop;
          int addrExplain = sqlite3WhereExplainOneScan(
              pParse, pOrTab, &pSubWInfo->a[0], iLevel, pLevel->iFrom, 0
          );
          sqlite3WhereAddScanStatus(v, pOrTab, &pSubWInfo->a[0], addrExplain);

          /* This is the sub-WHERE clause body.  First skip over
          ** duplicate rows from prior sub-WHERE clauses, and record the
          ** rowid (or PRIMARY KEY) for the current row so that the same
          ** row will be skipped in subsequent sub-WHERE clauses.
          */
          if( (pWInfo->wctrlFlags & WHERE_DUPLICATES_OK)==0 ){
            int r;
            int iSet = ((ii==pOrWc->nTerm-1)?-1:ii);
            if( HasRowid(pTab) ){
              r = sqlite3ExprCodeGetColumn(pParse, pTab, -1, iCur, regRowid, 0);
              j1 = sqlite3VdbeAddOp4Int(v, OP_RowSetTest, regRowset, 0, r,iSet);
              VdbeCoverage(v);
            }else{
              Index *pPk = sqlite3PrimaryKeyIndex(pTab);
              int nPk = pPk->nKeyCol;
              int iPk;

              /* Read the PK into an array of temp registers. */
              r = sqlite3GetTempRange(pParse, nPk);
              for(iPk=0; iPk<nPk; iPk++){
                int iCol = pPk->aiColumn[iPk];
                int rx;
                rx = sqlite3ExprCodeGetColumn(pParse, pTab, iCol, iCur,r+iPk,0);
                if( rx!=r+iPk ){
                  sqlite3VdbeAddOp2(v, OP_SCopy, rx, r+iPk);
                }
              }

              /* Check if the temp table already contains this key. If so,
              ** the row has already been included in the result set and
              ** can be ignored (by jumping past the Gosub below). Otherwise,
              ** insert the key into the temp table and proceed with processing
              ** the row.
              **
              ** Use some of the same optimizations as OP_RowSetTest: If iSet
              ** is zero, assume that the key cannot already be present in
              ** the temp table. And if iSet is -1, assume that there is no 
              ** need to insert the key into the temp table, as it will never 
              ** be tested for.  */ 
              if( iSet ){
                j1 = sqlite3VdbeAddOp4Int(v, OP_Found, regRowset, 0, r, nPk);
                VdbeCoverage(v);
              }
              if( iSet>=0 ){
                sqlite3VdbeAddOp3(v, OP_MakeRecord, r, nPk, regRowid);
                sqlite3VdbeAddOp3(v, OP_IdxInsert, regRowset, regRowid, 0);
                if( iSet ) sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
              }

              /* Release the array of temp registers */
              sqlite3ReleaseTempRange(pParse, r, nPk);
            }
          }

          /* Invoke the main loop body as a subroutine */
          sqlite3VdbeAddOp2(v, OP_Gosub, regReturn, iLoopBody);

          /* Jump here (skipping the main loop body subroutine) if the
          ** current sub-WHERE row is a duplicate from prior sub-WHEREs. */
          if( j1 ) sqlite3VdbeJumpHere(v, j1);

          /* The pSubWInfo->untestedTerms flag means that this OR term
          ** contained one or more AND term from a notReady table.  The
          ** terms from the notReady table could not be tested and will
          ** need to be tested later.
          */
          if( pSubWInfo->untestedTerms ) untestedTerms = 1;

          /* If all of the OR-connected terms are optimized using the same
          ** index, and the index is opened using the same cursor number
          ** by each call to sqlite3WhereBegin() made by this loop, it may
          ** be possible to use that index as a covering index.
          **
          ** If the call to sqlite3WhereBegin() above resulted in a scan that
          ** uses an index, and this is either the first OR-connected term
          ** processed or the index is the same as that used by all previous
          ** terms, set pCov to the candidate covering index. Otherwise, set 
          ** pCov to NULL to indicate that no candidate covering index will 
          ** be available.
          */
          pSubLoop = pSubWInfo->a[0].pWLoop;
          assert( (pSubLoop->wsFlags & WHERE_AUTO_INDEX)==0 );
          if( (pSubLoop->wsFlags & WHERE_INDEXED)!=0
           && (ii==0 || pSubLoop->u.btree.pIndex==pCov)
           && (HasRowid(pTab) || !IsPrimaryKeyIndex(pSubLoop->u.btree.pIndex))
          ){
            assert( pSubWInfo->a[0].iIdxCur==iCovCur );
            pCov = pSubLoop->u.btree.pIndex;
            wctrlFlags |= WHERE_REOPEN_IDX;
          }else{
            pCov = 0;
          }

          /* Finish the loop through table entries that match term pOrTerm. */
          sqlite3WhereEnd(pSubWInfo);
        }
      }
    }
    pLevel->u.pCovidx = pCov;
    if( pCov ) pLevel->iIdxCur = iCovCur;
    if( pAndExpr ){
      pAndExpr->pLeft = 0;
      sqlite3ExprDelete(db, pAndExpr);
    }
    sqlite3VdbeChangeP1(v, iRetInit, sqlite3VdbeCurrentAddr(v));
    sqlite3VdbeAddOp2(v, OP_Goto, 0, pLevel->addrBrk);
    sqlite3VdbeResolveLabel(v, iLoopBody);

    if( pWInfo->nLevel>1 ) sqlite3StackFree(db, pOrTab);
    if( !untestedTerms ) disableTerm(pLevel, pTerm);
  }else
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */

  {
    /* Case 6:  There is no usable index.  We must do a complete
    **          scan of the entire table.
    */
    static const u8 aStep[] = { OP_Next, OP_Prev };
    static const u8 aStart[] = { OP_Rewind, OP_Last };
    assert( bRev==0 || bRev==1 );
    if( pTabItem->isRecursive ){
      /* Tables marked isRecursive have only a single row that is stored in
      ** a pseudo-cursor.  No need to Rewind or Next such cursors. */
      pLevel->op = OP_Noop;
    }else{
      pLevel->op = aStep[bRev];
      pLevel->p1 = iCur;
      pLevel->p2 = 1 + sqlite3VdbeAddOp2(v, aStart[bRev], iCur, addrBrk);
      VdbeCoverageIf(v, bRev==0);
      VdbeCoverageIf(v, bRev!=0);
      pLevel->p5 = SQLITE_STMTSTATUS_FULLSCAN_STEP;
    }
  }

#ifdef SQLITE_ENABLE_STMT_SCANSTATUS
  pLevel->addrVisit = sqlite3VdbeCurrentAddr(v);
#endif

  /* Insert code to test every subexpression that can be completely
  ** computed using the current set of tables.
  */
  for(pTerm=pWC->a, j=pWC->nTerm; j>0; j--, pTerm++){
    Expr *pE;
    int skipLikeAddr = 0;
    testcase( pTerm->wtFlags & TERM_VIRTUAL );
    testcase( pTerm->wtFlags & TERM_CODED );
    if( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) continue;
    if( (pTerm->prereqAll & pLevel->notReady)!=0 ){
      testcase( pWInfo->untestedTerms==0
               && (pWInfo->wctrlFlags & WHERE_ONETABLE_ONLY)!=0 );
      pWInfo->untestedTerms = 1;
      continue;
    }
    pE = pTerm->pExpr;
    assert( pE!=0 );
    if( pLevel->iLeftJoin && !ExprHasProperty(pE, EP_FromJoin) ){
      continue;
    }
    if( pTerm->wtFlags & TERM_LIKECOND ){
      assert( pLevel->iLikeRepCntr>0 );
      skipLikeAddr = sqlite3VdbeAddOp1(v, OP_IfNot, pLevel->iLikeRepCntr);
      VdbeCoverage(v);
    }
    sqlite3ExprIfFalse(pParse, pE, addrCont, SQLITE_JUMPIFNULL);
    if( skipLikeAddr ) sqlite3VdbeJumpHere(v, skipLikeAddr);
    pTerm->wtFlags |= TERM_CODED;
  }

  /* Insert code to test for implied constraints based on transitivity
  ** of the "==" operator.
  **
  ** Example: If the WHERE clause contains "t1.a=t2.b" and "t2.b=123"
  ** and we are coding the t1 loop and the t2 loop has not yet coded,
  ** then we cannot use the "t1.a=t2.b" constraint, but we can code
  ** the implied "t1.a=123" constraint.
  */
  for(pTerm=pWC->a, j=pWC->nTerm; j>0; j--, pTerm++){
    Expr *pE, *pEAlt;
    WhereTerm *pAlt;
    if( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) continue;
    if( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) continue;
    if( (pTerm->eOperator & WO_EQUIV)==0 ) continue;
    if( pTerm->leftCursor!=iCur ) continue;
    if( pLevel->iLeftJoin ) continue;
    pE = pTerm->pExpr;
    assert( !ExprHasProperty(pE, EP_FromJoin) );
    assert( (pTerm->prereqRight & pLevel->notReady)!=0 );
    pAlt = sqlite3WhereFindTerm(pWC, iCur, pTerm->u.leftColumn, notReady,
                    WO_EQ|WO_IN|WO_IS, 0);
    if( pAlt==0 ) continue;
    if( pAlt->wtFlags & (TERM_CODED) ) continue;
    testcase( pAlt->eOperator & WO_EQ );
    testcase( pAlt->eOperator & WO_IS );
    testcase( pAlt->eOperator & WO_IN );
    VdbeModuleComment((v, "begin transitive constraint"));
    pEAlt = sqlite3StackAllocRaw(db, sizeof(*pEAlt));
    if( pEAlt ){
      *pEAlt = *pAlt->pExpr;
      pEAlt->pLeft = pE->pLeft;
      sqlite3ExprIfFalse(pParse, pEAlt, addrCont, SQLITE_JUMPIFNULL);
      sqlite3StackFree(db, pEAlt);
    }
  }

  /* For a LEFT OUTER JOIN, generate code that will record the fact that
  ** at least one row of the right table has matched the left table.  
  */
  if( pLevel->iLeftJoin ){
    pLevel->addrFirst = sqlite3VdbeCurrentAddr(v);
    sqlite3VdbeAddOp2(v, OP_Integer, 1, pLevel->iLeftJoin);
    VdbeComment((v, "record LEFT JOIN hit"));
    sqlite3ExprCacheClear(pParse);
    for(pTerm=pWC->a, j=0; j<pWC->nTerm; j++, pTerm++){
      testcase( pTerm->wtFlags & TERM_VIRTUAL );
      testcase( pTerm->wtFlags & TERM_CODED );
      if( pTerm->wtFlags & (TERM_VIRTUAL|TERM_CODED) ) continue;
      if( (pTerm->prereqAll & pLevel->notReady)!=0 ){
        assert( pWInfo->untestedTerms );
        continue;
      }
      assert( pTerm->pExpr );
      sqlite3ExprIfFalse(pParse, pTerm->pExpr, addrCont, SQLITE_JUMPIFNULL);
      pTerm->wtFlags |= TERM_CODED;
    }
  }

  return pLevel->notReady;
}

/************** End of wherecode.c *******************************************/
/************** Begin file whereexpr.c ***************************************/
/*
** 2015-06-08
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This module contains C code that generates VDBE code used to process
** the WHERE clause of SQL statements.
**
** This file was originally part of where.c but was split out to improve
** readability and editabiliity.  This file contains utility routines for
** analyzing Expr objects in the WHERE clause.
*/
/* #include "sqliteInt.h" */
/* #include "whereInt.h" */

/* Forward declarations */
static void exprAnalyze(SrcList*, WhereClause*, int);

/*
** Deallocate all memory associated with a WhereOrInfo object.
*/
static void whereOrInfoDelete(sqlite3 *db, WhereOrInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Deallocate all memory associated with a WhereAndInfo object.
*/
static void whereAndInfoDelete(sqlite3 *db, WhereAndInfo *p){
  sqlite3WhereClauseClear(&p->wc);
  sqlite3DbFree(db, p);
}

/*
** Add a single new WhereTerm entry to the WhereClause object pWC.
** The new WhereTerm object is constructed from Expr p and with wtFlags.
** The index in pWC->a[] of the new WhereTerm is returned on success.
** 0 is returned if the new WhereTerm could not be added due to a memory
** allocation error.  The memory allocation failure will be recorded in
** the db->mallocFailed flag so that higher-level functions can detect it.
**
** This routine will increase the size of the pWC->a[] array as necessary.
**
** If the wtFlags argument includes TERM_DYNAMIC, then responsibility
** for freeing the expression p is assumed by the WhereClause object pWC.
** This is true even if this routine fails to allocate a new WhereTerm.
**
** WARNING:  This routine might reallocate the space used to store
** WhereTerms.  All pointers to WhereTerms should be invalidated after
** calling this routine.  Such pointers may be reinitialized by referencing
** the pWC->a[] array.
*/
static int whereClauseInsert(WhereClause *pWC, Expr *p, u16 wtFlags){
  WhereTerm *pTerm;
  int idx;
  testcase( wtFlags & TERM_VIRTUAL );
  if( pWC->nTerm>=pWC->nSlot ){
    WhereTerm *pOld = pWC->a;
    sqlite3 *db = pWC->pWInfo->pParse->db;
    pWC->a = sqlite3DbMallocRaw(db, sizeof(pWC->a[0])*pWC->nSlot*2 );
    if( pWC->a==0 ){
      if( wtFlags & TERM_DYNAMIC ){
        sqlite3ExprDelete(db, p);
      }
      pWC->a = pOld;
      return 0;
    }
    memcpy(pWC->a, pOld, sizeof(pWC->a[0])*pWC->nTerm);
    if( pOld!=pWC->aStatic ){
      sqlite3DbFree(db, pOld);
    }
    pWC->nSlot = sqlite3DbMallocSize(db, pWC->a)/sizeof(pWC->a[0]);
    memset(&pWC->a[pWC->nTerm], 0, sizeof(pWC->a[0])*(pWC->nSlot-pWC->nTerm));
  }
  pTerm = &pWC->a[idx = pWC->nTerm++];
  if( p && ExprHasProperty(p, EP_Unlikely) ){
    pTerm->truthProb = sqlite3LogEst(p->iTable) - 270;
  }else{
    pTerm->truthProb = 1;
  }
  pTerm->pExpr = sqlite3ExprSkipCollate(p);
  pTerm->wtFlags = wtFlags;
  pTerm->pWC = pWC;
  pTerm->iParent = -1;
  return idx;
}

/*
** Return TRUE if the given operator is one of the operators that is
** allowed for an indexable WHERE clause term.  The allowed operators are
** "=", "<", ">", "<=", ">=", "IN", and "IS NULL"
*/
static int allowedOp(int op){
  assert( TK_GT>TK_EQ && TK_GT<TK_GE );
  assert( TK_LT>TK_EQ && TK_LT<TK_GE );
  assert( TK_LE>TK_EQ && TK_LE<TK_GE );
  assert( TK_GE==TK_EQ+4 );
  return op==TK_IN || (op>=TK_EQ && op<=TK_GE) || op==TK_ISNULL || op==TK_IS;
}

/*
** Commute a comparison operator.  Expressions of the form "X op Y"
** are converted into "Y op X".
**
** If left/right precedence rules come into play when determining the
** collating sequence, then COLLATE operators are adjusted to ensure
** that the collating sequence does not change.  For example:
** "Y collate NOCASE op X" becomes "X op Y" because any collation sequence on
** the left hand side of a comparison overrides any collation sequence 
** attached to the right. For the same reason the EP_Collate flag
** is not commuted.
*/
static void exprCommute(Parse *pParse, Expr *pExpr){
  u16 expRight = (pExpr->pRight->flags & EP_Collate);
  u16 expLeft = (pExpr->pLeft->flags & EP_Collate);
  assert( allowedOp(pExpr->op) && pExpr->op!=TK_IN );
  if( expRight==expLeft ){
    /* Either X and Y both have COLLATE operator or neither do */
    if( expRight ){
      /* Both X and Y have COLLATE operators.  Make sure X is always
      ** used by clearing the EP_Collate flag from Y. */
      pExpr->pRight->flags &= ~EP_Collate;
    }else if( sqlite3ExprCollSeq(pParse, pExpr->pLeft)!=0 ){
      /* Neither X nor Y have COLLATE operators, but X has a non-default
      ** collating sequence.  So add the EP_Collate marker on X to cause
      ** it to be searched first. */
      pExpr->pLeft->flags |= EP_Collate;
    }
  }
  SWAP(Expr*,pExpr->pRight,pExpr->pLeft);
  if( pExpr->op>=TK_GT ){
    assert( TK_LT==TK_GT+2 );
    assert( TK_GE==TK_LE+2 );
    assert( TK_GT>TK_EQ );
    assert( TK_GT<TK_LE );
    assert( pExpr->op>=TK_GT && pExpr->op<=TK_GE );
    pExpr->op = ((pExpr->op-TK_GT)^2)+TK_GT;
  }
}

/*
** Translate from TK_xx operator to WO_xx bitmask.
*/
static u16 operatorMask(int op){
  u16 c;
  assert( allowedOp(op) );
  if( op==TK_IN ){
    c = WO_IN;
  }else if( op==TK_ISNULL ){
    c = WO_ISNULL;
  }else if( op==TK_IS ){
    c = WO_IS;
  }else{
    assert( (WO_EQ<<(op-TK_EQ)) < 0x7fff );
    c = (u16)(WO_EQ<<(op-TK_EQ));
  }
  assert( op!=TK_ISNULL || c==WO_ISNULL );
  assert( op!=TK_IN || c==WO_IN );
  assert( op!=TK_EQ || c==WO_EQ );
  assert( op!=TK_LT || c==WO_LT );
  assert( op!=TK_LE || c==WO_LE );
  assert( op!=TK_GT || c==WO_GT );
  assert( op!=TK_GE || c==WO_GE );
  assert( op!=TK_IS || c==WO_IS );
  return c;
}


#ifndef SQLITE_OMIT_LIKE_OPTIMIZATION
/*
** Check to see if the given expression is a LIKE or GLOB operator that
** can be optimized using inequality constraints.  Return TRUE if it is
** so and false if not.
**
** In order for the operator to be optimizible, the RHS must be a string
** literal that does not begin with a wildcard.  The LHS must be a column
** that may only be NULL, a string, or a BLOB, never a number. (This means
** that virtual tables cannot participate in the LIKE optimization.)  The
** collating sequence for the column on the LHS must be appropriate for
** the operator.
*/
static int isLikeOrGlob(
  Parse *pParse,    /* Parsing and code generating context */
  Expr *pExpr,      /* Test this expression */
  Expr **ppPrefix,  /* Pointer to TK_STRING expression with pattern prefix */
  int *pisComplete, /* True if the only wildcard is % in the last character */
  int *pnoCase      /* True if uppercase is equivalent to lowercase */
){
  const char *z = 0;         /* String on RHS of LIKE operator */
  Expr *pRight, *pLeft;      /* Right and left size of LIKE operator */
  ExprList *pList;           /* List of operands to the LIKE operator */
  int c;                     /* One character in z[] */
  int cnt;                   /* Number of non-wildcard prefix characters */
  char wc[3];                /* Wildcard characters */
  sqlite3 *db = pParse->db;  /* Database connection */
  sqlite3_value *pVal = 0;
  int op;                    /* Opcode of pRight */

  if( !sqlite3IsLikeFunction(db, pExpr, pnoCase, wc) ){
    return 0;
  }
#ifdef SQLITE_EBCDIC
  if( *pnoCase ) return 0;
#endif
  pList = pExpr->x.pList;
  pLeft = pList->a[1].pExpr;
  if( pLeft->op!=TK_COLUMN 
   || sqlite3ExprAffinity(pLeft)!=SQLITE_AFF_TEXT 
   || IsVirtual(pLeft->pTab)  /* Value might be numeric */
  ){
    /* IMP: R-02065-49465 The left-hand side of the LIKE or GLOB operator must
    ** be the name of an indexed column with TEXT affinity. */
    return 0;
  }
  assert( pLeft->iColumn!=(-1) ); /* Because IPK never has AFF_TEXT */

  pRight = sqlite3ExprSkipCollate(pList->a[0].pExpr);
  op = pRight->op;
  if( op==TK_VARIABLE ){
    Vdbe *pReprepare = pParse->pReprepare;
    int iCol = pRight->iColumn;
    pVal = sqlite3VdbeGetBoundValue(pReprepare, iCol, SQLITE_AFF_BLOB);
    if( pVal && sqlite3_value_type(pVal)==SQLITE_TEXT ){
      z = (char *)sqlite3_value_text(pVal);
    }
    sqlite3VdbeSetVarmask(pParse->pVdbe, iCol);
    assert( pRight->op==TK_VARIABLE || pRight->op==TK_REGISTER );
  }else if( op==TK_STRING ){
    z = pRight->u.zToken;
  }
  if( z ){
    cnt = 0;
    while( (c=z[cnt])!=0 && c!=wc[0] && c!=wc[1] && c!=wc[2] ){
      cnt++;
    }
    if( cnt!=0 && 255!=(u8)z[cnt-1] ){
      Expr *pPrefix;
      *pisComplete = c==wc[0] && z[cnt+1]==0;
      pPrefix = sqlite3Expr(db, TK_STRING, z);
      if( pPrefix ) pPrefix->u.zToken[cnt] = 0;
      *ppPrefix = pPrefix;
      if( op==TK_VARIABLE ){
        Vdbe *v = pParse->pVdbe;
        sqlite3VdbeSetVarmask(v, pRight->iColumn);
        if( *pisComplete && pRight->u.zToken[1] ){
          /* If the rhs of the LIKE expression is a variable, and the current
          ** value of the variable means there is no need to invoke the LIKE
          ** function, then no OP_Variable will be added to the program.
          ** This causes problems for the sqlite3_bind_parameter_name()
          ** API. To work around them, add a dummy OP_Variable here.
          */ 
          int r1 = sqlite3GetTempReg(pParse);
          sqlite3ExprCodeTarget(pParse, pRight, r1);
          sqlite3VdbeChangeP3(v, sqlite3VdbeCurrentAddr(v)-1, 0);
          sqlite3ReleaseTempReg(pParse, r1);
        }
      }
    }else{
      z = 0;
    }
  }

  sqlite3ValueFree(pVal);
  return (z!=0);
}
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */


#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Check to see if the given expression is of the form
**
**         column MATCH expr
**
** If it is then return TRUE.  If not, return FALSE.
*/
static int isMatchOfColumn(
  Expr *pExpr      /* Test this expression */
){
  ExprList *pList;

  if( pExpr->op!=TK_FUNCTION ){
    return 0;
  }
  if( sqlite3StrICmp(pExpr->u.zToken,"match")!=0 ){
    return 0;
  }
  pList = pExpr->x.pList;
  if( pList->nExpr!=2 ){
    return 0;
  }
  if( pList->a[1].pExpr->op != TK_COLUMN ){
    return 0;
  }
  return 1;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** If the pBase expression originated in the ON or USING clause of
** a join, then transfer the appropriate markings over to derived.
*/
static void transferJoinMarkings(Expr *pDerived, Expr *pBase){
  if( pDerived ){
    pDerived->flags |= pBase->flags & EP_FromJoin;
    pDerived->iRightJoinTable = pBase->iRightJoinTable;
  }
}

/*
** Mark term iChild as being a child of term iParent
*/
static void markTermAsChild(WhereClause *pWC, int iChild, int iParent){
  pWC->a[iChild].iParent = iParent;
  pWC->a[iChild].truthProb = pWC->a[iParent].truthProb;
  pWC->a[iParent].nChild++;
}

/*
** Return the N-th AND-connected subterm of pTerm.  Or if pTerm is not
** a conjunction, then return just pTerm when N==0.  If N is exceeds
** the number of available subterms, return NULL.
*/
static WhereTerm *whereNthSubterm(WhereTerm *pTerm, int N){
  if( pTerm->eOperator!=WO_AND ){
    return N==0 ? pTerm : 0;
  }
  if( N<pTerm->u.pAndInfo->wc.nTerm ){
    return &pTerm->u.pAndInfo->wc.a[N];
  }
  return 0;
}

/*
** Subterms pOne and pTwo are contained within WHERE clause pWC.  The
** two subterms are in disjunction - they are OR-ed together.
**
** If these two terms are both of the form:  "A op B" with the same
** A and B values but different operators and if the operators are
** compatible (if one is = and the other is <, for example) then
** add a new virtual AND term to pWC that is the combination of the
** two.
**
** Some examples:
**
**    x<y OR x=y    -->     x<=y
**    x=y OR x=y    -->     x=y
**    x<=y OR x<y   -->     x<=y
**
** The following is NOT generated:
**
**    x<y OR x>y    -->     x!=y     
*/
static void whereCombineDisjuncts(
  SrcList *pSrc,         /* the FROM clause */
  WhereClause *pWC,      /* The complete WHERE clause */
  WhereTerm *pOne,       /* First disjunct */
  WhereTerm *pTwo        /* Second disjunct */
){
  u16 eOp = pOne->eOperator | pTwo->eOperator;
  sqlite3 *db;           /* Database connection (for malloc) */
  Expr *pNew;            /* New virtual expression */
  int op;                /* Operator for the combined expression */
  int idxNew;            /* Index in pWC of the next virtual term */

  if( (pOne->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) return;
  if( (pTwo->eOperator & (WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE))==0 ) return;
  if( (eOp & (WO_EQ|WO_LT|WO_LE))!=eOp
   && (eOp & (WO_EQ|WO_GT|WO_GE))!=eOp ) return;
  assert( pOne->pExpr->pLeft!=0 && pOne->pExpr->pRight!=0 );
  assert( pTwo->pExpr->pLeft!=0 && pTwo->pExpr->pRight!=0 );
  if( sqlite3ExprCompare(pOne->pExpr->pLeft, pTwo->pExpr->pLeft, -1) ) return;
  if( sqlite3ExprCompare(pOne->pExpr->pRight, pTwo->pExpr->pRight, -1) )return;
  /* If we reach this point, it means the two subterms can be combined */
  if( (eOp & (eOp-1))!=0 ){
    if( eOp & (WO_LT|WO_LE) ){
      eOp = WO_LE;
    }else{
      assert( eOp & (WO_GT|WO_GE) );
      eOp = WO_GE;
    }
  }
  db = pWC->pWInfo->pParse->db;
  pNew = sqlite3ExprDup(db, pOne->pExpr, 0);
  if( pNew==0 ) return;
  for(op=TK_EQ; eOp!=(WO_EQ<<(op-TK_EQ)); op++){ assert( op<TK_GE ); }
  pNew->op = op;
  idxNew = whereClauseInsert(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
  exprAnalyze(pSrc, pWC, idxNew);
}

#if !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
/*
** Analyze a term that consists of two or more OR-connected
** subterms.  So in:
**
**     ... WHERE  (a=5) AND (b=7 OR c=9 OR d=13) AND (d=13)
**                          ^^^^^^^^^^^^^^^^^^^^
**
** This routine analyzes terms such as the middle term in the above example.
** A WhereOrTerm object is computed and attached to the term under
** analysis, regardless of the outcome of the analysis.  Hence:
**
**     WhereTerm.wtFlags   |=  TERM_ORINFO
**     WhereTerm.u.pOrInfo  =  a dynamically allocated WhereOrTerm object
**
** The term being analyzed must have two or more of OR-connected subterms.
** A single subterm might be a set of AND-connected sub-subterms.
** Examples of terms under analysis:
**
**     (A)     t1.x=t2.y OR t1.x=t2.z OR t1.y=15 OR t1.z=t3.a+5
**     (B)     x=expr1 OR expr2=x OR x=expr3
**     (C)     t1.x=t2.y OR (t1.x=t2.z AND t1.y=15)
**     (D)     x=expr1 OR (y>11 AND y<22 AND z LIKE '*hello*')
**     (E)     (p.a=1 AND q.b=2 AND r.c=3) OR (p.x=4 AND q.y=5 AND r.z=6)
**     (F)     x>A OR (x=A AND y>=B)
**
** CASE 1:
**
** If all subterms are of the form T.C=expr for some single column of C and
** a single table T (as shown in example B above) then create a new virtual
** term that is an equivalent IN expression.  In other words, if the term
** being analyzed is:
**
**      x = expr1  OR  expr2 = x  OR  x = expr3
**
** then create a new virtual term like this:
**
**      x IN (expr1,expr2,expr3)
**
** CASE 2:
**
** If there are exactly two disjuncts and one side has x>A and the other side
** has x=A (for the same x and A) then add a new virtual conjunct term to the
** WHERE clause of the form "x>=A".  Example:
**
**      x>A OR (x=A AND y>B)    adds:    x>=A
**
** The added conjunct can sometimes be helpful in query planning.
**
** CASE 3:
**
** If all subterms are indexable by a single table T, then set
**
**     WhereTerm.eOperator              =  WO_OR
**     WhereTerm.u.pOrInfo->indexable  |=  the cursor number for table T
**
** A subterm is "indexable" if it is of the form
** "T.C <op> <expr>" where C is any column of table T and 
** <op> is one of "=", "<", "<=", ">", ">=", "IS NULL", or "IN".
** A subterm is also indexable if it is an AND of two or more
** subsubterms at least one of which is indexable.  Indexable AND 
** subterms have their eOperator set to WO_AND and they have
** u.pAndInfo set to a dynamically allocated WhereAndTerm object.
**
** From another point of view, "indexable" means that the subterm could
** potentially be used with an index if an appropriate index exists.
** This analysis does not consider whether or not the index exists; that
** is decided elsewhere.  This analysis only looks at whether subterms
** appropriate for indexing exist.
**
** All examples A through E above satisfy case 3.  But if a term
** also satisfies case 1 (such as B) we know that the optimizer will
** always prefer case 1, so in that case we pretend that case 3 is not
** satisfied.
**
** It might be the case that multiple tables are indexable.  For example,
** (E) above is indexable on tables P, Q, and R.
**
** Terms that satisfy case 3 are candidates for lookup by using
** separate indices to find rowids for each subterm and composing
** the union of all rowids using a RowSet object.  This is similar
** to "bitmap indices" in other database engines.
**
** OTHERWISE:
**
** If none of cases 1, 2, or 3 apply, then leave the eOperator set to
** zero.  This term is not useful for search.
*/
static void exprAnalyzeOrTerm(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the complete WHERE clause */
  int idxTerm               /* Index of the OR-term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo;        /* WHERE clause processing context */
  Parse *pParse = pWInfo->pParse;         /* Parser context */
  sqlite3 *db = pParse->db;               /* Database connection */
  WhereTerm *pTerm = &pWC->a[idxTerm];    /* The term to be analyzed */
  Expr *pExpr = pTerm->pExpr;             /* The expression of the term */
  int i;                                  /* Loop counters */
  WhereClause *pOrWc;       /* Breakup of pTerm into subterms */
  WhereTerm *pOrTerm;       /* A Sub-term within the pOrWc */
  WhereOrInfo *pOrInfo;     /* Additional information associated with pTerm */
  Bitmask chngToIN;         /* Tables that might satisfy case 1 */
  Bitmask indexable;        /* Tables that are indexable, satisfying case 2 */

  /*
  ** Break the OR clause into its separate subterms.  The subterms are
  ** stored in a WhereClause structure containing within the WhereOrInfo
  ** object that is attached to the original OR clause term.
  */
  assert( (pTerm->wtFlags & (TERM_DYNAMIC|TERM_ORINFO|TERM_ANDINFO))==0 );
  assert( pExpr->op==TK_OR );
  pTerm->u.pOrInfo = pOrInfo = sqlite3DbMallocZero(db, sizeof(*pOrInfo));
  if( pOrInfo==0 ) return;
  pTerm->wtFlags |= TERM_ORINFO;
  pOrWc = &pOrInfo->wc;
  sqlite3WhereClauseInit(pOrWc, pWInfo);
  sqlite3WhereSplit(pOrWc, pExpr, TK_OR);
  sqlite3WhereExprAnalyze(pSrc, pOrWc);
  if( db->mallocFailed ) return;
  assert( pOrWc->nTerm>=2 );

  /*
  ** Compute the set of tables that might satisfy cases 1 or 3.
  */
  indexable = ~(Bitmask)0;
  chngToIN = ~(Bitmask)0;
  for(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0 && indexable; i--, pOrTerm++){
    if( (pOrTerm->eOperator & WO_SINGLE)==0 ){
      WhereAndInfo *pAndInfo;
      assert( (pOrTerm->wtFlags & (TERM_ANDINFO|TERM_ORINFO))==0 );
      chngToIN = 0;
      pAndInfo = sqlite3DbMallocRaw(db, sizeof(*pAndInfo));
      if( pAndInfo ){
        WhereClause *pAndWC;
        WhereTerm *pAndTerm;
        int j;
        Bitmask b = 0;
        pOrTerm->u.pAndInfo = pAndInfo;
        pOrTerm->wtFlags |= TERM_ANDINFO;
        pOrTerm->eOperator = WO_AND;
        pAndWC = &pAndInfo->wc;
        sqlite3WhereClauseInit(pAndWC, pWC->pWInfo);
        sqlite3WhereSplit(pAndWC, pOrTerm->pExpr, TK_AND);
        sqlite3WhereExprAnalyze(pSrc, pAndWC);
        pAndWC->pOuter = pWC;
        testcase( db->mallocFailed );
        if( !db->mallocFailed ){
          for(j=0, pAndTerm=pAndWC->a; j<pAndWC->nTerm; j++, pAndTerm++){
            assert( pAndTerm->pExpr );
            if( allowedOp(pAndTerm->pExpr->op) ){
              b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pAndTerm->leftCursor);
            }
          }
        }
        indexable &= b;
      }
    }else if( pOrTerm->wtFlags & TERM_COPIED ){
      /* Skip this term for now.  We revisit it when we process the
      ** corresponding TERM_VIRTUAL term */
    }else{
      Bitmask b;
      b = sqlite3WhereGetMask(&pWInfo->sMaskSet, pOrTerm->leftCursor);
      if( pOrTerm->wtFlags & TERM_VIRTUAL ){
        WhereTerm *pOther = &pOrWc->a[pOrTerm->iParent];
        b |= sqlite3WhereGetMask(&pWInfo->sMaskSet, pOther->leftCursor);
      }
      indexable &= b;
      if( (pOrTerm->eOperator & WO_EQ)==0 ){
        chngToIN = 0;
      }else{
        chngToIN &= b;
      }
    }
  }

  /*
  ** Record the set of tables that satisfy case 3.  The set might be
  ** empty.
  */
  pOrInfo->indexable = indexable;
  pTerm->eOperator = indexable==0 ? 0 : WO_OR;

  /* For a two-way OR, attempt to implementation case 2.
  */
  if( indexable && pOrWc->nTerm==2 ){
    int iOne = 0;
    WhereTerm *pOne;
    while( (pOne = whereNthSubterm(&pOrWc->a[0],iOne++))!=0 ){
      int iTwo = 0;
      WhereTerm *pTwo;
      while( (pTwo = whereNthSubterm(&pOrWc->a[1],iTwo++))!=0 ){
        whereCombineDisjuncts(pSrc, pWC, pOne, pTwo);
      }
    }
  }

  /*
  ** chngToIN holds a set of tables that *might* satisfy case 1.  But
  ** we have to do some additional checking to see if case 1 really
  ** is satisfied.
  **
  ** chngToIN will hold either 0, 1, or 2 bits.  The 0-bit case means
  ** that there is no possibility of transforming the OR clause into an
  ** IN operator because one or more terms in the OR clause contain
  ** something other than == on a column in the single table.  The 1-bit
  ** case means that every term of the OR clause is of the form
  ** "table.column=expr" for some single table.  The one bit that is set
  ** will correspond to the common table.  We still need to check to make
  ** sure the same column is used on all terms.  The 2-bit case is when
  ** the all terms are of the form "table1.column=table2.column".  It
  ** might be possible to form an IN operator with either table1.column
  ** or table2.column as the LHS if either is common to every term of
  ** the OR clause.
  **
  ** Note that terms of the form "table.column1=table.column2" (the
  ** same table on both sizes of the ==) cannot be optimized.
  */
  if( chngToIN ){
    int okToChngToIN = 0;     /* True if the conversion to IN is valid */
    int iColumn = -1;         /* Column index on lhs of IN operator */
    int iCursor = -1;         /* Table cursor common to all terms */
    int j = 0;                /* Loop counter */

    /* Search for a table and column that appears on one side or the
    ** other of the == operator in every subterm.  That table and column
    ** will be recorded in iCursor and iColumn.  There might not be any
    ** such table and column.  Set okToChngToIN if an appropriate table
    ** and column is found but leave okToChngToIN false if not found.
    */
    for(j=0; j<2 && !okToChngToIN; j++){
      pOrTerm = pOrWc->a;
      for(i=pOrWc->nTerm-1; i>=0; i--, pOrTerm++){
        assert( pOrTerm->eOperator & WO_EQ );
        pOrTerm->wtFlags &= ~TERM_OR_OK;
        if( pOrTerm->leftCursor==iCursor ){
          /* This is the 2-bit case and we are on the second iteration and
          ** current term is from the first iteration.  So skip this term. */
          assert( j==1 );
          continue;
        }
        if( (chngToIN & sqlite3WhereGetMask(&pWInfo->sMaskSet,
                                            pOrTerm->leftCursor))==0 ){
          /* This term must be of the form t1.a==t2.b where t2 is in the
          ** chngToIN set but t1 is not.  This term will be either preceded
          ** or follwed by an inverted copy (t2.b==t1.a).  Skip this term 
          ** and use its inversion. */
          testcase( pOrTerm->wtFlags & TERM_COPIED );
          testcase( pOrTerm->wtFlags & TERM_VIRTUAL );
          assert( pOrTerm->wtFlags & (TERM_COPIED|TERM_VIRTUAL) );
          continue;
        }
        iColumn = pOrTerm->u.leftColumn;
        iCursor = pOrTerm->leftCursor;
        break;
      }
      if( i<0 ){
        /* No candidate table+column was found.  This can only occur
        ** on the second iteration */
        assert( j==1 );
        assert( IsPowerOfTwo(chngToIN) );
        assert( chngToIN==sqlite3WhereGetMask(&pWInfo->sMaskSet, iCursor) );
        break;
      }
      testcase( j==1 );

      /* We have found a candidate table and column.  Check to see if that
      ** table and column is common to every term in the OR clause */
      okToChngToIN = 1;
      for(; i>=0 && okToChngToIN; i--, pOrTerm++){
        assert( pOrTerm->eOperator & WO_EQ );
        if( pOrTerm->leftCursor!=iCursor ){
          pOrTerm->wtFlags &= ~TERM_OR_OK;
        }else if( pOrTerm->u.leftColumn!=iColumn ){
          okToChngToIN = 0;
        }else{
          int affLeft, affRight;
          /* If the right-hand side is also a column, then the affinities
          ** of both right and left sides must be such that no type
          ** conversions are required on the right.  (Ticket #2249)
          */
          affRight = sqlite3ExprAffinity(pOrTerm->pExpr->pRight);
          affLeft = sqlite3ExprAffinity(pOrTerm->pExpr->pLeft);
          if( affRight!=0 && affRight!=affLeft ){
            okToChngToIN = 0;
          }else{
            pOrTerm->wtFlags |= TERM_OR_OK;
          }
        }
      }
    }

    /* At this point, okToChngToIN is true if original pTerm satisfies
    ** case 1.  In that case, construct a new virtual term that is 
    ** pTerm converted into an IN operator.
    */
    if( okToChngToIN ){
      Expr *pDup;            /* A transient duplicate expression */
      ExprList *pList = 0;   /* The RHS of the IN operator */
      Expr *pLeft = 0;       /* The LHS of the IN operator */
      Expr *pNew;            /* The complete IN operator */

      for(i=pOrWc->nTerm-1, pOrTerm=pOrWc->a; i>=0; i--, pOrTerm++){
        if( (pOrTerm->wtFlags & TERM_OR_OK)==0 ) continue;
        assert( pOrTerm->eOperator & WO_EQ );
        assert( pOrTerm->leftCursor==iCursor );
        assert( pOrTerm->u.leftColumn==iColumn );
        pDup = sqlite3ExprDup(db, pOrTerm->pExpr->pRight, 0);
        pList = sqlite3ExprListAppend(pWInfo->pParse, pList, pDup);
        pLeft = pOrTerm->pExpr->pLeft;
      }
      assert( pLeft!=0 );
      pDup = sqlite3ExprDup(db, pLeft, 0);
      pNew = sqlite3PExpr(pParse, TK_IN, pDup, 0, 0);
      if( pNew ){
        int idxNew;
        transferJoinMarkings(pNew, pExpr);
        assert( !ExprHasProperty(pNew, EP_xIsSelect) );
        pNew->x.pList = pList;
        idxNew = whereClauseInsert(pWC, pNew, TERM_VIRTUAL|TERM_DYNAMIC);
        testcase( idxNew==0 );
        exprAnalyze(pSrc, pWC, idxNew);
        pTerm = &pWC->a[idxTerm];
        markTermAsChild(pWC, idxNew, idxTerm);
      }else{
        sqlite3ExprListDelete(db, pList);
      }
      pTerm->eOperator = WO_NOOP;  /* case 1 trumps case 3 */
    }
  }
}
#endif /* !SQLITE_OMIT_OR_OPTIMIZATION && !SQLITE_OMIT_SUBQUERY */

/*
** We already know that pExpr is a binary operator where both operands are
** column references.  This routine checks to see if pExpr is an equivalence
** relation:
**   1.  The SQLITE_Transitive optimization must be enabled
**   2.  Must be either an == or an IS operator
**   3.  Not originating in the ON clause of an OUTER JOIN
**   4.  The affinities of A and B must be compatible
**   5a. Both operands use the same collating sequence OR
**   5b. The overall collating sequence is BINARY
** If this routine returns TRUE, that means that the RHS can be substituted
** for the LHS anyplace else in the WHERE clause where the LHS column occurs.
** This is an optimization.  No harm comes from returning 0.  But if 1 is
** returned when it should not be, then incorrect answers might result.
*/
static int termIsEquivalence(Parse *pParse, Expr *pExpr){
  char aff1, aff2;
  CollSeq *pColl;
  const char *zColl1, *zColl2;
  if( !OptimizationEnabled(pParse->db, SQLITE_Transitive) ) return 0;
  if( pExpr->op!=TK_EQ && pExpr->op!=TK_IS ) return 0;
  if( ExprHasProperty(pExpr, EP_FromJoin) ) return 0;
  aff1 = sqlite3ExprAffinity(pExpr->pLeft);
  aff2 = sqlite3ExprAffinity(pExpr->pRight);
  if( aff1!=aff2
   && (!sqlite3IsNumericAffinity(aff1) || !sqlite3IsNumericAffinity(aff2))
  ){
    return 0;
  }
  pColl = sqlite3BinaryCompareCollSeq(pParse, pExpr->pLeft, pExpr->pRight);
  if( pColl==0 || sqlite3StrICmp(pColl->zName, "BINARY")==0 ) return 1;
  pColl = sqlite3ExprCollSeq(pParse, pExpr->pLeft);
  /* Since pLeft and pRight are both a column references, their collating
  ** sequence should always be defined. */
  zColl1 = ALWAYS(pColl) ? pColl->zName : 0;
  pColl = sqlite3ExprCollSeq(pParse, pExpr->pRight);
  zColl2 = ALWAYS(pColl) ? pColl->zName : 0;
  return sqlite3StrICmp(zColl1, zColl2)==0;
}

/*
** Recursively walk the expressions of a SELECT statement and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
static Bitmask exprSelectUsage(WhereMaskSet *pMaskSet, Select *pS){
  Bitmask mask = 0;
  while( pS ){
    SrcList *pSrc = pS->pSrc;
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pEList);
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pGroupBy);
    mask |= sqlite3WhereExprListUsage(pMaskSet, pS->pOrderBy);
    mask |= sqlite3WhereExprUsage(pMaskSet, pS->pWhere);
    mask |= sqlite3WhereExprUsage(pMaskSet, pS->pHaving);
    if( ALWAYS(pSrc!=0) ){
      int i;
      for(i=0; i<pSrc->nSrc; i++){
        mask |= exprSelectUsage(pMaskSet, pSrc->a[i].pSelect);
        mask |= sqlite3WhereExprUsage(pMaskSet, pSrc->a[i].pOn);
      }
    }
    pS = pS->pPrior;
  }
  return mask;
}

/*
** The input to this routine is an WhereTerm structure with only the
** "pExpr" field filled in.  The job of this routine is to analyze the
** subexpression and populate all the other fields of the WhereTerm
** structure.
**
** If the expression is of the form "<expr> <op> X" it gets commuted
** to the standard form of "X <op> <expr>".
**
** If the expression is of the form "X <op> Y" where both X and Y are
** columns, then the original expression is unchanged and a new virtual
** term of the form "Y <op> X" is added to the WHERE clause and
** analyzed separately.  The original term is marked with TERM_COPIED
** and the new term is marked with TERM_DYNAMIC (because it's pExpr
** needs to be freed with the WhereClause) and TERM_VIRTUAL (because it
** is a commuted copy of a prior term.)  The original term has nChild=1
** and the copy has idxParent set to the index of the original term.
*/
static void exprAnalyze(
  SrcList *pSrc,            /* the FROM clause */
  WhereClause *pWC,         /* the WHERE clause */
  int idxTerm               /* Index of the term to be analyzed */
){
  WhereInfo *pWInfo = pWC->pWInfo; /* WHERE clause processing context */
  WhereTerm *pTerm;                /* The term to be analyzed */
  WhereMaskSet *pMaskSet;          /* Set of table index masks */
  Expr *pExpr;                     /* The expression to be analyzed */
  Bitmask prereqLeft;              /* Prerequesites of the pExpr->pLeft */
  Bitmask prereqAll;               /* Prerequesites of pExpr */
  Bitmask extraRight = 0;          /* Extra dependencies on LEFT JOIN */
  Expr *pStr1 = 0;                 /* RHS of LIKE/GLOB operator */
  int isComplete = 0;              /* RHS of LIKE/GLOB ends with wildcard */
  int noCase = 0;                  /* uppercase equivalent to lowercase */
  int op;                          /* Top-level operator.  pExpr->op */
  Parse *pParse = pWInfo->pParse;  /* Parsing context */
  sqlite3 *db = pParse->db;        /* Database connection */

  if( db->mallocFailed ){
    return;
  }
  pTerm = &pWC->a[idxTerm];
  pMaskSet = &pWInfo->sMaskSet;
  pExpr = pTerm->pExpr;
  assert( pExpr->op!=TK_AS && pExpr->op!=TK_COLLATE );
  prereqLeft = sqlite3WhereExprUsage(pMaskSet, pExpr->pLeft);
  op = pExpr->op;
  if( op==TK_IN ){
    assert( pExpr->pRight==0 );
    if( ExprHasProperty(pExpr, EP_xIsSelect) ){
      pTerm->prereqRight = exprSelectUsage(pMaskSet, pExpr->x.pSelect);
    }else{
      pTerm->prereqRight = sqlite3WhereExprListUsage(pMaskSet, pExpr->x.pList);
    }
  }else if( op==TK_ISNULL ){
    pTerm->prereqRight = 0;
  }else{
    pTerm->prereqRight = sqlite3WhereExprUsage(pMaskSet, pExpr->pRight);
  }
  prereqAll = sqlite3WhereExprUsage(pMaskSet, pExpr);
  if( ExprHasProperty(pExpr, EP_FromJoin) ){
    Bitmask x = sqlite3WhereGetMask(pMaskSet, pExpr->iRightJoinTable);
    prereqAll |= x;
    extraRight = x-1;  /* ON clause terms may not be used with an index
                       ** on left table of a LEFT JOIN.  Ticket #3015 */
  }
  pTerm->prereqAll = prereqAll;
  pTerm->leftCursor = -1;
  pTerm->iParent = -1;
  pTerm->eOperator = 0;
  if( allowedOp(op) ){
    Expr *pLeft = sqlite3ExprSkipCollate(pExpr->pLeft);
    Expr *pRight = sqlite3ExprSkipCollate(pExpr->pRight);
    u16 opMask = (pTerm->prereqRight & prereqLeft)==0 ? WO_ALL : WO_EQUIV;
    if( pLeft->op==TK_COLUMN ){
      pTerm->leftCursor = pLeft->iTable;
      pTerm->u.leftColumn = pLeft->iColumn;
      pTerm->eOperator = operatorMask(op) & opMask;
    }
    if( op==TK_IS ) pTerm->wtFlags |= TERM_IS;
    if( pRight && pRight->op==TK_COLUMN ){
      WhereTerm *pNew;
      Expr *pDup;
      u16 eExtraOp = 0;        /* Extra bits for pNew->eOperator */
      if( pTerm->leftCursor>=0 ){
        int idxNew;
        pDup = sqlite3ExprDup(db, pExpr, 0);
        if( db->mallocFailed ){
          sqlite3ExprDelete(db, pDup);
          return;
        }
        idxNew = whereClauseInsert(pWC, pDup, TERM_VIRTUAL|TERM_DYNAMIC);
        if( idxNew==0 ) return;
        pNew = &pWC->a[idxNew];
        markTermAsChild(pWC, idxNew, idxTerm);
        if( op==TK_IS ) pNew->wtFlags |= TERM_IS;
        pTerm = &pWC->a[idxTerm];
        pTerm->wtFlags |= TERM_COPIED;

        if( termIsEquivalence(pParse, pDup) ){
          pTerm->eOperator |= WO_EQUIV;
          eExtraOp = WO_EQUIV;
        }
      }else{
        pDup = pExpr;
        pNew = pTerm;
      }
      exprCommute(pParse, pDup);
      pLeft = sqlite3ExprSkipCollate(pDup->pLeft);
      pNew->leftCursor = pLeft->iTable;
      pNew->u.leftColumn = pLeft->iColumn;
      testcase( (prereqLeft | extraRight) != prereqLeft );
      pNew->prereqRight = prereqLeft | extraRight;
      pNew->prereqAll = prereqAll;
      pNew->eOperator = (operatorMask(pDup->op) + eExtraOp) & opMask;
    }
  }

#ifndef SQLITE_OMIT_BETWEEN_OPTIMIZATION
  /* If a term is the BETWEEN operator, create two new virtual terms
  ** that define the range that the BETWEEN implements.  For example:
  **
  **      a BETWEEN b AND c
  **
  ** is converted into:
  **
  **      (a BETWEEN b AND c) AND (a>=b) AND (a<=c)
  **
  ** The two new terms are added onto the end of the WhereClause object.
  ** The new terms are "dynamic" and are children of the original BETWEEN
  ** term.  That means that if the BETWEEN term is coded, the children are
  ** skipped.  Or, if the children are satisfied by an index, the original
  ** BETWEEN term is skipped.
  */
  else if( pExpr->op==TK_BETWEEN && pWC->op==TK_AND ){
    ExprList *pList = pExpr->x.pList;
    int i;
    static const u8 ops[] = {TK_GE, TK_LE};
    assert( pList!=0 );
    assert( pList->nExpr==2 );
    for(i=0; i<2; i++){
      Expr *pNewExpr;
      int idxNew;
      pNewExpr = sqlite3PExpr(pParse, ops[i], 
                             sqlite3ExprDup(db, pExpr->pLeft, 0),
                             sqlite3ExprDup(db, pList->a[i].pExpr, 0), 0);
      transferJoinMarkings(pNewExpr, pExpr);
      idxNew = whereClauseInsert(pWC, pNewExpr, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      exprAnalyze(pSrc, pWC, idxNew);
      pTerm = &pWC->a[idxTerm];
      markTermAsChild(pWC, idxNew, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_BETWEEN_OPTIMIZATION */

#if !defined(SQLITE_OMIT_OR_OPTIMIZATION) && !defined(SQLITE_OMIT_SUBQUERY)
  /* Analyze a term that is composed of two or more subterms connected by
  ** an OR operator.
  */
  else if( pExpr->op==TK_OR ){
    assert( pWC->op==TK_AND );
    exprAnalyzeOrTerm(pSrc, pWC, idxTerm);
    pTerm = &pWC->a[idxTerm];
  }
#endif /* SQLITE_OMIT_OR_OPTIMIZATION */

#ifndef SQLITE_OMIT_LIKE_OPTIMIZATION
  /* Add constraints to reduce the search space on a LIKE or GLOB
  ** operator.
  **
  ** A like pattern of the form "x LIKE 'aBc%'" is changed into constraints
  **
  **          x>='ABC' AND x<'abd' AND x LIKE 'aBc%'
  **
  ** The last character of the prefix "abc" is incremented to form the
  ** termination condition "abd".  If case is not significant (the default
  ** for LIKE) then the lower-bound is made all uppercase and the upper-
  ** bound is made all lowercase so that the bounds also work when comparing
  ** BLOBs.
  */
  if( pWC->op==TK_AND 
   && isLikeOrGlob(pParse, pExpr, &pStr1, &isComplete, &noCase)
  ){
    Expr *pLeft;       /* LHS of LIKE/GLOB operator */
    Expr *pStr2;       /* Copy of pStr1 - RHS of LIKE/GLOB operator */
    Expr *pNewExpr1;
    Expr *pNewExpr2;
    int idxNew1;
    int idxNew2;
    const char *zCollSeqName;     /* Name of collating sequence */
    const u16 wtFlags = TERM_LIKEOPT | TERM_VIRTUAL | TERM_DYNAMIC;

    pLeft = pExpr->x.pList->a[1].pExpr;
    pStr2 = sqlite3ExprDup(db, pStr1, 0);

    /* Convert the lower bound to upper-case and the upper bound to
    ** lower-case (upper-case is less than lower-case in ASCII) so that
    ** the range constraints also work for BLOBs
    */
    if( noCase && !pParse->db->mallocFailed ){
      int i;
      char c;
      pTerm->wtFlags |= TERM_LIKE;
      for(i=0; (c = pStr1->u.zToken[i])!=0; i++){
        pStr1->u.zToken[i] = sqlite3Toupper(c);
        pStr2->u.zToken[i] = sqlite3Tolower(c);
      }
    }

    if( !db->mallocFailed ){
      u8 c, *pC;       /* Last character before the first wildcard */
      pC = (u8*)&pStr2->u.zToken[sqlite3Strlen30(pStr2->u.zToken)-1];
      c = *pC;
      if( noCase ){
        /* The point is to increment the last character before the first
        ** wildcard.  But if we increment '@', that will push it into the
        ** alphabetic range where case conversions will mess up the 
        ** inequality.  To avoid this, make sure to also run the full
        ** LIKE on all candidate expressions by clearing the isComplete flag
        */
        if( c=='A'-1 ) isComplete = 0;
        c = sqlite3UpperToLower[c];
      }
      *pC = c + 1;
    }
    zCollSeqName = noCase ? "NOCASE" : "BINARY";
    pNewExpr1 = sqlite3ExprDup(db, pLeft, 0);
    pNewExpr1 = sqlite3PExpr(pParse, TK_GE,
           sqlite3ExprAddCollateString(pParse,pNewExpr1,zCollSeqName),
           pStr1, 0);
    transferJoinMarkings(pNewExpr1, pExpr);
    idxNew1 = whereClauseInsert(pWC, pNewExpr1, wtFlags);
    testcase( idxNew1==0 );
    exprAnalyze(pSrc, pWC, idxNew1);
    pNewExpr2 = sqlite3ExprDup(db, pLeft, 0);
    pNewExpr2 = sqlite3PExpr(pParse, TK_LT,
           sqlite3ExprAddCollateString(pParse,pNewExpr2,zCollSeqName),
           pStr2, 0);
    transferJoinMarkings(pNewExpr2, pExpr);
    idxNew2 = whereClauseInsert(pWC, pNewExpr2, wtFlags);
    testcase( idxNew2==0 );
    exprAnalyze(pSrc, pWC, idxNew2);
    pTerm = &pWC->a[idxTerm];
    if( isComplete ){
      markTermAsChild(pWC, idxNew1, idxTerm);
      markTermAsChild(pWC, idxNew2, idxTerm);
    }
  }
#endif /* SQLITE_OMIT_LIKE_OPTIMIZATION */

#ifndef SQLITE_OMIT_VIRTUALTABLE
  /* Add a WO_MATCH auxiliary term to the constraint set if the
  ** current expression is of the form:  column MATCH expr.
  ** This information is used by the xBestIndex methods of
  ** virtual tables.  The native query optimizer does not attempt
  ** to do anything with MATCH functions.
  */
  if( isMatchOfColumn(pExpr) ){
    int idxNew;
    Expr *pRight, *pLeft;
    WhereTerm *pNewTerm;
    Bitmask prereqColumn, prereqExpr;

    pRight = pExpr->x.pList->a[0].pExpr;
    pLeft = pExpr->x.pList->a[1].pExpr;
    prereqExpr = sqlite3WhereExprUsage(pMaskSet, pRight);
    prereqColumn = sqlite3WhereExprUsage(pMaskSet, pLeft);
    if( (prereqExpr & prereqColumn)==0 ){
      Expr *pNewExpr;
      pNewExpr = sqlite3PExpr(pParse, TK_MATCH, 
                              0, sqlite3ExprDup(db, pRight, 0), 0);
      idxNew = whereClauseInsert(pWC, pNewExpr, TERM_VIRTUAL|TERM_DYNAMIC);
      testcase( idxNew==0 );
      pNewTerm = &pWC->a[idxNew];
      pNewTerm->prereqRight = prereqExpr;
      pNewTerm->leftCursor = pLeft->iTable;
      pNewTerm->u.leftColumn = pLeft->iColumn;
      pNewTerm->eOperator = WO_MATCH;
      markTermAsChild(pWC, idxNew, idxTerm);
      pTerm = &pWC->a[idxTerm];
      pTerm->wtFlags |= TERM_COPIED;
      pNewTerm->prereqAll = pTerm->prereqAll;
    }
  }
#endif /* SQLITE_OMIT_VIRTUALTABLE */

#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
  /* When sqlite_stat3 histogram data is available an operator of the
  ** form "x IS NOT NULL" can sometimes be evaluated more efficiently
  ** as "x>NULL" if x is not an INTEGER PRIMARY KEY.  So construct a
  ** virtual term of that form.
  **
  ** Note that the virtual term must be tagged with TERM_VNULL.
  */
  if( pExpr->op==TK_NOTNULL
   && pExpr->pLeft->op==TK_COLUMN
   && pExpr->pLeft->iColumn>=0
   && OptimizationEnabled(db, SQLITE_Stat34)
  ){
    Expr *pNewExpr;
    Expr *pLeft = pExpr->pLeft;
    int idxNew;
    WhereTerm *pNewTerm;

    pNewExpr = sqlite3PExpr(pParse, TK_GT,
                            sqlite3ExprDup(db, pLeft, 0),
                            sqlite3PExpr(pParse, TK_NULL, 0, 0, 0), 0);

    idxNew = whereClauseInsert(pWC, pNewExpr,
                              TERM_VIRTUAL|TERM_DYNAMIC|TERM_VNULL);
    if( idxNew ){
      pNewTerm = &pWC->a[idxNew];
      pNewTerm->prereqRight = 0;
      pNewTerm->leftCursor = pLeft->iTable;
      pNewTerm->u.leftColumn = pLeft->iColumn;
      pNewTerm->eOperator = WO_GT;
      markTermAsChild(pWC, idxNew, idxTerm);
      pTerm = &pWC->a[idxTerm];
      pTerm->wtFlags |= TERM_COPIED;
      pNewTerm->prereqAll = pTerm->prereqAll;
    }
  }
#endif /* SQLITE_ENABLE_STAT3_OR_STAT4 */

  /* Prevent ON clause terms of a LEFT JOIN from being used to drive
  ** an index for tables to the left of the join.
  */
  pTerm->prereqRight |= extraRight;
}

/***************************************************************************
** Routines with file scope above.  Interface to the rest of the where.c
** subsystem follows.
***************************************************************************/

/*
** This routine identifies subexpressions in the WHERE clause where
** each subexpression is separated by the AND operator or some other
** operator specified in the op parameter.  The WhereClause structure
** is filled with pointers to subexpressions.  For example:
**
**    WHERE  a=='hello' AND coalesce(b,11)<10 AND (c+12!=d OR c==22)
**           \________/     \_______________/     \________________/
**            slot[0]            slot[1]               slot[2]
**
** The original WHERE clause in pExpr is unaltered.  All this routine
** does is make slot[] entries point to substructure within pExpr.
**
** In the previous sentence and in the diagram, "slot[]" refers to
** the WhereClause.a[] array.  The slot[] array grows as needed to contain
** all terms of the WHERE clause.
*/
SQLITE_PRIVATE void sqlite3WhereSplit(WhereClause *pWC, Expr *pExpr, u8 op){
  Expr *pE2 = sqlite3ExprSkipCollate(pExpr);
  pWC->op = op;
  if( pE2==0 ) return;
  if( pE2->op!=op ){
    whereClauseInsert(pWC, pExpr, 0);
  }else{
    sqlite3WhereSplit(pWC, pE2->pLeft, op);
    sqlite3WhereSplit(pWC, pE2->pRight, op);
  }
}

/*
** Initialize a preallocated WhereClause structure.
*/
SQLITE_PRIVATE void sqlite3WhereClauseInit(
  WhereClause *pWC,        /* The WhereClause to be initialized */
  WhereInfo *pWInfo        /* The WHERE processing context */
){
  pWC->pWInfo = pWInfo;
  pWC->pOuter = 0;
  pWC->nTerm = 0;
  pWC->nSlot = ArraySize(pWC->aStatic);
  pWC->a = pWC->aStatic;
}

/*
** Deallocate a WhereClause structure.  The WhereClause structure
** itself is not freed.  This routine is the inverse of sqlite3WhereClauseInit().
*/
SQLITE_PRIVATE void sqlite3WhereClauseClear(WhereClause *pWC){
  int i;
  WhereTerm *a;
  sqlite3 *db = pWC->pWInfo->pParse->db;
  for(i=pWC->nTerm-1, a=pWC->a; i>=0; i--, a++){
    if( a->wtFlags & TERM_DYNAMIC ){
      sqlite3ExprDelete(db, a->pExpr);
    }
    if( a->wtFlags & TERM_ORINFO ){
      whereOrInfoDelete(db, a->u.pOrInfo);
    }else if( a->wtFlags & TERM_ANDINFO ){
      whereAndInfoDelete(db, a->u.pAndInfo);
    }
  }
  if( pWC->a!=pWC->aStatic ){
    sqlite3DbFree(db, pWC->a);
  }
}


/*
** These routines walk (recursively) an expression tree and generate
** a bitmask indicating which tables are used in that expression
** tree.
*/
SQLITE_PRIVATE Bitmask sqlite3WhereExprUsage(WhereMaskSet *pMaskSet, Expr *p){
  Bitmask mask = 0;
  if( p==0 ) return 0;
  if( p->op==TK_COLUMN ){
    mask = sqlite3WhereGetMask(pMaskSet, p->iTable);
    return mask;
  }
  mask = sqlite3WhereExprUsage(pMaskSet, p->pRight);
  mask |= sqlite3WhereExprUsage(pMaskSet, p->pLeft);
  if( ExprHasProperty(p, EP_xIsSelect) ){
    mask |= exprSelectUsage(pMaskSet, p->x.pSelect);
  }else{
    mask |= sqlite3WhereExprListUsage(pMaskSet, p->x.pList);
  }
  return mask;
}
SQLITE_PRIVATE Bitmask sqlite3WhereExprListUsage(WhereMaskSet *pMaskSet, ExprList *pList){
  int i;
  Bitmask mask = 0;
  if( pList ){
    for(i=0; i<pList->nExpr; i++){
      mask |= sqlite3WhereExprUsage(pMaskSet, pList->a[i].pExpr);
    }
  }
  return mask;
}


/*
** Call exprAnalyze on all terms in a WHERE clause.  
**
** Note that exprAnalyze() might add new virtual terms onto the
** end of the WHERE clause.  We do not want to analyze these new
** virtual terms, so start analyzing at the end and work forward
** so that the added virtual terms are never processed.
*/
SQLITE_PRIVATE void sqlite3WhereExprAnalyze(
  SrcList *pTabList,       /* the FROM clause */
  WhereClause *pWC         /* the WHERE clause to be analyzed */
){
  int i;
  for(i=pWC->nTerm-1; i>=0; i--){
    exprAnalyze(pTabList, pWC, i);
  }
}

/************** End of whereexpr.c *******************************************/
/************** Begin file where.c *******************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This module contains C code that generates VDBE code used to process
** the WHERE clause of SQL statements.  This module is responsible for
** generating the code that loops through a table looking for applicable
** rows.  Indices are selected and used to speed the search when doing
** so is applicable.  Because this module is responsible for selecting
** indices, you might also think of this module as the "query optimizer".
*/
/* #include "sqliteInt.h" */
/* #include "whereInt.h" */

/* Forward declaration of methods */
static int whereLoopResize(sqlite3*, WhereLoop*, int);

/* Test variable that can be set to enable WHERE tracing */
#if defined(SQLITE_TEST) || defined(SQLITE_DEBUG)
/***/ int sqlite3WhereTrace = 0;
#endif


/*
** Return the estimated number of output rows from a WHERE clause
*/
SQLITE_PRIVATE u64 sqlite3WhereOutputRowCount(WhereInfo *pWInfo){
  return sqlite3LogEstToInt(pWInfo->nRowOut);
}

/*
** Return one of the WHERE_DISTINCT_xxxxx values to indicate how this
** WHERE clause returns outputs for DISTINCT processing.
*/
SQLITE_PRIVATE int sqlite3WhereIsDistinct(WhereInfo *pWInfo){
  return pWInfo->eDistinct;
}

/*
** Return TRUE if the WHERE clause returns rows in ORDER BY order.
** Return FALSE if the output needs to be sorted.
*/
SQLITE_PRIVATE int sqlite3WhereIsOrdered(WhereInfo *pWInfo){
  return pWInfo->nOBSat;
}

/*
** Return the VDBE address or label to jump to in order to continue
** immediately with the next row of a WHERE clause.
*/
SQLITE_PRIVATE int sqlite3WhereContinueLabel(WhereInfo *pWInfo){
  assert( pWInfo->iContinue!=0 );
  return pWInfo->iContinue;
}

/*
** Return the VDBE address or label to jump to in order to break
** out of a WHERE loop.
*/
SQLITE_PRIVATE int sqlite3WhereBreakLabel(WhereInfo *pWInfo){
  return pWInfo->iBreak;
}

/*
** Return TRUE if an UPDATE or DELETE statement can operate directly on
** the rowids returned by a WHERE clause.  Return FALSE if doing an
** UPDATE or DELETE might change subsequent WHERE clause results.
**
** If the ONEPASS optimization is used (if this routine returns true)
** then also write the indices of open cursors used by ONEPASS
** into aiCur[0] and aiCur[1].  iaCur[0] gets the cursor of the data
** table and iaCur[1] gets the cursor used by an auxiliary index.
** Either value may be -1, indicating that cursor is not used.
** Any cursors returned will have been opened for writing.
**
** aiCur[0] and aiCur[1] both get -1 if the where-clause logic is
** unable to use the ONEPASS optimization.
*/
SQLITE_PRIVATE int sqlite3WhereOkOnePass(WhereInfo *pWInfo, int *aiCur){
  memcpy(aiCur, pWInfo->aiCurOnePass, sizeof(int)*2);
  return pWInfo->okOnePass;
}

/*
** Move the content of pSrc into pDest
*/
static void whereOrMove(WhereOrSet *pDest, WhereOrSet *pSrc){
  pDest->n = pSrc->n;
  memcpy(pDest->a, pSrc->a, pDest->n*sizeof(pDest->a[0]));
}

/*
** Try to insert a new prerequisite/cost entry into the WhereOrSet pSet.
**
** The new entry might overwrite an existing entry, or it might be
** appended, or it might be discarded.  Do whatever is the right thing
** so that pSet keeps the N_OR_COST best entries seen so far.
*/
static int whereOrInsert(
  WhereOrSet *pSet,      /* The WhereOrSet to be updated */
  Bitmask prereq,        /* Prerequisites of the new entry */
  LogEst rRun,           /* Run-cost of the new entry */
  LogEst nOut            /* Number of outputs for the new entry */
){
  u16 i;
  WhereOrCost *p;
  for(i=pSet->n, p=pSet->a; i>0; i--, p++){
    if( rRun<=p->rRun && (prereq & p->prereq)==prereq ){
      goto whereOrInsert_done;
    }
    if( p->rRun<=rRun && (p->prereq & prereq)==p->prereq ){
      return 0;
    }
  }
  if( pSet->n<N_OR_COST ){
    p = &pSet->a[pSet->n++];
    p->nOut = nOut;
  }else{
    p = pSet->a;
    for(i=1; i<pSet->n; i++){
      if( p->rRun>pSet->a[i].rRun ) p = pSet->a + i;
    }
    if( p->rRun<=rRun ) return 0;
  }
whereOrInsert_done:
  p->prereq = prereq;
  p->rRun = rRun;
  if( p->nOut>nOut ) p->nOut = nOut;
  return 1;
}

/*
** Return the bitmask for the given cursor number.  Return 0 if
** iCursor is not in the set.
*/
SQLITE_PRIVATE Bitmask sqlite3WhereGetMask(WhereMaskSet *pMaskSet, int iCursor){
  int i;
  assert( pMaskSet->n<=(int)sizeof(Bitmask)*8 );
  for(i=0; i<pMaskSet->n; i++){
    if( pMaskSet->ix[i]==iCursor ){
      return MASKBIT(i);
    }
  }
  return 0;
}

/*
** Create a new mask for cursor iCursor.
**
** There is one cursor per table in the FROM clause.  The number of
** tables in the FROM clause is limited by a test early in the
** sqlite3WhereBegin() routine.  So we know that the pMaskSet->ix[]
** array will never overflow.
*/
static void createMask(WhereMaskSet *pMaskSet, int iCursor){
  assert( pMaskSet->n < ArraySize(pMaskSet->ix) );
  pMaskSet->ix[pMaskSet->n++] = iCursor;
}

/*
** Advance to the next WhereTerm that matches according to the criteria
** established when the pScan object was initialized by whereScanInit().
** Return NULL if there are no more matching WhereTerms.
*/
static WhereTerm *whereScanNext(WhereScan *pScan){
  int iCur;            /* The cursor on the LHS of the term */
  int iColumn;         /* The column on the LHS of the term.  -1 for IPK */
  Expr *pX;            /* An expression being tested */
  WhereClause *pWC;    /* Shorthand for pScan->pWC */
  WhereTerm *pTerm;    /* The term being tested */
  int k = pScan->k;    /* Where to start scanning */

  while( pScan->iEquiv<=pScan->nEquiv ){
    iCur = pScan->aEquiv[pScan->iEquiv-2];
    iColumn = pScan->aEquiv[pScan->iEquiv-1];
    while( (pWC = pScan->pWC)!=0 ){
      for(pTerm=pWC->a+k; k<pWC->nTerm; k++, pTerm++){
        if( pTerm->leftCursor==iCur
         && pTerm->u.leftColumn==iColumn
         && (pScan->iEquiv<=2 || !ExprHasProperty(pTerm->pExpr, EP_FromJoin))
        ){
          if( (pTerm->eOperator & WO_EQUIV)!=0
           && pScan->nEquiv<ArraySize(pScan->aEquiv)
          ){
            int j;
            pX = sqlite3ExprSkipCollate(pTerm->pExpr->pRight);
            assert( pX->op==TK_COLUMN );
            for(j=0; j<pScan->nEquiv; j+=2){
              if( pScan->aEquiv[j]==pX->iTable
               && pScan->aEquiv[j+1]==pX->iColumn ){
                  break;
              }
            }
            if( j==pScan->nEquiv ){
              pScan->aEquiv[j] = pX->iTable;
              pScan->aEquiv[j+1] = pX->iColumn;
              pScan->nEquiv += 2;
            }
          }
          if( (pTerm->eOperator & pScan->opMask)!=0 ){
            /* Verify the affinity and collating sequence match */
            if( pScan->zCollName && (pTerm->eOperator & WO_ISNULL)==0 ){
              CollSeq *pColl;
              Parse *pParse = pWC->pWInfo->pParse;
              pX = pTerm->pExpr;
              if( !sqlite3IndexAffinityOk(pX, pScan->idxaff) ){
                continue;
              }
              assert(pX->pLeft);
              pColl = sqlite3BinaryCompareCollSeq(pParse,
                                                  pX->pLeft, pX->pRight);
              if( pColl==0 ) pColl = pParse->db->pDfltColl;
              if( sqlite3StrICmp(pColl->zName, pScan->zCollName) ){
                continue;
              }
            }
            if( (pTerm->eOperator & (WO_EQ|WO_IS))!=0
             && (pX = pTerm->pExpr->pRight)->op==TK_COLUMN
             && pX->iTable==pScan->aEquiv[0]
             && pX->iColumn==pScan->aEquiv[1]
            ){
              testcase( pTerm->eOperator & WO_IS );
              continue;
            }
            pScan->k = k+1;
            return pTerm;
          }
        }
      }
      pScan->pWC = pScan->pWC->pOuter;
      k = 0;
    }
    pScan->pWC = pScan->pOrigWC;
    k = 0;
    pScan->iEquiv += 2;
  }
  return 0;
}

/*
** Initialize a WHERE clause scanner object.  Return a pointer to the
** first match.  Return NULL if there are no matches.
**
** The scanner will be searching the WHERE clause pWC.  It will look
** for terms of the form "X <op> <expr>" where X is column iColumn of table
** iCur.  The <op> must be one of the operators described by opMask.
**
** If the search is for X and the WHERE clause contains terms of the
** form X=Y then this routine might also return terms of the form
** "Y <op> <expr>".  The number of levels of transitivity is limited,
** but is enough to handle most commonly occurring SQL statements.
**
** If X is not the INTEGER PRIMARY KEY then X must be compatible with
** index pIdx.
*/
static WhereTerm *whereScanInit(
  WhereScan *pScan,       /* The WhereScan object being initialized */
  WhereClause *pWC,       /* The WHERE clause to be scanned */
  int iCur,               /* Cursor to scan for */
  int iColumn,            /* Column to scan for */
  u32 opMask,             /* Operator(s) to scan for */
  Index *pIdx             /* Must be compatible with this index */
){
  int j;

  /* memset(pScan, 0, sizeof(*pScan)); */
  pScan->pOrigWC = pWC;
  pScan->pWC = pWC;
  if( pIdx && iColumn>=0 ){
    pScan->idxaff = pIdx->pTable->aCol[iColumn].affinity;
    for(j=0; pIdx->aiColumn[j]!=iColumn; j++){
      if( NEVER(j>pIdx->nColumn) ) return 0;
    }
    pScan->zCollName = pIdx->azColl[j];
  }else{
    pScan->idxaff = 0;
    pScan->zCollName = 0;
  }
  pScan->opMask = opMask;
  pScan->k = 0;
  pScan->aEquiv[0] = iCur;
  pScan->aEquiv[1] = iColumn;
  pScan->nEquiv = 2;
  pScan->iEquiv = 2;
  return whereScanNext(pScan);
}

/*
** Search for a term in the WHERE clause that is of the form "X <op> <expr>"
** where X is a reference to the iColumn of table iCur and <op> is one of
** the WO_xx operator codes specified by the op parameter.
** Return a pointer to the term.  Return 0 if not found.
**
** The term returned might by Y=<expr> if there is another constraint in
** the WHERE clause that specifies that X=Y.  Any such constraints will be
** identified by the WO_EQUIV bit in the pTerm->eOperator field.  The
** aEquiv[] array holds X and all its equivalents, with each SQL variable
** taking up two slots in aEquiv[].  The first slot is for the cursor number
** and the second is for the column number.  There are 22 slots in aEquiv[]
** so that means we can look for X plus up to 10 other equivalent values.
** Hence a search for X will return <expr> if X=A1 and A1=A2 and A2=A3
** and ... and A9=A10 and A10=<expr>.
**
** If there are multiple terms in the WHERE clause of the form "X <op> <expr>"
** then try for the one with no dependencies on <expr> - in other words where
** <expr> is a constant expression of some kind.  Only return entries of
** the form "X <op> Y" where Y is a column in another table if no terms of
** the form "X <op> <const-expr>" exist.   If no terms with a constant RHS
** exist, try to return a term that does not use WO_EQUIV.
*/
SQLITE_PRIVATE WhereTerm *sqlite3WhereFindTerm(
  WhereClause *pWC,     /* The WHERE clause to be searched */
  int iCur,             /* Cursor number of LHS */
  int iColumn,          /* Column number of LHS */
  Bitmask notReady,     /* RHS must not overlap with this mask */
  u32 op,               /* Mask of WO_xx values describing operator */
  Index *pIdx           /* Must be compatible with this index, if not NULL */
){
  WhereTerm *pResult = 0;
  WhereTerm *p;
  WhereScan scan;

  p = whereScanInit(&scan, pWC, iCur, iColumn, op, pIdx);
  op &= WO_EQ|WO_IS;
  while( p ){
    if( (p->prereqRight & notReady)==0 ){
      if( p->prereqRight==0 && (p->eOperator&op)!=0 ){
        testcase( p->eOperator & WO_IS );
        return p;
      }
      if( pResult==0 ) pResult = p;
    }
    p = whereScanNext(&scan);
  }
  return pResult;
}

/*
** This function searches pList for an entry that matches the iCol-th column
** of index pIdx.
**
** If such an expression is found, its index in pList->a[] is returned. If
** no expression is found, -1 is returned.
*/
static int findIndexCol(
  Parse *pParse,                  /* Parse context */
  ExprList *pList,                /* Expression list to search */
  int iBase,                      /* Cursor for table associated with pIdx */
  Index *pIdx,                    /* Index to match column of */
  int iCol                        /* Column of index to match */
){
  int i;
  const char *zColl = pIdx->azColl[iCol];

  for(i=0; i<pList->nExpr; i++){
    Expr *p = sqlite3ExprSkipCollate(pList->a[i].pExpr);
    if( p->op==TK_COLUMN
     && p->iColumn==pIdx->aiColumn[iCol]
     && p->iTable==iBase
    ){
      CollSeq *pColl = sqlite3ExprCollSeq(pParse, pList->a[i].pExpr);
      if( pColl && 0==sqlite3StrICmp(pColl->zName, zColl) ){
        return i;
      }
    }
  }

  return -1;
}

/*
** Return true if the DISTINCT expression-list passed as the third argument
** is redundant.
**
** A DISTINCT list is redundant if any subset of the columns in the
** DISTINCT list are collectively unique and individually non-null.
*/
static int isDistinctRedundant(
  Parse *pParse,            /* Parsing context */
  SrcList *pTabList,        /* The FROM clause */
  WhereClause *pWC,         /* The WHERE clause */
  ExprList *pDistinct       /* The result set that needs to be DISTINCT */
){
  Table *pTab;
  Index *pIdx;
  int i;                          
  int iBase;

  /* If there is more than one table or sub-select in the FROM clause of
  ** this query, then it will not be possible to show that the DISTINCT 
  ** clause is redundant. */
  if( pTabList->nSrc!=1 ) return 0;
  iBase = pTabList->a[0].iCursor;
  pTab = pTabList->a[0].pTab;

  /* If any of the expressions is an IPK column on table iBase, then return 
  ** true. Note: The (p->iTable==iBase) part of this test may be false if the
  ** current SELECT is a correlated sub-query.
  */
  for(i=0; i<pDistinct->nExpr; i++){
    Expr *p = sqlite3ExprSkipCollate(pDistinct->a[i].pExpr);
    if( p->op==TK_COLUMN && p->iTable==iBase && p->iColumn<0 ) return 1;
  }

  /* Loop through all indices on the table, checking each to see if it makes
  ** the DISTINCT qualifier redundant. It does so if:
  **
  **   1. The index is itself UNIQUE, and
  **
  **   2. All of the columns in the index are either part of the pDistinct
  **      list, or else the WHERE clause contains a term of the form "col=X",
  **      where X is a constant value. The collation sequences of the
  **      comparison and select-list expressions must match those of the index.
  **
  **   3. All of those index columns for which the WHERE clause does not
  **      contain a "col=X" term are subject to a NOT NULL constraint.
  */
  for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
    if( !IsUniqueIndex(pIdx) ) continue;
    for(i=0; i<pIdx->nKeyCol; i++){
      i16 iCol = pIdx->aiColumn[i];
      if( 0==sqlite3WhereFindTerm(pWC, iBase, iCol, ~(Bitmask)0, WO_EQ, pIdx) ){
        int iIdxCol = findIndexCol(pParse, pDistinct, iBase, pIdx, i);
        if( iIdxCol<0 || pTab->aCol[iCol].notNull==0 ){
          break;
        }
      }
    }
    if( i==pIdx->nKeyCol ){
      /* This index implies that the DISTINCT qualifier is redundant. */
      return 1;
    }
  }

  return 0;
}


/*
** Estimate the logarithm of the input value to base 2.
*/
static LogEst estLog(LogEst N){
  return N<=10 ? 0 : sqlite3LogEst(N) - 33;
}

/*
** Convert OP_Column opcodes to OP_Copy in previously generated code.
**
** This routine runs over generated VDBE code and translates OP_Column
** opcodes into OP_Copy, and OP_Rowid into OP_Null, when the table is being
** accessed via co-routine instead of via table lookup.
*/
static void translateColumnToCopy(
  Vdbe *v,            /* The VDBE containing code to translate */
  int iStart,         /* Translate from this opcode to the end */
  int iTabCur,        /* OP_Column/OP_Rowid references to this table */
  int iRegister       /* The first column is in this register */
){
  VdbeOp *pOp = sqlite3VdbeGetOp(v, iStart);
  int iEnd = sqlite3VdbeCurrentAddr(v);
  for(; iStart<iEnd; iStart++, pOp++){
    if( pOp->p1!=iTabCur ) continue;
    if( pOp->opcode==OP_Column ){
      pOp->opcode = OP_Copy;
      pOp->p1 = pOp->p2 + iRegister;
      pOp->p2 = pOp->p3;
      pOp->p3 = 0;
    }else if( pOp->opcode==OP_Rowid ){
      pOp->opcode = OP_Null;
      pOp->p1 = 0;
      pOp->p3 = 0;
    }
  }
}

/*
** Two routines for printing the content of an sqlite3_index_info
** structure.  Used for testing and debugging only.  If neither
** SQLITE_TEST or SQLITE_DEBUG are defined, then these routines
** are no-ops.
*/
#if !defined(SQLITE_OMIT_VIRTUALTABLE) && defined(WHERETRACE_ENABLED)
static void TRACE_IDX_INPUTS(sqlite3_index_info *p){
  int i;
  if( !sqlite3WhereTrace ) return;
  for(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  constraint[%d]: col=%d termid=%d op=%d usabled=%d\n",
       i,
       p->aConstraint[i].iColumn,
       p->aConstraint[i].iTermOffset,
       p->aConstraint[i].op,
       p->aConstraint[i].usable);
  }
  for(i=0; i<p->nOrderBy; i++){
    sqlite3DebugPrintf("  orderby[%d]: col=%d desc=%d\n",
       i,
       p->aOrderBy[i].iColumn,
       p->aOrderBy[i].desc);
  }
}
static void TRACE_IDX_OUTPUTS(sqlite3_index_info *p){
  int i;
  if( !sqlite3WhereTrace ) return;
  for(i=0; i<p->nConstraint; i++){
    sqlite3DebugPrintf("  usage[%d]: argvIdx=%d omit=%d\n",
       i,
       p->aConstraintUsage[i].argvIndex,
       p->aConstraintUsage[i].omit);
  }
  sqlite3DebugPrintf("  idxNum=%d\n", p->idxNum);
  sqlite3DebugPrintf("  idxStr=%s\n", p->idxStr);
  sqlite3DebugPrintf("  orderByConsumed=%d\n", p->orderByConsumed);
  sqlite3DebugPrintf("  estimatedCost=%g\n", p->estimatedCost);
  sqlite3DebugPrintf("  estimatedRows=%lld\n", p->estimatedRows);
}
#else
#define TRACE_IDX_INPUTS(A)
#define TRACE_IDX_OUTPUTS(A)
#endif

#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
/*
** Return TRUE if the WHERE clause term pTerm is of a form where it
** could be used with an index to access pSrc, assuming an appropriate
** index existed.
*/
static int termCanDriveIndex(
  WhereTerm *pTerm,              /* WHERE clause term to check */
  struct SrcList_item *pSrc,     /* Table we are trying to access */
  Bitmask notReady               /* Tables in outer loops of the join */
){
  char aff;
  if( pTerm->leftCursor!=pSrc->iCursor ) return 0;
  if( (pTerm->eOperator & (WO_EQ|WO_IS))==0 ) return 0;
  if( (pTerm->prereqRight & notReady)!=0 ) return 0;
  if( pTerm->u.leftColumn<0 ) return 0;
  aff = pSrc->pTab->aCol[pTerm->u.leftColumn].affinity;
  if( !sqlite3IndexAffinityOk(pTerm->pExpr, aff) ) return 0;
  testcase( pTerm->pExpr->op==TK_IS );
  return 1;
}
#endif


#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
/*
** Generate code to construct the Index object for an automatic index
** and to set up the WhereLevel object pLevel so that the code generator
** makes use of the automatic index.
*/
static void constructAutomaticIndex(
  Parse *pParse,              /* The parsing context */
  WhereClause *pWC,           /* The WHERE clause */
  struct SrcList_item *pSrc,  /* The FROM clause term to get the next index */
  Bitmask notReady,           /* Mask of cursors that are not available */
  WhereLevel *pLevel          /* Write new index here */
){
  int nKeyCol;                /* Number of columns in the constructed index */
  WhereTerm *pTerm;           /* A single term of the WHERE clause */
  WhereTerm *pWCEnd;          /* End of pWC->a[] */
  Index *pIdx;                /* Object describing the transient index */
  Vdbe *v;                    /* Prepared statement under construction */
  int addrInit;               /* Address of the initialization bypass jump */
  Table *pTable;              /* The table being indexed */
  int addrTop;                /* Top of the index fill loop */
  int regRecord;              /* Register holding an index record */
  int n;                      /* Column counter */
  int i;                      /* Loop counter */
  int mxBitCol;               /* Maximum column in pSrc->colUsed */
  CollSeq *pColl;             /* Collating sequence to on a column */
  WhereLoop *pLoop;           /* The Loop object */
  char *zNotUsed;             /* Extra space on the end of pIdx */
  Bitmask idxCols;            /* Bitmap of columns used for indexing */
  Bitmask extraCols;          /* Bitmap of additional columns */
  u8 sentWarning = 0;         /* True if a warnning has been issued */
  Expr *pPartial = 0;         /* Partial Index Expression */
  int iContinue = 0;          /* Jump here to skip excluded rows */
  struct SrcList_item *pTabItem;  /* FROM clause term being indexed */

  /* Generate code to skip over the creation and initialization of the
  ** transient index on 2nd and subsequent iterations of the loop. */
  v = pParse->pVdbe;
  assert( v!=0 );
  addrInit = sqlite3CodeOnce(pParse); VdbeCoverage(v);

  /* Count the number of columns that will be added to the index
  ** and used to match WHERE clause constraints */
  nKeyCol = 0;
  pTable = pSrc->pTab;
  pWCEnd = &pWC->a[pWC->nTerm];
  pLoop = pLevel->pWLoop;
  idxCols = 0;
  for(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    Expr *pExpr = pTerm->pExpr;
    assert( !ExprHasProperty(pExpr, EP_FromJoin)    /* prereq always non-zero */
         || pExpr->iRightJoinTable!=pSrc->iCursor   /*   for the right-hand   */
         || pLoop->prereq!=0 );                     /*   table of a LEFT JOIN */
    if( pLoop->prereq==0
     && (pTerm->wtFlags & TERM_VIRTUAL)==0
     && !ExprHasProperty(pExpr, EP_FromJoin)
     && sqlite3ExprIsTableConstant(pExpr, pSrc->iCursor) ){
      pPartial = sqlite3ExprAnd(pParse->db, pPartial,
                                sqlite3ExprDup(pParse->db, pExpr, 0));
    }
    if( termCanDriveIndex(pTerm, pSrc, notReady) ){
      int iCol = pTerm->u.leftColumn;
      Bitmask cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS );
      testcase( iCol==BMS-1 );
      if( !sentWarning ){
        sqlite3_log(SQLITE_WARNING_AUTOINDEX,
            "automatic index on %s(%s)", pTable->zName,
            pTable->aCol[iCol].zName);
        sentWarning = 1;
      }
      if( (idxCols & cMask)==0 ){
        if( whereLoopResize(pParse->db, pLoop, nKeyCol+1) ){
          goto end_auto_index_create;
        }
        pLoop->aLTerm[nKeyCol++] = pTerm;
        idxCols |= cMask;
      }
    }
  }
  assert( nKeyCol>0 );
  pLoop->u.btree.nEq = pLoop->nLTerm = nKeyCol;
  pLoop->wsFlags = WHERE_COLUMN_EQ | WHERE_IDX_ONLY | WHERE_INDEXED
                     | WHERE_AUTO_INDEX;

  /* Count the number of additional columns needed to create a
  ** covering index.  A "covering index" is an index that contains all
  ** columns that are needed by the query.  With a covering index, the
  ** original table never needs to be accessed.  Automatic indices must
  ** be a covering index because the index will not be updated if the
  ** original table changes and the index and table cannot both be used
  ** if they go out of sync.
  */
  extraCols = pSrc->colUsed & (~idxCols | MASKBIT(BMS-1));
  mxBitCol = MIN(BMS-1,pTable->nCol);
  testcase( pTable->nCol==BMS-1 );
  testcase( pTable->nCol==BMS-2 );
  for(i=0; i<mxBitCol; i++){
    if( extraCols & MASKBIT(i) ) nKeyCol++;
  }
  if( pSrc->colUsed & MASKBIT(BMS-1) ){
    nKeyCol += pTable->nCol - BMS + 1;
  }

  /* Construct the Index object to describe this index */
  pIdx = sqlite3AllocateIndexObject(pParse->db, nKeyCol+1, 0, &zNotUsed);
  if( pIdx==0 ) goto end_auto_index_create;
  pLoop->u.btree.pIndex = pIdx;
  pIdx->zName = "auto-index";
  pIdx->pTable = pTable;
  n = 0;
  idxCols = 0;
  for(pTerm=pWC->a; pTerm<pWCEnd; pTerm++){
    if( termCanDriveIndex(pTerm, pSrc, notReady) ){
      int iCol = pTerm->u.leftColumn;
      Bitmask cMask = iCol>=BMS ? MASKBIT(BMS-1) : MASKBIT(iCol);
      testcase( iCol==BMS-1 );
      testcase( iCol==BMS );
      if( (idxCols & cMask)==0 ){
        Expr *pX = pTerm->pExpr;
        idxCols |= cMask;
        pIdx->aiColumn[n] = pTerm->u.leftColumn;
        pColl = sqlite3BinaryCompareCollSeq(pParse, pX->pLeft, pX->pRight);
        pIdx->azColl[n] = pColl ? pColl->zName : "BINARY";
        n++;
      }
    }
  }
  assert( (u32)n==pLoop->u.btree.nEq );

  /* Add additional columns needed to make the automatic index into
  ** a covering index */
  for(i=0; i<mxBitCol; i++){
    if( extraCols & MASKBIT(i) ){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = "BINARY";
      n++;
    }
  }
  if( pSrc->colUsed & MASKBIT(BMS-1) ){
    for(i=BMS-1; i<pTable->nCol; i++){
      pIdx->aiColumn[n] = i;
      pIdx->azColl[n] = "BINARY";
      n++;
    }
  }
  assert( n==nKeyCol );
  pIdx->aiColumn[n] = -1;
  pIdx->azColl[n] = "BINARY";

  /* Create the automatic index */
  assert( pLevel->iIdxCur>=0 );
  pLevel->iIdxCur = pParse->nTab++;
  sqlite3VdbeAddOp2(v, OP_OpenAutoindex, pLevel->iIdxCur, nKeyCol+1);
  sqlite3VdbeSetP4KeyInfo(pParse, pIdx);
  VdbeComment((v, "for %s", pTable->zName));

  /* Fill the automatic index with content */
  sqlite3ExprCachePush(pParse);
  pTabItem = &pWC->pWInfo->pTabList->a[pLevel->iFrom];
  if( pTabItem->viaCoroutine ){
    int regYield = pTabItem->regReturn;
    sqlite3VdbeAddOp3(v, OP_InitCoroutine, regYield, 0, pTabItem->addrFillSub);
    addrTop =  sqlite3VdbeAddOp1(v, OP_Yield, regYield);
    VdbeCoverage(v);
    VdbeComment((v, "next row of \"%s\"", pTabItem->pTab->zName));
  }else{
    addrTop = sqlite3VdbeAddOp1(v, OP_Rewind, pLevel->iTabCur); VdbeCoverage(v);
  }
  if( pPartial ){
    iContinue = sqlite3VdbeMakeLabel(v);
    sqlite3ExprIfFalse(pParse, pPartial, iContinue, SQLITE_JUMPIFNULL);
    pLoop->wsFlags |= WHERE_PARTIALIDX;
  }
  regRecord = sqlite3GetTempReg(pParse);
  sqlite3GenerateIndexKey(pParse, pIdx, pLevel->iTabCur, regRecord, 0, 0, 0, 0);
  sqlite3VdbeAddOp2(v, OP_IdxInsert, pLevel->iIdxCur, regRecord);
  sqlite3VdbeChangeP5(v, OPFLAG_USESEEKRESULT);
  if( pPartial ) sqlite3VdbeResolveLabel(v, iContinue);
  if( pTabItem->viaCoroutine ){
    translateColumnToCopy(v, addrTop, pLevel->iTabCur, pTabItem->regResult);
    sqlite3VdbeAddOp2(v, OP_Goto, 0, addrTop);
    pTabItem->viaCoroutine = 0;
  }else{
    sqlite3VdbeAddOp2(v, OP_Next, pLevel->iTabCur, addrTop+1); VdbeCoverage(v);
  }
  sqlite3VdbeChangeP5(v, SQLITE_STMTSTATUS_AUTOINDEX);
  sqlite3VdbeJumpHere(v, addrTop);
  sqlite3ReleaseTempReg(pParse, regRecord);
  sqlite3ExprCachePop(pParse);
  
  /* Jump here when skipping the initialization */
  sqlite3VdbeJumpHere(v, addrInit);

end_auto_index_create:
  sqlite3ExprDelete(pParse->db, pPartial);
}
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */

#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Allocate and populate an sqlite3_index_info structure. It is the 
** responsibility of the caller to eventually release the structure
** by passing the pointer returned by this function to sqlite3_free().
*/
static sqlite3_index_info *allocateIndexInfo(
  Parse *pParse,
  WhereClause *pWC,
  Bitmask mUnusable,              /* Ignore terms with these prereqs */
  struct SrcList_item *pSrc,
  ExprList *pOrderBy
){
  int i, j;
  int nTerm;
  struct sqlite3_index_constraint *pIdxCons;
  struct sqlite3_index_orderby *pIdxOrderBy;
  struct sqlite3_index_constraint_usage *pUsage;
  WhereTerm *pTerm;
  int nOrderBy;
  sqlite3_index_info *pIdxInfo;

  /* Count the number of possible WHERE clause constraints referring
  ** to this virtual table */
  for(i=nTerm=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    if( pTerm->leftCursor != pSrc->iCursor ) continue;
    if( pTerm->prereqRight & mUnusable ) continue;
    assert( IsPowerOfTwo(pTerm->eOperator & ~WO_EQUIV) );
    testcase( pTerm->eOperator & WO_IN );
    testcase( pTerm->eOperator & WO_ISNULL );
    testcase( pTerm->eOperator & WO_IS );
    testcase( pTerm->eOperator & WO_ALL );
    if( (pTerm->eOperator & ~(WO_ISNULL|WO_EQUIV|WO_IS))==0 ) continue;
    if( pTerm->wtFlags & TERM_VNULL ) continue;
    nTerm++;
  }

  /* If the ORDER BY clause contains only columns in the current 
  ** virtual table then allocate space for the aOrderBy part of
  ** the sqlite3_index_info structure.
  */
  nOrderBy = 0;
  if( pOrderBy ){
    int n = pOrderBy->nExpr;
    for(i=0; i<n; i++){
      Expr *pExpr = pOrderBy->a[i].pExpr;
      if( pExpr->op!=TK_COLUMN || pExpr->iTable!=pSrc->iCursor ) break;
    }
    if( i==n){
      nOrderBy = n;
    }
  }

  /* Allocate the sqlite3_index_info structure
  */
  pIdxInfo = sqlite3DbMallocZero(pParse->db, sizeof(*pIdxInfo)
                           + (sizeof(*pIdxCons) + sizeof(*pUsage))*nTerm
                           + sizeof(*pIdxOrderBy)*nOrderBy );
  if( pIdxInfo==0 ){
    sqlite3ErrorMsg(pParse, "out of memory");
    return 0;
  }

  /* Initialize the structure.  The sqlite3_index_info structure contains
  ** many fields that are declared "const" to prevent xBestIndex from
  ** changing them.  We have to do some funky casting in order to
  ** initialize those fields.
  */
  pIdxCons = (struct sqlite3_index_constraint*)&pIdxInfo[1];
  pIdxOrderBy = (struct sqlite3_index_orderby*)&pIdxCons[nTerm];
  pUsage = (struct sqlite3_index_constraint_usage*)&pIdxOrderBy[nOrderBy];
  *(int*)&pIdxInfo->nConstraint = nTerm;
  *(int*)&pIdxInfo->nOrderBy = nOrderBy;
  *(struct sqlite3_index_constraint**)&pIdxInfo->aConstraint = pIdxCons;
  *(struct sqlite3_index_orderby**)&pIdxInfo->aOrderBy = pIdxOrderBy;
  *(struct sqlite3_index_constraint_usage**)&pIdxInfo->aConstraintUsage =
                                                                   pUsage;

  for(i=j=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    u8 op;
    if( pTerm->leftCursor != pSrc->iCursor ) continue;
    if( pTerm->prereqRight & mUnusable ) continue;
    assert( IsPowerOfTwo(pTerm->eOperator & ~WO_EQUIV) );
    testcase( pTerm->eOperator & WO_IN );
    testcase( pTerm->eOperator & WO_IS );
    testcase( pTerm->eOperator & WO_ISNULL );
    testcase( pTerm->eOperator & WO_ALL );
    if( (pTerm->eOperator & ~(WO_ISNULL|WO_EQUIV|WO_IS))==0 ) continue;
    if( pTerm->wtFlags & TERM_VNULL ) continue;
    pIdxCons[j].iColumn = pTerm->u.leftColumn;
    pIdxCons[j].iTermOffset = i;
    op = (u8)pTerm->eOperator & WO_ALL;
    if( op==WO_IN ) op = WO_EQ;
    pIdxCons[j].op = op;
    /* The direct assignment in the previous line is possible only because
    ** the WO_ and SQLITE_INDEX_CONSTRAINT_ codes are identical.  The
    ** following asserts verify this fact. */
    assert( WO_EQ==SQLITE_INDEX_CONSTRAINT_EQ );
    assert( WO_LT==SQLITE_INDEX_CONSTRAINT_LT );
    assert( WO_LE==SQLITE_INDEX_CONSTRAINT_LE );
    assert( WO_GT==SQLITE_INDEX_CONSTRAINT_GT );
    assert( WO_GE==SQLITE_INDEX_CONSTRAINT_GE );
    assert( WO_MATCH==SQLITE_INDEX_CONSTRAINT_MATCH );
    assert( pTerm->eOperator & (WO_IN|WO_EQ|WO_LT|WO_LE|WO_GT|WO_GE|WO_MATCH) );
    j++;
  }
  for(i=0; i<nOrderBy; i++){
    Expr *pExpr = pOrderBy->a[i].pExpr;
    pIdxOrderBy[i].iColumn = pExpr->iColumn;
    pIdxOrderBy[i].desc = pOrderBy->a[i].sortOrder;
  }

  return pIdxInfo;
}

/*
** The table object reference passed as the second argument to this function
** must represent a virtual table. This function invokes the xBestIndex()
** method of the virtual table with the sqlite3_index_info object that
** comes in as the 3rd argument to this function.
**
** If an error occurs, pParse is populated with an error message and a
** non-zero value is returned. Otherwise, 0 is returned and the output
** part of the sqlite3_index_info structure is left populated.
**
** Whether or not an error is returned, it is the responsibility of the
** caller to eventually free p->idxStr if p->needToFreeIdxStr indicates
** that this is required.
*/
static int vtabBestIndex(Parse *pParse, Table *pTab, sqlite3_index_info *p){
  sqlite3_vtab *pVtab = sqlite3GetVTable(pParse->db, pTab)->pVtab;
  int i;
  int rc;

  TRACE_IDX_INPUTS(p);
  rc = pVtab->pModule->xBestIndex(pVtab, p);
  TRACE_IDX_OUTPUTS(p);

  if( rc!=SQLITE_OK ){
    if( rc==SQLITE_NOMEM ){
      pParse->db->mallocFailed = 1;
    }else if( !pVtab->zErrMsg ){
      sqlite3ErrorMsg(pParse, "%s", sqlite3ErrStr(rc));
    }else{
      sqlite3ErrorMsg(pParse, "%s", pVtab->zErrMsg);
    }
  }
  sqlite3_free(pVtab->zErrMsg);
  pVtab->zErrMsg = 0;

  for(i=0; i<p->nConstraint; i++){
    if( !p->aConstraint[i].usable && p->aConstraintUsage[i].argvIndex>0 ){
      sqlite3ErrorMsg(pParse, 
          "table %s: xBestIndex returned an invalid plan", pTab->zName);
    }
  }

  return pParse->nErr;
}
#endif /* !defined(SQLITE_OMIT_VIRTUALTABLE) */

#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
/*
** Estimate the location of a particular key among all keys in an
** index.  Store the results in aStat as follows:
**
**    aStat[0]      Est. number of rows less than pRec
**    aStat[1]      Est. number of rows equal to pRec
**
** Return the index of the sample that is the smallest sample that
** is greater than or equal to pRec. Note that this index is not an index
** into the aSample[] array - it is an index into a virtual set of samples
** based on the contents of aSample[] and the number of fields in record 
** pRec. 
*/
static int whereKeyStats(
  Parse *pParse,              /* Database connection */
  Index *pIdx,                /* Index to consider domain of */
  UnpackedRecord *pRec,       /* Vector of values to consider */
  int roundUp,                /* Round up if true.  Round down if false */
  tRowcnt *aStat              /* OUT: stats written here */
){
  IndexSample *aSample = pIdx->aSample;
  int iCol;                   /* Index of required stats in anEq[] etc. */
  int i;                      /* Index of first sample >= pRec */
  int iSample;                /* Smallest sample larger than or equal to pRec */
  int iMin = 0;               /* Smallest sample not yet tested */
  int iTest;                  /* Next sample to test */
  int res;                    /* Result of comparison operation */
  int nField;                 /* Number of fields in pRec */
  tRowcnt iLower = 0;         /* anLt[] + anEq[] of largest sample pRec is > */

#ifndef SQLITE_DEBUG
  UNUSED_PARAMETER( pParse );
#endif
  assert( pRec!=0 );
  assert( pIdx->nSample>0 );
  assert( pRec->nField>0 && pRec->nField<=pIdx->nSampleCol );

  /* Do a binary search to find the first sample greater than or equal
  ** to pRec. If pRec contains a single field, the set of samples to search
  ** is simply the aSample[] array. If the samples in aSample[] contain more
  ** than one fields, all fields following the first are ignored.
  **
  ** If pRec contains N fields, where N is more than one, then as well as the
  ** samples in aSample[] (truncated to N fields), the search also has to
  ** consider prefixes of those samples. For example, if the set of samples
  ** in aSample is:
  **
  **     aSample[0] = (a, 5) 
  **     aSample[1] = (a, 10) 
  **     aSample[2] = (b, 5) 
  **     aSample[3] = (c, 100) 
  **     aSample[4] = (c, 105)
  **
  ** Then the search space should ideally be the samples above and the 
  ** unique prefixes [a], [b] and [c]. But since that is hard to organize, 
  ** the code actually searches this set:
  **
  **     0: (a) 
  **     1: (a, 5) 
  **     2: (a, 10) 
  **     3: (a, 10) 
  **     4: (b) 
  **     5: (b, 5) 
  **     6: (c) 
  **     7: (c, 100) 
  **     8: (c, 105)
  **     9: (c, 105)
  **
  ** For each sample in the aSample[] array, N samples are present in the
  ** effective sample array. In the above, samples 0 and 1 are based on 
  ** sample aSample[0]. Samples 2 and 3 on aSample[1] etc.
  **
  ** Often, sample i of each block of N effective samples has (i+1) fields.
  ** Except, each sample may be extended to ensure that it is greater than or
  ** equal to the previous sample in the array. For example, in the above, 
  ** sample 2 is the first sample of a block of N samples, so at first it 
  ** appears that it should be 1 field in size. However, that would make it 
  ** smaller than sample 1, so the binary search would not work. As a result, 
  ** it is extended to two fields. The duplicates that this creates do not 
  ** cause any problems.
  */
  nField = pRec->nField;
  iCol = 0;
  iSample = pIdx->nSample * nField;
  do{
    int iSamp;                    /* Index in aSample[] of test sample */
    int n;                        /* Number of fields in test sample */

    iTest = (iMin+iSample)/2;
    iSamp = iTest / nField;
    if( iSamp>0 ){
      /* The proposed effective sample is a prefix of sample aSample[iSamp].
      ** Specifically, the shortest prefix of at least (1 + iTest%nField) 
      ** fields that is greater than the previous effective sample.  */
      for(n=(iTest % nField) + 1; n<nField; n++){
        if( aSample[iSamp-1].anLt[n-1]!=aSample[iSamp].anLt[n-1] ) break;
      }
    }else{
      n = iTest + 1;
    }

    pRec->nField = n;
    res = sqlite3VdbeRecordCompare(aSample[iSamp].n, aSample[iSamp].p, pRec);
    if( res<0 ){
      iLower = aSample[iSamp].anLt[n-1] + aSample[iSamp].anEq[n-1];
      iMin = iTest+1;
    }else if( res==0 && n<nField ){
      iLower = aSample[iSamp].anLt[n-1];
      iMin = iTest+1;
      res = -1;
    }else{
      iSample = iTest;
      iCol = n-1;
    }
  }while( res && iMin<iSample );
  i = iSample / nField;

#ifdef SQLITE_DEBUG
  /* The following assert statements check that the binary search code
  ** above found the right answer. This block serves no purpose other
  ** than to invoke the asserts.  */
  if( pParse->db->mallocFailed==0 ){
    if( res==0 ){
      /* If (res==0) is true, then pRec must be equal to sample i. */
      assert( i<pIdx->nSample );
      assert( iCol==nField-1 );
      pRec->nField = nField;
      assert( 0==sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec) 
           || pParse->db->mallocFailed 
      );
    }else{
      /* Unless i==pIdx->nSample, indicating that pRec is larger than
      ** all samples in the aSample[] array, pRec must be smaller than the
      ** (iCol+1) field prefix of sample i.  */
      assert( i<=pIdx->nSample && i>=0 );
      pRec->nField = iCol+1;
      assert( i==pIdx->nSample 
           || sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)>0
           || pParse->db->mallocFailed );

      /* if i==0 and iCol==0, then record pRec is smaller than all samples
      ** in the aSample[] array. Otherwise, if (iCol>0) then pRec must
      ** be greater than or equal to the (iCol) field prefix of sample i.
      ** If (i>0), then pRec must also be greater than sample (i-1).  */
      if( iCol>0 ){
        pRec->nField = iCol;
        assert( sqlite3VdbeRecordCompare(aSample[i].n, aSample[i].p, pRec)<=0
             || pParse->db->mallocFailed );
      }
      if( i>0 ){
        pRec->nField = nField;
        assert( sqlite3VdbeRecordCompare(aSample[i-1].n, aSample[i-1].p, pRec)<0
             || pParse->db->mallocFailed );
      }
    }
  }
#endif /* ifdef SQLITE_DEBUG */

  if( res==0 ){
    /* Record pRec is equal to sample i */
    assert( iCol==nField-1 );
    aStat[0] = aSample[i].anLt[iCol];
    aStat[1] = aSample[i].anEq[iCol];
  }else{
    /* At this point, the (iCol+1) field prefix of aSample[i] is the first 
    ** sample that is greater than pRec. Or, if i==pIdx->nSample then pRec
    ** is larger than all samples in the array. */
    tRowcnt iUpper, iGap;
    if( i>=pIdx->nSample ){
      iUpper = sqlite3LogEstToInt(pIdx->aiRowLogEst[0]);
    }else{
      iUpper = aSample[i].anLt[iCol];
    }

    if( iLower>=iUpper ){
      iGap = 0;
    }else{
      iGap = iUpper - iLower;
    }
    if( roundUp ){
      iGap = (iGap*2)/3;
    }else{
      iGap = iGap/3;
    }
    aStat[0] = iLower + iGap;
    aStat[1] = pIdx->aAvgEq[iCol];
  }

  /* Restore the pRec->nField value before returning.  */
  pRec->nField = nField;
  return i;
}
#endif /* SQLITE_ENABLE_STAT3_OR_STAT4 */

/*
** If it is not NULL, pTerm is a term that provides an upper or lower
** bound on a range scan. Without considering pTerm, it is estimated 
** that the scan will visit nNew rows. This function returns the number
** estimated to be visited after taking pTerm into account.
**
** If the user explicitly specified a likelihood() value for this term,
** then the return value is the likelihood multiplied by the number of
** input rows. Otherwise, this function assumes that an "IS NOT NULL" term
** has a likelihood of 0.50, and any other term a likelihood of 0.25.
*/
static LogEst whereRangeAdjust(WhereTerm *pTerm, LogEst nNew){
  LogEst nRet = nNew;
  if( pTerm ){
    if( pTerm->truthProb<=0 ){
      nRet += pTerm->truthProb;
    }else if( (pTerm->wtFlags & TERM_VNULL)==0 ){
      nRet -= 20;        assert( 20==sqlite3LogEst(4) );
    }
  }
  return nRet;
}

#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
/* 
** This function is called to estimate the number of rows visited by a
** range-scan on a skip-scan index. For example:
**
**   CREATE INDEX i1 ON t1(a, b, c);
**   SELECT * FROM t1 WHERE a=? AND c BETWEEN ? AND ?;
**
** Value pLoop->nOut is currently set to the estimated number of rows 
** visited for scanning (a=? AND b=?). This function reduces that estimate 
** by some factor to account for the (c BETWEEN ? AND ?) expression based
** on the stat4 data for the index. this scan will be peformed multiple 
** times (once for each (a,b) combination that matches a=?) is dealt with 
** by the caller.
**
** It does this by scanning through all stat4 samples, comparing values
** extracted from pLower and pUpper with the corresponding column in each
** sample. If L and U are the number of samples found to be less than or
** equal to the values extracted from pLower and pUpper respectively, and
** N is the total number of samples, the pLoop->nOut value is adjusted
** as follows:
**
**   nOut = nOut * ( min(U - L, 1) / N )
**
** If pLower is NULL, or a value cannot be extracted from the term, L is
** set to zero. If pUpper is NULL, or a value cannot be extracted from it,
** U is set to N.
**
** Normally, this function sets *pbDone to 1 before returning. However,
** if no value can be extracted from either pLower or pUpper (and so the
** estimate of the number of rows delivered remains unchanged), *pbDone
** is left as is.
**
** If an error occurs, an SQLite error code is returned. Otherwise, 
** SQLITE_OK.
*/
static int whereRangeSkipScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop,    /* Update the .nOut value of this loop */
  int *pbDone          /* Set to true if at least one expr. value extracted */
){
  Index *p = pLoop->u.btree.pIndex;
  int nEq = pLoop->u.btree.nEq;
  sqlite3 *db = pParse->db;
  int nLower = -1;
  int nUpper = p->nSample+1;
  int rc = SQLITE_OK;
  int iCol = p->aiColumn[nEq];
  u8 aff = iCol>=0 ? p->pTable->aCol[iCol].affinity : SQLITE_AFF_INTEGER;
  CollSeq *pColl;
  
  sqlite3_value *p1 = 0;          /* Value extracted from pLower */
  sqlite3_value *p2 = 0;          /* Value extracted from pUpper */
  sqlite3_value *pVal = 0;        /* Value extracted from record */

  pColl = sqlite3LocateCollSeq(pParse, p->azColl[nEq]);
  if( pLower ){
    rc = sqlite3Stat4ValueFromExpr(pParse, pLower->pExpr->pRight, aff, &p1);
    nLower = 0;
  }
  if( pUpper && rc==SQLITE_OK ){
    rc = sqlite3Stat4ValueFromExpr(pParse, pUpper->pExpr->pRight, aff, &p2);
    nUpper = p2 ? 0 : p->nSample;
  }

  if( p1 || p2 ){
    int i;
    int nDiff;
    for(i=0; rc==SQLITE_OK && i<p->nSample; i++){
      rc = sqlite3Stat4Column(db, p->aSample[i].p, p->aSample[i].n, nEq, &pVal);
      if( rc==SQLITE_OK && p1 ){
        int res = sqlite3MemCompare(p1, pVal, pColl);
        if( res>=0 ) nLower++;
      }
      if( rc==SQLITE_OK && p2 ){
        int res = sqlite3MemCompare(p2, pVal, pColl);
        if( res>=0 ) nUpper++;
      }
    }
    nDiff = (nUpper - nLower);
    if( nDiff<=0 ) nDiff = 1;

    /* If there is both an upper and lower bound specified, and the 
    ** comparisons indicate that they are close together, use the fallback
    ** method (assume that the scan visits 1/64 of the rows) for estimating
    ** the number of rows visited. Otherwise, estimate the number of rows
    ** using the method described in the header comment for this function. */
    if( nDiff!=1 || pUpper==0 || pLower==0 ){
      int nAdjust = (sqlite3LogEst(p->nSample) - sqlite3LogEst(nDiff));
      pLoop->nOut -= nAdjust;
      *pbDone = 1;
      WHERETRACE(0x10, ("range skip-scan regions: %u..%u  adjust=%d est=%d\n",
                           nLower, nUpper, nAdjust*-1, pLoop->nOut));
    }

  }else{
    assert( *pbDone==0 );
  }

  sqlite3ValueFree(p1);
  sqlite3ValueFree(p2);
  sqlite3ValueFree(pVal);

  return rc;
}
#endif /* SQLITE_ENABLE_STAT3_OR_STAT4 */

/*
** This function is used to estimate the number of rows that will be visited
** by scanning an index for a range of values. The range may have an upper
** bound, a lower bound, or both. The WHERE clause terms that set the upper
** and lower bounds are represented by pLower and pUpper respectively. For
** example, assuming that index p is on t1(a):
**
**   ... FROM t1 WHERE a > ? AND a < ? ...
**                    |_____|   |_____|
**                       |         |
**                     pLower    pUpper
**
** If either of the upper or lower bound is not present, then NULL is passed in
** place of the corresponding WhereTerm.
**
** The value in (pBuilder->pNew->u.btree.nEq) is the number of the index
** column subject to the range constraint. Or, equivalently, the number of
** equality constraints optimized by the proposed index scan. For example,
** assuming index p is on t1(a, b), and the SQL query is:
**
**   ... FROM t1 WHERE a = ? AND b > ? AND b < ? ...
**
** then nEq is set to 1 (as the range restricted column, b, is the second 
** left-most column of the index). Or, if the query is:
**
**   ... FROM t1 WHERE a > ? AND a < ? ...
**
** then nEq is set to 0.
**
** When this function is called, *pnOut is set to the sqlite3LogEst() of the
** number of rows that the index scan is expected to visit without 
** considering the range constraints. If nEq is 0, then *pnOut is the number of 
** rows in the index. Assuming no error occurs, *pnOut is adjusted (reduced)
** to account for the range constraints pLower and pUpper.
** 
** In the absence of sqlite_stat4 ANALYZE data, or if such data cannot be
** used, a single range inequality reduces the search space by a factor of 4. 
** and a pair of constraints (x>? AND x<?) reduces the expected number of
** rows visited by a factor of 64.
*/
static int whereRangeScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  WhereTerm *pLower,   /* Lower bound on the range. ex: "x>123" Might be NULL */
  WhereTerm *pUpper,   /* Upper bound on the range. ex: "x<455" Might be NULL */
  WhereLoop *pLoop     /* Modify the .nOut and maybe .rRun fields */
){
  int rc = SQLITE_OK;
  int nOut = pLoop->nOut;
  LogEst nNew;

#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
  Index *p = pLoop->u.btree.pIndex;
  int nEq = pLoop->u.btree.nEq;

  if( p->nSample>0 && nEq<p->nSampleCol ){
    if( nEq==pBuilder->nRecValid ){
      UnpackedRecord *pRec = pBuilder->pRec;
      tRowcnt a[2];
      u8 aff;

      /* Variable iLower will be set to the estimate of the number of rows in 
      ** the index that are less than the lower bound of the range query. The
      ** lower bound being the concatenation of $P and $L, where $P is the
      ** key-prefix formed by the nEq values matched against the nEq left-most
      ** columns of the index, and $L is the value in pLower.
      **
      ** Or, if pLower is NULL or $L cannot be extracted from it (because it
      ** is not a simple variable or literal value), the lower bound of the
      ** range is $P. Due to a quirk in the way whereKeyStats() works, even
      ** if $L is available, whereKeyStats() is called for both ($P) and 
      ** ($P:$L) and the larger of the two returned values is used.
      **
      ** Similarly, iUpper is to be set to the estimate of the number of rows
      ** less than the upper bound of the range query. Where the upper bound
      ** is either ($P) or ($P:$U). Again, even if $U is available, both values
      ** of iUpper are requested of whereKeyStats() and the smaller used.
      **
      ** The number of rows between the two bounds is then just iUpper-iLower.
      */
      tRowcnt iLower;     /* Rows less than the lower bound */
      tRowcnt iUpper;     /* Rows less than the upper bound */
      int iLwrIdx = -2;   /* aSample[] for the lower bound */
      int iUprIdx = -1;   /* aSample[] for the upper bound */

      if( pRec ){
        testcase( pRec->nField!=pBuilder->nRecValid );
        pRec->nField = pBuilder->nRecValid;
      }
      if( nEq==p->nKeyCol ){
        aff = SQLITE_AFF_INTEGER;
      }else{
        aff = p->pTable->aCol[p->aiColumn[nEq]].affinity;
      }
      /* Determine iLower and iUpper using ($P) only. */
      if( nEq==0 ){
        iLower = 0;
        iUpper = p->nRowEst0;
      }else{
        /* Note: this call could be optimized away - since the same values must 
        ** have been requested when testing key $P in whereEqualScanEst().  */
        whereKeyStats(pParse, p, pRec, 0, a);
        iLower = a[0];
        iUpper = a[0] + a[1];
      }

      assert( pLower==0 || (pLower->eOperator & (WO_GT|WO_GE))!=0 );
      assert( pUpper==0 || (pUpper->eOperator & (WO_LT|WO_LE))!=0 );
      assert( p->aSortOrder!=0 );
      if( p->aSortOrder[nEq] ){
        /* The roles of pLower and pUpper are swapped for a DESC index */
        SWAP(WhereTerm*, pLower, pUpper);
      }

      /* If possible, improve on the iLower estimate using ($P:$L). */
      if( pLower ){
        int bOk;                    /* True if value is extracted from pExpr */
        Expr *pExpr = pLower->pExpr->pRight;
        rc = sqlite3Stat4ProbeSetValue(pParse, p, &pRec, pExpr, aff, nEq, &bOk);
        if( rc==SQLITE_OK && bOk ){
          tRowcnt iNew;
          iLwrIdx = whereKeyStats(pParse, p, pRec, 0, a);
          iNew = a[0] + ((pLower->eOperator & (WO_GT|WO_LE)) ? a[1] : 0);
          if( iNew>iLower ) iLower = iNew;
          nOut--;
          pLower = 0;
        }
      }

      /* If possible, improve on the iUpper estimate using ($P:$U). */
      if( pUpper ){
        int bOk;                    /* True if value is extracted from pExpr */
        Expr *pExpr = pUpper->pExpr->pRight;
        rc = sqlite3Stat4ProbeSetValue(pParse, p, &pRec, pExpr, aff, nEq, &bOk);
        if( rc==SQLITE_OK && bOk ){
          tRowcnt iNew;
          iUprIdx = whereKeyStats(pParse, p, pRec, 1, a);
          iNew = a[0] + ((pUpper->eOperator & (WO_GT|WO_LE)) ? a[1] : 0);
          if( iNew<iUpper ) iUpper = iNew;
          nOut--;
          pUpper = 0;
        }
      }

      pBuilder->pRec = pRec;
      if( rc==SQLITE_OK ){
        if( iUpper>iLower ){
          nNew = sqlite3LogEst(iUpper - iLower);
          /* TUNING:  If both iUpper and iLower are derived from the same
          ** sample, then assume they are 4x more selective.  This brings
          ** the estimated selectivity more in line with what it would be
          ** if estimated without the use of STAT3/4 tables. */
          if( iLwrIdx==iUprIdx ) nNew -= 20;  assert( 20==sqlite3LogEst(4) );
        }else{
          nNew = 10;        assert( 10==sqlite3LogEst(2) );
        }
        if( nNew<nOut ){
          nOut = nNew;
        }
        WHERETRACE(0x10, ("STAT4 range scan: %u..%u  est=%d\n",
                           (u32)iLower, (u32)iUpper, nOut));
      }
    }else{
      int bDone = 0;
      rc = whereRangeSkipScanEst(pParse, pLower, pUpper, pLoop, &bDone);
      if( bDone ) return rc;
    }
  }
#else
  UNUSED_PARAMETER(pParse);
  UNUSED_PARAMETER(pBuilder);
  assert( pLower || pUpper );
#endif
  assert( pUpper==0 || (pUpper->wtFlags & TERM_VNULL)==0 );
  nNew = whereRangeAdjust(pLower, nOut);
  nNew = whereRangeAdjust(pUpper, nNew);

  /* TUNING: If there is both an upper and lower limit and neither limit
  ** has an application-defined likelihood(), assume the range is
  ** reduced by an additional 75%. This means that, by default, an open-ended
  ** range query (e.g. col > ?) is assumed to match 1/4 of the rows in the
  ** index. While a closed range (e.g. col BETWEEN ? AND ?) is estimated to
  ** match 1/64 of the index. */ 
  if( pLower && pLower->truthProb>0 && pUpper && pUpper->truthProb>0 ){
    nNew -= 20;
  }

  nOut -= (pLower!=0) + (pUpper!=0);
  if( nNew<10 ) nNew = 10;
  if( nNew<nOut ) nOut = nNew;
#if defined(WHERETRACE_ENABLED)
  if( pLoop->nOut>nOut ){
    WHERETRACE(0x10,("Range scan lowers nOut from %d to %d\n",
                    pLoop->nOut, nOut));
  }
#endif
  pLoop->nOut = (LogEst)nOut;
  return rc;
}

#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
/*
** Estimate the number of rows that will be returned based on
** an equality constraint x=VALUE and where that VALUE occurs in
** the histogram data.  This only works when x is the left-most
** column of an index and sqlite_stat3 histogram data is available
** for that index.  When pExpr==NULL that means the constraint is
** "x IS NULL" instead of "x=VALUE".
**
** Write the estimated row count into *pnRow and return SQLITE_OK. 
** If unable to make an estimate, leave *pnRow unchanged and return
** non-zero.
**
** This routine can fail if it is unable to load a collating sequence
** required for string comparison, or if unable to allocate memory
** for a UTF conversion required for comparison.  The error is stored
** in the pParse structure.
*/
static int whereEqualScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  Expr *pExpr,         /* Expression for VALUE in the x=VALUE constraint */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  Index *p = pBuilder->pNew->u.btree.pIndex;
  int nEq = pBuilder->pNew->u.btree.nEq;
  UnpackedRecord *pRec = pBuilder->pRec;
  u8 aff;                   /* Column affinity */
  int rc;                   /* Subfunction return code */
  tRowcnt a[2];             /* Statistics */
  int bOk;

  assert( nEq>=1 );
  assert( nEq<=p->nColumn );
  assert( p->aSample!=0 );
  assert( p->nSample>0 );
  assert( pBuilder->nRecValid<nEq );

  /* If values are not available for all fields of the index to the left
  ** of this one, no estimate can be made. Return SQLITE_NOTFOUND. */
  if( pBuilder->nRecValid<(nEq-1) ){
    return SQLITE_NOTFOUND;
  }

  /* This is an optimization only. The call to sqlite3Stat4ProbeSetValue()
  ** below would return the same value.  */
  if( nEq>=p->nColumn ){
    *pnRow = 1;
    return SQLITE_OK;
  }

  aff = p->pTable->aCol[p->aiColumn[nEq-1]].affinity;
  rc = sqlite3Stat4ProbeSetValue(pParse, p, &pRec, pExpr, aff, nEq-1, &bOk);
  pBuilder->pRec = pRec;
  if( rc!=SQLITE_OK ) return rc;
  if( bOk==0 ) return SQLITE_NOTFOUND;
  pBuilder->nRecValid = nEq;

  whereKeyStats(pParse, p, pRec, 0, a);
  WHERETRACE(0x10,("equality scan regions: %d\n", (int)a[1]));
  *pnRow = a[1];
  
  return rc;
}
#endif /* SQLITE_ENABLE_STAT3_OR_STAT4 */

#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
/*
** Estimate the number of rows that will be returned based on
** an IN constraint where the right-hand side of the IN operator
** is a list of values.  Example:
**
**        WHERE x IN (1,2,3,4)
**
** Write the estimated row count into *pnRow and return SQLITE_OK. 
** If unable to make an estimate, leave *pnRow unchanged and return
** non-zero.
**
** This routine can fail if it is unable to load a collating sequence
** required for string comparison, or if unable to allocate memory
** for a UTF conversion required for comparison.  The error is stored
** in the pParse structure.
*/
static int whereInScanEst(
  Parse *pParse,       /* Parsing & code generating context */
  WhereLoopBuilder *pBuilder,
  ExprList *pList,     /* The value list on the RHS of "x IN (v1,v2,v3,...)" */
  tRowcnt *pnRow       /* Write the revised row estimate here */
){
  Index *p = pBuilder->pNew->u.btree.pIndex;
  i64 nRow0 = sqlite3LogEstToInt(p->aiRowLogEst[0]);
  int nRecValid = pBuilder->nRecValid;
  int rc = SQLITE_OK;     /* Subfunction return code */
  tRowcnt nEst;           /* Number of rows for a single term */
  tRowcnt nRowEst = 0;    /* New estimate of the number of rows */
  int i;                  /* Loop counter */

  assert( p->aSample!=0 );
  for(i=0; rc==SQLITE_OK && i<pList->nExpr; i++){
    nEst = nRow0;
    rc = whereEqualScanEst(pParse, pBuilder, pList->a[i].pExpr, &nEst);
    nRowEst += nEst;
    pBuilder->nRecValid = nRecValid;
  }

  if( rc==SQLITE_OK ){
    if( nRowEst > nRow0 ) nRowEst = nRow0;
    *pnRow = nRowEst;
    WHERETRACE(0x10,("IN row estimate: est=%d\n", nRowEst));
  }
  assert( pBuilder->nRecValid==nRecValid );
  return rc;
}
#endif /* SQLITE_ENABLE_STAT3_OR_STAT4 */


#ifdef WHERETRACE_ENABLED
/*
** Print the content of a WhereTerm object
*/
static void whereTermPrint(WhereTerm *pTerm, int iTerm){
  if( pTerm==0 ){
    sqlite3DebugPrintf("TERM-%-3d NULL\n", iTerm);
  }else{
    char zType[4];
    memcpy(zType, "...", 4);
    if( pTerm->wtFlags & TERM_VIRTUAL ) zType[0] = 'V';
    if( pTerm->eOperator & WO_EQUIV  ) zType[1] = 'E';
    if( ExprHasProperty(pTerm->pExpr, EP_FromJoin) ) zType[2] = 'L';
    sqlite3DebugPrintf(
       "TERM-%-3d %p %s cursor=%-3d prob=%-3d op=0x%03x wtFlags=0x%04x\n",
       iTerm, pTerm, zType, pTerm->leftCursor, pTerm->truthProb,
       pTerm->eOperator, pTerm->wtFlags);
    sqlite3TreeViewExpr(0, pTerm->pExpr, 0);
  }
}
#endif

#ifdef WHERETRACE_ENABLED
/*
** Print a WhereLoop object for debugging purposes
*/
static void whereLoopPrint(WhereLoop *p, WhereClause *pWC){
  WhereInfo *pWInfo = pWC->pWInfo;
  int nb = 1+(pWInfo->pTabList->nSrc+7)/8;
  struct SrcList_item *pItem = pWInfo->pTabList->a + p->iTab;
  Table *pTab = pItem->pTab;
  sqlite3DebugPrintf("%c%2d.%0*llx.%0*llx", p->cId,
                     p->iTab, nb, p->maskSelf, nb, p->prereq);
  sqlite3DebugPrintf(" %12s",
                     pItem->zAlias ? pItem->zAlias : pTab->zName);
  if( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    const char *zName;
    if( p->u.btree.pIndex && (zName = p->u.btree.pIndex->zName)!=0 ){
      if( strncmp(zName, "sqlite_autoindex_", 17)==0 ){
        int i = sqlite3Strlen30(zName) - 1;
        while( zName[i]!='_' ) i--;
        zName += i;
      }
      sqlite3DebugPrintf(".%-16s %2d", zName, p->u.btree.nEq);
    }else{
      sqlite3DebugPrintf("%20s","");
    }
  }else{
    char *z;
    if( p->u.vtab.idxStr ){
      z = sqlite3_mprintf("(%d,\"%s\",%x)",
                p->u.vtab.idxNum, p->u.vtab.idxStr, p->u.vtab.omitMask);
    }else{
      z = sqlite3_mprintf("(%d,%x)", p->u.vtab.idxNum, p->u.vtab.omitMask);
    }
    sqlite3DebugPrintf(" %-19s", z);
    sqlite3_free(z);
  }
  if( p->wsFlags & WHERE_SKIPSCAN ){
    sqlite3DebugPrintf(" f %05x %d-%d", p->wsFlags, p->nLTerm,p->nSkip);
  }else{
    sqlite3DebugPrintf(" f %05x N %d", p->wsFlags, p->nLTerm);
  }
  sqlite3DebugPrintf(" cost %d,%d,%d\n", p->rSetup, p->rRun, p->nOut);
  if( p->nLTerm && (sqlite3WhereTrace & 0x100)!=0 ){
    int i;
    for(i=0; i<p->nLTerm; i++){
      whereTermPrint(p->aLTerm[i], i);
    }
  }
}
#endif

/*
** Convert bulk memory into a valid WhereLoop that can be passed
** to whereLoopClear harmlessly.
*/
static void whereLoopInit(WhereLoop *p){
  p->aLTerm = p->aLTermSpace;
  p->nLTerm = 0;
  p->nLSlot = ArraySize(p->aLTermSpace);
  p->wsFlags = 0;
}

/*
** Clear the WhereLoop.u union.  Leave WhereLoop.pLTerm intact.
*/
static void whereLoopClearUnion(sqlite3 *db, WhereLoop *p){
  if( p->wsFlags & (WHERE_VIRTUALTABLE|WHERE_AUTO_INDEX) ){
    if( (p->wsFlags & WHERE_VIRTUALTABLE)!=0 && p->u.vtab.needFree ){
      sqlite3_free(p->u.vtab.idxStr);
      p->u.vtab.needFree = 0;
      p->u.vtab.idxStr = 0;
    }else if( (p->wsFlags & WHERE_AUTO_INDEX)!=0 && p->u.btree.pIndex!=0 ){
      sqlite3DbFree(db, p->u.btree.pIndex->zColAff);
      sqlite3DbFree(db, p->u.btree.pIndex);
      p->u.btree.pIndex = 0;
    }
  }
}

/*
** Deallocate internal memory used by a WhereLoop object
*/
static void whereLoopClear(sqlite3 *db, WhereLoop *p){
  if( p->aLTerm!=p->aLTermSpace ) sqlite3DbFree(db, p->aLTerm);
  whereLoopClearUnion(db, p);
  whereLoopInit(p);
}

/*
** Increase the memory allocation for pLoop->aLTerm[] to be at least n.
*/
static int whereLoopResize(sqlite3 *db, WhereLoop *p, int n){
  WhereTerm **paNew;
  if( p->nLSlot>=n ) return SQLITE_OK;
  n = (n+7)&~7;
  paNew = sqlite3DbMallocRaw(db, sizeof(p->aLTerm[0])*n);
  if( paNew==0 ) return SQLITE_NOMEM;
  memcpy(paNew, p->aLTerm, sizeof(p->aLTerm[0])*p->nLSlot);
  if( p->aLTerm!=p->aLTermSpace ) sqlite3DbFree(db, p->aLTerm);
  p->aLTerm = paNew;
  p->nLSlot = n;
  return SQLITE_OK;
}

/*
** Transfer content from the second pLoop into the first.
*/
static int whereLoopXfer(sqlite3 *db, WhereLoop *pTo, WhereLoop *pFrom){
  whereLoopClearUnion(db, pTo);
  if( whereLoopResize(db, pTo, pFrom->nLTerm) ){
    memset(&pTo->u, 0, sizeof(pTo->u));
    return SQLITE_NOMEM;
  }
  memcpy(pTo, pFrom, WHERE_LOOP_XFER_SZ);
  memcpy(pTo->aLTerm, pFrom->aLTerm, pTo->nLTerm*sizeof(pTo->aLTerm[0]));
  if( pFrom->wsFlags & WHERE_VIRTUALTABLE ){
    pFrom->u.vtab.needFree = 0;
  }else if( (pFrom->wsFlags & WHERE_AUTO_INDEX)!=0 ){
    pFrom->u.btree.pIndex = 0;
  }
  return SQLITE_OK;
}

/*
** Delete a WhereLoop object
*/
static void whereLoopDelete(sqlite3 *db, WhereLoop *p){
  whereLoopClear(db, p);
  sqlite3DbFree(db, p);
}

/*
** Free a WhereInfo structure
*/
static void whereInfoFree(sqlite3 *db, WhereInfo *pWInfo){
  if( ALWAYS(pWInfo) ){
    int i;
    for(i=0; i<pWInfo->nLevel; i++){
      WhereLevel *pLevel = &pWInfo->a[i];
      if( pLevel->pWLoop && (pLevel->pWLoop->wsFlags & WHERE_IN_ABLE) ){
        sqlite3DbFree(db, pLevel->u.in.aInLoop);
      }
    }
    sqlite3WhereClauseClear(&pWInfo->sWC);
    while( pWInfo->pLoops ){
      WhereLoop *p = pWInfo->pLoops;
      pWInfo->pLoops = p->pNextLoop;
      whereLoopDelete(db, p);
    }
    sqlite3DbFree(db, pWInfo);
  }
}

/*
** Return TRUE if all of the following are true:
**
**   (1)  X has the same or lower cost that Y
**   (2)  X is a proper subset of Y
**   (3)  X skips at least as many columns as Y
**
** By "proper subset" we mean that X uses fewer WHERE clause terms
** than Y and that every WHERE clause term used by X is also used
** by Y.
**
** If X is a proper subset of Y then Y is a better choice and ought
** to have a lower cost.  This routine returns TRUE when that cost 
** relationship is inverted and needs to be adjusted.  The third rule
** was added because if X uses skip-scan less than Y it still might
** deserve a lower cost even if it is a proper subset of Y.
*/
static int whereLoopCheaperProperSubset(
  const WhereLoop *pX,       /* First WhereLoop to compare */
  const WhereLoop *pY        /* Compare against this WhereLoop */
){
  int i, j;
  if( pX->nLTerm-pX->nSkip >= pY->nLTerm-pY->nSkip ){
    return 0; /* X is not a subset of Y */
  }
  if( pY->nSkip > pX->nSkip ) return 0;
  if( pX->rRun >= pY->rRun ){
    if( pX->rRun > pY->rRun ) return 0;    /* X costs more than Y */
    if( pX->nOut > pY->nOut ) return 0;    /* X costs more than Y */
  }
  for(i=pX->nLTerm-1; i>=0; i--){
    if( pX->aLTerm[i]==0 ) continue;
    for(j=pY->nLTerm-1; j>=0; j--){
      if( pY->aLTerm[j]==pX->aLTerm[i] ) break;
    }
    if( j<0 ) return 0;  /* X not a subset of Y since term X[i] not used by Y */
  }
  return 1;  /* All conditions meet */
}

/*
** Try to adjust the cost of WhereLoop pTemplate upwards or downwards so
** that:
**
**   (1) pTemplate costs less than any other WhereLoops that are a proper
**       subset of pTemplate
**
**   (2) pTemplate costs more than any other WhereLoops for which pTemplate
**       is a proper subset.
**
** To say "WhereLoop X is a proper subset of Y" means that X uses fewer
** WHERE clause terms than Y and that every WHERE clause term used by X is
** also used by Y.
*/
static void whereLoopAdjustCost(const WhereLoop *p, WhereLoop *pTemplate){
  if( (pTemplate->wsFlags & WHERE_INDEXED)==0 ) return;
  for(; p; p=p->pNextLoop){
    if( p->iTab!=pTemplate->iTab ) continue;
    if( (p->wsFlags & WHERE_INDEXED)==0 ) continue;
    if( whereLoopCheaperProperSubset(p, pTemplate) ){
      /* Adjust pTemplate cost downward so that it is cheaper than its 
      ** subset p. */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->rRun, pTemplate->nOut, p->rRun, p->nOut-1));
      pTemplate->rRun = p->rRun;
      pTemplate->nOut = p->nOut - 1;
    }else if( whereLoopCheaperProperSubset(pTemplate, p) ){
      /* Adjust pTemplate cost upward so that it is costlier than p since
      ** pTemplate is a proper subset of p */
      WHERETRACE(0x80,("subset cost adjustment %d,%d to %d,%d\n",
                       pTemplate->rRun, pTemplate->nOut, p->rRun, p->nOut+1));
      pTemplate->rRun = p->rRun;
      pTemplate->nOut = p->nOut + 1;
    }
  }
}

/*
** Search the list of WhereLoops in *ppPrev looking for one that can be
** supplanted by pTemplate.
**
** Return NULL if the WhereLoop list contains an entry that can supplant
** pTemplate, in other words if pTemplate does not belong on the list.
**
** If pX is a WhereLoop that pTemplate can supplant, then return the
** link that points to pX.
**
** If pTemplate cannot supplant any existing element of the list but needs
** to be added to the list, then return a pointer to the tail of the list.
*/
static WhereLoop **whereLoopFindLesser(
  WhereLoop **ppPrev,
  const WhereLoop *pTemplate
){
  WhereLoop *p;
  for(p=(*ppPrev); p; ppPrev=&p->pNextLoop, p=*ppPrev){
    if( p->iTab!=pTemplate->iTab || p->iSortIdx!=pTemplate->iSortIdx ){
      /* If either the iTab or iSortIdx values for two WhereLoop are different
      ** then those WhereLoops need to be considered separately.  Neither is
      ** a candidate to replace the other. */
      continue;
    }
    /* In the current implementation, the rSetup value is either zero
    ** or the cost of building an automatic index (NlogN) and the NlogN
    ** is the same for compatible WhereLoops. */
    assert( p->rSetup==0 || pTemplate->rSetup==0 
                 || p->rSetup==pTemplate->rSetup );

    /* whereLoopAddBtree() always generates and inserts the automatic index
    ** case first.  Hence compatible candidate WhereLoops never have a larger
    ** rSetup. Call this SETUP-INVARIANT */
    assert( p->rSetup>=pTemplate->rSetup );

    /* Any loop using an appliation-defined index (or PRIMARY KEY or
    ** UNIQUE constraint) with one or more == constraints is better
    ** than an automatic index. Unless it is a skip-scan. */
    if( (p->wsFlags & WHERE_AUTO_INDEX)!=0
     && (pTemplate->nSkip)==0
     && (pTemplate->wsFlags & WHERE_INDEXED)!=0
     && (pTemplate->wsFlags & WHERE_COLUMN_EQ)!=0
     && (p->prereq & pTemplate->prereq)==pTemplate->prereq
    ){
      break;
    }

    /* If existing WhereLoop p is better than pTemplate, pTemplate can be
    ** discarded.  WhereLoop p is better if:
    **   (1)  p has no more dependencies than pTemplate, and
    **   (2)  p has an equal or lower cost than pTemplate
    */
    if( (p->prereq & pTemplate->prereq)==p->prereq    /* (1)  */
     && p->rSetup<=pTemplate->rSetup                  /* (2a) */
     && p->rRun<=pTemplate->rRun                      /* (2b) */
     && p->nOut<=pTemplate->nOut                      /* (2c) */
    ){
      return 0;  /* Discard pTemplate */
    }

    /* If pTemplate is always better than p, then cause p to be overwritten
    ** with pTemplate.  pTemplate is better than p if:
    **   (1)  pTemplate has no more dependences than p, and
    **   (2)  pTemplate has an equal or lower cost than p.
    */
    if( (p->prereq & pTemplate->prereq)==pTemplate->prereq   /* (1)  */
     && p->rRun>=pTemplate->rRun                             /* (2a) */
     && p->nOut>=pTemplate->nOut                             /* (2b) */
    ){
      assert( p->rSetup>=pTemplate->rSetup ); /* SETUP-INVARIANT above */
      break;   /* Cause p to be overwritten by pTemplate */
    }
  }
  return ppPrev;
}

/*
** Insert or replace a WhereLoop entry using the template supplied.
**
** An existing WhereLoop entry might be overwritten if the new template
** is better and has fewer dependencies.  Or the template will be ignored
** and no insert will occur if an existing WhereLoop is faster and has
** fewer dependencies than the template.  Otherwise a new WhereLoop is
** added based on the template.
**
** If pBuilder->pOrSet is not NULL then we care about only the
** prerequisites and rRun and nOut costs of the N best loops.  That
** information is gathered in the pBuilder->pOrSet object.  This special
** processing mode is used only for OR clause processing.
**
** When accumulating multiple loops (when pBuilder->pOrSet is NULL) we
** still might overwrite similar loops with the new template if the
** new template is better.  Loops may be overwritten if the following 
** conditions are met:
**
**    (1)  They have the same iTab.
**    (2)  They have the same iSortIdx.
**    (3)  The template has same or fewer dependencies than the current loop
**    (4)  The template has the same or lower cost than the current loop
*/
static int whereLoopInsert(WhereLoopBuilder *pBuilder, WhereLoop *pTemplate){
  WhereLoop **ppPrev, *p;
  WhereInfo *pWInfo = pBuilder->pWInfo;
  sqlite3 *db = pWInfo->pParse->db;

  /* If pBuilder->pOrSet is defined, then only keep track of the costs
  ** and prereqs.
  */
  if( pBuilder->pOrSet!=0 ){
#if WHERETRACE_ENABLED
    u16 n = pBuilder->pOrSet->n;
    int x =
#endif
    whereOrInsert(pBuilder->pOrSet, pTemplate->prereq, pTemplate->rRun,
                                    pTemplate->nOut);
#if WHERETRACE_ENABLED /* 0x8 */
    if( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf(x?"   or-%d:  ":"   or-X:  ", n);
      whereLoopPrint(pTemplate, pBuilder->pWC);
    }
#endif
    return SQLITE_OK;
  }

  /* Look for an existing WhereLoop to replace with pTemplate
  */
  whereLoopAdjustCost(pWInfo->pLoops, pTemplate);
  ppPrev = whereLoopFindLesser(&pWInfo->pLoops, pTemplate);

  if( ppPrev==0 ){
    /* There already exists a WhereLoop on the list that is better
    ** than pTemplate, so just ignore pTemplate */
#if WHERETRACE_ENABLED /* 0x8 */
    if( sqlite3WhereTrace & 0x8 ){
      sqlite3DebugPrintf("   skip: ");
      whereLoopPrint(pTemplate, pBuilder->pWC);
    }
#endif
    return SQLITE_OK;  
  }else{
    p = *ppPrev;
  }

  /* If we reach this point it means that either p[] should be overwritten
  ** with pTemplate[] if p[] exists, or if p==NULL then allocate a new
  ** WhereLoop and insert it.
  */
#if WHERETRACE_ENABLED /* 0x8 */
  if( sqlite3WhereTrace & 0x8 ){
    if( p!=0 ){
      sqlite3DebugPrintf("replace: ");
      whereLoopPrint(p, pBuilder->pWC);
    }
    sqlite3DebugPrintf("    add: ");
    whereLoopPrint(pTemplate, pBuilder->pWC);
  }
#endif
  if( p==0 ){
    /* Allocate a new WhereLoop to add to the end of the list */
    *ppPrev = p = sqlite3DbMallocRaw(db, sizeof(WhereLoop));
    if( p==0 ) return SQLITE_NOMEM;
    whereLoopInit(p);
    p->pNextLoop = 0;
  }else{
    /* We will be overwriting WhereLoop p[].  But before we do, first
    ** go through the rest of the list and delete any other entries besides
    ** p[] that are also supplated by pTemplate */
    WhereLoop **ppTail = &p->pNextLoop;
    WhereLoop *pToDel;
    while( *ppTail ){
      ppTail = whereLoopFindLesser(ppTail, pTemplate);
      if( ppTail==0 ) break;
      pToDel = *ppTail;
      if( pToDel==0 ) break;
      *ppTail = pToDel->pNextLoop;
#if WHERETRACE_ENABLED /* 0x8 */
      if( sqlite3WhereTrace & 0x8 ){
        sqlite3DebugPrintf(" delete: ");
        whereLoopPrint(pToDel, pBuilder->pWC);
      }
#endif
      whereLoopDelete(db, pToDel);
    }
  }
  whereLoopXfer(db, p, pTemplate);
  if( (p->wsFlags & WHERE_VIRTUALTABLE)==0 ){
    Index *pIndex = p->u.btree.pIndex;
    if( pIndex && pIndex->tnum==0 ){
      p->u.btree.pIndex = 0;
    }
  }
  return SQLITE_OK;
}

/*
** Adjust the WhereLoop.nOut value downward to account for terms of the
** WHERE clause that reference the loop but which are not used by an
** index.
*
** For every WHERE clause term that is not used by the index
** and which has a truth probability assigned by one of the likelihood(),
** likely(), or unlikely() SQL functions, reduce the estimated number
** of output rows by the probability specified.
**
** TUNING:  For every WHERE clause term that is not used by the index
** and which does not have an assigned truth probability, heuristics
** described below are used to try to estimate the truth probability.
** TODO --> Perhaps this is something that could be improved by better
** table statistics.
**
** Heuristic 1:  Estimate the truth probability as 93.75%.  The 93.75%
** value corresponds to -1 in LogEst notation, so this means decrement
** the WhereLoop.nOut field for every such WHERE clause term.
**
** Heuristic 2:  If there exists one or more WHERE clause terms of the
** form "x==EXPR" and EXPR is not a constant 0 or 1, then make sure the
** final output row estimate is no greater than 1/4 of the total number
** of rows in the table.  In other words, assume that x==EXPR will filter
** out at least 3 out of 4 rows.  If EXPR is -1 or 0 or 1, then maybe the
** "x" column is boolean or else -1 or 0 or 1 is a common default value
** on the "x" column and so in that case only cap the output row estimate
** at 1/2 instead of 1/4.
*/
static void whereLoopOutputAdjust(
  WhereClause *pWC,      /* The WHERE clause */
  WhereLoop *pLoop,      /* The loop to adjust downward */
  LogEst nRow            /* Number of rows in the entire table */
){
  WhereTerm *pTerm, *pX;
  Bitmask notAllowed = ~(pLoop->prereq|pLoop->maskSelf);
  int i, j, k;
  LogEst iReduce = 0;    /* pLoop->nOut should not exceed nRow-iReduce */

  assert( (pLoop->wsFlags & WHERE_AUTO_INDEX)==0 );
  for(i=pWC->nTerm, pTerm=pWC->a; i>0; i--, pTerm++){
    if( (pTerm->wtFlags & TERM_VIRTUAL)!=0 ) break;
    if( (pTerm->prereqAll & pLoop->maskSelf)==0 ) continue;
    if( (pTerm->prereqAll & notAllowed)!=0 ) continue;
    for(j=pLoop->nLTerm-1; j>=0; j--){
      pX = pLoop->aLTerm[j];
      if( pX==0 ) continue;
      if( pX==pTerm ) break;
      if( pX->iParent>=0 && (&pWC->a[pX->iParent])==pTerm ) break;
    }
    if( j<0 ){
      if( pTerm->truthProb<=0 ){
        /* If a truth probability is specified using the likelihood() hints,
        ** then use the probability provided by the application. */
        pLoop->nOut += pTerm->truthProb;
      }else{
        /* In the absence of explicit truth probabilities, use heuristics to
        ** guess a reasonable truth probability. */
        pLoop->nOut--;
        if( pTerm->eOperator&(WO_EQ|WO_IS) ){
          Expr *pRight = pTerm->pExpr->pRight;
          testcase( pTerm->pExpr->op==TK_IS );
          if( sqlite3ExprIsInteger(pRight, &k) && k>=(-1) && k<=1 ){
            k = 10;
          }else{
            k = 20;
          }
          if( iReduce<k ) iReduce = k;
        }
      }
    }
  }
  if( pLoop->nOut > nRow-iReduce )  pLoop->nOut = nRow - iReduce;
}

/*
** Adjust the cost C by the costMult facter T.  This only occurs if
** compiled with -DSQLITE_ENABLE_COSTMULT
*/
#ifdef SQLITE_ENABLE_COSTMULT
# define ApplyCostMultiplier(C,T)  C += T
#else
# define ApplyCostMultiplier(C,T)
#endif

/*
** We have so far matched pBuilder->pNew->u.btree.nEq terms of the 
** index pIndex. Try to match one more.
**
** When this function is called, pBuilder->pNew->nOut contains the 
** number of rows expected to be visited by filtering using the nEq 
** terms only. If it is modified, this value is restored before this 
** function returns.
**
** If pProbe->tnum==0, that means pIndex is a fake index used for the
** INTEGER PRIMARY KEY.
*/
static int whereLoopAddBtreeIndex(
  WhereLoopBuilder *pBuilder,     /* The WhereLoop factory */
  struct SrcList_item *pSrc,      /* FROM clause term being analyzed */
  Index *pProbe,                  /* An index on pSrc */
  LogEst nInMul                   /* log(Number of iterations due to IN) */
){
  WhereInfo *pWInfo = pBuilder->pWInfo;  /* WHERE analyse context */
  Parse *pParse = pWInfo->pParse;        /* Parsing context */
  sqlite3 *db = pParse->db;       /* Database connection malloc context */
  WhereLoop *pNew;                /* Template WhereLoop under construction */
  WhereTerm *pTerm;               /* A WhereTerm under consideration */
  int opMask;                     /* Valid operators for constraints */
  WhereScan scan;                 /* Iterator for WHERE terms */
  Bitmask saved_prereq;           /* Original value of pNew->prereq */
  u16 saved_nLTerm;               /* Original value of pNew->nLTerm */
  u16 saved_nEq;                  /* Original value of pNew->u.btree.nEq */
  u16 saved_nSkip;                /* Original value of pNew->nSkip */
  u32 saved_wsFlags;              /* Original value of pNew->wsFlags */
  LogEst saved_nOut;              /* Original value of pNew->nOut */
  int iCol;                       /* Index of the column in the table */
  int rc = SQLITE_OK;             /* Return code */
  LogEst rSize;                   /* Number of rows in the table */
  LogEst rLogSize;                /* Logarithm of table size */
  WhereTerm *pTop = 0, *pBtm = 0; /* Top and bottom range constraints */

  pNew = pBuilder->pNew;
  if( db->mallocFailed ) return SQLITE_NOMEM;

  assert( (pNew->wsFlags & WHERE_VIRTUALTABLE)==0 );
  assert( (pNew->wsFlags & WHERE_TOP_LIMIT)==0 );
  if( pNew->wsFlags & WHERE_BTM_LIMIT ){
    opMask = WO_LT|WO_LE;
  }else if( /*pProbe->tnum<=0 ||*/ (pSrc->jointype & JT_LEFT)!=0 ){
    opMask = WO_EQ|WO_IN|WO_GT|WO_GE|WO_LT|WO_LE;
  }else{
    opMask = WO_EQ|WO_IN|WO_GT|WO_GE|WO_LT|WO_LE|WO_ISNULL|WO_IS;
  }
  if( pProbe->bUnordered ) opMask &= ~(WO_GT|WO_GE|WO_LT|WO_LE);

  assert( pNew->u.btree.nEq<pProbe->nColumn );
  iCol = pProbe->aiColumn[pNew->u.btree.nEq];

  pTerm = whereScanInit(&scan, pBuilder->pWC, pSrc->iCursor, iCol,
                        opMask, pProbe);
  saved_nEq = pNew->u.btree.nEq;
  saved_nSkip = pNew->nSkip;
  saved_nLTerm = pNew->nLTerm;
  saved_wsFlags = pNew->wsFlags;
  saved_prereq = pNew->prereq;
  saved_nOut = pNew->nOut;
  pNew->rSetup = 0;
  rSize = pProbe->aiRowLogEst[0];
  rLogSize = estLog(rSize);
  for(; rc==SQLITE_OK && pTerm!=0; pTerm = whereScanNext(&scan)){
    u16 eOp = pTerm->eOperator;   /* Shorthand for pTerm->eOperator */
    LogEst rCostIdx;
    LogEst nOutUnadjusted;        /* nOut before IN() and WHERE adjustments */
    int nIn = 0;
#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
    int nRecValid = pBuilder->nRecValid;
#endif
    if( (eOp==WO_ISNULL || (pTerm->wtFlags&TERM_VNULL)!=0)
     && (iCol<0 || pSrc->pTab->aCol[iCol].notNull)
    ){
      continue; /* ignore IS [NOT] NULL constraints on NOT NULL columns */
    }
    if( pTerm->prereqRight & pNew->maskSelf ) continue;

    /* Do not allow the upper bound of a LIKE optimization range constraint
    ** to mix with a lower range bound from some other source */
    if( pTerm->wtFlags & TERM_LIKEOPT && pTerm->eOperator==WO_LT ) continue;

    pNew->wsFlags = saved_wsFlags;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nLTerm = saved_nLTerm;
    if( whereLoopResize(db, pNew, pNew->nLTerm+1) ) break; /* OOM */
    pNew->aLTerm[pNew->nLTerm++] = pTerm;
    pNew->prereq = (saved_prereq | pTerm->prereqRight) & ~pNew->maskSelf;

    assert( nInMul==0
        || (pNew->wsFlags & WHERE_COLUMN_NULL)!=0 
        || (pNew->wsFlags & WHERE_COLUMN_IN)!=0 
        || (pNew->wsFlags & WHERE_SKIPSCAN)!=0 
    );

    if( eOp & WO_IN ){
      Expr *pExpr = pTerm->pExpr;
      pNew->wsFlags |= WHERE_COLUMN_IN;
      if( ExprHasProperty(pExpr, EP_xIsSelect) ){
        /* "x IN (SELECT ...)":  TUNING: the SELECT returns 25 rows */
        nIn = 46;  assert( 46==sqlite3LogEst(25) );
      }else if( ALWAYS(pExpr->x.pList && pExpr->x.pList->nExpr) ){
        /* "x IN (value, value, ...)" */
        nIn = sqlite3LogEst(pExpr->x.pList->nExpr);
      }
      assert( nIn>0 );  /* RHS always has 2 or more terms...  The parser
                        ** changes "x IN (?)" into "x=?". */

    }else if( eOp & (WO_EQ|WO_IS) ){
      pNew->wsFlags |= WHERE_COLUMN_EQ;
      if( iCol<0 || (nInMul==0 && pNew->u.btree.nEq==pProbe->nKeyCol-1) ){
        if( iCol>=0 && pProbe->uniqNotNull==0 ){
          pNew->wsFlags |= WHERE_UNQ_WANTED;
        }else{
          pNew->wsFlags |= WHERE_ONEROW;
        }
      }
    }else if( eOp & WO_ISNULL ){
      pNew->wsFlags |= WHERE_COLUMN_NULL;
    }else if( eOp & (WO_GT|WO_GE) ){
      testcase( eOp & WO_GT );
      testcase( eOp & WO_GE );
      pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_BTM_LIMIT;
      pBtm = pTerm;
      pTop = 0;
      if( pTerm->wtFlags & TERM_LIKEOPT ){
        /* Range contraints that come from the LIKE optimization are
        ** always used in pairs. */
        pTop = &pTerm[1];
        assert( (pTop-(pTerm->pWC->a))<pTerm->pWC->nTerm );
        assert( pTop->wtFlags & TERM_LIKEOPT );
        assert( pTop->eOperator==WO_LT );
        if( whereLoopResize(db, pNew, pNew->nLTerm+1) ) break; /* OOM */
        pNew->aLTerm[pNew->nLTerm++] = pTop;
        pNew->wsFlags |= WHERE_TOP_LIMIT;
      }
    }else{
      assert( eOp & (WO_LT|WO_LE) );
      testcase( eOp & WO_LT );
      testcase( eOp & WO_LE );
      pNew->wsFlags |= WHERE_COLUMN_RANGE|WHERE_TOP_LIMIT;
      pTop = pTerm;
      pBtm = (pNew->wsFlags & WHERE_BTM_LIMIT)!=0 ?
                     pNew->aLTerm[pNew->nLTerm-2] : 0;
    }

    /* At this point pNew->nOut is set to the number of rows expected to
    ** be visited by the index scan before considering term pTerm, or the
    ** values of nIn and nInMul. In other words, assuming that all 
    ** "x IN(...)" terms are replaced with "x = ?". This block updates
    ** the value of pNew->nOut to account for pTerm (but not nIn/nInMul).  */
    assert( pNew->nOut==saved_nOut );
    if( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      /* Adjust nOut using stat3/stat4 data. Or, if there is no stat3/stat4
      ** data, using some other estimate.  */
      whereRangeScanEst(pParse, pBuilder, pBtm, pTop, pNew);
    }else{
      int nEq = ++pNew->u.btree.nEq;
      assert( eOp & (WO_ISNULL|WO_EQ|WO_IN|WO_IS) );

      assert( pNew->nOut==saved_nOut );
      if( pTerm->truthProb<=0 && iCol>=0 ){
        assert( (eOp & WO_IN) || nIn==0 );
        testcase( eOp & WO_IN );
        pNew->nOut += pTerm->truthProb;
        pNew->nOut -= nIn;
      }else{
#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
        tRowcnt nOut = 0;
        if( nInMul==0 
         && pProbe->nSample 
         && pNew->u.btree.nEq<=pProbe->nSampleCol
         && ((eOp & WO_IN)==0 || !ExprHasProperty(pTerm->pExpr, EP_xIsSelect))
        ){
          Expr *pExpr = pTerm->pExpr;
          if( (eOp & (WO_EQ|WO_ISNULL|WO_IS))!=0 ){
            testcase( eOp & WO_EQ );
            testcase( eOp & WO_IS );
            testcase( eOp & WO_ISNULL );
            rc = whereEqualScanEst(pParse, pBuilder, pExpr->pRight, &nOut);
          }else{
            rc = whereInScanEst(pParse, pBuilder, pExpr->x.pList, &nOut);
          }
          if( rc==SQLITE_NOTFOUND ) rc = SQLITE_OK;
          if( rc!=SQLITE_OK ) break;          /* Jump out of the pTerm loop */
          if( nOut ){
            pNew->nOut = sqlite3LogEst(nOut);
            if( pNew->nOut>saved_nOut ) pNew->nOut = saved_nOut;
            pNew->nOut -= nIn;
          }
        }
        if( nOut==0 )
#endif
        {
          pNew->nOut += (pProbe->aiRowLogEst[nEq] - pProbe->aiRowLogEst[nEq-1]);
          if( eOp & WO_ISNULL ){
            /* TUNING: If there is no likelihood() value, assume that a 
            ** "col IS NULL" expression matches twice as many rows 
            ** as (col=?). */
            pNew->nOut += 10;
          }
        }
      }
    }

    /* Set rCostIdx to the cost of visiting selected rows in index. Add
    ** it to pNew->rRun, which is currently set to the cost of the index
    ** seek only. Then, if this is a non-covering index, add the cost of
    ** visiting the rows in the main table.  */
    rCostIdx = pNew->nOut + 1 + (15*pProbe->szIdxRow)/pSrc->pTab->szTabRow;
    pNew->rRun = sqlite3LogEstAdd(rLogSize, rCostIdx);
    if( (pNew->wsFlags & (WHERE_IDX_ONLY|WHERE_IPK))==0 ){
      pNew->rRun = sqlite3LogEstAdd(pNew->rRun, pNew->nOut + 16);
    }
    ApplyCostMultiplier(pNew->rRun, pProbe->pTable->costMult);

    nOutUnadjusted = pNew->nOut;
    pNew->rRun += nInMul + nIn;
    pNew->nOut += nInMul + nIn;
    whereLoopOutputAdjust(pBuilder->pWC, pNew, rSize);
    rc = whereLoopInsert(pBuilder, pNew);

    if( pNew->wsFlags & WHERE_COLUMN_RANGE ){
      pNew->nOut = saved_nOut;
    }else{
      pNew->nOut = nOutUnadjusted;
    }

    if( (pNew->wsFlags & WHERE_TOP_LIMIT)==0
     && pNew->u.btree.nEq<pProbe->nColumn
    ){
      whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, nInMul+nIn);
    }
    pNew->nOut = saved_nOut;
#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
    pBuilder->nRecValid = nRecValid;
#endif
  }
  pNew->prereq = saved_prereq;
  pNew->u.btree.nEq = saved_nEq;
  pNew->nSkip = saved_nSkip;
  pNew->wsFlags = saved_wsFlags;
  pNew->nOut = saved_nOut;
  pNew->nLTerm = saved_nLTerm;

  /* Consider using a skip-scan if there are no WHERE clause constraints
  ** available for the left-most terms of the index, and if the average
  ** number of repeats in the left-most terms is at least 18. 
  **
  ** The magic number 18 is selected on the basis that scanning 17 rows
  ** is almost always quicker than an index seek (even though if the index
  ** contains fewer than 2^17 rows we assume otherwise in other parts of
  ** the code). And, even if it is not, it should not be too much slower. 
  ** On the other hand, the extra seeks could end up being significantly
  ** more expensive.  */
  assert( 42==sqlite3LogEst(18) );
  if( saved_nEq==saved_nSkip
   && saved_nEq+1<pProbe->nKeyCol
   && pProbe->noSkipScan==0
   && pProbe->aiRowLogEst[saved_nEq+1]>=42  /* TUNING: Minimum for skip-scan */
   && (rc = whereLoopResize(db, pNew, pNew->nLTerm+1))==SQLITE_OK
  ){
    LogEst nIter;
    pNew->u.btree.nEq++;
    pNew->nSkip++;
    pNew->aLTerm[pNew->nLTerm++] = 0;
    pNew->wsFlags |= WHERE_SKIPSCAN;
    nIter = pProbe->aiRowLogEst[saved_nEq] - pProbe->aiRowLogEst[saved_nEq+1];
    pNew->nOut -= nIter;
    /* TUNING:  Because uncertainties in the estimates for skip-scan queries,
    ** add a 1.375 fudge factor to make skip-scan slightly less likely. */
    nIter += 5;
    whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, nIter + nInMul);
    pNew->nOut = saved_nOut;
    pNew->u.btree.nEq = saved_nEq;
    pNew->nSkip = saved_nSkip;
    pNew->wsFlags = saved_wsFlags;
  }

  return rc;
}

/*
** Return True if it is possible that pIndex might be useful in
** implementing the ORDER BY clause in pBuilder.
**
** Return False if pBuilder does not contain an ORDER BY clause or
** if there is no way for pIndex to be useful in implementing that
** ORDER BY clause.
*/
static int indexMightHelpWithOrderBy(
  WhereLoopBuilder *pBuilder,
  Index *pIndex,
  int iCursor
){
  ExprList *pOB;
  int ii, jj;

  if( pIndex->bUnordered ) return 0;
  if( (pOB = pBuilder->pWInfo->pOrderBy)==0 ) return 0;
  for(ii=0; ii<pOB->nExpr; ii++){
    Expr *pExpr = sqlite3ExprSkipCollate(pOB->a[ii].pExpr);
    if( pExpr->op!=TK_COLUMN ) return 0;
    if( pExpr->iTable==iCursor ){
      if( pExpr->iColumn<0 ) return 1;
      for(jj=0; jj<pIndex->nKeyCol; jj++){
        if( pExpr->iColumn==pIndex->aiColumn[jj] ) return 1;
      }
    }
  }
  return 0;
}

/*
** Return a bitmask where 1s indicate that the corresponding column of
** the table is used by an index.  Only the first 63 columns are considered.
*/
static Bitmask columnsInIndex(Index *pIdx){
  Bitmask m = 0;
  int j;
  for(j=pIdx->nColumn-1; j>=0; j--){
    int x = pIdx->aiColumn[j];
    if( x>=0 ){
      testcase( x==BMS-1 );
      testcase( x==BMS-2 );
      if( x<BMS-1 ) m |= MASKBIT(x);
    }
  }
  return m;
}

/* Check to see if a partial index with pPartIndexWhere can be used
** in the current query.  Return true if it can be and false if not.
*/
static int whereUsablePartialIndex(int iTab, WhereClause *pWC, Expr *pWhere){
  int i;
  WhereTerm *pTerm;
  for(i=0, pTerm=pWC->a; i<pWC->nTerm; i++, pTerm++){
    Expr *pExpr = pTerm->pExpr;
    if( sqlite3ExprImpliesExpr(pExpr, pWhere, iTab) 
     && (!ExprHasProperty(pExpr, EP_FromJoin) || pExpr->iRightJoinTable==iTab)
    ){
      return 1;
    }
  }
  return 0;
}

/*
** Add all WhereLoop objects for a single table of the join where the table
** is idenfied by pBuilder->pNew->iTab.  That table is guaranteed to be
** a b-tree table, not a virtual table.
**
** The costs (WhereLoop.rRun) of the b-tree loops added by this function
** are calculated as follows:
**
** For a full scan, assuming the table (or index) contains nRow rows:
**
**     cost = nRow * 3.0                    // full-table scan
**     cost = nRow * K                      // scan of covering index
**     cost = nRow * (K+3.0)                // scan of non-covering index
**
** where K is a value between 1.1 and 3.0 set based on the relative 
** estimated average size of the index and table records.
**
** For an index scan, where nVisit is the number of index rows visited
** by the scan, and nSeek is the number of seek operations required on 
** the index b-tree:
**
**     cost = nSeek * (log(nRow) + K * nVisit)          // covering index
**     cost = nSeek * (log(nRow) + (K+3.0) * nVisit)    // non-covering index
**
** Normally, nSeek is 1. nSeek values greater than 1 come about if the 
** WHERE clause includes "x IN (....)" terms used in place of "x=?". Or when 
** implicit "x IN (SELECT x FROM tbl)" terms are added for skip-scans.
**
** The estimated values (nRow, nVisit, nSeek) often contain a large amount
** of uncertainty.  For this reason, scoring is designed to pick plans that
** "do the least harm" if the estimates are inaccurate.  For example, a
** log(nRow) factor is omitted from a non-covering index scan in order to
** bias the scoring in favor of using an index, since the worst-case
** performance of using an index is far better than the worst-case performance
** of a full table scan.
*/
static int whereLoopAddBtree(
  WhereLoopBuilder *pBuilder, /* WHERE clause information */
  Bitmask mExtra              /* Extra prerequesites for using this table */
){
  WhereInfo *pWInfo;          /* WHERE analysis context */
  Index *pProbe;              /* An index we are evaluating */
  Index sPk;                  /* A fake index object for the primary key */
  LogEst aiRowEstPk[2];       /* The aiRowLogEst[] value for the sPk index */
  i16 aiColumnPk = -1;        /* The aColumn[] value for the sPk index */
  SrcList *pTabList;          /* The FROM clause */
  struct SrcList_item *pSrc;  /* The FROM clause btree term to add */
  WhereLoop *pNew;            /* Template WhereLoop object */
  int rc = SQLITE_OK;         /* Return code */
  int iSortIdx = 1;           /* Index number */
  int b;                      /* A boolean value */
  LogEst rSize;               /* number of rows in the table */
  LogEst rLogSize;            /* Logarithm of the number of rows in the table */
  WhereClause *pWC;           /* The parsed WHERE clause */
  Table *pTab;                /* Table being queried */
  
  pNew = pBuilder->pNew;
  pWInfo = pBuilder->pWInfo;
  pTabList = pWInfo->pTabList;
  pSrc = pTabList->a + pNew->iTab;
  pTab = pSrc->pTab;
  pWC = pBuilder->pWC;
  assert( !IsVirtual(pSrc->pTab) );

  if( pSrc->pIndex ){
    /* An INDEXED BY clause specifies a particular index to use */
    pProbe = pSrc->pIndex;
  }else if( !HasRowid(pTab) ){
    pProbe = pTab->pIndex;
  }else{
    /* There is no INDEXED BY clause.  Create a fake Index object in local
    ** variable sPk to represent the rowid primary key index.  Make this
    ** fake index the first in a chain of Index objects with all of the real
    ** indices to follow */
    Index *pFirst;                  /* First of real indices on the table */
    memset(&sPk, 0, sizeof(Index));
    sPk.nKeyCol = 1;
    sPk.nColumn = 1;
    sPk.aiColumn = &aiColumnPk;
    sPk.aiRowLogEst = aiRowEstPk;
    sPk.onError = OE_Replace;
    sPk.pTable = pTab;
    sPk.szIdxRow = pTab->szTabRow;
    aiRowEstPk[0] = pTab->nRowLogEst;
    aiRowEstPk[1] = 0;
    pFirst = pSrc->pTab->pIndex;
    if( pSrc->notIndexed==0 ){
      /* The real indices of the table are only considered if the
      ** NOT INDEXED qualifier is omitted from the FROM clause */
      sPk.pNext = pFirst;
    }
    pProbe = &sPk;
  }
  rSize = pTab->nRowLogEst;
  rLogSize = estLog(rSize);

#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
  /* Automatic indexes */
  if( !pBuilder->pOrSet   /* Not part of an OR optimization */
   && (pWInfo->wctrlFlags & WHERE_NO_AUTOINDEX)==0
   && (pWInfo->pParse->db->flags & SQLITE_AutoIndex)!=0
   && pSrc->pIndex==0     /* Has no INDEXED BY clause */
   && !pSrc->notIndexed   /* Has no NOT INDEXED clause */
   && HasRowid(pTab)      /* Is not a WITHOUT ROWID table. (FIXME: Why not?) */
   && !pSrc->isCorrelated /* Not a correlated subquery */
   && !pSrc->isRecursive  /* Not a recursive common table expression. */
  ){
    /* Generate auto-index WhereLoops */
    WhereTerm *pTerm;
    WhereTerm *pWCEnd = pWC->a + pWC->nTerm;
    for(pTerm=pWC->a; rc==SQLITE_OK && pTerm<pWCEnd; pTerm++){
      if( pTerm->prereqRight & pNew->maskSelf ) continue;
      if( termCanDriveIndex(pTerm, pSrc, 0) ){
        pNew->u.btree.nEq = 1;
        pNew->nSkip = 0;
        pNew->u.btree.pIndex = 0;
        pNew->nLTerm = 1;
        pNew->aLTerm[0] = pTerm;
        /* TUNING: One-time cost for computing the automatic index is
        ** estimated to be X*N*log2(N) where N is the number of rows in
        ** the table being indexed and where X is 7 (LogEst=28) for normal
        ** tables or 1.375 (LogEst=4) for views and subqueries.  The value
        ** of X is smaller for views and subqueries so that the query planner
        ** will be more aggressive about generating automatic indexes for
        ** those objects, since there is no opportunity to add schema
        ** indexes on subqueries and views. */
        pNew->rSetup = rLogSize + rSize + 4;
        if( pTab->pSelect==0 && (pTab->tabFlags & TF_Ephemeral)==0 ){
          pNew->rSetup += 24;
        }
        ApplyCostMultiplier(pNew->rSetup, pTab->costMult);
        /* TUNING: Each index lookup yields 20 rows in the table.  This
        ** is more than the usual guess of 10 rows, since we have no way
        ** of knowing how selective the index will ultimately be.  It would
        ** not be unreasonable to make this value much larger. */
        pNew->nOut = 43;  assert( 43==sqlite3LogEst(20) );
        pNew->rRun = sqlite3LogEstAdd(rLogSize,pNew->nOut);
        pNew->wsFlags = WHERE_AUTO_INDEX;
        pNew->prereq = mExtra | pTerm->prereqRight;
        rc = whereLoopInsert(pBuilder, pNew);
      }
    }
  }
#endif /* SQLITE_OMIT_AUTOMATIC_INDEX */

  /* Loop over all indices
  */
  for(; rc==SQLITE_OK && pProbe; pProbe=pProbe->pNext, iSortIdx++){
    if( pProbe->pPartIdxWhere!=0
     && !whereUsablePartialIndex(pSrc->iCursor, pWC, pProbe->pPartIdxWhere) ){
      testcase( pNew->iTab!=pSrc->iCursor );  /* See ticket [98d973b8f5] */
      continue;  /* Partial index inappropriate for this query */
    }
    rSize = pProbe->aiRowLogEst[0];
    pNew->u.btree.nEq = 0;
    pNew->nSkip = 0;
    pNew->nLTerm = 0;
    pNew->iSortIdx = 0;
    pNew->rSetup = 0;
    pNew->prereq = mExtra;
    pNew->nOut = rSize;
    pNew->u.btree.pIndex = pProbe;
    b = indexMightHelpWithOrderBy(pBuilder, pProbe, pSrc->iCursor);
    /* The ONEPASS_DESIRED flags never occurs together with ORDER BY */
    assert( (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || b==0 );
    if( pProbe->tnum<=0 ){
      /* Integer primary key index */
      pNew->wsFlags = WHERE_IPK;

      /* Full table scan */
      pNew->iSortIdx = b ? iSortIdx : 0;
      /* TUNING: Cost of full table scan is (N*3.0). */
      pNew->rRun = rSize + 16;
      ApplyCostMultiplier(pNew->rRun, pTab->costMult);
      whereLoopOutputAdjust(pWC, pNew, rSize);
      rc = whereLoopInsert(pBuilder, pNew);
      pNew->nOut = rSize;
      if( rc ) break;
    }else{
      Bitmask m;
      if( pProbe->isCovering ){
        pNew->wsFlags = WHERE_IDX_ONLY | WHERE_INDEXED;
        m = 0;
      }else{
        m = pSrc->colUsed & ~columnsInIndex(pProbe);
        pNew->wsFlags = (m==0) ? (WHERE_IDX_ONLY|WHERE_INDEXED) : WHERE_INDEXED;
      }

      /* Full scan via index */
      if( b
       || !HasRowid(pTab)
       || ( m==0
         && pProbe->bUnordered==0
         && (pProbe->szIdxRow<pTab->szTabRow)
         && (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0
         && sqlite3GlobalConfig.bUseCis
         && OptimizationEnabled(pWInfo->pParse->db, SQLITE_CoverIdxScan)
          )
      ){
        pNew->iSortIdx = b ? iSortIdx : 0;

        /* The cost of visiting the index rows is N*K, where K is
        ** between 1.1 and 3.0, depending on the relative sizes of the
        ** index and table rows. If this is a non-covering index scan,
        ** also add the cost of visiting table rows (N*3.0).  */
        pNew->rRun = rSize + 1 + (15*pProbe->szIdxRow)/pTab->szTabRow;
        if( m!=0 ){
          pNew->rRun = sqlite3LogEstAdd(pNew->rRun, rSize+16);
        }
        ApplyCostMultiplier(pNew->rRun, pTab->costMult);
        whereLoopOutputAdjust(pWC, pNew, rSize);
        rc = whereLoopInsert(pBuilder, pNew);
        pNew->nOut = rSize;
        if( rc ) break;
      }
    }

    rc = whereLoopAddBtreeIndex(pBuilder, pSrc, pProbe, 0);
#ifdef SQLITE_ENABLE_STAT3_OR_STAT4
    sqlite3Stat4ProbeFree(pBuilder->pRec);
    pBuilder->nRecValid = 0;
    pBuilder->pRec = 0;
#endif

    /* If there was an INDEXED BY clause, then only that one index is
    ** considered. */
    if( pSrc->pIndex ) break;
  }
  return rc;
}

#ifndef SQLITE_OMIT_VIRTUALTABLE
/*
** Add all WhereLoop objects for a table of the join identified by
** pBuilder->pNew->iTab.  That table is guaranteed to be a virtual table.
**
** If there are no LEFT or CROSS JOIN joins in the query, both mExtra and
** mUnusable are set to 0. Otherwise, mExtra is a mask of all FROM clause
** entries that occur before the virtual table in the FROM clause and are
** separated from it by at least one LEFT or CROSS JOIN. Similarly, the
** mUnusable mask contains all FROM clause entries that occur after the
** virtual table and are separated from it by at least one LEFT or 
** CROSS JOIN. 
**
** For example, if the query were:
**
**   ... FROM t1, t2 LEFT JOIN t3, t4, vt CROSS JOIN t5, t6;
**
** then mExtra corresponds to (t1, t2) and mUnusable to (t5, t6).
**
** All the tables in mExtra must be scanned before the current virtual 
** table. So any terms for which all prerequisites are satisfied by 
** mExtra may be specified as "usable" in all calls to xBestIndex. 
** Conversely, all tables in mUnusable must be scanned after the current
** virtual table, so any terms for which the prerequisites overlap with
** mUnusable should always be configured as "not-usable" for xBestIndex.
*/
static int whereLoopAddVirtual(
  WhereLoopBuilder *pBuilder,  /* WHERE clause information */
  Bitmask mExtra,              /* Tables that must be scanned before this one */
  Bitmask mUnusable            /* Tables that must be scanned after this one */
){
  WhereInfo *pWInfo;           /* WHERE analysis context */
  Parse *pParse;               /* The parsing context */
  WhereClause *pWC;            /* The WHERE clause */
  struct SrcList_item *pSrc;   /* The FROM clause term to search */
  Table *pTab;
  sqlite3 *db;
  sqlite3_index_info *pIdxInfo;
  struct sqlite3_index_constraint *pIdxCons;
  struct sqlite3_index_constraint_usage *pUsage;
  WhereTerm *pTerm;
  int i, j;
  int iTerm, mxTerm;
  int nConstraint;
  int seenIn = 0;              /* True if an IN operator is seen */
  int seenVar = 0;             /* True if a non-constant constraint is seen */
  int iPhase;                  /* 0: const w/o IN, 1: const, 2: no IN,  2: IN */
  WhereLoop *pNew;
  int rc = SQLITE_OK;

  assert( (mExtra & mUnusable)==0 );
  pWInfo = pBuilder->pWInfo;
  pParse = pWInfo->pParse;
  db = pParse->db;
  pWC = pBuilder->pWC;
  pNew = pBuilder->pNew;
  pSrc = &pWInfo->pTabList->a[pNew->iTab];
  pTab = pSrc->pTab;
  assert( IsVirtual(pTab) );
  pIdxInfo = allocateIndexInfo(pParse, pWC, mUnusable, pSrc,pBuilder->pOrderBy);
  if( pIdxInfo==0 ) return SQLITE_NOMEM;
  pNew->prereq = 0;
  pNew->rSetup = 0;
  pNew->wsFlags = WHERE_VIRTUALTABLE;
  pNew->nLTerm = 0;
  pNew->u.vtab.needFree = 0;
  pUsage = pIdxInfo->aConstraintUsage;
  nConstraint = pIdxInfo->nConstraint;
  if( whereLoopResize(db, pNew, nConstraint) ){
    sqlite3DbFree(db, pIdxInfo);
    return SQLITE_NOMEM;
  }

  for(iPhase=0; iPhase<=3; iPhase++){
    if( !seenIn && (iPhase&1)!=0 ){
      iPhase++;
      if( iPhase>3 ) break;
    }
    if( !seenVar && iPhase>1 ) break;
    pIdxCons = *(struct sqlite3_index_constraint**)&pIdxInfo->aConstraint;
    for(i=0; i<pIdxInfo->nConstraint; i++, pIdxCons++){
      j = pIdxCons->iTermOffset;
      pTerm = &pWC->a[j];
      switch( iPhase ){
        case 0:    /* Constants without IN operator */
          pIdxCons->usable = 0;
          if( (pTerm->eOperator & WO_IN)!=0 ){
            seenIn = 1;
          }
          if( (pTerm->prereqRight & ~mExtra)!=0 ){
            seenVar = 1;
          }else if( (pTerm->eOperator & WO_IN)==0 ){
            pIdxCons->usable = 1;
          }
          break;
        case 1:    /* Constants with IN operators */
          assert( seenIn );
          pIdxCons->usable = (pTerm->prereqRight & ~mExtra)==0;
          break;
        case 2:    /* Variables without IN */
          assert( seenVar );
          pIdxCons->usable = (pTerm->eOperator & WO_IN)==0;
          break;
        default:   /* Variables with IN */
          assert( seenVar && seenIn );
          pIdxCons->usable = 1;
          break;
      }
    }
    memset(pUsage, 0, sizeof(pUsage[0])*pIdxInfo->nConstraint);
    if( pIdxInfo->needToFreeIdxStr ) sqlite3_free(pIdxInfo->idxStr);
    pIdxInfo->idxStr = 0;
    pIdxInfo->idxNum = 0;
    pIdxInfo->needToFreeIdxStr = 0;
    pIdxInfo->orderByConsumed = 0;
    pIdxInfo->estimatedCost = SQLITE_BIG_DBL / (double)2;
    pIdxInfo->estimatedRows = 25;
    rc = vtabBestIndex(pParse, pTab, pIdxInfo);
    if( rc ) goto whereLoopAddVtab_exit;
    pIdxCons = *(struct sqlite3_index_constraint**)&pIdxInfo->aConstraint;
    pNew->prereq = mExtra;
    mxTerm = -1;
    assert( pNew->nLSlot>=nConstraint );
    for(i=0; i<nConstraint; i++) pNew->aLTerm[i] = 0;
    pNew->u.vtab.omitMask = 0;
    for(i=0; i<nConstraint; i++, pIdxCons++){
      if( (iTerm = pUsage[i].argvIndex - 1)>=0 ){
        j = pIdxCons->iTermOffset;
        if( iTerm>=nConstraint
         || j<0
         || j>=pWC->nTerm
         || pNew->aLTerm[iTerm]!=0
        ){
          rc = SQLITE_ERROR;
          sqlite3ErrorMsg(pParse, "%s.xBestIndex() malfunction", pTab->zName);
          goto whereLoopAddVtab_exit;
        }
        testcase( iTerm==nConstraint-1 );
        testcase( j==0 );
        testcase( j==pWC->nTerm-1 );
        pTerm = &pWC->a[j];
        pNew->prereq |= pTerm->prereqRight;
        assert( iTerm<pNew->nLSlot );
        pNew->aLTerm[iTerm] = pTerm;
        if( iTerm>mxTerm ) mxTerm = iTerm;
        testcase( iTerm==15 );
        testcase( iTerm==16 );
        if( iTerm<16 && pUsage[i].omit ) pNew->u.vtab.omitMask |= 1<<iTerm;
        if( (pTerm->eOperator & WO_IN)!=0 ){
          if( pUsage[i].omit==0 ){
            /* Do not attempt to use an IN constraint if the virtual table
            ** says that the equivalent EQ constraint cannot be safely omitted.
            ** If we do attempt to use such a constraint, some rows might be
            ** repeated in the output. */
            break;
          }
          /* A virtual table that is constrained by an IN clause may not
          ** consume the ORDER BY clause because (1) the order of IN terms
          ** is not necessarily related to the order of output terms and
          ** (2) Multiple outputs from a single IN value will not merge
          ** together.  */
          pIdxInfo->orderByConsumed = 0;
        }
      }
    }
    if( i>=nConstraint ){
      pNew->nLTerm = mxTerm+1;
      assert( pNew->nLTerm<=pNew->nLSlot );
      pNew->u.vtab.idxNum = pIdxInfo->idxNum;
      pNew->u.vtab.needFree = pIdxInfo->needToFreeIdxStr;
      pIdxInfo->needToFreeIdxStr = 0;
      pNew->u.vtab.idxStr = pIdxInfo->idxStr;
      pNew->u.vtab.isOrdered = (i8)(pIdxInfo->orderByConsumed ?
                                      pIdxInfo->nOrderBy : 0);
      pNew->rSetup = 0;
      pNew->rRun = sqlite3LogEstFromDouble(pIdxInfo->estimatedCost);
      pNew->nOut = sqlite3LogEst(pIdxInfo->estimatedRows);
      whereLoopInsert(pBuilder, pNew);
      if( pNew->u.vtab.needFree ){
        sqlite3_free(pNew->u.vtab.idxStr);
        pNew->u.vtab.needFree = 0;
      }
    }
  }  

whereLoopAddVtab_exit:
  if( pIdxInfo->needToFreeIdxStr ) sqlite3_free(pIdxInfo->idxStr);
  sqlite3DbFree(db, pIdxInfo);
  return rc;
}
#endif /* SQLITE_OMIT_VIRTUALTABLE */

/*
** Add WhereLoop entries to handle OR terms.  This works for either
** btrees or virtual tables.
*/
static int whereLoopAddOr(
  WhereLoopBuilder *pBuilder, 
  Bitmask mExtra, 
  Bitmask mUnusable
){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  WhereClause *pWC;
  WhereLoop *pNew;
  WhereTerm *pTerm, *pWCEnd;
  int rc = SQLITE_OK;
  int iCur;
  WhereClause tempWC;
  WhereLoopBuilder sSubBuild;
  WhereOrSet sSum, sCur;
  struct SrcList_item *pItem;
  
  pWC = pBuilder->pWC;
  pWCEnd = pWC->a + pWC->nTerm;
  pNew = pBuilder->pNew;
  memset(&sSum, 0, sizeof(sSum));
  pItem = pWInfo->pTabList->a + pNew->iTab;
  iCur = pItem->iCursor;

  for(pTerm=pWC->a; pTerm<pWCEnd && rc==SQLITE_OK; pTerm++){
    if( (pTerm->eOperator & WO_OR)!=0
     && (pTerm->u.pOrInfo->indexable & pNew->maskSelf)!=0 
    ){
      WhereClause * const pOrWC = &pTerm->u.pOrInfo->wc;
      WhereTerm * const pOrWCEnd = &pOrWC->a[pOrWC->nTerm];
      WhereTerm *pOrTerm;
      int once = 1;
      int i, j;
    
      sSubBuild = *pBuilder;
      sSubBuild.pOrderBy = 0;
      sSubBuild.pOrSet = &sCur;

      WHERETRACE(0x200, ("Begin processing OR-clause %p\n", pTerm));
      for(pOrTerm=pOrWC->a; pOrTerm<pOrWCEnd; pOrTerm++){
        if( (pOrTerm->eOperator & WO_AND)!=0 ){
          sSubBuild.pWC = &pOrTerm->u.pAndInfo->wc;
        }else if( pOrTerm->leftCursor==iCur ){
          tempWC.pWInfo = pWC->pWInfo;
          tempWC.pOuter = pWC;
          tempWC.op = TK_AND;
          tempWC.nTerm = 1;
          tempWC.a = pOrTerm;
          sSubBuild.pWC = &tempWC;
        }else{
          continue;
        }
        sCur.n = 0;
#ifdef WHERETRACE_ENABLED
        WHERETRACE(0x200, ("OR-term %d of %p has %d subterms:\n", 
                   (int)(pOrTerm-pOrWC->a), pTerm, sSubBuild.pWC->nTerm));
        if( sqlite3WhereTrace & 0x400 ){
          for(i=0; i<sSubBuild.pWC->nTerm; i++){
            whereTermPrint(&sSubBuild.pWC->a[i], i);
          }
        }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
        if( IsVirtual(pItem->pTab) ){
          rc = whereLoopAddVirtual(&sSubBuild, mExtra, mUnusable);
        }else
#endif
        {
          rc = whereLoopAddBtree(&sSubBuild, mExtra);
        }
        if( rc==SQLITE_OK ){
          rc = whereLoopAddOr(&sSubBuild, mExtra, mUnusable);
        }
        assert( rc==SQLITE_OK || sCur.n==0 );
        if( sCur.n==0 ){
          sSum.n = 0;
          break;
        }else if( once ){
          whereOrMove(&sSum, &sCur);
          once = 0;
        }else{
          WhereOrSet sPrev;
          whereOrMove(&sPrev, &sSum);
          sSum.n = 0;
          for(i=0; i<sPrev.n; i++){
            for(j=0; j<sCur.n; j++){
              whereOrInsert(&sSum, sPrev.a[i].prereq | sCur.a[j].prereq,
                            sqlite3LogEstAdd(sPrev.a[i].rRun, sCur.a[j].rRun),
                            sqlite3LogEstAdd(sPrev.a[i].nOut, sCur.a[j].nOut));
            }
          }
        }
      }
      pNew->nLTerm = 1;
      pNew->aLTerm[0] = pTerm;
      pNew->wsFlags = WHERE_MULTI_OR;
      pNew->rSetup = 0;
      pNew->iSortIdx = 0;
      memset(&pNew->u, 0, sizeof(pNew->u));
      for(i=0; rc==SQLITE_OK && i<sSum.n; i++){
        /* TUNING: Currently sSum.a[i].rRun is set to the sum of the costs
        ** of all sub-scans required by the OR-scan. However, due to rounding
        ** errors, it may be that the cost of the OR-scan is equal to its
        ** most expensive sub-scan. Add the smallest possible penalty 
        ** (equivalent to multiplying the cost by 1.07) to ensure that 
        ** this does not happen. Otherwise, for WHERE clauses such as the
        ** following where there is an index on "y":
        **
        **     WHERE likelihood(x=?, 0.99) OR y=?
        **
        ** the planner may elect to "OR" together a full-table scan and an
        ** index lookup. And other similarly odd results.  */
        pNew->rRun = sSum.a[i].rRun + 1;
        pNew->nOut = sSum.a[i].nOut;
        pNew->prereq = sSum.a[i].prereq;
        rc = whereLoopInsert(pBuilder, pNew);
      }
      WHERETRACE(0x200, ("End processing OR-clause %p\n", pTerm));
    }
  }
  return rc;
}

/*
** Add all WhereLoop objects for all tables 
*/
static int whereLoopAddAll(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo = pBuilder->pWInfo;
  Bitmask mExtra = 0;
  Bitmask mPrior = 0;
  int iTab;
  SrcList *pTabList = pWInfo->pTabList;
  struct SrcList_item *pItem;
  struct SrcList_item *pEnd = &pTabList->a[pWInfo->nLevel];
  sqlite3 *db = pWInfo->pParse->db;
  int rc = SQLITE_OK;
  WhereLoop *pNew;
  u8 priorJointype = 0;

  /* Loop over the tables in the join, from left to right */
  pNew = pBuilder->pNew;
  whereLoopInit(pNew);
  for(iTab=0, pItem=pTabList->a; pItem<pEnd; iTab++, pItem++){
    Bitmask mUnusable = 0;
    pNew->iTab = iTab;
    pNew->maskSelf = sqlite3WhereGetMask(&pWInfo->sMaskSet, pItem->iCursor);
    if( ((pItem->jointype|priorJointype) & (JT_LEFT|JT_CROSS))!=0 ){
      /* This condition is true when pItem is the FROM clause term on the
      ** right-hand-side of a LEFT or CROSS JOIN.  */
      mExtra = mPrior;
    }
    priorJointype = pItem->jointype;
    if( IsVirtual(pItem->pTab) ){
      struct SrcList_item *p;
      for(p=&pItem[1]; p<pEnd; p++){
        if( mUnusable || (p->jointype & (JT_LEFT|JT_CROSS)) ){
          mUnusable |= sqlite3WhereGetMask(&pWInfo->sMaskSet, p->iCursor);
        }
      }
      rc = whereLoopAddVirtual(pBuilder, mExtra, mUnusable);
    }else{
      rc = whereLoopAddBtree(pBuilder, mExtra);
    }
    if( rc==SQLITE_OK ){
      rc = whereLoopAddOr(pBuilder, mExtra, mUnusable);
    }
    mPrior |= pNew->maskSelf;
    if( rc || db->mallocFailed ) break;
  }

  whereLoopClear(db, pNew);
  return rc;
}

/*
** Examine a WherePath (with the addition of the extra WhereLoop of the 5th
** parameters) to see if it outputs rows in the requested ORDER BY
** (or GROUP BY) without requiring a separate sort operation.  Return N:
** 
**   N>0:   N terms of the ORDER BY clause are satisfied
**   N==0:  No terms of the ORDER BY clause are satisfied
**   N<0:   Unknown yet how many terms of ORDER BY might be satisfied.   
**
** Note that processing for WHERE_GROUPBY and WHERE_DISTINCTBY is not as
** strict.  With GROUP BY and DISTINCT the only requirement is that
** equivalent rows appear immediately adjacent to one another.  GROUP BY
** and DISTINCT do not require rows to appear in any particular order as long
** as equivalent rows are grouped together.  Thus for GROUP BY and DISTINCT
** the pOrderBy terms can be matched in any order.  With ORDER BY, the 
** pOrderBy terms must be matched in strict left-to-right order.
*/
static i8 wherePathSatisfiesOrderBy(
  WhereInfo *pWInfo,    /* The WHERE clause */
  ExprList *pOrderBy,   /* ORDER BY or GROUP BY or DISTINCT clause to check */
  WherePath *pPath,     /* The WherePath to check */
  u16 wctrlFlags,       /* Might contain WHERE_GROUPBY or WHERE_DISTINCTBY */
  u16 nLoop,            /* Number of entries in pPath->aLoop[] */
  WhereLoop *pLast,     /* Add this WhereLoop to the end of pPath->aLoop[] */
  Bitmask *pRevMask     /* OUT: Mask of WhereLoops to run in reverse order */
){
  u8 revSet;            /* True if rev is known */
  u8 rev;               /* Composite sort order */
  u8 revIdx;            /* Index sort order */
  u8 isOrderDistinct;   /* All prior WhereLoops are order-distinct */
  u8 distinctColumns;   /* True if the loop has UNIQUE NOT NULL columns */
  u8 isMatch;           /* iColumn matches a term of the ORDER BY clause */
  u16 nKeyCol;          /* Number of key columns in pIndex */
  u16 nColumn;          /* Total number of ordered columns in the index */
  u16 nOrderBy;         /* Number terms in the ORDER BY clause */
  int iLoop;            /* Index of WhereLoop in pPath being processed */
  int i, j;             /* Loop counters */
  int iCur;             /* Cursor number for current WhereLoop */
  int iColumn;          /* A column number within table iCur */
  WhereLoop *pLoop = 0; /* Current WhereLoop being processed. */
  WhereTerm *pTerm;     /* A single term of the WHERE clause */
  Expr *pOBExpr;        /* An expression from the ORDER BY clause */
  CollSeq *pColl;       /* COLLATE function from an ORDER BY clause term */
  Index *pIndex;        /* The index associated with pLoop */
  sqlite3 *db = pWInfo->pParse->db;  /* Database connection */
  Bitmask obSat = 0;    /* Mask of ORDER BY terms satisfied so far */
  Bitmask obDone;       /* Mask of all ORDER BY terms */
  Bitmask orderDistinctMask;  /* Mask of all well-ordered loops */
  Bitmask ready;              /* Mask of inner loops */

  /*
  ** We say the WhereLoop is "one-row" if it generates no more than one
  ** row of output.  A WhereLoop is one-row if all of the following are true:
  **  (a) All index columns match with WHERE_COLUMN_EQ.
  **  (b) The index is unique
  ** Any WhereLoop with an WHERE_COLUMN_EQ constraint on the rowid is one-row.
  ** Every one-row WhereLoop will have the WHERE_ONEROW bit set in wsFlags.
  **
  ** We say the WhereLoop is "order-distinct" if the set of columns from
  ** that WhereLoop that are in the ORDER BY clause are different for every
  ** row of the WhereLoop.  Every one-row WhereLoop is automatically
  ** order-distinct.   A WhereLoop that has no columns in the ORDER BY clause
  ** is not order-distinct. To be order-distinct is not quite the same as being
  ** UNIQUE since a UNIQUE column or index can have multiple rows that 
  ** are NULL and NULL values are equivalent for the purpose of order-distinct.
  ** To be order-distinct, the columns must be UNIQUE and NOT NULL.
  **
  ** The rowid for a table is always UNIQUE and NOT NULL so whenever the
  ** rowid appears in the ORDER BY clause, the corresponding WhereLoop is
  ** automatically order-distinct.
  */

  assert( pOrderBy!=0 );
  if( nLoop && OptimizationDisabled(db, SQLITE_OrderByIdxJoin) ) return 0;

  nOrderBy = pOrderBy->nExpr;
  testcase( nOrderBy==BMS-1 );
  if( nOrderBy>BMS-1 ) return 0;  /* Cannot optimize overly large ORDER BYs */
  isOrderDistinct = 1;
  obDone = MASKBIT(nOrderBy)-1;
  orderDistinctMask = 0;
  ready = 0;
  for(iLoop=0; isOrderDistinct && obSat<obDone && iLoop<=nLoop; iLoop++){
    if( iLoop>0 ) ready |= pLoop->maskSelf;
    pLoop = iLoop<nLoop ? pPath->aLoop[iLoop] : pLast;
    if( pLoop->wsFlags & WHERE_VIRTUALTABLE ){
      if( pLoop->u.vtab.isOrdered ) obSat = obDone;
      break;
    }
    iCur = pWInfo->pTabList->a[pLoop->iTab].iCursor;

    /* Mark off any ORDER BY term X that is a column in the table of
    ** the current loop for which there is term in the WHERE
    ** clause of the form X IS NULL or X=? that reference only outer
    ** loops.
    */
    for(i=0; i<nOrderBy; i++){
      if( MASKBIT(i) & obSat ) continue;
      pOBExpr = sqlite3ExprSkipCollate(pOrderBy->a[i].pExpr);
      if( pOBExpr->op!=TK_COLUMN ) continue;
      if( pOBExpr->iTable!=iCur ) continue;
      pTerm = sqlite3WhereFindTerm(&pWInfo->sWC, iCur, pOBExpr->iColumn,
                       ~ready, WO_EQ|WO_ISNULL|WO_IS, 0);
      if( pTerm==0 ) continue;
      if( (pTerm->eOperator&(WO_EQ|WO_IS))!=0 && pOBExpr->iColumn>=0 ){
        const char *z1, *z2;
        pColl = sqlite3ExprCollSeq(pWInfo->pParse, pOrderBy->a[i].pExpr);
        if( !pColl ) pColl = db->pDfltColl;
        z1 = pColl->zName;
        pColl = sqlite3ExprCollSeq(pWInfo->pParse, pTerm->pExpr);
        if( !pColl ) pColl = db->pDfltColl;
        z2 = pColl->zName;
        if( sqlite3StrICmp(z1, z2)!=0 ) continue;
        testcase( pTerm->pExpr->op==TK_IS );
      }
      obSat |= MASKBIT(i);
    }

    if( (pLoop->wsFlags & WHERE_ONEROW)==0 ){
      if( pLoop->wsFlags & WHERE_IPK ){
        pIndex = 0;
        nKeyCol = 0;
        nColumn = 1;
      }else if( (pIndex = pLoop->u.btree.pIndex)==0 || pIndex->bUnordered ){
        return 0;
      }else{
        nKeyCol = pIndex->nKeyCol;
        nColumn = pIndex->nColumn;
        assert( nColumn==nKeyCol+1 || !HasRowid(pIndex->pTable) );
        assert( pIndex->aiColumn[nColumn-1]==(-1) || !HasRowid(pIndex->pTable));
        isOrderDistinct = IsUniqueIndex(pIndex);
      }

      /* Loop through all columns of the index and deal with the ones
      ** that are not constrained by == or IN.
      */
      rev = revSet = 0;
      distinctColumns = 0;
      for(j=0; j<nColumn; j++){
        u8 bOnce;   /* True to run the ORDER BY search loop */

        /* Skip over == and IS NULL terms */
        if( j<pLoop->u.btree.nEq
         && pLoop->nSkip==0
         && ((i = pLoop->aLTerm[j]->eOperator) & (WO_EQ|WO_ISNULL|WO_IS))!=0
        ){
          if( i & WO_ISNULL ){
            testcase( isOrderDistinct );
            isOrderDistinct = 0;
          }
          continue;  
        }

        /* Get the column number in the table (iColumn) and sort order
        ** (revIdx) for the j-th column of the index.
        */
        if( pIndex ){
          iColumn = pIndex->aiColumn[j];
          revIdx = pIndex->aSortOrder[j];
          if( iColumn==pIndex->pTable->iPKey ) iColumn = -1;
        }else{
          iColumn = -1;
          revIdx = 0;
        }

        /* An unconstrained column that might be NULL means that this
        ** WhereLoop is not well-ordered
        */
        if( isOrderDistinct
         && iColumn>=0
         && j>=pLoop->u.btree.nEq
         && pIndex->pTable->aCol[iColumn].notNull==0
        ){
          isOrderDistinct = 0;
        }

        /* Find the ORDER BY term that corresponds to the j-th column
        ** of the index and mark that ORDER BY term off 
        */
        bOnce = 1;
        isMatch = 0;
        for(i=0; bOnce && i<nOrderBy; i++){
          if( MASKBIT(i) & obSat ) continue;
          pOBExpr = sqlite3ExprSkipCollate(pOrderBy->a[i].pExpr);
          testcase( wctrlFlags & WHERE_GROUPBY );
          testcase( wctrlFlags & WHERE_DISTINCTBY );
          if( (wctrlFlags & (WHERE_GROUPBY|WHERE_DISTINCTBY))==0 ) bOnce = 0;
          if( pOBExpr->op!=TK_COLUMN ) continue;
          if( pOBExpr->iTable!=iCur ) continue;
          if( pOBExpr->iColumn!=iColumn ) continue;
          if( iColumn>=0 ){
            pColl = sqlite3ExprCollSeq(pWInfo->pParse, pOrderBy->a[i].pExpr);
            if( !pColl ) pColl = db->pDfltColl;
            if( sqlite3StrICmp(pColl->zName, pIndex->azColl[j])!=0 ) continue;
          }
          isMatch = 1;
          break;
        }
        if( isMatch && (wctrlFlags & WHERE_GROUPBY)==0 ){
          /* Make sure the sort order is compatible in an ORDER BY clause.
          ** Sort order is irrelevant for a GROUP BY clause. */
          if( revSet ){
            if( (rev ^ revIdx)!=pOrderBy->a[i].sortOrder ) isMatch = 0;
          }else{
            rev = revIdx ^ pOrderBy->a[i].sortOrder;
            if( rev ) *pRevMask |= MASKBIT(iLoop);
            revSet = 1;
          }
        }
        if( isMatch ){
          if( iColumn<0 ){
            testcase( distinctColumns==0 );
            distinctColumns = 1;
          }
          obSat |= MASKBIT(i);
        }else{
          /* No match found */
          if( j==0 || j<nKeyCol ){
            testcase( isOrderDistinct!=0 );
            isOrderDistinct = 0;
          }
          break;
        }
      } /* end Loop over all index columns */
      if( distinctColumns ){
        testcase( isOrderDistinct==0 );
        isOrderDistinct = 1;
      }
    } /* end-if not one-row */

    /* Mark off any other ORDER BY terms that reference pLoop */
    if( isOrderDistinct ){
      orderDistinctMask |= pLoop->maskSelf;
      for(i=0; i<nOrderBy; i++){
        Expr *p;
        Bitmask mTerm;
        if( MASKBIT(i) & obSat ) continue;
        p = pOrderBy->a[i].pExpr;
        mTerm = sqlite3WhereExprUsage(&pWInfo->sMaskSet,p);
        if( mTerm==0 && !sqlite3ExprIsConstant(p) ) continue;
        if( (mTerm&~orderDistinctMask)==0 ){
          obSat |= MASKBIT(i);
        }
      }
    }
  } /* End the loop over all WhereLoops from outer-most down to inner-most */
  if( obSat==obDone ) return (i8)nOrderBy;
  if( !isOrderDistinct ){
    for(i=nOrderBy-1; i>0; i--){
      Bitmask m = MASKBIT(i) - 1;
      if( (obSat&m)==m ) return i;
    }
    return 0;
  }
  return -1;
}


/*
** If the WHERE_GROUPBY flag is set in the mask passed to sqlite3WhereBegin(),
** the planner assumes that the specified pOrderBy list is actually a GROUP
** BY clause - and so any order that groups rows as required satisfies the
** request.
**
** Normally, in this case it is not possible for the caller to determine
** whether or not the rows are really being delivered in sorted order, or
** just in some other order that provides the required grouping. However,
** if the WHERE_SORTBYGROUP flag is also passed to sqlite3WhereBegin(), then
** this function may be called on the returned WhereInfo object. It returns
** true if the rows really will be sorted in the specified order, or false
** otherwise.
**
** For example, assuming:
**
**   CREATE INDEX i1 ON t1(x, Y);
**
** then
**
**   SELECT * FROM t1 GROUP BY x,y ORDER BY x,y;   -- IsSorted()==1
**   SELECT * FROM t1 GROUP BY y,x ORDER BY y,x;   -- IsSorted()==0
*/
SQLITE_PRIVATE int sqlite3WhereIsSorted(WhereInfo *pWInfo){
  assert( pWInfo->wctrlFlags & WHERE_GROUPBY );
  assert( pWInfo->wctrlFlags & WHERE_SORTBYGROUP );
  return pWInfo->sorted;
}

#ifdef WHERETRACE_ENABLED
/* For debugging use only: */
static const char *wherePathName(WherePath *pPath, int nLoop, WhereLoop *pLast){
  static char zName[65];
  int i;
  for(i=0; i<nLoop; i++){ zName[i] = pPath->aLoop[i]->cId; }
  if( pLast ) zName[i++] = pLast->cId;
  zName[i] = 0;
  return zName;
}
#endif

/*
** Return the cost of sorting nRow rows, assuming that the keys have 
** nOrderby columns and that the first nSorted columns are already in
** order.
*/
static LogEst whereSortingCost(
  WhereInfo *pWInfo,
  LogEst nRow,
  int nOrderBy,
  int nSorted
){
  /* TUNING: Estimated cost of a full external sort, where N is 
  ** the number of rows to sort is:
  **
  **   cost = (3.0 * N * log(N)).
  ** 
  ** Or, if the order-by clause has X terms but only the last Y 
  ** terms are out of order, then block-sorting will reduce the 
  ** sorting cost to:
  **
  **   cost = (3.0 * N * log(N)) * (Y/X)
  **
  ** The (Y/X) term is implemented using stack variable rScale
  ** below.  */
  LogEst rScale, rSortCost;
  assert( nOrderBy>0 && 66==sqlite3LogEst(100) );
  rScale = sqlite3LogEst((nOrderBy-nSorted)*100/nOrderBy) - 66;
  rSortCost = nRow + estLog(nRow) + rScale + 16;

  /* TUNING: The cost of implementing DISTINCT using a B-TREE is
  ** similar but with a larger constant of proportionality. 
  ** Multiply by an additional factor of 3.0.  */
  if( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
    rSortCost += 16;
  }

  return rSortCost;
}

/*
** Given the list of WhereLoop objects at pWInfo->pLoops, this routine
** attempts to find the lowest cost path that visits each WhereLoop
** once.  This path is then loaded into the pWInfo->a[].pWLoop fields.
**
** Assume that the total number of output rows that will need to be sorted
** will be nRowEst (in the 10*log2 representation).  Or, ignore sorting
** costs if nRowEst==0.
**
** Return SQLITE_OK on success or SQLITE_NOMEM of a memory allocation
** error occurs.
*/
static int wherePathSolver(WhereInfo *pWInfo, LogEst nRowEst){
  int mxChoice;             /* Maximum number of simultaneous paths tracked */
  int nLoop;                /* Number of terms in the join */
  Parse *pParse;            /* Parsing context */
  sqlite3 *db;              /* The database connection */
  int iLoop;                /* Loop counter over the terms of the join */
  int ii, jj;               /* Loop counters */
  int mxI = 0;              /* Index of next entry to replace */
  int nOrderBy;             /* Number of ORDER BY clause terms */
  LogEst mxCost = 0;        /* Maximum cost of a set of paths */
  LogEst mxUnsorted = 0;    /* Maximum unsorted cost of a set of path */
  int nTo, nFrom;           /* Number of valid entries in aTo[] and aFrom[] */
  WherePath *aFrom;         /* All nFrom paths at the previous level */
  WherePath *aTo;           /* The nTo best paths at the current level */
  WherePath *pFrom;         /* An element of aFrom[] that we are working on */
  WherePath *pTo;           /* An element of aTo[] that we are working on */
  WhereLoop *pWLoop;        /* One of the WhereLoop objects */
  WhereLoop **pX;           /* Used to divy up the pSpace memory */
  LogEst *aSortCost = 0;    /* Sorting and partial sorting costs */
  char *pSpace;             /* Temporary memory used by this routine */
  int nSpace;               /* Bytes of space allocated at pSpace */

  pParse = pWInfo->pParse;
  db = pParse->db;
  nLoop = pWInfo->nLevel;
  /* TUNING: For simple queries, only the best path is tracked.
  ** For 2-way joins, the 5 best paths are followed.
  ** For joins of 3 or more tables, track the 10 best paths */
  mxChoice = (nLoop<=1) ? 1 : (nLoop==2 ? 5 : 10);
  assert( nLoop<=pWInfo->pTabList->nSrc );
  WHERETRACE(0x002, ("---- begin solver.  (nRowEst=%d)\n", nRowEst));

  /* If nRowEst is zero and there is an ORDER BY clause, ignore it. In this
  ** case the purpose of this call is to estimate the number of rows returned
  ** by the overall query. Once this estimate has been obtained, the caller
  ** will invoke this function a second time, passing the estimate as the
  ** nRowEst parameter.  */
  if( pWInfo->pOrderBy==0 || nRowEst==0 ){
    nOrderBy = 0;
  }else{
    nOrderBy = pWInfo->pOrderBy->nExpr;
  }

  /* Allocate and initialize space for aTo, aFrom and aSortCost[] */
  nSpace = (sizeof(WherePath)+sizeof(WhereLoop*)*nLoop)*mxChoice*2;
  nSpace += sizeof(LogEst) * nOrderBy;
  pSpace = sqlite3DbMallocRaw(db, nSpace);
  if( pSpace==0 ) return SQLITE_NOMEM;
  aTo = (WherePath*)pSpace;
  aFrom = aTo+mxChoice;
  memset(aFrom, 0, sizeof(aFrom[0]));
  pX = (WhereLoop**)(aFrom+mxChoice);
  for(ii=mxChoice*2, pFrom=aTo; ii>0; ii--, pFrom++, pX += nLoop){
    pFrom->aLoop = pX;
  }
  if( nOrderBy ){
    /* If there is an ORDER BY clause and it is not being ignored, set up
    ** space for the aSortCost[] array. Each element of the aSortCost array
    ** is either zero - meaning it has not yet been initialized - or the
    ** cost of sorting nRowEst rows of data where the first X terms of
    ** the ORDER BY clause are already in order, where X is the array 
    ** index.  */
    aSortCost = (LogEst*)pX;
    memset(aSortCost, 0, sizeof(LogEst) * nOrderBy);
  }
  assert( aSortCost==0 || &pSpace[nSpace]==(char*)&aSortCost[nOrderBy] );
  assert( aSortCost!=0 || &pSpace[nSpace]==(char*)pX );

  /* Seed the search with a single WherePath containing zero WhereLoops.
  **
  ** TUNING: Do not let the number of iterations go above 28.  If the cost
  ** of computing an automatic index is not paid back within the first 28
  ** rows, then do not use the automatic index. */
  aFrom[0].nRow = MIN(pParse->nQueryLoop, 48);  assert( 48==sqlite3LogEst(28) );
  nFrom = 1;
  assert( aFrom[0].isOrdered==0 );
  if( nOrderBy ){
    /* If nLoop is zero, then there are no FROM terms in the query. Since
    ** in this case the query may return a maximum of one row, the results
    ** are already in the requested order. Set isOrdered to nOrderBy to
    ** indicate this. Or, if nLoop is greater than zero, set isOrdered to
    ** -1, indicating that the result set may or may not be ordered, 
    ** depending on the loops added to the current plan.  */
    aFrom[0].isOrdered = nLoop>0 ? -1 : nOrderBy;
  }

  /* Compute successively longer WherePaths using the previous generation
  ** of WherePaths as the basis for the next.  Keep track of the mxChoice
  ** best paths at each generation */
  for(iLoop=0; iLoop<nLoop; iLoop++){
    nTo = 0;
    for(ii=0, pFrom=aFrom; ii<nFrom; ii++, pFrom++){
      for(pWLoop=pWInfo->pLoops; pWLoop; pWLoop=pWLoop->pNextLoop){
        LogEst nOut;                      /* Rows visited by (pFrom+pWLoop) */
        LogEst rCost;                     /* Cost of path (pFrom+pWLoop) */
        LogEst rUnsorted;                 /* Unsorted cost of (pFrom+pWLoop) */
        i8 isOrdered = pFrom->isOrdered;  /* isOrdered for (pFrom+pWLoop) */
        Bitmask maskNew;                  /* Mask of src visited by (..) */
        Bitmask revMask = 0;              /* Mask of rev-order loops for (..) */

        if( (pWLoop->prereq & ~pFrom->maskLoop)!=0 ) continue;
        if( (pWLoop->maskSelf & pFrom->maskLoop)!=0 ) continue;
        /* At this point, pWLoop is a candidate to be the next loop. 
        ** Compute its cost */
        rUnsorted = sqlite3LogEstAdd(pWLoop->rSetup,pWLoop->rRun + pFrom->nRow);
        rUnsorted = sqlite3LogEstAdd(rUnsorted, pFrom->rUnsorted);
        nOut = pFrom->nRow + pWLoop->nOut;
        maskNew = pFrom->maskLoop | pWLoop->maskSelf;
        if( isOrdered<0 ){
          isOrdered = wherePathSatisfiesOrderBy(pWInfo,
                       pWInfo->pOrderBy, pFrom, pWInfo->wctrlFlags,
                       iLoop, pWLoop, &revMask);
        }else{
          revMask = pFrom->revLoop;
        }
        if( isOrdered>=0 && isOrdered<nOrderBy ){
          if( aSortCost[isOrdered]==0 ){
            aSortCost[isOrdered] = whereSortingCost(
                pWInfo, nRowEst, nOrderBy, isOrdered
            );
          }
          rCost = sqlite3LogEstAdd(rUnsorted, aSortCost[isOrdered]);

          WHERETRACE(0x002,
              ("---- sort cost=%-3d (%d/%d) increases cost %3d to %-3d\n",
               aSortCost[isOrdered], (nOrderBy-isOrdered), nOrderBy, 
               rUnsorted, rCost));
        }else{
          rCost = rUnsorted;
        }

        /* Check to see if pWLoop should be added to the set of
        ** mxChoice best-so-far paths.
        **
        ** First look for an existing path among best-so-far paths
        ** that covers the same set of loops and has the same isOrdered
        ** setting as the current path candidate.
        **
        ** The term "((pTo->isOrdered^isOrdered)&0x80)==0" is equivalent
        ** to (pTo->isOrdered==(-1))==(isOrdered==(-1))" for the range
        ** of legal values for isOrdered, -1..64.
        */
        for(jj=0, pTo=aTo; jj<nTo; jj++, pTo++){
          if( pTo->maskLoop==maskNew
           && ((pTo->isOrdered^isOrdered)&0x80)==0
          ){
            testcase( jj==nTo-1 );
            break;
          }
        }
        if( jj>=nTo ){
          /* None of the existing best-so-far paths match the candidate. */
          if( nTo>=mxChoice
           && (rCost>mxCost || (rCost==mxCost && rUnsorted>=mxUnsorted))
          ){
            /* The current candidate is no better than any of the mxChoice
            ** paths currently in the best-so-far buffer.  So discard
            ** this candidate as not viable. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
            if( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf("Skip   %s cost=%-3d,%3d order=%c\n",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut,
                  isOrdered>=0 ? isOrdered+'0' : '?');
            }
#endif
            continue;
          }
          /* If we reach this points it means that the new candidate path
          ** needs to be added to the set of best-so-far paths. */
          if( nTo<mxChoice ){
            /* Increase the size of the aTo set by one */
            jj = nTo++;
          }else{
            /* New path replaces the prior worst to keep count below mxChoice */
            jj = mxI;
          }
          pTo = &aTo[jj];
#ifdef WHERETRACE_ENABLED /* 0x4 */
          if( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf("New    %s cost=%-3d,%3d order=%c\n",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut,
                isOrdered>=0 ? isOrdered+'0' : '?');
          }
#endif
        }else{
          /* Control reaches here if best-so-far path pTo=aTo[jj] covers the
          ** same set of loops and has the sam isOrdered setting as the
          ** candidate path.  Check to see if the candidate should replace
          ** pTo or if the candidate should be skipped */
          if( pTo->rCost<rCost || (pTo->rCost==rCost && pTo->nRow<=nOut) ){
#ifdef WHERETRACE_ENABLED /* 0x4 */
            if( sqlite3WhereTrace&0x4 ){
              sqlite3DebugPrintf(
                  "Skip   %s cost=%-3d,%3d order=%c",
                  wherePathName(pFrom, iLoop, pWLoop), rCost, nOut,
                  isOrdered>=0 ? isOrdered+'0' : '?');
              sqlite3DebugPrintf("   vs %s cost=%-3d,%d order=%c\n",
                  wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
                  pTo->isOrdered>=0 ? pTo->isOrdered+'0' : '?');
            }
#endif
            /* Discard the candidate path from further consideration */
            testcase( pTo->rCost==rCost );
            continue;
          }
          testcase( pTo->rCost==rCost+1 );
          /* Control reaches here if the candidate path is better than the
          ** pTo path.  Replace pTo with the candidate. */
#ifdef WHERETRACE_ENABLED /* 0x4 */
          if( sqlite3WhereTrace&0x4 ){
            sqlite3DebugPrintf(
                "Update %s cost=%-3d,%3d order=%c",
                wherePathName(pFrom, iLoop, pWLoop), rCost, nOut,
                isOrdered>=0 ? isOrdered+'0' : '?');
            sqlite3DebugPrintf("  was %s cost=%-3d,%3d order=%c\n",
                wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
                pTo->isOrdered>=0 ? pTo->isOrdered+'0' : '?');
          }
#endif
        }
        /* pWLoop is a winner.  Add it to the set of best so far */
        pTo->maskLoop = pFrom->maskLoop | pWLoop->maskSelf;
        pTo->revLoop = revMask;
        pTo->nRow = nOut;
        pTo->rCost = rCost;
        pTo->rUnsorted = rUnsorted;
        pTo->isOrdered = isOrdered;
        memcpy(pTo->aLoop, pFrom->aLoop, sizeof(WhereLoop*)*iLoop);
        pTo->aLoop[iLoop] = pWLoop;
        if( nTo>=mxChoice ){
          mxI = 0;
          mxCost = aTo[0].rCost;
          mxUnsorted = aTo[0].nRow;
          for(jj=1, pTo=&aTo[1]; jj<mxChoice; jj++, pTo++){
            if( pTo->rCost>mxCost 
             || (pTo->rCost==mxCost && pTo->rUnsorted>mxUnsorted) 
            ){
              mxCost = pTo->rCost;
              mxUnsorted = pTo->rUnsorted;
              mxI = jj;
            }
          }
        }
      }
    }

#ifdef WHERETRACE_ENABLED  /* >=2 */
    if( sqlite3WhereTrace & 0x02 ){
      sqlite3DebugPrintf("---- after round %d ----\n", iLoop);
      for(ii=0, pTo=aTo; ii<nTo; ii++, pTo++){
        sqlite3DebugPrintf(" %s cost=%-3d nrow=%-3d order=%c",
           wherePathName(pTo, iLoop+1, 0), pTo->rCost, pTo->nRow,
           pTo->isOrdered>=0 ? (pTo->isOrdered+'0') : '?');
        if( pTo->isOrdered>0 ){
          sqlite3DebugPrintf(" rev=0x%llx\n", pTo->revLoop);
        }else{
          sqlite3DebugPrintf("\n");
        }
      }
    }
#endif

    /* Swap the roles of aFrom and aTo for the next generation */
    pFrom = aTo;
    aTo = aFrom;
    aFrom = pFrom;
    nFrom = nTo;
  }

  if( nFrom==0 ){
    sqlite3ErrorMsg(pParse, "no query solution");
    sqlite3DbFree(db, pSpace);
    return SQLITE_ERROR;
  }
  
  /* Find the lowest cost path.  pFrom will be left pointing to that path */
  pFrom = aFrom;
  for(ii=1; ii<nFrom; ii++){
    if( pFrom->rCost>aFrom[ii].rCost ) pFrom = &aFrom[ii];
  }
  assert( pWInfo->nLevel==nLoop );
  /* Load the lowest cost path into pWInfo */
  for(iLoop=0; iLoop<nLoop; iLoop++){
    WhereLevel *pLevel = pWInfo->a + iLoop;
    pLevel->pWLoop = pWLoop = pFrom->aLoop[iLoop];
    pLevel->iFrom = pWLoop->iTab;
    pLevel->iTabCur = pWInfo->pTabList->a[pLevel->iFrom].iCursor;
  }
  if( (pWInfo->wctrlFlags & WHERE_WANT_DISTINCT)!=0
   && (pWInfo->wctrlFlags & WHERE_DISTINCTBY)==0
   && pWInfo->eDistinct==WHERE_DISTINCT_NOOP
   && nRowEst
  ){
    Bitmask notUsed;
    int rc = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pResultSet, pFrom,
                 WHERE_DISTINCTBY, nLoop-1, pFrom->aLoop[nLoop-1], &notUsed);
    if( rc==pWInfo->pResultSet->nExpr ){
      pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
    }
  }
  if( pWInfo->pOrderBy ){
    if( pWInfo->wctrlFlags & WHERE_DISTINCTBY ){
      if( pFrom->isOrdered==pWInfo->pOrderBy->nExpr ){
        pWInfo->eDistinct = WHERE_DISTINCT_ORDERED;
      }
    }else{
      pWInfo->nOBSat = pFrom->isOrdered;
      if( pWInfo->nOBSat<0 ) pWInfo->nOBSat = 0;
      pWInfo->revMask = pFrom->revLoop;
    }
    if( (pWInfo->wctrlFlags & WHERE_SORTBYGROUP)
        && pWInfo->nOBSat==pWInfo->pOrderBy->nExpr && nLoop>0
    ){
      Bitmask revMask = 0;
      int nOrder = wherePathSatisfiesOrderBy(pWInfo, pWInfo->pOrderBy, 
          pFrom, 0, nLoop-1, pFrom->aLoop[nLoop-1], &revMask
      );
      assert( pWInfo->sorted==0 );
      if( nOrder==pWInfo->pOrderBy->nExpr ){
        pWInfo->sorted = 1;
        pWInfo->revMask = revMask;
      }
    }
  }


  pWInfo->nRowOut = pFrom->nRow;

  /* Free temporary memory and return success */
  sqlite3DbFree(db, pSpace);
  return SQLITE_OK;
}

/*
** Most queries use only a single table (they are not joins) and have
** simple == constraints against indexed fields.  This routine attempts
** to plan those simple cases using much less ceremony than the
** general-purpose query planner, and thereby yield faster sqlite3_prepare()
** times for the common case.
**
** Return non-zero on success, if this query can be handled by this
** no-frills query planner.  Return zero if this query needs the 
** general-purpose query planner.
*/
static int whereShortCut(WhereLoopBuilder *pBuilder){
  WhereInfo *pWInfo;
  struct SrcList_item *pItem;
  WhereClause *pWC;
  WhereTerm *pTerm;
  WhereLoop *pLoop;
  int iCur;
  int j;
  Table *pTab;
  Index *pIdx;
  
  pWInfo = pBuilder->pWInfo;
  if( pWInfo->wctrlFlags & WHERE_FORCE_TABLE ) return 0;
  assert( pWInfo->pTabList->nSrc>=1 );
  pItem = pWInfo->pTabList->a;
  pTab = pItem->pTab;
  if( IsVirtual(pTab) ) return 0;
  if( pItem->zIndexedBy ) return 0;
  iCur = pItem->iCursor;
  pWC = &pWInfo->sWC;
  pLoop = pBuilder->pNew;
  pLoop->wsFlags = 0;
  pLoop->nSkip = 0;
  pTerm = sqlite3WhereFindTerm(pWC, iCur, -1, 0, WO_EQ|WO_IS, 0);
  if( pTerm ){
    testcase( pTerm->eOperator & WO_IS );
    pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_IPK|WHERE_ONEROW;
    pLoop->aLTerm[0] = pTerm;
    pLoop->nLTerm = 1;
    pLoop->u.btree.nEq = 1;
    /* TUNING: Cost of a rowid lookup is 10 */
    pLoop->rRun = 33;  /* 33==sqlite3LogEst(10) */
  }else{
    for(pIdx=pTab->pIndex; pIdx; pIdx=pIdx->pNext){
      int opMask;
      assert( pLoop->aLTermSpace==pLoop->aLTerm );
      if( !IsUniqueIndex(pIdx)
       || pIdx->pPartIdxWhere!=0 
       || pIdx->nKeyCol>ArraySize(pLoop->aLTermSpace) 
      ) continue;
      opMask = pIdx->uniqNotNull ? (WO_EQ|WO_IS) : WO_EQ;
      for(j=0; j<pIdx->nKeyCol; j++){
        pTerm = sqlite3WhereFindTerm(pWC, iCur, pIdx->aiColumn[j], 0, opMask, pIdx);
        if( pTerm==0 ) break;
        testcase( pTerm->eOperator & WO_IS );
        pLoop->aLTerm[j] = pTerm;
      }
      if( j!=pIdx->nKeyCol ) continue;
      pLoop->wsFlags = WHERE_COLUMN_EQ|WHERE_ONEROW|WHERE_INDEXED;
      if( pIdx->isCovering || (pItem->colUsed & ~columnsInIndex(pIdx))==0 ){
        pLoop->wsFlags |= WHERE_IDX_ONLY;
      }
      pLoop->nLTerm = j;
      pLoop->u.btree.nEq = j;
      pLoop->u.btree.pIndex = pIdx;
      /* TUNING: Cost of a unique index lookup is 15 */
      pLoop->rRun = 39;  /* 39==sqlite3LogEst(15) */
      break;
    }
  }
  if( pLoop->wsFlags ){
    pLoop->nOut = (LogEst)1;
    pWInfo->a[0].pWLoop = pLoop;
    pLoop->maskSelf = sqlite3WhereGetMask(&pWInfo->sMaskSet, iCur);
    pWInfo->a[0].iTabCur = iCur;
    pWInfo->nRowOut = 1;
    if( pWInfo->pOrderBy ) pWInfo->nOBSat =  pWInfo->pOrderBy->nExpr;
    if( pWInfo->wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
#ifdef SQLITE_DEBUG
    pLoop->cId = '0';
#endif
    return 1;
  }
  return 0;
}

/*
** Generate the beginning of the loop used for WHERE clause processing.
** The return value is a pointer to an opaque structure that contains
** information needed to terminate the loop.  Later, the calling routine
** should invoke sqlite3WhereEnd() with the return value of this function
** in order to complete the WHERE clause processing.
**
** If an error occurs, this routine returns NULL.
**
** The basic idea is to do a nested loop, one loop for each table in
** the FROM clause of a select.  (INSERT and UPDATE statements are the
** same as a SELECT with only a single table in the FROM clause.)  For
** example, if the SQL is this:
**
**       SELECT * FROM t1, t2, t3 WHERE ...;
**
** Then the code generated is conceptually like the following:
**
**      foreach row1 in t1 do       \    Code generated
**        foreach row2 in t2 do      |-- by sqlite3WhereBegin()
**          foreach row3 in t3 do   /
**            ...
**          end                     \    Code generated
**        end                        |-- by sqlite3WhereEnd()
**      end                         /
**
** Note that the loops might not be nested in the order in which they
** appear in the FROM clause if a different order is better able to make
** use of indices.  Note also that when the IN operator appears in
** the WHERE clause, it might result in additional nested loops for
** scanning through all values on the right-hand side of the IN.
**
** There are Btree cursors associated with each table.  t1 uses cursor
** number pTabList->a[0].iCursor.  t2 uses the cursor pTabList->a[1].iCursor.
** And so forth.  This routine generates code to open those VDBE cursors
** and sqlite3WhereEnd() generates the code to close them.
**
** The code that sqlite3WhereBegin() generates leaves the cursors named
** in pTabList pointing at their appropriate entries.  The [...] code
** can use OP_Column and OP_Rowid opcodes on these cursors to extract
** data from the various tables of the loop.
**
** If the WHERE clause is empty, the foreach loops must each scan their
** entire tables.  Thus a three-way join is an O(N^3) operation.  But if
** the tables have indices and there are terms in the WHERE clause that
** refer to those indices, a complete table scan can be avoided and the
** code will run much faster.  Most of the work of this routine is checking
** to see if there are indices that can be used to speed up the loop.
**
** Terms of the WHERE clause are also used to limit which rows actually
** make it to the "..." in the middle of the loop.  After each "foreach",
** terms of the WHERE clause that use only terms in that loop and outer
** loops are evaluated and if false a jump is made around all subsequent
** inner loops (or around the "..." if the test occurs within the inner-
** most loop)
**
** OUTER JOINS
**
** An outer join of tables t1 and t2 is conceptally coded as follows:
**
**    foreach row1 in t1 do
**      flag = 0
**      foreach row2 in t2 do
**        start:
**          ...
**          flag = 1
**      end
**      if flag==0 then
**        move the row2 cursor to a null row
**        goto start
**      fi
**    end
**
** ORDER BY CLAUSE PROCESSING
**
** pOrderBy is a pointer to the ORDER BY clause (or the GROUP BY clause
** if the WHERE_GROUPBY flag is set in wctrlFlags) of a SELECT statement
** if there is one.  If there is no ORDER BY clause or if this routine
** is called from an UPDATE or DELETE statement, then pOrderBy is NULL.
**
** The iIdxCur parameter is the cursor number of an index.  If 
** WHERE_ONETABLE_ONLY is set, iIdxCur is the cursor number of an index
** to use for OR clause processing.  The WHERE clause should use this
** specific cursor.  If WHERE_ONEPASS_DESIRED is set, then iIdxCur is
** the first cursor in an array of cursors for all indices.  iIdxCur should
** be used to compute the appropriate cursor depending on which index is
** used.
*/
SQLITE_PRIVATE WhereInfo *sqlite3WhereBegin(
  Parse *pParse,        /* The parser context */
  SrcList *pTabList,    /* FROM clause: A list of all tables to be scanned */
  Expr *pWhere,         /* The WHERE clause */
  ExprList *pOrderBy,   /* An ORDER BY (or GROUP BY) clause, or NULL */
  ExprList *pResultSet, /* Result set of the query */
  u16 wctrlFlags,       /* One of the WHERE_* flags defined in sqliteInt.h */
  int iIdxCur           /* If WHERE_ONETABLE_ONLY is set, index cursor number */
){
  int nByteWInfo;            /* Num. bytes allocated for WhereInfo struct */
  int nTabList;              /* Number of elements in pTabList */
  WhereInfo *pWInfo;         /* Will become the return value of this function */
  Vdbe *v = pParse->pVdbe;   /* The virtual database engine */
  Bitmask notReady;          /* Cursors that are not yet positioned */
  WhereLoopBuilder sWLB;     /* The WhereLoop builder */
  WhereMaskSet *pMaskSet;    /* The expression mask set */
  WhereLevel *pLevel;        /* A single level in pWInfo->a[] */
  WhereLoop *pLoop;          /* Pointer to a single WhereLoop object */
  int ii;                    /* Loop counter */
  sqlite3 *db;               /* Database connection */
  int rc;                    /* Return code */


  /* Variable initialization */
  db = pParse->db;
  memset(&sWLB, 0, sizeof(sWLB));

  /* An ORDER/GROUP BY clause of more than 63 terms cannot be optimized */
  testcase( pOrderBy && pOrderBy->nExpr==BMS-1 );
  if( pOrderBy && pOrderBy->nExpr>=BMS ) pOrderBy = 0;
  sWLB.pOrderBy = pOrderBy;

  /* Disable the DISTINCT optimization if SQLITE_DistinctOpt is set via
  ** sqlite3_test_ctrl(SQLITE_TESTCTRL_OPTIMIZATIONS,...) */
  if( OptimizationDisabled(db, SQLITE_DistinctOpt) ){
    wctrlFlags &= ~WHERE_WANT_DISTINCT;
  }

  /* The number of tables in the FROM clause is limited by the number of
  ** bits in a Bitmask 
  */
  testcase( pTabList->nSrc==BMS );
  if( pTabList->nSrc>BMS ){
    sqlite3ErrorMsg(pParse, "at most %d tables in a join", BMS);
    return 0;
  }

  /* This function normally generates a nested loop for all tables in 
  ** pTabList.  But if the WHERE_ONETABLE_ONLY flag is set, then we should
  ** only generate code for the first table in pTabList and assume that
  ** any cursors associated with subsequent tables are uninitialized.
  */
  nTabList = (wctrlFlags & WHERE_ONETABLE_ONLY) ? 1 : pTabList->nSrc;

  /* Allocate and initialize the WhereInfo structure that will become the
  ** return value. A single allocation is used to store the WhereInfo
  ** struct, the contents of WhereInfo.a[], the WhereClause structure
  ** and the WhereMaskSet structure. Since WhereClause contains an 8-byte
  ** field (type Bitmask) it must be aligned on an 8-byte boundary on
  ** some architectures. Hence the ROUND8() below.
  */
  nByteWInfo = ROUND8(sizeof(WhereInfo)+(nTabList-1)*sizeof(WhereLevel));
  pWInfo = sqlite3DbMallocZero(db, nByteWInfo + sizeof(WhereLoop));
  if( db->mallocFailed ){
    sqlite3DbFree(db, pWInfo);
    pWInfo = 0;
    goto whereBeginError;
  }
  pWInfo->aiCurOnePass[0] = pWInfo->aiCurOnePass[1] = -1;
  pWInfo->nLevel = nTabList;
  pWInfo->pParse = pParse;
  pWInfo->pTabList = pTabList;
  pWInfo->pOrderBy = pOrderBy;
  pWInfo->pResultSet = pResultSet;
  pWInfo->iBreak = pWInfo->iContinue = sqlite3VdbeMakeLabel(v);
  pWInfo->wctrlFlags = wctrlFlags;
  pWInfo->savedNQueryLoop = pParse->nQueryLoop;
  pMaskSet = &pWInfo->sMaskSet;
  sWLB.pWInfo = pWInfo;
  sWLB.pWC = &pWInfo->sWC;
  sWLB.pNew = (WhereLoop*)(((char*)pWInfo)+nByteWInfo);
  assert( EIGHT_BYTE_ALIGNMENT(sWLB.pNew) );
  whereLoopInit(sWLB.pNew);
#ifdef SQLITE_DEBUG
  sWLB.pNew->cId = '*';
#endif

  /* Split the WHERE clause into separate subexpressions where each
  ** subexpression is separated by an AND operator.
  */
  initMaskSet(pMaskSet);
  sqlite3WhereClauseInit(&pWInfo->sWC, pWInfo);
  sqlite3WhereSplit(&pWInfo->sWC, pWhere, TK_AND);
    
  /* Special case: a WHERE clause that is constant.  Evaluate the
  ** expression and either jump over all of the code or fall thru.
  */
  for(ii=0; ii<sWLB.pWC->nTerm; ii++){
    if( nTabList==0 || sqlite3ExprIsConstantNotJoin(sWLB.pWC->a[ii].pExpr) ){
      sqlite3ExprIfFalse(pParse, sWLB.pWC->a[ii].pExpr, pWInfo->iBreak,
                         SQLITE_JUMPIFNULL);
      sWLB.pWC->a[ii].wtFlags |= TERM_CODED;
    }
  }

  /* Special case: No FROM clause
  */
  if( nTabList==0 ){
    if( pOrderBy ) pWInfo->nOBSat = pOrderBy->nExpr;
    if( wctrlFlags & WHERE_WANT_DISTINCT ){
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }
  }

  /* Assign a bit from the bitmask to every term in the FROM clause.
  **
  ** When assigning bitmask values to FROM clause cursors, it must be
  ** the case that if X is the bitmask for the N-th FROM clause term then
  ** the bitmask for all FROM clause terms to the left of the N-th term
  ** is (X-1).   An expression from the ON clause of a LEFT JOIN can use
  ** its Expr.iRightJoinTable value to find the bitmask of the right table
  ** of the join.  Subtracting one from the right table bitmask gives a
  ** bitmask for all tables to the left of the join.  Knowing the bitmask
  ** for all tables to the left of a left join is important.  Ticket #3015.
  **
  ** Note that bitmasks are created for all pTabList->nSrc tables in
  ** pTabList, not just the first nTabList tables.  nTabList is normally
  ** equal to pTabList->nSrc but might be shortened to 1 if the
  ** WHERE_ONETABLE_ONLY flag is set.
  */
  for(ii=0; ii<pTabList->nSrc; ii++){
    createMask(pMaskSet, pTabList->a[ii].iCursor);
  }
#ifndef NDEBUG
  {
    Bitmask toTheLeft = 0;
    for(ii=0; ii<pTabList->nSrc; ii++){
      Bitmask m = sqlite3WhereGetMask(pMaskSet, pTabList->a[ii].iCursor);
      assert( (m-1)==toTheLeft );
      toTheLeft |= m;
    }
  }
#endif

  /* Analyze all of the subexpressions. */
  sqlite3WhereExprAnalyze(pTabList, &pWInfo->sWC);
  if( db->mallocFailed ) goto whereBeginError;

  if( wctrlFlags & WHERE_WANT_DISTINCT ){
    if( isDistinctRedundant(pParse, pTabList, &pWInfo->sWC, pResultSet) ){
      /* The DISTINCT marking is pointless.  Ignore it. */
      pWInfo->eDistinct = WHERE_DISTINCT_UNIQUE;
    }else if( pOrderBy==0 ){
      /* Try to ORDER BY the result set to make distinct processing easier */
      pWInfo->wctrlFlags |= WHERE_DISTINCTBY;
      pWInfo->pOrderBy = pResultSet;
    }
  }

  /* Construct the WhereLoop objects */
  WHERETRACE(0xffff,("*** Optimizer Start ***\n"));
#if defined(WHERETRACE_ENABLED)
  if( sqlite3WhereTrace & 0x100 ){ /* Display all terms of the WHERE clause */
    int i;
    for(i=0; i<sWLB.pWC->nTerm; i++){
      whereTermPrint(&sWLB.pWC->a[i], i);
    }
  }
#endif

  if( nTabList!=1 || whereShortCut(&sWLB)==0 ){
    rc = whereLoopAddAll(&sWLB);
    if( rc ) goto whereBeginError;
  
#ifdef WHERETRACE_ENABLED
    if( sqlite3WhereTrace ){    /* Display all of the WhereLoop objects */
      WhereLoop *p;
      int i;
      static const char zLabel[] = "0123456789abcdefghijklmnopqrstuvwyxz"
                                             "ABCDEFGHIJKLMNOPQRSTUVWYXZ";
      for(p=pWInfo->pLoops, i=0; p; p=p->pNextLoop, i++){
        p->cId = zLabel[i%sizeof(zLabel)];
        whereLoopPrint(p, sWLB.pWC);
      }
    }
#endif
  
    wherePathSolver(pWInfo, 0);
    if( db->mallocFailed ) goto whereBeginError;
    if( pWInfo->pOrderBy ){
       wherePathSolver(pWInfo, pWInfo->nRowOut+1);
       if( db->mallocFailed ) goto whereBeginError;
    }
  }
  if( pWInfo->pOrderBy==0 && (db->flags & SQLITE_ReverseOrder)!=0 ){
     pWInfo->revMask = (Bitmask)(-1);
  }
  if( pParse->nErr || NEVER(db->mallocFailed) ){
    goto whereBeginError;
  }
#ifdef WHERETRACE_ENABLED
  if( sqlite3WhereTrace ){
    sqlite3DebugPrintf("---- Solution nRow=%d", pWInfo->nRowOut);
    if( pWInfo->nOBSat>0 ){
      sqlite3DebugPrintf(" ORDERBY=%d,0x%llx", pWInfo->nOBSat, pWInfo->revMask);
    }
    switch( pWInfo->eDistinct ){
      case WHERE_DISTINCT_UNIQUE: {
        sqlite3DebugPrintf("  DISTINCT=unique");
        break;
      }
      case WHERE_DISTINCT_ORDERED: {
        sqlite3DebugPrintf("  DISTINCT=ordered");
        break;
      }
      case WHERE_DISTINCT_UNORDERED: {
        sqlite3DebugPrintf("  DISTINCT=unordered");
        break;
      }
    }
    sqlite3DebugPrintf("\n");
    for(ii=0; ii<pWInfo->nLevel; ii++){
      whereLoopPrint(pWInfo->a[ii].pWLoop, sWLB.pWC);
    }
  }
#endif
  /* Attempt to omit tables from the join that do not effect the result */
  if( pWInfo->nLevel>=2
   && pResultSet!=0
   && OptimizationEnabled(db, SQLITE_OmitNoopJoin)
  ){
    Bitmask tabUsed = sqlite3WhereExprListUsage(pMaskSet, pResultSet);
    if( sWLB.pOrderBy ){
      tabUsed |= sqlite3WhereExprListUsage(pMaskSet, sWLB.pOrderBy);
    }
    while( pWInfo->nLevel>=2 ){
      WhereTerm *pTerm, *pEnd;
      pLoop = pWInfo->a[pWInfo->nLevel-1].pWLoop;
      if( (pWInfo->pTabList->a[pLoop->iTab].jointype & JT_LEFT)==0 ) break;
      if( (wctrlFlags & WHERE_WANT_DISTINCT)==0
       && (pLoop->wsFlags & WHERE_ONEROW)==0
      ){
        break;
      }
      if( (tabUsed & pLoop->maskSelf)!=0 ) break;
      pEnd = sWLB.pWC->a + sWLB.pWC->nTerm;
      for(pTerm=sWLB.pWC->a; pTerm<pEnd; pTerm++){
        if( (pTerm->prereqAll & pLoop->maskSelf)!=0
         && !ExprHasProperty(pTerm->pExpr, EP_FromJoin)
        ){
          break;
        }
      }
      if( pTerm<pEnd ) break;
      WHERETRACE(0xffff, ("-> drop loop %c not used\n", pLoop->cId));
      pWInfo->nLevel--;
      nTabList--;
    }
  }
  WHERETRACE(0xffff,("*** Optimizer Finished ***\n"));
  pWInfo->pParse->nQueryLoop += pWInfo->nRowOut;

  /* If the caller is an UPDATE or DELETE statement that is requesting
  ** to use a one-pass algorithm, determine if this is appropriate.
  ** The one-pass algorithm only works if the WHERE clause constrains
  ** the statement to update or delete a single row.
  */
  assert( (wctrlFlags & WHERE_ONEPASS_DESIRED)==0 || pWInfo->nLevel==1 );
  if( (wctrlFlags & WHERE_ONEPASS_DESIRED)!=0 
   && (pWInfo->a[0].pWLoop->wsFlags & WHERE_ONEROW)!=0 ){
    pWInfo->okOnePass = 1;
    if( HasRowid(pTabList->a[0].pTab) ){
      pWInfo->a[0].pWLoop->wsFlags &= ~WHERE_IDX_ONLY;
    }
  }

  /* Open all tables in the pTabList and any indices selected for
  ** searching those tables.
  */
  for(ii=0, pLevel=pWInfo->a; ii<nTabList; ii++, pLevel++){
    Table *pTab;     /* Table to open */
    int iDb;         /* Index of database containing table/index */
    struct SrcList_item *pTabItem;

    pTabItem = &pTabList->a[pLevel->iFrom];
    pTab = pTabItem->pTab;
    iDb = sqlite3SchemaToIndex(db, pTab->pSchema);
    pLoop = pLevel->pWLoop;
    if( (pTab->tabFlags & TF_Ephemeral)!=0 || pTab->pSelect ){
      /* Do nothing */
    }else
#ifndef SQLITE_OMIT_VIRTUALTABLE
    if( (pLoop->wsFlags & WHERE_VIRTUALTABLE)!=0 ){
      const char *pVTab = (const char *)sqlite3GetVTable(db, pTab);
      int iCur = pTabItem->iCursor;
      sqlite3VdbeAddOp4(v, OP_VOpen, iCur, 0, 0, pVTab, P4_VTAB);
    }else if( IsVirtual(pTab) ){
      /* noop */
    }else
#endif
    if( (pLoop->wsFlags & WHERE_IDX_ONLY)==0
         && (wctrlFlags & WHERE_OMIT_OPEN_CLOSE)==0 ){
      int op = OP_OpenRead;
      if( pWInfo->okOnePass ){
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[0] = pTabItem->iCursor;
      };
      sqlite3OpenTable(pParse, pTabItem->iCursor, iDb, pTab, op);
      assert( pTabItem->iCursor==pLevel->iTabCur );
      testcase( !pWInfo->okOnePass && pTab->nCol==BMS-1 );
      testcase( !pWInfo->okOnePass && pTab->nCol==BMS );
      if( !pWInfo->okOnePass && pTab->nCol<BMS && HasRowid(pTab) ){
        Bitmask b = pTabItem->colUsed;
        int n = 0;
        for(; b; b=b>>1, n++){}
        sqlite3VdbeChangeP4(v, sqlite3VdbeCurrentAddr(v)-1, 
                            SQLITE_INT_TO_PTR(n), P4_INT32);
        assert( n<=pTab->nCol );
      }
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
      sqlite3VdbeAddOp4Dup8(v, OP_ColumnsUsed, pTabItem->iCursor, 0, 0,
                            (const u8*)&pTabItem->colUsed, P4_INT64);
#endif
    }else{
      sqlite3TableLock(pParse, iDb, pTab->tnum, 0, pTab->zName);
    }
    if( pLoop->wsFlags & WHERE_INDEXED ){
      Index *pIx = pLoop->u.btree.pIndex;
      int iIndexCur;
      int op = OP_OpenRead;
      /* iIdxCur is always set if to a positive value if ONEPASS is possible */
      assert( iIdxCur!=0 || (pWInfo->wctrlFlags & WHERE_ONEPASS_DESIRED)==0 );
      if( !HasRowid(pTab) && IsPrimaryKeyIndex(pIx)
       && (wctrlFlags & WHERE_ONETABLE_ONLY)!=0
      ){
        /* This is one term of an OR-optimization using the PRIMARY KEY of a
        ** WITHOUT ROWID table.  No need for a separate index */
        iIndexCur = pLevel->iTabCur;
        op = 0;
      }else if( pWInfo->okOnePass ){
        Index *pJ = pTabItem->pTab->pIndex;
        iIndexCur = iIdxCur;
        assert( wctrlFlags & WHERE_ONEPASS_DESIRED );
        while( ALWAYS(pJ) && pJ!=pIx ){
          iIndexCur++;
          pJ = pJ->pNext;
        }
        op = OP_OpenWrite;
        pWInfo->aiCurOnePass[1] = iIndexCur;
      }else if( iIdxCur && (wctrlFlags & WHERE_ONETABLE_ONLY)!=0 ){
        iIndexCur = iIdxCur;
        if( wctrlFlags & WHERE_REOPEN_IDX ) op = OP_ReopenIdx;
      }else{
        iIndexCur = pParse->nTab++;
      }
      pLevel->iIdxCur = iIndexCur;
      assert( pIx->pSchema==pTab->pSchema );
      assert( iIndexCur>=0 );
      if( op ){
        sqlite3VdbeAddOp3(v, op, iIndexCur, pIx->tnum, iDb);
        sqlite3VdbeSetP4KeyInfo(pParse, pIx);
        if( (pLoop->wsFlags & WHERE_CONSTRAINT)!=0
         && (pLoop->wsFlags & (WHERE_COLUMN_RANGE|WHERE_SKIPSCAN))==0
         && (pWInfo->wctrlFlags&WHERE_ORDERBY_MIN)==0
        ){
          sqlite3VdbeChangeP5(v, OPFLAG_SEEKEQ); /* Hint to COMDB2 */
        }
        VdbeComment((v, "%s", pIx->zName));
#ifdef SQLITE_ENABLE_COLUMN_USED_MASK
        {
          u64 colUsed = 0;
          int ii, jj;
          for(ii=0; ii<pIx->nColumn; ii++){
            jj = pIx->aiColumn[ii];
            if( jj<0 ) continue;
            if( jj>63 ) jj = 63;
            if( (pTabItem->colUsed & MASKBIT(jj))==0 ) continue;
            colUsed |= ((u64)1)<<(ii<63 ? ii : 63);
          }
          sqlite3VdbeAddOp4Dup8(v, OP_ColumnsUsed, iIndexCur, 0, 0,
                                (u8*)&colUsed, P4_INT64);
        }
#endif /* SQLITE_ENABLE_COLUMN_USED_MASK */
      }
    }
    if( iDb>=0 ) sqlite3CodeVerifySchema(pParse, iDb);
  }
  pWInfo->iTop = sqlite3VdbeCurrentAddr(v);
  if( db->mallocFailed ) goto whereBeginError;

  /* Generate the code to do the search.  Each iteration of the for
  ** loop below generates code for a single nested loop of the VM
  ** program.
  */
  notReady = ~(Bitmask)0;
  for(ii=0; ii<nTabList; ii++){
    int addrExplain;
    int wsFlags;
    pLevel = &pWInfo->a[ii];
    wsFlags = pLevel->pWLoop->wsFlags;
#ifndef SQLITE_OMIT_AUTOMATIC_INDEX
    if( (pLevel->pWLoop->wsFlags & WHERE_AUTO_INDEX)!=0 ){
      constructAutomaticIndex(pParse, &pWInfo->sWC,
                &pTabList->a[pLevel->iFrom], notReady, pLevel);
      if( db->mallocFailed ) goto whereBeginError;
    }
#endif
    addrExplain = sqlite3WhereExplainOneScan(
        pParse, pTabList, pLevel, ii, pLevel->iFrom, wctrlFlags
    );
    pLevel->addrBody = sqlite3VdbeCurrentAddr(v);
    notReady = sqlite3WhereCodeOneLoopStart(pWInfo, ii, notReady);
    pWInfo->iContinue = pLevel->addrCont;
    if( (wsFlags&WHERE_MULTI_OR)==0 && (wctrlFlags&WHERE_ONETABLE_ONLY)==0 ){
      sqlite3WhereAddScanStatus(v, pTabList, pLevel, addrExplain);
    }
  }

  /* Done. */
  VdbeModuleComment((v, "Begin WHERE-core"));
  return pWInfo;

  /* Jump here if malloc fails */
whereBeginError:
  if( pWInfo ){
    pParse->nQueryLoop = pWInfo->savedNQueryLoop;
    whereInfoFree(db, pWInfo);
  }
  return 0;
}

/*
** Generate the end of the WHERE loop.  See comments on 
** sqlite3WhereBegin() for additional information.
*/
SQLITE_PRIVATE void sqlite3WhereEnd(WhereInfo *pWInfo){
  Parse *pParse = pWInfo->pParse;
  Vdbe *v = pParse->pVdbe;
  int i;
  WhereLevel *pLevel;
  WhereLoop *pLoop;
  SrcList *pTabList = pWInfo->pTabList;
  sqlite3 *db = pParse->db;

  /* Generate loop termination code.
  */
  VdbeModuleComment((v, "End WHERE-core"));
  sqlite3ExprCacheClear(pParse);
  for(i=pWInfo->nLevel-1; i>=0; i--){
    int addr;
    pLevel = &pWInfo->a[i];
    pLoop = pLevel->pWLoop;
    sqlite3VdbeResolveLabel(v, pLevel->addrCont);
    if( pLevel->op!=OP_Noop ){
      sqlite3VdbeAddOp3(v, pLevel->op, pLevel->p1, pLevel->p2, pLevel->p3);
      sqlite3VdbeChangeP5(v, pLevel->p5);
      VdbeCoverage(v);
      VdbeCoverageIf(v, pLevel->op==OP_Next);
      VdbeCoverageIf(v, pLevel->op==OP_Prev);
      VdbeCoverageIf(v, pLevel->op==OP_VNext);
    }
    if( pLoop->wsFlags & WHERE_IN_ABLE && pLevel->u.in.nIn>0 ){
      struct InLoop *pIn;
      int j;
      sqlite3VdbeResolveLabel(v, pLevel->addrNxt);
      for(j=pLevel->u.in.nIn, pIn=&pLevel->u.in.aInLoop[j-1]; j>0; j--, pIn--){
        sqlite3VdbeJumpHere(v, pIn->addrInTop+1);
        sqlite3VdbeAddOp2(v, pIn->eEndLoopOp, pIn->iCur, pIn->addrInTop);
        VdbeCoverage(v);
        VdbeCoverageIf(v, pIn->eEndLoopOp==OP_PrevIfOpen);
        VdbeCoverageIf(v, pIn->eEndLoopOp==OP_NextIfOpen);
        sqlite3VdbeJumpHere(v, pIn->addrInTop-1);
      }
    }
    sqlite3VdbeResolveLabel(v, pLevel->addrBrk);
    if( pLevel->addrSkip ){
      sqlite3VdbeAddOp2(v, OP_Goto, 0, pLevel->addrSkip);
      VdbeComment((v, "next skip-scan on %s", pLoop->u.btree.pIndex->zName));
      sqlite3VdbeJumpHere(v, pLevel->addrSkip);
      sqlite3VdbeJumpHere(v, pLevel->addrSkip-2);
    }
    if( pLevel->addrLikeRep ){
      int op;
      if( sqlite3VdbeGetOp(v, pLevel->addrLikeRep-1)->p1 ){
        op = OP_DecrJumpZero;
      }else{
        op = OP_JumpZeroIncr;
      }
      sqlite3VdbeAddOp2(v, op, pLevel->iLikeRepCntr, pLevel->addrLikeRep);
      VdbeCoverage(v);
    }
    if( pLevel->iLeftJoin ){
      addr = sqlite3VdbeAddOp1(v, OP_IfPos, pLevel->iLeftJoin); VdbeCoverage(v);
      assert( (pLoop->wsFlags & WHERE_IDX_ONLY)==0
           || (pLoop->wsFlags & WHERE_INDEXED)!=0 );
      if( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 ){
        sqlite3VdbeAddOp1(v, OP_NullRow, pTabList->a[i].iCursor);
      }
      if( pLoop->wsFlags & WHERE_INDEXED ){
        sqlite3VdbeAddOp1(v, OP_NullRow, pLevel->iIdxCur);
      }
      if( pLevel->op==OP_Return ){
        sqlite3VdbeAddOp2(v, OP_Gosub, pLevel->p1, pLevel->addrFirst);
      }else{
        sqlite3VdbeAddOp2(v, OP_Goto, 0, pLevel->addrFirst);
      }
      sqlite3VdbeJumpHere(v, addr);
    }
    VdbeModuleComment((v, "End WHERE-loop%d: %s", i,
                     pWInfo->pTabList->a[pLevel->iFrom].pTab->zName));
  }

  /* The "break" point is here, just past the end of the outer loop.
  ** Set it.
  */
  sqlite3VdbeResolveLabel(v, pWInfo->iBreak);

  assert( pWInfo->nLevel<=pTabList->nSrc );
  for(i=0, pLevel=pWInfo->a; i<pWInfo->nLevel; i++, pLevel++){
    int k, last;
    VdbeOp *pOp;
    Index *pIdx = 0;
    struct SrcList_item *pTabItem = &pTabList->a[pLevel->iFrom];
    Table *pTab = pTabItem->pTab;
    assert( pTab!=0 );
    pLoop = pLevel->pWLoop;

    /* For a co-routine, change all OP_Column references to the table of
    ** the co-routine into OP_Copy of result contained in a register.
    ** OP_Rowid becomes OP_Null.
    */
    if( pTabItem->viaCoroutine && !db->mallocFailed ){
      translateColumnToCopy(v, pLevel->addrBody, pLevel->iTabCur,
                            pTabItem->regResult);
      continue;
    }

    /* Close all of the cursors that were opened by sqlite3WhereBegin.
    ** Except, do not close cursors that will be reused by the OR optimization
    ** (WHERE_OMIT_OPEN_CLOSE).  And do not close the OP_OpenWrite cursors
    ** created for the ONEPASS optimization.
    */
    if( (pTab->tabFlags & TF_Ephemeral)==0
     && pTab->pSelect==0
     && (pWInfo->wctrlFlags & WHERE_OMIT_OPEN_CLOSE)==0
    ){
      int ws = pLoop->wsFlags;
      if( !pWInfo->okOnePass && (ws & WHERE_IDX_ONLY)==0 ){
        sqlite3VdbeAddOp1(v, OP_Close, pTabItem->iCursor);
      }
      if( (ws & WHERE_INDEXED)!=0
       && (ws & (WHERE_IPK|WHERE_AUTO_INDEX))==0 
       && pLevel->iIdxCur!=pWInfo->aiCurOnePass[1]
      ){
        sqlite3VdbeAddOp1(v, OP_Close, pLevel->iIdxCur);
      }
    }

    /* If this scan uses an index, make VDBE code substitutions to read data
    ** from the index instead of from the table where possible.  In some cases
    ** this optimization prevents the table from ever being read, which can
    ** yield a significant performance boost.
    ** 
    ** Calls to the code generator in between sqlite3WhereBegin and
    ** sqlite3WhereEnd will have created code that references the table
    ** directly.  This loop scans all that code looking for opcodes
    ** that reference the table and converts them into opcodes that
    ** reference the index.
    */
    if( pLoop->wsFlags & (WHERE_INDEXED|WHERE_IDX_ONLY) ){
      pIdx = pLoop->u.btree.pIndex;
    }else if( pLoop->wsFlags & WHERE_MULTI_OR ){
      pIdx = pLevel->u.pCovidx;
    }
    if( pIdx && !db->mallocFailed ){
      last = sqlite3VdbeCurrentAddr(v);
      k = pLevel->addrBody;
      pOp = sqlite3VdbeGetOp(v, k);
      for(; k<last; k++, pOp++){
        if( pOp->p1!=pLevel->iTabCur ) continue;
        if( pOp->opcode==OP_Column ){
          int x = pOp->p2;
          assert( pIdx->pTable==pTab );
          if( !HasRowid(pTab) ){
            Index *pPk = sqlite3PrimaryKeyIndex(pTab);
            x = pPk->aiColumn[x];
          }
          x = sqlite3ColumnOfIndex(pIdx, x);
          if( x>=0 ){
            pOp->p2 = x;
            pOp->p1 = pLevel->iIdxCur;
          }
          assert( (pLoop->wsFlags & WHERE_IDX_ONLY)==0 || x>=0 );
        }else if( pOp->opcode==OP_Rowid ){
          pOp->p1 = pLevel->iIdxCur;
          pOp->opcode = OP_IdxRowid;
        }
      }
    }
  }

  /* Final cleanup
  */
  pParse->nQueryLoop = pWInfo->savedNQueryLoop;
  whereInfoFree(db, pWInfo);
  return;
}

/************** End of where.c ***********************************************/
/************** Begin file parse.c *******************************************/
/* Driver template for the LEMON parser generator.
** The author disclaims copyright to this source code.
**
** This version of "lempar.c" is modified, slightly, for use by SQLite.
** The only modifications are the addition of a couple of NEVER()
** macros to disable tests that are needed in the case of a general
** LALR(1) grammar but which are always false in the
** specific grammar used by SQLite.
*/
/* First off, code is included that follows the "include" declaration
** in the input grammar file. */
/* #include <stdio.h> */

/* #include "sqliteInt.h" */

/*
** Disable all error recovery processing in the parser push-down
** automaton.
*/
#define YYNOERRORRECOVERY 1

/*
** Make yytestcase() the same as testcase()
*/
#define yytestcase(X) testcase(X)

/*
** An instance of this structure holds information about the
** LIMIT clause of a SELECT statement.
*/
struct LimitVal {
  Expr *pLimit;    /* The LIMIT expression.  NULL if there is no limit */
  Expr *pOffset;   /* The OFFSET expression.  NULL if there is none */
};

/*
** An instance of this structure is used to store the LIKE,
** GLOB, NOT LIKE, and NOT GLOB operators.
*/
struct LikeOp {
  Token eOperator;  /* "like" or "glob" or "regexp" */
  int bNot;         /* True if the NOT keyword is present */
};

/*
** An instance of the following structure describes the event of a
** TRIGGER.  "a" is the event type, one of TK_UPDATE, TK_INSERT,
** TK_DELETE, or TK_INSTEAD.  If the event is of the form
**
**      UPDATE ON (a,b,c)
**
** Then the "b" IdList records the list "a,b,c".
*/
struct TrigEvent { int a; IdList * b; };

/*
** An instance of this structure holds the ATTACH key and the key type.
*/
struct AttachKey { int type;  Token key; };


  /*
  ** For a compound SELECT statement, make sure p->pPrior->pNext==p for
  ** all elements in the list.  And make sure list length does not exceed
  ** SQLITE_LIMIT_COMPOUND_SELECT.
  */
  static void parserDoubleLinkSelect(Parse *pParse, Select *p){
    if( p->pPrior ){
      Select *pNext = 0, *pLoop;
      int mxSelect, cnt = 0;
      for(pLoop=p; pLoop; pNext=pLoop, pLoop=pLoop->pPrior, cnt++){
        pLoop->pNext = pNext;
        pLoop->selFlags |= SF_Compound;
      }
      if( (p->selFlags & SF_MultiValue)==0 && 
        (mxSelect = pParse->db->aLimit[SQLITE_LIMIT_COMPOUND_SELECT])>0 &&
        cnt>mxSelect
      ){
        sqlite3ErrorMsg(pParse, "too many terms in compound SELECT");
      }
    }
  }

  /* This is a utility routine used to set the ExprSpan.zStart and
  ** ExprSpan.zEnd values of pOut so that the span covers the complete
  ** range of text beginning with pStart and going to the end of pEnd.
  */
  static void spanSet(ExprSpan *pOut, Token *pStart, Token *pEnd){
    pOut->zStart = pStart->z;
    pOut->zEnd = &pEnd->z[pEnd->n];
  }

  /* Construct a new Expr object from a single identifier.  Use the
  ** new Expr to populate pOut.  Set the span of pOut to be the identifier
  ** that created the expression.
  */
  static void spanExpr(ExprSpan *pOut, Parse *pParse, int op, Token *pValue){
    pOut->pExpr = sqlite3PExpr(pParse, op, 0, 0, pValue);
    pOut->zStart = pValue->z;
    pOut->zEnd = &pValue->z[pValue->n];
  }

  /* This routine constructs a binary expression node out of two ExprSpan
  ** objects and uses the result to populate a new ExprSpan object.
  */
  static void spanBinaryExpr(
    ExprSpan *pOut,     /* Write the result here */
    Parse *pParse,      /* The parsing context.  Errors accumulate here */
    int op,             /* The binary operation */
    ExprSpan *pLeft,    /* The left operand */
    ExprSpan *pRight    /* The right operand */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pLeft->pExpr, pRight->pExpr, 0);
    pOut->zStart = pLeft->zStart;
    pOut->zEnd = pRight->zEnd;
  }

  /* Construct an expression node for a unary postfix operator
  */
  static void spanUnaryPostfix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPostOp         /* The operand token for setting the span */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zStart = pOperand->zStart;
    pOut->zEnd = &pPostOp->z[pPostOp->n];
  }                           

  /* A routine to convert a binary TK_IS or TK_ISNOT expression into a
  ** unary TK_ISNULL or TK_NOTNULL expression. */
  static void binaryToUnaryIfNull(Parse *pParse, Expr *pY, Expr *pA, int op){
    sqlite3 *db = pParse->db;
    if( pY && pA && pY->op==TK_NULL ){
      pA->op = (u8)op;
      sqlite3ExprDelete(db, pA->pRight);
      pA->pRight = 0;
    }
  }

  /* Construct an expression node for a unary prefix operator
  */
  static void spanUnaryPrefix(
    ExprSpan *pOut,        /* Write the new expression node here */
    Parse *pParse,         /* Parsing context to record errors */
    int op,                /* The operator */
    ExprSpan *pOperand,    /* The operand */
    Token *pPreOp         /* The operand token for setting the span */
  ){
    pOut->pExpr = sqlite3PExpr(pParse, op, pOperand->pExpr, 0, 0);
    pOut->zStart = pPreOp->z;
    pOut->zEnd = pOperand->zEnd;
  }
/* Next is all token values, in a form suitable for use by makeheaders.
** This section will be null unless lemon is run with the -m switch.
*/
/* 
** These constants (all generated automatically by the parser generator)
** specify the various kinds of tokens (terminals) that the parser
** understands. 
**
** Each symbol here is a terminal symbol in the grammar.
*/
/* Make sure the INTERFACE macro is defined.
*/
#ifndef INTERFACE
# define INTERFACE 1
#endif
/* The next thing included is series of defines which control
** various aspects of the generated parser.
**    YYCODETYPE         is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 terminals
**                       and nonterminals.  "int" is used otherwise.
**    YYNOCODE           is a number of type YYCODETYPE which corresponds
**                       to no legal terminal or nonterminal number.  This
**                       number is used to fill in empty slots of the hash 
**                       table.
**    YYFALLBACK         If defined, this indicates that one or more tokens
**                       have fall-back values which should be used if the
**                       original value of the token will not parse.
**    YYACTIONTYPE       is the data type used for storing terminal
**                       and nonterminal numbers.  "unsigned char" is
**                       used if there are fewer than 250 rules and
**                       states combined.  "int" is used otherwise.
**    sqlite3ParserTOKENTYPE     is the data type used for minor tokens given 
**                       directly to the parser from the tokenizer.
**    YYMINORTYPE        is the data type used for all minor tokens.
**                       This is typically a union of many types, one of
**                       which is sqlite3ParserTOKENTYPE.  The entry in the union
**                       for base tokens is called "yy0".
**    YYSTACKDEPTH       is the maximum depth of the parser's stack.  If
**                       zero the stack is dynamically sized using realloc()
**    sqlite3ParserARG_SDECL     A static variable declaration for the %extra_argument
**    sqlite3ParserARG_PDECL     A parameter declaration for the %extra_argument
**    sqlite3ParserARG_STORE     Code to store %extra_argument into yypParser
**    sqlite3ParserARG_FETCH     Code to extract %extra_argument from yypParser
**    YYNSTATE           the combined number of states.
**    YYNRULE            the number of rules in the grammar
**    YYERRORSYMBOL      is the code number of the error symbol.  If not
**                       defined, then do no error processing.
*/
#define YYCODETYPE unsigned char
#define YYNOCODE 254
#define YYACTIONTYPE unsigned short int
#define YYWILDCARD 70
#define sqlite3ParserTOKENTYPE Token
typedef union {
  int yyinit;
  sqlite3ParserTOKENTYPE yy0;
  Select* yy3;
  ExprList* yy14;
  With* yy59;
  SrcList* yy65;
  struct LikeOp yy96;
  Expr* yy132;
  u8 yy186;
  int yy328;
  ExprSpan yy346;
  struct TrigEvent yy378;
  u16 yy381;
  IdList* yy408;
  struct {int value; int mask;} yy429;
  TriggerStep* yy473;
  struct LimitVal yy476;
} YYMINORTYPE;
#ifndef YYSTACKDEPTH
#define YYSTACKDEPTH 100
#endif
#define sqlite3ParserARG_SDECL Parse *pParse;
#define sqlite3ParserARG_PDECL ,Parse *pParse
#define sqlite3ParserARG_FETCH Parse *pParse = yypParser->pParse
#define sqlite3ParserARG_STORE yypParser->pParse = pParse
#define YYNSTATE 642
#define YYNRULE 327
#define YYFALLBACK 1
#define YY_NO_ACTION      (YYNSTATE+YYNRULE+2)
#define YY_ACCEPT_ACTION  (YYNSTATE+YYNRULE+1)
#define YY_ERROR_ACTION   (YYNSTATE+YYNRULE)

/* The yyzerominor constant is used to initialize instances of
** YYMINORTYPE objects to zero. */
static const YYMINORTYPE yyzerominor = { 0 };

/* Define the yytestcase() macro to be a no-op if is not already defined
** otherwise.
**
** Applications can choose to define yytestcase() in the %include section
** to a macro that can assist in verifying code coverage.  For production
** code the yytestcase() macro should be turned off.  But it is useful
** for testing.
*/
#ifndef yytestcase
# define yytestcase(X)
#endif


/* Next are the tables used to determine what action to take based on the
** current state and lookahead token.  These tables are used to implement
** functions that take a state number and lookahead value and return an
** action integer.  
**
** Suppose the action integer is N.  Then the action is determined as
** follows
**
**   0 <= N < YYNSTATE                  Shift N.  That is, push the lookahead
**                                      token onto the stack and goto state N.
**
**   YYNSTATE <= N < YYNSTATE+YYNRULE   Reduce by rule N-YYNSTATE.
**
**   N == YYNSTATE+YYNRULE              A syntax error has occurred.
**
**   N == YYNSTATE+YYNRULE+1            The parser accepts its input.
**
**   N == YYNSTATE+YYNRULE+2            No such action.  Denotes unused
**                                      slots in the yy_action[] table.
**
** The action table is constructed as a single large table named yy_action[].
** Given state S and lookahead X, the action is computed as
**
**      yy_action[ yy_shift_ofst[S] + X ]
**
** If the index value yy_shift_ofst[S]+X is out of range or if the value
** yy_lookahead[yy_shift_ofst[S]+X] is not equal to X or if yy_shift_ofst[S]
** is equal to YY_SHIFT_USE_DFLT, it means that the action is not in the table
** and that yy_default[S] should be used instead.  
**
** The formula above is for computing the action when the lookahead is
** a terminal symbol.  If the lookahead is a non-terminal (as occurs after
** a reduce action) then the yy_reduce_ofst[] array is used in place of
** the yy_shift_ofst[] array and YY_REDUCE_USE_DFLT is used in place of
** YY_SHIFT_USE_DFLT.
**
** The following are the tables generated in this section:
**
**  yy_action[]        A single table containing all actions.
**  yy_lookahead[]     A table containing the lookahead for each entry in
**                     yy_action.  Used to detect hash collisions.
**  yy_shift_ofst[]    For each state, the offset into yy_action for
**                     shifting terminals.
**  yy_reduce_ofst[]   For each state, the offset into yy_action for
**                     shifting non-terminals after a reduce.
**  yy_default[]       Default action for each state.
*/
#define YY_ACTTAB_COUNT (1497)
static const YYACTIONTYPE yy_action[] = {
 /*     0 */   306,  212,  432,  955,  639,  191,  955,  295,  559,   88,
 /*    10 */    88,   88,   88,   81,   86,   86,   86,   86,   85,   85,
 /*    20 */    84,   84,   84,   83,  330,  185,  184,  183,  635,  635,
 /*    30 */   292,  606,  606,   88,   88,   88,   88,  683,   86,   86,
 /*    40 */    86,   86,   85,   85,   84,   84,   84,   83,  330,   16,
 /*    50 */   436,  597,   89,   90,   80,  600,  599,  601,  601,   87,
 /*    60 */    87,   88,   88,   88,   88,  684,   86,   86,   86,   86,
 /*    70 */    85,   85,   84,   84,   84,   83,  330,  306,  559,   84,
 /*    80 */    84,   84,   83,  330,   65,   86,   86,   86,   86,   85,
 /*    90 */    85,   84,   84,   84,   83,  330,  635,  635,  634,  633,
 /*   100 */   182,  682,  550,  379,  376,  375,   17,  322,  606,  606,
 /*   110 */   371,  198,  479,   91,  374,   82,   79,  165,   85,   85,
 /*   120 */    84,   84,   84,   83,  330,  598,  635,  635,  107,   89,
 /*   130 */    90,   80,  600,  599,  601,  601,   87,   87,   88,   88,
 /*   140 */    88,   88,  186,   86,   86,   86,   86,   85,   85,   84,
 /*   150 */    84,   84,   83,  330,  306,  594,  594,  142,  328,  327,
 /*   160 */   484,  249,  344,  238,  635,  635,  634,  633,  585,  448,
 /*   170 */   526,  525,  229,  388,    1,  394,  450,  584,  449,  635,
 /*   180 */   635,  635,  635,  319,  395,  606,  606,  199,  157,  273,
 /*   190 */   382,  268,  381,  187,  635,  635,  634,  633,  311,  555,
 /*   200 */   266,  593,  593,  266,  347,  588,   89,   90,   80,  600,
 /*   210 */   599,  601,  601,   87,   87,   88,   88,   88,   88,  478,
 /*   220 */    86,   86,   86,   86,   85,   85,   84,   84,   84,   83,
 /*   230 */   330,  306,  272,  536,  634,  633,  146,  610,  197,  310,
 /*   240 */   575,  182,  482,  271,  379,  376,  375,  506,   21,  634,
 /*   250 */   633,  634,  633,  635,  635,  374,  611,  574,  548,  440,
 /*   260 */   111,  563,  606,  606,  634,  633,  324,  479,  608,  608,
 /*   270 */   608,  300,  435,  573,  119,  407,  210,  162,  562,  883,
 /*   280 */   592,  592,  306,   89,   90,   80,  600,  599,  601,  601,
 /*   290 */    87,   87,   88,   88,   88,   88,  506,   86,   86,   86,
 /*   300 */    86,   85,   85,   84,   84,   84,   83,  330,  620,  111,
 /*   310 */   635,  635,  361,  606,  606,  358,  249,  349,  248,  433,
 /*   320 */   243,  479,  586,  634,  633,  195,  611,   93,  119,  221,
 /*   330 */   575,  497,  534,  534,   89,   90,   80,  600,  599,  601,
 /*   340 */   601,   87,   87,   88,   88,   88,   88,  574,   86,   86,
 /*   350 */    86,   86,   85,   85,   84,   84,   84,   83,  330,  306,
 /*   360 */    77,  429,  638,  573,  589,  530,  240,  230,  242,  105,
 /*   370 */   249,  349,  248,  515,  588,  208,  460,  529,  564,  173,
 /*   380 */   634,  633,  970,  144,  430,    2,  424,  228,  380,  557,
 /*   390 */   606,  606,  190,  153,  159,  158,  514,   51,  632,  631,
 /*   400 */   630,   71,  536,  432,  954,  196,  610,  954,  614,   45,
 /*   410 */    18,   89,   90,   80,  600,  599,  601,  601,   87,   87,
 /*   420 */    88,   88,   88,   88,  261,   86,   86,   86,   86,   85,
 /*   430 */    85,   84,   84,   84,   83,  330,  306,  608,  608,  608,
 /*   440 */   542,  424,  402,  385,  241,  506,  451,  320,  211,  543,
 /*   450 */   164,  436,  386,  293,  451,  587,  108,  496,  111,  334,
 /*   460 */   391,  591,  424,  614,   27,  452,  453,  606,  606,   72,
 /*   470 */   257,   70,  259,  452,  339,  342,  564,  582,   68,  415,
 /*   480 */   469,  328,  327,   62,  614,   45,  110,  393,   89,   90,
 /*   490 */    80,  600,  599,  601,  601,   87,   87,   88,   88,   88,
 /*   500 */    88,  152,   86,   86,   86,   86,   85,   85,   84,   84,
 /*   510 */    84,   83,  330,  306,  110,  499,  520,  538,  402,  389,
 /*   520 */   424,  110,  566,  500,  593,  593,  454,   82,   79,  165,
 /*   530 */   424,  591,  384,  564,  340,  615,  188,  162,  424,  350,
 /*   540 */   616,  424,  614,   44,  606,  606,  445,  582,  300,  434,
 /*   550 */   151,   19,  614,    9,  568,  580,  348,  615,  469,  567,
 /*   560 */   614,   26,  616,  614,   45,   89,   90,   80,  600,  599,
 /*   570 */   601,  601,   87,   87,   88,   88,   88,   88,  411,   86,
 /*   580 */    86,   86,   86,   85,   85,   84,   84,   84,   83,  330,
 /*   590 */   306,  579,  110,  578,  521,  282,  433,  398,  400,  255,
 /*   600 */   486,   82,   79,  165,  487,  164,   82,   79,  165,  488,
 /*   610 */   488,  364,  387,  424,  544,  544,  509,  350,  362,  155,
 /*   620 */   191,  606,  606,  559,  642,  640,  333,   82,   79,  165,
 /*   630 */   305,  564,  507,  312,  357,  614,   45,  329,  596,  595,
 /*   640 */   194,  337,   89,   90,   80,  600,  599,  601,  601,   87,
 /*   650 */    87,   88,   88,   88,   88,  424,   86,   86,   86,   86,
 /*   660 */    85,   85,   84,   84,   84,   83,  330,  306,   20,  323,
 /*   670 */   150,  263,  211,  543,  421,  596,  595,  614,   22,  424,
 /*   680 */   193,  424,  284,  424,  391,  424,  509,  424,  577,  424,
 /*   690 */   186,  335,  424,  559,  424,  313,  120,  546,  606,  606,
 /*   700 */    67,  614,   47,  614,   50,  614,   48,  614,  100,  614,
 /*   710 */    99,  614,  101,  576,  614,  102,  614,  109,  326,   89,
 /*   720 */    90,   80,  600,  599,  601,  601,   87,   87,   88,   88,
 /*   730 */    88,   88,  424,   86,   86,   86,   86,   85,   85,   84,
 /*   740 */    84,   84,   83,  330,  306,  424,  311,  424,  585,   54,
 /*   750 */   424,  516,  517,  590,  614,  112,  424,  584,  424,  572,
 /*   760 */   424,  195,  424,  571,  424,   67,  424,  614,   94,  614,
 /*   770 */    98,  424,  614,   97,  264,  606,  606,  195,  614,   46,
 /*   780 */   614,   96,  614,   30,  614,   49,  614,  115,  614,  114,
 /*   790 */   418,  229,  388,  614,  113,  306,   89,   90,   80,  600,
 /*   800 */   599,  601,  601,   87,   87,   88,   88,   88,   88,  424,
 /*   810 */    86,   86,   86,   86,   85,   85,   84,   84,   84,   83,
 /*   820 */   330,  119,  424,  590,  110,  372,  606,  606,  195,   53,
 /*   830 */   250,  614,   29,  195,  472,  438,  729,  190,  302,  498,
 /*   840 */    14,  523,  641,    2,  614,   43,  306,   89,   90,   80,
 /*   850 */   600,  599,  601,  601,   87,   87,   88,   88,   88,   88,
 /*   860 */   424,   86,   86,   86,   86,   85,   85,   84,   84,   84,
 /*   870 */    83,  330,  424,  613,  964,  964,  354,  606,  606,  420,
 /*   880 */   312,   64,  614,   42,  391,  355,  283,  437,  301,  255,
 /*   890 */   414,  410,  495,  492,  614,   28,  471,  306,   89,   90,
 /*   900 */    80,  600,  599,  601,  601,   87,   87,   88,   88,   88,
 /*   910 */    88,  424,   86,   86,   86,   86,   85,   85,   84,   84,
 /*   920 */    84,   83,  330,  424,  110,  110,  110,  110,  606,  606,
 /*   930 */   110,  254,   13,  614,   41,  532,  531,  283,  481,  531,
 /*   940 */   457,  284,  119,  561,  356,  614,   40,  284,  306,   89,
 /*   950 */    78,   80,  600,  599,  601,  601,   87,   87,   88,   88,
 /*   960 */    88,   88,  424,   86,   86,   86,   86,   85,   85,   84,
 /*   970 */    84,   84,   83,  330,  110,  424,  341,  220,  555,  606,
 /*   980 */   606,  351,  555,  318,  614,   95,  413,  255,   83,  330,
 /*   990 */   284,  284,  255,  640,  333,  356,  255,  614,   39,  306,
 /*  1000 */   356,   90,   80,  600,  599,  601,  601,   87,   87,   88,
 /*  1010 */    88,   88,   88,  424,   86,   86,   86,   86,   85,   85,
 /*  1020 */    84,   84,   84,   83,  330,  424,  317,  316,  141,  465,
 /*  1030 */   606,  606,  219,  619,  463,  614,   10,  417,  462,  255,
 /*  1040 */   189,  510,  553,  351,  207,  363,  161,  614,   38,  315,
 /*  1050 */   218,  255,  255,   80,  600,  599,  601,  601,   87,   87,
 /*  1060 */    88,   88,   88,   88,  424,   86,   86,   86,   86,   85,
 /*  1070 */    85,   84,   84,   84,   83,  330,   76,  419,  255,    3,
 /*  1080 */   878,  461,  424,  247,  331,  331,  614,   37,  217,   76,
 /*  1090 */   419,  390,    3,  216,  215,  422,    4,  331,  331,  424,
 /*  1100 */   547,   12,  424,  545,  614,   36,  424,  541,  422,  424,
 /*  1110 */   540,  424,  214,  424,  408,  424,  539,  403,  605,  605,
 /*  1120 */   237,  614,   25,  119,  614,   24,  588,  408,  614,   45,
 /*  1130 */   118,  614,   35,  614,   34,  614,   33,  614,   23,  588,
 /*  1140 */    60,  223,  603,  602,  513,  378,   73,   74,  140,  139,
 /*  1150 */   424,  110,  265,   75,  426,  425,   59,  424,  610,   73,
 /*  1160 */    74,  549,  402,  404,  424,  373,   75,  426,  425,  604,
 /*  1170 */   138,  610,  614,   11,  392,   76,  419,  181,    3,  614,
 /*  1180 */    32,  271,  369,  331,  331,  493,  614,   31,  149,  608,
 /*  1190 */   608,  608,  607,   15,  422,  365,  614,    8,  137,  489,
 /*  1200 */   136,  190,  608,  608,  608,  607,   15,  485,  176,  135,
 /*  1210 */     7,  252,  477,  408,  174,  133,  175,  474,   57,   56,
 /*  1220 */   132,  130,  119,   76,  419,  588,    3,  468,  245,  464,
 /*  1230 */   171,  331,  331,  125,  123,  456,  447,  122,  446,  104,
 /*  1240 */   336,  231,  422,  166,  154,   73,   74,  332,  116,  431,
 /*  1250 */   121,  309,   75,  426,  425,  222,  106,  610,  308,  637,
 /*  1260 */   204,  408,  629,  627,  628,    6,  200,  428,  427,  290,
 /*  1270 */   203,  622,  201,  588,   62,   63,  289,   66,  419,  399,
 /*  1280 */     3,  401,  288,   92,  143,  331,  331,  287,  608,  608,
 /*  1290 */   608,  607,   15,   73,   74,  227,  422,  325,   69,  416,
 /*  1300 */    75,  426,  425,  612,  412,  610,  192,   61,  569,  209,
 /*  1310 */   396,  226,  278,  225,  383,  408,  527,  558,  276,  533,
 /*  1320 */   552,  528,  321,  523,  370,  508,  180,  588,  494,  179,
 /*  1330 */   366,  117,  253,  269,  522,  503,  608,  608,  608,  607,
 /*  1340 */    15,  551,  502,   58,  274,  524,  178,   73,   74,  304,
 /*  1350 */   501,  368,  303,  206,   75,  426,  425,  491,  360,  610,
 /*  1360 */   213,  177,  483,  131,  345,  298,  297,  296,  202,  294,
 /*  1370 */   480,  490,  466,  134,  172,  129,  444,  346,  470,  128,
 /*  1380 */   314,  459,  103,  127,  126,  148,  124,  167,  443,  235,
 /*  1390 */   608,  608,  608,  607,   15,  442,  439,  623,  234,  299,
 /*  1400 */   145,  583,  291,  377,  581,  160,  119,  156,  270,  636,
 /*  1410 */   971,  169,  279,  626,  520,  625,  473,  624,  170,  621,
 /*  1420 */   618,  119,  168,   55,  409,  423,  537,  609,  286,  285,
 /*  1430 */   405,  570,  560,  556,    5,   52,  458,  554,  147,  267,
 /*  1440 */   519,  504,  518,  406,  262,  239,  260,  512,  343,  511,
 /*  1450 */   258,  353,  565,  256,  224,  251,  359,  277,  275,  476,
 /*  1460 */   475,  246,  352,  244,  467,  455,  236,  233,  232,  307,
 /*  1470 */   441,  281,  205,  163,  397,  280,  535,  505,  330,  617,
 /*  1480 */   971,  971,  971,  971,  367,  971,  971,  971,  971,  971,
 /*  1490 */   971,  971,  971,  971,  971,  971,  338,
};
static const YYCODETYPE yy_lookahead[] = {
 /*     0 */    19,   22,   22,   23,    1,   24,   26,   15,   27,   80,
 /*    10 */    81,   82,   83,   84,   85,   86,   87,   88,   89,   90,
 /*    20 */    91,   92,   93,   94,   95,  108,  109,  110,   27,   28,
 /*    30 */    23,   50,   51,   80,   81,   82,   83,  122,   85,   86,
 /*    40 */    87,   88,   89,   90,   91,   92,   93,   94,   95,   22,
 /*    50 */    70,   23,   71,   72,   73,   74,   75,   76,   77,   78,
 /*    60 */    79,   80,   81,   82,   83,  122,   85,   86,   87,   88,
 /*    70 */    89,   90,   91,   92,   93,   94,   95,   19,   97,   91,
 /*    80 */    92,   93,   94,   95,   26,   85,   86,   87,   88,   89,
 /*    90 */    90,   91,   92,   93,   94,   95,   27,   28,   97,   98,
 /*   100 */    99,  122,  211,  102,  103,  104,   79,   19,   50,   51,
 /*   110 */    19,  122,   59,   55,  113,  224,  225,  226,   89,   90,
 /*   120 */    91,   92,   93,   94,   95,   23,   27,   28,   26,   71,
 /*   130 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   140 */    82,   83,   51,   85,   86,   87,   88,   89,   90,   91,
 /*   150 */    92,   93,   94,   95,   19,  132,  133,   58,   89,   90,
 /*   160 */    21,  108,  109,  110,   27,   28,   97,   98,   33,  100,
 /*   170 */     7,    8,  119,  120,   22,   19,  107,   42,  109,   27,
 /*   180 */    28,   27,   28,   95,   28,   50,   51,   99,  100,  101,
 /*   190 */   102,  103,  104,  105,   27,   28,   97,   98,  107,  152,
 /*   200 */   112,  132,  133,  112,   65,   69,   71,   72,   73,   74,
 /*   210 */    75,   76,   77,   78,   79,   80,   81,   82,   83,   11,
 /*   220 */    85,   86,   87,   88,   89,   90,   91,   92,   93,   94,
 /*   230 */    95,   19,  101,   97,   97,   98,   24,  101,  122,  157,
 /*   240 */    12,   99,  103,  112,  102,  103,  104,  152,   22,   97,
 /*   250 */    98,   97,   98,   27,   28,  113,   27,   29,   91,  164,
 /*   260 */   165,  124,   50,   51,   97,   98,  219,   59,  132,  133,
 /*   270 */   134,   22,   23,   45,   66,   47,  212,  213,  124,  140,
 /*   280 */   132,  133,   19,   71,   72,   73,   74,   75,   76,   77,
 /*   290 */    78,   79,   80,   81,   82,   83,  152,   85,   86,   87,
 /*   300 */    88,   89,   90,   91,   92,   93,   94,   95,  164,  165,
 /*   310 */    27,   28,  230,   50,   51,  233,  108,  109,  110,   70,
 /*   320 */    16,   59,   23,   97,   98,   26,   97,   22,   66,  185,
 /*   330 */    12,  187,   27,   28,   71,   72,   73,   74,   75,   76,
 /*   340 */    77,   78,   79,   80,   81,   82,   83,   29,   85,   86,
 /*   350 */    87,   88,   89,   90,   91,   92,   93,   94,   95,   19,
 /*   360 */    22,  148,  149,   45,   23,   47,   62,  154,   64,  156,
 /*   370 */   108,  109,  110,   37,   69,   23,  163,   59,   26,   26,
 /*   380 */    97,   98,  144,  145,  146,  147,  152,  200,   52,   23,
 /*   390 */    50,   51,   26,   22,   89,   90,   60,  210,    7,    8,
 /*   400 */     9,  138,   97,   22,   23,   26,  101,   26,  174,  175,
 /*   410 */   197,   71,   72,   73,   74,   75,   76,   77,   78,   79,
 /*   420 */    80,   81,   82,   83,   16,   85,   86,   87,   88,   89,
 /*   430 */    90,   91,   92,   93,   94,   95,   19,  132,  133,  134,
 /*   440 */    23,  152,  208,  209,  140,  152,  152,  111,  195,  196,
 /*   450 */    98,   70,  163,  160,  152,   23,   22,  164,  165,  246,
 /*   460 */   207,   27,  152,  174,  175,  171,  172,   50,   51,  137,
 /*   470 */    62,  139,   64,  171,  172,  222,  124,   27,  138,   24,
 /*   480 */   163,   89,   90,  130,  174,  175,  197,  163,   71,   72,
 /*   490 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   500 */    83,   22,   85,   86,   87,   88,   89,   90,   91,   92,
 /*   510 */    93,   94,   95,   19,  197,  181,  182,   23,  208,  209,
 /*   520 */   152,  197,   26,  189,  132,  133,  232,  224,  225,  226,
 /*   530 */   152,   97,   91,   26,  232,  116,  212,  213,  152,  222,
 /*   540 */   121,  152,  174,  175,   50,   51,  243,   97,   22,   23,
 /*   550 */    22,  234,  174,  175,  177,   23,  239,  116,  163,  177,
 /*   560 */   174,  175,  121,  174,  175,   71,   72,   73,   74,   75,
 /*   570 */    76,   77,   78,   79,   80,   81,   82,   83,   24,   85,
 /*   580 */    86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
 /*   590 */    19,   23,  197,   11,   23,  227,   70,  208,  220,  152,
 /*   600 */    31,  224,  225,  226,   35,   98,  224,  225,  226,  108,
 /*   610 */   109,  110,  115,  152,  117,  118,   27,  222,   49,  123,
 /*   620 */    24,   50,   51,   27,    0,    1,    2,  224,  225,  226,
 /*   630 */   166,  124,  168,  169,  239,  174,  175,  170,  171,  172,
 /*   640 */    22,  194,   71,   72,   73,   74,   75,   76,   77,   78,
 /*   650 */    79,   80,   81,   82,   83,  152,   85,   86,   87,   88,
 /*   660 */    89,   90,   91,   92,   93,   94,   95,   19,   22,  208,
 /*   670 */    24,   23,  195,  196,  170,  171,  172,  174,  175,  152,
 /*   680 */    26,  152,  152,  152,  207,  152,   97,  152,   23,  152,
 /*   690 */    51,  244,  152,   97,  152,  247,  248,   23,   50,   51,
 /*   700 */    26,  174,  175,  174,  175,  174,  175,  174,  175,  174,
 /*   710 */   175,  174,  175,   23,  174,  175,  174,  175,  188,   71,
 /*   720 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   730 */    82,   83,  152,   85,   86,   87,   88,   89,   90,   91,
 /*   740 */    92,   93,   94,   95,   19,  152,  107,  152,   33,   24,
 /*   750 */   152,  100,  101,   27,  174,  175,  152,   42,  152,   23,
 /*   760 */   152,   26,  152,   23,  152,   26,  152,  174,  175,  174,
 /*   770 */   175,  152,  174,  175,   23,   50,   51,   26,  174,  175,
 /*   780 */   174,  175,  174,  175,  174,  175,  174,  175,  174,  175,
 /*   790 */   163,  119,  120,  174,  175,   19,   71,   72,   73,   74,
 /*   800 */    75,   76,   77,   78,   79,   80,   81,   82,   83,  152,
 /*   810 */    85,   86,   87,   88,   89,   90,   91,   92,   93,   94,
 /*   820 */    95,   66,  152,   97,  197,   23,   50,   51,   26,   53,
 /*   830 */    23,  174,  175,   26,   23,   23,   23,   26,   26,   26,
 /*   840 */    36,  106,  146,  147,  174,  175,   19,   71,   72,   73,
 /*   850 */    74,   75,   76,   77,   78,   79,   80,   81,   82,   83,
 /*   860 */   152,   85,   86,   87,   88,   89,   90,   91,   92,   93,
 /*   870 */    94,   95,  152,  196,  119,  120,   19,   50,   51,  168,
 /*   880 */   169,   26,  174,  175,  207,   28,  152,  249,  250,  152,
 /*   890 */   163,  163,  163,  163,  174,  175,  163,   19,   71,   72,
 /*   900 */    73,   74,   75,   76,   77,   78,   79,   80,   81,   82,
 /*   910 */    83,  152,   85,   86,   87,   88,   89,   90,   91,   92,
 /*   920 */    93,   94,   95,  152,  197,  197,  197,  197,   50,   51,
 /*   930 */   197,  194,   36,  174,  175,  191,  192,  152,  191,  192,
 /*   940 */   163,  152,   66,  124,  152,  174,  175,  152,   19,   71,
 /*   950 */    72,   73,   74,   75,   76,   77,   78,   79,   80,   81,
 /*   960 */    82,   83,  152,   85,   86,   87,   88,   89,   90,   91,
 /*   970 */    92,   93,   94,   95,  197,  152,  100,  188,  152,   50,
 /*   980 */    51,  152,  152,  188,  174,  175,  252,  152,   94,   95,
 /*   990 */   152,  152,  152,    1,    2,  152,  152,  174,  175,   19,
 /*  1000 */   152,   72,   73,   74,   75,   76,   77,   78,   79,   80,
 /*  1010 */    81,   82,   83,  152,   85,   86,   87,   88,   89,   90,
 /*  1020 */    91,   92,   93,   94,   95,  152,  188,  188,   22,  194,
 /*  1030 */    50,   51,  240,  173,  194,  174,  175,  252,  194,  152,
 /*  1040 */    36,  181,   28,  152,   23,  219,  122,  174,  175,  219,
 /*  1050 */   221,  152,  152,   73,   74,   75,   76,   77,   78,   79,
 /*  1060 */    80,   81,   82,   83,  152,   85,   86,   87,   88,   89,
 /*  1070 */    90,   91,   92,   93,   94,   95,   19,   20,  152,   22,
 /*  1080 */    23,  194,  152,  240,   27,   28,  174,  175,  240,   19,
 /*  1090 */    20,   26,   22,  194,  194,   38,   22,   27,   28,  152,
 /*  1100 */    23,   22,  152,  116,  174,  175,  152,   23,   38,  152,
 /*  1110 */    23,  152,  221,  152,   57,  152,   23,  163,   50,   51,
 /*  1120 */   194,  174,  175,   66,  174,  175,   69,   57,  174,  175,
 /*  1130 */    40,  174,  175,  174,  175,  174,  175,  174,  175,   69,
 /*  1140 */    22,   53,   74,   75,   30,   53,   89,   90,   22,   22,
 /*  1150 */   152,  197,   23,   96,   97,   98,   22,  152,  101,   89,
 /*  1160 */    90,   91,  208,  209,  152,   53,   96,   97,   98,  101,
 /*  1170 */    22,  101,  174,  175,  152,   19,   20,  105,   22,  174,
 /*  1180 */   175,  112,   19,   27,   28,   20,  174,  175,   24,  132,
 /*  1190 */   133,  134,  135,  136,   38,   44,  174,  175,  107,   61,
 /*  1200 */    54,   26,  132,  133,  134,  135,  136,   54,  107,   22,
 /*  1210 */     5,  140,    1,   57,   36,  111,  122,   28,   79,   79,
 /*  1220 */   131,  123,   66,   19,   20,   69,   22,    1,   16,   20,
 /*  1230 */   125,   27,   28,  123,  111,  120,   23,  131,   23,   16,
 /*  1240 */    68,  142,   38,   15,   22,   89,   90,    3,  167,    4,
 /*  1250 */   248,  251,   96,   97,   98,  180,  180,  101,  251,  151,
 /*  1260 */     6,   57,  151,   13,  151,   26,   25,  151,  161,  202,
 /*  1270 */   153,  162,  153,   69,  130,  128,  203,   19,   20,  127,
 /*  1280 */    22,  126,  204,  129,   22,   27,   28,  205,  132,  133,
 /*  1290 */   134,  135,  136,   89,   90,  231,   38,   95,  137,  179,
 /*  1300 */    96,   97,   98,  206,  179,  101,  122,  107,  159,  159,
 /*  1310 */   125,  231,  216,  228,  107,   57,  184,  217,  216,  176,
 /*  1320 */   217,  176,   48,  106,   18,  184,  158,   69,  159,  158,
 /*  1330 */    46,   71,  237,  176,  176,  176,  132,  133,  134,  135,
 /*  1340 */   136,  217,  176,  137,  216,  178,  158,   89,   90,  179,
 /*  1350 */   176,  159,  179,  159,   96,   97,   98,  159,  159,  101,
 /*  1360 */     5,  158,  202,   22,   18,   10,   11,   12,   13,   14,
 /*  1370 */   190,  238,   17,  190,  158,  193,   41,  159,  202,  193,
 /*  1380 */   159,  202,  245,  193,  193,  223,  190,   32,  159,   34,
 /*  1390 */   132,  133,  134,  135,  136,  159,   39,  155,   43,  150,
 /*  1400 */   223,  177,  201,  178,  177,  186,   66,  199,  177,  152,
 /*  1410 */   253,   56,  215,  152,  182,  152,  202,  152,   63,  152,
 /*  1420 */   152,   66,   67,  242,  229,  152,  174,  152,  152,  152,
 /*  1430 */   152,  152,  152,  152,  199,  242,  202,  152,  198,  152,
 /*  1440 */   152,  152,  183,  192,  152,  215,  152,  183,  215,  183,
 /*  1450 */   152,  241,  214,  152,  211,  152,  152,  211,  211,  152,
 /*  1460 */   152,  241,  152,  152,  152,  152,  152,  152,  152,  114,
 /*  1470 */   152,  152,  235,  152,  152,  152,  174,  187,   95,  174,
 /*  1480 */   253,  253,  253,  253,  236,  253,  253,  253,  253,  253,
 /*  1490 */   253,  253,  253,  253,  253,  253,  141,
};
#define YY_SHIFT_USE_DFLT (-86)
#define YY_SHIFT_COUNT (429)
#define YY_SHIFT_MIN   (-85)
#define YY_SHIFT_MAX   (1383)
static const short yy_shift_ofst[] = {
 /*     0 */   992, 1057, 1355, 1156, 1204, 1204,    1,  262,  -19,  135,
 /*    10 */   135,  776, 1204, 1204, 1204, 1204,   69,   69,   53,  208,
 /*    20 */   283,  755,   58,  725,  648,  571,  494,  417,  340,  263,
 /*    30 */   212,  827,  827,  827,  827,  827,  827,  827,  827,  827,
 /*    40 */   827,  827,  827,  827,  827,  827,  878,  827,  929,  980,
 /*    50 */   980, 1070, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    60 */  1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    70 */  1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    80 */  1258, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204, 1204,
 /*    90 */  1204, 1204, 1204, 1204,  -71,  -47,  -47,  -47,  -47,  -47,
 /*   100 */     0,   29,  -12,  283,  283,  139,   91,  392,  392,  894,
 /*   110 */   672,  726, 1383,  -86,  -86,  -86,   88,  318,  318,   99,
 /*   120 */   381,  -20,  283,  283,  283,  283,  283,  283,  283,  283,
 /*   130 */   283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
 /*   140 */   283,  283,  283,  283,  624,  876,  726,  672, 1340, 1340,
 /*   150 */  1340, 1340, 1340, 1340,  -86,  -86,  -86,  305,  136,  136,
 /*   160 */   142,  167,  226,  154,  137,  152,  283,  283,  283,  283,
 /*   170 */   283,  283,  283,  283,  283,  283,  283,  283,  283,  283,
 /*   180 */   283,  283,  283,  336,  336,  336,  283,  283,  352,  283,
 /*   190 */   283,  283,  283,  283,  228,  283,  283,  283,  283,  283,
 /*   200 */   283,  283,  283,  283,  283,  501,  569,  596,  596,  596,
 /*   210 */   507,  497,  441,  391,  353,  156,  156,  857,  353,  857,
 /*   220 */   735,  813,  639,  715,  156,  332,  715,  715,  496,  419,
 /*   230 */   646, 1357, 1184, 1184, 1335, 1335, 1184, 1341, 1260, 1144,
 /*   240 */  1346, 1346, 1346, 1346, 1184, 1306, 1144, 1341, 1260, 1260,
 /*   250 */  1144, 1184, 1306, 1206, 1284, 1184, 1184, 1306, 1184, 1306,
 /*   260 */  1184, 1306, 1262, 1207, 1207, 1207, 1274, 1262, 1207, 1217,
 /*   270 */  1207, 1274, 1207, 1207, 1185, 1200, 1185, 1200, 1185, 1200,
 /*   280 */  1184, 1184, 1161, 1262, 1202, 1202, 1262, 1154, 1155, 1147,
 /*   290 */  1152, 1144, 1241, 1239, 1250, 1250, 1254, 1254, 1254, 1254,
 /*   300 */   -86,  -86,  -86,  -86,  -86,  -86, 1068,  304,  526,  249,
 /*   310 */   408,  -83,  434,  812,   27,  811,  807,  802,  751,  589,
 /*   320 */   651,  163,  131,  674,  366,  450,  299,  148,   23,  102,
 /*   330 */   229,  -21, 1245, 1244, 1222, 1099, 1228, 1172, 1223, 1215,
 /*   340 */  1213, 1115, 1106, 1123, 1110, 1209, 1105, 1212, 1226, 1098,
 /*   350 */  1089, 1140, 1139, 1104, 1189, 1178, 1094, 1211, 1205, 1187,
 /*   360 */  1101, 1071, 1153, 1175, 1146, 1138, 1151, 1091, 1164, 1165,
 /*   370 */  1163, 1069, 1072, 1148, 1112, 1134, 1127, 1129, 1126, 1092,
 /*   380 */  1114, 1118, 1088, 1090, 1093, 1087, 1084,  987, 1079, 1077,
 /*   390 */  1074, 1065,  924, 1021, 1014, 1004, 1006,  819,  739,  896,
 /*   400 */   855,  804,  739,  740,  736,  690,  654,  665,  618,  582,
 /*   410 */   568,  528,  554,  379,  532,  479,  455,  379,  432,  371,
 /*   420 */   341,   28,  338,  116,  -11,  -57,  -85,    7,   -8,    3,
};
#define YY_REDUCE_USE_DFLT (-110)
#define YY_REDUCE_COUNT (305)
#define YY_REDUCE_MIN   (-109)
#define YY_REDUCE_MAX   (1323)
static const short yy_reduce_ofst[] = {
 /*     0 */   238,  954,  213,  289,  310,  234,  144,  317, -109,  382,
 /*    10 */   377,  303,  461,  389,  378,  368,  302,  294,  253,  395,
 /*    20 */   293,  324,  403,  403,  403,  403,  403,  403,  403,  403,
 /*    30 */   403,  403,  403,  403,  403,  403,  403,  403,  403,  403,
 /*    40 */   403,  403,  403,  403,  403,  403,  403,  403,  403,  403,
 /*    50 */   403, 1022, 1012, 1005,  998,  963,  961,  959,  957,  950,
 /*    60 */   947,  930,  912,  873,  861,  823,  810,  771,  759,  720,
 /*    70 */   708,  670,  657,  619,  614,  612,  610,  608,  606,  604,
 /*    80 */   598,  595,  593,  580,  542,  540,  537,  535,  533,  531,
 /*    90 */   529,  527,  503,  386,  403,  403,  403,  403,  403,  403,
 /*   100 */   403,  403,  403,   95,  447,   82,  334,  504,  467,  403,
 /*   110 */   477,  464,  403,  403,  403,  403,  860,  747,  744,  785,
 /*   120 */   638,  638,  926,  891,  900,  899,  887,  844,  840,  835,
 /*   130 */   848,  830,  843,  829,  792,  839,  826,  737,  838,  795,
 /*   140 */   789,   47,  734,  530,  696,  777,  711,  677,  733,  730,
 /*   150 */   729,  728,  727,  627,  448,   64,  187, 1305, 1302, 1252,
 /*   160 */  1290, 1273, 1323, 1322, 1321, 1319, 1318, 1316, 1315, 1314,
 /*   170 */  1313, 1312, 1311, 1310, 1308, 1307, 1304, 1303, 1301, 1298,
 /*   180 */  1294, 1292, 1289, 1266, 1264, 1259, 1288, 1287, 1238, 1285,
 /*   190 */  1281, 1280, 1279, 1278, 1251, 1277, 1276, 1275, 1273, 1268,
 /*   200 */  1267, 1265, 1263, 1261, 1257, 1248, 1237, 1247, 1246, 1243,
 /*   210 */  1238, 1240, 1235, 1249, 1234, 1233, 1230, 1220, 1214, 1210,
 /*   220 */  1225, 1219, 1232, 1231, 1197, 1195, 1227, 1224, 1201, 1208,
 /*   230 */  1242, 1137, 1236, 1229, 1193, 1181, 1221, 1177, 1196, 1179,
 /*   240 */  1191, 1190, 1186, 1182, 1218, 1216, 1176, 1162, 1183, 1180,
 /*   250 */  1160, 1199, 1203, 1133, 1095, 1198, 1194, 1188, 1192, 1171,
 /*   260 */  1169, 1168, 1173, 1174, 1166, 1159, 1141, 1170, 1158, 1167,
 /*   270 */  1157, 1132, 1145, 1143, 1124, 1128, 1103, 1102, 1100, 1096,
 /*   280 */  1150, 1149, 1085, 1125, 1080, 1064, 1120, 1097, 1082, 1078,
 /*   290 */  1073, 1067, 1109, 1107, 1119, 1117, 1116, 1113, 1111, 1108,
 /*   300 */  1007, 1000, 1002, 1076, 1075, 1081,
};
static const YYACTIONTYPE yy_default[] = {
 /*     0 */   647,  964,  964,  964,  878,  878,  969,  964,  774,  802,
 /*    10 */   802,  938,  969,  969,  969,  876,  969,  969,  969,  964,
 /*    20 */   969,  778,  808,  969,  969,  969,  969,  969,  969,  969,
 /*    30 */   969,  937,  939,  816,  815,  918,  789,  813,  806,  810,
 /*    40 */   879,  872,  873,  871,  875,  880,  969,  809,  841,  856,
 /*    50 */   840,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*    60 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*    70 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*    80 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*    90 */   969,  969,  969,  969,  850,  855,  862,  854,  851,  843,
 /*   100 */   842,  844,  845,  969,  969,  673,  739,  969,  969,  846,
 /*   110 */   969,  685,  847,  859,  858,  857,  680,  969,  969,  969,
 /*   120 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   130 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   140 */   969,  969,  969,  969,  647,  964,  969,  969,  964,  964,
 /*   150 */   964,  964,  964,  964,  956,  778,  768,  969,  969,  969,
 /*   160 */   969,  969,  969,  969,  969,  969,  969,  944,  942,  969,
 /*   170 */   891,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   180 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   190 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   200 */   969,  969,  969,  969,  653,  969,  911,  774,  774,  774,
 /*   210 */   776,  754,  766,  655,  812,  791,  791,  923,  812,  923,
 /*   220 */   710,  733,  707,  802,  791,  874,  802,  802,  775,  766,
 /*   230 */   969,  949,  782,  782,  941,  941,  782,  821,  743,  812,
 /*   240 */   750,  750,  750,  750,  782,  670,  812,  821,  743,  743,
 /*   250 */   812,  782,  670,  917,  915,  782,  782,  670,  782,  670,
 /*   260 */   782,  670,  884,  741,  741,  741,  725,  884,  741,  710,
 /*   270 */   741,  725,  741,  741,  795,  790,  795,  790,  795,  790,
 /*   280 */   782,  782,  969,  884,  888,  888,  884,  807,  796,  805,
 /*   290 */   803,  812,  676,  728,  663,  663,  652,  652,  652,  652,
 /*   300 */   961,  961,  956,  712,  712,  695,  969,  969,  969,  969,
 /*   310 */   969,  969,  687,  969,  893,  969,  969,  969,  969,  969,
 /*   320 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   330 */   969,  828,  969,  648,  951,  969,  969,  948,  969,  969,
 /*   340 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   350 */   969,  969,  969,  969,  969,  969,  921,  969,  969,  969,
 /*   360 */   969,  969,  969,  914,  913,  969,  969,  969,  969,  969,
 /*   370 */   969,  969,  969,  969,  969,  969,  969,  969,  969,  969,
 /*   380 */   969,  969,  969,  969,  969,  969,  969,  757,  969,  969,
 /*   390 */   969,  761,  969,  969,  969,  969,  969,  969,  804,  969,
 /*   400 */   797,  969,  877,  969,  969,  969,  969,  969,  969,  969,
 /*   410 */   969,  969,  969,  966,  969,  969,  969,  965,  969,  969,
 /*   420 */   969,  969,  969,  830,  969,  829,  833,  969,  661,  969,
 /*   430 */   644,  649,  960,  963,  962,  959,  958,  957,  952,  950,
 /*   440 */   947,  946,  945,  943,  940,  936,  897,  895,  902,  901,
 /*   450 */   900,  899,  898,  896,  894,  892,  818,  817,  814,  811,
 /*   460 */   753,  935,  890,  752,  749,  748,  669,  953,  920,  929,
 /*   470 */   928,  927,  822,  926,  925,  924,  922,  919,  906,  820,
 /*   480 */   819,  744,  882,  881,  672,  910,  909,  908,  912,  916,
 /*   490 */   907,  784,  751,  671,  668,  675,  679,  731,  732,  740,
 /*   500 */   738,  737,  736,  735,  734,  730,  681,  686,  724,  709,
 /*   510 */   708,  717,  716,  722,  721,  720,  719,  718,  715,  714,
 /*   520 */   713,  706,  705,  711,  704,  727,  726,  723,  703,  747,
 /*   530 */   746,  745,  742,  702,  701,  700,  833,  699,  698,  838,
 /*   540 */   837,  866,  826,  755,  759,  758,  762,  763,  771,  770,
 /*   550 */   769,  780,  781,  793,  792,  824,  823,  794,  779,  773,
 /*   560 */   772,  788,  787,  786,  785,  777,  767,  799,  798,  868,
 /*   570 */   783,  867,  865,  934,  933,  932,  931,  930,  870,  967,
 /*   580 */   968,  887,  889,  886,  801,  800,  885,  869,  839,  836,
 /*   590 */   690,  691,  905,  904,  903,  693,  692,  689,  688,  863,
 /*   600 */   860,  852,  864,  861,  853,  849,  848,  834,  832,  831,
 /*   610 */   827,  835,  760,  756,  825,  765,  764,  697,  696,  694,
 /*   620 */   678,  677,  674,  667,  665,  664,  666,  662,  660,  659,
 /*   630 */   658,  657,  656,  684,  683,  682,  654,  651,  650,  646,
 /*   640 */   645,  643,
};

/* The next table maps tokens into fallback tokens.  If a construct
** like the following:
** 
**      %fallback ID X Y Z.
**
** appears in the grammar, then ID becomes a fallback token for X, Y,
** and Z.  Whenever one of the tokens X, Y, or Z is input to the parser
** but it does not parse, the type of the token is changed to ID and
** the parse is retried before an error is thrown.
*/
#ifdef YYFALLBACK
static const YYCODETYPE yyFallback[] = {
    0,  /*          $ => nothing */
    0,  /*       SEMI => nothing */
   27,  /*    EXPLAIN => ID */
   27,  /*      QUERY => ID */
   27,  /*       PLAN => ID */
   27,  /*      BEGIN => ID */
    0,  /* TRANSACTION => nothing */
   27,  /*   DEFERRED => ID */
   27,  /*  IMMEDIATE => ID */
   27,  /*  EXCLUSIVE => ID */
    0,  /*     COMMIT => nothing */
   27,  /*        END => ID */
   27,  /*   ROLLBACK => ID */
   27,  /*  SAVEPOINT => ID */
   27,  /*    RELEASE => ID */
    0,  /*         TO => nothing */
    0,  /*      TABLE => nothing */
    0,  /*     CREATE => nothing */
   27,  /*         IF => ID */
    0,  /*        NOT => nothing */
    0,  /*     EXISTS => nothing */
   27,  /*       TEMP => ID */
    0,  /*         LP => nothing */
    0,  /*         RP => nothing */
    0,  /*         AS => nothing */
   27,  /*    WITHOUT => ID */
    0,  /*      COMMA => nothing */
    0,  /*         ID => nothing */
    0,  /*    INDEXED => nothing */
   27,  /*      ABORT => ID */
   27,  /*     ACTION => ID */
   27,  /*      AFTER => ID */
   27,  /*    ANALYZE => ID */
   27,  /*        ASC => ID */
   27,  /*     ATTACH => ID */
   27,  /*     BEFORE => ID */
   27,  /*         BY => ID */
   27,  /*    CASCADE => ID */
   27,  /*       CAST => ID */
   27,  /*   COLUMNKW => ID */
   27,  /*   CONFLICT => ID */
   27,  /*   DATABASE => ID */
   27,  /*       DESC => ID */
   27,  /*     DETACH => ID */
   27,  /*       EACH => ID */
   27,  /*       FAIL => ID */
   27,  /*        FOR => ID */
   27,  /*     IGNORE => ID */
   27,  /*  INITIALLY => ID */
   27,  /*    INSTEAD => ID */
   27,  /*    LIKE_KW => ID */
   27,  /*      MATCH => ID */
   27,  /*         NO => ID */
   27,  /*        KEY => ID */
   27,  /*         OF => ID */
   27,  /*     OFFSET => ID */
   27,  /*     PRAGMA => ID */
   27,  /*      RAISE => ID */
   27,  /*  RECURSIVE => ID */
   27,  /*    REPLACE => ID */
   27,  /*   RESTRICT => ID */
   27,  /*        ROW => ID */
   27,  /*    TRIGGER => ID */
   27,  /*     VACUUM => ID */
   27,  /*       VIEW => ID */
   27,  /*    VIRTUAL => ID */
   27,  /*       WITH => ID */
   27,  /*    REINDEX => ID */
   27,  /*     RENAME => ID */
   27,  /*   CTIME_KW => ID */
};
#endif /* YYFALLBACK */

/* The following structure represents a single element of the
** parser's stack.  Information stored includes:
**
**   +  The state number for the parser at this level of the stack.
**
**   +  The value of the token stored at this level of the stack.
**      (In other words, the "major" token.)
**
**   +  The semantic value stored at this level of the stack.  This is
**      the information used by the action routines in the grammar.
**      It is sometimes called the "minor" token.
*/
struct yyStackEntry {
  YYACTIONTYPE stateno;  /* The state-number */
  YYCODETYPE major;      /* The major token value.  This is the code
                         ** number for the token at this stack level */
  YYMINORTYPE minor;     /* The user-supplied minor token value.  This
                         ** is the value of the token  */
};
typedef struct yyStackEntry yyStackEntry;

/* The state of the parser is completely contained in an instance of
** the following structure */
struct yyParser {
  int yyidx;                    /* Index of top element in stack */
#ifdef YYTRACKMAXSTACKDEPTH
  int yyidxMax;                 /* Maximum value of yyidx */
#endif
  int yyerrcnt;                 /* Shifts left before out of the error */
  sqlite3ParserARG_SDECL                /* A place to hold %extra_argument */
#if YYSTACKDEPTH<=0
  int yystksz;                  /* Current side of the stack */
  yyStackEntry *yystack;        /* The parser's stack */
#else
  yyStackEntry yystack[YYSTACKDEPTH];  /* The parser's stack */
#endif
};
typedef struct yyParser yyParser;

#ifndef NDEBUG
/* #include <stdio.h> */
static FILE *yyTraceFILE = 0;
static char *yyTracePrompt = 0;
#endif /* NDEBUG */

#ifndef NDEBUG
/* 
** Turn parser tracing on by giving a stream to which to write the trace
** and a prompt to preface each trace message.  Tracing is turned off
** by making either argument NULL 
**
** Inputs:
** <ul>
** <li> A FILE* to which trace output should be written.
**      If NULL, then tracing is turned off.
** <li> A prefix string written at the beginning of every
**      line of trace output.  If NULL, then tracing is
**      turned off.
** </ul>
**
** Outputs:
** None.
*/
SQLITE_PRIVATE void sqlite3ParserTrace(FILE *TraceFILE, char *zTracePrompt){
  yyTraceFILE = TraceFILE;
  yyTracePrompt = zTracePrompt;
  if( yyTraceFILE==0 ) yyTracePrompt = 0;
  else if( yyTracePrompt==0 ) yyTraceFILE = 0;
}
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing shifts, the names of all terminals and nonterminals
** are required.  The following table supplies these names */
static const char *const yyTokenName[] = { 
  "$",             "SEMI",          "EXPLAIN",       "QUERY",       
  "PLAN",          "BEGIN",         "TRANSACTION",   "DEFERRED",    
  "IMMEDIATE",     "EXCLUSIVE",     "COMMIT",        "END",         
  "ROLLBACK",      "SAVEPOINT",     "RELEASE",       "TO",          
  "TABLE",         "CREATE",        "IF",            "NOT",         
  "EXISTS",        "TEMP",          "LP",            "RP",          
  "AS",            "WITHOUT",       "COMMA",         "ID",          
  "INDEXED",       "ABORT",         "ACTION",        "AFTER",       
  "ANALYZE",       "ASC",           "ATTACH",        "BEFORE",      
  "BY",            "CASCADE",       "CAST",          "COLUMNKW",    
  "CONFLICT",      "DATABASE",      "DESC",          "DETACH",      
  "EACH",          "FAIL",          "FOR",           "IGNORE",      
  "INITIALLY",     "INSTEAD",       "LIKE_KW",       "MATCH",       
  "NO",            "KEY",           "OF",            "OFFSET",      
  "PRAGMA",        "RAISE",         "RECURSIVE",     "REPLACE",     
  "RESTRICT",      "ROW",           "TRIGGER",       "VACUUM",      
  "VIEW",          "VIRTUAL",       "WITH",          "REINDEX",     
  "RENAME",        "CTIME_KW",      "ANY",           "OR",          
  "AND",           "IS",            "BETWEEN",       "IN",          
  "ISNULL",        "NOTNULL",       "NE",            "EQ",          
  "GT",            "LE",            "LT",            "GE",          
  "ESCAPE",        "BITAND",        "BITOR",         "LSHIFT",      
  "RSHIFT",        "PLUS",          "MINUS",         "STAR",        
  "SLASH",         "REM",           "CONCAT",        "COLLATE",     
  "BITNOT",        "STRING",        "JOIN_KW",       "CONSTRAINT",  
  "DEFAULT",       "NULL",          "PRIMARY",       "UNIQUE",      
  "CHECK",         "REFERENCES",    "AUTOINCR",      "ON",          
  "INSERT",        "DELETE",        "UPDATE",        "SET",         
  "DEFERRABLE",    "FOREIGN",       "DROP",          "UNION",       
  "ALL",           "EXCEPT",        "INTERSECT",     "SELECT",      
  "VALUES",        "DISTINCT",      "DOT",           "FROM",        
  "JOIN",          "USING",         "ORDER",         "GROUP",       
  "HAVING",        "LIMIT",         "WHERE",         "INTO",        
  "INTEGER",       "FLOAT",         "BLOB",          "VARIABLE",    
  "CASE",          "WHEN",          "THEN",          "ELSE",        
  "INDEX",         "ALTER",         "ADD",           "error",       
  "input",         "cmdlist",       "ecmd",          "explain",     
  "cmdx",          "cmd",           "transtype",     "trans_opt",   
  "nm",            "savepoint_opt",  "create_table",  "create_table_args",
  "createkw",      "temp",          "ifnotexists",   "dbnm",        
  "columnlist",    "conslist_opt",  "table_options",  "select",      
  "column",        "columnid",      "type",          "carglist",    
  "typetoken",     "typename",      "signed",        "plus_num",    
  "minus_num",     "ccons",         "term",          "expr",        
  "onconf",        "sortorder",     "autoinc",       "idxlist_opt", 
  "refargs",       "defer_subclause",  "refarg",        "refact",      
  "init_deferred_pred_opt",  "conslist",      "tconscomma",    "tcons",       
  "idxlist",       "defer_subclause_opt",  "orconf",        "resolvetype", 
  "raisetype",     "ifexists",      "fullname",      "selectnowith",
  "oneselect",     "with",          "multiselect_op",  "distinct",    
  "selcollist",    "from",          "where_opt",     "groupby_opt", 
  "having_opt",    "orderby_opt",   "limit_opt",     "values",      
  "nexprlist",     "exprlist",      "sclp",          "as",          
  "seltablist",    "stl_prefix",    "joinop",        "indexed_opt", 
  "on_opt",        "using_opt",     "joinop2",       "idlist",      
  "sortlist",      "setlist",       "insert_cmd",    "inscollist_opt",
  "likeop",        "between_op",    "in_op",         "case_operand",
  "case_exprlist",  "case_else",     "uniqueflag",    "collate",     
  "nmnum",         "trigger_decl",  "trigger_cmd_list",  "trigger_time",
  "trigger_event",  "foreach_clause",  "when_clause",   "trigger_cmd", 
  "trnm",          "tridxby",       "database_kw_opt",  "key_opt",     
  "add_column_fullname",  "kwcolumn_opt",  "create_vtab",   "vtabarglist", 
  "vtabarg",       "vtabargtoken",  "lp",            "anylist",     
  "wqlist",      
};
#endif /* NDEBUG */

#ifndef NDEBUG
/* For tracing reduce actions, the names of all rules are required.
*/
static const char *const yyRuleName[] = {
 /*   0 */ "input ::= cmdlist",
 /*   1 */ "cmdlist ::= cmdlist ecmd",
 /*   2 */ "cmdlist ::= ecmd",
 /*   3 */ "ecmd ::= SEMI",
 /*   4 */ "ecmd ::= explain cmdx SEMI",
 /*   5 */ "explain ::=",
 /*   6 */ "explain ::= EXPLAIN",
 /*   7 */ "explain ::= EXPLAIN QUERY PLAN",
 /*   8 */ "cmdx ::= cmd",
 /*   9 */ "cmd ::= BEGIN transtype trans_opt",
 /*  10 */ "trans_opt ::=",
 /*  11 */ "trans_opt ::= TRANSACTION",
 /*  12 */ "trans_opt ::= TRANSACTION nm",
 /*  13 */ "transtype ::=",
 /*  14 */ "transtype ::= DEFERRED",
 /*  15 */ "transtype ::= IMMEDIATE",
 /*  16 */ "transtype ::= EXCLUSIVE",
 /*  17 */ "cmd ::= COMMIT trans_opt",
 /*  18 */ "cmd ::= END trans_opt",
 /*  19 */ "cmd ::= ROLLBACK trans_opt",
 /*  20 */ "savepoint_opt ::= SAVEPOINT",
 /*  21 */ "savepoint_opt ::=",
 /*  22 */ "cmd ::= SAVEPOINT nm",
 /*  23 */ "cmd ::= RELEASE savepoint_opt nm",
 /*  24 */ "cmd ::= ROLLBACK trans_opt TO savepoint_opt nm",
 /*  25 */ "cmd ::= create_table create_table_args",
 /*  26 */ "create_table ::= createkw temp TABLE ifnotexists nm dbnm",
 /*  27 */ "createkw ::= CREATE",
 /*  28 */ "ifnotexists ::=",
 /*  29 */ "ifnotexists ::= IF NOT EXISTS",
 /*  30 */ "temp ::= TEMP",
 /*  31 */ "temp ::=",
 /*  32 */ "create_table_args ::= LP columnlist conslist_opt RP table_options",
 /*  33 */ "create_table_args ::= AS select",
 /*  34 */ "table_options ::=",
 /*  35 */ "table_options ::= WITHOUT nm",
 /*  36 */ "columnlist ::= columnlist COMMA column",
 /*  37 */ "columnlist ::= column",
 /*  38 */ "column ::= columnid type carglist",
 /*  39 */ "columnid ::= nm",
 /*  40 */ "nm ::= ID|INDEXED",
 /*  41 */ "nm ::= STRING",
 /*  42 */ "nm ::= JOIN_KW",
 /*  43 */ "type ::=",
 /*  44 */ "type ::= typetoken",
 /*  45 */ "typetoken ::= typename",
 /*  46 */ "typetoken ::= typename LP signed RP",
 /*  47 */ "typetoken ::= typename LP signed COMMA signed RP",
 /*  48 */ "typename ::= ID|STRING",
 /*  49 */ "typename ::= typename ID|STRING",
 /*  50 */ "signed ::= plus_num",
 /*  51 */ "signed ::= minus_num",
 /*  52 */ "carglist ::= carglist ccons",
 /*  53 */ "carglist ::=",
 /*  54 */ "ccons ::= CONSTRAINT nm",
 /*  55 */ "ccons ::= DEFAULT term",
 /*  56 */ "ccons ::= DEFAULT LP expr RP",
 /*  57 */ "ccons ::= DEFAULT PLUS term",
 /*  58 */ "ccons ::= DEFAULT MINUS term",
 /*  59 */ "ccons ::= DEFAULT ID|INDEXED",
 /*  60 */ "ccons ::= NULL onconf",
 /*  61 */ "ccons ::= NOT NULL onconf",
 /*  62 */ "ccons ::= PRIMARY KEY sortorder onconf autoinc",
 /*  63 */ "ccons ::= UNIQUE onconf",
 /*  64 */ "ccons ::= CHECK LP expr RP",
 /*  65 */ "ccons ::= REFERENCES nm idxlist_opt refargs",
 /*  66 */ "ccons ::= defer_subclause",
 /*  67 */ "ccons ::= COLLATE ID|STRING",
 /*  68 */ "autoinc ::=",
 /*  69 */ "autoinc ::= AUTOINCR",
 /*  70 */ "refargs ::=",
 /*  71 */ "refargs ::= refargs refarg",
 /*  72 */ "refarg ::= MATCH nm",
 /*  73 */ "refarg ::= ON INSERT refact",
 /*  74 */ "refarg ::= ON DELETE refact",
 /*  75 */ "refarg ::= ON UPDATE refact",
 /*  76 */ "refact ::= SET NULL",
 /*  77 */ "refact ::= SET DEFAULT",
 /*  78 */ "refact ::= CASCADE",
 /*  79 */ "refact ::= RESTRICT",
 /*  80 */ "refact ::= NO ACTION",
 /*  81 */ "defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt",
 /*  82 */ "defer_subclause ::= DEFERRABLE init_deferred_pred_opt",
 /*  83 */ "init_deferred_pred_opt ::=",
 /*  84 */ "init_deferred_pred_opt ::= INITIALLY DEFERRED",
 /*  85 */ "init_deferred_pred_opt ::= INITIALLY IMMEDIATE",
 /*  86 */ "conslist_opt ::=",
 /*  87 */ "conslist_opt ::= COMMA conslist",
 /*  88 */ "conslist ::= conslist tconscomma tcons",
 /*  89 */ "conslist ::= tcons",
 /*  90 */ "tconscomma ::= COMMA",
 /*  91 */ "tconscomma ::=",
 /*  92 */ "tcons ::= CONSTRAINT nm",
 /*  93 */ "tcons ::= PRIMARY KEY LP idxlist autoinc RP onconf",
 /*  94 */ "tcons ::= UNIQUE LP idxlist RP onconf",
 /*  95 */ "tcons ::= CHECK LP expr RP onconf",
 /*  96 */ "tcons ::= FOREIGN KEY LP idxlist RP REFERENCES nm idxlist_opt refargs defer_subclause_opt",
 /*  97 */ "defer_subclause_opt ::=",
 /*  98 */ "defer_subclause_opt ::= defer_subclause",
 /*  99 */ "onconf ::=",
 /* 100 */ "onconf ::= ON CONFLICT resolvetype",
 /* 101 */ "orconf ::=",
 /* 102 */ "orconf ::= OR resolvetype",
 /* 103 */ "resolvetype ::= raisetype",
 /* 104 */ "resolvetype ::= IGNORE",
 /* 105 */ "resolvetype ::= REPLACE",
 /* 106 */ "cmd ::= DROP TABLE ifexists fullname",
 /* 107 */ "ifexists ::= IF EXISTS",
 /* 108 */ "ifexists ::=",
 /* 109 */ "cmd ::= createkw temp VIEW ifnotexists nm dbnm AS select",
 /* 110 */ "cmd ::= DROP VIEW ifexists fullname",
 /* 111 */ "cmd ::= select",
 /* 112 */ "select ::= with selectnowith",
 /* 113 */ "selectnowith ::= oneselect",
 /* 114 */ "selectnowith ::= selectnowith multiselect_op oneselect",
 /* 115 */ "multiselect_op ::= UNION",
 /* 116 */ "multiselect_op ::= UNION ALL",
 /* 117 */ "multiselect_op ::= EXCEPT|INTERSECT",
 /* 118 */ "oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt",
 /* 119 */ "oneselect ::= values",
 /* 120 */ "values ::= VALUES LP nexprlist RP",
 /* 121 */ "values ::= values COMMA LP exprlist RP",
 /* 122 */ "distinct ::= DISTINCT",
 /* 123 */ "distinct ::= ALL",
 /* 124 */ "distinct ::=",
 /* 125 */ "sclp ::= selcollist COMMA",
 /* 126 */ "sclp ::=",
 /* 127 */ "selcollist ::= sclp expr as",
 /* 128 */ "selcollist ::= sclp STAR",
 /* 129 */ "selcollist ::= sclp nm DOT STAR",
 /* 130 */ "as ::= AS nm",
 /* 131 */ "as ::= ID|STRING",
 /* 132 */ "as ::=",
 /* 133 */ "from ::=",
 /* 134 */ "from ::= FROM seltablist",
 /* 135 */ "stl_prefix ::= seltablist joinop",
 /* 136 */ "stl_prefix ::=",
 /* 137 */ "seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt",
 /* 138 */ "seltablist ::= stl_prefix LP select RP as on_opt using_opt",
 /* 139 */ "seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt",
 /* 140 */ "dbnm ::=",
 /* 141 */ "dbnm ::= DOT nm",
 /* 142 */ "fullname ::= nm dbnm",
 /* 143 */ "joinop ::= COMMA|JOIN",
 /* 144 */ "joinop ::= JOIN_KW JOIN",
 /* 145 */ "joinop ::= JOIN_KW nm JOIN",
 /* 146 */ "joinop ::= JOIN_KW nm nm JOIN",
 /* 147 */ "on_opt ::= ON expr",
 /* 148 */ "on_opt ::=",
 /* 149 */ "indexed_opt ::=",
 /* 150 */ "indexed_opt ::= INDEXED BY nm",
 /* 151 */ "indexed_opt ::= NOT INDEXED",
 /* 152 */ "using_opt ::= USING LP idlist RP",
 /* 153 */ "using_opt ::=",
 /* 154 */ "orderby_opt ::=",
 /* 155 */ "orderby_opt ::= ORDER BY sortlist",
 /* 156 */ "sortlist ::= sortlist COMMA expr sortorder",
 /* 157 */ "sortlist ::= expr sortorder",
 /* 158 */ "sortorder ::= ASC",
 /* 159 */ "sortorder ::= DESC",
 /* 160 */ "sortorder ::=",
 /* 161 */ "groupby_opt ::=",
 /* 162 */ "groupby_opt ::= GROUP BY nexprlist",
 /* 163 */ "having_opt ::=",
 /* 164 */ "having_opt ::= HAVING expr",
 /* 165 */ "limit_opt ::=",
 /* 166 */ "limit_opt ::= LIMIT expr",
 /* 167 */ "limit_opt ::= LIMIT expr OFFSET expr",
 /* 168 */ "limit_opt ::= LIMIT expr COMMA expr",
 /* 169 */ "cmd ::= with DELETE FROM fullname indexed_opt where_opt",
 /* 170 */ "where_opt ::=",
 /* 171 */ "where_opt ::= WHERE expr",
 /* 172 */ "cmd ::= with UPDATE orconf fullname indexed_opt SET setlist where_opt",
 /* 173 */ "setlist ::= setlist COMMA nm EQ expr",
 /* 174 */ "setlist ::= nm EQ expr",
 /* 175 */ "cmd ::= with insert_cmd INTO fullname inscollist_opt select",
 /* 176 */ "cmd ::= with insert_cmd INTO fullname inscollist_opt DEFAULT VALUES",
 /* 177 */ "insert_cmd ::= INSERT orconf",
 /* 178 */ "insert_cmd ::= REPLACE",
 /* 179 */ "inscollist_opt ::=",
 /* 180 */ "inscollist_opt ::= LP idlist RP",
 /* 181 */ "idlist ::= idlist COMMA nm",
 /* 182 */ "idlist ::= nm",
 /* 183 */ "expr ::= term",
 /* 184 */ "expr ::= LP expr RP",
 /* 185 */ "term ::= NULL",
 /* 186 */ "expr ::= ID|INDEXED",
 /* 187 */ "expr ::= JOIN_KW",
 /* 188 */ "expr ::= nm DOT nm",
 /* 189 */ "expr ::= nm DOT nm DOT nm",
 /* 190 */ "term ::= INTEGER|FLOAT|BLOB",
 /* 191 */ "term ::= STRING",
 /* 192 */ "expr ::= VARIABLE",
 /* 193 */ "expr ::= expr COLLATE ID|STRING",
 /* 194 */ "expr ::= CAST LP expr AS typetoken RP",
 /* 195 */ "expr ::= ID|INDEXED LP distinct exprlist RP",
 /* 196 */ "expr ::= ID|INDEXED LP STAR RP",
 /* 197 */ "term ::= CTIME_KW",
 /* 198 */ "expr ::= expr AND expr",
 /* 199 */ "expr ::= expr OR expr",
 /* 200 */ "expr ::= expr LT|GT|GE|LE expr",
 /* 201 */ "expr ::= expr EQ|NE expr",
 /* 202 */ "expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr",
 /* 203 */ "expr ::= expr PLUS|MINUS expr",
 /* 204 */ "expr ::= expr STAR|SLASH|REM expr",
 /* 205 */ "expr ::= expr CONCAT expr",
 /* 206 */ "likeop ::= LIKE_KW|MATCH",
 /* 207 */ "likeop ::= NOT LIKE_KW|MATCH",
 /* 208 */ "expr ::= expr likeop expr",
 /* 209 */ "expr ::= expr likeop expr ESCAPE expr",
 /* 210 */ "expr ::= expr ISNULL|NOTNULL",
 /* 211 */ "expr ::= expr NOT NULL",
 /* 212 */ "expr ::= expr IS expr",
 /* 213 */ "expr ::= expr IS NOT expr",
 /* 214 */ "expr ::= NOT expr",
 /* 215 */ "expr ::= BITNOT expr",
 /* 216 */ "expr ::= MINUS expr",
 /* 217 */ "expr ::= PLUS expr",
 /* 218 */ "between_op ::= BETWEEN",
 /* 219 */ "between_op ::= NOT BETWEEN",
 /* 220 */ "expr ::= expr between_op expr AND expr",
 /* 221 */ "in_op ::= IN",
 /* 222 */ "in_op ::= NOT IN",
 /* 223 */ "expr ::= expr in_op LP exprlist RP",
 /* 224 */ "expr ::= LP select RP",
 /* 225 */ "expr ::= expr in_op LP select RP",
 /* 226 */ "expr ::= expr in_op nm dbnm",
 /* 227 */ "expr ::= EXISTS LP select RP",
 /* 228 */ "expr ::= CASE case_operand case_exprlist case_else END",
 /* 229 */ "case_exprlist ::= case_exprlist WHEN expr THEN expr",
 /* 230 */ "case_exprlist ::= WHEN expr THEN expr",
 /* 231 */ "case_else ::= ELSE expr",
 /* 232 */ "case_else ::=",
 /* 233 */ "case_operand ::= expr",
 /* 234 */ "case_operand ::=",
 /* 235 */ "exprlist ::= nexprlist",
 /* 236 */ "exprlist ::=",
 /* 237 */ "nexprlist ::= nexprlist COMMA expr",
 /* 238 */ "nexprlist ::= expr",
 /* 239 */ "cmd ::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP idxlist RP where_opt",
 /* 240 */ "uniqueflag ::= UNIQUE",
 /* 241 */ "uniqueflag ::=",
 /* 242 */ "idxlist_opt ::=",
 /* 243 */ "idxlist_opt ::= LP idxlist RP",
 /* 244 */ "idxlist ::= idxlist COMMA nm collate sortorder",
 /* 245 */ "idxlist ::= nm collate sortorder",
 /* 246 */ "collate ::=",
 /* 247 */ "collate ::= COLLATE ID|STRING",
 /* 248 */ "cmd ::= DROP INDEX ifexists fullname",
 /* 249 */ "cmd ::= VACUUM",
 /* 250 */ "cmd ::= VACUUM nm",
 /* 251 */ "cmd ::= PRAGMA nm dbnm",
 /* 252 */ "cmd ::= PRAGMA nm dbnm EQ nmnum",
 /* 253 */ "cmd ::= PRAGMA nm dbnm LP nmnum RP",
 /* 254 */ "cmd ::= PRAGMA nm dbnm EQ minus_num",
 /* 255 */ "cmd ::= PRAGMA nm dbnm LP minus_num RP",
 /* 256 */ "nmnum ::= plus_num",
 /* 257 */ "nmnum ::= nm",
 /* 258 */ "nmnum ::= ON",
 /* 259 */ "nmnum ::= DELETE",
 /* 260 */ "nmnum ::= DEFAULT",
 /* 261 */ "plus_num ::= PLUS INTEGER|FLOAT",
 /* 262 */ "plus_num ::= INTEGER|FLOAT",
 /* 263 */ "minus_num ::= MINUS INTEGER|FLOAT",
 /* 264 */ "cmd ::= createkw trigger_decl BEGIN trigger_cmd_list END",
 /* 265 */ "trigger_decl ::= temp TRIGGER ifnotexists nm dbnm trigger_time trigger_event ON fullname foreach_clause when_clause",
 /* 266 */ "trigger_time ::= BEFORE",
 /* 267 */ "trigger_time ::= AFTER",
 /* 268 */ "trigger_time ::= INSTEAD OF",
 /* 269 */ "trigger_time ::=",
 /* 270 */ "trigger_event ::= DELETE|INSERT",
 /* 271 */ "trigger_event ::= UPDATE",
 /* 272 */ "trigger_event ::= UPDATE OF idlist",
 /* 273 */ "foreach_clause ::=",
 /* 274 */ "foreach_clause ::= FOR EACH ROW",
 /* 275 */ "when_clause ::=",
 /* 276 */ "when_clause ::= WHEN expr",
 /* 277 */ "trigger_cmd_list ::= trigger_cmd_list trigger_cmd SEMI",
 /* 278 */ "trigger_cmd_list ::= trigger_cmd SEMI",
 /* 279 */ "trnm ::= nm",
 /* 280 */ "trnm ::= nm DOT nm",
 /* 281 */ "tridxby ::=",
 /* 282 */ "tridxby ::= INDEXED BY nm",
 /* 283 */ "tridxby ::= NOT INDEXED",
 /* 284 */ "trigger_cmd ::= UPDATE orconf trnm tridxby SET setlist where_opt",
 /* 285 */ "trigger_cmd ::= insert_cmd INTO trnm inscollist_opt select",
 /* 286 */ "trigger_cmd ::= DELETE FROM trnm tridxby where_opt",
 /* 287 */ "trigger_cmd ::= select",
 /* 288 */ "expr ::= RAISE LP IGNORE RP",
 /* 289 */ "expr ::= RAISE LP raisetype COMMA nm RP",
 /* 290 */ "raisetype ::= ROLLBACK",
 /* 291 */ "raisetype ::= ABORT",
 /* 292 */ "raisetype ::= FAIL",
 /* 293 */ "cmd ::= DROP TRIGGER ifexists fullname",
 /* 294 */ "cmd ::= ATTACH database_kw_opt expr AS expr key_opt",
 /* 295 */ "cmd ::= DETACH database_kw_opt expr",
 /* 296 */ "key_opt ::=",
 /* 297 */ "key_opt ::= KEY expr",
 /* 298 */ "database_kw_opt ::= DATABASE",
 /* 299 */ "database_kw_opt ::=",
 /* 300 */ "cmd ::= REINDEX",
 /* 301 */ "cmd ::= REINDEX nm dbnm",
 /* 302 */ "cmd ::= ANALYZE",
 /* 303 */ "cmd ::= ANALYZE nm dbnm",
 /* 304 */ "cmd ::= ALTER TABLE fullname RENAME TO nm",
 /* 305 */ "cmd ::= ALTER TABLE add_column_fullname ADD kwcolumn_opt column",
 /* 306 */ "add_column_fullname ::= fullname",
 /* 307 */ "kwcolumn_opt ::=",
 /* 308 */ "kwcolumn_opt ::= COLUMNKW",
 /* 309 */ "cmd ::= create_vtab",
 /* 310 */ "cmd ::= create_vtab LP vtabarglist RP",
 /* 311 */ "create_vtab ::= createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm",
 /* 312 */ "vtabarglist ::= vtabarg",
 /* 313 */ "vtabarglist ::= vtabarglist COMMA vtabarg",
 /* 314 */ "vtabarg ::=",
 /* 315 */ "vtabarg ::= vtabarg vtabargtoken",
 /* 316 */ "vtabargtoken ::= ANY",
 /* 317 */ "vtabargtoken ::= lp anylist RP",
 /* 318 */ "lp ::= LP",
 /* 319 */ "anylist ::=",
 /* 320 */ "anylist ::= anylist LP anylist RP",
 /* 321 */ "anylist ::= anylist ANY",
 /* 322 */ "with ::=",
 /* 323 */ "with ::= WITH wqlist",
 /* 324 */ "with ::= WITH RECURSIVE wqlist",
 /* 325 */ "wqlist ::= nm idxlist_opt AS LP select RP",
 /* 326 */ "wqlist ::= wqlist COMMA nm idxlist_opt AS LP select RP",
};
#endif /* NDEBUG */


#if YYSTACKDEPTH<=0
/*
** Try to increase the size of the parser stack.
*/
static void yyGrowStack(yyParser *p){
  int newSize;
  yyStackEntry *pNew;

  newSize = p->yystksz*2 + 100;
  pNew = realloc(p->yystack, newSize*sizeof(pNew[0]));
  if( pNew ){
    p->yystack = pNew;
    p->yystksz = newSize;
#ifndef NDEBUG
    if( yyTraceFILE ){
      fprintf(yyTraceFILE,"%sStack grows to %d entries!\n",
              yyTracePrompt, p->yystksz);
    }
#endif
  }
}
#endif

/* 
** This function allocates a new parser.
** The only argument is a pointer to a function which works like
** malloc.
**
** Inputs:
** A pointer to the function used to allocate memory.
**
** Outputs:
** A pointer to a parser.  This pointer is used in subsequent calls
** to sqlite3Parser and sqlite3ParserFree.
*/
SQLITE_PRIVATE void *sqlite3ParserAlloc(void *(*mallocProc)(u64)){
  yyParser *pParser;
  pParser = (yyParser*)(*mallocProc)( (u64)sizeof(yyParser) );
  if( pParser ){
    pParser->yyidx = -1;
#ifdef YYTRACKMAXSTACKDEPTH
    pParser->yyidxMax = 0;
#endif
#if YYSTACKDEPTH<=0
    pParser->yystack = NULL;
    pParser->yystksz = 0;
    yyGrowStack(pParser);
#endif
  }
  return pParser;
}

/* The following function deletes the value associated with a
** symbol.  The symbol can be either a terminal or nonterminal.
** "yymajor" is the symbol code, and "yypminor" is a pointer to
** the value.
*/
static void yy_destructor(
  yyParser *yypParser,    /* The parser */
  YYCODETYPE yymajor,     /* Type code for object to destroy */
  YYMINORTYPE *yypminor   /* The object to be destroyed */
){
  sqlite3ParserARG_FETCH;
  switch( yymajor ){
    /* Here is inserted the actions which take place when a
    ** terminal or non-terminal is destroyed.  This can happen
    ** when the symbol is popped from the stack during a
    ** reduce or during error processing or when a parser is 
    ** being destroyed before it is finished parsing.
    **
    ** Note: during a reduce, the only symbols destroyed are those
    ** which appear on the RHS of the rule, but which are not used
    ** inside the C code.
    */
    case 163: /* select */
    case 195: /* selectnowith */
    case 196: /* oneselect */
    case 207: /* values */
{
sqlite3SelectDelete(pParse->db, (yypminor->yy3));
}
      break;
    case 174: /* term */
    case 175: /* expr */
{
sqlite3ExprDelete(pParse->db, (yypminor->yy346).pExpr);
}
      break;
    case 179: /* idxlist_opt */
    case 188: /* idxlist */
    case 200: /* selcollist */
    case 203: /* groupby_opt */
    case 205: /* orderby_opt */
    case 208: /* nexprlist */
    case 209: /* exprlist */
    case 210: /* sclp */
    case 220: /* sortlist */
    case 221: /* setlist */
    case 228: /* case_exprlist */
{
sqlite3ExprListDelete(pParse->db, (yypminor->yy14));
}
      break;
    case 194: /* fullname */
    case 201: /* from */
    case 212: /* seltablist */
    case 213: /* stl_prefix */
{
sqlite3SrcListDelete(pParse->db, (yypminor->yy65));
}
      break;
    case 197: /* with */
    case 252: /* wqlist */
{
sqlite3WithDelete(pParse->db, (yypminor->yy59));
}
      break;
    case 202: /* where_opt */
    case 204: /* having_opt */
    case 216: /* on_opt */
    case 227: /* case_operand */
    case 229: /* case_else */
    case 238: /* when_clause */
    case 243: /* key_opt */
{
sqlite3ExprDelete(pParse->db, (yypminor->yy132));
}
      break;
    case 217: /* using_opt */
    case 219: /* idlist */
    case 223: /* inscollist_opt */
{
sqlite3IdListDelete(pParse->db, (yypminor->yy408));
}
      break;
    case 234: /* trigger_cmd_list */
    case 239: /* trigger_cmd */
{
sqlite3DeleteTriggerStep(pParse->db, (yypminor->yy473));
}
      break;
    case 236: /* trigger_event */
{
sqlite3IdListDelete(pParse->db, (yypminor->yy378).b);
}
      break;
    default:  break;   /* If no destructor action specified: do nothing */
  }
}

/*
** Pop the parser's stack once.
**
** If there is a destructor routine associated with the token which
** is popped from the stack, then call it.
**
** Return the major token number for the symbol popped.
*/
static int yy_pop_parser_stack(yyParser *pParser){
  YYCODETYPE yymajor;
  yyStackEntry *yytos = &pParser->yystack[pParser->yyidx];

  /* There is no mechanism by which the parser stack can be popped below
  ** empty in SQLite.  */
  assert( pParser->yyidx>=0 );
#ifndef NDEBUG
  if( yyTraceFILE && pParser->yyidx>=0 ){
    fprintf(yyTraceFILE,"%sPopping %s\n",
      yyTracePrompt,
      yyTokenName[yytos->major]);
  }
#endif
  yymajor = yytos->major;
  yy_destructor(pParser, yymajor, &yytos->minor);
  pParser->yyidx--;
  return yymajor;
}

/* 
** Deallocate and destroy a parser.  Destructors are all called for
** all stack elements before shutting the parser down.
**
** Inputs:
** <ul>
** <li>  A pointer to the parser.  This should be a pointer
**       obtained from sqlite3ParserAlloc.
** <li>  A pointer to a function used to reclaim memory obtained
**       from malloc.
** </ul>
*/
SQLITE_PRIVATE void sqlite3ParserFree(
  void *p,                    /* The parser to be deleted */
  void (*freeProc)(void*)     /* Function used to reclaim memory */
){
  yyParser *pParser = (yyParser*)p;
  /* In SQLite, we never try to destroy a parser that was not successfully
  ** created in the first place. */
  if( NEVER(pParser==0) ) return;
  while( pParser->yyidx>=0 ) yy_pop_parser_stack(pParser);
#if YYSTACKDEPTH<=0
  free(pParser->yystack);
#endif
  (*freeProc)((void*)pParser);
}

/*
** Return the peak depth of the stack for a parser.
*/
#ifdef YYTRACKMAXSTACKDEPTH
SQLITE_PRIVATE int sqlite3ParserStackPeak(void *p){
  yyParser *pParser = (yyParser*)p;
  return pParser->yyidxMax;
}
#endif

/*
** Find the appropriate action for a parser given the terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_shift_action(
  yyParser *pParser,        /* The parser */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
  int stateno = pParser->yystack[pParser->yyidx].stateno;
 
  if( stateno>YY_SHIFT_COUNT
   || (i = yy_shift_ofst[stateno])==YY_SHIFT_USE_DFLT ){
    return yy_default[stateno];
  }
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    if( iLookAhead>0 ){
#ifdef YYFALLBACK
      YYCODETYPE iFallback;            /* Fallback token */
      if( iLookAhead<sizeof(yyFallback)/sizeof(yyFallback[0])
             && (iFallback = yyFallback[iLookAhead])!=0 ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE, "%sFALLBACK %s => %s\n",
             yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[iFallback]);
        }
#endif
        return yy_find_shift_action(pParser, iFallback);
      }
#endif
#ifdef YYWILDCARD
      {
        int j = i - iLookAhead + YYWILDCARD;
        if( 
#if YY_SHIFT_MIN+YYWILDCARD<0
          j>=0 &&
#endif
#if YY_SHIFT_MAX+YYWILDCARD>=YY_ACTTAB_COUNT
          j<YY_ACTTAB_COUNT &&
#endif
          yy_lookahead[j]==YYWILDCARD
        ){
#ifndef NDEBUG
          if( yyTraceFILE ){
            fprintf(yyTraceFILE, "%sWILDCARD %s => %s\n",
               yyTracePrompt, yyTokenName[iLookAhead], yyTokenName[YYWILDCARD]);
          }
#endif /* NDEBUG */
          return yy_action[j];
        }
      }
#endif /* YYWILDCARD */
    }
    return yy_default[stateno];
  }else{
    return yy_action[i];
  }
}

/*
** Find the appropriate action for a parser given the non-terminal
** look-ahead token iLookAhead.
**
** If the look-ahead token is YYNOCODE, then check to see if the action is
** independent of the look-ahead.  If it is, return the action, otherwise
** return YY_NO_ACTION.
*/
static int yy_find_reduce_action(
  int stateno,              /* Current state number */
  YYCODETYPE iLookAhead     /* The look-ahead token */
){
  int i;
#ifdef YYERRORSYMBOL
  if( stateno>YY_REDUCE_COUNT ){
    return yy_default[stateno];
  }
#else
  assert( stateno<=YY_REDUCE_COUNT );
#endif
  i = yy_reduce_ofst[stateno];
  assert( i!=YY_REDUCE_USE_DFLT );
  assert( iLookAhead!=YYNOCODE );
  i += iLookAhead;
#ifdef YYERRORSYMBOL
  if( i<0 || i>=YY_ACTTAB_COUNT || yy_lookahead[i]!=iLookAhead ){
    return yy_default[stateno];
  }
#else
  assert( i>=0 && i<YY_ACTTAB_COUNT );
  assert( yy_lookahead[i]==iLookAhead );
#endif
  return yy_action[i];
}

/*
** The following routine is called if the stack overflows.
*/
static void yyStackOverflow(yyParser *yypParser, YYMINORTYPE *yypMinor){
   sqlite3ParserARG_FETCH;
   yypParser->yyidx--;
#ifndef NDEBUG
   if( yyTraceFILE ){
     fprintf(yyTraceFILE,"%sStack Overflow!\n",yyTracePrompt);
   }
#endif
   while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
   /* Here code is inserted which will execute if the parser
   ** stack every overflows */

  UNUSED_PARAMETER(yypMinor); /* Silence some compiler warnings */
  sqlite3ErrorMsg(pParse, "parser stack overflow");
   sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument var */
}

/*
** Perform a shift action.
*/
static void yy_shift(
  yyParser *yypParser,          /* The parser to be shifted */
  int yyNewState,               /* The new state to shift in */
  int yyMajor,                  /* The major token to shift in */
  YYMINORTYPE *yypMinor         /* Pointer to the minor token to shift in */
){
  yyStackEntry *yytos;
  yypParser->yyidx++;
#ifdef YYTRACKMAXSTACKDEPTH
  if( yypParser->yyidx>yypParser->yyidxMax ){
    yypParser->yyidxMax = yypParser->yyidx;
  }
#endif
#if YYSTACKDEPTH>0 
  if( yypParser->yyidx>=YYSTACKDEPTH ){
    yyStackOverflow(yypParser, yypMinor);
    return;
  }
#else
  if( yypParser->yyidx>=yypParser->yystksz ){
    yyGrowStack(yypParser);
    if( yypParser->yyidx>=yypParser->yystksz ){
      yyStackOverflow(yypParser, yypMinor);
      return;
    }
  }
#endif
  yytos = &yypParser->yystack[yypParser->yyidx];
  yytos->stateno = (YYACTIONTYPE)yyNewState;
  yytos->major = (YYCODETYPE)yyMajor;
  yytos->minor = *yypMinor;
#ifndef NDEBUG
  if( yyTraceFILE && yypParser->yyidx>0 ){
    int i;
    fprintf(yyTraceFILE,"%sShift %d\n",yyTracePrompt,yyNewState);
    fprintf(yyTraceFILE,"%sStack:",yyTracePrompt);
    for(i=1; i<=yypParser->yyidx; i++)
      fprintf(yyTraceFILE," %s",yyTokenName[yypParser->yystack[i].major]);
    fprintf(yyTraceFILE,"\n");
  }
#endif
}

/* The following table contains information about every rule that
** is used during the reduce.
*/
static const struct {
  YYCODETYPE lhs;         /* Symbol on the left-hand side of the rule */
  unsigned char nrhs;     /* Number of right-hand side symbols in the rule */
} yyRuleInfo[] = {
  { 144, 1 },
  { 145, 2 },
  { 145, 1 },
  { 146, 1 },
  { 146, 3 },
  { 147, 0 },
  { 147, 1 },
  { 147, 3 },
  { 148, 1 },
  { 149, 3 },
  { 151, 0 },
  { 151, 1 },
  { 151, 2 },
  { 150, 0 },
  { 150, 1 },
  { 150, 1 },
  { 150, 1 },
  { 149, 2 },
  { 149, 2 },
  { 149, 2 },
  { 153, 1 },
  { 153, 0 },
  { 149, 2 },
  { 149, 3 },
  { 149, 5 },
  { 149, 2 },
  { 154, 6 },
  { 156, 1 },
  { 158, 0 },
  { 158, 3 },
  { 157, 1 },
  { 157, 0 },
  { 155, 5 },
  { 155, 2 },
  { 162, 0 },
  { 162, 2 },
  { 160, 3 },
  { 160, 1 },
  { 164, 3 },
  { 165, 1 },
  { 152, 1 },
  { 152, 1 },
  { 152, 1 },
  { 166, 0 },
  { 166, 1 },
  { 168, 1 },
  { 168, 4 },
  { 168, 6 },
  { 169, 1 },
  { 169, 2 },
  { 170, 1 },
  { 170, 1 },
  { 167, 2 },
  { 167, 0 },
  { 173, 2 },
  { 173, 2 },
  { 173, 4 },
  { 173, 3 },
  { 173, 3 },
  { 173, 2 },
  { 173, 2 },
  { 173, 3 },
  { 173, 5 },
  { 173, 2 },
  { 173, 4 },
  { 173, 4 },
  { 173, 1 },
  { 173, 2 },
  { 178, 0 },
  { 178, 1 },
  { 180, 0 },
  { 180, 2 },
  { 182, 2 },
  { 182, 3 },
  { 182, 3 },
  { 182, 3 },
  { 183, 2 },
  { 183, 2 },
  { 183, 1 },
  { 183, 1 },
  { 183, 2 },
  { 181, 3 },
  { 181, 2 },
  { 184, 0 },
  { 184, 2 },
  { 184, 2 },
  { 161, 0 },
  { 161, 2 },
  { 185, 3 },
  { 185, 1 },
  { 186, 1 },
  { 186, 0 },
  { 187, 2 },
  { 187, 7 },
  { 187, 5 },
  { 187, 5 },
  { 187, 10 },
  { 189, 0 },
  { 189, 1 },
  { 176, 0 },
  { 176, 3 },
  { 190, 0 },
  { 190, 2 },
  { 191, 1 },
  { 191, 1 },
  { 191, 1 },
  { 149, 4 },
  { 193, 2 },
  { 193, 0 },
  { 149, 8 },
  { 149, 4 },
  { 149, 1 },
  { 163, 2 },
  { 195, 1 },
  { 195, 3 },
  { 198, 1 },
  { 198, 2 },
  { 198, 1 },
  { 196, 9 },
  { 196, 1 },
  { 207, 4 },
  { 207, 5 },
  { 199, 1 },
  { 199, 1 },
  { 199, 0 },
  { 210, 2 },
  { 210, 0 },
  { 200, 3 },
  { 200, 2 },
  { 200, 4 },
  { 211, 2 },
  { 211, 1 },
  { 211, 0 },
  { 201, 0 },
  { 201, 2 },
  { 213, 2 },
  { 213, 0 },
  { 212, 7 },
  { 212, 7 },
  { 212, 7 },
  { 159, 0 },
  { 159, 2 },
  { 194, 2 },
  { 214, 1 },
  { 214, 2 },
  { 214, 3 },
  { 214, 4 },
  { 216, 2 },
  { 216, 0 },
  { 215, 0 },
  { 215, 3 },
  { 215, 2 },
  { 217, 4 },
  { 217, 0 },
  { 205, 0 },
  { 205, 3 },
  { 220, 4 },
  { 220, 2 },
  { 177, 1 },
  { 177, 1 },
  { 177, 0 },
  { 203, 0 },
  { 203, 3 },
  { 204, 0 },
  { 204, 2 },
  { 206, 0 },
  { 206, 2 },
  { 206, 4 },
  { 206, 4 },
  { 149, 6 },
  { 202, 0 },
  { 202, 2 },
  { 149, 8 },
  { 221, 5 },
  { 221, 3 },
  { 149, 6 },
  { 149, 7 },
  { 222, 2 },
  { 222, 1 },
  { 223, 0 },
  { 223, 3 },
  { 219, 3 },
  { 219, 1 },
  { 175, 1 },
  { 175, 3 },
  { 174, 1 },
  { 175, 1 },
  { 175, 1 },
  { 175, 3 },
  { 175, 5 },
  { 174, 1 },
  { 174, 1 },
  { 175, 1 },
  { 175, 3 },
  { 175, 6 },
  { 175, 5 },
  { 175, 4 },
  { 174, 1 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 175, 3 },
  { 224, 1 },
  { 224, 2 },
  { 175, 3 },
  { 175, 5 },
  { 175, 2 },
  { 175, 3 },
  { 175, 3 },
  { 175, 4 },
  { 175, 2 },
  { 175, 2 },
  { 175, 2 },
  { 175, 2 },
  { 225, 1 },
  { 225, 2 },
  { 175, 5 },
  { 226, 1 },
  { 226, 2 },
  { 175, 5 },
  { 175, 3 },
  { 175, 5 },
  { 175, 4 },
  { 175, 4 },
  { 175, 5 },
  { 228, 5 },
  { 228, 4 },
  { 229, 2 },
  { 229, 0 },
  { 227, 1 },
  { 227, 0 },
  { 209, 1 },
  { 209, 0 },
  { 208, 3 },
  { 208, 1 },
  { 149, 12 },
  { 230, 1 },
  { 230, 0 },
  { 179, 0 },
  { 179, 3 },
  { 188, 5 },
  { 188, 3 },
  { 231, 0 },
  { 231, 2 },
  { 149, 4 },
  { 149, 1 },
  { 149, 2 },
  { 149, 3 },
  { 149, 5 },
  { 149, 6 },
  { 149, 5 },
  { 149, 6 },
  { 232, 1 },
  { 232, 1 },
  { 232, 1 },
  { 232, 1 },
  { 232, 1 },
  { 171, 2 },
  { 171, 1 },
  { 172, 2 },
  { 149, 5 },
  { 233, 11 },
  { 235, 1 },
  { 235, 1 },
  { 235, 2 },
  { 235, 0 },
  { 236, 1 },
  { 236, 1 },
  { 236, 3 },
  { 237, 0 },
  { 237, 3 },
  { 238, 0 },
  { 238, 2 },
  { 234, 3 },
  { 234, 2 },
  { 240, 1 },
  { 240, 3 },
  { 241, 0 },
  { 241, 3 },
  { 241, 2 },
  { 239, 7 },
  { 239, 5 },
  { 239, 5 },
  { 239, 1 },
  { 175, 4 },
  { 175, 6 },
  { 192, 1 },
  { 192, 1 },
  { 192, 1 },
  { 149, 4 },
  { 149, 6 },
  { 149, 3 },
  { 243, 0 },
  { 243, 2 },
  { 242, 1 },
  { 242, 0 },
  { 149, 1 },
  { 149, 3 },
  { 149, 1 },
  { 149, 3 },
  { 149, 6 },
  { 149, 6 },
  { 244, 1 },
  { 245, 0 },
  { 245, 1 },
  { 149, 1 },
  { 149, 4 },
  { 246, 8 },
  { 247, 1 },
  { 247, 3 },
  { 248, 0 },
  { 248, 2 },
  { 249, 1 },
  { 249, 3 },
  { 250, 1 },
  { 251, 0 },
  { 251, 4 },
  { 251, 2 },
  { 197, 0 },
  { 197, 2 },
  { 197, 3 },
  { 252, 6 },
  { 252, 8 },
};

static void yy_accept(yyParser*);  /* Forward Declaration */

/*
** Perform a reduce action and the shift that must immediately
** follow the reduce.
*/
static void yy_reduce(
  yyParser *yypParser,         /* The parser */
  int yyruleno                 /* Number of the rule by which to reduce */
){
  int yygoto;                     /* The next state */
  int yyact;                      /* The next action */
  YYMINORTYPE yygotominor;        /* The LHS of the rule reduced */
  yyStackEntry *yymsp;            /* The top of the parser's stack */
  int yysize;                     /* Amount to pop the stack */
  sqlite3ParserARG_FETCH;
  yymsp = &yypParser->yystack[yypParser->yyidx];
#ifndef NDEBUG
  if( yyTraceFILE && yyruleno>=0 
        && yyruleno<(int)(sizeof(yyRuleName)/sizeof(yyRuleName[0])) ){
    fprintf(yyTraceFILE, "%sReduce [%s].\n", yyTracePrompt,
      yyRuleName[yyruleno]);
  }
#endif /* NDEBUG */

  /* Silence complaints from purify about yygotominor being uninitialized
  ** in some cases when it is copied into the stack after the following
  ** switch.  yygotominor is uninitialized when a rule reduces that does
  ** not set the value of its left-hand side nonterminal.  Leaving the
  ** value of the nonterminal uninitialized is utterly harmless as long
  ** as the value is never used.  So really the only thing this code
  ** accomplishes is to quieten purify.  
  **
  ** 2007-01-16:  The wireshark project (www.wireshark.org) reports that
  ** without this code, their parser segfaults.  I'm not sure what there
  ** parser is doing to make this happen.  This is the second bug report
  ** from wireshark this week.  Clearly they are stressing Lemon in ways
  ** that it has not been previously stressed...  (SQLite ticket #2172)
  */
  /*memset(&yygotominor, 0, sizeof(yygotominor));*/
  yygotominor = yyzerominor;


  switch( yyruleno ){
  /* Beginning here are the reduction cases.  A typical example
  ** follows:
  **   case 0:
  **  #line <lineno> <grammarfile>
  **     { ... }           // User supplied code
  **  #line <lineno> <thisfile>
  **     break;
  */
      case 5: /* explain ::= */
{ sqlite3BeginParse(pParse, 0); }
        break;
      case 6: /* explain ::= EXPLAIN */
{ sqlite3BeginParse(pParse, 1); }
        break;
      case 7: /* explain ::= EXPLAIN QUERY PLAN */
{ sqlite3BeginParse(pParse, 2); }
        break;
      case 8: /* cmdx ::= cmd */
{ sqlite3FinishCoding(pParse); }
        break;
      case 9: /* cmd ::= BEGIN transtype trans_opt */
{sqlite3BeginTransaction(pParse, yymsp[-1].minor.yy328);}
        break;
      case 13: /* transtype ::= */
{yygotominor.yy328 = TK_DEFERRED;}
        break;
      case 14: /* transtype ::= DEFERRED */
      case 15: /* transtype ::= IMMEDIATE */ yytestcase(yyruleno==15);
      case 16: /* transtype ::= EXCLUSIVE */ yytestcase(yyruleno==16);
      case 115: /* multiselect_op ::= UNION */ yytestcase(yyruleno==115);
      case 117: /* multiselect_op ::= EXCEPT|INTERSECT */ yytestcase(yyruleno==117);
{yygotominor.yy328 = yymsp[0].major;}
        break;
      case 17: /* cmd ::= COMMIT trans_opt */
      case 18: /* cmd ::= END trans_opt */ yytestcase(yyruleno==18);
{sqlite3CommitTransaction(pParse);}
        break;
      case 19: /* cmd ::= ROLLBACK trans_opt */
{sqlite3RollbackTransaction(pParse);}
        break;
      case 22: /* cmd ::= SAVEPOINT nm */
{
  sqlite3Savepoint(pParse, SAVEPOINT_BEGIN, &yymsp[0].minor.yy0);
}
        break;
      case 23: /* cmd ::= RELEASE savepoint_opt nm */
{
  sqlite3Savepoint(pParse, SAVEPOINT_RELEASE, &yymsp[0].minor.yy0);
}
        break;
      case 24: /* cmd ::= ROLLBACK trans_opt TO savepoint_opt nm */
{
  sqlite3Savepoint(pParse, SAVEPOINT_ROLLBACK, &yymsp[0].minor.yy0);
}
        break;
      case 26: /* create_table ::= createkw temp TABLE ifnotexists nm dbnm */
{
   sqlite3StartTable(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0,yymsp[-4].minor.yy328,0,0,yymsp[-2].minor.yy328);
}
        break;
      case 27: /* createkw ::= CREATE */
{
  pParse->db->lookaside.bEnabled = 0;
  yygotominor.yy0 = yymsp[0].minor.yy0;
}
        break;
      case 28: /* ifnotexists ::= */
      case 31: /* temp ::= */ yytestcase(yyruleno==31);
      case 68: /* autoinc ::= */ yytestcase(yyruleno==68);
      case 81: /* defer_subclause ::= NOT DEFERRABLE init_deferred_pred_opt */ yytestcase(yyruleno==81);
      case 83: /* init_deferred_pred_opt ::= */ yytestcase(yyruleno==83);
      case 85: /* init_deferred_pred_opt ::= INITIALLY IMMEDIATE */ yytestcase(yyruleno==85);
      case 97: /* defer_subclause_opt ::= */ yytestcase(yyruleno==97);
      case 108: /* ifexists ::= */ yytestcase(yyruleno==108);
      case 218: /* between_op ::= BETWEEN */ yytestcase(yyruleno==218);
      case 221: /* in_op ::= IN */ yytestcase(yyruleno==221);
{yygotominor.yy328 = 0;}
        break;
      case 29: /* ifnotexists ::= IF NOT EXISTS */
      case 30: /* temp ::= TEMP */ yytestcase(yyruleno==30);
      case 69: /* autoinc ::= AUTOINCR */ yytestcase(yyruleno==69);
      case 84: /* init_deferred_pred_opt ::= INITIALLY DEFERRED */ yytestcase(yyruleno==84);
      case 107: /* ifexists ::= IF EXISTS */ yytestcase(yyruleno==107);
      case 219: /* between_op ::= NOT BETWEEN */ yytestcase(yyruleno==219);
      case 222: /* in_op ::= NOT IN */ yytestcase(yyruleno==222);
{yygotominor.yy328 = 1;}
        break;
      case 32: /* create_table_args ::= LP columnlist conslist_opt RP table_options */
{
  sqlite3EndTable(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,yymsp[0].minor.yy186,0);
}
        break;
      case 33: /* create_table_args ::= AS select */
{
  sqlite3EndTable(pParse,0,0,0,yymsp[0].minor.yy3);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy3);
}
        break;
      case 34: /* table_options ::= */
{yygotominor.yy186 = 0;}
        break;
      case 35: /* table_options ::= WITHOUT nm */
{
  if( yymsp[0].minor.yy0.n==5 && sqlite3_strnicmp(yymsp[0].minor.yy0.z,"rowid",5)==0 ){
    yygotominor.yy186 = TF_WithoutRowid | TF_NoVisibleRowid;
  }else{
    yygotominor.yy186 = 0;
    sqlite3ErrorMsg(pParse, "unknown table option: %.*s", yymsp[0].minor.yy0.n, yymsp[0].minor.yy0.z);
  }
}
        break;
      case 38: /* column ::= columnid type carglist */
{
  yygotominor.yy0.z = yymsp[-2].minor.yy0.z;
  yygotominor.yy0.n = (int)(pParse->sLastToken.z-yymsp[-2].minor.yy0.z) + pParse->sLastToken.n;
}
        break;
      case 39: /* columnid ::= nm */
{
  sqlite3AddColumn(pParse,&yymsp[0].minor.yy0);
  yygotominor.yy0 = yymsp[0].minor.yy0;
  pParse->constraintName.n = 0;
}
        break;
      case 40: /* nm ::= ID|INDEXED */
      case 41: /* nm ::= STRING */ yytestcase(yyruleno==41);
      case 42: /* nm ::= JOIN_KW */ yytestcase(yyruleno==42);
      case 45: /* typetoken ::= typename */ yytestcase(yyruleno==45);
      case 48: /* typename ::= ID|STRING */ yytestcase(yyruleno==48);
      case 130: /* as ::= AS nm */ yytestcase(yyruleno==130);
      case 131: /* as ::= ID|STRING */ yytestcase(yyruleno==131);
      case 141: /* dbnm ::= DOT nm */ yytestcase(yyruleno==141);
      case 150: /* indexed_opt ::= INDEXED BY nm */ yytestcase(yyruleno==150);
      case 247: /* collate ::= COLLATE ID|STRING */ yytestcase(yyruleno==247);
      case 256: /* nmnum ::= plus_num */ yytestcase(yyruleno==256);
      case 257: /* nmnum ::= nm */ yytestcase(yyruleno==257);
      case 258: /* nmnum ::= ON */ yytestcase(yyruleno==258);
      case 259: /* nmnum ::= DELETE */ yytestcase(yyruleno==259);
      case 260: /* nmnum ::= DEFAULT */ yytestcase(yyruleno==260);
      case 261: /* plus_num ::= PLUS INTEGER|FLOAT */ yytestcase(yyruleno==261);
      case 262: /* plus_num ::= INTEGER|FLOAT */ yytestcase(yyruleno==262);
      case 263: /* minus_num ::= MINUS INTEGER|FLOAT */ yytestcase(yyruleno==263);
      case 279: /* trnm ::= nm */ yytestcase(yyruleno==279);
{yygotominor.yy0 = yymsp[0].minor.yy0;}
        break;
      case 44: /* type ::= typetoken */
{sqlite3AddColumnType(pParse,&yymsp[0].minor.yy0);}
        break;
      case 46: /* typetoken ::= typename LP signed RP */
{
  yygotominor.yy0.z = yymsp[-3].minor.yy0.z;
  yygotominor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-3].minor.yy0.z);
}
        break;
      case 47: /* typetoken ::= typename LP signed COMMA signed RP */
{
  yygotominor.yy0.z = yymsp[-5].minor.yy0.z;
  yygotominor.yy0.n = (int)(&yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] - yymsp[-5].minor.yy0.z);
}
        break;
      case 49: /* typename ::= typename ID|STRING */
{yygotominor.yy0.z=yymsp[-1].minor.yy0.z; yygotominor.yy0.n=yymsp[0].minor.yy0.n+(int)(yymsp[0].minor.yy0.z-yymsp[-1].minor.yy0.z);}
        break;
      case 54: /* ccons ::= CONSTRAINT nm */
      case 92: /* tcons ::= CONSTRAINT nm */ yytestcase(yyruleno==92);
{pParse->constraintName = yymsp[0].minor.yy0;}
        break;
      case 55: /* ccons ::= DEFAULT term */
      case 57: /* ccons ::= DEFAULT PLUS term */ yytestcase(yyruleno==57);
{sqlite3AddDefaultValue(pParse,&yymsp[0].minor.yy346);}
        break;
      case 56: /* ccons ::= DEFAULT LP expr RP */
{sqlite3AddDefaultValue(pParse,&yymsp[-1].minor.yy346);}
        break;
      case 58: /* ccons ::= DEFAULT MINUS term */
{
  ExprSpan v;
  v.pExpr = sqlite3PExpr(pParse, TK_UMINUS, yymsp[0].minor.yy346.pExpr, 0, 0);
  v.zStart = yymsp[-1].minor.yy0.z;
  v.zEnd = yymsp[0].minor.yy346.zEnd;
  sqlite3AddDefaultValue(pParse,&v);
}
        break;
      case 59: /* ccons ::= DEFAULT ID|INDEXED */
{
  ExprSpan v;
  spanExpr(&v, pParse, TK_STRING, &yymsp[0].minor.yy0);
  sqlite3AddDefaultValue(pParse,&v);
}
        break;
      case 61: /* ccons ::= NOT NULL onconf */
{sqlite3AddNotNull(pParse, yymsp[0].minor.yy328);}
        break;
      case 62: /* ccons ::= PRIMARY KEY sortorder onconf autoinc */
{sqlite3AddPrimaryKey(pParse,0,yymsp[-1].minor.yy328,yymsp[0].minor.yy328,yymsp[-2].minor.yy328);}
        break;
      case 63: /* ccons ::= UNIQUE onconf */
{sqlite3CreateIndex(pParse,0,0,0,0,yymsp[0].minor.yy328,0,0,0,0);}
        break;
      case 64: /* ccons ::= CHECK LP expr RP */
{sqlite3AddCheckConstraint(pParse,yymsp[-1].minor.yy346.pExpr);}
        break;
      case 65: /* ccons ::= REFERENCES nm idxlist_opt refargs */
{sqlite3CreateForeignKey(pParse,0,&yymsp[-2].minor.yy0,yymsp[-1].minor.yy14,yymsp[0].minor.yy328);}
        break;
      case 66: /* ccons ::= defer_subclause */
{sqlite3DeferForeignKey(pParse,yymsp[0].minor.yy328);}
        break;
      case 67: /* ccons ::= COLLATE ID|STRING */
{sqlite3AddCollateType(pParse, &yymsp[0].minor.yy0);}
        break;
      case 70: /* refargs ::= */
{ yygotominor.yy328 = OE_None*0x0101; /* EV: R-19803-45884 */}
        break;
      case 71: /* refargs ::= refargs refarg */
{ yygotominor.yy328 = (yymsp[-1].minor.yy328 & ~yymsp[0].minor.yy429.mask) | yymsp[0].minor.yy429.value; }
        break;
      case 72: /* refarg ::= MATCH nm */
      case 73: /* refarg ::= ON INSERT refact */ yytestcase(yyruleno==73);
{ yygotominor.yy429.value = 0;     yygotominor.yy429.mask = 0x000000; }
        break;
      case 74: /* refarg ::= ON DELETE refact */
{ yygotominor.yy429.value = yymsp[0].minor.yy328;     yygotominor.yy429.mask = 0x0000ff; }
        break;
      case 75: /* refarg ::= ON UPDATE refact */
{ yygotominor.yy429.value = yymsp[0].minor.yy328<<8;  yygotominor.yy429.mask = 0x00ff00; }
        break;
      case 76: /* refact ::= SET NULL */
{ yygotominor.yy328 = OE_SetNull;  /* EV: R-33326-45252 */}
        break;
      case 77: /* refact ::= SET DEFAULT */
{ yygotominor.yy328 = OE_SetDflt;  /* EV: R-33326-45252 */}
        break;
      case 78: /* refact ::= CASCADE */
{ yygotominor.yy328 = OE_Cascade;  /* EV: R-33326-45252 */}
        break;
      case 79: /* refact ::= RESTRICT */
{ yygotominor.yy328 = OE_Restrict; /* EV: R-33326-45252 */}
        break;
      case 80: /* refact ::= NO ACTION */
{ yygotominor.yy328 = OE_None;     /* EV: R-33326-45252 */}
        break;
      case 82: /* defer_subclause ::= DEFERRABLE init_deferred_pred_opt */
      case 98: /* defer_subclause_opt ::= defer_subclause */ yytestcase(yyruleno==98);
      case 100: /* onconf ::= ON CONFLICT resolvetype */ yytestcase(yyruleno==100);
      case 103: /* resolvetype ::= raisetype */ yytestcase(yyruleno==103);
{yygotominor.yy328 = yymsp[0].minor.yy328;}
        break;
      case 86: /* conslist_opt ::= */
{yygotominor.yy0.n = 0; yygotominor.yy0.z = 0;}
        break;
      case 87: /* conslist_opt ::= COMMA conslist */
{yygotominor.yy0 = yymsp[-1].minor.yy0;}
        break;
      case 90: /* tconscomma ::= COMMA */
{pParse->constraintName.n = 0;}
        break;
      case 93: /* tcons ::= PRIMARY KEY LP idxlist autoinc RP onconf */
{sqlite3AddPrimaryKey(pParse,yymsp[-3].minor.yy14,yymsp[0].minor.yy328,yymsp[-2].minor.yy328,0);}
        break;
      case 94: /* tcons ::= UNIQUE LP idxlist RP onconf */
{sqlite3CreateIndex(pParse,0,0,0,yymsp[-2].minor.yy14,yymsp[0].minor.yy328,0,0,0,0);}
        break;
      case 95: /* tcons ::= CHECK LP expr RP onconf */
{sqlite3AddCheckConstraint(pParse,yymsp[-2].minor.yy346.pExpr);}
        break;
      case 96: /* tcons ::= FOREIGN KEY LP idxlist RP REFERENCES nm idxlist_opt refargs defer_subclause_opt */
{
    sqlite3CreateForeignKey(pParse, yymsp[-6].minor.yy14, &yymsp[-3].minor.yy0, yymsp[-2].minor.yy14, yymsp[-1].minor.yy328);
    sqlite3DeferForeignKey(pParse, yymsp[0].minor.yy328);
}
        break;
      case 99: /* onconf ::= */
{yygotominor.yy328 = OE_Default;}
        break;
      case 101: /* orconf ::= */
{yygotominor.yy186 = OE_Default;}
        break;
      case 102: /* orconf ::= OR resolvetype */
{yygotominor.yy186 = (u8)yymsp[0].minor.yy328;}
        break;
      case 104: /* resolvetype ::= IGNORE */
{yygotominor.yy328 = OE_Ignore;}
        break;
      case 105: /* resolvetype ::= REPLACE */
{yygotominor.yy328 = OE_Replace;}
        break;
      case 106: /* cmd ::= DROP TABLE ifexists fullname */
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy65, 0, yymsp[-1].minor.yy328);
}
        break;
      case 109: /* cmd ::= createkw temp VIEW ifnotexists nm dbnm AS select */
{
  sqlite3CreateView(pParse, &yymsp[-7].minor.yy0, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, yymsp[0].minor.yy3, yymsp[-6].minor.yy328, yymsp[-4].minor.yy328);
}
        break;
      case 110: /* cmd ::= DROP VIEW ifexists fullname */
{
  sqlite3DropTable(pParse, yymsp[0].minor.yy65, 1, yymsp[-1].minor.yy328);
}
        break;
      case 111: /* cmd ::= select */
{
  SelectDest dest = {SRT_Output, 0, 0, 0, 0, 0};
  sqlite3Select(pParse, yymsp[0].minor.yy3, &dest);
  sqlite3SelectDelete(pParse->db, yymsp[0].minor.yy3);
}
        break;
      case 112: /* select ::= with selectnowith */
{
  Select *p = yymsp[0].minor.yy3;
  if( p ){
    p->pWith = yymsp[-1].minor.yy59;
    parserDoubleLinkSelect(pParse, p);
  }else{
    sqlite3WithDelete(pParse->db, yymsp[-1].minor.yy59);
  }
  yygotominor.yy3 = p;
}
        break;
      case 113: /* selectnowith ::= oneselect */
      case 119: /* oneselect ::= values */ yytestcase(yyruleno==119);
{yygotominor.yy3 = yymsp[0].minor.yy3;}
        break;
      case 114: /* selectnowith ::= selectnowith multiselect_op oneselect */
{
  Select *pRhs = yymsp[0].minor.yy3;
  Select *pLhs = yymsp[-2].minor.yy3;
  if( pRhs && pRhs->pPrior ){
    SrcList *pFrom;
    Token x;
    x.n = 0;
    parserDoubleLinkSelect(pParse, pRhs);
    pFrom = sqlite3SrcListAppendFromTerm(pParse,0,0,0,&x,pRhs,0,0);
    pRhs = sqlite3SelectNew(pParse,0,pFrom,0,0,0,0,0,0,0);
  }
  if( pRhs ){
    pRhs->op = (u8)yymsp[-1].minor.yy328;
    pRhs->pPrior = pLhs;
    if( ALWAYS(pLhs) ) pLhs->selFlags &= ~SF_MultiValue;
    pRhs->selFlags &= ~SF_MultiValue;
    if( yymsp[-1].minor.yy328!=TK_ALL ) pParse->hasCompound = 1;
  }else{
    sqlite3SelectDelete(pParse->db, pLhs);
  }
  yygotominor.yy3 = pRhs;
}
        break;
      case 116: /* multiselect_op ::= UNION ALL */
{yygotominor.yy328 = TK_ALL;}
        break;
      case 118: /* oneselect ::= SELECT distinct selcollist from where_opt groupby_opt having_opt orderby_opt limit_opt */
{
  yygotominor.yy3 = sqlite3SelectNew(pParse,yymsp[-6].minor.yy14,yymsp[-5].minor.yy65,yymsp[-4].minor.yy132,yymsp[-3].minor.yy14,yymsp[-2].minor.yy132,yymsp[-1].minor.yy14,yymsp[-7].minor.yy381,yymsp[0].minor.yy476.pLimit,yymsp[0].minor.yy476.pOffset);
#if SELECTTRACE_ENABLED
  /* Populate the Select.zSelName[] string that is used to help with
  ** query planner debugging, to differentiate between multiple Select
  ** objects in a complex query.
  **
  ** If the SELECT keyword is immediately followed by a C-style comment
  ** then extract the first few alphanumeric characters from within that
  ** comment to be the zSelName value.  Otherwise, the label is #N where
  ** is an integer that is incremented with each SELECT statement seen.
  */
  if( yygotominor.yy3!=0 ){
    const char *z = yymsp[-8].minor.yy0.z+6;
    int i;
    sqlite3_snprintf(sizeof(yygotominor.yy3->zSelName), yygotominor.yy3->zSelName, "#%d",
                     ++pParse->nSelect);
    while( z[0]==' ' ) z++;
    if( z[0]=='/' && z[1]=='*' ){
      z += 2;
      while( z[0]==' ' ) z++;
      for(i=0; sqlite3Isalnum(z[i]); i++){}
      sqlite3_snprintf(sizeof(yygotominor.yy3->zSelName), yygotominor.yy3->zSelName, "%.*s", i, z);
    }
  }
#endif /* SELECTRACE_ENABLED */
}
        break;
      case 120: /* values ::= VALUES LP nexprlist RP */
{
  yygotominor.yy3 = sqlite3SelectNew(pParse,yymsp[-1].minor.yy14,0,0,0,0,0,SF_Values,0,0);
}
        break;
      case 121: /* values ::= values COMMA LP exprlist RP */
{
  Select *pRight, *pLeft = yymsp[-4].minor.yy3;
  pRight = sqlite3SelectNew(pParse,yymsp[-1].minor.yy14,0,0,0,0,0,SF_Values|SF_MultiValue,0,0);
  if( ALWAYS(pLeft) ) pLeft->selFlags &= ~SF_MultiValue;
  if( pRight ){
    pRight->op = TK_ALL;
    pLeft = yymsp[-4].minor.yy3;
    pRight->pPrior = pLeft;
    yygotominor.yy3 = pRight;
  }else{
    yygotominor.yy3 = pLeft;
  }
}
        break;
      case 122: /* distinct ::= DISTINCT */
{yygotominor.yy381 = SF_Distinct;}
        break;
      case 123: /* distinct ::= ALL */
{yygotominor.yy381 = SF_All;}
        break;
      case 124: /* distinct ::= */
{yygotominor.yy381 = 0;}
        break;
      case 125: /* sclp ::= selcollist COMMA */
      case 243: /* idxlist_opt ::= LP idxlist RP */ yytestcase(yyruleno==243);
{yygotominor.yy14 = yymsp[-1].minor.yy14;}
        break;
      case 126: /* sclp ::= */
      case 154: /* orderby_opt ::= */ yytestcase(yyruleno==154);
      case 161: /* groupby_opt ::= */ yytestcase(yyruleno==161);
      case 236: /* exprlist ::= */ yytestcase(yyruleno==236);
      case 242: /* idxlist_opt ::= */ yytestcase(yyruleno==242);
{yygotominor.yy14 = 0;}
        break;
      case 127: /* selcollist ::= sclp expr as */
{
   yygotominor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-2].minor.yy14, yymsp[-1].minor.yy346.pExpr);
   if( yymsp[0].minor.yy0.n>0 ) sqlite3ExprListSetName(pParse, yygotominor.yy14, &yymsp[0].minor.yy0, 1);
   sqlite3ExprListSetSpan(pParse,yygotominor.yy14,&yymsp[-1].minor.yy346);
}
        break;
      case 128: /* selcollist ::= sclp STAR */
{
  Expr *p = sqlite3Expr(pParse->db, TK_ALL, 0);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-1].minor.yy14, p);
}
        break;
      case 129: /* selcollist ::= sclp nm DOT STAR */
{
  Expr *pRight = sqlite3PExpr(pParse, TK_ALL, 0, 0, &yymsp[0].minor.yy0);
  Expr *pLeft = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *pDot = sqlite3PExpr(pParse, TK_DOT, pLeft, pRight, 0);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy14, pDot);
}
        break;
      case 132: /* as ::= */
{yygotominor.yy0.n = 0;}
        break;
      case 133: /* from ::= */
{yygotominor.yy65 = sqlite3DbMallocZero(pParse->db, sizeof(*yygotominor.yy65));}
        break;
      case 134: /* from ::= FROM seltablist */
{
  yygotominor.yy65 = yymsp[0].minor.yy65;
  sqlite3SrcListShiftJoinType(yygotominor.yy65);
}
        break;
      case 135: /* stl_prefix ::= seltablist joinop */
{
   yygotominor.yy65 = yymsp[-1].minor.yy65;
   if( ALWAYS(yygotominor.yy65 && yygotominor.yy65->nSrc>0) ) yygotominor.yy65->a[yygotominor.yy65->nSrc-1].jointype = (u8)yymsp[0].minor.yy328;
}
        break;
      case 136: /* stl_prefix ::= */
{yygotominor.yy65 = 0;}
        break;
      case 137: /* seltablist ::= stl_prefix nm dbnm as indexed_opt on_opt using_opt */
{
  yygotominor.yy65 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy65,&yymsp[-5].minor.yy0,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,0,yymsp[-1].minor.yy132,yymsp[0].minor.yy408);
  sqlite3SrcListIndexedBy(pParse, yygotominor.yy65, &yymsp[-2].minor.yy0);
}
        break;
      case 138: /* seltablist ::= stl_prefix LP select RP as on_opt using_opt */
{
    yygotominor.yy65 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy65,0,0,&yymsp[-2].minor.yy0,yymsp[-4].minor.yy3,yymsp[-1].minor.yy132,yymsp[0].minor.yy408);
  }
        break;
      case 139: /* seltablist ::= stl_prefix LP seltablist RP as on_opt using_opt */
{
    if( yymsp[-6].minor.yy65==0 && yymsp[-2].minor.yy0.n==0 && yymsp[-1].minor.yy132==0 && yymsp[0].minor.yy408==0 ){
      yygotominor.yy65 = yymsp[-4].minor.yy65;
    }else if( yymsp[-4].minor.yy65->nSrc==1 ){
      yygotominor.yy65 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy65,0,0,&yymsp[-2].minor.yy0,0,yymsp[-1].minor.yy132,yymsp[0].minor.yy408);
      if( yygotominor.yy65 ){
        struct SrcList_item *pNew = &yygotominor.yy65->a[yygotominor.yy65->nSrc-1];
        struct SrcList_item *pOld = yymsp[-4].minor.yy65->a;
        pNew->zName = pOld->zName;
        pNew->zDatabase = pOld->zDatabase;
        pNew->pSelect = pOld->pSelect;
        pOld->zName = pOld->zDatabase = 0;
        pOld->pSelect = 0;
      }
      sqlite3SrcListDelete(pParse->db, yymsp[-4].minor.yy65);
    }else{
      Select *pSubquery;
      sqlite3SrcListShiftJoinType(yymsp[-4].minor.yy65);
      pSubquery = sqlite3SelectNew(pParse,0,yymsp[-4].minor.yy65,0,0,0,0,SF_NestedFrom,0,0);
      yygotominor.yy65 = sqlite3SrcListAppendFromTerm(pParse,yymsp[-6].minor.yy65,0,0,&yymsp[-2].minor.yy0,pSubquery,yymsp[-1].minor.yy132,yymsp[0].minor.yy408);
    }
  }
        break;
      case 140: /* dbnm ::= */
      case 149: /* indexed_opt ::= */ yytestcase(yyruleno==149);
{yygotominor.yy0.z=0; yygotominor.yy0.n=0;}
        break;
      case 142: /* fullname ::= nm dbnm */
{yygotominor.yy65 = sqlite3SrcListAppend(pParse->db,0,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0);}
        break;
      case 143: /* joinop ::= COMMA|JOIN */
{ yygotominor.yy328 = JT_INNER; }
        break;
      case 144: /* joinop ::= JOIN_KW JOIN */
{ yygotominor.yy328 = sqlite3JoinType(pParse,&yymsp[-1].minor.yy0,0,0); }
        break;
      case 145: /* joinop ::= JOIN_KW nm JOIN */
{ yygotominor.yy328 = sqlite3JoinType(pParse,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0,0); }
        break;
      case 146: /* joinop ::= JOIN_KW nm nm JOIN */
{ yygotominor.yy328 = sqlite3JoinType(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[-1].minor.yy0); }
        break;
      case 147: /* on_opt ::= ON expr */
      case 164: /* having_opt ::= HAVING expr */ yytestcase(yyruleno==164);
      case 171: /* where_opt ::= WHERE expr */ yytestcase(yyruleno==171);
      case 231: /* case_else ::= ELSE expr */ yytestcase(yyruleno==231);
      case 233: /* case_operand ::= expr */ yytestcase(yyruleno==233);
{yygotominor.yy132 = yymsp[0].minor.yy346.pExpr;}
        break;
      case 148: /* on_opt ::= */
      case 163: /* having_opt ::= */ yytestcase(yyruleno==163);
      case 170: /* where_opt ::= */ yytestcase(yyruleno==170);
      case 232: /* case_else ::= */ yytestcase(yyruleno==232);
      case 234: /* case_operand ::= */ yytestcase(yyruleno==234);
{yygotominor.yy132 = 0;}
        break;
      case 151: /* indexed_opt ::= NOT INDEXED */
{yygotominor.yy0.z=0; yygotominor.yy0.n=1;}
        break;
      case 152: /* using_opt ::= USING LP idlist RP */
      case 180: /* inscollist_opt ::= LP idlist RP */ yytestcase(yyruleno==180);
{yygotominor.yy408 = yymsp[-1].minor.yy408;}
        break;
      case 153: /* using_opt ::= */
      case 179: /* inscollist_opt ::= */ yytestcase(yyruleno==179);
{yygotominor.yy408 = 0;}
        break;
      case 155: /* orderby_opt ::= ORDER BY sortlist */
      case 162: /* groupby_opt ::= GROUP BY nexprlist */ yytestcase(yyruleno==162);
      case 235: /* exprlist ::= nexprlist */ yytestcase(yyruleno==235);
{yygotominor.yy14 = yymsp[0].minor.yy14;}
        break;
      case 156: /* sortlist ::= sortlist COMMA expr sortorder */
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-3].minor.yy14,yymsp[-1].minor.yy346.pExpr);
  if( yygotominor.yy14 ) yygotominor.yy14->a[yygotominor.yy14->nExpr-1].sortOrder = (u8)yymsp[0].minor.yy328;
}
        break;
      case 157: /* sortlist ::= expr sortorder */
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,0,yymsp[-1].minor.yy346.pExpr);
  if( yygotominor.yy14 && ALWAYS(yygotominor.yy14->a) ) yygotominor.yy14->a[0].sortOrder = (u8)yymsp[0].minor.yy328;
}
        break;
      case 158: /* sortorder ::= ASC */
      case 160: /* sortorder ::= */ yytestcase(yyruleno==160);
{yygotominor.yy328 = SQLITE_SO_ASC;}
        break;
      case 159: /* sortorder ::= DESC */
{yygotominor.yy328 = SQLITE_SO_DESC;}
        break;
      case 165: /* limit_opt ::= */
{yygotominor.yy476.pLimit = 0; yygotominor.yy476.pOffset = 0;}
        break;
      case 166: /* limit_opt ::= LIMIT expr */
{yygotominor.yy476.pLimit = yymsp[0].minor.yy346.pExpr; yygotominor.yy476.pOffset = 0;}
        break;
      case 167: /* limit_opt ::= LIMIT expr OFFSET expr */
{yygotominor.yy476.pLimit = yymsp[-2].minor.yy346.pExpr; yygotominor.yy476.pOffset = yymsp[0].minor.yy346.pExpr;}
        break;
      case 168: /* limit_opt ::= LIMIT expr COMMA expr */
{yygotominor.yy476.pOffset = yymsp[-2].minor.yy346.pExpr; yygotominor.yy476.pLimit = yymsp[0].minor.yy346.pExpr;}
        break;
      case 169: /* cmd ::= with DELETE FROM fullname indexed_opt where_opt */
{
  sqlite3WithPush(pParse, yymsp[-5].minor.yy59, 1);
  sqlite3SrcListIndexedBy(pParse, yymsp[-2].minor.yy65, &yymsp[-1].minor.yy0);
  sqlite3DeleteFrom(pParse,yymsp[-2].minor.yy65,yymsp[0].minor.yy132);
}
        break;
      case 172: /* cmd ::= with UPDATE orconf fullname indexed_opt SET setlist where_opt */
{
  sqlite3WithPush(pParse, yymsp[-7].minor.yy59, 1);
  sqlite3SrcListIndexedBy(pParse, yymsp[-4].minor.yy65, &yymsp[-3].minor.yy0);
  sqlite3ExprListCheckLength(pParse,yymsp[-1].minor.yy14,"set list"); 
  sqlite3Update(pParse,yymsp[-4].minor.yy65,yymsp[-1].minor.yy14,yymsp[0].minor.yy132,yymsp[-5].minor.yy186);
}
        break;
      case 173: /* setlist ::= setlist COMMA nm EQ expr */
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse, yymsp[-4].minor.yy14, yymsp[0].minor.yy346.pExpr);
  sqlite3ExprListSetName(pParse, yygotominor.yy14, &yymsp[-2].minor.yy0, 1);
}
        break;
      case 174: /* setlist ::= nm EQ expr */
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse, 0, yymsp[0].minor.yy346.pExpr);
  sqlite3ExprListSetName(pParse, yygotominor.yy14, &yymsp[-2].minor.yy0, 1);
}
        break;
      case 175: /* cmd ::= with insert_cmd INTO fullname inscollist_opt select */
{
  sqlite3WithPush(pParse, yymsp[-5].minor.yy59, 1);
  sqlite3Insert(pParse, yymsp[-2].minor.yy65, yymsp[0].minor.yy3, yymsp[-1].minor.yy408, yymsp[-4].minor.yy186);
}
        break;
      case 176: /* cmd ::= with insert_cmd INTO fullname inscollist_opt DEFAULT VALUES */
{
  sqlite3WithPush(pParse, yymsp[-6].minor.yy59, 1);
  sqlite3Insert(pParse, yymsp[-3].minor.yy65, 0, yymsp[-2].minor.yy408, yymsp[-5].minor.yy186);
}
        break;
      case 177: /* insert_cmd ::= INSERT orconf */
{yygotominor.yy186 = yymsp[0].minor.yy186;}
        break;
      case 178: /* insert_cmd ::= REPLACE */
{yygotominor.yy186 = OE_Replace;}
        break;
      case 181: /* idlist ::= idlist COMMA nm */
{yygotominor.yy408 = sqlite3IdListAppend(pParse->db,yymsp[-2].minor.yy408,&yymsp[0].minor.yy0);}
        break;
      case 182: /* idlist ::= nm */
{yygotominor.yy408 = sqlite3IdListAppend(pParse->db,0,&yymsp[0].minor.yy0);}
        break;
      case 183: /* expr ::= term */
{yygotominor.yy346 = yymsp[0].minor.yy346;}
        break;
      case 184: /* expr ::= LP expr RP */
{yygotominor.yy346.pExpr = yymsp[-1].minor.yy346.pExpr; spanSet(&yygotominor.yy346,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);}
        break;
      case 185: /* term ::= NULL */
      case 190: /* term ::= INTEGER|FLOAT|BLOB */ yytestcase(yyruleno==190);
      case 191: /* term ::= STRING */ yytestcase(yyruleno==191);
{spanExpr(&yygotominor.yy346, pParse, yymsp[0].major, &yymsp[0].minor.yy0);}
        break;
      case 186: /* expr ::= ID|INDEXED */
      case 187: /* expr ::= JOIN_KW */ yytestcase(yyruleno==187);
{spanExpr(&yygotominor.yy346, pParse, TK_ID, &yymsp[0].minor.yy0);}
        break;
      case 188: /* expr ::= nm DOT nm */
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp2, 0);
  spanSet(&yygotominor.yy346,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0);
}
        break;
      case 189: /* expr ::= nm DOT nm DOT nm */
{
  Expr *temp1 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-4].minor.yy0);
  Expr *temp2 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[-2].minor.yy0);
  Expr *temp3 = sqlite3PExpr(pParse, TK_ID, 0, 0, &yymsp[0].minor.yy0);
  Expr *temp4 = sqlite3PExpr(pParse, TK_DOT, temp2, temp3, 0);
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_DOT, temp1, temp4, 0);
  spanSet(&yygotominor.yy346,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
}
        break;
      case 192: /* expr ::= VARIABLE */
{
  if( yymsp[0].minor.yy0.n>=2 && yymsp[0].minor.yy0.z[0]=='#' && sqlite3Isdigit(yymsp[0].minor.yy0.z[1]) ){
    /* When doing a nested parse, one can include terms in an expression
    ** that look like this:   #1 #2 ...  These terms refer to registers
    ** in the virtual machine.  #N is the N-th register. */
    if( pParse->nested==0 ){
      sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &yymsp[0].minor.yy0);
      yygotominor.yy346.pExpr = 0;
    }else{
      yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_REGISTER, 0, 0, &yymsp[0].minor.yy0);
      if( yygotominor.yy346.pExpr ) sqlite3GetInt32(&yymsp[0].minor.yy0.z[1], &yygotominor.yy346.pExpr->iTable);
    }
  }else{
    spanExpr(&yygotominor.yy346, pParse, TK_VARIABLE, &yymsp[0].minor.yy0);
    sqlite3ExprAssignVarNumber(pParse, yygotominor.yy346.pExpr);
  }
  spanSet(&yygotominor.yy346, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
        break;
      case 193: /* expr ::= expr COLLATE ID|STRING */
{
  yygotominor.yy346.pExpr = sqlite3ExprAddCollateToken(pParse, yymsp[-2].minor.yy346.pExpr, &yymsp[0].minor.yy0, 1);
  yygotominor.yy346.zStart = yymsp[-2].minor.yy346.zStart;
  yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
        break;
      case 194: /* expr ::= CAST LP expr AS typetoken RP */
{
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_CAST, yymsp[-3].minor.yy346.pExpr, 0, &yymsp[-1].minor.yy0);
  spanSet(&yygotominor.yy346,&yymsp[-5].minor.yy0,&yymsp[0].minor.yy0);
}
        break;
      case 195: /* expr ::= ID|INDEXED LP distinct exprlist RP */
{
  if( yymsp[-1].minor.yy14 && yymsp[-1].minor.yy14->nExpr>pParse->db->aLimit[SQLITE_LIMIT_FUNCTION_ARG] ){
    sqlite3ErrorMsg(pParse, "too many arguments on function %T", &yymsp[-4].minor.yy0);
  }
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, yymsp[-1].minor.yy14, &yymsp[-4].minor.yy0);
  spanSet(&yygotominor.yy346,&yymsp[-4].minor.yy0,&yymsp[0].minor.yy0);
  if( yymsp[-2].minor.yy381==SF_Distinct && yygotominor.yy346.pExpr ){
    yygotominor.yy346.pExpr->flags |= EP_Distinct;
  }
}
        break;
      case 196: /* expr ::= ID|INDEXED LP STAR RP */
{
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[-3].minor.yy0);
  spanSet(&yygotominor.yy346,&yymsp[-3].minor.yy0,&yymsp[0].minor.yy0);
}
        break;
      case 197: /* term ::= CTIME_KW */
{
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, 0, &yymsp[0].minor.yy0);
  spanSet(&yygotominor.yy346, &yymsp[0].minor.yy0, &yymsp[0].minor.yy0);
}
        break;
      case 198: /* expr ::= expr AND expr */
      case 199: /* expr ::= expr OR expr */ yytestcase(yyruleno==199);
      case 200: /* expr ::= expr LT|GT|GE|LE expr */ yytestcase(yyruleno==200);
      case 201: /* expr ::= expr EQ|NE expr */ yytestcase(yyruleno==201);
      case 202: /* expr ::= expr BITAND|BITOR|LSHIFT|RSHIFT expr */ yytestcase(yyruleno==202);
      case 203: /* expr ::= expr PLUS|MINUS expr */ yytestcase(yyruleno==203);
      case 204: /* expr ::= expr STAR|SLASH|REM expr */ yytestcase(yyruleno==204);
      case 205: /* expr ::= expr CONCAT expr */ yytestcase(yyruleno==205);
{spanBinaryExpr(&yygotominor.yy346,pParse,yymsp[-1].major,&yymsp[-2].minor.yy346,&yymsp[0].minor.yy346);}
        break;
      case 206: /* likeop ::= LIKE_KW|MATCH */
{yygotominor.yy96.eOperator = yymsp[0].minor.yy0; yygotominor.yy96.bNot = 0;}
        break;
      case 207: /* likeop ::= NOT LIKE_KW|MATCH */
{yygotominor.yy96.eOperator = yymsp[0].minor.yy0; yygotominor.yy96.bNot = 1;}
        break;
      case 208: /* expr ::= expr likeop expr */
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[0].minor.yy346.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-2].minor.yy346.pExpr);
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-1].minor.yy96.eOperator);
  if( yymsp[-1].minor.yy96.bNot ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
  yygotominor.yy346.zStart = yymsp[-2].minor.yy346.zStart;
  yygotominor.yy346.zEnd = yymsp[0].minor.yy346.zEnd;
  if( yygotominor.yy346.pExpr ) yygotominor.yy346.pExpr->flags |= EP_InfixFunc;
}
        break;
      case 209: /* expr ::= expr likeop expr ESCAPE expr */
{
  ExprList *pList;
  pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy346.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[-4].minor.yy346.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy346.pExpr);
  yygotominor.yy346.pExpr = sqlite3ExprFunction(pParse, pList, &yymsp[-3].minor.yy96.eOperator);
  if( yymsp[-3].minor.yy96.bNot ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
  yygotominor.yy346.zStart = yymsp[-4].minor.yy346.zStart;
  yygotominor.yy346.zEnd = yymsp[0].minor.yy346.zEnd;
  if( yygotominor.yy346.pExpr ) yygotominor.yy346.pExpr->flags |= EP_InfixFunc;
}
        break;
      case 210: /* expr ::= expr ISNULL|NOTNULL */
{spanUnaryPostfix(&yygotominor.yy346,pParse,yymsp[0].major,&yymsp[-1].minor.yy346,&yymsp[0].minor.yy0);}
        break;
      case 211: /* expr ::= expr NOT NULL */
{spanUnaryPostfix(&yygotominor.yy346,pParse,TK_NOTNULL,&yymsp[-2].minor.yy346,&yymsp[0].minor.yy0);}
        break;
      case 212: /* expr ::= expr IS expr */
{
  spanBinaryExpr(&yygotominor.yy346,pParse,TK_IS,&yymsp[-2].minor.yy346,&yymsp[0].minor.yy346);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy346.pExpr, yygotominor.yy346.pExpr, TK_ISNULL);
}
        break;
      case 213: /* expr ::= expr IS NOT expr */
{
  spanBinaryExpr(&yygotominor.yy346,pParse,TK_ISNOT,&yymsp[-3].minor.yy346,&yymsp[0].minor.yy346);
  binaryToUnaryIfNull(pParse, yymsp[0].minor.yy346.pExpr, yygotominor.yy346.pExpr, TK_NOTNULL);
}
        break;
      case 214: /* expr ::= NOT expr */
      case 215: /* expr ::= BITNOT expr */ yytestcase(yyruleno==215);
{spanUnaryPrefix(&yygotominor.yy346,pParse,yymsp[-1].major,&yymsp[0].minor.yy346,&yymsp[-1].minor.yy0);}
        break;
      case 216: /* expr ::= MINUS expr */
{spanUnaryPrefix(&yygotominor.yy346,pParse,TK_UMINUS,&yymsp[0].minor.yy346,&yymsp[-1].minor.yy0);}
        break;
      case 217: /* expr ::= PLUS expr */
{spanUnaryPrefix(&yygotominor.yy346,pParse,TK_UPLUS,&yymsp[0].minor.yy346,&yymsp[-1].minor.yy0);}
        break;
      case 220: /* expr ::= expr between_op expr AND expr */
{
  ExprList *pList = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy346.pExpr);
  pList = sqlite3ExprListAppend(pParse,pList, yymsp[0].minor.yy346.pExpr);
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_BETWEEN, yymsp[-4].minor.yy346.pExpr, 0, 0);
  if( yygotominor.yy346.pExpr ){
    yygotominor.yy346.pExpr->x.pList = pList;
  }else{
    sqlite3ExprListDelete(pParse->db, pList);
  } 
  if( yymsp[-3].minor.yy328 ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
  yygotominor.yy346.zStart = yymsp[-4].minor.yy346.zStart;
  yygotominor.yy346.zEnd = yymsp[0].minor.yy346.zEnd;
}
        break;
      case 223: /* expr ::= expr in_op LP exprlist RP */
{
    if( yymsp[-1].minor.yy14==0 ){
      /* Expressions of the form
      **
      **      expr1 IN ()
      **      expr1 NOT IN ()
      **
      ** simplify to constants 0 (false) and 1 (true), respectively,
      ** regardless of the value of expr1.
      */
      yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_INTEGER, 0, 0, &sqlite3IntTokens[yymsp[-3].minor.yy328]);
      sqlite3ExprDelete(pParse->db, yymsp[-4].minor.yy346.pExpr);
    }else if( yymsp[-1].minor.yy14->nExpr==1 ){
      /* Expressions of the form:
      **
      **      expr1 IN (?1)
      **      expr1 NOT IN (?2)
      **
      ** with exactly one value on the RHS can be simplified to something
      ** like this:
      **
      **      expr1 == ?1
      **      expr1 <> ?2
      **
      ** But, the RHS of the == or <> is marked with the EP_Generic flag
      ** so that it may not contribute to the computation of comparison
      ** affinity or the collating sequence to use for comparison.  Otherwise,
      ** the semantics would be subtly different from IN or NOT IN.
      */
      Expr *pRHS = yymsp[-1].minor.yy14->a[0].pExpr;
      yymsp[-1].minor.yy14->a[0].pExpr = 0;
      sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy14);
      /* pRHS cannot be NULL because a malloc error would have been detected
      ** before now and control would have never reached this point */
      if( ALWAYS(pRHS) ){
        pRHS->flags &= ~EP_Collate;
        pRHS->flags |= EP_Generic;
      }
      yygotominor.yy346.pExpr = sqlite3PExpr(pParse, yymsp[-3].minor.yy328 ? TK_NE : TK_EQ, yymsp[-4].minor.yy346.pExpr, pRHS, 0);
    }else{
      yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy346.pExpr, 0, 0);
      if( yygotominor.yy346.pExpr ){
        yygotominor.yy346.pExpr->x.pList = yymsp[-1].minor.yy14;
        sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
      }else{
        sqlite3ExprListDelete(pParse->db, yymsp[-1].minor.yy14);
      }
      if( yymsp[-3].minor.yy328 ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
    }
    yygotominor.yy346.zStart = yymsp[-4].minor.yy346.zStart;
    yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
        break;
      case 224: /* expr ::= LP select RP */
{
    yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_SELECT, 0, 0, 0);
    if( yygotominor.yy346.pExpr ){
      yygotominor.yy346.pExpr->x.pSelect = yymsp[-1].minor.yy3;
      ExprSetProperty(yygotominor.yy346.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy3);
    }
    yygotominor.yy346.zStart = yymsp[-2].minor.yy0.z;
    yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
        break;
      case 225: /* expr ::= expr in_op LP select RP */
{
    yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-4].minor.yy346.pExpr, 0, 0);
    if( yygotominor.yy346.pExpr ){
      yygotominor.yy346.pExpr->x.pSelect = yymsp[-1].minor.yy3;
      ExprSetProperty(yygotominor.yy346.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy3);
    }
    if( yymsp[-3].minor.yy328 ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
    yygotominor.yy346.zStart = yymsp[-4].minor.yy346.zStart;
    yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
        break;
      case 226: /* expr ::= expr in_op nm dbnm */
{
    SrcList *pSrc = sqlite3SrcListAppend(pParse->db, 0,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0);
    yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_IN, yymsp[-3].minor.yy346.pExpr, 0, 0);
    if( yygotominor.yy346.pExpr ){
      yygotominor.yy346.pExpr->x.pSelect = sqlite3SelectNew(pParse, 0,pSrc,0,0,0,0,0,0,0);
      ExprSetProperty(yygotominor.yy346.pExpr, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
    }else{
      sqlite3SrcListDelete(pParse->db, pSrc);
    }
    if( yymsp[-2].minor.yy328 ) yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_NOT, yygotominor.yy346.pExpr, 0, 0);
    yygotominor.yy346.zStart = yymsp[-3].minor.yy346.zStart;
    yygotominor.yy346.zEnd = yymsp[0].minor.yy0.z ? &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n] : &yymsp[-1].minor.yy0.z[yymsp[-1].minor.yy0.n];
  }
        break;
      case 227: /* expr ::= EXISTS LP select RP */
{
    Expr *p = yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_EXISTS, 0, 0, 0);
    if( p ){
      p->x.pSelect = yymsp[-1].minor.yy3;
      ExprSetProperty(p, EP_xIsSelect|EP_Subquery);
      sqlite3ExprSetHeightAndFlags(pParse, p);
    }else{
      sqlite3SelectDelete(pParse->db, yymsp[-1].minor.yy3);
    }
    yygotominor.yy346.zStart = yymsp[-3].minor.yy0.z;
    yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
  }
        break;
      case 228: /* expr ::= CASE case_operand case_exprlist case_else END */
{
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_CASE, yymsp[-3].minor.yy132, 0, 0);
  if( yygotominor.yy346.pExpr ){
    yygotominor.yy346.pExpr->x.pList = yymsp[-1].minor.yy132 ? sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy14,yymsp[-1].minor.yy132) : yymsp[-2].minor.yy14;
    sqlite3ExprSetHeightAndFlags(pParse, yygotominor.yy346.pExpr);
  }else{
    sqlite3ExprListDelete(pParse->db, yymsp[-2].minor.yy14);
    sqlite3ExprDelete(pParse->db, yymsp[-1].minor.yy132);
  }
  yygotominor.yy346.zStart = yymsp[-4].minor.yy0.z;
  yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
        break;
      case 229: /* case_exprlist ::= case_exprlist WHEN expr THEN expr */
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-4].minor.yy14, yymsp[-2].minor.yy346.pExpr);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yygotominor.yy14, yymsp[0].minor.yy346.pExpr);
}
        break;
      case 230: /* case_exprlist ::= WHEN expr THEN expr */
{
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,0, yymsp[-2].minor.yy346.pExpr);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yygotominor.yy14, yymsp[0].minor.yy346.pExpr);
}
        break;
      case 237: /* nexprlist ::= nexprlist COMMA expr */
{yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-2].minor.yy14,yymsp[0].minor.yy346.pExpr);}
        break;
      case 238: /* nexprlist ::= expr */
{yygotominor.yy14 = sqlite3ExprListAppend(pParse,0,yymsp[0].minor.yy346.pExpr);}
        break;
      case 239: /* cmd ::= createkw uniqueflag INDEX ifnotexists nm dbnm ON nm LP idxlist RP where_opt */
{
  sqlite3CreateIndex(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, 
                     sqlite3SrcListAppend(pParse->db,0,&yymsp[-4].minor.yy0,0), yymsp[-2].minor.yy14, yymsp[-10].minor.yy328,
                      &yymsp[-11].minor.yy0, yymsp[0].minor.yy132, SQLITE_SO_ASC, yymsp[-8].minor.yy328);
}
        break;
      case 240: /* uniqueflag ::= UNIQUE */
      case 291: /* raisetype ::= ABORT */ yytestcase(yyruleno==291);
{yygotominor.yy328 = OE_Abort;}
        break;
      case 241: /* uniqueflag ::= */
{yygotominor.yy328 = OE_None;}
        break;
      case 244: /* idxlist ::= idxlist COMMA nm collate sortorder */
{
  Expr *p = sqlite3ExprAddCollateToken(pParse, 0, &yymsp[-1].minor.yy0, 1);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,yymsp[-4].minor.yy14, p);
  sqlite3ExprListSetName(pParse,yygotominor.yy14,&yymsp[-2].minor.yy0,1);
  sqlite3ExprListCheckLength(pParse, yygotominor.yy14, "index");
  if( yygotominor.yy14 ) yygotominor.yy14->a[yygotominor.yy14->nExpr-1].sortOrder = (u8)yymsp[0].minor.yy328;
}
        break;
      case 245: /* idxlist ::= nm collate sortorder */
{
  Expr *p = sqlite3ExprAddCollateToken(pParse, 0, &yymsp[-1].minor.yy0, 1);
  yygotominor.yy14 = sqlite3ExprListAppend(pParse,0, p);
  sqlite3ExprListSetName(pParse, yygotominor.yy14, &yymsp[-2].minor.yy0, 1);
  sqlite3ExprListCheckLength(pParse, yygotominor.yy14, "index");
  if( yygotominor.yy14 ) yygotominor.yy14->a[yygotominor.yy14->nExpr-1].sortOrder = (u8)yymsp[0].minor.yy328;
}
        break;
      case 246: /* collate ::= */
{yygotominor.yy0.z = 0; yygotominor.yy0.n = 0;}
        break;
      case 248: /* cmd ::= DROP INDEX ifexists fullname */
{sqlite3DropIndex(pParse, yymsp[0].minor.yy65, yymsp[-1].minor.yy328);}
        break;
      case 249: /* cmd ::= VACUUM */
      case 250: /* cmd ::= VACUUM nm */ yytestcase(yyruleno==250);
{sqlite3Vacuum(pParse);}
        break;
      case 251: /* cmd ::= PRAGMA nm dbnm */
{sqlite3Pragma(pParse,&yymsp[-1].minor.yy0,&yymsp[0].minor.yy0,0,0);}
        break;
      case 252: /* cmd ::= PRAGMA nm dbnm EQ nmnum */
{sqlite3Pragma(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0,0);}
        break;
      case 253: /* cmd ::= PRAGMA nm dbnm LP nmnum RP */
{sqlite3Pragma(pParse,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,&yymsp[-1].minor.yy0,0);}
        break;
      case 254: /* cmd ::= PRAGMA nm dbnm EQ minus_num */
{sqlite3Pragma(pParse,&yymsp[-3].minor.yy0,&yymsp[-2].minor.yy0,&yymsp[0].minor.yy0,1);}
        break;
      case 255: /* cmd ::= PRAGMA nm dbnm LP minus_num RP */
{sqlite3Pragma(pParse,&yymsp[-4].minor.yy0,&yymsp[-3].minor.yy0,&yymsp[-1].minor.yy0,1);}
        break;
      case 264: /* cmd ::= createkw trigger_decl BEGIN trigger_cmd_list END */
{
  Token all;
  all.z = yymsp[-3].minor.yy0.z;
  all.n = (int)(yymsp[0].minor.yy0.z - yymsp[-3].minor.yy0.z) + yymsp[0].minor.yy0.n;
  sqlite3FinishTrigger(pParse, yymsp[-1].minor.yy473, &all);
}
        break;
      case 265: /* trigger_decl ::= temp TRIGGER ifnotexists nm dbnm trigger_time trigger_event ON fullname foreach_clause when_clause */
{
  sqlite3BeginTrigger(pParse, &yymsp[-7].minor.yy0, &yymsp[-6].minor.yy0, yymsp[-5].minor.yy328, yymsp[-4].minor.yy378.a, yymsp[-4].minor.yy378.b, yymsp[-2].minor.yy65, yymsp[0].minor.yy132, yymsp[-10].minor.yy328, yymsp[-8].minor.yy328);
  yygotominor.yy0 = (yymsp[-6].minor.yy0.n==0?yymsp[-7].minor.yy0:yymsp[-6].minor.yy0);
}
        break;
      case 266: /* trigger_time ::= BEFORE */
      case 269: /* trigger_time ::= */ yytestcase(yyruleno==269);
{ yygotominor.yy328 = TK_BEFORE; }
        break;
      case 267: /* trigger_time ::= AFTER */
{ yygotominor.yy328 = TK_AFTER;  }
        break;
      case 268: /* trigger_time ::= INSTEAD OF */
{ yygotominor.yy328 = TK_INSTEAD;}
        break;
      case 270: /* trigger_event ::= DELETE|INSERT */
      case 271: /* trigger_event ::= UPDATE */ yytestcase(yyruleno==271);
{yygotominor.yy378.a = yymsp[0].major; yygotominor.yy378.b = 0;}
        break;
      case 272: /* trigger_event ::= UPDATE OF idlist */
{yygotominor.yy378.a = TK_UPDATE; yygotominor.yy378.b = yymsp[0].minor.yy408;}
        break;
      case 275: /* when_clause ::= */
      case 296: /* key_opt ::= */ yytestcase(yyruleno==296);
{ yygotominor.yy132 = 0; }
        break;
      case 276: /* when_clause ::= WHEN expr */
      case 297: /* key_opt ::= KEY expr */ yytestcase(yyruleno==297);
{ yygotominor.yy132 = yymsp[0].minor.yy346.pExpr; }
        break;
      case 277: /* trigger_cmd_list ::= trigger_cmd_list trigger_cmd SEMI */
{
  assert( yymsp[-2].minor.yy473!=0 );
  yymsp[-2].minor.yy473->pLast->pNext = yymsp[-1].minor.yy473;
  yymsp[-2].minor.yy473->pLast = yymsp[-1].minor.yy473;
  yygotominor.yy473 = yymsp[-2].minor.yy473;
}
        break;
      case 278: /* trigger_cmd_list ::= trigger_cmd SEMI */
{ 
  assert( yymsp[-1].minor.yy473!=0 );
  yymsp[-1].minor.yy473->pLast = yymsp[-1].minor.yy473;
  yygotominor.yy473 = yymsp[-1].minor.yy473;
}
        break;
      case 280: /* trnm ::= nm DOT nm */
{
  yygotominor.yy0 = yymsp[0].minor.yy0;
  sqlite3ErrorMsg(pParse, 
        "qualified table names are not allowed on INSERT, UPDATE, and DELETE "
        "statements within triggers");
}
        break;
      case 282: /* tridxby ::= INDEXED BY nm */
{
  sqlite3ErrorMsg(pParse,
        "the INDEXED BY clause is not allowed on UPDATE or DELETE statements "
        "within triggers");
}
        break;
      case 283: /* tridxby ::= NOT INDEXED */
{
  sqlite3ErrorMsg(pParse,
        "the NOT INDEXED clause is not allowed on UPDATE or DELETE statements "
        "within triggers");
}
        break;
      case 284: /* trigger_cmd ::= UPDATE orconf trnm tridxby SET setlist where_opt */
{ yygotominor.yy473 = sqlite3TriggerUpdateStep(pParse->db, &yymsp[-4].minor.yy0, yymsp[-1].minor.yy14, yymsp[0].minor.yy132, yymsp[-5].minor.yy186); }
        break;
      case 285: /* trigger_cmd ::= insert_cmd INTO trnm inscollist_opt select */
{yygotominor.yy473 = sqlite3TriggerInsertStep(pParse->db, &yymsp[-2].minor.yy0, yymsp[-1].minor.yy408, yymsp[0].minor.yy3, yymsp[-4].minor.yy186);}
        break;
      case 286: /* trigger_cmd ::= DELETE FROM trnm tridxby where_opt */
{yygotominor.yy473 = sqlite3TriggerDeleteStep(pParse->db, &yymsp[-2].minor.yy0, yymsp[0].minor.yy132);}
        break;
      case 287: /* trigger_cmd ::= select */
{yygotominor.yy473 = sqlite3TriggerSelectStep(pParse->db, yymsp[0].minor.yy3); }
        break;
      case 288: /* expr ::= RAISE LP IGNORE RP */
{
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_RAISE, 0, 0, 0); 
  if( yygotominor.yy346.pExpr ){
    yygotominor.yy346.pExpr->affinity = OE_Ignore;
  }
  yygotominor.yy346.zStart = yymsp[-3].minor.yy0.z;
  yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
        break;
      case 289: /* expr ::= RAISE LP raisetype COMMA nm RP */
{
  yygotominor.yy346.pExpr = sqlite3PExpr(pParse, TK_RAISE, 0, 0, &yymsp[-1].minor.yy0); 
  if( yygotominor.yy346.pExpr ) {
    yygotominor.yy346.pExpr->affinity = (char)yymsp[-3].minor.yy328;
  }
  yygotominor.yy346.zStart = yymsp[-5].minor.yy0.z;
  yygotominor.yy346.zEnd = &yymsp[0].minor.yy0.z[yymsp[0].minor.yy0.n];
}
        break;
      case 290: /* raisetype ::= ROLLBACK */
{yygotominor.yy328 = OE_Rollback;}
        break;
      case 292: /* raisetype ::= FAIL */
{yygotominor.yy328 = OE_Fail;}
        break;
      case 293: /* cmd ::= DROP TRIGGER ifexists fullname */
{
  sqlite3DropTrigger(pParse,yymsp[0].minor.yy65,yymsp[-1].minor.yy328);
}
        break;
      case 294: /* cmd ::= ATTACH database_kw_opt expr AS expr key_opt */
{
  sqlite3Attach(pParse, yymsp[-3].minor.yy346.pExpr, yymsp[-1].minor.yy346.pExpr, yymsp[0].minor.yy132);
}
        break;
      case 295: /* cmd ::= DETACH database_kw_opt expr */
{
  sqlite3Detach(pParse, yymsp[0].minor.yy346.pExpr);
}
        break;
      case 300: /* cmd ::= REINDEX */
{sqlite3Reindex(pParse, 0, 0);}
        break;
      case 301: /* cmd ::= REINDEX nm dbnm */
{sqlite3Reindex(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);}
        break;
      case 302: /* cmd ::= ANALYZE */
{sqlite3Analyze(pParse, 0, 0);}
        break;
      case 303: /* cmd ::= ANALYZE nm dbnm */
{sqlite3Analyze(pParse, &yymsp[-1].minor.yy0, &yymsp[0].minor.yy0);}
        break;
      case 304: /* cmd ::= ALTER TABLE fullname RENAME TO nm */
{
  sqlite3AlterRenameTable(pParse,yymsp[-3].minor.yy65,&yymsp[0].minor.yy0);
}
        break;
      case 305: /* cmd ::= ALTER TABLE add_column_fullname ADD kwcolumn_opt column */
{
  sqlite3AlterFinishAddColumn(pParse, &yymsp[0].minor.yy0);
}
        break;
      case 306: /* add_column_fullname ::= fullname */
{
  pParse->db->lookaside.bEnabled = 0;
  sqlite3AlterBeginAddColumn(pParse, yymsp[0].minor.yy65);
}
        break;
      case 309: /* cmd ::= create_vtab */
{sqlite3VtabFinishParse(pParse,0);}
        break;
      case 310: /* cmd ::= create_vtab LP vtabarglist RP */
{sqlite3VtabFinishParse(pParse,&yymsp[0].minor.yy0);}
        break;
      case 311: /* create_vtab ::= createkw VIRTUAL TABLE ifnotexists nm dbnm USING nm */
{
    sqlite3VtabBeginParse(pParse, &yymsp[-3].minor.yy0, &yymsp[-2].minor.yy0, &yymsp[0].minor.yy0, yymsp[-4].minor.yy328);
}
        break;
      case 314: /* vtabarg ::= */
{sqlite3VtabArgInit(pParse);}
        break;
      case 316: /* vtabargtoken ::= ANY */
      case 317: /* vtabargtoken ::= lp anylist RP */ yytestcase(yyruleno==317);
      case 318: /* lp ::= LP */ yytestcase(yyruleno==318);
{sqlite3VtabArgExtend(pParse,&yymsp[0].minor.yy0);}
        break;
      case 322: /* with ::= */
{yygotominor.yy59 = 0;}
        break;
      case 323: /* with ::= WITH wqlist */
      case 324: /* with ::= WITH RECURSIVE wqlist */ yytestcase(yyruleno==324);
{ yygotominor.yy59 = yymsp[0].minor.yy59; }
        break;
      case 325: /* wqlist ::= nm idxlist_opt AS LP select RP */
{
  yygotominor.yy59 = sqlite3WithAdd(pParse, 0, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy14, yymsp[-1].minor.yy3);
}
        break;
      case 326: /* wqlist ::= wqlist COMMA nm idxlist_opt AS LP select RP */
{
  yygotominor.yy59 = sqlite3WithAdd(pParse, yymsp[-7].minor.yy59, &yymsp[-5].minor.yy0, yymsp[-4].minor.yy14, yymsp[-1].minor.yy3);
}
        break;
      default:
      /* (0) input ::= cmdlist */ yytestcase(yyruleno==0);
      /* (1) cmdlist ::= cmdlist ecmd */ yytestcase(yyruleno==1);
      /* (2) cmdlist ::= ecmd */ yytestcase(yyruleno==2);
      /* (3) ecmd ::= SEMI */ yytestcase(yyruleno==3);
      /* (4) ecmd ::= explain cmdx SEMI */ yytestcase(yyruleno==4);
      /* (10) trans_opt ::= */ yytestcase(yyruleno==10);
      /* (11) trans_opt ::= TRANSACTION */ yytestcase(yyruleno==11);
      /* (12) trans_opt ::= TRANSACTION nm */ yytestcase(yyruleno==12);
      /* (20) savepoint_opt ::= SAVEPOINT */ yytestcase(yyruleno==20);
      /* (21) savepoint_opt ::= */ yytestcase(yyruleno==21);
      /* (25) cmd ::= create_table create_table_args */ yytestcase(yyruleno==25);
      /* (36) columnlist ::= columnlist COMMA column */ yytestcase(yyruleno==36);
      /* (37) columnlist ::= column */ yytestcase(yyruleno==37);
      /* (43) type ::= */ yytestcase(yyruleno==43);
      /* (50) signed ::= plus_num */ yytestcase(yyruleno==50);
      /* (51) signed ::= minus_num */ yytestcase(yyruleno==51);
      /* (52) carglist ::= carglist ccons */ yytestcase(yyruleno==52);
      /* (53) carglist ::= */ yytestcase(yyruleno==53);
      /* (60) ccons ::= NULL onconf */ yytestcase(yyruleno==60);
      /* (88) conslist ::= conslist tconscomma tcons */ yytestcase(yyruleno==88);
      /* (89) conslist ::= tcons */ yytestcase(yyruleno==89);
      /* (91) tconscomma ::= */ yytestcase(yyruleno==91);
      /* (273) foreach_clause ::= */ yytestcase(yyruleno==273);
      /* (274) foreach_clause ::= FOR EACH ROW */ yytestcase(yyruleno==274);
      /* (281) tridxby ::= */ yytestcase(yyruleno==281);
      /* (298) database_kw_opt ::= DATABASE */ yytestcase(yyruleno==298);
      /* (299) database_kw_opt ::= */ yytestcase(yyruleno==299);
      /* (307) kwcolumn_opt ::= */ yytestcase(yyruleno==307);
      /* (308) kwcolumn_opt ::= COLUMNKW */ yytestcase(yyruleno==308);
      /* (312) vtabarglist ::= vtabarg */ yytestcase(yyruleno==312);
      /* (313) vtabarglist ::= vtabarglist COMMA vtabarg */ yytestcase(yyruleno==313);
      /* (315) vtabarg ::= vtabarg vtabargtoken */ yytestcase(yyruleno==315);
      /* (319) anylist ::= */ yytestcase(yyruleno==319);
      /* (320) anylist ::= anylist LP anylist RP */ yytestcase(yyruleno==320);
      /* (321) anylist ::= anylist ANY */ yytestcase(yyruleno==321);
        break;
  };
  assert( yyruleno>=0 && yyruleno<sizeof(yyRuleInfo)/sizeof(yyRuleInfo[0]) );
  yygoto = yyRuleInfo[yyruleno].lhs;
  yysize = yyRuleInfo[yyruleno].nrhs;
  yypParser->yyidx -= yysize;
  yyact = yy_find_reduce_action(yymsp[-yysize].stateno,(YYCODETYPE)yygoto);
  if( yyact < YYNSTATE ){
#ifdef NDEBUG
    /* If we are not debugging and the reduce action popped at least
    ** one element off the stack, then we can push the new element back
    ** onto the stack here, and skip the stack overflow test in yy_shift().
    ** That gives a significant speed improvement. */
    if( yysize ){
      yypParser->yyidx++;
      yymsp -= yysize-1;
      yymsp->stateno = (YYACTIONTYPE)yyact;
      yymsp->major = (YYCODETYPE)yygoto;
      yymsp->minor = yygotominor;
    }else
#endif
    {
      yy_shift(yypParser,yyact,yygoto,&yygotominor);
    }
  }else{
    assert( yyact == YYNSTATE + YYNRULE + 1 );
    yy_accept(yypParser);
  }
}

/*
** The following code executes when the parse fails
*/
#ifndef YYNOERRORRECOVERY
static void yy_parse_failed(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sFail!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser fails */
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}
#endif /* YYNOERRORRECOVERY */

/*
** The following code executes when a syntax error first occurs.
*/
static void yy_syntax_error(
  yyParser *yypParser,           /* The parser */
  int yymajor,                   /* The major type of the error token */
  YYMINORTYPE yyminor            /* The minor type of the error token */
){
  sqlite3ParserARG_FETCH;
#define TOKEN (yyminor.yy0)

  UNUSED_PARAMETER(yymajor);  /* Silence some compiler warnings */
  assert( TOKEN.z[0] );  /* The tokenizer always gives us a token */
  sqlite3ErrorMsg(pParse, "near \"%T\": syntax error", &TOKEN);
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/*
** The following is executed when the parser accepts
*/
static void yy_accept(
  yyParser *yypParser           /* The parser */
){
  sqlite3ParserARG_FETCH;
#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sAccept!\n",yyTracePrompt);
  }
#endif
  while( yypParser->yyidx>=0 ) yy_pop_parser_stack(yypParser);
  /* Here code is inserted which will be executed whenever the
  ** parser accepts */
  sqlite3ParserARG_STORE; /* Suppress warning about unused %extra_argument variable */
}

/* The main parser program.
** The first argument is a pointer to a structure obtained from
** "sqlite3ParserAlloc" which describes the current state of the parser.
** The second argument is the major token number.  The third is
** the minor token.  The fourth optional argument is whatever the
** user wants (and specified in the grammar) and is available for
** use by the action routines.
**
** Inputs:
** <ul>
** <li> A pointer to the parser (an opaque structure.)
** <li> The major token number.
** <li> The minor token number.
** <li> An option argument of a grammar-specified type.
** </ul>
**
** Outputs:
** None.
*/
SQLITE_PRIVATE void sqlite3Parser(
  void *yyp,                   /* The parser */
  int yymajor,                 /* The major token code number */
  sqlite3ParserTOKENTYPE yyminor       /* The value for the token */
  sqlite3ParserARG_PDECL               /* Optional %extra_argument parameter */
){
  YYMINORTYPE yyminorunion;
  int yyact;            /* The parser action. */
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  int yyendofinput;     /* True if we are at the end of input */
#endif
#ifdef YYERRORSYMBOL
  int yyerrorhit = 0;   /* True if yymajor has invoked an error */
#endif
  yyParser *yypParser;  /* The parser */

  /* (re)initialize the parser, if necessary */
  yypParser = (yyParser*)yyp;
  if( yypParser->yyidx<0 ){
#if YYSTACKDEPTH<=0
    if( yypParser->yystksz <=0 ){
      /*memset(&yyminorunion, 0, sizeof(yyminorunion));*/
      yyminorunion = yyzerominor;
      yyStackOverflow(yypParser, &yyminorunion);
      return;
    }
#endif
    yypParser->yyidx = 0;
    yypParser->yyerrcnt = -1;
    yypParser->yystack[0].stateno = 0;
    yypParser->yystack[0].major = 0;
  }
  yyminorunion.yy0 = yyminor;
#if !defined(YYERRORSYMBOL) && !defined(YYNOERRORRECOVERY)
  yyendofinput = (yymajor==0);
#endif
  sqlite3ParserARG_STORE;

#ifndef NDEBUG
  if( yyTraceFILE ){
    fprintf(yyTraceFILE,"%sInput %s\n",yyTracePrompt,yyTokenName[yymajor]);
  }
#endif

  do{
    yyact = yy_find_shift_action(yypParser,(YYCODETYPE)yymajor);
    if( yyact<YYNSTATE ){
      yy_shift(yypParser,yyact,yymajor,&yyminorunion);
      yypParser->yyerrcnt--;
      yymajor = YYNOCODE;
    }else if( yyact < YYNSTATE + YYNRULE ){
      yy_reduce(yypParser,yyact-YYNSTATE);
    }else{
      assert( yyact == YY_ERROR_ACTION );
#ifdef YYERRORSYMBOL
      int yymx;
#endif
#ifndef NDEBUG
      if( yyTraceFILE ){
        fprintf(yyTraceFILE,"%sSyntax Error!\n",yyTracePrompt);
      }
#endif
#ifdef YYERRORSYMBOL
      /* A syntax error has occurred.
      ** The response to an error depends upon whether or not the
      ** grammar defines an error token "ERROR".  
      **
      ** This is what we do if the grammar does define ERROR:
      **
      **  * Call the %syntax_error function.
      **
      **  * Begin popping the stack until we enter a state where
      **    it is legal to shift the error symbol, then shift
      **    the error symbol.
      **
      **  * Set the error count to three.
      **
      **  * Begin accepting and shifting new tokens.  No new error
      **    processing will occur until three tokens have been
      **    shifted successfully.
      **
      */
      if( yypParser->yyerrcnt<0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yymx = yypParser->yystack[yypParser->yyidx].major;
      if( yymx==YYERRORSYMBOL || yyerrorhit ){
#ifndef NDEBUG
        if( yyTraceFILE ){
          fprintf(yyTraceFILE,"%sDiscard input token %s\n",
             yyTracePrompt,yyTokenName[yymajor]);
        }
#endif
        yy_destructor(yypParser, (YYCODETYPE)yymajor,&yyminorunion);
        yymajor = YYNOCODE;
      }else{
         while(
          yypParser->yyidx >= 0 &&
          yymx != YYERRORSYMBOL &&
          (yyact = yy_find_reduce_action(
                        yypParser->yystack[yypParser->yyidx].stateno,
                        YYERRORSYMBOL)) >= YYNSTATE
        ){
          yy_pop_parser_stack(yypParser);
        }
        if( yypParser->yyidx < 0 || yymajor==0 ){
          yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
          yy_parse_failed(yypParser);
          yymajor = YYNOCODE;
        }else if( yymx!=YYERRORSYMBOL ){
          YYMINORTYPE u2;
          u2.YYERRSYMDT = 0;
          yy_shift(yypParser,yyact,YYERRORSYMBOL,&u2);
        }
      }
      yypParser->yyerrcnt = 3;
      yyerrorhit = 1;
#elif defined(YYNOERRORRECOVERY)
      /* If the YYNOERRORRECOVERY macro is defined, then do not attempt to
      ** do any kind of error recovery.  Instead, simply invoke the syntax
      ** error routine and continue going as if nothing had happened.
      **
      ** Applications can set this macro (for example inside %include) if
      ** they intend to abandon the parse upon the first syntax error seen.
      */
      yy_syntax_error(yypParser,yymajor,yyminorunion);
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      yymajor = YYNOCODE;
      
#else  /* YYERRORSYMBOL is not defined */
      /* This is what we do if the grammar does not define ERROR:
      **
      **  * Report an error message, and throw away the input token.
      **
      **  * If the input token is $, then fail the parse.
      **
      ** As before, subsequent error messages are suppressed until
      ** three input tokens have been successfully shifted.
      */
      if( yypParser->yyerrcnt<=0 ){
        yy_syntax_error(yypParser,yymajor,yyminorunion);
      }
      yypParser->yyerrcnt = 3;
      yy_destructor(yypParser,(YYCODETYPE)yymajor,&yyminorunion);
      if( yyendofinput ){
        yy_parse_failed(yypParser);
      }
      yymajor = YYNOCODE;
#endif
    }
  }while( yymajor!=YYNOCODE && yypParser->yyidx>=0 );
  return;
}

/************** End of parse.c ***********************************************/
/************** Begin file tokenize.c ****************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** An tokenizer for SQL
**
** This file contains C code that splits an SQL input string up into
** individual tokens and sends those tokens one-by-one over to the
** parser for analysis.
*/
/* #include "sqliteInt.h" */
/* #include <stdlib.h> */

/*
** The charMap() macro maps alphabetic characters into their
** lower-case ASCII equivalent.  On ASCII machines, this is just
** an upper-to-lower case map.  On EBCDIC machines we also need
** to adjust the encoding.  Only alphabetic characters and underscores
** need to be translated.
*/
#ifdef SQLITE_ASCII
# define charMap(X) sqlite3UpperToLower[(unsigned char)X]
#endif
#ifdef SQLITE_EBCDIC
# define charMap(X) ebcdicToAscii[(unsigned char)X]
const unsigned char ebcdicToAscii[] = {
/* 0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 0x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 1x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 2x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 3x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 4x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 5x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 95,  0,  0,  /* 6x */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* 7x */
   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,  /* 8x */
   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  /* 9x */
   0,  0,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,  /* Ax */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* Bx */
   0, 97, 98, 99,100,101,102,103,104,105,  0,  0,  0,  0,  0,  0,  /* Cx */
   0,106,107,108,109,110,111,112,113,114,  0,  0,  0,  0,  0,  0,  /* Dx */
   0,  0,115,116,117,118,119,120,121,122,  0,  0,  0,  0,  0,  0,  /* Ex */
   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  /* Fx */
};
#endif

/*
** The sqlite3KeywordCode function looks up an identifier to determine if
** it is a keyword.  If it is a keyword, the token code of that keyword is 
** returned.  If the input is not a keyword, TK_ID is returned.
**
** The implementation of this routine was generated by a program,
** mkkeywordhash.h, located in the tool subdirectory of the distribution.
** The output of the mkkeywordhash.c program is written into a file
** named keywordhash.h and then included into this source file by
** the #include below.
*/
/************** Include keywordhash.h in the middle of tokenize.c ************/
/************** Begin file keywordhash.h *************************************/
/***** This file contains automatically generated code ******
**
** The code in this file has been automatically generated by
**
**   sqlite/tool/mkkeywordhash.c
**
** The code in this file implements a function that determines whether
** or not a given identifier is really an SQL keyword.  The same thing
** might be implemented more directly using a hand-written hash table.
** But by using this automatically generated code, the size of the code
** is substantially reduced.  This is important for embedded applications
** on platforms with limited memory.
*/
/* Hash score: 182 */
static int keywordCode(const char *z, int n){
  /* zText[] encodes 834 bytes of keywords in 554 bytes */
  /*   REINDEXEDESCAPEACHECKEYBEFOREIGNOREGEXPLAINSTEADDATABASELECT       */
  /*   ABLEFTHENDEFERRABLELSEXCEPTRANSACTIONATURALTERAISEXCLUSIVE         */
  /*   XISTSAVEPOINTERSECTRIGGEREFERENCESCONSTRAINTOFFSETEMPORARY         */
  /*   UNIQUERYWITHOUTERELEASEATTACHAVINGROUPDATEBEGINNERECURSIVE         */
  /*   BETWEENOTNULLIKECASCADELETECASECOLLATECREATECURRENT_DATEDETACH     */
  /*   IMMEDIATEJOINSERTMATCHPLANALYZEPRAGMABORTVALUESVIRTUALIMITWHEN     */
  /*   WHERENAMEAFTEREPLACEANDEFAULTAUTOINCREMENTCASTCOLUMNCOMMIT         */
  /*   CONFLICTCROSSCURRENT_TIMESTAMPRIMARYDEFERREDISTINCTDROPFAIL        */
  /*   FROMFULLGLOBYIFISNULLORDERESTRICTRIGHTROLLBACKROWUNIONUSING        */
  /*   VACUUMVIEWINITIALLY                                                */
  static const char zText[553] = {
    'R','E','I','N','D','E','X','E','D','E','S','C','A','P','E','A','C','H',
    'E','C','K','E','Y','B','E','F','O','R','E','I','G','N','O','R','E','G',
    'E','X','P','L','A','I','N','S','T','E','A','D','D','A','T','A','B','A',
    'S','E','L','E','C','T','A','B','L','E','F','T','H','E','N','D','E','F',
    'E','R','R','A','B','L','E','L','S','E','X','C','E','P','T','R','A','N',
    'S','A','C','T','I','O','N','A','T','U','R','A','L','T','E','R','A','I',
    'S','E','X','C','L','U','S','I','V','E','X','I','S','T','S','A','V','E',
    'P','O','I','N','T','E','R','S','E','C','T','R','I','G','G','E','R','E',
    'F','E','R','E','N','C','E','S','C','O','N','S','T','R','A','I','N','T',
    'O','F','F','S','E','T','E','M','P','O','R','A','R','Y','U','N','I','Q',
    'U','E','R','Y','W','I','T','H','O','U','T','E','R','E','L','E','A','S',
    'E','A','T','T','A','C','H','A','V','I','N','G','R','O','U','P','D','A',
    'T','E','B','E','G','I','N','N','E','R','E','C','U','R','S','I','V','E',
    'B','E','T','W','E','E','N','O','T','N','U','L','L','I','K','E','C','A',
    'S','C','A','D','E','L','E','T','E','C','A','S','E','C','O','L','L','A',
    'T','E','C','R','E','A','T','E','C','U','R','R','E','N','T','_','D','A',
    'T','E','D','E','T','A','C','H','I','M','M','E','D','I','A','T','E','J',
    'O','I','N','S','E','R','T','M','A','T','C','H','P','L','A','N','A','L',
    'Y','Z','E','P','R','A','G','M','A','B','O','R','T','V','A','L','U','E',
    'S','V','I','R','T','U','A','L','I','M','I','T','W','H','E','N','W','H',
    'E','R','E','N','A','M','E','A','F','T','E','R','E','P','L','A','C','E',
    'A','N','D','E','F','A','U','L','T','A','U','T','O','I','N','C','R','E',
    'M','E','N','T','C','A','S','T','C','O','L','U','M','N','C','O','M','M',
    'I','T','C','O','N','F','L','I','C','T','C','R','O','S','S','C','U','R',
    'R','E','N','T','_','T','I','M','E','S','T','A','M','P','R','I','M','A',
    'R','Y','D','E','F','E','R','R','E','D','I','S','T','I','N','C','T','D',
    'R','O','P','F','A','I','L','F','R','O','M','F','U','L','L','G','L','O',
    'B','Y','I','F','I','S','N','U','L','L','O','R','D','E','R','E','S','T',
    'R','I','C','T','R','I','G','H','T','R','O','L','L','B','A','C','K','R',
    'O','W','U','N','I','O','N','U','S','I','N','G','V','A','C','U','U','M',
    'V','I','E','W','I','N','I','T','I','A','L','L','Y',
  };
  static const unsigned char aHash[127] = {
      76, 105, 117,  74,   0,  45,   0,   0,  82,   0,  77,   0,   0,
      42,  12,  78,  15,   0, 116,  85,  54, 112,   0,  19,   0,   0,
     121,   0, 119, 115,   0,  22,  93,   0,   9,   0,   0,  70,  71,
       0,  69,   6,   0,  48,  90, 102,   0, 118, 101,   0,   0,  44,
       0, 103,  24,   0,  17,   0, 122,  53,  23,   0,   5, 110,  25,
      96,   0,   0, 124, 106,  60, 123,  57,  28,  55,   0,  91,   0,
     100,  26,   0,  99,   0,   0,   0,  95,  92,  97,  88, 109,  14,
      39, 108,   0,  81,   0,  18,  89, 111,  32,   0, 120,  80, 113,
      62,  46,  84,   0,   0,  94,  40,  59, 114,   0,  36,   0,   0,
      29,   0,  86,  63,  64,   0,  20,  61,   0,  56,
  };
  static const unsigned char aNext[124] = {
       0,   0,   0,   0,   4,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   2,   0,   0,   0,   0,   0,   0,  13,   0,   0,   0,   0,
       0,   7,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,  33,   0,  21,   0,   0,   0,   0,   0,  50,
       0,  43,   3,  47,   0,   0,   0,   0,  30,   0,  58,   0,  38,
       0,   0,   0,   1,  66,   0,   0,  67,   0,  41,   0,   0,   0,
       0,   0,   0,  49,  65,   0,   0,   0,   0,  31,  52,  16,  34,
      10,   0,   0,   0,   0,   0,   0,   0,  11,  72,  79,   0,   8,
       0, 104,  98,   0, 107,   0,  87,   0,  75,  51,   0,  27,  37,
      73,  83,   0,  35,  68,   0,   0,
  };
  static const unsigned char aLen[124] = {
       7,   7,   5,   4,   6,   4,   5,   3,   6,   7,   3,   6,   6,
       7,   7,   3,   8,   2,   6,   5,   4,   4,   3,  10,   4,   6,
      11,   6,   2,   7,   5,   5,   9,   6,   9,   9,   7,  10,  10,
       4,   6,   2,   3,   9,   4,   2,   6,   5,   7,   4,   5,   7,
       6,   6,   5,   6,   5,   5,   9,   7,   7,   3,   2,   4,   4,
       7,   3,   6,   4,   7,   6,  12,   6,   9,   4,   6,   5,   4,
       7,   6,   5,   6,   7,   5,   4,   5,   6,   5,   7,   3,   7,
      13,   2,   2,   4,   6,   6,   8,   5,  17,  12,   7,   8,   8,
       2,   4,   4,   4,   4,   4,   2,   2,   6,   5,   8,   5,   8,
       3,   5,   5,   6,   4,   9,   3,
  };
  static const unsigned short int aOffset[124] = {
       0,   2,   2,   8,   9,  14,  16,  20,  23,  25,  25,  29,  33,
      36,  41,  46,  48,  53,  54,  59,  62,  65,  67,  69,  78,  81,
      86,  91,  95,  96, 101, 105, 109, 117, 122, 128, 136, 142, 152,
     159, 162, 162, 165, 167, 167, 171, 176, 179, 184, 184, 188, 192,
     199, 204, 209, 212, 218, 221, 225, 234, 240, 240, 240, 243, 246,
     250, 251, 255, 261, 265, 272, 278, 290, 296, 305, 307, 313, 318,
     320, 327, 332, 337, 343, 349, 354, 358, 361, 367, 371, 378, 380,
     387, 389, 391, 400, 404, 410, 416, 424, 429, 429, 445, 452, 459,
     460, 467, 471, 475, 479, 483, 486, 488, 490, 496, 500, 508, 513,
     521, 524, 529, 534, 540, 544, 549,
  };
  static const unsigned char aCode[124] = {
    TK_REINDEX,    TK_INDEXED,    TK_INDEX,      TK_DESC,       TK_ESCAPE,     
    TK_EACH,       TK_CHECK,      TK_KEY,        TK_BEFORE,     TK_FOREIGN,    
    TK_FOR,        TK_IGNORE,     TK_LIKE_KW,    TK_EXPLAIN,    TK_INSTEAD,    
    TK_ADD,        TK_DATABASE,   TK_AS,         TK_SELECT,     TK_TABLE,      
    TK_JOIN_KW,    TK_THEN,       TK_END,        TK_DEFERRABLE, TK_ELSE,       
    TK_EXCEPT,     TK_TRANSACTION,TK_ACTION,     TK_ON,         TK_JOIN_KW,    
    TK_ALTER,      TK_RAISE,      TK_EXCLUSIVE,  TK_EXISTS,     TK_SAVEPOINT,  
    TK_INTERSECT,  TK_TRIGGER,    TK_REFERENCES, TK_CONSTRAINT, TK_INTO,       
    TK_OFFSET,     TK_OF,         TK_SET,        TK_TEMP,       TK_TEMP,       
    TK_OR,         TK_UNIQUE,     TK_QUERY,      TK_WITHOUT,    TK_WITH,       
    TK_JOIN_KW,    TK_RELEASE,    TK_ATTACH,     TK_HAVING,     TK_GROUP,      
    TK_UPDATE,     TK_BEGIN,      TK_JOIN_KW,    TK_RECURSIVE,  TK_BETWEEN,    
    TK_NOTNULL,    TK_NOT,        TK_NO,         TK_NULL,       TK_LIKE_KW,    
    TK_CASCADE,    TK_ASC,        TK_DELETE,     TK_CASE,       TK_COLLATE,    
    TK_CREATE,     TK_CTIME_KW,   TK_DETACH,     TK_IMMEDIATE,  TK_JOIN,       
    TK_INSERT,     TK_MATCH,      TK_PLAN,       TK_ANALYZE,    TK_PRAGMA,     
    TK_ABORT,      TK_VALUES,     TK_VIRTUAL,    TK_LIMIT,      TK_WHEN,       
    TK_WHERE,      TK_RENAME,     TK_AFTER,      TK_REPLACE,    TK_AND,        
    TK_DEFAULT,    TK_AUTOINCR,   TK_TO,         TK_IN,         TK_CAST,       
    TK_COLUMNKW,   TK_COMMIT,     TK_CONFLICT,   TK_JOIN_KW,    TK_CTIME_KW,   
    TK_CTIME_KW,   TK_PRIMARY,    TK_DEFERRED,   TK_DISTINCT,   TK_IS,         
    TK_DROP,       TK_FAIL,       TK_FROM,       TK_JOIN_KW,    TK_LIKE_KW,    
    TK_BY,         TK_IF,         TK_ISNULL,     TK_ORDER,      TK_RESTRICT,   
    TK_JOIN_KW,    TK_ROLLBACK,   TK_ROW,        TK_UNION,      TK_USING,      
    TK_VACUUM,     TK_VIEW,       TK_INITIALLY,  TK_ALL,        
  };
  int h, i;
  if( n<2 ) return TK_ID;
  h = ((charMap(z[0])*4) ^
      (charMap(z[n-1])*3) ^
      n) % 127;
  for(i=((int)aHash[h])-1; i>=0; i=((int)aNext[i])-1){
    if( aLen[i]==n && sqlite3StrNICmp(&zText[aOffset[i]],z,n)==0 ){
      testcase( i==0 ); /* REINDEX */
      testcase( i==1 ); /* INDEXED */
      testcase( i==2 ); /* INDEX */
      testcase( i==3 ); /* DESC */
      testcase( i==4 ); /* ESCAPE */
      testcase( i==5 ); /* EACH */
      testcase( i==6 ); /* CHECK */
      testcase( i==7 ); /* KEY */
      testcase( i==8 ); /* BEFORE */
      testcase( i==9 ); /* FOREIGN */
      testcase( i==10 ); /* FOR */
      testcase( i==11 ); /* IGNORE */
      testcase( i==12 ); /* REGEXP */
      testcase( i==13 ); /* EXPLAIN */
      testcase( i==14 ); /* INSTEAD */
      testcase( i==15 ); /* ADD */
      testcase( i==16 ); /* DATABASE */
      testcase( i==17 ); /* AS */
      testcase( i==18 ); /* SELECT */
      testcase( i==19 ); /* TABLE */
      testcase( i==20 ); /* LEFT */
      testcase( i==21 ); /* THEN */
      testcase( i==22 ); /* END */
      testcase( i==23 ); /* DEFERRABLE */
      testcase( i==24 ); /* ELSE */
      testcase( i==25 ); /* EXCEPT */
      testcase( i==26 ); /* TRANSACTION */
      testcase( i==27 ); /* ACTION */
      testcase( i==28 ); /* ON */
      testcase( i==29 ); /* NATURAL */
      testcase( i==30 ); /* ALTER */
      testcase( i==31 ); /* RAISE */
      testcase( i==32 ); /* EXCLUSIVE */
      testcase( i==33 ); /* EXISTS */
      testcase( i==34 ); /* SAVEPOINT */
      testcase( i==35 ); /* INTERSECT */
      testcase( i==36 ); /* TRIGGER */
      testcase( i==37 ); /* REFERENCES */
      testcase( i==38 ); /* CONSTRAINT */
      testcase( i==39 ); /* INTO */
      testcase( i==40 ); /* OFFSET */
      testcase( i==41 ); /* OF */
      testcase( i==42 ); /* SET */
      testcase( i==43 ); /* TEMPORARY */
      testcase( i==44 ); /* TEMP */
      testcase( i==45 ); /* OR */
      testcase( i==46 ); /* UNIQUE */
      testcase( i==47 ); /* QUERY */
      testcase( i==48 ); /* WITHOUT */
      testcase( i==49 ); /* WITH */
      testcase( i==50 ); /* OUTER */
      testcase( i==51 ); /* RELEASE */
      testcase( i==52 ); /* ATTACH */
      testcase( i==53 ); /* HAVING */
      testcase( i==54 ); /* GROUP */
      testcase( i==55 ); /* UPDATE */
      testcase( i==56 ); /* BEGIN */
      testcase( i==57 ); /* INNER */
      testcase( i==58 ); /* RECURSIVE */
      testcase( i==59 ); /* BETWEEN */
      testcase( i==60 ); /* NOTNULL */
      testcase( i==61 ); /* NOT */
      testcase( i==62 ); /* NO */
      testcase( i==63 ); /* NULL */
      testcase( i==64 ); /* LIKE */
      testcase( i==65 ); /* CASCADE */
      testcase( i==66 ); /* ASC */
      testcase( i==67 ); /* DELETE */
      testcase( i==68 ); /* CASE */
      testcase( i==69 ); /* COLLATE */
      testcase( i==70 ); /* CREATE */
      testcase( i==71 ); /* CURRENT_DATE */
      testcase( i==72 ); /* DETACH */
      testcase( i==73 ); /* IMMEDIATE */
      testcase( i==74 ); /* JOIN */
      testcase( i==75 ); /* INSERT */
      testcase( i==76 ); /* MATCH */
      testcase( i==77 ); /* PLAN */
      testcase( i==78 ); /* ANALYZE */
      testcase( i==79 ); /* PRAGMA */
      testcase( i==80 ); /* ABORT */
      testcase( i==81 ); /* VALUES */
      testcase( i==82 ); /* VIRTUAL */
      testcase( i==83 ); /* LIMIT */
      testcase( i==84 ); /* WHEN */
      testcase( i==85 ); /* WHERE */
      testcase( i==86 ); /* RENAME */
      testcase( i==87 ); /* AFTER */
      testcase( i==88 ); /* REPLACE */
      testcase( i==89 ); /* AND */
      testcase( i==90 ); /* DEFAULT */
      testcase( i==91 ); /* AUTOINCREMENT */
      testcase( i==92 ); /* TO */
      testcase( i==93 ); /* IN */
      testcase( i==94 ); /* CAST */
      testcase( i==95 ); /* COLUMN */
      testcase( i==96 ); /* COMMIT */
      testcase( i==97 ); /* CONFLICT */
      testcase( i==98 ); /* CROSS */
      testcase( i==99 ); /* CURRENT_TIMESTAMP */
      testcase( i==100 ); /* CURRENT_TIME */
      testcase( i==101 ); /* PRIMARY */
      testcase( i==102 ); /* DEFERRED */
      testcase( i==103 ); /* DISTINCT */
      testcase( i==104 ); /* IS */
      testcase( i==105 ); /* DROP */
      testcase( i==106 ); /* FAIL */
      testcase( i==107 ); /* FROM */
      testcase( i==108 ); /* FULL */
      testcase( i==109 ); /* GLOB */
      testcase( i==110 ); /* BY */
      testcase( i==111 ); /* IF */
      testcase( i==112 ); /* ISNULL */
      testcase( i==113 ); /* ORDER */
      testcase( i==114 ); /* RESTRICT */
      testcase( i==115 ); /* RIGHT */
      testcase( i==116 ); /* ROLLBACK */
      testcase( i==117 ); /* ROW */
      testcase( i==118 ); /* UNION */
      testcase( i==119 ); /* USING */
      testcase( i==120 ); /* VACUUM */
      testcase( i==121 ); /* VIEW */
      testcase( i==122 ); /* INITIALLY */
      testcase( i==123 ); /* ALL */
      return aCode[i];
    }
  }
  return TK_ID;
}
SQLITE_PRIVATE int sqlite3KeywordCode(const unsigned char *z, int n){
  return keywordCode((char*)z, n);
}
#define SQLITE_N_KEYWORD 124

/************** End of keywordhash.h *****************************************/
/************** Continuing where we left off in tokenize.c *******************/


/*
** If X is a character that can be used in an identifier then
** IdChar(X) will be true.  Otherwise it is false.
**
** For ASCII, any character with the high-order bit set is
** allowed in an identifier.  For 7-bit characters, 
** sqlite3IsIdChar[X] must be 1.
**
** For EBCDIC, the rules are more complex but have the same
** end result.
**
** Ticket #1066.  the SQL standard does not allow '$' in the
** middle of identifiers.  But many SQL implementations do. 
** SQLite will allow '$' in identifiers for compatibility.
** But the feature is undocumented.
*/
#ifdef SQLITE_ASCII
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned char)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
SQLITE_PRIVATE const char sqlite3IsEbcdicIdChar[] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 4x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0,  /* 5x */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0,  /* 6x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  /* 7x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0,  /* 8x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0,  /* 9x */
    1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0,  /* Ax */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* Bx */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Cx */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Dx */
    0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1,  /* Ex */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0,  /* Fx */
};
#define IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#endif

/* Make the IdChar function accessible from ctime.c */
#ifndef SQLITE_OMIT_COMPILEOPTION_DIAGS
SQLITE_PRIVATE int sqlite3IsIdChar(u8 c){ return IdChar(c); }
#endif


/*
** Return the length of the token that begins at z[0]. 
** Store the token type in *tokenType before returning.
*/
SQLITE_PRIVATE int sqlite3GetToken(const unsigned char *z, int *tokenType){
  int i, c;
  switch( *z ){
    case ' ': case '\t': case '\n': case '\f': case '\r': {
      testcase( z[0]==' ' );
      testcase( z[0]=='\t' );
      testcase( z[0]=='\n' );
      testcase( z[0]=='\f' );
      testcase( z[0]=='\r' );
      for(i=1; sqlite3Isspace(z[i]); i++){}
      *tokenType = TK_SPACE;
      return i;
    }
    case '-': {
      if( z[1]=='-' ){
        for(i=2; (c=z[i])!=0 && c!='\n'; i++){}
        *tokenType = TK_SPACE;   /* IMP: R-22934-25134 */
        return i;
      }
      *tokenType = TK_MINUS;
      return 1;
    }
    case '(': {
      *tokenType = TK_LP;
      return 1;
    }
    case ')': {
      *tokenType = TK_RP;
      return 1;
    }
    case ';': {
      *tokenType = TK_SEMI;
      return 1;
    }
    case '+': {
      *tokenType = TK_PLUS;
      return 1;
    }
    case '*': {
      *tokenType = TK_STAR;
      return 1;
    }
    case '/': {
      if( z[1]!='*' || z[2]==0 ){
        *tokenType = TK_SLASH;
        return 1;
      }
      for(i=3, c=z[2]; (c!='*' || z[i]!='/') && (c=z[i])!=0; i++){}
      if( c ) i++;
      *tokenType = TK_SPACE;   /* IMP: R-22934-25134 */
      return i;
    }
    case '%': {
      *tokenType = TK_REM;
      return 1;
    }
    case '=': {
      *tokenType = TK_EQ;
      return 1 + (z[1]=='=');
    }
    case '<': {
      if( (c=z[1])=='=' ){
        *tokenType = TK_LE;
        return 2;
      }else if( c=='>' ){
        *tokenType = TK_NE;
        return 2;
      }else if( c=='<' ){
        *tokenType = TK_LSHIFT;
        return 2;
      }else{
        *tokenType = TK_LT;
        return 1;
      }
    }
    case '>': {
      if( (c=z[1])=='=' ){
        *tokenType = TK_GE;
        return 2;
      }else if( c=='>' ){
        *tokenType = TK_RSHIFT;
        return 2;
      }else{
        *tokenType = TK_GT;
        return 1;
      }
    }
    case '!': {
      if( z[1]!='=' ){
        *tokenType = TK_ILLEGAL;
        return 2;
      }else{
        *tokenType = TK_NE;
        return 2;
      }
    }
    case '|': {
      if( z[1]!='|' ){
        *tokenType = TK_BITOR;
        return 1;
      }else{
        *tokenType = TK_CONCAT;
        return 2;
      }
    }
    case ',': {
      *tokenType = TK_COMMA;
      return 1;
    }
    case '&': {
      *tokenType = TK_BITAND;
      return 1;
    }
    case '~': {
      *tokenType = TK_BITNOT;
      return 1;
    }
    case '`':
    case '\'':
    case '"': {
      int delim = z[0];
      testcase( delim=='`' );
      testcase( delim=='\'' );
      testcase( delim=='"' );
      for(i=1; (c=z[i])!=0; i++){
        if( c==delim ){
          if( z[i+1]==delim ){
            i++;
          }else{
            break;
          }
        }
      }
      if( c=='\'' ){
        *tokenType = TK_STRING;
        return i+1;
      }else if( c!=0 ){
        *tokenType = TK_ID;
        return i+1;
      }else{
        *tokenType = TK_ILLEGAL;
        return i;
      }
    }
    case '.': {
#ifndef SQLITE_OMIT_FLOATING_POINT
      if( !sqlite3Isdigit(z[1]) )
#endif
      {
        *tokenType = TK_DOT;
        return 1;
      }
      /* If the next character is a digit, this is a floating point
      ** number that begins with ".".  Fall thru into the next case */
    }
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9': {
      testcase( z[0]=='0' );  testcase( z[0]=='1' );  testcase( z[0]=='2' );
      testcase( z[0]=='3' );  testcase( z[0]=='4' );  testcase( z[0]=='5' );
      testcase( z[0]=='6' );  testcase( z[0]=='7' );  testcase( z[0]=='8' );
      testcase( z[0]=='9' );
      *tokenType = TK_INTEGER;
#ifndef SQLITE_OMIT_HEX_INTEGER
      if( z[0]=='0' && (z[1]=='x' || z[1]=='X') && sqlite3Isxdigit(z[2]) ){
        for(i=3; sqlite3Isxdigit(z[i]); i++){}
        return i;
      }
#endif
      for(i=0; sqlite3Isdigit(z[i]); i++){}
#ifndef SQLITE_OMIT_FLOATING_POINT
      if( z[i]=='.' ){
        i++;
        while( sqlite3Isdigit(z[i]) ){ i++; }
        *tokenType = TK_FLOAT;
      }
      if( (z[i]=='e' || z[i]=='E') &&
           ( sqlite3Isdigit(z[i+1]) 
            || ((z[i+1]=='+' || z[i+1]=='-') && sqlite3Isdigit(z[i+2]))
           )
      ){
        i += 2;
        while( sqlite3Isdigit(z[i]) ){ i++; }
        *tokenType = TK_FLOAT;
      }
#endif
      while( IdChar(z[i]) ){
        *tokenType = TK_ILLEGAL;
        i++;
      }
      return i;
    }
    case '[': {
      for(i=1, c=z[0]; c!=']' && (c=z[i])!=0; i++){}
      *tokenType = c==']' ? TK_ID : TK_ILLEGAL;
      return i;
    }
    case '?': {
      *tokenType = TK_VARIABLE;
      for(i=1; sqlite3Isdigit(z[i]); i++){}
      return i;
    }
#ifndef SQLITE_OMIT_TCL_VARIABLE
    case '$':
#endif
    case '@':  /* For compatibility with MS SQL Server */
    case '#':
    case ':': {
      int n = 0;
      testcase( z[0]=='$' );  testcase( z[0]=='@' );
      testcase( z[0]==':' );  testcase( z[0]=='#' );
      *tokenType = TK_VARIABLE;
      for(i=1; (c=z[i])!=0; i++){
        if( IdChar(c) ){
          n++;
#ifndef SQLITE_OMIT_TCL_VARIABLE
        }else if( c=='(' && n>0 ){
          do{
            i++;
          }while( (c=z[i])!=0 && !sqlite3Isspace(c) && c!=')' );
          if( c==')' ){
            i++;
          }else{
            *tokenType = TK_ILLEGAL;
          }
          break;
        }else if( c==':' && z[i+1]==':' ){
          i++;
#endif
        }else{
          break;
        }
      }
      if( n==0 ) *tokenType = TK_ILLEGAL;
      return i;
    }
#ifndef SQLITE_OMIT_BLOB_LITERAL
    case 'x': case 'X': {
      testcase( z[0]=='x' ); testcase( z[0]=='X' );
      if( z[1]=='\'' ){
        *tokenType = TK_BLOB;
        for(i=2; sqlite3Isxdigit(z[i]); i++){}
        if( z[i]!='\'' || i%2 ){
          *tokenType = TK_ILLEGAL;
          while( z[i] && z[i]!='\'' ){ i++; }
        }
        if( z[i] ) i++;
        return i;
      }
      /* Otherwise fall through to the next case */
    }
#endif
    default: {
      if( !IdChar(*z) ){
        break;
      }
      for(i=1; IdChar(z[i]); i++){}
      *tokenType = keywordCode((char*)z, i);
      return i;
    }
  }
  *tokenType = TK_ILLEGAL;
  return 1;
}

/*
** Run the parser on the given SQL string.  The parser structure is
** passed in.  An SQLITE_ status code is returned.  If an error occurs
** then an and attempt is made to write an error message into 
** memory obtained from sqlite3_malloc() and to make *pzErrMsg point to that
** error message.
*/
SQLITE_PRIVATE int sqlite3RunParser(Parse *pParse, const char *zSql, char **pzErrMsg){
  int nErr = 0;                   /* Number of errors encountered */
  int i;                          /* Loop counter */
  void *pEngine;                  /* The LEMON-generated LALR(1) parser */
  int tokenType;                  /* type of the next token */
  int lastTokenParsed = -1;       /* type of the previous token */
  u8 enableLookaside;             /* Saved value of db->lookaside.bEnabled */
  sqlite3 *db = pParse->db;       /* The database connection */
  int mxSqlLen;                   /* Max length of an SQL string */

  assert( zSql!=0 );
  mxSqlLen = db->aLimit[SQLITE_LIMIT_SQL_LENGTH];
  if( db->nVdbeActive==0 ){
    db->u1.isInterrupted = 0;
  }
  pParse->rc = SQLITE_OK;
  pParse->zTail = zSql;
  i = 0;
  assert( pzErrMsg!=0 );
  pEngine = sqlite3ParserAlloc(sqlite3Malloc);
  if( pEngine==0 ){
    db->mallocFailed = 1;
    return SQLITE_NOMEM;
  }
  assert( pParse->pNewTable==0 );
  assert( pParse->pNewTrigger==0 );
  assert( pParse->nVar==0 );
  assert( pParse->nzVar==0 );
  assert( pParse->azVar==0 );
  enableLookaside = db->lookaside.bEnabled;
  if( db->lookaside.pStart ) db->lookaside.bEnabled = 1;
  while( !db->mallocFailed && zSql[i]!=0 ){
    assert( i>=0 );
    pParse->sLastToken.z = &zSql[i];
    pParse->sLastToken.n = sqlite3GetToken((unsigned char*)&zSql[i],&tokenType);
    i += pParse->sLastToken.n;
    if( i>mxSqlLen ){
      pParse->rc = SQLITE_TOOBIG;
      break;
    }
    switch( tokenType ){
      case TK_SPACE: {
        if( db->u1.isInterrupted ){
          sqlite3ErrorMsg(pParse, "interrupt");
          pParse->rc = SQLITE_INTERRUPT;
          goto abort_parse;
        }
        break;
      }
      case TK_ILLEGAL: {
        sqlite3ErrorMsg(pParse, "unrecognized token: \"%T\"",
                        &pParse->sLastToken);
        goto abort_parse;
      }
      case TK_SEMI: {
        pParse->zTail = &zSql[i];
        /* Fall thru into the default case */
      }
      default: {
        sqlite3Parser(pEngine, tokenType, pParse->sLastToken, pParse);
        lastTokenParsed = tokenType;
        if( pParse->rc!=SQLITE_OK ){
          goto abort_parse;
        }
        break;
      }
    }
  }
abort_parse:
  assert( nErr==0 );
  if( pParse->rc==SQLITE_OK && db->mallocFailed==0 ){
    assert( zSql[i]==0 );
    if( lastTokenParsed!=TK_SEMI ){
      sqlite3Parser(pEngine, TK_SEMI, pParse->sLastToken, pParse);
      pParse->zTail = &zSql[i];
    }
    if( pParse->rc==SQLITE_OK && db->mallocFailed==0 ){
      sqlite3Parser(pEngine, 0, pParse->sLastToken, pParse);
    }
  }
#ifdef YYTRACKMAXSTACKDEPTH
  sqlite3_mutex_enter(sqlite3MallocMutex());
  sqlite3StatusSet(SQLITE_STATUS_PARSER_STACK,
      sqlite3ParserStackPeak(pEngine)
  );
  sqlite3_mutex_leave(sqlite3MallocMutex());
#endif /* YYDEBUG */
  sqlite3ParserFree(pEngine, sqlite3_free);
  db->lookaside.bEnabled = enableLookaside;
  if( db->mallocFailed ){
    pParse->rc = SQLITE_NOMEM;
  }
  if( pParse->rc!=SQLITE_OK && pParse->rc!=SQLITE_DONE && pParse->zErrMsg==0 ){
    pParse->zErrMsg = sqlite3MPrintf(db, "%s", sqlite3ErrStr(pParse->rc));
  }
  assert( pzErrMsg!=0 );
  if( pParse->zErrMsg ){
    *pzErrMsg = pParse->zErrMsg;
    sqlite3_log(pParse->rc, "%s", *pzErrMsg);
    pParse->zErrMsg = 0;
    nErr++;
  }
  if( pParse->pVdbe && pParse->nErr>0 && pParse->nested==0 ){
    sqlite3VdbeDelete(pParse->pVdbe);
    pParse->pVdbe = 0;
  }
#ifndef SQLITE_OMIT_SHARED_CACHE
  if( pParse->nested==0 ){
    sqlite3DbFree(db, pParse->aTableLock);
    pParse->aTableLock = 0;
    pParse->nTableLock = 0;
  }
#endif
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3_free(pParse->apVtabLock);
#endif

  if( !IN_DECLARE_VTAB ){
    /* If the pParse->declareVtab flag is set, do not delete any table 
    ** structure built up in pParse->pNewTable. The calling code (see vtab.c)
    ** will take responsibility for freeing the Table structure.
    */
    sqlite3DeleteTable(db, pParse->pNewTable);
  }

  if( pParse->bFreeWith ) sqlite3WithDelete(db, pParse->pWith);
  sqlite3DeleteTrigger(db, pParse->pNewTrigger);
  for(i=pParse->nzVar-1; i>=0; i--) sqlite3DbFree(db, pParse->azVar[i]);
  sqlite3DbFree(db, pParse->azVar);
  while( pParse->pAinc ){
    AutoincInfo *p = pParse->pAinc;
    pParse->pAinc = p->pNext;
    sqlite3DbFree(db, p);
  }
  while( pParse->pZombieTab ){
    Table *p = pParse->pZombieTab;
    pParse->pZombieTab = p->pNextZombie;
    sqlite3DeleteTable(db, p);
  }
  assert( nErr==0 || pParse->rc!=SQLITE_OK );
  return nErr;
}

/************** End of tokenize.c ********************************************/
/************** Begin file complete.c ****************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** An tokenizer for SQL
**
** This file contains C code that implements the sqlite3_complete() API.
** This code used to be part of the tokenizer.c source file.  But by
** separating it out, the code will be automatically omitted from
** static links that do not use it.
*/
/* #include "sqliteInt.h" */
#ifndef SQLITE_OMIT_COMPLETE

/*
** This is defined in tokenize.c.  We just have to import the definition.
*/
#ifndef SQLITE_AMALGAMATION
#ifdef SQLITE_ASCII
#define IdChar(C)  ((sqlite3CtypeMap[(unsigned char)C]&0x46)!=0)
#endif
#ifdef SQLITE_EBCDIC
SQLITE_PRIVATE const char sqlite3IsEbcdicIdChar[];
#define IdChar(C)  (((c=C)>=0x42 && sqlite3IsEbcdicIdChar[c-0x40]))
#endif
#endif /* SQLITE_AMALGAMATION */


/*
** Token types used by the sqlite3_complete() routine.  See the header
** comments on that procedure for additional information.
*/
#define tkSEMI    0
#define tkWS      1
#define tkOTHER   2
#ifndef SQLITE_OMIT_TRIGGER
#define tkEXPLAIN 3
#define tkCREATE  4
#define tkTEMP    5
#define tkTRIGGER 6
#define tkEND     7
#endif

/*
** Return TRUE if the given SQL string ends in a semicolon.
**
** Special handling is require for CREATE TRIGGER statements.
** Whenever the CREATE TRIGGER keywords are seen, the statement
** must end with ";END;".
**
** This implementation uses a state machine with 8 states:
**
**   (0) INVALID   We have not yet seen a non-whitespace character.
**
**   (1) START     At the beginning or end of an SQL statement.  This routine
**                 returns 1 if it ends in the START state and 0 if it ends
**                 in any other state.
**
**   (2) NORMAL    We are in the middle of statement which ends with a single
**                 semicolon.
**
**   (3) EXPLAIN   The keyword EXPLAIN has been seen at the beginning of 
**                 a statement.
**
**   (4) CREATE    The keyword CREATE has been seen at the beginning of a
**                 statement, possibly preceded by EXPLAIN and/or followed by
**                 TEMP or TEMPORARY
**
**   (5) TRIGGER   We are in the middle of a trigger definition that must be
**                 ended by a semicolon, the keyword END, and another semicolon.
**
**   (6) SEMI      We've seen the first semicolon in the ";END;" that occurs at
**                 the end of a trigger definition.
**
**   (7) END       We've seen the ";END" of the ";END;" that occurs at the end
**                 of a trigger definition.
**
** Transitions between states above are determined by tokens extracted
** from the input.  The following tokens are significant:
**
**   (0) tkSEMI      A semicolon.
**   (1) tkWS        Whitespace.
**   (2) tkOTHER     Any other SQL token.
**   (3) tkEXPLAIN   The "explain" keyword.
**   (4) tkCREATE    The "create" keyword.
**   (5) tkTEMP      The "temp" or "temporary" keyword.
**   (6) tkTRIGGER   The "trigger" keyword.
**   (7) tkEND       The "end" keyword.
**
** Whitespace never causes a state transition and is always ignored.
** This means that a SQL string of all whitespace is invalid.
**
** If we compile with SQLITE_OMIT_TRIGGER, all of the computation needed
** to recognize the end of a trigger can be omitted.  All we have to do
** is look for a semicolon that is not part of an string or comment.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_complete(const char *zSql){
  u8 state = 0;   /* Current state, using numbers defined in header comment */
  u8 token;       /* Value of the next token */

#ifndef SQLITE_OMIT_TRIGGER
  /* A complex statement machine used to detect the end of a CREATE TRIGGER
  ** statement.  This is the normal case.
  */
  static const u8 trans[8][8] = {
                     /* Token:                                                */
     /* State:       **  SEMI  WS  OTHER  EXPLAIN  CREATE  TEMP  TRIGGER  END */
     /* 0 INVALID: */ {    1,  0,     2,       3,      4,    2,       2,   2, },
     /* 1   START: */ {    1,  1,     2,       3,      4,    2,       2,   2, },
     /* 2  NORMAL: */ {    1,  2,     2,       2,      2,    2,       2,   2, },
     /* 3 EXPLAIN: */ {    1,  3,     3,       2,      4,    2,       2,   2, },
     /* 4  CREATE: */ {    1,  4,     2,       2,      2,    4,       5,   2, },
     /* 5 TRIGGER: */ {    6,  5,     5,       5,      5,    5,       5,   5, },
     /* 6    SEMI: */ {    6,  6,     5,       5,      5,    5,       5,   7, },
     /* 7     END: */ {    1,  7,     5,       5,      5,    5,       5,   5, },
  };
#else
  /* If triggers are not supported by this compile then the statement machine
  ** used to detect the end of a statement is much simpler
  */
  static const u8 trans[3][3] = {
                     /* Token:           */
     /* State:       **  SEMI  WS  OTHER */
     /* 0 INVALID: */ {    1,  0,     2, },
     /* 1   START: */ {    1,  1,     2, },
     /* 2  NORMAL: */ {    1,  2,     2, },
  };
#endif /* SQLITE_OMIT_TRIGGER */

#ifdef SQLITE_ENABLE_API_ARMOR
  if( zSql==0 ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif

  while( *zSql ){
    switch( *zSql ){
      case ';': {  /* A semicolon */
        token = tkSEMI;
        break;
      }
      case ' ':
      case '\r':
      case '\t':
      case '\n':
      case '\f': {  /* White space is ignored */
        token = tkWS;
        break;
      }
      case '/': {   /* C-style comments */
        if( zSql[1]!='*' ){
          token = tkOTHER;
          break;
        }
        zSql += 2;
        while( zSql[0] && (zSql[0]!='*' || zSql[1]!='/') ){ zSql++; }
        if( zSql[0]==0 ) return 0;
        zSql++;
        token = tkWS;
        break;
      }
      case '-': {   /* SQL-style comments from "--" to end of line */
        if( zSql[1]!='-' ){
          token = tkOTHER;
          break;
        }
        while( *zSql && *zSql!='\n' ){ zSql++; }
        if( *zSql==0 ) return state==1;
        token = tkWS;
        break;
      }
      case '[': {   /* Microsoft-style identifiers in [...] */
        zSql++;
        while( *zSql && *zSql!=']' ){ zSql++; }
        if( *zSql==0 ) return 0;
        token = tkOTHER;
        break;
      }
      case '`':     /* Grave-accent quoted symbols used by MySQL */
      case '"':     /* single- and double-quoted strings */
      case '\'': {
        int c = *zSql;
        zSql++;
        while( *zSql && *zSql!=c ){ zSql++; }
        if( *zSql==0 ) return 0;
        token = tkOTHER;
        break;
      }
      default: {
#ifdef SQLITE_EBCDIC
        unsigned char c;
#endif
        if( IdChar((u8)*zSql) ){
          /* Keywords and unquoted identifiers */
          int nId;
          for(nId=1; IdChar(zSql[nId]); nId++){}
#ifdef SQLITE_OMIT_TRIGGER
          token = tkOTHER;
#else
          switch( *zSql ){
            case 'c': case 'C': {
              if( nId==6 && sqlite3StrNICmp(zSql, "create", 6)==0 ){
                token = tkCREATE;
              }else{
                token = tkOTHER;
              }
              break;
            }
            case 't': case 'T': {
              if( nId==7 && sqlite3StrNICmp(zSql, "trigger", 7)==0 ){
                token = tkTRIGGER;
              }else if( nId==4 && sqlite3StrNICmp(zSql, "temp", 4)==0 ){
                token = tkTEMP;
              }else if( nId==9 && sqlite3StrNICmp(zSql, "temporary", 9)==0 ){
                token = tkTEMP;
              }else{
                token = tkOTHER;
              }
              break;
            }
            case 'e':  case 'E': {
              if( nId==3 && sqlite3StrNICmp(zSql, "end", 3)==0 ){
                token = tkEND;
              }else
#ifndef SQLITE_OMIT_EXPLAIN
              if( nId==7 && sqlite3StrNICmp(zSql, "explain", 7)==0 ){
                token = tkEXPLAIN;
              }else
#endif
              {
                token = tkOTHER;
              }
              break;
            }
            default: {
              token = tkOTHER;
              break;
            }
          }
#endif /* SQLITE_OMIT_TRIGGER */
          zSql += nId-1;
        }else{
          /* Operators and special symbols */
          token = tkOTHER;
        }
        break;
      }
    }
    state = trans[state][token];
    zSql++;
  }
  return state==1;
}

#ifndef SQLITE_OMIT_UTF16
/*
** This routine is the same as the sqlite3_complete() routine described
** above, except that the parameter is required to be UTF-16 encoded, not
** UTF-8.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_complete16(const void *zSql){
  sqlite3_value *pVal;
  char const *zSql8;
  int rc;

#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  if( rc ) return rc;
#endif
  pVal = sqlite3ValueNew(0);
  sqlite3ValueSetStr(pVal, -1, zSql, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zSql8 = sqlite3ValueText(pVal, SQLITE_UTF8);
  if( zSql8 ){
    rc = sqlite3_complete(zSql8);
  }else{
    rc = SQLITE_NOMEM;
  }
  sqlite3ValueFree(pVal);
  return rc & 0xff;
}
#endif /* SQLITE_OMIT_UTF16 */
#endif /* SQLITE_OMIT_COMPLETE */

/************** End of complete.c ********************************************/
/************** Begin file main.c ********************************************/
/*
** 2001 September 15
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Main file for the SQLite library.  The routines in this file
** implement the programmer interface to the library.  Routines in
** other files are for internal use by SQLite and should not be
** accessed by users of the library.
*/
/* #include "sqliteInt.h" */

#ifdef SQLITE_ENABLE_FTS3
/************** Include fts3.h in the middle of main.c ***********************/
/************** Begin file fts3.h ********************************************/
/*
** 2006 Oct 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This header file is used by programs that want to link against the
** FTS3 library.  All it does is declare the sqlite3Fts3Init() interface.
*/
/* #include "sqlite3.h" */

#if 0
extern "C" {
#endif  /* __cplusplus */

SQLITE_PRIVATE int sqlite3Fts3Init(sqlite3 *db);

#if 0
}  /* extern "C" */
#endif  /* __cplusplus */

/************** End of fts3.h ************************************************/
/************** Continuing where we left off in main.c ***********************/
#endif
#ifdef SQLITE_ENABLE_RTREE
/************** Include rtree.h in the middle of main.c **********************/
/************** Begin file rtree.h *******************************************/
/*
** 2008 May 26
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This header file is used by programs that want to link against the
** RTREE library.  All it does is declare the sqlite3RtreeInit() interface.
*/
/* #include "sqlite3.h" */

#if 0
extern "C" {
#endif  /* __cplusplus */

SQLITE_PRIVATE int sqlite3RtreeInit(sqlite3 *db);

#if 0
}  /* extern "C" */
#endif  /* __cplusplus */

/************** End of rtree.h ***********************************************/
/************** Continuing where we left off in main.c ***********************/
#endif
#ifdef SQLITE_ENABLE_ICU
/************** Include sqliteicu.h in the middle of main.c ******************/
/************** Begin file sqliteicu.h ***************************************/
/*
** 2008 May 26
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This header file is used by programs that want to link against the
** ICU extension.  All it does is declare the sqlite3IcuInit() interface.
*/
/* #include "sqlite3.h" */

#if 0
extern "C" {
#endif  /* __cplusplus */

SQLITE_PRIVATE int sqlite3IcuInit(sqlite3 *db);

#if 0
}  /* extern "C" */
#endif  /* __cplusplus */


/************** End of sqliteicu.h *******************************************/
/************** Continuing where we left off in main.c ***********************/
#endif

#ifndef SQLITE_AMALGAMATION
/* IMPLEMENTATION-OF: R-46656-45156 The sqlite3_version[] string constant
** contains the text of SQLITE_VERSION macro. 
*/
SQLITE_API const char sqlite3_version[] = SQLITE_VERSION;
#endif

/* IMPLEMENTATION-OF: R-53536-42575 The sqlite3_libversion() function returns
** a pointer to the to the sqlite3_version[] string constant. 
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_libversion(void){ return sqlite3_version; }

/* IMPLEMENTATION-OF: R-63124-39300 The sqlite3_sourceid() function returns a
** pointer to a string constant whose value is the same as the
** SQLITE_SOURCE_ID C preprocessor macro. 
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_sourceid(void){ return SQLITE_SOURCE_ID; }

/* IMPLEMENTATION-OF: R-35210-63508 The sqlite3_libversion_number() function
** returns an integer equal to SQLITE_VERSION_NUMBER.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_libversion_number(void){ return SQLITE_VERSION_NUMBER; }

/* IMPLEMENTATION-OF: R-20790-14025 The sqlite3_threadsafe() function returns
** zero if and only if SQLite was compiled with mutexing code omitted due to
** the SQLITE_THREADSAFE compile-time option being set to 0.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_threadsafe(void){ return SQLITE_THREADSAFE; }

/*
** When compiling the test fixture or with debugging enabled (on Win32),
** this variable being set to non-zero will cause OSTRACE macros to emit
** extra diagnostic information.
*/
#ifdef SQLITE_HAVE_OS_TRACE
# ifndef SQLITE_DEBUG_OS_TRACE
#   define SQLITE_DEBUG_OS_TRACE 0
# endif
  int sqlite3OSTrace = SQLITE_DEBUG_OS_TRACE;
#endif

#if !defined(SQLITE_OMIT_TRACE) && defined(SQLITE_ENABLE_IOTRACE)
/*
** If the following function pointer is not NULL and if
** SQLITE_ENABLE_IOTRACE is enabled, then messages describing
** I/O active are written using this function.  These messages
** are intended for debugging activity only.
*/
SQLITE_API void (SQLITE_CDECL *sqlite3IoTrace)(const char*, ...) = 0;
#endif

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** temporary files.
**
** See also the "PRAGMA temp_store_directory" SQL command.
*/
SQLITE_API char *sqlite3_temp_directory = 0;

/*
** If the following global variable points to a string which is the
** name of a directory, then that directory will be used to store
** all database files specified with a relative pathname.
**
** See also the "PRAGMA data_store_directory" SQL command.
*/
SQLITE_API char *sqlite3_data_directory = 0;

/*
** Initialize SQLite.  
**
** This routine must be called to initialize the memory allocation,
** VFS, and mutex subsystems prior to doing any serious work with
** SQLite.  But as long as you do not compile with SQLITE_OMIT_AUTOINIT
** this routine will be called automatically by key routines such as
** sqlite3_open().  
**
** This routine is a no-op except on its very first call for the process,
** or for the first call after a call to sqlite3_shutdown.
**
** The first thread to call this routine runs the initialization to
** completion.  If subsequent threads call this routine before the first
** thread has finished the initialization process, then the subsequent
** threads must block until the first thread finishes with the initialization.
**
** The first thread might call this routine recursively.  Recursive
** calls to this routine should not block, of course.  Otherwise the
** initialization process would never complete.
**
** Let X be the first thread to enter this routine.  Let Y be some other
** thread.  Then while the initial invocation of this routine by X is
** incomplete, it is required that:
**
**    *  Calls to this routine from Y must block until the outer-most
**       call by X completes.
**
**    *  Recursive calls to this routine from thread X return immediately
**       without blocking.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_initialize(void){
  MUTEX_LOGIC( sqlite3_mutex *pMaster; )       /* The main static mutex */
  int rc;                                      /* Result code */
#ifdef SQLITE_EXTRA_INIT
  int bRunExtraInit = 0;                       /* Extra initialization needed */
#endif

#ifdef SQLITE_OMIT_WSD
  rc = sqlite3_wsd_init(4096, 24);
  if( rc!=SQLITE_OK ){
    return rc;
  }
#endif

  /* If the following assert() fails on some obscure processor/compiler
  ** combination, the work-around is to set the correct pointer
  ** size at compile-time using -DSQLITE_PTRSIZE=n compile-time option */
  assert( SQLITE_PTRSIZE==sizeof(char*) );

  /* If SQLite is already completely initialized, then this call
  ** to sqlite3_initialize() should be a no-op.  But the initialization
  ** must be complete.  So isInit must not be set until the very end
  ** of this routine.
  */
  if( sqlite3GlobalConfig.isInit ) return SQLITE_OK;

  /* Make sure the mutex subsystem is initialized.  If unable to 
  ** initialize the mutex subsystem, return early with the error.
  ** If the system is so sick that we are unable to allocate a mutex,
  ** there is not much SQLite is going to be able to do.
  **
  ** The mutex subsystem must take care of serializing its own
  ** initialization.
  */
  rc = sqlite3MutexInit();
  if( rc ) return rc;

  /* Initialize the malloc() system and the recursive pInitMutex mutex.
  ** This operation is protected by the STATIC_MASTER mutex.  Note that
  ** MutexAlloc() is called for a static mutex prior to initializing the
  ** malloc subsystem - this implies that the allocation of a static
  ** mutex must not require support from the malloc subsystem.
  */
  MUTEX_LOGIC( pMaster = sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER); )
  sqlite3_mutex_enter(pMaster);
  sqlite3GlobalConfig.isMutexInit = 1;
  if( !sqlite3GlobalConfig.isMallocInit ){
    rc = sqlite3MallocInit();
  }
  if( rc==SQLITE_OK ){
    sqlite3GlobalConfig.isMallocInit = 1;
    if( !sqlite3GlobalConfig.pInitMutex ){
      sqlite3GlobalConfig.pInitMutex =
           sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
      if( sqlite3GlobalConfig.bCoreMutex && !sqlite3GlobalConfig.pInitMutex ){
        rc = SQLITE_NOMEM;
      }
    }
  }
  if( rc==SQLITE_OK ){
    sqlite3GlobalConfig.nRefInitMutex++;
  }
  sqlite3_mutex_leave(pMaster);

  /* If rc is not SQLITE_OK at this point, then either the malloc
  ** subsystem could not be initialized or the system failed to allocate
  ** the pInitMutex mutex. Return an error in either case.  */
  if( rc!=SQLITE_OK ){
    return rc;
  }

  /* Do the rest of the initialization under the recursive mutex so
  ** that we will be able to handle recursive calls into
  ** sqlite3_initialize().  The recursive calls normally come through
  ** sqlite3_os_init() when it invokes sqlite3_vfs_register(), but other
  ** recursive calls might also be possible.
  **
  ** IMPLEMENTATION-OF: R-00140-37445 SQLite automatically serializes calls
  ** to the xInit method, so the xInit method need not be threadsafe.
  **
  ** The following mutex is what serializes access to the appdef pcache xInit
  ** methods.  The sqlite3_pcache_methods.xInit() all is embedded in the
  ** call to sqlite3PcacheInitialize().
  */
  sqlite3_mutex_enter(sqlite3GlobalConfig.pInitMutex);
  if( sqlite3GlobalConfig.isInit==0 && sqlite3GlobalConfig.inProgress==0 ){
    FuncDefHash *pHash = &GLOBAL(FuncDefHash, sqlite3GlobalFunctions);
    sqlite3GlobalConfig.inProgress = 1;
    memset(pHash, 0, sizeof(sqlite3GlobalFunctions));
    sqlite3RegisterGlobalFunctions();
    if( sqlite3GlobalConfig.isPCacheInit==0 ){
      rc = sqlite3PcacheInitialize();
    }
    if( rc==SQLITE_OK ){
      sqlite3GlobalConfig.isPCacheInit = 1;
      rc = sqlite3OsInit();
    }
    if( rc==SQLITE_OK ){
      sqlite3PCacheBufferSetup( sqlite3GlobalConfig.pPage, 
          sqlite3GlobalConfig.szPage, sqlite3GlobalConfig.nPage);
      sqlite3GlobalConfig.isInit = 1;
#ifdef SQLITE_EXTRA_INIT
      bRunExtraInit = 1;
#endif
    }
    sqlite3GlobalConfig.inProgress = 0;
  }
  sqlite3_mutex_leave(sqlite3GlobalConfig.pInitMutex);

  /* Go back under the static mutex and clean up the recursive
  ** mutex to prevent a resource leak.
  */
  sqlite3_mutex_enter(pMaster);
  sqlite3GlobalConfig.nRefInitMutex--;
  if( sqlite3GlobalConfig.nRefInitMutex<=0 ){
    assert( sqlite3GlobalConfig.nRefInitMutex==0 );
    sqlite3_mutex_free(sqlite3GlobalConfig.pInitMutex);
    sqlite3GlobalConfig.pInitMutex = 0;
  }
  sqlite3_mutex_leave(pMaster);

  /* The following is just a sanity check to make sure SQLite has
  ** been compiled correctly.  It is important to run this code, but
  ** we don't want to run it too often and soak up CPU cycles for no
  ** reason.  So we run it once during initialization.
  */
#ifndef NDEBUG
#ifndef SQLITE_OMIT_FLOATING_POINT
  /* This section of code's only "output" is via assert() statements. */
  if ( rc==SQLITE_OK ){
    u64 x = (((u64)1)<<63)-1;
    double y;
    assert(sizeof(x)==8);
    assert(sizeof(x)==sizeof(y));
    memcpy(&y, &x, 8);
    assert( sqlite3IsNaN(y) );
  }
#endif
#endif

  /* Do extra initialization steps requested by the SQLITE_EXTRA_INIT
  ** compile-time option.
  */
#ifdef SQLITE_EXTRA_INIT
  if( bRunExtraInit ){
    int SQLITE_EXTRA_INIT(const char*);
    rc = SQLITE_EXTRA_INIT(0);
  }
#endif

  return rc;
}

/*
** Undo the effects of sqlite3_initialize().  Must not be called while
** there are outstanding database connections or memory allocations or
** while any part of SQLite is otherwise in use in any thread.  This
** routine is not threadsafe.  But it is safe to invoke this routine
** on when SQLite is already shut down.  If SQLite is already shut down
** when this routine is invoked, then this routine is a harmless no-op.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_shutdown(void){
#ifdef SQLITE_OMIT_WSD
  int rc = sqlite3_wsd_init(4096, 24);
  if( rc!=SQLITE_OK ){
    return rc;
  }
#endif

  if( sqlite3GlobalConfig.isInit ){
#ifdef SQLITE_EXTRA_SHUTDOWN
    void SQLITE_EXTRA_SHUTDOWN(void);
    SQLITE_EXTRA_SHUTDOWN();
#endif
    sqlite3_os_end();
    sqlite3_reset_auto_extension();
    sqlite3GlobalConfig.isInit = 0;
  }
  if( sqlite3GlobalConfig.isPCacheInit ){
    sqlite3PcacheShutdown();
    sqlite3GlobalConfig.isPCacheInit = 0;
  }
  if( sqlite3GlobalConfig.isMallocInit ){
    sqlite3MallocEnd();
    sqlite3GlobalConfig.isMallocInit = 0;

#ifndef SQLITE_OMIT_SHUTDOWN_DIRECTORIES
    /* The heap subsystem has now been shutdown and these values are supposed
    ** to be NULL or point to memory that was obtained from sqlite3_malloc(),
    ** which would rely on that heap subsystem; therefore, make sure these
    ** values cannot refer to heap memory that was just invalidated when the
    ** heap subsystem was shutdown.  This is only done if the current call to
    ** this function resulted in the heap subsystem actually being shutdown.
    */
    sqlite3_data_directory = 0;
    sqlite3_temp_directory = 0;
#endif
  }
  if( sqlite3GlobalConfig.isMutexInit ){
    sqlite3MutexEnd();
    sqlite3GlobalConfig.isMutexInit = 0;
  }

  return SQLITE_OK;
}

/*
** This API allows applications to modify the global configuration of
** the SQLite library at run-time.
**
** This routine should only be called when there are no outstanding
** database connections or memory allocations.  This routine is not
** threadsafe.  Failure to heed these warnings can lead to unpredictable
** behavior.
*/
SQLITE_API int SQLITE_CDECL sqlite3_config(int op, ...){
  va_list ap;
  int rc = SQLITE_OK;

  /* sqlite3_config() shall return SQLITE_MISUSE if it is invoked while
  ** the SQLite library is in use. */
  if( sqlite3GlobalConfig.isInit ) return SQLITE_MISUSE_BKPT;

  va_start(ap, op);
  switch( op ){

    /* Mutex configuration options are only available in a threadsafe
    ** compile.
    */
#if defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0  /* IMP: R-54466-46756 */
    case SQLITE_CONFIG_SINGLETHREAD: {
      /* EVIDENCE-OF: R-02748-19096 This option sets the threading mode to
      ** Single-thread. */
      sqlite3GlobalConfig.bCoreMutex = 0;  /* Disable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 0;  /* Disable mutex on connections */
      break;
    }
#endif
#if defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-20520-54086 */
    case SQLITE_CONFIG_MULTITHREAD: {
      /* EVIDENCE-OF: R-14374-42468 This option sets the threading mode to
      ** Multi-thread. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 0;  /* Disable mutex on connections */
      break;
    }
#endif
#if defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-59593-21810 */
    case SQLITE_CONFIG_SERIALIZED: {
      /* EVIDENCE-OF: R-41220-51800 This option sets the threading mode to
      ** Serialized. */
      sqlite3GlobalConfig.bCoreMutex = 1;  /* Enable mutex on core */
      sqlite3GlobalConfig.bFullMutex = 1;  /* Enable mutex on connections */
      break;
    }
#endif
#if defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-63666-48755 */
    case SQLITE_CONFIG_MUTEX: {
      /* Specify an alternative mutex implementation */
      sqlite3GlobalConfig.mutex = *va_arg(ap, sqlite3_mutex_methods*);
      break;
    }
#endif
#if defined(SQLITE_THREADSAFE) && SQLITE_THREADSAFE>0 /* IMP: R-14450-37597 */
    case SQLITE_CONFIG_GETMUTEX: {
      /* Retrieve the current mutex implementation */
      *va_arg(ap, sqlite3_mutex_methods*) = sqlite3GlobalConfig.mutex;
      break;
    }
#endif

    case SQLITE_CONFIG_MALLOC: {
      /* EVIDENCE-OF: R-55594-21030 The SQLITE_CONFIG_MALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The argument specifies alternative
      ** low-level memory allocation routines to be used in place of the memory
      ** allocation routines built into SQLite. */
      sqlite3GlobalConfig.m = *va_arg(ap, sqlite3_mem_methods*);
      break;
    }
    case SQLITE_CONFIG_GETMALLOC: {
      /* EVIDENCE-OF: R-51213-46414 The SQLITE_CONFIG_GETMALLOC option takes a
      ** single argument which is a pointer to an instance of the
      ** sqlite3_mem_methods structure. The sqlite3_mem_methods structure is
      ** filled with the currently defined memory allocation routines. */
      if( sqlite3GlobalConfig.m.xMalloc==0 ) sqlite3MemSetDefault();
      *va_arg(ap, sqlite3_mem_methods*) = sqlite3GlobalConfig.m;
      break;
    }
    case SQLITE_CONFIG_MEMSTATUS: {
      /* EVIDENCE-OF: R-61275-35157 The SQLITE_CONFIG_MEMSTATUS option takes
      ** single argument of type int, interpreted as a boolean, which enables
      ** or disables the collection of memory allocation statistics. */
      sqlite3GlobalConfig.bMemstat = va_arg(ap, int);
      break;
    }
    case SQLITE_CONFIG_SCRATCH: {
      /* EVIDENCE-OF: R-08404-60887 There are three arguments to
      ** SQLITE_CONFIG_SCRATCH: A pointer an 8-byte aligned memory buffer from
      ** which the scratch allocations will be drawn, the size of each scratch
      ** allocation (sz), and the maximum number of scratch allocations (N). */
      sqlite3GlobalConfig.pScratch = va_arg(ap, void*);
      sqlite3GlobalConfig.szScratch = va_arg(ap, int);
      sqlite3GlobalConfig.nScratch = va_arg(ap, int);
      break;
    }
    case SQLITE_CONFIG_PAGECACHE: {
      /* EVIDENCE-OF: R-31408-40510 There are three arguments to
      ** SQLITE_CONFIG_PAGECACHE: A pointer to 8-byte aligned memory, the size
      ** of each page buffer (sz), and the number of pages (N). */
      sqlite3GlobalConfig.pPage = va_arg(ap, void*);
      sqlite3GlobalConfig.szPage = va_arg(ap, int);
      sqlite3GlobalConfig.nPage = va_arg(ap, int);
      break;
    }
    case SQLITE_CONFIG_PCACHE_HDRSZ: {
      /* EVIDENCE-OF: R-39100-27317 The SQLITE_CONFIG_PCACHE_HDRSZ option takes
      ** a single parameter which is a pointer to an integer and writes into
      ** that integer the number of extra bytes per page required for each page
      ** in SQLITE_CONFIG_PAGECACHE. */
      *va_arg(ap, int*) = 
          sqlite3HeaderSizeBtree() +
          sqlite3HeaderSizePcache() +
          sqlite3HeaderSizePcache1();
      break;
    }

    case SQLITE_CONFIG_PCACHE: {
      /* no-op */
      break;
    }
    case SQLITE_CONFIG_GETPCACHE: {
      /* now an error */
      rc = SQLITE_ERROR;
      break;
    }

    case SQLITE_CONFIG_PCACHE2: {
      /* EVIDENCE-OF: R-63325-48378 The SQLITE_CONFIG_PCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. This object specifies the interface to a custom page cache
      ** implementation. */
      sqlite3GlobalConfig.pcache2 = *va_arg(ap, sqlite3_pcache_methods2*);
      break;
    }
    case SQLITE_CONFIG_GETPCACHE2: {
      /* EVIDENCE-OF: R-22035-46182 The SQLITE_CONFIG_GETPCACHE2 option takes a
      ** single argument which is a pointer to an sqlite3_pcache_methods2
      ** object. SQLite copies of the current page cache implementation into
      ** that object. */
      if( sqlite3GlobalConfig.pcache2.xInit==0 ){
        sqlite3PCacheSetDefault();
      }
      *va_arg(ap, sqlite3_pcache_methods2*) = sqlite3GlobalConfig.pcache2;
      break;
    }

/* EVIDENCE-OF: R-06626-12911 The SQLITE_CONFIG_HEAP option is only
** available if SQLite is compiled with either SQLITE_ENABLE_MEMSYS3 or
** SQLITE_ENABLE_MEMSYS5 and returns SQLITE_ERROR if invoked otherwise. */
#if defined(SQLITE_ENABLE_MEMSYS3) || defined(SQLITE_ENABLE_MEMSYS5)
    case SQLITE_CONFIG_HEAP: {
      /* EVIDENCE-OF: R-19854-42126 There are three arguments to
      ** SQLITE_CONFIG_HEAP: An 8-byte aligned pointer to the memory, the
      ** number of bytes in the memory buffer, and the minimum allocation size.
      */
      sqlite3GlobalConfig.pHeap = va_arg(ap, void*);
      sqlite3GlobalConfig.nHeap = va_arg(ap, int);
      sqlite3GlobalConfig.mnReq = va_arg(ap, int);

      if( sqlite3GlobalConfig.mnReq<1 ){
        sqlite3GlobalConfig.mnReq = 1;
      }else if( sqlite3GlobalConfig.mnReq>(1<<12) ){
        /* cap min request size at 2^12 */
        sqlite3GlobalConfig.mnReq = (1<<12);
      }

      if( sqlite3GlobalConfig.pHeap==0 ){
        /* EVIDENCE-OF: R-49920-60189 If the first pointer (the memory pointer)
        ** is NULL, then SQLite reverts to using its default memory allocator
        ** (the system malloc() implementation), undoing any prior invocation of
        ** SQLITE_CONFIG_MALLOC.
        **
        ** Setting sqlite3GlobalConfig.m to all zeros will cause malloc to
        ** revert to its default implementation when sqlite3_initialize() is run
        */
        memset(&sqlite3GlobalConfig.m, 0, sizeof(sqlite3GlobalConfig.m));
      }else{
        /* EVIDENCE-OF: R-61006-08918 If the memory pointer is not NULL then the
        ** alternative memory allocator is engaged to handle all of SQLites
        ** memory allocation needs. */
#ifdef SQLITE_ENABLE_MEMSYS3
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys3();
#endif
#ifdef SQLITE_ENABLE_MEMSYS5
        sqlite3GlobalConfig.m = *sqlite3MemGetMemsys5();
#endif
      }
      break;
    }
#endif

    case SQLITE_CONFIG_LOOKASIDE: {
      sqlite3GlobalConfig.szLookaside = va_arg(ap, int);
      sqlite3GlobalConfig.nLookaside = va_arg(ap, int);
      break;
    }
    
    /* Record a pointer to the logger function and its first argument.
    ** The default is NULL.  Logging is disabled if the function pointer is
    ** NULL.
    */
    case SQLITE_CONFIG_LOG: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xLog = va_arg(ap, void(*)(void*,int,const char*));
      */
      typedef void(*LOGFUNC_t)(void*,int,const char*);
      sqlite3GlobalConfig.xLog = va_arg(ap, LOGFUNC_t);
      sqlite3GlobalConfig.pLogArg = va_arg(ap, void*);
      break;
    }

    /* EVIDENCE-OF: R-55548-33817 The compile-time setting for URI filenames
    ** can be changed at start-time using the
    ** sqlite3_config(SQLITE_CONFIG_URI,1) or
    ** sqlite3_config(SQLITE_CONFIG_URI,0) configuration calls.
    */
    case SQLITE_CONFIG_URI: {
      /* EVIDENCE-OF: R-25451-61125 The SQLITE_CONFIG_URI option takes a single
      ** argument of type int. If non-zero, then URI handling is globally
      ** enabled. If the parameter is zero, then URI handling is globally
      ** disabled. */
      sqlite3GlobalConfig.bOpenUri = va_arg(ap, int);
      break;
    }

    case SQLITE_CONFIG_COVERING_INDEX_SCAN: {
      /* EVIDENCE-OF: R-36592-02772 The SQLITE_CONFIG_COVERING_INDEX_SCAN
      ** option takes a single integer argument which is interpreted as a
      ** boolean in order to enable or disable the use of covering indices for
      ** full table scans in the query optimizer. */
      sqlite3GlobalConfig.bUseCis = va_arg(ap, int);
      break;
    }

#ifdef SQLITE_ENABLE_SQLLOG
    case SQLITE_CONFIG_SQLLOG: {
      typedef void(*SQLLOGFUNC_t)(void*, sqlite3*, const char*, int);
      sqlite3GlobalConfig.xSqllog = va_arg(ap, SQLLOGFUNC_t);
      sqlite3GlobalConfig.pSqllogArg = va_arg(ap, void *);
      break;
    }
#endif

    case SQLITE_CONFIG_MMAP_SIZE: {
      /* EVIDENCE-OF: R-58063-38258 SQLITE_CONFIG_MMAP_SIZE takes two 64-bit
      ** integer (sqlite3_int64) values that are the default mmap size limit
      ** (the default setting for PRAGMA mmap_size) and the maximum allowed
      ** mmap size limit. */
      sqlite3_int64 szMmap = va_arg(ap, sqlite3_int64);
      sqlite3_int64 mxMmap = va_arg(ap, sqlite3_int64);
      /* EVIDENCE-OF: R-53367-43190 If either argument to this option is
      ** negative, then that argument is changed to its compile-time default.
      **
      ** EVIDENCE-OF: R-34993-45031 The maximum allowed mmap size will be
      ** silently truncated if necessary so that it does not exceed the
      ** compile-time maximum mmap size set by the SQLITE_MAX_MMAP_SIZE
      ** compile-time option.
      */
      if( mxMmap<0 || mxMmap>SQLITE_MAX_MMAP_SIZE ){
        mxMmap = SQLITE_MAX_MMAP_SIZE;
      }
      if( szMmap<0 ) szMmap = SQLITE_DEFAULT_MMAP_SIZE;
      if( szMmap>mxMmap) szMmap = mxMmap;
      sqlite3GlobalConfig.mxMmap = mxMmap;
      sqlite3GlobalConfig.szMmap = szMmap;
      break;
    }

#if SQLITE_OS_WIN && defined(SQLITE_WIN32_MALLOC) /* IMP: R-04780-55815 */
    case SQLITE_CONFIG_WIN32_HEAPSIZE: {
      /* EVIDENCE-OF: R-34926-03360 SQLITE_CONFIG_WIN32_HEAPSIZE takes a 32-bit
      ** unsigned integer value that specifies the maximum size of the created
      ** heap. */
      sqlite3GlobalConfig.nHeap = va_arg(ap, int);
      break;
    }
#endif

    case SQLITE_CONFIG_PMASZ: {
      sqlite3GlobalConfig.szPma = va_arg(ap, unsigned int);
      break;
    }

    default: {
      rc = SQLITE_ERROR;
      break;
    }
  }
  va_end(ap);
  return rc;
}

/*
** Set up the lookaside buffers for a database connection.
** Return SQLITE_OK on success.  
** If lookaside is already active, return SQLITE_BUSY.
**
** The sz parameter is the number of bytes in each lookaside slot.
** The cnt parameter is the number of slots.  If pStart is NULL the
** space for the lookaside memory is obtained from sqlite3_malloc().
** If pStart is not NULL then it is sz*cnt bytes of memory to use for
** the lookaside memory.
*/
static int setupLookaside(sqlite3 *db, void *pBuf, int sz, int cnt){
#ifndef SQLITE_OMIT_LOOKASIDE
  void *pStart;
  if( db->lookaside.nOut ){
    return SQLITE_BUSY;
  }
  /* Free any existing lookaside buffer for this handle before
  ** allocating a new one so we don't have to have space for 
  ** both at the same time.
  */
  if( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.pStart);
  }
  /* The size of a lookaside slot after ROUNDDOWN8 needs to be larger
  ** than a pointer to be useful.
  */
  sz = ROUNDDOWN8(sz);  /* IMP: R-33038-09382 */
  if( sz<=(int)sizeof(LookasideSlot*) ) sz = 0;
  if( cnt<0 ) cnt = 0;
  if( sz==0 || cnt==0 ){
    sz = 0;
    pStart = 0;
  }else if( pBuf==0 ){
    sqlite3BeginBenignMalloc();
    pStart = sqlite3Malloc( sz*cnt );  /* IMP: R-61949-35727 */
    sqlite3EndBenignMalloc();
    if( pStart ) cnt = sqlite3MallocSize(pStart)/sz;
  }else{
    pStart = pBuf;
  }
  db->lookaside.pStart = pStart;
  db->lookaside.pFree = 0;
  db->lookaside.sz = (u16)sz;
  if( pStart ){
    int i;
    LookasideSlot *p;
    assert( sz > (int)sizeof(LookasideSlot*) );
    p = (LookasideSlot*)pStart;
    for(i=cnt-1; i>=0; i--){
      p->pNext = db->lookaside.pFree;
      db->lookaside.pFree = p;
      p = (LookasideSlot*)&((u8*)p)[sz];
    }
    db->lookaside.pEnd = p;
    db->lookaside.bEnabled = 1;
    db->lookaside.bMalloced = pBuf==0 ?1:0;
  }else{
    db->lookaside.pStart = db;
    db->lookaside.pEnd = db;
    db->lookaside.bEnabled = 0;
    db->lookaside.bMalloced = 0;
  }
#endif /* SQLITE_OMIT_LOOKASIDE */
  return SQLITE_OK;
}

/*
** Return the mutex associated with a database connection.
*/
SQLITE_API sqlite3_mutex *SQLITE_STDCALL sqlite3_db_mutex(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  return db->mutex;
}

/*
** Free up as much memory as we can from the given database
** connection.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_release_memory(sqlite3 *db){
  int i;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  for(i=0; i<db->nDb; i++){
    Btree *pBt = db->aDb[i].pBt;
    if( pBt ){
      Pager *pPager = sqlite3BtreePager(pBt);
      sqlite3PagerShrink(pPager);
    }
  }
  sqlite3BtreeLeaveAll(db);
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

/*
** Configuration settings for an individual database connection
*/
SQLITE_API int SQLITE_CDECL sqlite3_db_config(sqlite3 *db, int op, ...){
  va_list ap;
  int rc;
  va_start(ap, op);
  switch( op ){
    case SQLITE_DBCONFIG_LOOKASIDE: {
      void *pBuf = va_arg(ap, void*); /* IMP: R-26835-10964 */
      int sz = va_arg(ap, int);       /* IMP: R-47871-25994 */
      int cnt = va_arg(ap, int);      /* IMP: R-04460-53386 */
      rc = setupLookaside(db, pBuf, sz, cnt);
      break;
    }
    default: {
      static const struct {
        int op;      /* The opcode */
        u32 mask;    /* Mask of the bit in sqlite3.flags to set/clear */
      } aFlagOp[] = {
        { SQLITE_DBCONFIG_ENABLE_FKEY,    SQLITE_ForeignKeys    },
        { SQLITE_DBCONFIG_ENABLE_TRIGGER, SQLITE_EnableTrigger  },
      };
      unsigned int i;
      rc = SQLITE_ERROR; /* IMP: R-42790-23372 */
      for(i=0; i<ArraySize(aFlagOp); i++){
        if( aFlagOp[i].op==op ){
          int onoff = va_arg(ap, int);
          int *pRes = va_arg(ap, int*);
          int oldFlags = db->flags;
          if( onoff>0 ){
            db->flags |= aFlagOp[i].mask;
          }else if( onoff==0 ){
            db->flags &= ~aFlagOp[i].mask;
          }
          if( oldFlags!=db->flags ){
            sqlite3ExpirePreparedStatements(db);
          }
          if( pRes ){
            *pRes = (db->flags & aFlagOp[i].mask)!=0;
          }
          rc = SQLITE_OK;
          break;
        }
      }
      break;
    }
  }
  va_end(ap);
  return rc;
}


/*
** Return true if the buffer z[0..n-1] contains all spaces.
*/
static int allSpaces(const char *z, int n){
  while( n>0 && z[n-1]==' ' ){ n--; }
  return n==0;
}

/*
** This is the default collating function named "BINARY" which is always
** available.
**
** If the padFlag argument is not NULL then space padding at the end
** of strings is ignored.  This implements the RTRIM collation.
*/
static int binCollFunc(
  void *padFlag,
  int nKey1, const void *pKey1,
  int nKey2, const void *pKey2
){
  int rc, n;
  n = nKey1<nKey2 ? nKey1 : nKey2;
  /* EVIDENCE-OF: R-65033-28449 The built-in BINARY collation compares
  ** strings byte by byte using the memcmp() function from the standard C
  ** library. */
  rc = memcmp(pKey1, pKey2, n);
  if( rc==0 ){
    if( padFlag
     && allSpaces(((char*)pKey1)+n, nKey1-n)
     && allSpaces(((char*)pKey2)+n, nKey2-n)
    ){
      /* EVIDENCE-OF: R-31624-24737 RTRIM is like BINARY except that extra
      ** spaces at the end of either string do not change the result. In other
      ** words, strings will compare equal to one another as long as they
      ** differ only in the number of spaces at the end.
      */
    }else{
      rc = nKey1 - nKey2;
    }
  }
  return rc;
}

/*
** Another built-in collating sequence: NOCASE. 
**
** This collating sequence is intended to be used for "case independent
** comparison". SQLite's knowledge of upper and lower case equivalents
** extends only to the 26 characters used in the English language.
**
** At the moment there is only a UTF-8 implementation.
*/
static int nocaseCollatingFunc(
  void *NotUsed,
  int nKey1, const void *pKey1,
  int nKey2, const void *pKey2
){
  int r = sqlite3StrNICmp(
      (const char *)pKey1, (const char *)pKey2, (nKey1<nKey2)?nKey1:nKey2);
  UNUSED_PARAMETER(NotUsed);
  if( 0==r ){
    r = nKey1-nKey2;
  }
  return r;
}

/*
** Return the ROWID of the most recent insert
*/
SQLITE_API sqlite_int64 SQLITE_STDCALL sqlite3_last_insert_rowid(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  return db->lastRowid;
}

/*
** Return the number of changes in the most recent call to sqlite3_exec().
*/
SQLITE_API int SQLITE_STDCALL sqlite3_changes(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  return db->nChange;
}

/*
** Return the number of changes since the database handle was opened.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_total_changes(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  return db->nTotalChange;
}

/*
** Close all open savepoints. This function only manipulates fields of the
** database handle object, it does not close any savepoints that may be open
** at the b-tree/pager level.
*/
SQLITE_PRIVATE void sqlite3CloseSavepoints(sqlite3 *db){
  while( db->pSavepoint ){
    Savepoint *pTmp = db->pSavepoint;
    db->pSavepoint = pTmp->pNext;
    sqlite3DbFree(db, pTmp);
  }
  db->nSavepoint = 0;
  db->nStatement = 0;
  db->isTransactionSavepoint = 0;
}

/*
** Invoke the destructor function associated with FuncDef p, if any. Except,
** if this is not the last copy of the function, do not invoke it. Multiple
** copies of a single function are created when create_function() is called
** with SQLITE_ANY as the encoding.
*/
static void functionDestroy(sqlite3 *db, FuncDef *p){
  FuncDestructor *pDestructor = p->pDestructor;
  if( pDestructor ){
    pDestructor->nRef--;
    if( pDestructor->nRef==0 ){
      pDestructor->xDestroy(pDestructor->pUserData);
      sqlite3DbFree(db, pDestructor);
    }
  }
}

/*
** Disconnect all sqlite3_vtab objects that belong to database connection
** db. This is called when db is being closed.
*/
static void disconnectAllVtab(sqlite3 *db){
#ifndef SQLITE_OMIT_VIRTUALTABLE
  int i;
  sqlite3BtreeEnterAll(db);
  for(i=0; i<db->nDb; i++){
    Schema *pSchema = db->aDb[i].pSchema;
    if( db->aDb[i].pSchema ){
      HashElem *p;
      for(p=sqliteHashFirst(&pSchema->tblHash); p; p=sqliteHashNext(p)){
        Table *pTab = (Table *)sqliteHashData(p);
        if( IsVirtual(pTab) ) sqlite3VtabDisconnect(db, pTab);
      }
    }
  }
  sqlite3VtabUnlockList(db);
  sqlite3BtreeLeaveAll(db);
#else
  UNUSED_PARAMETER(db);
#endif
}

/*
** Return TRUE if database connection db has unfinalized prepared
** statements or unfinished sqlite3_backup objects.  
*/
static int connectionIsBusy(sqlite3 *db){
  int j;
  assert( sqlite3_mutex_held(db->mutex) );
  if( db->pVdbe ) return 1;
  for(j=0; j<db->nDb; j++){
    Btree *pBt = db->aDb[j].pBt;
    if( pBt && sqlite3BtreeIsInBackup(pBt) ) return 1;
  }
  return 0;
}

/*
** Close an existing SQLite database
*/
static int sqlite3Close(sqlite3 *db, int forceZombie){
  if( !db ){
    /* EVIDENCE-OF: R-63257-11740 Calling sqlite3_close() or
    ** sqlite3_close_v2() with a NULL pointer argument is a harmless no-op. */
    return SQLITE_OK;
  }
  if( !sqlite3SafetyCheckSickOrOk(db) ){
    return SQLITE_MISUSE_BKPT;
  }
  sqlite3_mutex_enter(db->mutex);

  /* Force xDisconnect calls on all virtual tables */
  disconnectAllVtab(db);

  /* If a transaction is open, the disconnectAllVtab() call above
  ** will not have called the xDisconnect() method on any virtual
  ** tables in the db->aVTrans[] array. The following sqlite3VtabRollback()
  ** call will do so. We need to do this before the check for active
  ** SQL statements below, as the v-table implementation may be storing
  ** some prepared statements internally.
  */
  sqlite3VtabRollback(db);

  /* Legacy behavior (sqlite3_close() behavior) is to return
  ** SQLITE_BUSY if the connection can not be closed immediately.
  */
  if( !forceZombie && connectionIsBusy(db) ){
    sqlite3ErrorWithMsg(db, SQLITE_BUSY, "unable to close due to unfinalized "
       "statements or unfinished backups");
    sqlite3_mutex_leave(db->mutex);
    return SQLITE_BUSY;
  }

#ifdef SQLITE_ENABLE_SQLLOG
  if( sqlite3GlobalConfig.xSqllog ){
    /* Closing the handle. Fourth parameter is passed the value 2. */
    sqlite3GlobalConfig.xSqllog(sqlite3GlobalConfig.pSqllogArg, db, 0, 2);
  }
#endif

  /* Convert the connection into a zombie and then close it.
  */
  db->magic = SQLITE_MAGIC_ZOMBIE;
  sqlite3LeaveMutexAndCloseZombie(db);
  return SQLITE_OK;
}

/*
** Two variations on the public interface for closing a database
** connection. The sqlite3_close() version returns SQLITE_BUSY and
** leaves the connection option if there are unfinalized prepared
** statements or unfinished sqlite3_backups.  The sqlite3_close_v2()
** version forces the connection to become a zombie if there are
** unclosed resources, and arranges for deallocation when the last
** prepare statement or sqlite3_backup closes.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_close(sqlite3 *db){ return sqlite3Close(db,0); }
SQLITE_API int SQLITE_STDCALL sqlite3_close_v2(sqlite3 *db){ return sqlite3Close(db,1); }


/*
** Close the mutex on database connection db.
**
** Furthermore, if database connection db is a zombie (meaning that there
** has been a prior call to sqlite3_close(db) or sqlite3_close_v2(db)) and
** every sqlite3_stmt has now been finalized and every sqlite3_backup has
** finished, then free all resources.
*/
SQLITE_PRIVATE void sqlite3LeaveMutexAndCloseZombie(sqlite3 *db){
  HashElem *i;                    /* Hash table iterator */
  int j;

  /* If there are outstanding sqlite3_stmt or sqlite3_backup objects
  ** or if the connection has not yet been closed by sqlite3_close_v2(),
  ** then just leave the mutex and return.
  */
  if( db->magic!=SQLITE_MAGIC_ZOMBIE || connectionIsBusy(db) ){
    sqlite3_mutex_leave(db->mutex);
    return;
  }

  /* If we reach this point, it means that the database connection has
  ** closed all sqlite3_stmt and sqlite3_backup objects and has been
  ** passed to sqlite3_close (meaning that it is a zombie).  Therefore,
  ** go ahead and free all resources.
  */

  /* If a transaction is open, roll it back. This also ensures that if
  ** any database schemas have been modified by an uncommitted transaction
  ** they are reset. And that the required b-tree mutex is held to make
  ** the pager rollback and schema reset an atomic operation. */
  sqlite3RollbackAll(db, SQLITE_OK);

  /* Free any outstanding Savepoint structures. */
  sqlite3CloseSavepoints(db);

  /* Close all database connections */
  for(j=0; j<db->nDb; j++){
    struct Db *pDb = &db->aDb[j];
    if( pDb->pBt ){
      sqlite3BtreeClose(pDb->pBt);
      pDb->pBt = 0;
      if( j!=1 ){
        pDb->pSchema = 0;
      }
    }
  }
  /* Clear the TEMP schema separately and last */
  if( db->aDb[1].pSchema ){
    sqlite3SchemaClear(db->aDb[1].pSchema);
  }
  sqlite3VtabUnlockList(db);

  /* Free up the array of auxiliary databases */
  sqlite3CollapseDatabaseArray(db);
  assert( db->nDb<=2 );
  assert( db->aDb==db->aDbStatic );

  /* Tell the code in notify.c that the connection no longer holds any
  ** locks and does not require any further unlock-notify callbacks.
  */
  sqlite3ConnectionClosed(db);

  for(j=0; j<ArraySize(db->aFunc.a); j++){
    FuncDef *pNext, *pHash, *p;
    for(p=db->aFunc.a[j]; p; p=pHash){
      pHash = p->pHash;
      while( p ){
        functionDestroy(db, p);
        pNext = p->pNext;
        sqlite3DbFree(db, p);
        p = pNext;
      }
    }
  }
  for(i=sqliteHashFirst(&db->aCollSeq); i; i=sqliteHashNext(i)){
    CollSeq *pColl = (CollSeq *)sqliteHashData(i);
    /* Invoke any destructors registered for collation sequence user data. */
    for(j=0; j<3; j++){
      if( pColl[j].xDel ){
        pColl[j].xDel(pColl[j].pUser);
      }
    }
    sqlite3DbFree(db, pColl);
  }
  sqlite3HashClear(&db->aCollSeq);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  for(i=sqliteHashFirst(&db->aModule); i; i=sqliteHashNext(i)){
    Module *pMod = (Module *)sqliteHashData(i);
    if( pMod->xDestroy ){
      pMod->xDestroy(pMod->pAux);
    }
    sqlite3DbFree(db, pMod);
  }
  sqlite3HashClear(&db->aModule);
#endif

  sqlite3Error(db, SQLITE_OK); /* Deallocates any cached error strings. */
  sqlite3ValueFree(db->pErr);
  sqlite3CloseExtensions(db);
#if SQLITE_USER_AUTHENTICATION
  sqlite3_free(db->auth.zAuthUser);
  sqlite3_free(db->auth.zAuthPW);
#endif

  db->magic = SQLITE_MAGIC_ERROR;

  /* The temp-database schema is allocated differently from the other schema
  ** objects (using sqliteMalloc() directly, instead of sqlite3BtreeSchema()).
  ** So it needs to be freed here. Todo: Why not roll the temp schema into
  ** the same sqliteMalloc() as the one that allocates the database 
  ** structure?
  */
  sqlite3DbFree(db, db->aDb[1].pSchema);
  sqlite3_mutex_leave(db->mutex);
  db->magic = SQLITE_MAGIC_CLOSED;
  sqlite3_mutex_free(db->mutex);
  assert( db->lookaside.nOut==0 );  /* Fails on a lookaside memory leak */
  if( db->lookaside.bMalloced ){
    sqlite3_free(db->lookaside.pStart);
  }
  sqlite3_free(db);
}

/*
** Rollback all database files.  If tripCode is not SQLITE_OK, then
** any write cursors are invalidated ("tripped" - as in "tripping a circuit
** breaker") and made to return tripCode if there are any further
** attempts to use that cursor.  Read cursors remain open and valid
** but are "saved" in case the table pages are moved around.
*/
SQLITE_PRIVATE void sqlite3RollbackAll(sqlite3 *db, int tripCode){
  int i;
  int inTrans = 0;
  int schemaChange;
  assert( sqlite3_mutex_held(db->mutex) );
  sqlite3BeginBenignMalloc();

  /* Obtain all b-tree mutexes before making any calls to BtreeRollback(). 
  ** This is important in case the transaction being rolled back has
  ** modified the database schema. If the b-tree mutexes are not taken
  ** here, then another shared-cache connection might sneak in between
  ** the database rollback and schema reset, which can cause false
  ** corruption reports in some cases.  */
  sqlite3BtreeEnterAll(db);
  schemaChange = (db->flags & SQLITE_InternChanges)!=0 && db->init.busy==0;

  for(i=0; i<db->nDb; i++){
    Btree *p = db->aDb[i].pBt;
    if( p ){
      if( sqlite3BtreeIsInTrans(p) ){
        inTrans = 1;
      }
      sqlite3BtreeRollback(p, tripCode, !schemaChange);
    }
  }
  sqlite3VtabRollback(db);
  sqlite3EndBenignMalloc();

  if( (db->flags&SQLITE_InternChanges)!=0 && db->init.busy==0 ){
    sqlite3ExpirePreparedStatements(db);
    sqlite3ResetAllSchemasOfConnection(db);
  }
  sqlite3BtreeLeaveAll(db);

  /* Any deferred constraint violations have now been resolved. */
  db->nDeferredCons = 0;
  db->nDeferredImmCons = 0;
  db->flags &= ~SQLITE_DeferFKs;

  /* If one has been configured, invoke the rollback-hook callback */
  if( db->xRollbackCallback && (inTrans || !db->autoCommit) ){
    db->xRollbackCallback(db->pRollbackArg);
  }
}

/*
** Return a static string containing the name corresponding to the error code
** specified in the argument.
*/
#if defined(SQLITE_NEED_ERR_NAME)
SQLITE_PRIVATE const char *sqlite3ErrName(int rc){
  const char *zName = 0;
  int i, origRc = rc;
  for(i=0; i<2 && zName==0; i++, rc &= 0xff){
    switch( rc ){
      case SQLITE_OK:                 zName = "SQLITE_OK";                break;
      case SQLITE_ERROR:              zName = "SQLITE_ERROR";             break;
      case SQLITE_INTERNAL:           zName = "SQLITE_INTERNAL";          break;
      case SQLITE_PERM:               zName = "SQLITE_PERM";              break;
      case SQLITE_ABORT:              zName = "SQLITE_ABORT";             break;
      case SQLITE_ABORT_ROLLBACK:     zName = "SQLITE_ABORT_ROLLBACK";    break;
      case SQLITE_BUSY:               zName = "SQLITE_BUSY";              break;
      case SQLITE_BUSY_RECOVERY:      zName = "SQLITE_BUSY_RECOVERY";     break;
      case SQLITE_BUSY_SNAPSHOT:      zName = "SQLITE_BUSY_SNAPSHOT";     break;
      case SQLITE_LOCKED:             zName = "SQLITE_LOCKED";            break;
      case SQLITE_LOCKED_SHAREDCACHE: zName = "SQLITE_LOCKED_SHAREDCACHE";break;
      case SQLITE_NOMEM:              zName = "SQLITE_NOMEM";             break;
      case SQLITE_READONLY:           zName = "SQLITE_READONLY";          break;
      case SQLITE_READONLY_RECOVERY:  zName = "SQLITE_READONLY_RECOVERY"; break;
      case SQLITE_READONLY_CANTLOCK:  zName = "SQLITE_READONLY_CANTLOCK"; break;
      case SQLITE_READONLY_ROLLBACK:  zName = "SQLITE_READONLY_ROLLBACK"; break;
      case SQLITE_READONLY_DBMOVED:   zName = "SQLITE_READONLY_DBMOVED";  break;
      case SQLITE_INTERRUPT:          zName = "SQLITE_INTERRUPT";         break;
      case SQLITE_IOERR:              zName = "SQLITE_IOERR";             break;
      case SQLITE_IOERR_READ:         zName = "SQLITE_IOERR_READ";        break;
      case SQLITE_IOERR_SHORT_READ:   zName = "SQLITE_IOERR_SHORT_READ";  break;
      case SQLITE_IOERR_WRITE:        zName = "SQLITE_IOERR_WRITE";       break;
      case SQLITE_IOERR_FSYNC:        zName = "SQLITE_IOERR_FSYNC";       break;
      case SQLITE_IOERR_DIR_FSYNC:    zName = "SQLITE_IOERR_DIR_FSYNC";   break;
      case SQLITE_IOERR_TRUNCATE:     zName = "SQLITE_IOERR_TRUNCATE";    break;
      case SQLITE_IOERR_FSTAT:        zName = "SQLITE_IOERR_FSTAT";       break;
      case SQLITE_IOERR_UNLOCK:       zName = "SQLITE_IOERR_UNLOCK";      break;
      case SQLITE_IOERR_RDLOCK:       zName = "SQLITE_IOERR_RDLOCK";      break;
      case SQLITE_IOERR_DELETE:       zName = "SQLITE_IOERR_DELETE";      break;
      case SQLITE_IOERR_NOMEM:        zName = "SQLITE_IOERR_NOMEM";       break;
      case SQLITE_IOERR_ACCESS:       zName = "SQLITE_IOERR_ACCESS";      break;
      case SQLITE_IOERR_CHECKRESERVEDLOCK:
                                zName = "SQLITE_IOERR_CHECKRESERVEDLOCK"; break;
      case SQLITE_IOERR_LOCK:         zName = "SQLITE_IOERR_LOCK";        break;
      case SQLITE_IOERR_CLOSE:        zName = "SQLITE_IOERR_CLOSE";       break;
      case SQLITE_IOERR_DIR_CLOSE:    zName = "SQLITE_IOERR_DIR_CLOSE";   break;
      case SQLITE_IOERR_SHMOPEN:      zName = "SQLITE_IOERR_SHMOPEN";     break;
      case SQLITE_IOERR_SHMSIZE:      zName = "SQLITE_IOERR_SHMSIZE";     break;
      case SQLITE_IOERR_SHMLOCK:      zName = "SQLITE_IOERR_SHMLOCK";     break;
      case SQLITE_IOERR_SHMMAP:       zName = "SQLITE_IOERR_SHMMAP";      break;
      case SQLITE_IOERR_SEEK:         zName = "SQLITE_IOERR_SEEK";        break;
      case SQLITE_IOERR_DELETE_NOENT: zName = "SQLITE_IOERR_DELETE_NOENT";break;
      case SQLITE_IOERR_MMAP:         zName = "SQLITE_IOERR_MMAP";        break;
      case SQLITE_IOERR_GETTEMPPATH:  zName = "SQLITE_IOERR_GETTEMPPATH"; break;
      case SQLITE_IOERR_CONVPATH:     zName = "SQLITE_IOERR_CONVPATH";    break;
      case SQLITE_CORRUPT:            zName = "SQLITE_CORRUPT";           break;
      case SQLITE_CORRUPT_VTAB:       zName = "SQLITE_CORRUPT_VTAB";      break;
      case SQLITE_NOTFOUND:           zName = "SQLITE_NOTFOUND";          break;
      case SQLITE_FULL:               zName = "SQLITE_FULL";              break;
      case SQLITE_CANTOPEN:           zName = "SQLITE_CANTOPEN";          break;
      case SQLITE_CANTOPEN_NOTEMPDIR: zName = "SQLITE_CANTOPEN_NOTEMPDIR";break;
      case SQLITE_CANTOPEN_ISDIR:     zName = "SQLITE_CANTOPEN_ISDIR";    break;
      case SQLITE_CANTOPEN_FULLPATH:  zName = "SQLITE_CANTOPEN_FULLPATH"; break;
      case SQLITE_CANTOPEN_CONVPATH:  zName = "SQLITE_CANTOPEN_CONVPATH"; break;
      case SQLITE_PROTOCOL:           zName = "SQLITE_PROTOCOL";          break;
      case SQLITE_EMPTY:              zName = "SQLITE_EMPTY";             break;
      case SQLITE_SCHEMA:             zName = "SQLITE_SCHEMA";            break;
      case SQLITE_TOOBIG:             zName = "SQLITE_TOOBIG";            break;
      case SQLITE_CONSTRAINT:         zName = "SQLITE_CONSTRAINT";        break;
      case SQLITE_CONSTRAINT_UNIQUE:  zName = "SQLITE_CONSTRAINT_UNIQUE"; break;
      case SQLITE_CONSTRAINT_TRIGGER: zName = "SQLITE_CONSTRAINT_TRIGGER";break;
      case SQLITE_CONSTRAINT_FOREIGNKEY:
                                zName = "SQLITE_CONSTRAINT_FOREIGNKEY";   break;
      case SQLITE_CONSTRAINT_CHECK:   zName = "SQLITE_CONSTRAINT_CHECK";  break;
      case SQLITE_CONSTRAINT_PRIMARYKEY:
                                zName = "SQLITE_CONSTRAINT_PRIMARYKEY";   break;
      case SQLITE_CONSTRAINT_NOTNULL: zName = "SQLITE_CONSTRAINT_NOTNULL";break;
      case SQLITE_CONSTRAINT_COMMITHOOK:
                                zName = "SQLITE_CONSTRAINT_COMMITHOOK";   break;
      case SQLITE_CONSTRAINT_VTAB:    zName = "SQLITE_CONSTRAINT_VTAB";   break;
      case SQLITE_CONSTRAINT_FUNCTION:
                                zName = "SQLITE_CONSTRAINT_FUNCTION";     break;
      case SQLITE_CONSTRAINT_ROWID:   zName = "SQLITE_CONSTRAINT_ROWID";  break;
      case SQLITE_MISMATCH:           zName = "SQLITE_MISMATCH";          break;
      case SQLITE_MISUSE:             zName = "SQLITE_MISUSE";            break;
      case SQLITE_NOLFS:              zName = "SQLITE_NOLFS";             break;
      case SQLITE_AUTH:               zName = "SQLITE_AUTH";              break;
      case SQLITE_FORMAT:             zName = "SQLITE_FORMAT";            break;
      case SQLITE_RANGE:              zName = "SQLITE_RANGE";             break;
      case SQLITE_NOTADB:             zName = "SQLITE_NOTADB";            break;
      case SQLITE_ROW:                zName = "SQLITE_ROW";               break;
      case SQLITE_NOTICE:             zName = "SQLITE_NOTICE";            break;
      case SQLITE_NOTICE_RECOVER_WAL: zName = "SQLITE_NOTICE_RECOVER_WAL";break;
      case SQLITE_NOTICE_RECOVER_ROLLBACK:
                                zName = "SQLITE_NOTICE_RECOVER_ROLLBACK"; break;
      case SQLITE_WARNING:            zName = "SQLITE_WARNING";           break;
      case SQLITE_WARNING_AUTOINDEX:  zName = "SQLITE_WARNING_AUTOINDEX"; break;
      case SQLITE_DONE:               zName = "SQLITE_DONE";              break;
    }
  }
  if( zName==0 ){
    static char zBuf[50];
    sqlite3_snprintf(sizeof(zBuf), zBuf, "SQLITE_UNKNOWN(%d)", origRc);
    zName = zBuf;
  }
  return zName;
}
#endif

/*
** Return a static string that describes the kind of error specified in the
** argument.
*/
SQLITE_PRIVATE const char *sqlite3ErrStr(int rc){
  static const char* const aMsg[] = {
    /* SQLITE_OK          */ "not an error",
    /* SQLITE_ERROR       */ "SQL logic error or missing database",
    /* SQLITE_INTERNAL    */ 0,
    /* SQLITE_PERM        */ "access permission denied",
    /* SQLITE_ABORT       */ "callback requested query abort",
    /* SQLITE_BUSY        */ "database is locked",
    /* SQLITE_LOCKED      */ "database table is locked",
    /* SQLITE_NOMEM       */ "out of memory",
    /* SQLITE_READONLY    */ "attempt to write a readonly database",
    /* SQLITE_INTERRUPT   */ "interrupted",
    /* SQLITE_IOERR       */ "disk I/O error",
    /* SQLITE_CORRUPT     */ "database disk image is malformed",
    /* SQLITE_NOTFOUND    */ "unknown operation",
    /* SQLITE_FULL        */ "database or disk is full",
    /* SQLITE_CANTOPEN    */ "unable to open database file",
    /* SQLITE_PROTOCOL    */ "locking protocol",
    /* SQLITE_EMPTY       */ "table contains no data",
    /* SQLITE_SCHEMA      */ "database schema has changed",
    /* SQLITE_TOOBIG      */ "string or blob too big",
    /* SQLITE_CONSTRAINT  */ "constraint failed",
    /* SQLITE_MISMATCH    */ "datatype mismatch",
    /* SQLITE_MISUSE      */ "library routine called out of sequence",
    /* SQLITE_NOLFS       */ "large file support is disabled",
    /* SQLITE_AUTH        */ "authorization denied",
    /* SQLITE_FORMAT      */ "auxiliary database format error",
    /* SQLITE_RANGE       */ "bind or column index out of range",
    /* SQLITE_NOTADB      */ "file is encrypted or is not a database",
  };
  const char *zErr = "unknown error";
  switch( rc ){
    case SQLITE_ABORT_ROLLBACK: {
      zErr = "abort due to ROLLBACK";
      break;
    }
    default: {
      rc &= 0xff;
      if( ALWAYS(rc>=0) && rc<ArraySize(aMsg) && aMsg[rc]!=0 ){
        zErr = aMsg[rc];
      }
      break;
    }
  }
  return zErr;
}

/*
** This routine implements a busy callback that sleeps and tries
** again until a timeout value is reached.  The timeout value is
** an integer number of milliseconds passed in as the first
** argument.
*/
static int sqliteDefaultBusyCallback(
 void *ptr,               /* Database connection */
 int count                /* Number of times table has been busy */
){
#if SQLITE_OS_WIN || HAVE_USLEEP
  static const u8 delays[] =
     { 1, 2, 5, 10, 15, 20, 25, 25,  25,  50,  50, 100 };
  static const u8 totals[] =
     { 0, 1, 3,  8, 18, 33, 53, 78, 103, 128, 178, 228 };
# define NDELAY ArraySize(delays)
  sqlite3 *db = (sqlite3 *)ptr;
  int timeout = db->busyTimeout;
  int delay, prior;

  assert( count>=0 );
  if( count < NDELAY ){
    delay = delays[count];
    prior = totals[count];
  }else{
    delay = delays[NDELAY-1];
    prior = totals[NDELAY-1] + delay*(count-(NDELAY-1));
  }
  if( prior + delay > timeout ){
    delay = timeout - prior;
    if( delay<=0 ) return 0;
  }
  sqlite3OsSleep(db->pVfs, delay*1000);
  return 1;
#else
  sqlite3 *db = (sqlite3 *)ptr;
  int timeout = ((sqlite3 *)ptr)->busyTimeout;
  if( (count+1)*1000 > timeout ){
    return 0;
  }
  sqlite3OsSleep(db->pVfs, 1000000);
  return 1;
#endif
}

/*
** Invoke the given busy handler.
**
** This routine is called when an operation failed with a lock.
** If this routine returns non-zero, the lock is retried.  If it
** returns 0, the operation aborts with an SQLITE_BUSY error.
*/
SQLITE_PRIVATE int sqlite3InvokeBusyHandler(BusyHandler *p){
  int rc;
  if( NEVER(p==0) || p->xFunc==0 || p->nBusy<0 ) return 0;
  rc = p->xFunc(p->pArg, p->nBusy);
  if( rc==0 ){
    p->nBusy = -1;
  }else{
    p->nBusy++;
  }
  return rc; 
}

/*
** This routine sets the busy callback for an Sqlite database to the
** given callback function with the given argument.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_busy_handler(
  sqlite3 *db,
  int (*xBusy)(void*,int),
  void *pArg
){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->busyHandler.xFunc = xBusy;
  db->busyHandler.pArg = pArg;
  db->busyHandler.nBusy = 0;
  db->busyTimeout = 0;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

#ifndef SQLITE_OMIT_PROGRESS_CALLBACK
/*
** This routine sets the progress callback for an Sqlite database to the
** given callback function with the given argument. The progress callback will
** be invoked every nOps opcodes.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_progress_handler(
  sqlite3 *db, 
  int nOps,
  int (*xProgress)(void*), 
  void *pArg
){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  if( nOps>0 ){
    db->xProgress = xProgress;
    db->nProgressOps = (unsigned)nOps;
    db->pProgressArg = pArg;
  }else{
    db->xProgress = 0;
    db->nProgressOps = 0;
    db->pProgressArg = 0;
  }
  sqlite3_mutex_leave(db->mutex);
}
#endif


/*
** This routine installs a default busy handler that waits for the
** specified number of milliseconds before returning 0.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_busy_timeout(sqlite3 *db, int ms){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif
  if( ms>0 ){
    sqlite3_busy_handler(db, sqliteDefaultBusyCallback, (void*)db);
    db->busyTimeout = ms;
  }else{
    sqlite3_busy_handler(db, 0, 0);
  }
  return SQLITE_OK;
}

/*
** Cause any pending operation to stop at its earliest opportunity.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_interrupt(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return;
  }
#endif
  db->u1.isInterrupted = 1;
}


/*
** This function is exactly the same as sqlite3_create_function(), except
** that it is designed to be called by internal code. The difference is
** that if a malloc() fails in sqlite3_create_function(), an error code
** is returned and the mallocFailed flag cleared. 
*/
SQLITE_PRIVATE int sqlite3CreateFunc(
  sqlite3 *db,
  const char *zFunctionName,
  int nArg,
  int enc,
  void *pUserData,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value **),
  void (*xStep)(sqlite3_context*,int,sqlite3_value **),
  void (*xFinal)(sqlite3_context*),
  FuncDestructor *pDestructor
){
  FuncDef *p;
  int nName;
  int extraFlags;

  assert( sqlite3_mutex_held(db->mutex) );
  if( zFunctionName==0 ||
      (xFunc && (xFinal || xStep)) || 
      (!xFunc && (xFinal && !xStep)) ||
      (!xFunc && (!xFinal && xStep)) ||
      (nArg<-1 || nArg>SQLITE_MAX_FUNCTION_ARG) ||
      (255<(nName = sqlite3Strlen30( zFunctionName))) ){
    return SQLITE_MISUSE_BKPT;
  }

  assert( SQLITE_FUNC_CONSTANT==SQLITE_DETERMINISTIC );
  extraFlags = enc &  SQLITE_DETERMINISTIC;
  enc &= (SQLITE_FUNC_ENCMASK|SQLITE_ANY);
  
#ifndef SQLITE_OMIT_UTF16
  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  **
  ** If SQLITE_ANY is specified, add three versions of the function
  ** to the hash table.
  */
  if( enc==SQLITE_UTF16 ){
    enc = SQLITE_UTF16NATIVE;
  }else if( enc==SQLITE_ANY ){
    int rc;
    rc = sqlite3CreateFunc(db, zFunctionName, nArg, SQLITE_UTF8|extraFlags,
         pUserData, xFunc, xStep, xFinal, pDestructor);
    if( rc==SQLITE_OK ){
      rc = sqlite3CreateFunc(db, zFunctionName, nArg, SQLITE_UTF16LE|extraFlags,
          pUserData, xFunc, xStep, xFinal, pDestructor);
    }
    if( rc!=SQLITE_OK ){
      return rc;
    }
    enc = SQLITE_UTF16BE;
  }
#else
  enc = SQLITE_UTF8;
#endif
  
  /* Check if an existing function is being overridden or deleted. If so,
  ** and there are active VMs, then return SQLITE_BUSY. If a function
  ** is being overridden/deleted but there are no active VMs, allow the
  ** operation to continue but invalidate all precompiled statements.
  */
  p = sqlite3FindFunction(db, zFunctionName, nName, nArg, (u8)enc, 0);
  if( p && (p->funcFlags & SQLITE_FUNC_ENCMASK)==enc && p->nArg==nArg ){
    if( db->nVdbeActive ){
      sqlite3ErrorWithMsg(db, SQLITE_BUSY, 
        "unable to delete/modify user-function due to active statements");
      assert( !db->mallocFailed );
      return SQLITE_BUSY;
    }else{
      sqlite3ExpirePreparedStatements(db);
    }
  }

  p = sqlite3FindFunction(db, zFunctionName, nName, nArg, (u8)enc, 1);
  assert(p || db->mallocFailed);
  if( !p ){
    return SQLITE_NOMEM;
  }

  /* If an older version of the function with a configured destructor is
  ** being replaced invoke the destructor function here. */
  functionDestroy(db, p);

  if( pDestructor ){
    pDestructor->nRef++;
  }
  p->pDestructor = pDestructor;
  p->funcFlags = (p->funcFlags & SQLITE_FUNC_ENCMASK) | extraFlags;
  testcase( p->funcFlags & SQLITE_DETERMINISTIC );
  p->xFunc = xFunc;
  p->xStep = xStep;
  p->xFinalize = xFinal;
  p->pUserData = pUserData;
  p->nArg = (u16)nArg;
  return SQLITE_OK;
}

/*
** Create new user functions.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_function(
  sqlite3 *db,
  const char *zFunc,
  int nArg,
  int enc,
  void *p,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value **),
  void (*xStep)(sqlite3_context*,int,sqlite3_value **),
  void (*xFinal)(sqlite3_context*)
){
  return sqlite3_create_function_v2(db, zFunc, nArg, enc, p, xFunc, xStep,
                                    xFinal, 0);
}

SQLITE_API int SQLITE_STDCALL sqlite3_create_function_v2(
  sqlite3 *db,
  const char *zFunc,
  int nArg,
  int enc,
  void *p,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value **),
  void (*xStep)(sqlite3_context*,int,sqlite3_value **),
  void (*xFinal)(sqlite3_context*),
  void (*xDestroy)(void *)
){
  int rc = SQLITE_ERROR;
  FuncDestructor *pArg = 0;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    return SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  if( xDestroy ){
    pArg = (FuncDestructor *)sqlite3DbMallocZero(db, sizeof(FuncDestructor));
    if( !pArg ){
      xDestroy(p);
      goto out;
    }
    pArg->xDestroy = xDestroy;
    pArg->pUserData = p;
  }
  rc = sqlite3CreateFunc(db, zFunc, nArg, enc, p, xFunc, xStep, xFinal, pArg);
  if( pArg && pArg->nRef==0 ){
    assert( rc!=SQLITE_OK );
    xDestroy(p);
    sqlite3DbFree(db, pArg);
  }

 out:
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

#ifndef SQLITE_OMIT_UTF16
SQLITE_API int SQLITE_STDCALL sqlite3_create_function16(
  sqlite3 *db,
  const void *zFunctionName,
  int nArg,
  int eTextRep,
  void *p,
  void (*xFunc)(sqlite3_context*,int,sqlite3_value**),
  void (*xStep)(sqlite3_context*,int,sqlite3_value**),
  void (*xFinal)(sqlite3_context*)
){
  int rc;
  char *zFunc8;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) || zFunctionName==0 ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  zFunc8 = sqlite3Utf16to8(db, zFunctionName, -1, SQLITE_UTF16NATIVE);
  rc = sqlite3CreateFunc(db, zFunc8, nArg, eTextRep, p, xFunc, xStep, xFinal,0);
  sqlite3DbFree(db, zFunc8);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}
#endif


/*
** Declare that a function has been overloaded by a virtual table.
**
** If the function already exists as a regular global function, then
** this routine is a no-op.  If the function does not exist, then create
** a new one that always throws a run-time error.  
**
** When virtual tables intend to provide an overloaded function, they
** should call this routine to make sure the global function exists.
** A global function must exist in order for name resolution to work
** properly.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_overload_function(
  sqlite3 *db,
  const char *zName,
  int nArg
){
  int nName = sqlite3Strlen30(zName);
  int rc = SQLITE_OK;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) || zName==0 || nArg<-2 ){
    return SQLITE_MISUSE_BKPT;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  if( sqlite3FindFunction(db, zName, nName, nArg, SQLITE_UTF8, 0)==0 ){
    rc = sqlite3CreateFunc(db, zName, nArg, SQLITE_UTF8,
                           0, sqlite3InvalidFunction, 0, 0, 0);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

#ifndef SQLITE_OMIT_TRACE
/*
** Register a trace function.  The pArg from the previously registered trace
** is returned.  
**
** A NULL trace function means that no tracing is executes.  A non-NULL
** trace is a pointer to a function that is invoked at the start of each
** SQL statement.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_trace(sqlite3 *db, void (*xTrace)(void*,const char*), void *pArg){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pTraceArg;
  db->xTrace = xTrace;
  db->pTraceArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pOld;
}
/*
** Register a profile function.  The pArg from the previously registered 
** profile function is returned.  
**
** A NULL profile function means that no profiling is executes.  A non-NULL
** profile is a pointer to a function that is invoked at the conclusion of
** each SQL statement that is run.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_profile(
  sqlite3 *db,
  void (*xProfile)(void*,const char*,sqlite_uint64),
  void *pArg
){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pProfileArg;
  db->xProfile = xProfile;
  db->pProfileArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pOld;
}
#endif /* SQLITE_OMIT_TRACE */

/*
** Register a function to be invoked when a transaction commits.
** If the invoked function returns non-zero, then the commit becomes a
** rollback.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_commit_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  int (*xCallback)(void*),  /* Function to invoke on each commit */
  void *pArg                /* Argument to the function */
){
  void *pOld;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pOld = db->pCommitArg;
  db->xCommitCallback = xCallback;
  db->pCommitArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pOld;
}

/*
** Register a callback to be invoked each time a row is updated,
** inserted or deleted using this database connection.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_update_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  void (*xCallback)(void*,int,char const *,char const *,sqlite_int64),
  void *pArg                /* Argument to the function */
){
  void *pRet;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pUpdateArg;
  db->xUpdateCallback = xCallback;
  db->pUpdateArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pRet;
}

/*
** Register a callback to be invoked each time a transaction is rolled
** back by this database connection.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_rollback_hook(
  sqlite3 *db,              /* Attach the hook to this database */
  void (*xCallback)(void*), /* Callback function */
  void *pArg                /* Argument to the function */
){
  void *pRet;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pRollbackArg;
  db->xRollbackCallback = xCallback;
  db->pRollbackArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pRet;
}

#ifndef SQLITE_OMIT_WAL
/*
** The sqlite3_wal_hook() callback registered by sqlite3_wal_autocheckpoint().
** Invoke sqlite3_wal_checkpoint if the number of frames in the log file
** is greater than sqlite3.pWalArg cast to an integer (the value configured by
** wal_autocheckpoint()).
*/ 
SQLITE_PRIVATE int sqlite3WalDefaultHook(
  void *pClientData,     /* Argument */
  sqlite3 *db,           /* Connection */
  const char *zDb,       /* Database */
  int nFrame             /* Size of WAL */
){
  if( nFrame>=SQLITE_PTR_TO_INT(pClientData) ){
    sqlite3BeginBenignMalloc();
    sqlite3_wal_checkpoint(db, zDb);
    sqlite3EndBenignMalloc();
  }
  return SQLITE_OK;
}
#endif /* SQLITE_OMIT_WAL */

/*
** Configure an sqlite3_wal_hook() callback to automatically checkpoint
** a database after committing a transaction if there are nFrame or
** more frames in the log file. Passing zero or a negative value as the
** nFrame parameter disables automatic checkpoints entirely.
**
** The callback registered by this function replaces any existing callback
** registered using sqlite3_wal_hook(). Likewise, registering a callback
** using sqlite3_wal_hook() disables the automatic checkpoint mechanism
** configured by this function.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_autocheckpoint(sqlite3 *db, int nFrame){
#ifdef SQLITE_OMIT_WAL
  UNUSED_PARAMETER(db);
  UNUSED_PARAMETER(nFrame);
#else
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif
  if( nFrame>0 ){
    sqlite3_wal_hook(db, sqlite3WalDefaultHook, SQLITE_INT_TO_PTR(nFrame));
  }else{
    sqlite3_wal_hook(db, 0, 0);
  }
#endif
  return SQLITE_OK;
}

/*
** Register a callback to be invoked each time a transaction is written
** into the write-ahead-log by this database connection.
*/
SQLITE_API void *SQLITE_STDCALL sqlite3_wal_hook(
  sqlite3 *db,                    /* Attach the hook to this db handle */
  int(*xCallback)(void *, sqlite3*, const char*, int),
  void *pArg                      /* First argument passed to xCallback() */
){
#ifndef SQLITE_OMIT_WAL
  void *pRet;
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  sqlite3_mutex_enter(db->mutex);
  pRet = db->pWalArg;
  db->xWalCallback = xCallback;
  db->pWalArg = pArg;
  sqlite3_mutex_leave(db->mutex);
  return pRet;
#else
  return 0;
#endif
}

/*
** Checkpoint database zDb.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_checkpoint_v2(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of attached database (or NULL) */
  int eMode,                      /* SQLITE_CHECKPOINT_* value */
  int *pnLog,                     /* OUT: Size of WAL log in frames */
  int *pnCkpt                     /* OUT: Total number of frames checkpointed */
){
#ifdef SQLITE_OMIT_WAL
  return SQLITE_OK;
#else
  int rc;                         /* Return code */
  int iDb = SQLITE_MAX_ATTACHED;  /* sqlite3.aDb[] index of db to checkpoint */

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif

  /* Initialize the output variables to -1 in case an error occurs. */
  if( pnLog ) *pnLog = -1;
  if( pnCkpt ) *pnCkpt = -1;

  assert( SQLITE_CHECKPOINT_PASSIVE==0 );
  assert( SQLITE_CHECKPOINT_FULL==1 );
  assert( SQLITE_CHECKPOINT_RESTART==2 );
  assert( SQLITE_CHECKPOINT_TRUNCATE==3 );
  if( eMode<SQLITE_CHECKPOINT_PASSIVE || eMode>SQLITE_CHECKPOINT_TRUNCATE ){
    /* EVIDENCE-OF: R-03996-12088 The M parameter must be a valid checkpoint
    ** mode: */
    return SQLITE_MISUSE;
  }

  sqlite3_mutex_enter(db->mutex);
  if( zDb && zDb[0] ){
    iDb = sqlite3FindDbName(db, zDb);
  }
  if( iDb<0 ){
    rc = SQLITE_ERROR;
    sqlite3ErrorWithMsg(db, SQLITE_ERROR, "unknown database: %s", zDb);
  }else{
    db->busyHandler.nBusy = 0;
    rc = sqlite3Checkpoint(db, iDb, eMode, pnLog, pnCkpt);
    sqlite3Error(db, rc);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
#endif
}


/*
** Checkpoint database zDb. If zDb is NULL, or if the buffer zDb points
** to contains a zero-length string, all attached databases are 
** checkpointed.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_wal_checkpoint(sqlite3 *db, const char *zDb){
  /* EVIDENCE-OF: R-41613-20553 The sqlite3_wal_checkpoint(D,X) is equivalent to
  ** sqlite3_wal_checkpoint_v2(D,X,SQLITE_CHECKPOINT_PASSIVE,0,0). */
  return sqlite3_wal_checkpoint_v2(db,zDb,SQLITE_CHECKPOINT_PASSIVE,0,0);
}

#ifndef SQLITE_OMIT_WAL
/*
** Run a checkpoint on database iDb. This is a no-op if database iDb is
** not currently open in WAL mode.
**
** If a transaction is open on the database being checkpointed, this 
** function returns SQLITE_LOCKED and a checkpoint is not attempted. If 
** an error occurs while running the checkpoint, an SQLite error code is 
** returned (i.e. SQLITE_IOERR). Otherwise, SQLITE_OK.
**
** The mutex on database handle db should be held by the caller. The mutex
** associated with the specific b-tree being checkpointed is taken by
** this function while the checkpoint is running.
**
** If iDb is passed SQLITE_MAX_ATTACHED, then all attached databases are
** checkpointed. If an error is encountered it is returned immediately -
** no attempt is made to checkpoint any remaining databases.
**
** Parameter eMode is one of SQLITE_CHECKPOINT_PASSIVE, FULL or RESTART.
*/
SQLITE_PRIVATE int sqlite3Checkpoint(sqlite3 *db, int iDb, int eMode, int *pnLog, int *pnCkpt){
  int rc = SQLITE_OK;             /* Return code */
  int i;                          /* Used to iterate through attached dbs */
  int bBusy = 0;                  /* True if SQLITE_BUSY has been encountered */

  assert( sqlite3_mutex_held(db->mutex) );
  assert( !pnLog || *pnLog==-1 );
  assert( !pnCkpt || *pnCkpt==-1 );

  for(i=0; i<db->nDb && rc==SQLITE_OK; i++){
    if( i==iDb || iDb==SQLITE_MAX_ATTACHED ){
      rc = sqlite3BtreeCheckpoint(db->aDb[i].pBt, eMode, pnLog, pnCkpt);
      pnLog = 0;
      pnCkpt = 0;
      if( rc==SQLITE_BUSY ){
        bBusy = 1;
        rc = SQLITE_OK;
      }
    }
  }

  return (rc==SQLITE_OK && bBusy) ? SQLITE_BUSY : rc;
}
#endif /* SQLITE_OMIT_WAL */

/*
** This function returns true if main-memory should be used instead of
** a temporary file for transient pager files and statement journals.
** The value returned depends on the value of db->temp_store (runtime
** parameter) and the compile time value of SQLITE_TEMP_STORE. The
** following table describes the relationship between these two values
** and this functions return value.
**
**   SQLITE_TEMP_STORE     db->temp_store     Location of temporary database
**   -----------------     --------------     ------------------------------
**   0                     any                file      (return 0)
**   1                     1                  file      (return 0)
**   1                     2                  memory    (return 1)
**   1                     0                  file      (return 0)
**   2                     1                  file      (return 0)
**   2                     2                  memory    (return 1)
**   2                     0                  memory    (return 1)
**   3                     any                memory    (return 1)
*/
SQLITE_PRIVATE int sqlite3TempInMemory(const sqlite3 *db){
#if SQLITE_TEMP_STORE==1
  return ( db->temp_store==2 );
#endif
#if SQLITE_TEMP_STORE==2
  return ( db->temp_store!=1 );
#endif
#if SQLITE_TEMP_STORE==3
  UNUSED_PARAMETER(db);
  return 1;
#endif
#if SQLITE_TEMP_STORE<1 || SQLITE_TEMP_STORE>3
  UNUSED_PARAMETER(db);
  return 0;
#endif
}

/*
** Return UTF-8 encoded English language explanation of the most recent
** error.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_errmsg(sqlite3 *db){
  const char *z;
  if( !db ){
    return sqlite3ErrStr(SQLITE_NOMEM);
  }
  if( !sqlite3SafetyCheckSickOrOk(db) ){
    return sqlite3ErrStr(SQLITE_MISUSE_BKPT);
  }
  sqlite3_mutex_enter(db->mutex);
  if( db->mallocFailed ){
    z = sqlite3ErrStr(SQLITE_NOMEM);
  }else{
    testcase( db->pErr==0 );
    z = (char*)sqlite3_value_text(db->pErr);
    assert( !db->mallocFailed );
    if( z==0 ){
      z = sqlite3ErrStr(db->errCode);
    }
  }
  sqlite3_mutex_leave(db->mutex);
  return z;
}

#ifndef SQLITE_OMIT_UTF16
/*
** Return UTF-16 encoded English language explanation of the most recent
** error.
*/
SQLITE_API const void *SQLITE_STDCALL sqlite3_errmsg16(sqlite3 *db){
  static const u16 outOfMem[] = {
    'o', 'u', 't', ' ', 'o', 'f', ' ', 'm', 'e', 'm', 'o', 'r', 'y', 0
  };
  static const u16 misuse[] = {
    'l', 'i', 'b', 'r', 'a', 'r', 'y', ' ', 
    'r', 'o', 'u', 't', 'i', 'n', 'e', ' ', 
    'c', 'a', 'l', 'l', 'e', 'd', ' ', 
    'o', 'u', 't', ' ', 
    'o', 'f', ' ', 
    's', 'e', 'q', 'u', 'e', 'n', 'c', 'e', 0
  };

  const void *z;
  if( !db ){
    return (void *)outOfMem;
  }
  if( !sqlite3SafetyCheckSickOrOk(db) ){
    return (void *)misuse;
  }
  sqlite3_mutex_enter(db->mutex);
  if( db->mallocFailed ){
    z = (void *)outOfMem;
  }else{
    z = sqlite3_value_text16(db->pErr);
    if( z==0 ){
      sqlite3ErrorWithMsg(db, db->errCode, sqlite3ErrStr(db->errCode));
      z = sqlite3_value_text16(db->pErr);
    }
    /* A malloc() may have failed within the call to sqlite3_value_text16()
    ** above. If this is the case, then the db->mallocFailed flag needs to
    ** be cleared before returning. Do this directly, instead of via
    ** sqlite3ApiExit(), to avoid setting the database handle error message.
    */
    db->mallocFailed = 0;
  }
  sqlite3_mutex_leave(db->mutex);
  return z;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Return the most recent error code generated by an SQLite routine. If NULL is
** passed to this function, we assume a malloc() failed during sqlite3_open().
*/
SQLITE_API int SQLITE_STDCALL sqlite3_errcode(sqlite3 *db){
  if( db && !sqlite3SafetyCheckSickOrOk(db) ){
    return SQLITE_MISUSE_BKPT;
  }
  if( !db || db->mallocFailed ){
    return SQLITE_NOMEM;
  }
  return db->errCode & db->errMask;
}
SQLITE_API int SQLITE_STDCALL sqlite3_extended_errcode(sqlite3 *db){
  if( db && !sqlite3SafetyCheckSickOrOk(db) ){
    return SQLITE_MISUSE_BKPT;
  }
  if( !db || db->mallocFailed ){
    return SQLITE_NOMEM;
  }
  return db->errCode;
}

/*
** Return a string that describes the kind of error specified in the
** argument.  For now, this simply calls the internal sqlite3ErrStr()
** function.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_errstr(int rc){
  return sqlite3ErrStr(rc);
}

/*
** Create a new collating function for database "db".  The name is zName
** and the encoding is enc.
*/
static int createCollation(
  sqlite3* db,
  const char *zName, 
  u8 enc,
  void* pCtx,
  int(*xCompare)(void*,int,const void*,int,const void*),
  void(*xDel)(void*)
){
  CollSeq *pColl;
  int enc2;
  
  assert( sqlite3_mutex_held(db->mutex) );

  /* If SQLITE_UTF16 is specified as the encoding type, transform this
  ** to one of SQLITE_UTF16LE or SQLITE_UTF16BE using the
  ** SQLITE_UTF16NATIVE macro. SQLITE_UTF16 is not used internally.
  */
  enc2 = enc;
  testcase( enc2==SQLITE_UTF16 );
  testcase( enc2==SQLITE_UTF16_ALIGNED );
  if( enc2==SQLITE_UTF16 || enc2==SQLITE_UTF16_ALIGNED ){
    enc2 = SQLITE_UTF16NATIVE;
  }
  if( enc2<SQLITE_UTF8 || enc2>SQLITE_UTF16BE ){
    return SQLITE_MISUSE_BKPT;
  }

  /* Check if this call is removing or replacing an existing collation 
  ** sequence. If so, and there are active VMs, return busy. If there
  ** are no active VMs, invalidate any pre-compiled statements.
  */
  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 0);
  if( pColl && pColl->xCmp ){
    if( db->nVdbeActive ){
      sqlite3ErrorWithMsg(db, SQLITE_BUSY, 
        "unable to delete/modify collation sequence due to active statements");
      return SQLITE_BUSY;
    }
    sqlite3ExpirePreparedStatements(db);

    /* If collation sequence pColl was created directly by a call to
    ** sqlite3_create_collation, and not generated by synthCollSeq(),
    ** then any copies made by synthCollSeq() need to be invalidated.
    ** Also, collation destructor - CollSeq.xDel() - function may need
    ** to be called.
    */ 
    if( (pColl->enc & ~SQLITE_UTF16_ALIGNED)==enc2 ){
      CollSeq *aColl = sqlite3HashFind(&db->aCollSeq, zName);
      int j;
      for(j=0; j<3; j++){
        CollSeq *p = &aColl[j];
        if( p->enc==pColl->enc ){
          if( p->xDel ){
            p->xDel(p->pUser);
          }
          p->xCmp = 0;
        }
      }
    }
  }

  pColl = sqlite3FindCollSeq(db, (u8)enc2, zName, 1);
  if( pColl==0 ) return SQLITE_NOMEM;
  pColl->xCmp = xCompare;
  pColl->pUser = pCtx;
  pColl->xDel = xDel;
  pColl->enc = (u8)(enc2 | (enc & SQLITE_UTF16_ALIGNED));
  sqlite3Error(db, SQLITE_OK);
  return SQLITE_OK;
}


/*
** This array defines hard upper bounds on limit values.  The
** initializer must be kept in sync with the SQLITE_LIMIT_*
** #defines in sqlite3.h.
*/
static const int aHardLimit[] = {
  SQLITE_MAX_LENGTH,
  SQLITE_MAX_SQL_LENGTH,
  SQLITE_MAX_COLUMN,
  SQLITE_MAX_EXPR_DEPTH,
  SQLITE_MAX_COMPOUND_SELECT,
  SQLITE_MAX_VDBE_OP,
  SQLITE_MAX_FUNCTION_ARG,
  SQLITE_MAX_ATTACHED,
  SQLITE_MAX_LIKE_PATTERN_LENGTH,
  SQLITE_MAX_VARIABLE_NUMBER,      /* IMP: R-38091-32352 */
  SQLITE_MAX_TRIGGER_DEPTH,
  SQLITE_MAX_WORKER_THREADS,
};

/*
** Make sure the hard limits are set to reasonable values
*/
#if SQLITE_MAX_LENGTH<100
# error SQLITE_MAX_LENGTH must be at least 100
#endif
#if SQLITE_MAX_SQL_LENGTH<100
# error SQLITE_MAX_SQL_LENGTH must be at least 100
#endif
#if SQLITE_MAX_SQL_LENGTH>SQLITE_MAX_LENGTH
# error SQLITE_MAX_SQL_LENGTH must not be greater than SQLITE_MAX_LENGTH
#endif
#if SQLITE_MAX_COMPOUND_SELECT<2
# error SQLITE_MAX_COMPOUND_SELECT must be at least 2
#endif
#if SQLITE_MAX_VDBE_OP<40
# error SQLITE_MAX_VDBE_OP must be at least 40
#endif
#if SQLITE_MAX_FUNCTION_ARG<0 || SQLITE_MAX_FUNCTION_ARG>1000
# error SQLITE_MAX_FUNCTION_ARG must be between 0 and 1000
#endif
#if SQLITE_MAX_ATTACHED<0 || SQLITE_MAX_ATTACHED>125
# error SQLITE_MAX_ATTACHED must be between 0 and 125
#endif
#if SQLITE_MAX_LIKE_PATTERN_LENGTH<1
# error SQLITE_MAX_LIKE_PATTERN_LENGTH must be at least 1
#endif
#if SQLITE_MAX_COLUMN>32767
# error SQLITE_MAX_COLUMN must not exceed 32767
#endif
#if SQLITE_MAX_TRIGGER_DEPTH<1
# error SQLITE_MAX_TRIGGER_DEPTH must be at least 1
#endif
#if SQLITE_MAX_WORKER_THREADS<0 || SQLITE_MAX_WORKER_THREADS>50
# error SQLITE_MAX_WORKER_THREADS must be between 0 and 50
#endif


/*
** Change the value of a limit.  Report the old value.
** If an invalid limit index is supplied, report -1.
** Make no changes but still report the old value if the
** new limit is negative.
**
** A new lower limit does not shrink existing constructs.
** It merely prevents new constructs that exceed the limit
** from forming.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_limit(sqlite3 *db, int limitId, int newLimit){
  int oldLimit;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return -1;
  }
#endif

  /* EVIDENCE-OF: R-30189-54097 For each limit category SQLITE_LIMIT_NAME
  ** there is a hard upper bound set at compile-time by a C preprocessor
  ** macro called SQLITE_MAX_NAME. (The "_LIMIT_" in the name is changed to
  ** "_MAX_".)
  */
  assert( aHardLimit[SQLITE_LIMIT_LENGTH]==SQLITE_MAX_LENGTH );
  assert( aHardLimit[SQLITE_LIMIT_SQL_LENGTH]==SQLITE_MAX_SQL_LENGTH );
  assert( aHardLimit[SQLITE_LIMIT_COLUMN]==SQLITE_MAX_COLUMN );
  assert( aHardLimit[SQLITE_LIMIT_EXPR_DEPTH]==SQLITE_MAX_EXPR_DEPTH );
  assert( aHardLimit[SQLITE_LIMIT_COMPOUND_SELECT]==SQLITE_MAX_COMPOUND_SELECT);
  assert( aHardLimit[SQLITE_LIMIT_VDBE_OP]==SQLITE_MAX_VDBE_OP );
  assert( aHardLimit[SQLITE_LIMIT_FUNCTION_ARG]==SQLITE_MAX_FUNCTION_ARG );
  assert( aHardLimit[SQLITE_LIMIT_ATTACHED]==SQLITE_MAX_ATTACHED );
  assert( aHardLimit[SQLITE_LIMIT_LIKE_PATTERN_LENGTH]==
                                               SQLITE_MAX_LIKE_PATTERN_LENGTH );
  assert( aHardLimit[SQLITE_LIMIT_VARIABLE_NUMBER]==SQLITE_MAX_VARIABLE_NUMBER);
  assert( aHardLimit[SQLITE_LIMIT_TRIGGER_DEPTH]==SQLITE_MAX_TRIGGER_DEPTH );
  assert( aHardLimit[SQLITE_LIMIT_WORKER_THREADS]==SQLITE_MAX_WORKER_THREADS );
  assert( SQLITE_LIMIT_WORKER_THREADS==(SQLITE_N_LIMIT-1) );


  if( limitId<0 || limitId>=SQLITE_N_LIMIT ){
    return -1;
  }
  oldLimit = db->aLimit[limitId];
  if( newLimit>=0 ){                   /* IMP: R-52476-28732 */
    if( newLimit>aHardLimit[limitId] ){
      newLimit = aHardLimit[limitId];  /* IMP: R-51463-25634 */
    }
    db->aLimit[limitId] = newLimit;
  }
  return oldLimit;                     /* IMP: R-53341-35419 */
}

/*
** This function is used to parse both URIs and non-URI filenames passed by the
** user to API functions sqlite3_open() or sqlite3_open_v2(), and for database
** URIs specified as part of ATTACH statements.
**
** The first argument to this function is the name of the VFS to use (or
** a NULL to signify the default VFS) if the URI does not contain a "vfs=xxx"
** query parameter. The second argument contains the URI (or non-URI filename)
** itself. When this function is called the *pFlags variable should contain
** the default flags to open the database handle with. The value stored in
** *pFlags may be updated before returning if the URI filename contains 
** "cache=xxx" or "mode=xxx" query parameters.
**
** If successful, SQLITE_OK is returned. In this case *ppVfs is set to point to
** the VFS that should be used to open the database file. *pzFile is set to
** point to a buffer containing the name of the file to open. It is the 
** responsibility of the caller to eventually call sqlite3_free() to release
** this buffer.
**
** If an error occurs, then an SQLite error code is returned and *pzErrMsg
** may be set to point to a buffer containing an English language error 
** message. It is the responsibility of the caller to eventually release
** this buffer by calling sqlite3_free().
*/
SQLITE_PRIVATE int sqlite3ParseUri(
  const char *zDefaultVfs,        /* VFS to use if no "vfs=xxx" query option */
  const char *zUri,               /* Nul-terminated URI to parse */
  unsigned int *pFlags,           /* IN/OUT: SQLITE_OPEN_XXX flags */
  sqlite3_vfs **ppVfs,            /* OUT: VFS to use */ 
  char **pzFile,                  /* OUT: Filename component of URI */
  char **pzErrMsg                 /* OUT: Error message (if rc!=SQLITE_OK) */
){
  int rc = SQLITE_OK;
  unsigned int flags = *pFlags;
  const char *zVfs = zDefaultVfs;
  char *zFile;
  char c;
  int nUri = sqlite3Strlen30(zUri);

  assert( *pzErrMsg==0 );

  if( ((flags & SQLITE_OPEN_URI)             /* IMP: R-48725-32206 */
            || sqlite3GlobalConfig.bOpenUri) /* IMP: R-51689-46548 */
   && nUri>=5 && memcmp(zUri, "file:", 5)==0 /* IMP: R-57884-37496 */
  ){
    char *zOpt;
    int eState;                   /* Parser state when parsing URI */
    int iIn;                      /* Input character index */
    int iOut = 0;                 /* Output character index */
    u64 nByte = nUri+2;           /* Bytes of space to allocate */

    /* Make sure the SQLITE_OPEN_URI flag is set to indicate to the VFS xOpen 
    ** method that there may be extra parameters following the file-name.  */
    flags |= SQLITE_OPEN_URI;

    for(iIn=0; iIn<nUri; iIn++) nByte += (zUri[iIn]=='&');
    zFile = sqlite3_malloc64(nByte);
    if( !zFile ) return SQLITE_NOMEM;

    iIn = 5;
#ifdef SQLITE_ALLOW_URI_AUTHORITY
    if( strncmp(zUri+5, "///", 3)==0 ){
      iIn = 7;
      /* The following condition causes URIs with five leading / characters
      ** like file://///host/path to be converted into UNCs like //host/path.
      ** The correct URI for that UNC has only two or four leading / characters
      ** file://host/path or file:////host/path.  But 5 leading slashes is a 
      ** common error, we are told, so we handle it as a special case. */
      if( strncmp(zUri+7, "///", 3)==0 ){ iIn++; }
    }else if( strncmp(zUri+5, "//localhost/", 12)==0 ){
      iIn = 16;
    }
#else
    /* Discard the scheme and authority segments of the URI. */
    if( zUri[5]=='/' && zUri[6]=='/' ){
      iIn = 7;
      while( zUri[iIn] && zUri[iIn]!='/' ) iIn++;
      if( iIn!=7 && (iIn!=16 || memcmp("localhost", &zUri[7], 9)) ){
        *pzErrMsg = sqlite3_mprintf("invalid uri authority: %.*s", 
            iIn-7, &zUri[7]);
        rc = SQLITE_ERROR;
        goto parse_uri_out;
      }
    }
#endif

    /* Copy the filename and any query parameters into the zFile buffer. 
    ** Decode %HH escape codes along the way. 
    **
    ** Within this loop, variable eState may be set to 0, 1 or 2, depending
    ** on the parsing context. As follows:
    **
    **   0: Parsing file-name.
    **   1: Parsing name section of a name=value query parameter.
    **   2: Parsing value section of a name=value query parameter.
    */
    eState = 0;
    while( (c = zUri[iIn])!=0 && c!='#' ){
      iIn++;
      if( c=='%' 
       && sqlite3Isxdigit(zUri[iIn]) 
       && sqlite3Isxdigit(zUri[iIn+1]) 
      ){
        int octet = (sqlite3HexToInt(zUri[iIn++]) << 4);
        octet += sqlite3HexToInt(zUri[iIn++]);

        assert( octet>=0 && octet<256 );
        if( octet==0 ){
          /* This branch is taken when "%00" appears within the URI. In this
          ** case we ignore all text in the remainder of the path, name or
          ** value currently being parsed. So ignore the current character
          ** and skip to the next "?", "=" or "&", as appropriate. */
          while( (c = zUri[iIn])!=0 && c!='#' 
              && (eState!=0 || c!='?')
              && (eState!=1 || (c!='=' && c!='&'))
              && (eState!=2 || c!='&')
          ){
            iIn++;
          }
          continue;
        }
        c = octet;
      }else if( eState==1 && (c=='&' || c=='=') ){
        if( zFile[iOut-1]==0 ){
          /* An empty option name. Ignore this option altogether. */
          while( zUri[iIn] && zUri[iIn]!='#' && zUri[iIn-1]!='&' ) iIn++;
          continue;
        }
        if( c=='&' ){
          zFile[iOut++] = '\0';
        }else{
          eState = 2;
        }
        c = 0;
      }else if( (eState==0 && c=='?') || (eState==2 && c=='&') ){
        c = 0;
        eState = 1;
      }
      zFile[iOut++] = c;
    }
    if( eState==1 ) zFile[iOut++] = '\0';
    zFile[iOut++] = '\0';
    zFile[iOut++] = '\0';

    /* Check if there were any options specified that should be interpreted 
    ** here. Options that are interpreted here include "vfs" and those that
    ** correspond to flags that may be passed to the sqlite3_open_v2()
    ** method. */
    zOpt = &zFile[sqlite3Strlen30(zFile)+1];
    while( zOpt[0] ){
      int nOpt = sqlite3Strlen30(zOpt);
      char *zVal = &zOpt[nOpt+1];
      int nVal = sqlite3Strlen30(zVal);

      if( nOpt==3 && memcmp("vfs", zOpt, 3)==0 ){
        zVfs = zVal;
      }else{
        struct OpenMode {
          const char *z;
          int mode;
        } *aMode = 0;
        char *zModeType = 0;
        int mask = 0;
        int limit = 0;

        if( nOpt==5 && memcmp("cache", zOpt, 5)==0 ){
          static struct OpenMode aCacheMode[] = {
            { "shared",  SQLITE_OPEN_SHAREDCACHE },
            { "private", SQLITE_OPEN_PRIVATECACHE },
            { 0, 0 }
          };

          mask = SQLITE_OPEN_SHAREDCACHE|SQLITE_OPEN_PRIVATECACHE;
          aMode = aCacheMode;
          limit = mask;
          zModeType = "cache";
        }
        if( nOpt==4 && memcmp("mode", zOpt, 4)==0 ){
          static struct OpenMode aOpenMode[] = {
            { "ro",  SQLITE_OPEN_READONLY },
            { "rw",  SQLITE_OPEN_READWRITE }, 
            { "rwc", SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE },
            { "memory", SQLITE_OPEN_MEMORY },
            { 0, 0 }
          };

          mask = SQLITE_OPEN_READONLY | SQLITE_OPEN_READWRITE
                   | SQLITE_OPEN_CREATE | SQLITE_OPEN_MEMORY;
          aMode = aOpenMode;
          limit = mask & flags;
          zModeType = "access";
        }

        if( aMode ){
          int i;
          int mode = 0;
          for(i=0; aMode[i].z; i++){
            const char *z = aMode[i].z;
            if( nVal==sqlite3Strlen30(z) && 0==memcmp(zVal, z, nVal) ){
              mode = aMode[i].mode;
              break;
            }
          }
          if( mode==0 ){
            *pzErrMsg = sqlite3_mprintf("no such %s mode: %s", zModeType, zVal);
            rc = SQLITE_ERROR;
            goto parse_uri_out;
          }
          if( (mode & ~SQLITE_OPEN_MEMORY)>limit ){
            *pzErrMsg = sqlite3_mprintf("%s mode not allowed: %s",
                                        zModeType, zVal);
            rc = SQLITE_PERM;
            goto parse_uri_out;
          }
          flags = (flags & ~mask) | mode;
        }
      }

      zOpt = &zVal[nVal+1];
    }

  }else{
    zFile = sqlite3_malloc64(nUri+2);
    if( !zFile ) return SQLITE_NOMEM;
    memcpy(zFile, zUri, nUri);
    zFile[nUri] = '\0';
    zFile[nUri+1] = '\0';
    flags &= ~SQLITE_OPEN_URI;
  }

  *ppVfs = sqlite3_vfs_find(zVfs);
  if( *ppVfs==0 ){
    *pzErrMsg = sqlite3_mprintf("no such vfs: %s", zVfs);
    rc = SQLITE_ERROR;
  }
 parse_uri_out:
  if( rc!=SQLITE_OK ){
    sqlite3_free(zFile);
    zFile = 0;
  }
  *pFlags = flags;
  *pzFile = zFile;
  return rc;
}


/*
** This routine does the work of opening a database on behalf of
** sqlite3_open() and sqlite3_open16(). The database filename "zFilename"  
** is UTF-8 encoded.
*/
static int openDatabase(
  const char *zFilename, /* Database filename UTF-8 encoded */
  sqlite3 **ppDb,        /* OUT: Returned database handle */
  unsigned int flags,    /* Operational flags */
  const char *zVfs       /* Name of the VFS to use */
){
  sqlite3 *db;                    /* Store allocated handle here */
  int rc;                         /* Return code */
  int isThreadsafe;               /* True for threadsafe connections */
  char *zOpen = 0;                /* Filename argument to pass to BtreeOpen() */
  char *zErrMsg = 0;              /* Error message from sqlite3ParseUri() */

#ifdef SQLITE_ENABLE_API_ARMOR
  if( ppDb==0 ) return SQLITE_MISUSE_BKPT;
#endif
  *ppDb = 0;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  if( rc ) return rc;
#endif

  /* Only allow sensible combinations of bits in the flags argument.  
  ** Throw an error if any non-sense combination is used.  If we
  ** do not block illegal combinations here, it could trigger
  ** assert() statements in deeper layers.  Sensible combinations
  ** are:
  **
  **  1:  SQLITE_OPEN_READONLY
  **  2:  SQLITE_OPEN_READWRITE
  **  6:  SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
  */
  assert( SQLITE_OPEN_READONLY  == 0x01 );
  assert( SQLITE_OPEN_READWRITE == 0x02 );
  assert( SQLITE_OPEN_CREATE    == 0x04 );
  testcase( (1<<(flags&7))==0x02 ); /* READONLY */
  testcase( (1<<(flags&7))==0x04 ); /* READWRITE */
  testcase( (1<<(flags&7))==0x40 ); /* READWRITE | CREATE */
  if( ((1<<(flags&7)) & 0x46)==0 ){
    return SQLITE_MISUSE_BKPT;  /* IMP: R-65497-44594 */
  }

  if( sqlite3GlobalConfig.bCoreMutex==0 ){
    isThreadsafe = 0;
  }else if( flags & SQLITE_OPEN_NOMUTEX ){
    isThreadsafe = 0;
  }else if( flags & SQLITE_OPEN_FULLMUTEX ){
    isThreadsafe = 1;
  }else{
    isThreadsafe = sqlite3GlobalConfig.bFullMutex;
  }
  if( flags & SQLITE_OPEN_PRIVATECACHE ){
    flags &= ~SQLITE_OPEN_SHAREDCACHE;
  }else if( sqlite3GlobalConfig.sharedCacheEnabled ){
    flags |= SQLITE_OPEN_SHAREDCACHE;
  }

  /* Remove harmful bits from the flags parameter
  **
  ** The SQLITE_OPEN_NOMUTEX and SQLITE_OPEN_FULLMUTEX flags were
  ** dealt with in the previous code block.  Besides these, the only
  ** valid input flags for sqlite3_open_v2() are SQLITE_OPEN_READONLY,
  ** SQLITE_OPEN_READWRITE, SQLITE_OPEN_CREATE, SQLITE_OPEN_SHAREDCACHE,
  ** SQLITE_OPEN_PRIVATECACHE, and some reserved bits.  Silently mask
  ** off all other flags.
  */
  flags &=  ~( SQLITE_OPEN_DELETEONCLOSE |
               SQLITE_OPEN_EXCLUSIVE |
               SQLITE_OPEN_MAIN_DB |
               SQLITE_OPEN_TEMP_DB | 
               SQLITE_OPEN_TRANSIENT_DB | 
               SQLITE_OPEN_MAIN_JOURNAL | 
               SQLITE_OPEN_TEMP_JOURNAL | 
               SQLITE_OPEN_SUBJOURNAL | 
               SQLITE_OPEN_MASTER_JOURNAL |
               SQLITE_OPEN_NOMUTEX |
               SQLITE_OPEN_FULLMUTEX |
               SQLITE_OPEN_WAL
             );

  /* Allocate the sqlite data structure */
  db = sqlite3MallocZero( sizeof(sqlite3) );
  if( db==0 ) goto opendb_out;
  if( isThreadsafe ){
    db->mutex = sqlite3MutexAlloc(SQLITE_MUTEX_RECURSIVE);
    if( db->mutex==0 ){
      sqlite3_free(db);
      db = 0;
      goto opendb_out;
    }
  }
  sqlite3_mutex_enter(db->mutex);
  db->errMask = 0xff;
  db->nDb = 2;
  db->magic = SQLITE_MAGIC_BUSY;
  db->aDb = db->aDbStatic;

  assert( sizeof(db->aLimit)==sizeof(aHardLimit) );
  memcpy(db->aLimit, aHardLimit, sizeof(db->aLimit));
  db->aLimit[SQLITE_LIMIT_WORKER_THREADS] = SQLITE_DEFAULT_WORKER_THREADS;
  db->autoCommit = 1;
  db->nextAutovac = -1;
  db->szMmap = sqlite3GlobalConfig.szMmap;
  db->nextPagesize = 0;
  db->nMaxSorterMmap = 0x7FFFFFFF;
  db->flags |= SQLITE_ShortColNames | SQLITE_EnableTrigger | SQLITE_CacheSpill
#if !defined(SQLITE_DEFAULT_AUTOMATIC_INDEX) || SQLITE_DEFAULT_AUTOMATIC_INDEX
                 | SQLITE_AutoIndex
#endif
#if SQLITE_DEFAULT_CKPTFULLFSYNC
                 | SQLITE_CkptFullFSync
#endif
#if SQLITE_DEFAULT_FILE_FORMAT<4
                 | SQLITE_LegacyFileFmt
#endif
#ifdef SQLITE_ENABLE_LOAD_EXTENSION
                 | SQLITE_LoadExtension
#endif
#if SQLITE_DEFAULT_RECURSIVE_TRIGGERS
                 | SQLITE_RecTriggers
#endif
#if defined(SQLITE_DEFAULT_FOREIGN_KEYS) && SQLITE_DEFAULT_FOREIGN_KEYS
                 | SQLITE_ForeignKeys
#endif
#if defined(SQLITE_REVERSE_UNORDERED_SELECTS)
                 | SQLITE_ReverseOrder
#endif
#if defined(SQLITE_ENABLE_OVERSIZE_CELL_CHECK)
                 | SQLITE_CellSizeCk
#endif
      ;
  sqlite3HashInit(&db->aCollSeq);
#ifndef SQLITE_OMIT_VIRTUALTABLE
  sqlite3HashInit(&db->aModule);
#endif

  /* Add the default collation sequence BINARY. BINARY works for both UTF-8
  ** and UTF-16, so add a version for each to avoid any unnecessary
  ** conversions. The only error that can occur here is a malloc() failure.
  **
  ** EVIDENCE-OF: R-52786-44878 SQLite defines three built-in collating
  ** functions:
  */
  createCollation(db, "BINARY", SQLITE_UTF8, 0, binCollFunc, 0);
  createCollation(db, "BINARY", SQLITE_UTF16BE, 0, binCollFunc, 0);
  createCollation(db, "BINARY", SQLITE_UTF16LE, 0, binCollFunc, 0);
  createCollation(db, "NOCASE", SQLITE_UTF8, 0, nocaseCollatingFunc, 0);
  createCollation(db, "RTRIM", SQLITE_UTF8, (void*)1, binCollFunc, 0);
  if( db->mallocFailed ){
    goto opendb_out;
  }
  /* EVIDENCE-OF: R-08308-17224 The default collating function for all
  ** strings is BINARY. 
  */
  db->pDfltColl = sqlite3FindCollSeq(db, SQLITE_UTF8, "BINARY", 0);
  assert( db->pDfltColl!=0 );

  /* Parse the filename/URI argument. */
  db->openFlags = flags;
  rc = sqlite3ParseUri(zVfs, zFilename, &flags, &db->pVfs, &zOpen, &zErrMsg);
  if( rc!=SQLITE_OK ){
    if( rc==SQLITE_NOMEM ) db->mallocFailed = 1;
    sqlite3ErrorWithMsg(db, rc, zErrMsg ? "%s" : 0, zErrMsg);
    sqlite3_free(zErrMsg);
    goto opendb_out;
  }

  /* Open the backend database driver */
  rc = sqlite3BtreeOpen(db->pVfs, zOpen, db, &db->aDb[0].pBt, 0,
                        flags | SQLITE_OPEN_MAIN_DB);
  if( rc!=SQLITE_OK ){
    if( rc==SQLITE_IOERR_NOMEM ){
      rc = SQLITE_NOMEM;
    }
    sqlite3Error(db, rc);
    goto opendb_out;
  }
  sqlite3BtreeEnter(db->aDb[0].pBt);
  db->aDb[0].pSchema = sqlite3SchemaGet(db, db->aDb[0].pBt);
  if( !db->mallocFailed ) ENC(db) = SCHEMA_ENC(db);
  sqlite3BtreeLeave(db->aDb[0].pBt);
  db->aDb[1].pSchema = sqlite3SchemaGet(db, 0);

  /* The default safety_level for the main database is 'full'; for the temp
  ** database it is 'NONE'. This matches the pager layer defaults.  
  */
  db->aDb[0].zName = "main";
  db->aDb[0].safety_level = 3;
  db->aDb[1].zName = "temp";
  db->aDb[1].safety_level = 1;

  db->magic = SQLITE_MAGIC_OPEN;
  if( db->mallocFailed ){
    goto opendb_out;
  }

  /* Register all built-in functions, but do not attempt to read the
  ** database schema yet. This is delayed until the first time the database
  ** is accessed.
  */
  sqlite3Error(db, SQLITE_OK);
  sqlite3RegisterBuiltinFunctions(db);

  /* Load automatic extensions - extensions that have been registered
  ** using the sqlite3_automatic_extension() API.
  */
  rc = sqlite3_errcode(db);
  if( rc==SQLITE_OK ){
    sqlite3AutoLoadExtensions(db);
    rc = sqlite3_errcode(db);
    if( rc!=SQLITE_OK ){
      goto opendb_out;
    }
  }

#ifdef SQLITE_ENABLE_FTS1
  if( !db->mallocFailed ){
    extern int sqlite3Fts1Init(sqlite3*);
    rc = sqlite3Fts1Init(db);
  }
#endif

#ifdef SQLITE_ENABLE_FTS2
  if( !db->mallocFailed && rc==SQLITE_OK ){
    extern int sqlite3Fts2Init(sqlite3*);
    rc = sqlite3Fts2Init(db);
  }
#endif

#ifdef SQLITE_ENABLE_FTS3
  if( !db->mallocFailed && rc==SQLITE_OK ){
    rc = sqlite3Fts3Init(db);
  }
#endif

#ifdef SQLITE_ENABLE_ICU
  if( !db->mallocFailed && rc==SQLITE_OK ){
    rc = sqlite3IcuInit(db);
  }
#endif

#ifdef SQLITE_ENABLE_RTREE
  if( !db->mallocFailed && rc==SQLITE_OK){
    rc = sqlite3RtreeInit(db);
  }
#endif

#ifdef SQLITE_ENABLE_DBSTAT_VTAB
  if( !db->mallocFailed && rc==SQLITE_OK){
    rc = sqlite3DbstatRegister(db);
  }
#endif

  /* -DSQLITE_DEFAULT_LOCKING_MODE=1 makes EXCLUSIVE the default locking
  ** mode.  -DSQLITE_DEFAULT_LOCKING_MODE=0 make NORMAL the default locking
  ** mode.  Doing nothing at all also makes NORMAL the default.
  */
#ifdef SQLITE_DEFAULT_LOCKING_MODE
  db->dfltLockMode = SQLITE_DEFAULT_LOCKING_MODE;
  sqlite3PagerLockingMode(sqlite3BtreePager(db->aDb[0].pBt),
                          SQLITE_DEFAULT_LOCKING_MODE);
#endif

  if( rc ) sqlite3Error(db, rc);

  /* Enable the lookaside-malloc subsystem */
  setupLookaside(db, 0, sqlite3GlobalConfig.szLookaside,
                        sqlite3GlobalConfig.nLookaside);

  sqlite3_wal_autocheckpoint(db, SQLITE_DEFAULT_WAL_AUTOCHECKPOINT);

opendb_out:
  sqlite3_free(zOpen);
  if( db ){
    assert( db->mutex!=0 || isThreadsafe==0
           || sqlite3GlobalConfig.bFullMutex==0 );
    sqlite3_mutex_leave(db->mutex);
  }
  rc = sqlite3_errcode(db);
  assert( db!=0 || rc==SQLITE_NOMEM );
  if( rc==SQLITE_NOMEM ){
    sqlite3_close(db);
    db = 0;
  }else if( rc!=SQLITE_OK ){
    db->magic = SQLITE_MAGIC_SICK;
  }
  *ppDb = db;
#ifdef SQLITE_ENABLE_SQLLOG
  if( sqlite3GlobalConfig.xSqllog ){
    /* Opening a db handle. Fourth parameter is passed 0. */
    void *pArg = sqlite3GlobalConfig.pSqllogArg;
    sqlite3GlobalConfig.xSqllog(pArg, db, zFilename, 0);
  }
#endif
  return rc & 0xff;
}

/*
** Open a new database handle.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_open(
  const char *zFilename, 
  sqlite3 **ppDb 
){
  return openDatabase(zFilename, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
}
SQLITE_API int SQLITE_STDCALL sqlite3_open_v2(
  const char *filename,   /* Database filename (UTF-8) */
  sqlite3 **ppDb,         /* OUT: SQLite db handle */
  int flags,              /* Flags */
  const char *zVfs        /* Name of VFS module to use */
){
  return openDatabase(filename, ppDb, (unsigned int)flags, zVfs);
}

#ifndef SQLITE_OMIT_UTF16
/*
** Open a new database handle.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_open16(
  const void *zFilename, 
  sqlite3 **ppDb
){
  char const *zFilename8;   /* zFilename encoded in UTF-8 instead of UTF-16 */
  sqlite3_value *pVal;
  int rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( ppDb==0 ) return SQLITE_MISUSE_BKPT;
#endif
  *ppDb = 0;
#ifndef SQLITE_OMIT_AUTOINIT
  rc = sqlite3_initialize();
  if( rc ) return rc;
#endif
  if( zFilename==0 ) zFilename = "\000\000";
  pVal = sqlite3ValueNew(0);
  sqlite3ValueSetStr(pVal, -1, zFilename, SQLITE_UTF16NATIVE, SQLITE_STATIC);
  zFilename8 = sqlite3ValueText(pVal, SQLITE_UTF8);
  if( zFilename8 ){
    rc = openDatabase(zFilename8, ppDb,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, 0);
    assert( *ppDb || rc==SQLITE_NOMEM );
    if( rc==SQLITE_OK && !DbHasProperty(*ppDb, 0, DB_SchemaLoaded) ){
      SCHEMA_ENC(*ppDb) = ENC(*ppDb) = SQLITE_UTF16NATIVE;
    }
  }else{
    rc = SQLITE_NOMEM;
  }
  sqlite3ValueFree(pVal);

  return rc & 0xff;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Register a new collation sequence with the database handle db.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation(
  sqlite3* db, 
  const char *zName, 
  int enc, 
  void* pCtx,
  int(*xCompare)(void*,int,const void*,int,const void*)
){
  return sqlite3_create_collation_v2(db, zName, enc, pCtx, xCompare, 0);
}

/*
** Register a new collation sequence with the database handle db.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation_v2(
  sqlite3* db, 
  const char *zName, 
  int enc, 
  void* pCtx,
  int(*xCompare)(void*,int,const void*,int,const void*),
  void(*xDel)(void*)
){
  int rc;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) || zName==0 ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  rc = createCollation(db, zName, (u8)enc, pCtx, xCompare, xDel);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

#ifndef SQLITE_OMIT_UTF16
/*
** Register a new collation sequence with the database handle db.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_create_collation16(
  sqlite3* db, 
  const void *zName,
  int enc, 
  void* pCtx,
  int(*xCompare)(void*,int,const void*,int,const void*)
){
  int rc = SQLITE_OK;
  char *zName8;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) || zName==0 ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  assert( !db->mallocFailed );
  zName8 = sqlite3Utf16to8(db, zName, -1, SQLITE_UTF16NATIVE);
  if( zName8 ){
    rc = createCollation(db, zName8, (u8)enc, pCtx, xCompare, 0);
    sqlite3DbFree(db, zName8);
  }
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}
#endif /* SQLITE_OMIT_UTF16 */

/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_collation_needed(
  sqlite3 *db, 
  void *pCollNeededArg, 
  void(*xCollNeeded)(void*,sqlite3*,int eTextRep,const char*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = xCollNeeded;
  db->xCollNeeded16 = 0;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

#ifndef SQLITE_OMIT_UTF16
/*
** Register a collation sequence factory callback with the database handle
** db. Replace any previously installed collation sequence factory.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_collation_needed16(
  sqlite3 *db, 
  void *pCollNeededArg, 
  void(*xCollNeeded16)(void*,sqlite3*,int eTextRep,const void*)
){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->xCollNeeded = 0;
  db->xCollNeeded16 = xCollNeeded16;
  db->pCollNeededArg = pCollNeededArg;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}
#endif /* SQLITE_OMIT_UTF16 */

#ifndef SQLITE_OMIT_DEPRECATED
/*
** This function is now an anachronism. It used to be used to recover from a
** malloc() failure, but SQLite now does this automatically.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_global_recover(void){
  return SQLITE_OK;
}
#endif

/*
** Test to see whether or not the database connection is in autocommit
** mode.  Return TRUE if it is and FALSE if not.  Autocommit mode is on
** by default.  Autocommit is disabled by a BEGIN statement and reenabled
** by the next COMMIT or ROLLBACK.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_get_autocommit(sqlite3 *db){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  return db->autoCommit;
}

/*
** The following routines are substitutes for constants SQLITE_CORRUPT,
** SQLITE_MISUSE, SQLITE_CANTOPEN, SQLITE_IOERR and possibly other error
** constants.  They serve two purposes:
**
**   1.  Serve as a convenient place to set a breakpoint in a debugger
**       to detect when version error conditions occurs.
**
**   2.  Invoke sqlite3_log() to provide the source code location where
**       a low-level error is first detected.
*/
SQLITE_PRIVATE int sqlite3CorruptError(int lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  sqlite3_log(SQLITE_CORRUPT,
              "database corruption at line %d of [%.10s]",
              lineno, 20+sqlite3_sourceid());
  return SQLITE_CORRUPT;
}
SQLITE_PRIVATE int sqlite3MisuseError(int lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  sqlite3_log(SQLITE_MISUSE, 
              "misuse at line %d of [%.10s]",
              lineno, 20+sqlite3_sourceid());
  return SQLITE_MISUSE;
}
SQLITE_PRIVATE int sqlite3CantopenError(int lineno){
  testcase( sqlite3GlobalConfig.xLog!=0 );
  sqlite3_log(SQLITE_CANTOPEN, 
              "cannot open file at line %d of [%.10s]",
              lineno, 20+sqlite3_sourceid());
  return SQLITE_CANTOPEN;
}


#ifndef SQLITE_OMIT_DEPRECATED
/*
** This is a convenience routine that makes sure that all thread-specific
** data for this thread has been deallocated.
**
** SQLite no longer uses thread-specific data so this routine is now a
** no-op.  It is retained for historical compatibility.
*/
SQLITE_API void SQLITE_STDCALL sqlite3_thread_cleanup(void){
}
#endif

/*
** Return meta information about a specific column of a database table.
** See comment in sqlite3.h (sqlite.h.in) for details.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_table_column_metadata(
  sqlite3 *db,                /* Connection handle */
  const char *zDbName,        /* Database name or NULL */
  const char *zTableName,     /* Table name */
  const char *zColumnName,    /* Column name */
  char const **pzDataType,    /* OUTPUT: Declared data type */
  char const **pzCollSeq,     /* OUTPUT: Collation sequence name */
  int *pNotNull,              /* OUTPUT: True if NOT NULL constraint exists */
  int *pPrimaryKey,           /* OUTPUT: True if column part of PK */
  int *pAutoinc               /* OUTPUT: True if column is auto-increment */
){
  int rc;
  char *zErrMsg = 0;
  Table *pTab = 0;
  Column *pCol = 0;
  int iCol = 0;
  char const *zDataType = 0;
  char const *zCollSeq = 0;
  int notnull = 0;
  int primarykey = 0;
  int autoinc = 0;


#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) || zTableName==0 ){
    return SQLITE_MISUSE_BKPT;
  }
#endif

  /* Ensure the database schema has been loaded */
  sqlite3_mutex_enter(db->mutex);
  sqlite3BtreeEnterAll(db);
  rc = sqlite3Init(db, &zErrMsg);
  if( SQLITE_OK!=rc ){
    goto error_out;
  }

  /* Locate the table in question */
  pTab = sqlite3FindTable(db, zTableName, zDbName);
  if( !pTab || pTab->pSelect ){
    pTab = 0;
    goto error_out;
  }

  /* Find the column for which info is requested */
  if( zColumnName==0 ){
    /* Query for existance of table only */
  }else{
    for(iCol=0; iCol<pTab->nCol; iCol++){
      pCol = &pTab->aCol[iCol];
      if( 0==sqlite3StrICmp(pCol->zName, zColumnName) ){
        break;
      }
    }
    if( iCol==pTab->nCol ){
      if( HasRowid(pTab) && sqlite3IsRowid(zColumnName) ){
        iCol = pTab->iPKey;
        pCol = iCol>=0 ? &pTab->aCol[iCol] : 0;
      }else{
        pTab = 0;
        goto error_out;
      }
    }
  }

  /* The following block stores the meta information that will be returned
  ** to the caller in local variables zDataType, zCollSeq, notnull, primarykey
  ** and autoinc. At this point there are two possibilities:
  ** 
  **     1. The specified column name was rowid", "oid" or "_rowid_" 
  **        and there is no explicitly declared IPK column. 
  **
  **     2. The table is not a view and the column name identified an 
  **        explicitly declared column. Copy meta information from *pCol.
  */ 
  if( pCol ){
    zDataType = pCol->zType;
    zCollSeq = pCol->zColl;
    notnull = pCol->notNull!=0;
    primarykey  = (pCol->colFlags & COLFLAG_PRIMKEY)!=0;
    autoinc = pTab->iPKey==iCol && (pTab->tabFlags & TF_Autoincrement)!=0;
  }else{
    zDataType = "INTEGER";
    primarykey = 1;
  }
  if( !zCollSeq ){
    zCollSeq = "BINARY";
  }

error_out:
  sqlite3BtreeLeaveAll(db);

  /* Whether the function call succeeded or failed, set the output parameters
  ** to whatever their local counterparts contain. If an error did occur,
  ** this has the effect of zeroing all output parameters.
  */
  if( pzDataType ) *pzDataType = zDataType;
  if( pzCollSeq ) *pzCollSeq = zCollSeq;
  if( pNotNull ) *pNotNull = notnull;
  if( pPrimaryKey ) *pPrimaryKey = primarykey;
  if( pAutoinc ) *pAutoinc = autoinc;

  if( SQLITE_OK==rc && !pTab ){
    sqlite3DbFree(db, zErrMsg);
    zErrMsg = sqlite3MPrintf(db, "no such table column: %s.%s", zTableName,
        zColumnName);
    rc = SQLITE_ERROR;
  }
  sqlite3ErrorWithMsg(db, rc, (zErrMsg?"%s":0), zErrMsg);
  sqlite3DbFree(db, zErrMsg);
  rc = sqlite3ApiExit(db, rc);
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

/*
** Sleep for a little while.  Return the amount of time slept.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_sleep(int ms){
  sqlite3_vfs *pVfs;
  int rc;
  pVfs = sqlite3_vfs_find(0);
  if( pVfs==0 ) return 0;

  /* This function works in milliseconds, but the underlying OsSleep() 
  ** API uses microseconds. Hence the 1000's.
  */
  rc = (sqlite3OsSleep(pVfs, 1000*ms)/1000);
  return rc;
}

/*
** Enable or disable the extended result codes.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_extended_result_codes(sqlite3 *db, int onoff){
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  db->errMask = onoff ? 0xffffffff : 0xff;
  sqlite3_mutex_leave(db->mutex);
  return SQLITE_OK;
}

/*
** Invoke the xFileControl method on a particular database.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_file_control(sqlite3 *db, const char *zDbName, int op, void *pArg){
  int rc = SQLITE_ERROR;
  Btree *pBtree;

#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ) return SQLITE_MISUSE_BKPT;
#endif
  sqlite3_mutex_enter(db->mutex);
  pBtree = sqlite3DbNameToBtree(db, zDbName);
  if( pBtree ){
    Pager *pPager;
    sqlite3_file *fd;
    sqlite3BtreeEnter(pBtree);
    pPager = sqlite3BtreePager(pBtree);
    assert( pPager!=0 );
    fd = sqlite3PagerFile(pPager);
    assert( fd!=0 );
    if( op==SQLITE_FCNTL_FILE_POINTER ){
      *(sqlite3_file**)pArg = fd;
      rc = SQLITE_OK;
    }else if( fd->pMethods ){
      rc = sqlite3OsFileControl(fd, op, pArg);
    }else{
      rc = SQLITE_NOTFOUND;
    }
    sqlite3BtreeLeave(pBtree);
  }
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

/*
** Interface to the testing logic.
*/
SQLITE_API int SQLITE_CDECL sqlite3_test_control(int op, ...){
  int rc = 0;
#ifdef SQLITE_OMIT_BUILTIN_TEST
  UNUSED_PARAMETER(op);
#else
  va_list ap;
  va_start(ap, op);
  switch( op ){

    /*
    ** Save the current state of the PRNG.
    */
    case SQLITE_TESTCTRL_PRNG_SAVE: {
      sqlite3PrngSaveState();
      break;
    }

    /*
    ** Restore the state of the PRNG to the last state saved using
    ** PRNG_SAVE.  If PRNG_SAVE has never before been called, then
    ** this verb acts like PRNG_RESET.
    */
    case SQLITE_TESTCTRL_PRNG_RESTORE: {
      sqlite3PrngRestoreState();
      break;
    }

    /*
    ** Reset the PRNG back to its uninitialized state.  The next call
    ** to sqlite3_randomness() will reseed the PRNG using a single call
    ** to the xRandomness method of the default VFS.
    */
    case SQLITE_TESTCTRL_PRNG_RESET: {
      sqlite3_randomness(0,0);
      break;
    }

    /*
    **  sqlite3_test_control(BITVEC_TEST, size, program)
    **
    ** Run a test against a Bitvec object of size.  The program argument
    ** is an array of integers that defines the test.  Return -1 on a
    ** memory allocation error, 0 on success, or non-zero for an error.
    ** See the sqlite3BitvecBuiltinTest() for additional information.
    */
    case SQLITE_TESTCTRL_BITVEC_TEST: {
      int sz = va_arg(ap, int);
      int *aProg = va_arg(ap, int*);
      rc = sqlite3BitvecBuiltinTest(sz, aProg);
      break;
    }

    /*
    **  sqlite3_test_control(FAULT_INSTALL, xCallback)
    **
    ** Arrange to invoke xCallback() whenever sqlite3FaultSim() is called,
    ** if xCallback is not NULL.
    **
    ** As a test of the fault simulator mechanism itself, sqlite3FaultSim(0)
    ** is called immediately after installing the new callback and the return
    ** value from sqlite3FaultSim(0) becomes the return from
    ** sqlite3_test_control().
    */
    case SQLITE_TESTCTRL_FAULT_INSTALL: {
      /* MSVC is picky about pulling func ptrs from va lists.
      ** http://support.microsoft.com/kb/47961
      ** sqlite3GlobalConfig.xTestCallback = va_arg(ap, int(*)(int));
      */
      typedef int(*TESTCALLBACKFUNC_t)(int);
      sqlite3GlobalConfig.xTestCallback = va_arg(ap, TESTCALLBACKFUNC_t);
      rc = sqlite3FaultSim(0);
      break;
    }

    /*
    **  sqlite3_test_control(BENIGN_MALLOC_HOOKS, xBegin, xEnd)
    **
    ** Register hooks to call to indicate which malloc() failures 
    ** are benign.
    */
    case SQLITE_TESTCTRL_BENIGN_MALLOC_HOOKS: {
      typedef void (*void_function)(void);
      void_function xBenignBegin;
      void_function xBenignEnd;
      xBenignBegin = va_arg(ap, void_function);
      xBenignEnd = va_arg(ap, void_function);
      sqlite3BenignMallocHooks(xBenignBegin, xBenignEnd);
      break;
    }

    /*
    **  sqlite3_test_control(SQLITE_TESTCTRL_PENDING_BYTE, unsigned int X)
    **
    ** Set the PENDING byte to the value in the argument, if X>0.
    ** Make no changes if X==0.  Return the value of the pending byte
    ** as it existing before this routine was called.
    **
    ** IMPORTANT:  Changing the PENDING byte from 0x40000000 results in
    ** an incompatible database file format.  Changing the PENDING byte
    ** while any database connection is open results in undefined and
    ** deleterious behavior.
    */
    case SQLITE_TESTCTRL_PENDING_BYTE: {
      rc = PENDING_BYTE;
#ifndef SQLITE_OMIT_WSD
      {
        unsigned int newVal = va_arg(ap, unsigned int);
        if( newVal ) sqlite3PendingByte = newVal;
      }
#endif
      break;
    }

    /*
    **  sqlite3_test_control(SQLITE_TESTCTRL_ASSERT, int X)
    **
    ** This action provides a run-time test to see whether or not
    ** assert() was enabled at compile-time.  If X is true and assert()
    ** is enabled, then the return value is true.  If X is true and
    ** assert() is disabled, then the return value is zero.  If X is
    ** false and assert() is enabled, then the assertion fires and the
    ** process aborts.  If X is false and assert() is disabled, then the
    ** return value is zero.
    */
    case SQLITE_TESTCTRL_ASSERT: {
      volatile int x = 0;
      assert( (x = va_arg(ap,int))!=0 );
      rc = x;
      break;
    }


    /*
    **  sqlite3_test_control(SQLITE_TESTCTRL_ALWAYS, int X)
    **
    ** This action provides a run-time test to see how the ALWAYS and
    ** NEVER macros were defined at compile-time.
    **
    ** The return value is ALWAYS(X).  
    **
    ** The recommended test is X==2.  If the return value is 2, that means
    ** ALWAYS() and NEVER() are both no-op pass-through macros, which is the
    ** default setting.  If the return value is 1, then ALWAYS() is either
    ** hard-coded to true or else it asserts if its argument is false.
    ** The first behavior (hard-coded to true) is the case if
    ** SQLITE_TESTCTRL_ASSERT shows that assert() is disabled and the second
    ** behavior (assert if the argument to ALWAYS() is false) is the case if
    ** SQLITE_TESTCTRL_ASSERT shows that assert() is enabled.
    **
    ** The run-time test procedure might look something like this:
    **
    **    if( sqlite3_test_control(SQLITE_TESTCTRL_ALWAYS, 2)==2 ){
    **      // ALWAYS() and NEVER() are no-op pass-through macros
    **    }else if( sqlite3_test_control(SQLITE_TESTCTRL_ASSERT, 1) ){
    **      // ALWAYS(x) asserts that x is true. NEVER(x) asserts x is false.
    **    }else{
    **      // ALWAYS(x) is a constant 1.  NEVER(x) is a constant 0.
    **    }
    */
    case SQLITE_TESTCTRL_ALWAYS: {
      int x = va_arg(ap,int);
      rc = ALWAYS(x);
      break;
    }

    /*
    **   sqlite3_test_control(SQLITE_TESTCTRL_BYTEORDER);
    **
    ** The integer returned reveals the byte-order of the computer on which
    ** SQLite is running:
    **
    **       1     big-endian,    determined at run-time
    **      10     little-endian, determined at run-time
    **  432101     big-endian,    determined at compile-time
    **  123410     little-endian, determined at compile-time
    */ 
    case SQLITE_TESTCTRL_BYTEORDER: {
      rc = SQLITE_BYTEORDER*100 + SQLITE_LITTLEENDIAN*10 + SQLITE_BIGENDIAN;
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_RESERVE, sqlite3 *db, int N)
    **
    ** Set the nReserve size to N for the main database on the database
    ** connection db.
    */
    case SQLITE_TESTCTRL_RESERVE: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      int x = va_arg(ap,int);
      sqlite3_mutex_enter(db->mutex);
      sqlite3BtreeSetPageSize(db->aDb[0].pBt, 0, x, 0);
      sqlite3_mutex_leave(db->mutex);
      break;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_OPTIMIZATIONS, sqlite3 *db, int N)
    **
    ** Enable or disable various optimizations for testing purposes.  The 
    ** argument N is a bitmask of optimizations to be disabled.  For normal
    ** operation N should be 0.  The idea is that a test program (like the
    ** SQL Logic Test or SLT test module) can run the same SQL multiple times
    ** with various optimizations disabled to verify that the same answer
    ** is obtained in every case.
    */
    case SQLITE_TESTCTRL_OPTIMIZATIONS: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->dbOptFlags = (u16)(va_arg(ap, int) & 0xffff);
      break;
    }

#ifdef SQLITE_N_KEYWORD
    /* sqlite3_test_control(SQLITE_TESTCTRL_ISKEYWORD, const char *zWord)
    **
    ** If zWord is a keyword recognized by the parser, then return the
    ** number of keywords.  Or if zWord is not a keyword, return 0.
    ** 
    ** This test feature is only available in the amalgamation since
    ** the SQLITE_N_KEYWORD macro is not defined in this file if SQLite
    ** is built using separate source files.
    */
    case SQLITE_TESTCTRL_ISKEYWORD: {
      const char *zWord = va_arg(ap, const char*);
      int n = sqlite3Strlen30(zWord);
      rc = (sqlite3KeywordCode((u8*)zWord, n)!=TK_ID) ? SQLITE_N_KEYWORD : 0;
      break;
    }
#endif 

    /* sqlite3_test_control(SQLITE_TESTCTRL_SCRATCHMALLOC, sz, &pNew, pFree);
    **
    ** Pass pFree into sqlite3ScratchFree(). 
    ** If sz>0 then allocate a scratch buffer into pNew.  
    */
    case SQLITE_TESTCTRL_SCRATCHMALLOC: {
      void *pFree, **ppNew;
      int sz;
      sz = va_arg(ap, int);
      ppNew = va_arg(ap, void**);
      pFree = va_arg(ap, void*);
      if( sz ) *ppNew = sqlite3ScratchMalloc(sz);
      sqlite3ScratchFree(pFree);
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_LOCALTIME_FAULT, int onoff);
    **
    ** If parameter onoff is non-zero, configure the wrappers so that all
    ** subsequent calls to localtime() and variants fail. If onoff is zero,
    ** undo this setting.
    */
    case SQLITE_TESTCTRL_LOCALTIME_FAULT: {
      sqlite3GlobalConfig.bLocaltimeFault = va_arg(ap, int);
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_NEVER_CORRUPT, int);
    **
    ** Set or clear a flag that indicates that the database file is always well-
    ** formed and never corrupt.  This flag is clear by default, indicating that
    ** database files might have arbitrary corruption.  Setting the flag during
    ** testing causes certain assert() statements in the code to be activated
    ** that demonstrat invariants on well-formed database files.
    */
    case SQLITE_TESTCTRL_NEVER_CORRUPT: {
      sqlite3GlobalConfig.neverCorrupt = va_arg(ap, int);
      break;
    }


    /*   sqlite3_test_control(SQLITE_TESTCTRL_VDBE_COVERAGE, xCallback, ptr);
    **
    ** Set the VDBE coverage callback function to xCallback with context 
    ** pointer ptr.
    */
    case SQLITE_TESTCTRL_VDBE_COVERAGE: {
#ifdef SQLITE_VDBE_COVERAGE
      typedef void (*branch_callback)(void*,int,u8,u8);
      sqlite3GlobalConfig.xVdbeBranch = va_arg(ap,branch_callback);
      sqlite3GlobalConfig.pVdbeBranchArg = va_arg(ap,void*);
#endif
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_SORTER_MMAP, db, nMax); */
    case SQLITE_TESTCTRL_SORTER_MMAP: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      db->nMaxSorterMmap = va_arg(ap, int);
      break;
    }

    /*   sqlite3_test_control(SQLITE_TESTCTRL_ISINIT);
    **
    ** Return SQLITE_OK if SQLite has been initialized and SQLITE_ERROR if
    ** not.
    */
    case SQLITE_TESTCTRL_ISINIT: {
      if( sqlite3GlobalConfig.isInit==0 ) rc = SQLITE_ERROR;
      break;
    }

    /*  sqlite3_test_control(SQLITE_TESTCTRL_IMPOSTER, db, dbName, onOff, tnum);
    **
    ** This test control is used to create imposter tables.  "db" is a pointer
    ** to the database connection.  dbName is the database name (ex: "main" or
    ** "temp") which will receive the imposter.  "onOff" turns imposter mode on
    ** or off.  "tnum" is the root page of the b-tree to which the imposter
    ** table should connect.
    **
    ** Enable imposter mode only when the schema has already been parsed.  Then
    ** run a single CREATE TABLE statement to construct the imposter table in
    ** the parsed schema.  Then turn imposter mode back off again.
    **
    ** If onOff==0 and tnum>0 then reset the schema for all databases, causing
    ** the schema to be reparsed the next time it is needed.  This has the
    ** effect of erasing all imposter tables.
    */
    case SQLITE_TESTCTRL_IMPOSTER: {
      sqlite3 *db = va_arg(ap, sqlite3*);
      sqlite3_mutex_enter(db->mutex);
      db->init.iDb = sqlite3FindDbName(db, va_arg(ap,const char*));
      db->init.busy = db->init.imposterTable = va_arg(ap,int);
      db->init.newTnum = va_arg(ap,int);
      if( db->init.busy==0 && db->init.newTnum>0 ){
        sqlite3ResetAllSchemasOfConnection(db);
      }
      sqlite3_mutex_leave(db->mutex);
      break;
    }
  }
  va_end(ap);
#endif /* SQLITE_OMIT_BUILTIN_TEST */
  return rc;
}

/*
** This is a utility routine, useful to VFS implementations, that checks
** to see if a database file was a URI that contained a specific query 
** parameter, and if so obtains the value of the query parameter.
**
** The zFilename argument is the filename pointer passed into the xOpen()
** method of a VFS implementation.  The zParam argument is the name of the
** query parameter we seek.  This routine returns the value of the zParam
** parameter if it exists.  If the parameter does not exist, this routine
** returns a NULL pointer.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_uri_parameter(const char *zFilename, const char *zParam){
  if( zFilename==0 || zParam==0 ) return 0;
  zFilename += sqlite3Strlen30(zFilename) + 1;
  while( zFilename[0] ){
    int x = strcmp(zFilename, zParam);
    zFilename += sqlite3Strlen30(zFilename) + 1;
    if( x==0 ) return zFilename;
    zFilename += sqlite3Strlen30(zFilename) + 1;
  }
  return 0;
}

/*
** Return a boolean value for a query parameter.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_uri_boolean(const char *zFilename, const char *zParam, int bDflt){
  const char *z = sqlite3_uri_parameter(zFilename, zParam);
  bDflt = bDflt!=0;
  return z ? sqlite3GetBoolean(z, bDflt) : bDflt;
}

/*
** Return a 64-bit integer value for a query parameter.
*/
SQLITE_API sqlite3_int64 SQLITE_STDCALL sqlite3_uri_int64(
  const char *zFilename,    /* Filename as passed to xOpen */
  const char *zParam,       /* URI parameter sought */
  sqlite3_int64 bDflt       /* return if parameter is missing */
){
  const char *z = sqlite3_uri_parameter(zFilename, zParam);
  sqlite3_int64 v;
  if( z && sqlite3DecOrHexToI64(z, &v)==SQLITE_OK ){
    bDflt = v;
  }
  return bDflt;
}

/*
** Return the Btree pointer identified by zDbName.  Return NULL if not found.
*/
SQLITE_PRIVATE Btree *sqlite3DbNameToBtree(sqlite3 *db, const char *zDbName){
  int i;
  for(i=0; i<db->nDb; i++){
    if( db->aDb[i].pBt
     && (zDbName==0 || sqlite3StrICmp(zDbName, db->aDb[i].zName)==0)
    ){
      return db->aDb[i].pBt;
    }
  }
  return 0;
}

/*
** Return the filename of the database associated with a database
** connection.
*/
SQLITE_API const char *SQLITE_STDCALL sqlite3_db_filename(sqlite3 *db, const char *zDbName){
  Btree *pBt;
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return 0;
  }
#endif
  pBt = sqlite3DbNameToBtree(db, zDbName);
  return pBt ? sqlite3BtreeGetFilename(pBt) : 0;
}

/*
** Return 1 if database is read-only or 0 if read/write.  Return -1 if
** no such database exists.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_db_readonly(sqlite3 *db, const char *zDbName){
  Btree *pBt;
#ifdef SQLITE_ENABLE_API_ARMOR
  if( !sqlite3SafetyCheckOk(db) ){
    (void)SQLITE_MISUSE_BKPT;
    return -1;
  }
#endif
  pBt = sqlite3DbNameToBtree(db, zDbName);
  return pBt ? sqlite3BtreeIsReadonly(pBt) : -1;
}

/************** End of main.c ************************************************/
/************** Begin file notify.c ******************************************/
/*
** 2009 March 3
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
**
** This file contains the implementation of the sqlite3_unlock_notify()
** API method and its associated functionality.
*/
/* #include "sqliteInt.h" */
/* #include "btreeInt.h" */

/* Omit this entire file if SQLITE_ENABLE_UNLOCK_NOTIFY is not defined. */
#ifdef SQLITE_ENABLE_UNLOCK_NOTIFY

/*
** Public interfaces:
**
**   sqlite3ConnectionBlocked()
**   sqlite3ConnectionUnlocked()
**   sqlite3ConnectionClosed()
**   sqlite3_unlock_notify()
*/

#define assertMutexHeld() \
  assert( sqlite3_mutex_held(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER)) )

/*
** Head of a linked list of all sqlite3 objects created by this process
** for which either sqlite3.pBlockingConnection or sqlite3.pUnlockConnection
** is not NULL. This variable may only accessed while the STATIC_MASTER
** mutex is held.
*/
static sqlite3 *SQLITE_WSD sqlite3BlockedList = 0;

#ifndef NDEBUG
/*
** This function is a complex assert() that verifies the following 
** properties of the blocked connections list:
**
**   1) Each entry in the list has a non-NULL value for either 
**      pUnlockConnection or pBlockingConnection, or both.
**
**   2) All entries in the list that share a common value for 
**      xUnlockNotify are grouped together.
**
**   3) If the argument db is not NULL, then none of the entries in the
**      blocked connections list have pUnlockConnection or pBlockingConnection
**      set to db. This is used when closing connection db.
*/
static void checkListProperties(sqlite3 *db){
  sqlite3 *p;
  for(p=sqlite3BlockedList; p; p=p->pNextBlocked){
    int seen = 0;
    sqlite3 *p2;

    /* Verify property (1) */
    assert( p->pUnlockConnection || p->pBlockingConnection );

    /* Verify property (2) */
    for(p2=sqlite3BlockedList; p2!=p; p2=p2->pNextBlocked){
      if( p2->xUnlockNotify==p->xUnlockNotify ) seen = 1;
      assert( p2->xUnlockNotify==p->xUnlockNotify || !seen );
      assert( db==0 || p->pUnlockConnection!=db );
      assert( db==0 || p->pBlockingConnection!=db );
    }
  }
}
#else
# define checkListProperties(x)
#endif

/*
** Remove connection db from the blocked connections list. If connection
** db is not currently a part of the list, this function is a no-op.
*/
static void removeFromBlockedList(sqlite3 *db){
  sqlite3 **pp;
  assertMutexHeld();
  for(pp=&sqlite3BlockedList; *pp; pp = &(*pp)->pNextBlocked){
    if( *pp==db ){
      *pp = (*pp)->pNextBlocked;
      break;
    }
  }
}

/*
** Add connection db to the blocked connections list. It is assumed
** that it is not already a part of the list.
*/
static void addToBlockedList(sqlite3 *db){
  sqlite3 **pp;
  assertMutexHeld();
  for(
    pp=&sqlite3BlockedList; 
    *pp && (*pp)->xUnlockNotify!=db->xUnlockNotify; 
    pp=&(*pp)->pNextBlocked
  );
  db->pNextBlocked = *pp;
  *pp = db;
}

/*
** Obtain the STATIC_MASTER mutex.
*/
static void enterMutex(void){
  sqlite3_mutex_enter(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
  checkListProperties(0);
}

/*
** Release the STATIC_MASTER mutex.
*/
static void leaveMutex(void){
  assertMutexHeld();
  checkListProperties(0);
  sqlite3_mutex_leave(sqlite3MutexAlloc(SQLITE_MUTEX_STATIC_MASTER));
}

/*
** Register an unlock-notify callback.
**
** This is called after connection "db" has attempted some operation
** but has received an SQLITE_LOCKED error because another connection
** (call it pOther) in the same process was busy using the same shared
** cache.  pOther is found by looking at db->pBlockingConnection.
**
** If there is no blocking connection, the callback is invoked immediately,
** before this routine returns.
**
** If pOther is already blocked on db, then report SQLITE_LOCKED, to indicate
** a deadlock.
**
** Otherwise, make arrangements to invoke xNotify when pOther drops
** its locks.
**
** Each call to this routine overrides any prior callbacks registered
** on the same "db".  If xNotify==0 then any prior callbacks are immediately
** cancelled.
*/
SQLITE_API int SQLITE_STDCALL sqlite3_unlock_notify(
  sqlite3 *db,
  void (*xNotify)(void **, int),
  void *pArg
){
  int rc = SQLITE_OK;

  sqlite3_mutex_enter(db->mutex);
  enterMutex();

  if( xNotify==0 ){
    removeFromBlockedList(db);
    db->pBlockingConnection = 0;
    db->pUnlockConnection = 0;
    db->xUnlockNotify = 0;
    db->pUnlockArg = 0;
  }else if( 0==db->pBlockingConnection ){
    /* The blocking transaction has been concluded. Or there never was a 
    ** blocking transaction. In either case, invoke the notify callback
    ** immediately. 
    */
    xNotify(&pArg, 1);
  }else{
    sqlite3 *p;

    for(p=db->pBlockingConnection; p && p!=db; p=p->pUnlockConnection){}
    if( p ){
      rc = SQLITE_LOCKED;              /* Deadlock detected. */
    }else{
      db->pUnlockConnection = db->pBlockingConnection;
      db->xUnlockNotify = xNotify;
      db->pUnlockArg = pArg;
      removeFromBlockedList(db);
      addToBlockedList(db);
    }
  }

  leaveMutex();
  assert( !db->mallocFailed );
  sqlite3ErrorWithMsg(db, rc, (rc?"database is deadlocked":0));
  sqlite3_mutex_leave(db->mutex);
  return rc;
}

/*
** This function is called while stepping or preparing a statement 
** associated with connection db. The operation will return SQLITE_LOCKED
** to the user because it requires a lock that will not be available
** until connection pBlocker concludes its current transaction.
*/
SQLITE_PRIVATE void sqlite3ConnectionBlocked(sqlite3 *db, sqlite3 *pBlocker){
  enterMutex();
  if( db->pBlockingConnection==0 && db->pUnlockConnection==0 ){
    addToBlockedList(db);
  }
  db->pBlockingConnection = pBlocker;
  leaveMutex();
}

/*
** This function is called when
** the transaction opened by database db has just finished. Locks held 
** by database connection db have been released.
**
** This function loops through each entry in the blocked connections
** list and does the following:
**
**   1) If the sqlite3.pBlockingConnection member of a list entry is
**      set to db, then set pBlockingConnection=0.
**
**   2) If the sqlite3.pUnlockConnection member of a list entry is
**      set to db, then invoke the configured unlock-notify callback and
**      set pUnlockConnection=0.
**
**   3) If the two steps above mean that pBlockingConnection==0 and
**      pUnlockConnection==0, remove the entry from the blocked connections
**      list.
*/
SQLITE_PRIVATE void sqlite3ConnectionUnlocked(sqlite3 *db){
  void (*xUnlockNotify)(void **, int) = 0; /* Unlock-notify cb to invoke */
  int nArg = 0;                            /* Number of entries in aArg[] */
  sqlite3 **pp;                            /* Iterator variable */
  void **aArg;               /* Arguments to the unlock callback */
  void **aDyn = 0;           /* Dynamically allocated space for aArg[] */
  void *aStatic[16];         /* Starter space for aArg[].  No malloc required */

  aArg = aStatic;
  enterMutex();         /* Enter STATIC_MASTER mutex */

  /* This loop runs once for each entry in the blocked-connections list. */
  for(pp=&sqlite3BlockedList; *pp; /* no-op */ ){
    sqlite3 *p = *pp;

    /* Step 1. */
    if( p->pBlockingConnection==db ){
      p->pBlockingConnection = 0;
    }

    /* Step 2. */
    if( p->pUnlockConnection==db ){
      assert( p->xUnlockNotify );
      if( p->xUnlockNotify!=xUnlockNotify && nArg!=0 ){
        xUnlockNotify(aArg, nArg);
        nArg = 0;
      }

      sqlite3BeginBenignMalloc();
      assert( aArg==aDyn || (aDyn==0 && aArg==aStatic) );
      assert( nArg<=(int)ArraySize(aStatic) || aArg==aDyn );
      if( (!aDyn && nArg==(int)ArraySize(aStatic))
       || (aDyn && nArg==(int)(sqlite3MallocSize(aDyn)/sizeof(void*)))
      ){
        /* The aArg[] array needs to grow. */
        void **pNew = (void **)sqlite3Malloc(nArg*sizeof(void *)*2);
        if( pNew ){
          memcpy(pNew, aArg, nArg*sizeof(void *));
          sqlite3_free(aDyn);
          aDyn = aArg = pNew;
        }else{
          /* This occurs when the array of context pointers that need to
          ** be passed to the unlock-notify callback is larger than the
          ** aStatic[] array allocated on the stack and the attempt to 
          ** allocate a larger array from the heap has failed.
          **
          ** This is a difficult situation to handle. Returning an error
          ** code to the caller is insufficient, as even if an error code
          ** is returned the transaction on connection db will still be
          ** closed and the unlock-notify callbacks on blocked connections
          ** will go unissued. This might cause the application to wait
          ** indefinitely for an unlock-notify callback that will never 
          ** arrive.
          **
          ** Instead, invoke the unlock-notify callback with the context
          ** array already accumulated. We can then clear the array and
          ** begin accumulating any further context pointers without 
          ** requiring any dynamic allocation. This is sub-optimal because
          ** it means that instead of one callback with a large array of
          ** context pointers the application will receive two or more
          ** callbacks with smaller arrays of context pointers, which will
          ** reduce the applications ability to prioritize multiple 
          ** connections. But it is the best that can be done under the
          ** circumstances.
          */
          xUnlockNotify(aArg, nArg);
          nArg = 0;
        }
      }
      sqlite3EndBenignMalloc();

      aArg[nArg++] = p->pUnlockArg;
      xUnlockNotify = p->xUnlockNotify;
      p->pUnlockConnection = 0;
      p->xUnlockNotify = 0;
      p->pUnlockArg = 0;
    }

    /* Step 3. */
    if( p->pBlockingConnection==0 && p->pUnlockConnection==0 ){
      /* Remove connection p from the blocked connections list. */
      *pp = p->pNextBlocked;
      p->pNextBlocked = 0;
    }else{
      pp = &p->pNextBlocked;
    }
  }

  if( nArg!=0 ){
    xUnlockNotify(aArg, nArg);
  }
  sqlite3_free(aDyn);
  leaveMutex();         /* Leave STATIC_MASTER mutex */
}

/*
** This is called when the database connection passed as an argument is 
** being closed. The connection is removed from the blocked list.
*/
SQLITE_PRIVATE void sqlite3ConnectionClosed(sqlite3 *db){
  sqlite3ConnectionUnlocked(db);
  enterMutex();
  removeFromBlockedList(db);
  checkListProperties(db);
  leaveMutex();
}
#endif

/************** End of notify.c **********************************************/
/************** Begin file fts3.c ********************************************/
/*
** 2006 Oct 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This is an SQLite module implementing full-text search.
*/

/*
** The code in this file is only compiled if:
**
**     * The FTS3 module is being built as an extension
**       (in which case SQLITE_CORE is not defined), or
**
**     * The FTS3 module is being built into the core of
**       SQLite (in which case SQLITE_ENABLE_FTS3 is defined).
*/

/* The full-text index is stored in a series of b+tree (-like)
** structures called segments which map terms to doclists.  The
** structures are like b+trees in layout, but are constructed from the
** bottom up in optimal fashion and are not updatable.  Since trees
** are built from the bottom up, things will be described from the
** bottom up.
**
**
**** Varints ****
** The basic unit of encoding is a variable-length integer called a
** varint.  We encode variable-length integers in little-endian order
** using seven bits * per byte as follows:
**
** KEY:
**         A = 0xxxxxxx    7 bits of data and one flag bit
**         B = 1xxxxxxx    7 bits of data and one flag bit
**
**  7 bits - A
** 14 bits - BA
** 21 bits - BBA
** and so on.
**
** This is similar in concept to how sqlite encodes "varints" but
** the encoding is not the same.  SQLite varints are big-endian
** are are limited to 9 bytes in length whereas FTS3 varints are
** little-endian and can be up to 10 bytes in length (in theory).
**
** Example encodings:
**
**     1:    0x01
**   127:    0x7f
**   128:    0x81 0x00
**
**
**** Document lists ****
** A doclist (document list) holds a docid-sorted list of hits for a
** given term.  Doclists hold docids and associated token positions.
** A docid is the unique integer identifier for a single document.
** A position is the index of a word within the document.  The first 
** word of the document has a position of 0.
**
** FTS3 used to optionally store character offsets using a compile-time
** option.  But that functionality is no longer supported.
**
** A doclist is stored like this:
**
** array {
**   varint docid;          (delta from previous doclist)
**   array {                (position list for column 0)
**     varint position;     (2 more than the delta from previous position)
**   }
**   array {
**     varint POS_COLUMN;   (marks start of position list for new column)
**     varint column;       (index of new column)
**     array {
**       varint position;   (2 more than the delta from previous position)
**     }
**   }
**   varint POS_END;        (marks end of positions for this document.
** }
**
** Here, array { X } means zero or more occurrences of X, adjacent in
** memory.  A "position" is an index of a token in the token stream
** generated by the tokenizer. Note that POS_END and POS_COLUMN occur 
** in the same logical place as the position element, and act as sentinals
** ending a position list array.  POS_END is 0.  POS_COLUMN is 1.
** The positions numbers are not stored literally but rather as two more
** than the difference from the prior position, or the just the position plus
** 2 for the first position.  Example:
**
**   label:       A B C D E  F  G H   I  J K
**   value:     123 5 9 1 1 14 35 0 234 72 0
**
** The 123 value is the first docid.  For column zero in this document
** there are two matches at positions 3 and 10 (5-2 and 9-2+3).  The 1
** at D signals the start of a new column; the 1 at E indicates that the
** new column is column number 1.  There are two positions at 12 and 45
** (14-2 and 35-2+12).  The 0 at H indicate the end-of-document.  The
** 234 at I is the delta to next docid (357).  It has one position 70
** (72-2) and then terminates with the 0 at K.
**
** A "position-list" is the list of positions for multiple columns for
** a single docid.  A "column-list" is the set of positions for a single
** column.  Hence, a position-list consists of one or more column-lists,
** a document record consists of a docid followed by a position-list and
** a doclist consists of one or more document records.
**
** A bare doclist omits the position information, becoming an 
** array of varint-encoded docids.
**
**** Segment leaf nodes ****
** Segment leaf nodes store terms and doclists, ordered by term.  Leaf
** nodes are written using LeafWriter, and read using LeafReader (to
** iterate through a single leaf node's data) and LeavesReader (to
** iterate through a segment's entire leaf layer).  Leaf nodes have
** the format:
**
** varint iHeight;             (height from leaf level, always 0)
** varint nTerm;               (length of first term)
** char pTerm[nTerm];          (content of first term)
** varint nDoclist;            (length of term's associated doclist)
** char pDoclist[nDoclist];    (content of doclist)
** array {
**                             (further terms are delta-encoded)
**   varint nPrefix;           (length of prefix shared with previous term)
**   varint nSuffix;           (length of unshared suffix)
**   char pTermSuffix[nSuffix];(unshared suffix of next term)
**   varint nDoclist;          (length of term's associated doclist)
**   char pDoclist[nDoclist];  (content of doclist)
** }
**
** Here, array { X } means zero or more occurrences of X, adjacent in
** memory.
**
** Leaf nodes are broken into blocks which are stored contiguously in
** the %_segments table in sorted order.  This means that when the end
** of a node is reached, the next term is in the node with the next
** greater node id.
**
** New data is spilled to a new leaf node when the current node
** exceeds LEAF_MAX bytes (default 2048).  New data which itself is
** larger than STANDALONE_MIN (default 1024) is placed in a standalone
** node (a leaf node with a single term and doclist).  The goal of
** these settings is to pack together groups of small doclists while
** making it efficient to directly access large doclists.  The
** assumption is that large doclists represent terms which are more
** likely to be query targets.
**
** TODO(shess) It may be useful for blocking decisions to be more
** dynamic.  For instance, it may make more sense to have a 2.5k leaf
** node rather than splitting into 2k and .5k nodes.  My intuition is
** that this might extend through 2x or 4x the pagesize.
**
**
**** Segment interior nodes ****
** Segment interior nodes store blockids for subtree nodes and terms
** to describe what data is stored by the each subtree.  Interior
** nodes are written using InteriorWriter, and read using
** InteriorReader.  InteriorWriters are created as needed when
** SegmentWriter creates new leaf nodes, or when an interior node
** itself grows too big and must be split.  The format of interior
** nodes:
**
** varint iHeight;           (height from leaf level, always >0)
** varint iBlockid;          (block id of node's leftmost subtree)
** optional {
**   varint nTerm;           (length of first term)
**   char pTerm[nTerm];      (content of first term)
**   array {
**                                (further terms are delta-encoded)
**     varint nPrefix;            (length of shared prefix with previous term)
**     varint nSuffix;            (length of unshared suffix)
**     char pTermSuffix[nSuffix]; (unshared suffix of next term)
**   }
** }
**
** Here, optional { X } means an optional element, while array { X }
** means zero or more occurrences of X, adjacent in memory.
**
** An interior node encodes n terms separating n+1 subtrees.  The
** subtree blocks are contiguous, so only the first subtree's blockid
** is encoded.  The subtree at iBlockid will contain all terms less
** than the first term encoded (or all terms if no term is encoded).
** Otherwise, for terms greater than or equal to pTerm[i] but less
** than pTerm[i+1], the subtree for that term will be rooted at
** iBlockid+i.  Interior nodes only store enough term data to
** distinguish adjacent children (if the rightmost term of the left
** child is "something", and the leftmost term of the right child is
** "wicked", only "w" is stored).
**
** New data is spilled to a new interior node at the same height when
** the current node exceeds INTERIOR_MAX bytes (default 2048).
** INTERIOR_MIN_TERMS (default 7) keeps large terms from monopolizing
** interior nodes and making the tree too skinny.  The interior nodes
** at a given height are naturally tracked by interior nodes at
** height+1, and so on.
**
**
**** Segment directory ****
** The segment directory in table %_segdir stores meta-information for
** merging and deleting segments, and also the root node of the
** segment's tree.
**
** The root node is the top node of the segment's tree after encoding
** the entire segment, restricted to ROOT_MAX bytes (default 1024).
** This could be either a leaf node or an interior node.  If the top
** node requires more than ROOT_MAX bytes, it is flushed to %_segments
** and a new root interior node is generated (which should always fit
** within ROOT_MAX because it only needs space for 2 varints, the
** height and the blockid of the previous root).
**
** The meta-information in the segment directory is:
**   level               - segment level (see below)
**   idx                 - index within level
**                       - (level,idx uniquely identify a segment)
**   start_block         - first leaf node
**   leaves_end_block    - last leaf node
**   end_block           - last block (including interior nodes)
**   root                - contents of root node
**
** If the root node is a leaf node, then start_block,
** leaves_end_block, and end_block are all 0.
**
**
**** Segment merging ****
** To amortize update costs, segments are grouped into levels and
** merged in batches.  Each increase in level represents exponentially
** more documents.
**
** New documents (actually, document updates) are tokenized and
** written individually (using LeafWriter) to a level 0 segment, with
** incrementing idx.  When idx reaches MERGE_COUNT (default 16), all
** level 0 segments are merged into a single level 1 segment.  Level 1
** is populated like level 0, and eventually MERGE_COUNT level 1
** segments are merged to a single level 2 segment (representing
** MERGE_COUNT^2 updates), and so on.
**
** A segment merge traverses all segments at a given level in
** parallel, performing a straightforward sorted merge.  Since segment
** leaf nodes are written in to the %_segments table in order, this
** merge traverses the underlying sqlite disk structures efficiently.
** After the merge, all segment blocks from the merged level are
** deleted.
**
** MERGE_COUNT controls how often we merge segments.  16 seems to be
** somewhat of a sweet spot for insertion performance.  32 and 64 show
** very similar performance numbers to 16 on insertion, though they're
** a tiny bit slower (perhaps due to more overhead in merge-time
** sorting).  8 is about 20% slower than 16, 4 about 50% slower than
** 16, 2 about 66% slower than 16.
**
** At query time, high MERGE_COUNT increases the number of segments
** which need to be scanned and merged.  For instance, with 100k docs
** inserted:
**
**    MERGE_COUNT   segments
**       16           25
**        8           12
**        4           10
**        2            6
**
** This appears to have only a moderate impact on queries for very
** frequent terms (which are somewhat dominated by segment merge
** costs), and infrequent and non-existent terms still seem to be fast
** even with many segments.
**
** TODO(shess) That said, it would be nice to have a better query-side
** argument for MERGE_COUNT of 16.  Also, it is possible/likely that
** optimizations to things like doclist merging will swing the sweet
** spot around.
**
**
**
**** Handling of deletions and updates ****
** Since we're using a segmented structure, with no docid-oriented
** index into the term index, we clearly cannot simply update the term
** index when a document is deleted or updated.  For deletions, we
** write an empty doclist (varint(docid) varint(POS_END)), for updates
** we simply write the new doclist.  Segment merges overwrite older
** data for a particular docid with newer data, so deletes or updates
** will eventually overtake the earlier data and knock it out.  The
** query logic likewise merges doclists so that newer data knocks out
** older data.
*/

/************** Include fts3Int.h in the middle of fts3.c ********************/
/************** Begin file fts3Int.h *****************************************/
/*
** 2009 Nov 12
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
*/
#ifndef _FTSINT_H
#define _FTSINT_H

#if !defined(NDEBUG) && !defined(SQLITE_DEBUG) 
# define NDEBUG 1
#endif

/*
** FTS4 is really an extension for FTS3.  It is enabled using the
** SQLITE_ENABLE_FTS3 macro.  But to avoid confusion we also all
** the SQLITE_ENABLE_FTS4 macro to serve as an alisse for SQLITE_ENABLE_FTS3.
*/
#if defined(SQLITE_ENABLE_FTS4) && !defined(SQLITE_ENABLE_FTS3)
# define SQLITE_ENABLE_FTS3
#endif

#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* If not building as part of the core, include sqlite3ext.h. */
#ifndef SQLITE_CORE
/* # include "sqlite3ext.h"  */
SQLITE_EXTENSION_INIT3
#endif

/* #include "sqlite3.h" */
/************** Include fts3_tokenizer.h in the middle of fts3Int.h **********/
/************** Begin file fts3_tokenizer.h **********************************/
/*
** 2006 July 10
**
** The author disclaims copyright to this source code.
**
*************************************************************************
** Defines the interface to tokenizers used by fulltext-search.  There
** are three basic components:
**
** sqlite3_tokenizer_module is a singleton defining the tokenizer
** interface functions.  This is essentially the class structure for
** tokenizers.
**
** sqlite3_tokenizer is used to define a particular tokenizer, perhaps
** including customization information defined at creation time.
**
** sqlite3_tokenizer_cursor is generated by a tokenizer to generate
** tokens from a particular input.
*/
#ifndef _FTS3_TOKENIZER_H_
#define _FTS3_TOKENIZER_H_

/* TODO(shess) Only used for SQLITE_OK and SQLITE_DONE at this time.
** If tokenizers are to be allowed to call sqlite3_*() functions, then
** we will need a way to register the API consistently.
*/
/* #include "sqlite3.h" */

/*
** Structures used by the tokenizer interface. When a new tokenizer
** implementation is registered, the caller provides a pointer to
** an sqlite3_tokenizer_module containing pointers to the callback
** functions that make up an implementation.
**
** When an fts3 table is created, it passes any arguments passed to
** the tokenizer clause of the CREATE VIRTUAL TABLE statement to the
** sqlite3_tokenizer_module.xCreate() function of the requested tokenizer
** implementation. The xCreate() function in turn returns an 
** sqlite3_tokenizer structure representing the specific tokenizer to
** be used for the fts3 table (customized by the tokenizer clause arguments).
**
** To tokenize an input buffer, the sqlite3_tokenizer_module.xOpen()
** method is called. It returns an sqlite3_tokenizer_cursor object
** that may be used to tokenize a specific input buffer based on
** the tokenization rules supplied by a specific sqlite3_tokenizer
** object.
*/
typedef struct sqlite3_tokenizer_module sqlite3_tokenizer_module;
typedef struct sqlite3_tokenizer sqlite3_tokenizer;
typedef struct sqlite3_tokenizer_cursor sqlite3_tokenizer_cursor;

struct sqlite3_tokenizer_module {

  /*
  ** Structure version. Should always be set to 0 or 1.
  */
  int iVersion;

  /*
  ** Create a new tokenizer. The values in the argv[] array are the
  ** arguments passed to the "tokenizer" clause of the CREATE VIRTUAL
  ** TABLE statement that created the fts3 table. For example, if
  ** the following SQL is executed:
  **
  **   CREATE .. USING fts3( ... , tokenizer <tokenizer-name> arg1 arg2)
  **
  ** then argc is set to 2, and the argv[] array contains pointers
  ** to the strings "arg1" and "arg2".
  **
  ** This method should return either SQLITE_OK (0), or an SQLite error 
  ** code. If SQLITE_OK is returned, then *ppTokenizer should be set
  ** to point at the newly created tokenizer structure. The generic
  ** sqlite3_tokenizer.pModule variable should not be initialized by
  ** this callback. The caller will do so.
  */
  int (*xCreate)(
    int argc,                           /* Size of argv array */
    const char *const*argv,             /* Tokenizer argument strings */
    sqlite3_tokenizer **ppTokenizer     /* OUT: Created tokenizer */
  );

  /*
  ** Destroy an existing tokenizer. The fts3 module calls this method
  ** exactly once for each successful call to xCreate().
  */
  int (*xDestroy)(sqlite3_tokenizer *pTokenizer);

  /*
  ** Create a tokenizer cursor to tokenize an input buffer. The caller
  ** is responsible for ensuring that the input buffer remains valid
  ** until the cursor is closed (using the xClose() method). 
  */
  int (*xOpen)(
    sqlite3_tokenizer *pTokenizer,       /* Tokenizer object */
    const char *pInput, int nBytes,      /* Input buffer */
    sqlite3_tokenizer_cursor **ppCursor  /* OUT: Created tokenizer cursor */
  );

  /*
  ** Destroy an existing tokenizer cursor. The fts3 module calls this 
  ** method exactly once for each successful call to xOpen().
  */
  int (*xClose)(sqlite3_tokenizer_cursor *pCursor);

  /*
  ** Retrieve the next token from the tokenizer cursor pCursor. This
  ** method should either return SQLITE_OK and set the values of the
  ** "OUT" variables identified below, or SQLITE_DONE to indicate that
  ** the end of the buffer has been reached, or an SQLite error code.
  **
  ** *ppToken should be set to point at a buffer containing the 
  ** normalized version of the token (i.e. after any case-folding and/or
  ** stemming has been performed). *pnBytes should be set to the length
  ** of this buffer in bytes. The input text that generated the token is
  ** identified by the byte offsets returned in *piStartOffset and
  ** *piEndOffset. *piStartOffset should be set to the index of the first
  ** byte of the token in the input buffer. *piEndOffset should be set
  ** to the index of the first byte just past the end of the token in
  ** the input buffer.
  **
  ** The buffer *ppToken is set to point at is managed by the tokenizer
  ** implementation. It is only required to be valid until the next call
  ** to xNext() or xClose(). 
  */
  /* TODO(shess) current implementation requires pInput to be
  ** nul-terminated.  This should either be fixed, or pInput/nBytes
  ** should be converted to zInput.
  */
  int (*xNext)(
    sqlite3_tokenizer_cursor *pCursor,   /* Tokenizer cursor */
    const char **ppToken, int *pnBytes,  /* OUT: Normalized text for token */
    int *piStartOffset,  /* OUT: Byte offset of token in input buffer */
    int *piEndOffset,    /* OUT: Byte offset of end of token in input buffer */
    int *piPosition      /* OUT: Number of tokens returned before this one */
  );

  /***********************************************************************
  ** Methods below this point are only available if iVersion>=1.
  */

  /* 
  ** Configure the language id of a tokenizer cursor.
  */
  int (*xLanguageid)(sqlite3_tokenizer_cursor *pCsr, int iLangid);
};

struct sqlite3_tokenizer {
  const sqlite3_tokenizer_module *pModule;  /* The module for this tokenizer */
  /* Tokenizer implementations will typically add additional fields */
};

struct sqlite3_tokenizer_cursor {
  sqlite3_tokenizer *pTokenizer;       /* Tokenizer for this cursor. */
  /* Tokenizer implementations will typically add additional fields */
};

int fts3_global_term_cnt(int iTerm, int iCol);
int fts3_term_cnt(int iTerm, int iCol);


#endif /* _FTS3_TOKENIZER_H_ */

/************** End of fts3_tokenizer.h **************************************/
/************** Continuing where we left off in fts3Int.h ********************/
/************** Include fts3_hash.h in the middle of fts3Int.h ***************/
/************** Begin file fts3_hash.h ***************************************/
/*
** 2001 September 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This is the header file for the generic hash-table implementation
** used in SQLite.  We've modified it slightly to serve as a standalone
** hash table implementation for the full-text indexing module.
**
*/
#ifndef _FTS3_HASH_H_
#define _FTS3_HASH_H_

/* Forward declarations of structures. */
typedef struct Fts3Hash Fts3Hash;
typedef struct Fts3HashElem Fts3HashElem;

/* A complete hash table is an instance of the following structure.
** The internals of this structure are intended to be opaque -- client
** code should not attempt to access or modify the fields of this structure
** directly.  Change this structure only by using the routines below.
** However, many of the "procedures" and "functions" for modifying and
** accessing this structure are really macros, so we can't really make
** this structure opaque.
*/
struct Fts3Hash {
  char keyClass;          /* HASH_INT, _POINTER, _STRING, _BINARY */
  char copyKey;           /* True if copy of key made on insert */
  int count;              /* Number of entries in this table */
  Fts3HashElem *first;    /* The first element of the array */
  int htsize;             /* Number of buckets in the hash table */
  struct _fts3ht {        /* the hash table */
    int count;               /* Number of entries with this hash */
    Fts3HashElem *chain;     /* Pointer to first entry with this hash */
  } *ht;
};

/* Each element in the hash table is an instance of the following 
** structure.  All elements are stored on a single doubly-linked list.
**
** Again, this structure is intended to be opaque, but it can't really
** be opaque because it is used by macros.
*/
struct Fts3HashElem {
  Fts3HashElem *next, *prev; /* Next and previous elements in the table */
  void *data;                /* Data associated with this element */
  void *pKey; int nKey;      /* Key associated with this element */
};

/*
** There are 2 different modes of operation for a hash table:
**
**   FTS3_HASH_STRING        pKey points to a string that is nKey bytes long
**                           (including the null-terminator, if any).  Case
**                           is respected in comparisons.
**
**   FTS3_HASH_BINARY        pKey points to binary data nKey bytes long. 
**                           memcmp() is used to compare keys.
**
** A copy of the key is made if the copyKey parameter to fts3HashInit is 1.  
*/
#define FTS3_HASH_STRING    1
#define FTS3_HASH_BINARY    2

/*
** Access routines.  To delete, insert a NULL pointer.
*/
SQLITE_PRIVATE void sqlite3Fts3HashInit(Fts3Hash *pNew, char keyClass, char copyKey);
SQLITE_PRIVATE void *sqlite3Fts3HashInsert(Fts3Hash*, const void *pKey, int nKey, void *pData);
SQLITE_PRIVATE void *sqlite3Fts3HashFind(const Fts3Hash*, const void *pKey, int nKey);
SQLITE_PRIVATE void sqlite3Fts3HashClear(Fts3Hash*);
SQLITE_PRIVATE Fts3HashElem *sqlite3Fts3HashFindElem(const Fts3Hash *, const void *, int);

/*
** Shorthand for the functions above
*/
#define fts3HashInit     sqlite3Fts3HashInit
#define fts3HashInsert   sqlite3Fts3HashInsert
#define fts3HashFind     sqlite3Fts3HashFind
#define fts3HashClear    sqlite3Fts3HashClear
#define fts3HashFindElem sqlite3Fts3HashFindElem

/*
** Macros for looping over all elements of a hash table.  The idiom is
** like this:
**
**   Fts3Hash h;
**   Fts3HashElem *p;
**   ...
**   for(p=fts3HashFirst(&h); p; p=fts3HashNext(p)){
**     SomeStructure *pData = fts3HashData(p);
**     // do something with pData
**   }
*/
#define fts3HashFirst(H)  ((H)->first)
#define fts3HashNext(E)   ((E)->next)
#define fts3HashData(E)   ((E)->data)
#define fts3HashKey(E)    ((E)->pKey)
#define fts3HashKeysize(E) ((E)->nKey)

/*
** Number of entries in a hash table
*/
#define fts3HashCount(H)  ((H)->count)

#endif /* _FTS3_HASH_H_ */

/************** End of fts3_hash.h *******************************************/
/************** Continuing where we left off in fts3Int.h ********************/

/*
** This constant determines the maximum depth of an FTS expression tree
** that the library will create and use. FTS uses recursion to perform 
** various operations on the query tree, so the disadvantage of a large
** limit is that it may allow very large queries to use large amounts
** of stack space (perhaps causing a stack overflow).
*/
#ifndef SQLITE_FTS3_MAX_EXPR_DEPTH
# define SQLITE_FTS3_MAX_EXPR_DEPTH 12
#endif


/*
** This constant controls how often segments are merged. Once there are
** FTS3_MERGE_COUNT segments of level N, they are merged into a single
** segment of level N+1.
*/
#define FTS3_MERGE_COUNT 16

/*
** This is the maximum amount of data (in bytes) to store in the 
** Fts3Table.pendingTerms hash table. Normally, the hash table is
** populated as documents are inserted/updated/deleted in a transaction
** and used to create a new segment when the transaction is committed.
** However if this limit is reached midway through a transaction, a new 
** segment is created and the hash table cleared immediately.
*/
#define FTS3_MAX_PENDING_DATA (1*1024*1024)

/*
** Macro to return the number of elements in an array. SQLite has a
** similar macro called ArraySize(). Use a different name to avoid
** a collision when building an amalgamation with built-in FTS3.
*/
#define SizeofArray(X) ((int)(sizeof(X)/sizeof(X[0])))


#ifndef MIN
# define MIN(x,y) ((x)<(y)?(x):(y))
#endif
#ifndef MAX
# define MAX(x,y) ((x)>(y)?(x):(y))
#endif

/*
** Maximum length of a varint encoded integer. The varint format is different
** from that used by SQLite, so the maximum length is 10, not 9.
*/
#define FTS3_VARINT_MAX 10

/*
** FTS4 virtual tables may maintain multiple indexes - one index of all terms
** in the document set and zero or more prefix indexes. All indexes are stored
** as one or more b+-trees in the %_segments and %_segdir tables. 
**
** It is possible to determine which index a b+-tree belongs to based on the
** value stored in the "%_segdir.level" column. Given this value L, the index
** that the b+-tree belongs to is (L<<10). In other words, all b+-trees with
** level values between 0 and 1023 (inclusive) belong to index 0, all levels
** between 1024 and 2047 to index 1, and so on.
**
** It is considered impossible for an index to use more than 1024 levels. In 
** theory though this may happen, but only after at least 
** (FTS3_MERGE_COUNT^1024) separate flushes of the pending-terms tables.
*/
#define FTS3_SEGDIR_MAXLEVEL      1024
#define FTS3_SEGDIR_MAXLEVEL_STR "1024"

/*
** The testcase() macro is only used by the amalgamation.  If undefined,
** make it a no-op.
*/
#ifndef testcase
# define testcase(X)
#endif

/*
** Terminator values for position-lists and column-lists.
*/
#define POS_COLUMN  (1)     /* Column-list terminator */
#define POS_END     (0)     /* Position-list terminator */ 

/*
** This section provides definitions to allow the
** FTS3 extension to be compiled outside of the 
** amalgamation.
*/
#ifndef SQLITE_AMALGAMATION
/*
** Macros indicating that conditional expressions are always true or
** false.
*/
#ifdef SQLITE_COVERAGE_TEST
# define ALWAYS(x) (1)
# define NEVER(X)  (0)
#elif defined(SQLITE_DEBUG)
# define ALWAYS(x) sqlite3Fts3Always((x)!=0)
# define NEVER(x) sqlite3Fts3Never((x)!=0)
SQLITE_PRIVATE int sqlite3Fts3Always(int b);
SQLITE_PRIVATE int sqlite3Fts3Never(int b);
#else
# define ALWAYS(x) (x)
# define NEVER(x)  (x)
#endif

/*
** Internal types used by SQLite.
*/
typedef unsigned char u8;         /* 1-byte (or larger) unsigned integer */
typedef short int i16;            /* 2-byte (or larger) signed integer */
typedef unsigned int u32;         /* 4-byte unsigned integer */
typedef sqlite3_uint64 u64;       /* 8-byte unsigned integer */
typedef sqlite3_int64 i64;        /* 8-byte signed integer */

/*
** Macro used to suppress compiler warnings for unused parameters.
*/
#define UNUSED_PARAMETER(x) (void)(x)

/*
** Activate assert() only if SQLITE_TEST is enabled.
*/
#if !defined(NDEBUG) && !defined(SQLITE_DEBUG) 
# define NDEBUG 1
#endif

/*
** The TESTONLY macro is used to enclose variable declarations or
** other bits of code that are needed to support the arguments
** within testcase() and assert() macros.
*/
#if defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
# define TESTONLY(X)  X
#else
# define TESTONLY(X)
#endif

#endif /* SQLITE_AMALGAMATION */

#ifdef SQLITE_DEBUG
SQLITE_PRIVATE int sqlite3Fts3Corrupt(void);
# define FTS_CORRUPT_VTAB sqlite3Fts3Corrupt()
#else
# define FTS_CORRUPT_VTAB SQLITE_CORRUPT_VTAB
#endif

typedef struct Fts3Table Fts3Table;
typedef struct Fts3Cursor Fts3Cursor;
typedef struct Fts3Expr Fts3Expr;
typedef struct Fts3Phrase Fts3Phrase;
typedef struct Fts3PhraseToken Fts3PhraseToken;

typedef struct Fts3Doclist Fts3Doclist;
typedef struct Fts3SegFilter Fts3SegFilter;
typedef struct Fts3DeferredToken Fts3DeferredToken;
typedef struct Fts3SegReader Fts3SegReader;
typedef struct Fts3MultiSegReader Fts3MultiSegReader;

typedef struct MatchinfoBuffer MatchinfoBuffer;

/*
** A connection to a fulltext index is an instance of the following
** structure. The xCreate and xConnect methods create an instance
** of this structure and xDestroy and xDisconnect free that instance.
** All other methods receive a pointer to the structure as one of their
** arguments.
*/
struct Fts3Table {
  sqlite3_vtab base;              /* Base class used by SQLite core */
  sqlite3 *db;                    /* The database connection */
  const char *zDb;                /* logical database name */
  const char *zName;              /* virtual table name */
  int nColumn;                    /* number of named columns in virtual table */
  char **azColumn;                /* column names.  malloced */
  u8 *abNotindexed;               /* True for 'notindexed' columns */
  sqlite3_tokenizer *pTokenizer;  /* tokenizer for inserts and queries */
  char *zContentTbl;              /* content=xxx option, or NULL */
  char *zLanguageid;              /* languageid=xxx option, or NULL */
  int nAutoincrmerge;             /* Value configured by 'automerge' */
  u32 nLeafAdd;                   /* Number of leaf blocks added this trans */

  /* Precompiled statements used by the implementation. Each of these 
  ** statements is run and reset within a single virtual table API call. 
  */
  sqlite3_stmt *aStmt[40];

  char *zReadExprlist;
  char *zWriteExprlist;

  int nNodeSize;                  /* Soft limit for node size */
  u8 bFts4;                       /* True for FTS4, false for FTS3 */
  u8 bHasStat;                    /* True if %_stat table exists (2==unknown) */
  u8 bHasDocsize;                 /* True if %_docsize table exists */
  u8 bDescIdx;                    /* True if doclists are in reverse order */
  u8 bIgnoreSavepoint;            /* True to ignore xSavepoint invocations */
  int nPgsz;                      /* Page size for host database */
  char *zSegmentsTbl;             /* Name of %_segments table */
  sqlite3_blob *pSegments;        /* Blob handle open on %_segments table */

  /* 
  ** The following array of hash tables is used to buffer pending index 
  ** updates during transactions. All pending updates buffered at any one
  ** time must share a common language-id (see the FTS4 langid= feature).
  ** The current language id is stored in variable iPrevLangid.
  **
  ** A single FTS4 table may have multiple full-text indexes. For each index
  ** there is an entry in the aIndex[] array. Index 0 is an index of all the
  ** terms that appear in the document set. Each subsequent index in aIndex[]
  ** is an index of prefixes of a specific length.
  **
  ** Variable nPendingData contains an estimate the memory consumed by the 
  ** pending data structures, including hash table overhead, but not including
  ** malloc overhead.  When nPendingData exceeds nMaxPendingData, all hash
  ** tables are flushed to disk. Variable iPrevDocid is the docid of the most 
  ** recently inserted record.
  */
  int nIndex;                     /* Size of aIndex[] */
  struct Fts3Index {
    int nPrefix;                  /* Prefix length (0 for main terms index) */
    Fts3Hash hPending;            /* Pending terms table for this index */
  } *aIndex;
  int nMaxPendingData;            /* Max pending data before flush to disk */
  int nPendingData;               /* Current bytes of pending data */
  sqlite_int64 iPrevDocid;        /* Docid of most recently inserted document */
  int iPrevLangid;                /* Langid of recently inserted document */

#if defined(SQLITE_DEBUG) || defined(SQLITE_COVERAGE_TEST)
  /* State variables used for validating that the transaction control
  ** methods of the virtual table are called at appropriate times.  These
  ** values do not contribute to FTS functionality; they are used for
  ** verifying the operation of the SQLite core.
  */
  int inTransaction;     /* True after xBegin but before xCommit/xRollback */
  int mxSavepoint;       /* Largest valid xSavepoint integer */
#endif

#ifdef SQLITE_TEST
  /* True to disable the incremental doclist optimization. This is controled
  ** by special insert command 'test-no-incr-doclist'.  */
  int bNoIncrDoclist;
#endif
};

/*
** When the core wants to read from the virtual table, it creates a
** virtual table cursor (an instance of the following structure) using
** the xOpen method. Cursors are destroyed using the xClose method.
*/
struct Fts3Cursor {
  sqlite3_vtab_cursor base;       /* Base class used by SQLite core */
  i16 eSearch;                    /* Search strategy (see below) */
  u8 isEof;                       /* True if at End Of Results */
  u8 isRequireSeek;               /* True if must seek pStmt to %_content row */
  sqlite3_stmt *pStmt;            /* Prepared statement in use by the cursor */
  Fts3Expr *pExpr;                /* Parsed MATCH query string */
  int iLangid;                    /* Language being queried for */
  int nPhrase;                    /* Number of matchable phrases in query */
  Fts3DeferredToken *pDeferred;   /* Deferred search tokens, if any */
  sqlite3_int64 iPrevId;          /* Previous id read from aDoclist */
  char *pNextId;                  /* Pointer into the body of aDoclist */
  char *aDoclist;                 /* List of docids for full-text queries */
  int nDoclist;                   /* Size of buffer at aDoclist */
  u8 bDesc;                       /* True to sort in descending order */
  int eEvalmode;                  /* An FTS3_EVAL_XX constant */
  int nRowAvg;                    /* Average size of database rows, in pages */
  sqlite3_int64 nDoc;             /* Documents in table */
  i64 iMinDocid;                  /* Minimum docid to return */
  i64 iMaxDocid;                  /* Maximum docid to return */
  int isMatchinfoNeeded;          /* True when aMatchinfo[] needs filling in */
  MatchinfoBuffer *pMIBuffer;     /* Buffer for matchinfo data */
};

#define FTS3_EVAL_FILTER    0
#define FTS3_EVAL_NEXT      1
#define FTS3_EVAL_MATCHINFO 2

/*
** The Fts3Cursor.eSearch member is always set to one of the following.
** Actualy, Fts3Cursor.eSearch can be greater than or equal to
** FTS3_FULLTEXT_SEARCH.  If so, then Fts3Cursor.eSearch - 2 is the index
** of the column to be searched.  For example, in
**
**     CREATE VIRTUAL TABLE ex1 USING fts3(a,b,c,d);
**     SELECT docid FROM ex1 WHERE b MATCH 'one two three';
** 
** Because the LHS of the MATCH operator is 2nd column "b",
** Fts3Cursor.eSearch will be set to FTS3_FULLTEXT_SEARCH+1.  (+0 for a,
** +1 for b, +2 for c, +3 for d.)  If the LHS of MATCH were "ex1" 
** indicating that all columns should be searched,
** then eSearch would be set to FTS3_FULLTEXT_SEARCH+4.
*/
#define FTS3_FULLSCAN_SEARCH 0    /* Linear scan of %_content table */
#define FTS3_DOCID_SEARCH    1    /* Lookup by rowid on %_content table */
#define FTS3_FULLTEXT_SEARCH 2    /* Full-text index search */

/*
** The lower 16-bits of the sqlite3_index_info.idxNum value set by
** the xBestIndex() method contains the Fts3Cursor.eSearch value described
** above. The upper 16-bits contain a combination of the following
** bits, used to describe extra constraints on full-text searches.
*/
#define FTS3_HAVE_LANGID    0x00010000      /* languageid=? */
#define FTS3_HAVE_DOCID_GE  0x00020000      /* docid>=? */
#define FTS3_HAVE_DOCID_LE  0x00040000      /* docid<=? */

struct Fts3Doclist {
  char *aAll;                    /* Array containing doclist (or NULL) */
  int nAll;                      /* Size of a[] in bytes */
  char *pNextDocid;              /* Pointer to next docid */

  sqlite3_int64 iDocid;          /* Current docid (if pList!=0) */
  int bFreeList;                 /* True if pList should be sqlite3_free()d */
  char *pList;                   /* Pointer to position list following iDocid */
  int nList;                     /* Length of position list */
};

/*
** A "phrase" is a sequence of one or more tokens that must match in
** sequence.  A single token is the base case and the most common case.
** For a sequence of tokens contained in double-quotes (i.e. "one two three")
** nToken will be the number of tokens in the string.
*/
struct Fts3PhraseToken {
  char *z;                        /* Text of the token */
  int n;                          /* Number of bytes in buffer z */
  int isPrefix;                   /* True if token ends with a "*" character */
  int bFirst;                     /* True if token must appear at position 0 */

  /* Variables above this point are populated when the expression is
  ** parsed (by code in fts3_expr.c). Below this point the variables are
  ** used when evaluating the expression. */
  Fts3DeferredToken *pDeferred;   /* Deferred token object for this token */
  Fts3MultiSegReader *pSegcsr;    /* Segment-reader for this token */
};

struct Fts3Phrase {
  /* Cache of doclist for this phrase. */
  Fts3Doclist doclist;
  int bIncr;                 /* True if doclist is loaded incrementally */
  int iDoclistToken;

  /* Used by sqlite3Fts3EvalPhrasePoslist() if this is a descendent of an
  ** OR condition.  */
  char *pOrPoslist;
  i64 iOrDocid;

  /* Variables below this point are populated by fts3_expr.c when parsing 
  ** a MATCH expression. Everything above is part of the evaluation phase. 
  */
  int nToken;                /* Number of tokens in the phrase */
  int iColumn;               /* Index of column this phrase must match */
  Fts3PhraseToken aToken[1]; /* One entry for each token in the phrase */
};

/*
** A tree of these objects forms the RHS of a MATCH operator.
**
** If Fts3Expr.eType is FTSQUERY_PHRASE and isLoaded is true, then aDoclist 
** points to a malloced buffer, size nDoclist bytes, containing the results 
** of this phrase query in FTS3 doclist format. As usual, the initial 
** "Length" field found in doclists stored on disk is omitted from this 
** buffer.
**
** Variable aMI is used only for FTSQUERY_NEAR nodes to store the global
** matchinfo data. If it is not NULL, it points to an array of size nCol*3,
** where nCol is the number of columns in the queried FTS table. The array
** is populated as follows:
**
**   aMI[iCol*3 + 0] = Undefined
**   aMI[iCol*3 + 1] = Number of occurrences
**   aMI[iCol*3 + 2] = Number of rows containing at least one instance
**
** The aMI array is allocated using sqlite3_malloc(). It should be freed 
** when the expression node is.
*/
struct Fts3Expr {
  int eType;                 /* One of the FTSQUERY_XXX values defined below */
  int nNear;                 /* Valid if eType==FTSQUERY_NEAR */
  Fts3Expr *pParent;         /* pParent->pLeft==this or pParent->pRight==this */
  Fts3Expr *pLeft;           /* Left operand */
  Fts3Expr *pRight;          /* Right operand */
  Fts3Phrase *pPhrase;       /* Valid if eType==FTSQUERY_PHRASE */

  /* The following are used by the fts3_eval.c module. */
  sqlite3_int64 iDocid;      /* Current docid */
  u8 bEof;                   /* True this expression is at EOF already */
  u8 bStart;                 /* True if iDocid is valid */
  u8 bDeferred;              /* True if this expression is entirely deferred */

  /* The following are used by the fts3_snippet.c module. */
  int iPhrase;               /* Index of this phrase in matchinfo() results */
  u32 *aMI;                  /* See above */
};

/*
** Candidate values for Fts3Query.eType. Note that the order of the first
** four values is in order of precedence when parsing expressions. For 
** example, the following:
**
**   "a OR b AND c NOT d NEAR e"
**
** is equivalent to:
**
**   "a OR (b AND (c NOT (d NEAR e)))"
*/
#define FTSQUERY_NEAR   1
#define FTSQUERY_NOT    2
#define FTSQUERY_AND    3
#define FTSQUERY_OR     4
#define FTSQUERY_PHRASE 5


/* fts3_write.c */
SQLITE_PRIVATE int sqlite3Fts3UpdateMethod(sqlite3_vtab*,int,sqlite3_value**,sqlite3_int64*);
SQLITE_PRIVATE int sqlite3Fts3PendingTermsFlush(Fts3Table *);
SQLITE_PRIVATE void sqlite3Fts3PendingTermsClear(Fts3Table *);
SQLITE_PRIVATE int sqlite3Fts3Optimize(Fts3Table *);
SQLITE_PRIVATE int sqlite3Fts3SegReaderNew(int, int, sqlite3_int64,
  sqlite3_int64, sqlite3_int64, const char *, int, Fts3SegReader**);
SQLITE_PRIVATE int sqlite3Fts3SegReaderPending(
  Fts3Table*,int,const char*,int,int,Fts3SegReader**);
SQLITE_PRIVATE void sqlite3Fts3SegReaderFree(Fts3SegReader *);
SQLITE_PRIVATE int sqlite3Fts3AllSegdirs(Fts3Table*, int, int, int, sqlite3_stmt **);
SQLITE_PRIVATE int sqlite3Fts3ReadBlock(Fts3Table*, sqlite3_int64, char **, int*, int*);

SQLITE_PRIVATE int sqlite3Fts3SelectDoctotal(Fts3Table *, sqlite3_stmt **);
SQLITE_PRIVATE int sqlite3Fts3SelectDocsize(Fts3Table *, sqlite3_int64, sqlite3_stmt **);

#ifndef SQLITE_DISABLE_FTS4_DEFERRED
SQLITE_PRIVATE void sqlite3Fts3FreeDeferredTokens(Fts3Cursor *);
SQLITE_PRIVATE int sqlite3Fts3DeferToken(Fts3Cursor *, Fts3PhraseToken *, int);
SQLITE_PRIVATE int sqlite3Fts3CacheDeferredDoclists(Fts3Cursor *);
SQLITE_PRIVATE void sqlite3Fts3FreeDeferredDoclists(Fts3Cursor *);
SQLITE_PRIVATE int sqlite3Fts3DeferredTokenList(Fts3DeferredToken *, char **, int *);
#else
# define sqlite3Fts3FreeDeferredTokens(x)
# define sqlite3Fts3DeferToken(x,y,z) SQLITE_OK
# define sqlite3Fts3CacheDeferredDoclists(x) SQLITE_OK
# define sqlite3Fts3FreeDeferredDoclists(x)
# define sqlite3Fts3DeferredTokenList(x,y,z) SQLITE_OK
#endif

SQLITE_PRIVATE void sqlite3Fts3SegmentsClose(Fts3Table *);
SQLITE_PRIVATE int sqlite3Fts3MaxLevel(Fts3Table *, int *);

/* Special values interpreted by sqlite3SegReaderCursor() */
#define FTS3_SEGCURSOR_PENDING        -1
#define FTS3_SEGCURSOR_ALL            -2

SQLITE_PRIVATE int sqlite3Fts3SegReaderStart(Fts3Table*, Fts3MultiSegReader*, Fts3SegFilter*);
SQLITE_PRIVATE int sqlite3Fts3SegReaderStep(Fts3Table *, Fts3MultiSegReader *);
SQLITE_PRIVATE void sqlite3Fts3SegReaderFinish(Fts3MultiSegReader *);

SQLITE_PRIVATE int sqlite3Fts3SegReaderCursor(Fts3Table *, 
    int, int, int, const char *, int, int, int, Fts3MultiSegReader *);

/* Flags allowed as part of the 4th argument to SegmentReaderIterate() */
#define FTS3_SEGMENT_REQUIRE_POS   0x00000001
#define FTS3_SEGMENT_IGNORE_EMPTY  0x00000002
#define FTS3_SEGMENT_COLUMN_FILTER 0x00000004
#define FTS3_SEGMENT_PREFIX        0x00000008
#define FTS3_SEGMENT_SCAN          0x00000010
#define FTS3_SEGMENT_FIRST         0x00000020

/* Type passed as 4th argument to SegmentReaderIterate() */
struct Fts3SegFilter {
  const char *zTerm;
  int nTerm;
  int iCol;
  int flags;
};

struct Fts3MultiSegReader {
  /* Used internally by sqlite3Fts3SegReaderXXX() calls */
  Fts3SegReader **apSegment;      /* Array of Fts3SegReader objects */
  int nSegment;                   /* Size of apSegment array */
  int nAdvance;                   /* How many seg-readers to advance */
  Fts3SegFilter *pFilter;         /* Pointer to filter object */
  char *aBuffer;                  /* Buffer to merge doclists in */
  int nBuffer;                    /* Allocated size of aBuffer[] in bytes */

  int iColFilter;                 /* If >=0, filter for this column */
  int bRestart;

  /* Used by fts3.c only. */
  int nCost;                      /* Cost of running iterator */
  int bLookup;                    /* True if a lookup of a single entry. */

  /* Output values. Valid only after Fts3SegReaderStep() returns SQLITE_ROW. */
  char *zTerm;                    /* Pointer to term buffer */
  int nTerm;                      /* Size of zTerm in bytes */
  char *aDoclist;                 /* Pointer to doclist buffer */
  int nDoclist;                   /* Size of aDoclist[] in bytes */
};

SQLITE_PRIVATE int sqlite3Fts3Incrmerge(Fts3Table*,int,int);

#define fts3GetVarint32(p, piVal) (                                           \
  (*(u8*)(p)&0x80) ? sqlite3Fts3GetVarint32(p, piVal) : (*piVal=*(u8*)(p), 1) \
)

/* fts3.c */
SQLITE_PRIVATE void sqlite3Fts3ErrMsg(char**,const char*,...);
SQLITE_PRIVATE int sqlite3Fts3PutVarint(char *, sqlite3_int64);
SQLITE_PRIVATE int sqlite3Fts3GetVarint(const char *, sqlite_int64 *);
SQLITE_PRIVATE int sqlite3Fts3GetVarint32(const char *, int *);
SQLITE_PRIVATE int sqlite3Fts3VarintLen(sqlite3_uint64);
SQLITE_PRIVATE void sqlite3Fts3Dequote(char *);
SQLITE_PRIVATE void sqlite3Fts3DoclistPrev(int,char*,int,char**,sqlite3_int64*,int*,u8*);
SQLITE_PRIVATE int sqlite3Fts3EvalPhraseStats(Fts3Cursor *, Fts3Expr *, u32 *);
SQLITE_PRIVATE int sqlite3Fts3FirstFilter(sqlite3_int64, char *, int, char *);
SQLITE_PRIVATE void sqlite3Fts3CreateStatTable(int*, Fts3Table*);
SQLITE_PRIVATE int sqlite3Fts3EvalTestDeferred(Fts3Cursor *pCsr, int *pRc);

/* fts3_tokenizer.c */
SQLITE_PRIVATE const char *sqlite3Fts3NextToken(const char *, int *);
SQLITE_PRIVATE int sqlite3Fts3InitHashTable(sqlite3 *, Fts3Hash *, const char *);
SQLITE_PRIVATE int sqlite3Fts3InitTokenizer(Fts3Hash *pHash, const char *, 
    sqlite3_tokenizer **, char **
);
SQLITE_PRIVATE int sqlite3Fts3IsIdChar(char);

/* fts3_snippet.c */
SQLITE_PRIVATE void sqlite3Fts3Offsets(sqlite3_context*, Fts3Cursor*);
SQLITE_PRIVATE void sqlite3Fts3Snippet(sqlite3_context *, Fts3Cursor *, const char *,
  const char *, const char *, int, int
);
SQLITE_PRIVATE void sqlite3Fts3Matchinfo(sqlite3_context *, Fts3Cursor *, const char *);
SQLITE_PRIVATE void sqlite3Fts3MIBufferFree(MatchinfoBuffer *p);

/* fts3_expr.c */
SQLITE_PRIVATE int sqlite3Fts3ExprParse(sqlite3_tokenizer *, int,
  char **, int, int, int, const char *, int, Fts3Expr **, char **
);
SQLITE_PRIVATE void sqlite3Fts3ExprFree(Fts3Expr *);
#ifdef SQLITE_TEST
SQLITE_PRIVATE int sqlite3Fts3ExprInitTestInterface(sqlite3 *db);
SQLITE_PRIVATE int sqlite3Fts3InitTerm(sqlite3 *db);
#endif

SQLITE_PRIVATE int sqlite3Fts3OpenTokenizer(sqlite3_tokenizer *, int, const char *, int,
  sqlite3_tokenizer_cursor **
);

/* fts3_aux.c */
SQLITE_PRIVATE int sqlite3Fts3InitAux(sqlite3 *db);

SQLITE_PRIVATE void sqlite3Fts3EvalPhraseCleanup(Fts3Phrase *);

SQLITE_PRIVATE int sqlite3Fts3MsrIncrStart(
    Fts3Table*, Fts3MultiSegReader*, int, const char*, int);
SQLITE_PRIVATE int sqlite3Fts3MsrIncrNext(
    Fts3Table *, Fts3MultiSegReader *, sqlite3_int64 *, char **, int *);
SQLITE_PRIVATE int sqlite3Fts3EvalPhrasePoslist(Fts3Cursor *, Fts3Expr *, int iCol, char **); 
SQLITE_PRIVATE int sqlite3Fts3MsrOvfl(Fts3Cursor *, Fts3MultiSegReader *, int *);
SQLITE_PRIVATE int sqlite3Fts3MsrIncrRestart(Fts3MultiSegReader *pCsr);

/* fts3_tokenize_vtab.c */
SQLITE_PRIVATE int sqlite3Fts3InitTok(sqlite3*, Fts3Hash *);

/* fts3_unicode2.c (functions generated by parsing unicode text files) */
#ifndef SQLITE_DISABLE_FTS3_UNICODE
SQLITE_PRIVATE int sqlite3FtsUnicodeFold(int, int);
SQLITE_PRIVATE int sqlite3FtsUnicodeIsalnum(int);
SQLITE_PRIVATE int sqlite3FtsUnicodeIsdiacritic(int);
#endif

#endif /* !SQLITE_CORE || SQLITE_ENABLE_FTS3 */
#endif /* _FTSINT_H */

/************** End of fts3Int.h *********************************************/
/************** Continuing where we left off in fts3.c ***********************/
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

#if defined(SQLITE_ENABLE_FTS3) && !defined(SQLITE_CORE)
# define SQLITE_CORE 1
#endif

/* #include <assert.h> */
/* #include <stdlib.h> */
/* #include <stddef.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <stdarg.h> */

/* #include "fts3.h" */
#ifndef SQLITE_CORE 
/* # include "sqlite3ext.h" */
  SQLITE_EXTENSION_INIT1
#endif

static int fts3EvalNext(Fts3Cursor *pCsr);
static int fts3EvalStart(Fts3Cursor *pCsr);
static int fts3TermSegReaderCursor(
    Fts3Cursor *, const char *, int, int, Fts3MultiSegReader **);

#ifndef SQLITE_AMALGAMATION
# if defined(SQLITE_DEBUG)
SQLITE_PRIVATE int sqlite3Fts3Always(int b) { assert( b ); return b; }
SQLITE_PRIVATE int sqlite3Fts3Never(int b)  { assert( !b ); return b; }
# endif
#endif

/* 
** Write a 64-bit variable-length integer to memory starting at p[0].
** The length of data written will be between 1 and FTS3_VARINT_MAX bytes.
** The number of bytes written is returned.
*/
SQLITE_PRIVATE int sqlite3Fts3PutVarint(char *p, sqlite_int64 v){
  unsigned char *q = (unsigned char *) p;
  sqlite_uint64 vu = v;
  do{
    *q++ = (unsigned char) ((vu & 0x7f) | 0x80);
    vu >>= 7;
  }while( vu!=0 );
  q[-1] &= 0x7f;  /* turn off high bit in final byte */
  assert( q - (unsigned char *)p <= FTS3_VARINT_MAX );
  return (int) (q - (unsigned char *)p);
}

#define GETVARINT_STEP(v, ptr, shift, mask1, mask2, var, ret) \
  v = (v & mask1) | ( (*ptr++) << shift );                    \
  if( (v & mask2)==0 ){ var = v; return ret; }
#define GETVARINT_INIT(v, ptr, shift, mask1, mask2, var, ret) \
  v = (*ptr++);                                               \
  if( (v & mask2)==0 ){ var = v; return ret; }

/* 
** Read a 64-bit variable-length integer from memory starting at p[0].
** Return the number of bytes read, or 0 on error.
** The value is stored in *v.
*/
SQLITE_PRIVATE int sqlite3Fts3GetVarint(const char *p, sqlite_int64 *v){
  const char *pStart = p;
  u32 a;
  u64 b;
  int shift;

  GETVARINT_INIT(a, p, 0,  0x00,     0x80, *v, 1);
  GETVARINT_STEP(a, p, 7,  0x7F,     0x4000, *v, 2);
  GETVARINT_STEP(a, p, 14, 0x3FFF,   0x200000, *v, 3);
  GETVARINT_STEP(a, p, 21, 0x1FFFFF, 0x10000000, *v, 4);
  b = (a & 0x0FFFFFFF );

  for(shift=28; shift<=63; shift+=7){
    u64 c = *p++;
    b += (c&0x7F) << shift;
    if( (c & 0x80)==0 ) break;
  }
  *v = b;
  return (int)(p - pStart);
}

/*
** Similar to sqlite3Fts3GetVarint(), except that the output is truncated to a
** 32-bit integer before it is returned.
*/
SQLITE_PRIVATE int sqlite3Fts3GetVarint32(const char *p, int *pi){
  u32 a;

#ifndef fts3GetVarint32
  GETVARINT_INIT(a, p, 0,  0x00,     0x80, *pi, 1);
#else
  a = (*p++);
  assert( a & 0x80 );
#endif

  GETVARINT_STEP(a, p, 7,  0x7F,     0x4000, *pi, 2);
  GETVARINT_STEP(a, p, 14, 0x3FFF,   0x200000, *pi, 3);
  GETVARINT_STEP(a, p, 21, 0x1FFFFF, 0x10000000, *pi, 4);
  a = (a & 0x0FFFFFFF );
  *pi = (int)(a | ((u32)(*p & 0x0F) << 28));
  return 5;
}

/*
** Return the number of bytes required to encode v as a varint
*/
SQLITE_PRIVATE int sqlite3Fts3VarintLen(sqlite3_uint64 v){
  int i = 0;
  do{
    i++;
    v >>= 7;
  }while( v!=0 );
  return i;
}

/*
** Convert an SQL-style quoted string into a normal string by removing
** the quote characters.  The conversion is done in-place.  If the
** input does not begin with a quote character, then this routine
** is a no-op.
**
** Examples:
**
**     "abc"   becomes   abc
**     'xyz'   becomes   xyz
**     [pqr]   becomes   pqr
**     `mno`   becomes   mno
**
*/
SQLITE_PRIVATE void sqlite3Fts3Dequote(char *z){
  char quote;                     /* Quote character (if any ) */

  quote = z[0];
  if( quote=='[' || quote=='\'' || quote=='"' || quote=='`' ){
    int iIn = 1;                  /* Index of next byte to read from input */
    int iOut = 0;                 /* Index of next byte to write to output */

    /* If the first byte was a '[', then the close-quote character is a ']' */
    if( quote=='[' ) quote = ']';  

    while( z[iIn] ){
      if( z[iIn]==quote ){
        if( z[iIn+1]!=quote ) break;
        z[iOut++] = quote;
        iIn += 2;
      }else{
        z[iOut++] = z[iIn++];
      }
    }
    z[iOut] = '\0';
  }
}

/*
** Read a single varint from the doclist at *pp and advance *pp to point
** to the first byte past the end of the varint.  Add the value of the varint
** to *pVal.
*/
static void fts3GetDeltaVarint(char **pp, sqlite3_int64 *pVal){
  sqlite3_int64 iVal;
  *pp += sqlite3Fts3GetVarint(*pp, &iVal);
  *pVal += iVal;
}

/*
** When this function is called, *pp points to the first byte following a
** varint that is part of a doclist (or position-list, or any other list
** of varints). This function moves *pp to point to the start of that varint,
** and sets *pVal by the varint value.
**
** Argument pStart points to the first byte of the doclist that the
** varint is part of.
*/
static void fts3GetReverseVarint(
  char **pp, 
  char *pStart, 
  sqlite3_int64 *pVal
){
  sqlite3_int64 iVal;
  char *p;

  /* Pointer p now points at the first byte past the varint we are 
  ** interested in. So, unless the doclist is corrupt, the 0x80 bit is
  ** clear on character p[-1]. */
  for(p = (*pp)-2; p>=pStart && *p&0x80; p--);
  p++;
  *pp = p;

  sqlite3Fts3GetVarint(p, &iVal);
  *pVal = iVal;
}

/*
** The xDisconnect() virtual table method.
*/
static int fts3DisconnectMethod(sqlite3_vtab *pVtab){
  Fts3Table *p = (Fts3Table *)pVtab;
  int i;

  assert( p->nPendingData==0 );
  assert( p->pSegments==0 );

  /* Free any prepared statements held */
  for(i=0; i<SizeofArray(p->aStmt); i++){
    sqlite3_finalize(p->aStmt[i]);
  }
  sqlite3_free(p->zSegmentsTbl);
  sqlite3_free(p->zReadExprlist);
  sqlite3_free(p->zWriteExprlist);
  sqlite3_free(p->zContentTbl);
  sqlite3_free(p->zLanguageid);

  /* Invoke the tokenizer destructor to free the tokenizer. */
  p->pTokenizer->pModule->xDestroy(p->pTokenizer);

  sqlite3_free(p);
  return SQLITE_OK;
}

/*
** Write an error message into *pzErr
*/
SQLITE_PRIVATE void sqlite3Fts3ErrMsg(char **pzErr, const char *zFormat, ...){
  va_list ap;
  sqlite3_free(*pzErr);
  va_start(ap, zFormat);
  *pzErr = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
}

/*
** Construct one or more SQL statements from the format string given
** and then evaluate those statements. The success code is written
** into *pRc.
**
** If *pRc is initially non-zero then this routine is a no-op.
*/
static void fts3DbExec(
  int *pRc,              /* Success code */
  sqlite3 *db,           /* Database in which to run SQL */
  const char *zFormat,   /* Format string for SQL */
  ...                    /* Arguments to the format string */
){
  va_list ap;
  char *zSql;
  if( *pRc ) return;
  va_start(ap, zFormat);
  zSql = sqlite3_vmprintf(zFormat, ap);
  va_end(ap);
  if( zSql==0 ){
    *pRc = SQLITE_NOMEM;
  }else{
    *pRc = sqlite3_exec(db, zSql, 0, 0, 0);
    sqlite3_free(zSql);
  }
}

/*
** The xDestroy() virtual table method.
*/
static int fts3DestroyMethod(sqlite3_vtab *pVtab){
  Fts3Table *p = (Fts3Table *)pVtab;
  int rc = SQLITE_OK;              /* Return code */
  const char *zDb = p->zDb;        /* Name of database (e.g. "main", "temp") */
  sqlite3 *db = p->db;             /* Database handle */

  /* Drop the shadow tables */
  if( p->zContentTbl==0 ){
    fts3DbExec(&rc, db, "DROP TABLE IF EXISTS %Q.'%q_content'", zDb, p->zName);
  }
  fts3DbExec(&rc, db, "DROP TABLE IF EXISTS %Q.'%q_segments'", zDb,p->zName);
  fts3DbExec(&rc, db, "DROP TABLE IF EXISTS %Q.'%q_segdir'", zDb, p->zName);
  fts3DbExec(&rc, db, "DROP TABLE IF EXISTS %Q.'%q_docsize'", zDb, p->zName);
  fts3DbExec(&rc, db, "DROP TABLE IF EXISTS %Q.'%q_stat'", zDb, p->zName);

  /* If everything has worked, invoke fts3DisconnectMethod() to free the
  ** memory associated with the Fts3Table structure and return SQLITE_OK.
  ** Otherwise, return an SQLite error code.
  */
  return (rc==SQLITE_OK ? fts3DisconnectMethod(pVtab) : rc);
}


/*
** Invoke sqlite3_declare_vtab() to declare the schema for the FTS3 table
** passed as the first argument. This is done as part of the xConnect()
** and xCreate() methods.
**
** If *pRc is non-zero when this function is called, it is a no-op. 
** Otherwise, if an error occurs, an SQLite error code is stored in *pRc
** before returning.
*/
static void fts3DeclareVtab(int *pRc, Fts3Table *p){
  if( *pRc==SQLITE_OK ){
    int i;                        /* Iterator variable */
    int rc;                       /* Return code */
    char *zSql;                   /* SQL statement passed to declare_vtab() */
    char *zCols;                  /* List of user defined columns */
    const char *zLanguageid;

    zLanguageid = (p->zLanguageid ? p->zLanguageid : "__langid");
    sqlite3_vtab_config(p->db, SQLITE_VTAB_CONSTRAINT_SUPPORT, 1);

    /* Create a list of user columns for the virtual table */
    zCols = sqlite3_mprintf("%Q, ", p->azColumn[0]);
    for(i=1; zCols && i<p->nColumn; i++){
      zCols = sqlite3_mprintf("%z%Q, ", zCols, p->azColumn[i]);
    }

    /* Create the whole "CREATE TABLE" statement to pass to SQLite */
    zSql = sqlite3_mprintf(
        "CREATE TABLE x(%s %Q HIDDEN, docid HIDDEN, %Q HIDDEN)", 
        zCols, p->zName, zLanguageid
    );
    if( !zCols || !zSql ){
      rc = SQLITE_NOMEM;
    }else{
      rc = sqlite3_declare_vtab(p->db, zSql);
    }

    sqlite3_free(zSql);
    sqlite3_free(zCols);
    *pRc = rc;
  }
}

/*
** Create the %_stat table if it does not already exist.
*/
SQLITE_PRIVATE void sqlite3Fts3CreateStatTable(int *pRc, Fts3Table *p){
  fts3DbExec(pRc, p->db, 
      "CREATE TABLE IF NOT EXISTS %Q.'%q_stat'"
          "(id INTEGER PRIMARY KEY, value BLOB);",
      p->zDb, p->zName
  );
  if( (*pRc)==SQLITE_OK ) p->bHasStat = 1;
}

/*
** Create the backing store tables (%_content, %_segments and %_segdir)
** required by the FTS3 table passed as the only argument. This is done
** as part of the vtab xCreate() method.
**
** If the p->bHasDocsize boolean is true (indicating that this is an
** FTS4 table, not an FTS3 table) then also create the %_docsize and
** %_stat tables required by FTS4.
*/
static int fts3CreateTables(Fts3Table *p){
  int rc = SQLITE_OK;             /* Return code */
  int i;                          /* Iterator variable */
  sqlite3 *db = p->db;            /* The database connection */

  if( p->zContentTbl==0 ){
    const char *zLanguageid = p->zLanguageid;
    char *zContentCols;           /* Columns of %_content table */

    /* Create a list of user columns for the content table */
    zContentCols = sqlite3_mprintf("docid INTEGER PRIMARY KEY");
    for(i=0; zContentCols && i<p->nColumn; i++){
      char *z = p->azColumn[i];
      zContentCols = sqlite3_mprintf("%z, 'c%d%q'", zContentCols, i, z);
    }
    if( zLanguageid && zContentCols ){
      zContentCols = sqlite3_mprintf("%z, langid", zContentCols, zLanguageid);
    }
    if( zContentCols==0 ) rc = SQLITE_NOMEM;
  
    /* Create the content table */
    fts3DbExec(&rc, db, 
       "CREATE TABLE %Q.'%q_content'(%s)",
       p->zDb, p->zName, zContentCols
    );
    sqlite3_free(zContentCols);
  }

  /* Create other tables */
  fts3DbExec(&rc, db, 
      "CREATE TABLE %Q.'%q_segments'(blockid INTEGER PRIMARY KEY, block BLOB);",
      p->zDb, p->zName
  );
  fts3DbExec(&rc, db, 
      "CREATE TABLE %Q.'%q_segdir'("
        "level INTEGER,"
        "idx INTEGER,"
        "start_block INTEGER,"
        "leaves_end_block INTEGER,"
        "end_block INTEGER,"
        "root BLOB,"
        "PRIMARY KEY(level, idx)"
      ");",
      p->zDb, p->zName
  );
  if( p->bHasDocsize ){
    fts3DbExec(&rc, db, 
        "CREATE TABLE %Q.'%q_docsize'(docid INTEGER PRIMARY KEY, size BLOB);",
        p->zDb, p->zName
    );
  }
  assert( p->bHasStat==p->bFts4 );
  if( p->bHasStat ){
    sqlite3Fts3CreateStatTable(&rc, p);
  }
  return rc;
}

/*
** Store the current database page-size in bytes in p->nPgsz.
**
** If *pRc is non-zero when this function is called, it is a no-op. 
** Otherwise, if an error occurs, an SQLite error code is stored in *pRc
** before returning.
*/
static void fts3DatabasePageSize(int *pRc, Fts3Table *p){
  if( *pRc==SQLITE_OK ){
    int rc;                       /* Return code */
    char *zSql;                   /* SQL text "PRAGMA %Q.page_size" */
    sqlite3_stmt *pStmt;          /* Compiled "PRAGMA %Q.page_size" statement */
  
    zSql = sqlite3_mprintf("PRAGMA %Q.page_size", p->zDb);
    if( !zSql ){
      rc = SQLITE_NOMEM;
    }else{
      rc = sqlite3_prepare(p->db, zSql, -1, &pStmt, 0);
      if( rc==SQLITE_OK ){
        sqlite3_step(pStmt);
        p->nPgsz = sqlite3_column_int(pStmt, 0);
        rc = sqlite3_finalize(pStmt);
      }else if( rc==SQLITE_AUTH ){
        p->nPgsz = 1024;
        rc = SQLITE_OK;
      }
    }
    assert( p->nPgsz>0 || rc!=SQLITE_OK );
    sqlite3_free(zSql);
    *pRc = rc;
  }
}

/*
** "Special" FTS4 arguments are column specifications of the following form:
**
**   <key> = <value>
**
** There may not be whitespace surrounding the "=" character. The <value> 
** term may be quoted, but the <key> may not.
*/
static int fts3IsSpecialColumn(
  const char *z, 
  int *pnKey,
  char **pzValue
){
  char *zValue;
  const char *zCsr = z;

  while( *zCsr!='=' ){
    if( *zCsr=='\0' ) return 0;
    zCsr++;
  }

  *pnKey = (int)(zCsr-z);
  zValue = sqlite3_mprintf("%s", &zCsr[1]);
  if( zValue ){
    sqlite3Fts3Dequote(zValue);
  }
  *pzValue = zValue;
  return 1;
}

/*
** Append the output of a printf() style formatting to an existing string.
*/
static void fts3Appendf(
  int *pRc,                       /* IN/OUT: Error code */
  char **pz,                      /* IN/OUT: Pointer to string buffer */
  const char *zFormat,            /* Printf format string to append */
  ...                             /* Arguments for printf format string */
){
  if( *pRc==SQLITE_OK ){
    va_list ap;
    char *z;
    va_start(ap, zFormat);
    z = sqlite3_vmprintf(zFormat, ap);
    va_end(ap);
    if( z && *pz ){
      char *z2 = sqlite3_mprintf("%s%s", *pz, z);
      sqlite3_free(z);
      z = z2;
    }
    if( z==0 ) *pRc = SQLITE_NOMEM;
    sqlite3_free(*pz);
    *pz = z;
  }
}

/*
** Return a copy of input string zInput enclosed in double-quotes (") and
** with all double quote characters escaped. For example:
**
**     fts3QuoteId("un \"zip\"")   ->    "un \"\"zip\"\""
**
** The pointer returned points to memory obtained from sqlite3_malloc(). It
** is the callers responsibility to call sqlite3_free() to release this
** memory.
*/
static char *fts3QuoteId(char const *zInput){
  int nRet;
  char *zRet;
  nRet = 2 + (int)strlen(zInput)*2 + 1;
  zRet = sqlite3_malloc(nRet);
  if( zRet ){
    int i;
    char *z = zRet;
    *(z++) = '"';
    for(i=0; zInput[i]; i++){
      if( zInput[i]=='"' ) *(z++) = '"';
      *(z++) = zInput[i];
    }
    *(z++) = '"';
    *(z++) = '\0';
  }
  return zRet;
}

/*
** Return a list of comma separated SQL expressions and a FROM clause that 
** could be used in a SELECT statement such as the following:
**
**     SELECT <list of expressions> FROM %_content AS x ...
**
** to return the docid, followed by each column of text data in order
** from left to write. If parameter zFunc is not NULL, then instead of
** being returned directly each column of text data is passed to an SQL
** function named zFunc first. For example, if zFunc is "unzip" and the
** table has the three user-defined columns "a", "b", and "c", the following
** string is returned:
**
**     "docid, unzip(x.'a'), unzip(x.'b'), unzip(x.'c') FROM %_content AS x"
**
** The pointer returned points to a buffer allocated by sqlite3_malloc(). It
** is the responsibility of the caller to eventually free it.
**
** If *pRc is not SQLITE_OK when this function is called, it is a no-op (and
** a NULL pointer is returned). Otherwise, if an OOM error is encountered
** by this function, NULL is returned and *pRc is set to SQLITE_NOMEM. If
** no error occurs, *pRc is left unmodified.
*/
static char *fts3ReadExprList(Fts3Table *p, const char *zFunc, int *pRc){
  char *zRet = 0;
  char *zFree = 0;
  char *zFunction;
  int i;

  if( p->zContentTbl==0 ){
    if( !zFunc ){
      zFunction = "";
    }else{
      zFree = zFunction = fts3QuoteId(zFunc);
    }
    fts3Appendf(pRc, &zRet, "docid");
    for(i=0; i<p->nColumn; i++){
      fts3Appendf(pRc, &zRet, ",%s(x.'c%d%q')", zFunction, i, p->azColumn[i]);
    }
    if( p->zLanguageid ){
      fts3Appendf(pRc, &zRet, ", x.%Q", "langid");
    }
    sqlite3_free(zFree);
  }else{
    fts3Appendf(pRc, &zRet, "rowid");
    for(i=0; i<p->nColumn; i++){
      fts3Appendf(pRc, &zRet, ", x.'%q'", p->azColumn[i]);
    }
    if( p->zLanguageid ){
      fts3Appendf(pRc, &zRet, ", x.%Q", p->zLanguageid);
    }
  }
  fts3Appendf(pRc, &zRet, " FROM '%q'.'%q%s' AS x", 
      p->zDb,
      (p->zContentTbl ? p->zContentTbl : p->zName),
      (p->zContentTbl ? "" : "_content")
  );
  return zRet;
}

/*
** Return a list of N comma separated question marks, where N is the number
** of columns in the %_content table (one for the docid plus one for each
** user-defined text column).
**
** If argument zFunc is not NULL, then all but the first question mark
** is preceded by zFunc and an open bracket, and followed by a closed
** bracket. For example, if zFunc is "zip" and the FTS3 table has three 
** user-defined text columns, the following string is returned:
**
**     "?, zip(?), zip(?), zip(?)"
**
** The pointer returned points to a buffer allocated by sqlite3_malloc(). It
** is the responsibility of the caller to eventually free it.
**
** If *pRc is not SQLITE_OK when this function is called, it is a no-op (and
** a NULL pointer is returned). Otherwise, if an OOM error is encountered
** by this function, NULL is returned and *pRc is set to SQLITE_NOMEM. If
** no error occurs, *pRc is left unmodified.
*/
static char *fts3WriteExprList(Fts3Table *p, const char *zFunc, int *pRc){
  char *zRet = 0;
  char *zFree = 0;
  char *zFunction;
  int i;

  if( !zFunc ){
    zFunction = "";
  }else{
    zFree = zFunction = fts3QuoteId(zFunc);
  }
  fts3Appendf(pRc, &zRet, "?");
  for(i=0; i<p->nColumn; i++){
    fts3Appendf(pRc, &zRet, ",%s(?)", zFunction);
  }
  if( p->zLanguageid ){
    fts3Appendf(pRc, &zRet, ", ?");
  }
  sqlite3_free(zFree);
  return zRet;
}

/*
** This function interprets the string at (*pp) as a non-negative integer
** value. It reads the integer and sets *pnOut to the value read, then 
** sets *pp to point to the byte immediately following the last byte of
** the integer value.
**
** Only decimal digits ('0'..'9') may be part of an integer value. 
**
** If *pp does not being with a decimal digit SQLITE_ERROR is returned and
** the output value undefined. Otherwise SQLITE_OK is returned.
**
** This function is used when parsing the "prefix=" FTS4 parameter.
*/
static int fts3GobbleInt(const char **pp, int *pnOut){
  const int MAX_NPREFIX = 10000000;
  const char *p;                  /* Iterator pointer */
  int nInt = 0;                   /* Output value */

  for(p=*pp; p[0]>='0' && p[0]<='9'; p++){
    nInt = nInt * 10 + (p[0] - '0');
    if( nInt>MAX_NPREFIX ){
      nInt = 0;
      break;
    }
  }
  if( p==*pp ) return SQLITE_ERROR;
  *pnOut = nInt;
  *pp = p;
  return SQLITE_OK;
}

/*
** This function is called to allocate an array of Fts3Index structures
** representing the indexes maintained by the current FTS table. FTS tables
** always maintain the main "terms" index, but may also maintain one or
** more "prefix" indexes, depending on the value of the "prefix=" parameter
** (if any) specified as part of the CREATE VIRTUAL TABLE statement.
**
** Argument zParam is passed the value of the "prefix=" option if one was
** specified, or NULL otherwise.
**
** If no error occurs, SQLITE_OK is returned and *apIndex set to point to
** the allocated array. *pnIndex is set to the number of elements in the
** array. If an error does occur, an SQLite error code is returned.
**
** Regardless of whether or not an error is returned, it is the responsibility
** of the caller to call sqlite3_free() on the output array to free it.
*/
static int fts3PrefixParameter(
  const char *zParam,             /* ABC in prefix=ABC parameter to parse */
  int *pnIndex,                   /* OUT: size of *apIndex[] array */
  struct Fts3Index **apIndex      /* OUT: Array of indexes for this table */
){
  struct Fts3Index *aIndex;       /* Allocated array */
  int nIndex = 1;                 /* Number of entries in array */

  if( zParam && zParam[0] ){
    const char *p;
    nIndex++;
    for(p=zParam; *p; p++){
      if( *p==',' ) nIndex++;
    }
  }

  aIndex = sqlite3_malloc(sizeof(struct Fts3Index) * nIndex);
  *apIndex = aIndex;
  if( !aIndex ){
    return SQLITE_NOMEM;
  }

  memset(aIndex, 0, sizeof(struct Fts3Index) * nIndex);
  if( zParam ){
    const char *p = zParam;
    int i;
    for(i=1; i<nIndex; i++){
      int nPrefix = 0;
      if( fts3GobbleInt(&p, &nPrefix) ) return SQLITE_ERROR;
      assert( nPrefix>=0 );
      if( nPrefix==0 ){
        nIndex--;
        i--;
      }else{
        aIndex[i].nPrefix = nPrefix;
      }
      p++;
    }
  }

  *pnIndex = nIndex;
  return SQLITE_OK;
}

/*
** This function is called when initializing an FTS4 table that uses the
** content=xxx option. It determines the number of and names of the columns
** of the new FTS4 table.
**
** The third argument passed to this function is the value passed to the
** config=xxx option (i.e. "xxx"). This function queries the database for
** a table of that name. If found, the output variables are populated
** as follows:
**
**   *pnCol:   Set to the number of columns table xxx has,
**
**   *pnStr:   Set to the total amount of space required to store a copy
**             of each columns name, including the nul-terminator.
**
**   *pazCol:  Set to point to an array of *pnCol strings. Each string is
**             the name of the corresponding column in table xxx. The array
**             and its contents are allocated using a single allocation. It
**             is the responsibility of the caller to free this allocation
**             by eventually passing the *pazCol value to sqlite3_free().
**
** If the table cannot be found, an error code is returned and the output
** variables are undefined. Or, if an OOM is encountered, SQLITE_NOMEM is
** returned (and the output variables are undefined).
*/
static int fts3ContentColumns(
  sqlite3 *db,                    /* Database handle */
  const char *zDb,                /* Name of db (i.e. "main", "temp" etc.) */
  const char *zTbl,               /* Name of content table */
  const char ***pazCol,           /* OUT: Malloc'd array of column names */
  int *pnCol,                     /* OUT: Size of array *pazCol */
  int *pnStr,                     /* OUT: Bytes of string content */
  char **pzErr                    /* OUT: error message */
){
  int rc = SQLITE_OK;             /* Return code */
  char *zSql;                     /* "SELECT *" statement on zTbl */  
  sqlite3_stmt *pStmt = 0;        /* Compiled version of zSql */

  zSql = sqlite3_mprintf("SELECT * FROM %Q.%Q", zDb, zTbl);
  if( !zSql ){
    rc = SQLITE_NOMEM;
  }else{
    rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
    if( rc!=SQLITE_OK ){
      sqlite3Fts3ErrMsg(pzErr, "%s", sqlite3_errmsg(db));
    }
  }
  sqlite3_free(zSql);

  if( rc==SQLITE_OK ){
    const char **azCol;           /* Output array */
    int nStr = 0;                 /* Size of all column names (incl. 0x00) */
    int nCol;                     /* Number of table columns */
    int i;                        /* Used to iterate through columns */

    /* Loop through the returned columns. Set nStr to the number of bytes of
    ** space required to store a copy of each column name, including the
    ** nul-terminator byte.  */
    nCol = sqlite3_column_count(pStmt);
    for(i=0; i<nCol; i++){
      const char *zCol = sqlite3_column_name(pStmt, i);
      nStr += (int)strlen(zCol) + 1;
    }

    /* Allocate and populate the array to return. */
    azCol = (const char **)sqlite3_malloc(sizeof(char *) * nCol + nStr);
    if( azCol==0 ){
      rc = SQLITE_NOMEM;
    }else{
      char *p = (char *)&azCol[nCol];
      for(i=0; i<nCol; i++){
        const char *zCol = sqlite3_column_name(pStmt, i);
        int n = (int)strlen(zCol)+1;
        memcpy(p, zCol, n);
        azCol[i] = p;
        p += n;
      }
    }
    sqlite3_finalize(pStmt);

    /* Set the output variables. */
    *pnCol = nCol;
    *pnStr = nStr;
    *pazCol = azCol;
  }

  return rc;
}

/*
** This function is the implementation of both the xConnect and xCreate
** methods of the FTS3 virtual table.
**
** The argv[] array contains the following:
**
**   argv[0]   -> module name  ("fts3" or "fts4")
**   argv[1]   -> database name
**   argv[2]   -> table name
**   argv[...] -> "column name" and other module argument fields.
*/
static int fts3InitVtab(
  int isCreate,                   /* True for xCreate, false for xConnect */
  sqlite3 *db,                    /* The SQLite database connection */
  void *pAux,                     /* Hash table containing tokenizers */
  int argc,                       /* Number of elements in argv array */
  const char * const *argv,       /* xCreate/xConnect argument array */
  sqlite3_vtab **ppVTab,          /* Write the resulting vtab structure here */
  char **pzErr                    /* Write any error message here */
){
  Fts3Hash *pHash = (Fts3Hash *)pAux;
  Fts3Table *p = 0;               /* Pointer to allocated vtab */
  int rc = SQLITE_OK;             /* Return code */
  int i;                          /* Iterator variable */
  int nByte;                      /* Size of allocation used for *p */
  int iCol;                       /* Column index */
  int nString = 0;                /* Bytes required to hold all column names */
  int nCol = 0;                   /* Number of columns in the FTS table */
  char *zCsr;                     /* Space for holding column names */
  int nDb;                        /* Bytes required to hold database name */
  int nName;                      /* Bytes required to hold table name */
  int isFts4 = (argv[0][3]=='4'); /* True for FTS4, false for FTS3 */
  const char **aCol;              /* Array of column names */
  sqlite3_tokenizer *pTokenizer = 0;        /* Tokenizer for this table */

  int nIndex = 0;                 /* Size of aIndex[] array */
  struct Fts3Index *aIndex = 0;   /* Array of indexes for this table */

  /* The results of parsing supported FTS4 key=value options: */
  int bNoDocsize = 0;             /* True to omit %_docsize table */
  int bDescIdx = 0;               /* True to store descending indexes */
  char *zPrefix = 0;              /* Prefix parameter value (or NULL) */
  char *zCompress = 0;            /* compress=? parameter (or NULL) */
  char *zUncompress = 0;          /* uncompress=? parameter (or NULL) */
  char *zContent = 0;             /* content=? parameter (or NULL) */
  char *zLanguageid = 0;          /* languageid=? parameter (or NULL) */
  char **azNotindexed = 0;        /* The set of notindexed= columns */
  int nNotindexed = 0;            /* Size of azNotindexed[] array */

  assert( strlen(argv[0])==4 );
  assert( (sqlite3_strnicmp(argv[0], "fts4", 4)==0 && isFts4)
       || (sqlite3_strnicmp(argv[0], "fts3", 4)==0 && !isFts4)
  );

  nDb = (int)strlen(argv[1]) + 1;
  nName = (int)strlen(argv[2]) + 1;

  nByte = sizeof(const char *) * (argc-2);
  aCol = (const char **)sqlite3_malloc(nByte);
  if( aCol ){
    memset((void*)aCol, 0, nByte);
    azNotindexed = (char **)sqlite3_malloc(nByte);
  }
  if( azNotindexed ){
    memset(azNotindexed, 0, nByte);
  }
  if( !aCol || !azNotindexed ){
    rc = SQLITE_NOMEM;
    goto fts3_init_out;
  }

  /* Loop through all of the arguments passed by the user to the FTS3/4
  ** module (i.e. all the column names and special arguments). This loop
  ** does the following:
  **
  **   + Figures out the number of columns the FTSX table will have, and
  **     the number of bytes of space that must be allocated to store copies
  **     of the column names.
  **
  **   + If there is a tokenizer specification included in the arguments,
  **     initializes the tokenizer pTokenizer.
  */
  for(i=3; rc==SQLITE_OK && i<argc; i++){
    char const *z = argv[i];
    int nKey;
    char *zVal;

    /* Check if this is a tokenizer specification */
    if( !pTokenizer 
     && strlen(z)>8
     && 0==sqlite3_strnicmp(z, "tokenize", 8) 
     && 0==sqlite3Fts3IsIdChar(z[8])
    ){
      rc = sqlite3Fts3InitTokenizer(pHash, &z[9], &pTokenizer, pzErr);
    }

    /* Check if it is an FTS4 special argument. */
    else if( isFts4 && fts3IsSpecialColumn(z, &nKey, &zVal) ){
      struct Fts4Option {
        const char *zOpt;
        int nOpt;
      } aFts4Opt[] = {
        { "matchinfo",   9 },     /* 0 -> MATCHINFO */
        { "prefix",      6 },     /* 1 -> PREFIX */
        { "compress",    8 },     /* 2 -> COMPRESS */
        { "uncompress", 10 },     /* 3 -> UNCOMPRESS */
        { "order",       5 },     /* 4 -> ORDER */
        { "content",     7 },     /* 5 -> CONTENT */
        { "languageid", 10 },     /* 6 -> LANGUAGEID */
        { "notindexed", 10 }      /* 7 -> NOTINDEXED */
      };

      int iOpt;
      if( !zVal ){
        rc = SQLITE_NOMEM;
      }else{
        for(iOpt=0; iOpt<SizeofArray(aFts4Opt); iOpt++){
          struct Fts4Option *pOp = &aFts4Opt[iOpt];
          if( nKey==pOp->nOpt && !sqlite3_strnicmp(z, pOp->zOpt, pOp->nOpt) ){
            break;
          }
        }
        if( iOpt==SizeofArray(aFts4Opt) ){
          sqlite3Fts3ErrMsg(pzErr, "unrecognized parameter: %s", z);
          rc = SQLITE_ERROR;
        }else{
          switch( iOpt ){
            case 0:               /* MATCHINFO */
              if( strlen(zVal)!=4 || sqlite3_strnicmp(zVal, "fts3", 4) ){
                sqlite3Fts3ErrMsg(pzErr, "unrecognized matchinfo: %s", zVal);
                rc = SQLITE_ERROR;
              }
              bNoDocsize = 1;
              break;

            case 1:               /* PREFIX */
              sqlite3_free(zPrefix);
              zPrefix = zVal;
              zVal = 0;
              break;

            case 2:               /* COMPRESS */
              sqlite3_free(zCompress);
              zCompress = zVal;
              zVal = 0;
              break;

            case 3:               /* UNCOMPRESS */
              sqlite3_free(zUncompress);
              zUncompress = zVal;
              zVal = 0;
              break;

            case 4:               /* ORDER */
              if( (strlen(zVal)!=3 || sqlite3_strnicmp(zVal, "asc", 3)) 
               && (strlen(zVal)!=4 || sqlite3_strnicmp(zVal, "desc", 4)) 
              ){
                sqlite3Fts3ErrMsg(pzErr, "unrecognized order: %s", zVal);
                rc = SQLITE_ERROR;
              }
              bDescIdx = (zVal[0]=='d' || zVal[0]=='D');
              break;

            case 5:              /* CONTENT */
              sqlite3_free(zContent);
              zContent = zVal;
              zVal = 0;
              break;

            case 6:              /* LANGUAGEID */
              assert( iOpt==6 );
              sqlite3_free(zLanguageid);
              zLanguageid = zVal;
              zVal = 0;
              break;

            case 7:              /* NOTINDEXED */
              azNotindexed[nNotindexed++] = zVal;
              zVal = 0;
              break;
          }
        }
        sqlite3_free(zVal);
      }
    }

    /* Otherwise, the argument is a column name. */
    else {
      nString += (int)(strlen(z) + 1);
      aCol[nCol++] = z;
    }
  }

  /* If a content=xxx option was specified, the following:
  **
  **   1. Ignore any compress= and uncompress= options.
  **
  **   2. If no column names were specified as part of the CREATE VIRTUAL
  **      TABLE statement, use all columns from the content table.
  */
  if( rc==SQLITE_OK && zContent ){
    sqlite3_free(zCompress); 
    sqlite3_free(zUncompress); 
    zCompress = 0;
    zUncompress = 0;
    if( nCol==0 ){
      sqlite3_free((void*)aCol); 
      aCol = 0;
      rc = fts3ContentColumns(db, argv[1], zContent,&aCol,&nCol,&nString,pzErr);

      /* If a languageid= option was specified, remove the language id
      ** column from the aCol[] array. */ 
      if( rc==SQLITE_OK && zLanguageid ){
        int j;
        for(j=0; j<nCol; j++){
          if( sqlite3_stricmp(zLanguageid, aCol[j])==0 ){
            int k;
            for(k=j; k<nCol; k++) aCol[k] = aCol[k+1];
            nCol--;
            break;
          }
        }
      }
    }
  }
  if( rc!=SQLITE_OK ) goto fts3_init_out;

  if( nCol==0 ){
    assert( nString==0 );
    aCol[0] = "content";
    nString = 8;
    nCol = 1;
  }

  if( pTokenizer==0 ){
    rc = sqlite3Fts3InitTokenizer(pHash, "simple", &pTokenizer, pzErr);
    if( rc!=SQLITE_OK ) goto fts3_init_out;
  }
  assert( pTokenizer );

  rc = fts3PrefixParameter(zPrefix, &nIndex, &aIndex);
  if( rc==SQLITE_ERROR ){
    assert( zPrefix );
    sqlite3Fts3ErrMsg(pzErr, "error parsing prefix parameter: %s", zPrefix);
  }
  if( rc!=SQLITE_OK ) goto fts3_init_out;

  /* Allocate and populate the Fts3Table structure. */
  nByte = sizeof(Fts3Table) +                  /* Fts3Table */
          nCol * sizeof(char *) +              /* azColumn */
          nIndex * sizeof(struct Fts3Index) +  /* aIndex */
          nCol * sizeof(u8) +                  /* abNotindexed */
          nName +                              /* zName */
          nDb +                                /* zDb */
          nString;                             /* Space for azColumn strings */
  p = (Fts3Table*)sqlite3_malloc(nByte);
  if( p==0 ){
    rc = SQLITE_NOMEM;
    goto fts3_init_out;
  }
  memset(p, 0, nByte);
  p->db = db;
  p->nColumn = nCol;
  p->nPendingData = 0;
  p->azColumn = (char **)&p[1];
  p->pTokenizer = pTokenizer;
  p->nMaxPendingData = FTS3_MAX_PENDING_DATA;
  p->bHasDocsize = (isFts4 && bNoDocsize==0);
  p->bHasStat = isFts4;
  p->bFts4 = isFts4;
  p->bDescIdx = bDescIdx;
  p->nAutoincrmerge = 0xff;   /* 0xff means setting unknown */
  p->zContentTbl = zContent;
  p->zLanguageid = zLanguageid;
  zContent = 0;
  zLanguageid = 0;
  TESTONLY( p->inTransaction = -1 );
  TESTONLY( p->mxSavepoint = -1 );

  p->aIndex = (struct Fts3Index *)&p->azColumn[nCol];
  memcpy(p->aIndex, aIndex, sizeof(struct Fts3Index) * nIndex);
  p->nIndex = nIndex;
  for(i=0; i<nIndex; i++){
    fts3HashInit(&p->aIndex[i].hPending, FTS3_HASH_STRING, 1);
  }
  p->abNotindexed = (u8 *)&p->aIndex[nIndex];

  /* Fill in the zName and zDb fields of the vtab structure. */
  zCsr = (char *)&p->abNotindexed[nCol];
  p->zName = zCsr;
  memcpy(zCsr, argv[2], nName);
  zCsr += nName;
  p->zDb = zCsr;
  memcpy(zCsr, argv[1], nDb);
  zCsr += nDb;

  /* Fill in the azColumn array */
  for(iCol=0; iCol<nCol; iCol++){
    char *z; 
    int n = 0;
    z = (char *)sqlite3Fts3NextToken(aCol[iCol], &n);
    memcpy(zCsr, z, n);
    zCsr[n] = '\0';
    sqlite3Fts3Dequote(zCsr);
    p->azColumn[iCol] = zCsr;
    zCsr += n+1;
    assert( zCsr <= &((char *)p)[nByte] );
  }

  /* Fill in the abNotindexed array */
  for(iCol=0; iCol<nCol; iCol++){
    int n = (int)strlen(p->azColumn[iCol]);
    for(i=0; i<nNotindexed; i++){
      char *zNot = azNotindexed[i];
      if( zNot && n==(int)strlen(zNot)
       && 0==sqlite3_strnicmp(p->azColumn[iCol], zNot, n) 
      ){
        p->abNotindexed[iCol] = 1;
        sqlite3_free(zNot);
        azNotindexed[i] = 0;
      }
    }
  }
  for(i=0; i<nNotindexed; i++){
    if( azNotindexed[i] ){
      sqlite3Fts3ErrMsg(pzErr, "no such column: %s", azNotindexed[i]);
      rc = SQLITE_ERROR;
    }
  }

  if( rc==SQLITE_OK && (zCompress==0)!=(zUncompress==0) ){
    char const *zMiss = (zCompress==0 ? "compress" : "uncompress");
    rc = SQLITE_ERROR;
    sqlite3Fts3ErrMsg(pzErr, "missing %s parameter in fts4 constructor", zMiss);
  }
  p->zReadExprlist = fts3ReadExprList(p, zUncompress, &rc);
  p->zWriteExprlist = fts3WriteExprList(p, zCompress, &rc);
  if( rc!=SQLITE_OK ) goto fts3_init_out;

  /* If this is an xCreate call, create the underlying tables in the 
  ** database. TODO: For xConnect(), it could verify that said tables exist.
  */
  if( isCreate ){
    rc = fts3CreateTables(p);
  }

  /* Check to see if a legacy fts3 table has been "upgraded" by the
  ** addition of a %_stat table so that it can use incremental merge.
  */
  if( !isFts4 && !isCreate ){
    p->bHasStat = 2;
  }

  /* Figure out the page-size for the database. This is required in order to
  ** estimate the cost of loading large doclists from the database.  */
  fts3DatabasePageSize(&rc, p);
  p->nNodeSize = p->nPgsz-35;

  /* Declare the table schema to SQLite. */
  fts3DeclareVtab(&rc, p);

fts3_init_out:
  sqlite3_free(zPrefix);
  sqlite3_free(aIndex);
  sqlite3_free(zCompress);
  sqlite3_free(zUncompress);
  sqlite3_free(zContent);
  sqlite3_free(zLanguageid);
  for(i=0; i<nNotindexed; i++) sqlite3_free(azNotindexed[i]);
  sqlite3_free((void *)aCol);
  sqlite3_free((void *)azNotindexed);
  if( rc!=SQLITE_OK ){
    if( p ){
      fts3DisconnectMethod((sqlite3_vtab *)p);
    }else if( pTokenizer ){
      pTokenizer->pModule->xDestroy(pTokenizer);
    }
  }else{
    assert( p->pSegments==0 );
    *ppVTab = &p->base;
  }
  return rc;
}

/*
** The xConnect() and xCreate() methods for the virtual table. All the
** work is done in function fts3InitVtab().
*/
static int fts3ConnectMethod(
  sqlite3 *db,                    /* Database connection */
  void *pAux,                     /* Pointer to tokenizer hash table */
  int argc,                       /* Number of elements in argv array */
  const char * const *argv,       /* xCreate/xConnect argument array */
  sqlite3_vtab **ppVtab,          /* OUT: New sqlite3_vtab object */
  char **pzErr                    /* OUT: sqlite3_malloc'd error message */
){
  return fts3InitVtab(0, db, pAux, argc, argv, ppVtab, pzErr);
}
static int fts3CreateMethod(
  sqlite3 *db,                    /* Database connection */
  void *pAux,                     /* Pointer to tokenizer hash table */
  int argc,                       /* Number of elements in argv array */
  const char * const *argv,       /* xCreate/xConnect argument array */
  sqlite3_vtab **ppVtab,          /* OUT: New sqlite3_vtab object */
  char **pzErr                    /* OUT: sqlite3_malloc'd error message */
){
  return fts3InitVtab(1, db, pAux, argc, argv, ppVtab, pzErr);
}

/*
** Set the pIdxInfo->estimatedRows variable to nRow. Unless this
** extension is currently being used by a version of SQLite too old to
** support estimatedRows. In that case this function is a no-op.
*/
static void fts3SetEstimatedRows(sqlite3_index_info *pIdxInfo, i64 nRow){
#if SQLITE_VERSION_NUMBER>=3008002
  if( sqlite3_libversion_number()>=3008002 ){
    pIdxInfo->estimatedRows = nRow;
  }
#endif
}

/* 
** Implementation of the xBestIndex method for FTS3 tables. There
** are three possible strategies, in order of preference:
**
**   1. Direct lookup by rowid or docid. 
**   2. Full-text search using a MATCH operator on a non-docid column.
**   3. Linear scan of %_content table.
*/
static int fts3BestIndexMethod(sqlite3_vtab *pVTab, sqlite3_index_info *pInfo){
  Fts3Table *p = (Fts3Table *)pVTab;
  int i;                          /* Iterator variable */
  int iCons = -1;                 /* Index of constraint to use */

  int iLangidCons = -1;           /* Index of langid=x constraint, if present */
  int iDocidGe = -1;              /* Index of docid>=x constraint, if present */
  int iDocidLe = -1;              /* Index of docid<=x constraint, if present */
  int iIdx;

  /* By default use a full table scan. This is an expensive option,
  ** so search through the constraints to see if a more efficient 
  ** strategy is possible.
  */
  pInfo->idxNum = FTS3_FULLSCAN_SEARCH;
  pInfo->estimatedCost = 5000000;
  for(i=0; i<pInfo->nConstraint; i++){
    int bDocid;                 /* True if this constraint is on docid */
    struct sqlite3_index_constraint *pCons = &pInfo->aConstraint[i];
    if( pCons->usable==0 ){
      if( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH ){
        /* There exists an unusable MATCH constraint. This means that if
        ** the planner does elect to use the results of this call as part
        ** of the overall query plan the user will see an "unable to use
        ** function MATCH in the requested context" error. To discourage
        ** this, return a very high cost here.  */
        pInfo->idxNum = FTS3_FULLSCAN_SEARCH;
        pInfo->estimatedCost = 1e50;
        fts3SetEstimatedRows(pInfo, ((sqlite3_int64)1) << 50);
        return SQLITE_OK;
      }
      continue;
    }

    bDocid = (pCons->iColumn<0 || pCons->iColumn==p->nColumn+1);

    /* A direct lookup on the rowid or docid column. Assign a cost of 1.0. */
    if( iCons<0 && pCons->op==SQLITE_INDEX_CONSTRAINT_EQ && bDocid ){
      pInfo->idxNum = FTS3_DOCID_SEARCH;
      pInfo->estimatedCost = 1.0;
      iCons = i;
    }

    /* A MATCH constraint. Use a full-text search.
    **
    ** If there is more than one MATCH constraint available, use the first
    ** one encountered. If there is both a MATCH constraint and a direct
    ** rowid/docid lookup, prefer the MATCH strategy. This is done even 
    ** though the rowid/docid lookup is faster than a MATCH query, selecting
    ** it would lead to an "unable to use function MATCH in the requested 
    ** context" error.
    */
    if( pCons->op==SQLITE_INDEX_CONSTRAINT_MATCH 
     && pCons->iColumn>=0 && pCons->iColumn<=p->nColumn
    ){
      pInfo->idxNum = FTS3_FULLTEXT_SEARCH + pCons->iColumn;
      pInfo->estimatedCost = 2.0;
      iCons = i;
    }

    /* Equality constraint on the langid column */
    if( pCons->op==SQLITE_INDEX_CONSTRAINT_EQ 
     && pCons->iColumn==p->nColumn + 2
    ){
      iLangidCons = i;
    }

    if( bDocid ){
      switch( pCons->op ){
        case SQLITE_INDEX_CONSTRAINT_GE:
        case SQLITE_INDEX_CONSTRAINT_GT:
          iDocidGe = i;
          break;

        case SQLITE_INDEX_CONSTRAINT_LE:
        case SQLITE_INDEX_CONSTRAINT_LT:
          iDocidLe = i;
          break;
      }
    }
  }

  iIdx = 1;
  if( iCons>=0 ){
    pInfo->aConstraintUsage[iCons].argvIndex = iIdx++;
    pInfo->aConstraintUsage[iCons].omit = 1;
  } 
  if( iLangidCons>=0 ){
    pInfo->idxNum |= FTS3_HAVE_LANGID;
    pInfo->aConstraintUsage[iLangidCons].argvIndex = iIdx++;
  } 
  if( iDocidGe>=0 ){
    pInfo->idxNum |= FTS3_HAVE_DOCID_GE;
    pInfo->aConstraintUsage[iDocidGe].argvIndex = iIdx++;
  } 
  if( iDocidLe>=0 ){
    pInfo->idxNum |= FTS3_HAVE_DOCID_LE;
    pInfo->aConstraintUsage[iDocidLe].argvIndex = iIdx++;
  } 

  /* Regardless of the strategy selected, FTS can deliver rows in rowid (or
  ** docid) order. Both ascending and descending are possible. 
  */
  if( pInfo->nOrderBy==1 ){
    struct sqlite3_index_orderby *pOrder = &pInfo->aOrderBy[0];
    if( pOrder->iColumn<0 || pOrder->iColumn==p->nColumn+1 ){
      if( pOrder->desc ){
        pInfo->idxStr = "DESC";
      }else{
        pInfo->idxStr = "ASC";
      }
      pInfo->orderByConsumed = 1;
    }
  }

  assert( p->pSegments==0 );
  return SQLITE_OK;
}

/*
** Implementation of xOpen method.
*/
static int fts3OpenMethod(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCsr){
  sqlite3_vtab_cursor *pCsr;               /* Allocated cursor */

  UNUSED_PARAMETER(pVTab);

  /* Allocate a buffer large enough for an Fts3Cursor structure. If the
  ** allocation succeeds, zero it and return SQLITE_OK. Otherwise, 
  ** if the allocation fails, return SQLITE_NOMEM.
  */
  *ppCsr = pCsr = (sqlite3_vtab_cursor *)sqlite3_malloc(sizeof(Fts3Cursor));
  if( !pCsr ){
    return SQLITE_NOMEM;
  }
  memset(pCsr, 0, sizeof(Fts3Cursor));
  return SQLITE_OK;
}

/*
** Close the cursor.  For additional information see the documentation
** on the xClose method of the virtual table interface.
*/
static int fts3CloseMethod(sqlite3_vtab_cursor *pCursor){
  Fts3Cursor *pCsr = (Fts3Cursor *)pCursor;
  assert( ((Fts3Table *)pCsr->base.pVtab)->pSegments==0 );
  sqlite3_finalize(pCsr->pStmt);
  sqlite3Fts3ExprFree(pCsr->pExpr);
  sqlite3Fts3FreeDeferredTokens(pCsr);
  sqlite3_free(pCsr->aDoclist);
  sqlite3Fts3MIBufferFree(pCsr->pMIBuffer);
  assert( ((Fts3Table *)pCsr->base.pVtab)->pSegments==0 );
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** If pCsr->pStmt has not been prepared (i.e. if pCsr->pStmt==0), then
** compose and prepare an SQL statement of the form:
**
**    "SELECT <columns> FROM %_content WHERE rowid = ?"
**
** (or the equivalent for a content=xxx table) and set pCsr->pStmt to
** it. If an error occurs, return an SQLite error code.
**
** Otherwise, set *ppStmt to point to pCsr->pStmt and return SQLITE_OK.
*/
static int fts3CursorSeekStmt(Fts3Cursor *pCsr, sqlite3_stmt **ppStmt){
  int rc = SQLITE_OK;
  if( pCsr->pStmt==0 ){
    Fts3Table *p = (Fts3Table *)pCsr->base.pVtab;
    char *zSql;
    zSql = sqlite3_mprintf("SELECT %s WHERE rowid = ?", p->zReadExprlist);
    if( !zSql ) return SQLITE_NOMEM;
    rc = sqlite3_prepare_v2(p->db, zSql, -1, &pCsr->pStmt, 0);
    sqlite3_free(zSql);
  }
  *ppStmt = pCsr->pStmt;
  return rc;
}

/*
** Position the pCsr->pStmt statement so that it is on the row
** of the %_content table that contains the last match.  Return
** SQLITE_OK on success.  
*/
static int fts3CursorSeek(sqlite3_context *pContext, Fts3Cursor *pCsr){
  int rc = SQLITE_OK;
  if( pCsr->isRequireSeek ){
    sqlite3_stmt *pStmt = 0;

    rc = fts3CursorSeekStmt(pCsr, &pStmt);
    if( rc==SQLITE_OK ){
      sqlite3_bind_int64(pCsr->pStmt, 1, pCsr->iPrevId);
      pCsr->isRequireSeek = 0;
      if( SQLITE_ROW==sqlite3_step(pCsr->pStmt) ){
        return SQLITE_OK;
      }else{
        rc = sqlite3_reset(pCsr->pStmt);
        if( rc==SQLITE_OK && ((Fts3Table *)pCsr->base.pVtab)->zContentTbl==0 ){
          /* If no row was found and no error has occurred, then the %_content
          ** table is missing a row that is present in the full-text index.
          ** The data structures are corrupt.  */
          rc = FTS_CORRUPT_VTAB;
          pCsr->isEof = 1;
        }
      }
    }
  }

  if( rc!=SQLITE_OK && pContext ){
    sqlite3_result_error_code(pContext, rc);
  }
  return rc;
}

/*
** This function is used to process a single interior node when searching
** a b-tree for a term or term prefix. The node data is passed to this 
** function via the zNode/nNode parameters. The term to search for is
** passed in zTerm/nTerm.
**
** If piFirst is not NULL, then this function sets *piFirst to the blockid
** of the child node that heads the sub-tree that may contain the term.
**
** If piLast is not NULL, then *piLast is set to the right-most child node
** that heads a sub-tree that may contain a term for which zTerm/nTerm is
** a prefix.
**
** If an OOM error occurs, SQLITE_NOMEM is returned. Otherwise, SQLITE_OK.
*/
static int fts3ScanInteriorNode(
  const char *zTerm,              /* Term to select leaves for */
  int nTerm,                      /* Size of term zTerm in bytes */
  const char *zNode,              /* Buffer containing segment interior node */
  int nNode,                      /* Size of buffer at zNode */
  sqlite3_int64 *piFirst,         /* OUT: Selected child node */
  sqlite3_int64 *piLast           /* OUT: Selected child node */
){
  int rc = SQLITE_OK;             /* Return code */
  const char *zCsr = zNode;       /* Cursor to iterate through node */
  const char *zEnd = &zCsr[nNode];/* End of interior node buffer */
  char *zBuffer = 0;              /* Buffer to load terms into */
  int nAlloc = 0;                 /* Size of allocated buffer */
  int isFirstTerm = 1;            /* True when processing first term on page */
  sqlite3_int64 iChild;           /* Block id of child node to descend to */

  /* Skip over the 'height' varint that occurs at the start of every 
  ** interior node. Then load the blockid of the left-child of the b-tree
  ** node into variable iChild.  
  **
  ** Even if the data structure on disk is corrupted, this (reading two
  ** varints from the buffer) does not risk an overread. If zNode is a
  ** root node, then the buffer comes from a SELECT statement. SQLite does
  ** not make this guarantee explicitly, but in practice there are always
  ** either more than 20 bytes of allocated space following the nNode bytes of
  ** contents, or two zero bytes. Or, if the node is read from the %_segments
  ** table, then there are always 20 bytes of zeroed padding following the
  ** nNode bytes of content (see sqlite3Fts3ReadBlock() for details).
  */
  zCsr += sqlite3Fts3GetVarint(zCsr, &iChild);
  zCsr += sqlite3Fts3GetVarint(zCsr, &iChild);
  if( zCsr>zEnd ){
    return FTS_CORRUPT_VTAB;
  }
  
  while( zCsr<zEnd && (piFirst || piLast) ){
    int cmp;                      /* memcmp() result */
    int nSuffix;                  /* Size of term suffix */
    int nPrefix = 0;              /* Size of term prefix */
    int nBuffer;                  /* Total term size */
  
    /* Load the next term on the node into zBuffer. Use realloc() to expand
    ** the size of zBuffer if required.  */
    if( !isFirstTerm ){
      zCsr += fts3GetVarint32(zCsr, &nPrefix);
    }
    isFirstTerm = 0;
    zCsr += fts3GetVarint32(zCsr, &nSuffix);
    
    if( nPrefix<0 || nSuffix<0 || &zCsr[nSuffix]>zEnd ){
      rc = FTS_CORRUPT_VTAB;
      goto finish_scan;
    }
    if( nPrefix+nSuffix>nAlloc ){
      char *zNew;
      nAlloc = (nPrefix+nSuffix) * 2;
      zNew = (char *)sqlite3_realloc(zBuffer, nAlloc);
      if( !zNew ){
        rc = SQLITE_NOMEM;
        goto finish_scan;
      }
      zBuffer = zNew;
    }
    assert( zBuffer );
    memcpy(&zBuffer[nPrefix], zCsr, nSuffix);
    nBuffer = nPrefix + nSuffix;
    zCsr += nSuffix;

    /* Compare the term we are searching for with the term just loaded from
    ** the interior node. If the specified term is greater than or equal
    ** to the term from the interior node, then all terms on the sub-tree 
    ** headed by node iChild are smaller than zTerm. No need to search 
    ** iChild.
    **
    ** If the interior node term is larger than the specified term, then
    ** the tree headed by iChild may contain the specified term.
    */
    cmp = memcmp(zTerm, zBuffer, (nBuffer>nTerm ? nTerm : nBuffer));
    if( piFirst && (cmp<0 || (cmp==0 && nBuffer>nTerm)) ){
      *piFirst = iChild;
      piFirst = 0;
    }

    if( piLast && cmp<0 ){
      *piLast = iChild;
      piLast = 0;
    }

    iChild++;
  };

  if( piFirst ) *piFirst = iChild;
  if( piLast ) *piLast = iChild;

 finish_scan:
  sqlite3_free(zBuffer);
  return rc;
}


/*
** The buffer pointed to by argument zNode (size nNode bytes) contains an
** interior node of a b-tree segment. The zTerm buffer (size nTerm bytes)
** contains a term. This function searches the sub-tree headed by the zNode
** node for the range of leaf nodes that may contain the specified term
** or terms for which the specified term is a prefix.
**
** If piLeaf is not NULL, then *piLeaf is set to the blockid of the 
** left-most leaf node in the tree that may contain the specified term.
** If piLeaf2 is not NULL, then *piLeaf2 is set to the blockid of the
** right-most leaf node that may contain a term for which the specified
** term is a prefix.
**
** It is possible that the range of returned leaf nodes does not contain 
** the specified term or any terms for which it is a prefix. However, if the 
** segment does contain any such terms, they are stored within the identified
** range. Because this function only inspects interior segment nodes (and
** never loads leaf nodes into memory), it is not possible to be sure.
**
** If an error occurs, an error code other than SQLITE_OK is returned.
*/ 
static int fts3SelectLeaf(
  Fts3Table *p,                   /* Virtual table handle */
  const char *zTerm,              /* Term to select leaves for */
  int nTerm,                      /* Size of term zTerm in bytes */
  const char *zNode,              /* Buffer containing segment interior node */
  int nNode,                      /* Size of buffer at zNode */
  sqlite3_int64 *piLeaf,          /* Selected leaf node */
  sqlite3_int64 *piLeaf2          /* Selected leaf node */
){
  int rc = SQLITE_OK;             /* Return code */
  int iHeight;                    /* Height of this node in tree */

  assert( piLeaf || piLeaf2 );

  fts3GetVarint32(zNode, &iHeight);
  rc = fts3ScanInteriorNode(zTerm, nTerm, zNode, nNode, piLeaf, piLeaf2);
  assert( !piLeaf2 || !piLeaf || rc!=SQLITE_OK || (*piLeaf<=*piLeaf2) );

  if( rc==SQLITE_OK && iHeight>1 ){
    char *zBlob = 0;              /* Blob read from %_segments table */
    int nBlob = 0;                /* Size of zBlob in bytes */

    if( piLeaf && piLeaf2 && (*piLeaf!=*piLeaf2) ){
      rc = sqlite3Fts3ReadBlock(p, *piLeaf, &zBlob, &nBlob, 0);
      if( rc==SQLITE_OK ){
        rc = fts3SelectLeaf(p, zTerm, nTerm, zBlob, nBlob, piLeaf, 0);
      }
      sqlite3_free(zBlob);
      piLeaf = 0;
      zBlob = 0;
    }

    if( rc==SQLITE_OK ){
      rc = sqlite3Fts3ReadBlock(p, piLeaf?*piLeaf:*piLeaf2, &zBlob, &nBlob, 0);
    }
    if( rc==SQLITE_OK ){
      rc = fts3SelectLeaf(p, zTerm, nTerm, zBlob, nBlob, piLeaf, piLeaf2);
    }
    sqlite3_free(zBlob);
  }

  return rc;
}

/*
** This function is used to create delta-encoded serialized lists of FTS3 
** varints. Each call to this function appends a single varint to a list.
*/
static void fts3PutDeltaVarint(
  char **pp,                      /* IN/OUT: Output pointer */
  sqlite3_int64 *piPrev,          /* IN/OUT: Previous value written to list */
  sqlite3_int64 iVal              /* Write this value to the list */
){
  assert( iVal-*piPrev > 0 || (*piPrev==0 && iVal==0) );
  *pp += sqlite3Fts3PutVarint(*pp, iVal-*piPrev);
  *piPrev = iVal;
}

/*
** When this function is called, *ppPoslist is assumed to point to the 
** start of a position-list. After it returns, *ppPoslist points to the
** first byte after the position-list.
**
** A position list is list of positions (delta encoded) and columns for 
** a single document record of a doclist.  So, in other words, this
** routine advances *ppPoslist so that it points to the next docid in
** the doclist, or to the first byte past the end of the doclist.
**
** If pp is not NULL, then the contents of the position list are copied
** to *pp. *pp is set to point to the first byte past the last byte copied
** before this function returns.
*/
static void fts3PoslistCopy(char **pp, char **ppPoslist){
  char *pEnd = *ppPoslist;
  char c = 0;

  /* The end of a position list is marked by a zero encoded as an FTS3 
  ** varint. A single POS_END (0) byte. Except, if the 0 byte is preceded by
  ** a byte with the 0x80 bit set, then it is not a varint 0, but the tail
  ** of some other, multi-byte, value.
  **
  ** The following while-loop moves pEnd to point to the first byte that is not 
  ** immediately preceded by a byte with the 0x80 bit set. Then increments
  ** pEnd once more so that it points to the byte immediately following the
  ** last byte in the position-list.
  */
  while( *pEnd | c ){
    c = *pEnd++ & 0x80;
    testcase( c!=0 && (*pEnd)==0 );
  }
  pEnd++;  /* Advance past the POS_END terminator byte */

  if( pp ){
    int n = (int)(pEnd - *ppPoslist);
    char *p = *pp;
    memcpy(p, *ppPoslist, n);
    p += n;
    *pp = p;
  }
  *ppPoslist = pEnd;
}

/*
** When this function is called, *ppPoslist is assumed to point to the 
** start of a column-list. After it returns, *ppPoslist points to the
** to the terminator (POS_COLUMN or POS_END) byte of the column-list.
**
** A column-list is list of delta-encoded positions for a single column
** within a single document within a doclist.
**
** The column-list is terminated either by a POS_COLUMN varint (1) or
** a POS_END varint (0).  This routine leaves *ppPoslist pointing to
** the POS_COLUMN or POS_END that terminates the column-list.
**
** If pp is not NULL, then the contents of the column-list are copied
** to *pp. *pp is set to point to the first byte past the last byte copied
** before this function returns.  The POS_COLUMN or POS_END terminator
** is not copied into *pp.
*/
static void fts3ColumnlistCopy(char **pp, char **ppPoslist){
  char *pEnd = *ppPoslist;
  char c = 0;

  /* A column-list is terminated by either a 0x01 or 0x00 byte that is
  ** not part of a multi-byte varint.
  */
  while( 0xFE & (*pEnd | c) ){
    c = *pEnd++ & 0x80;
    testcase( c!=0 && ((*pEnd)&0xfe)==0 );
  }
  if( pp ){
    int n = (int)(pEnd - *ppPoslist);
    char *p = *pp;
    memcpy(p, *ppPoslist, n);
    p += n;
    *pp = p;
  }
  *ppPoslist = pEnd;
}

/*
** Value used to signify the end of an position-list. This is safe because
** it is not possible to have a document with 2^31 terms.
*/
#define POSITION_LIST_END 0x7fffffff

/*
** This function is used to help parse position-lists. When this function is
** called, *pp may point to the start of the next varint in the position-list
** being parsed, or it may point to 1 byte past the end of the position-list
** (in which case **pp will be a terminator bytes POS_END (0) or
** (1)).
**
** If *pp points past the end of the current position-list, set *pi to 
** POSITION_LIST_END and return. Otherwise, read the next varint from *pp,
** increment the current value of *pi by the value read, and set *pp to
** point to the next value before returning.
**
** Before calling this routine *pi must be initialized to the value of
** the previous position, or zero if we are reading the first position
** in the position-list.  Because positions are delta-encoded, the value
** of the previous position is needed in order to compute the value of
** the next position.
*/
static void fts3ReadNextPos(
  char **pp,                    /* IN/OUT: Pointer into position-list buffer */
  sqlite3_int64 *pi             /* IN/OUT: Value read from position-list */
){
  if( (**pp)&0xFE ){
    fts3GetDeltaVarint(pp, pi);
    *pi -= 2;
  }else{
    *pi = POSITION_LIST_END;
  }
}

/*
** If parameter iCol is not 0, write an POS_COLUMN (1) byte followed by
** the value of iCol encoded as a varint to *pp.   This will start a new
** column list.
**
** Set *pp to point to the byte just after the last byte written before 
** returning (do not modify it if iCol==0). Return the total number of bytes
** written (0 if iCol==0).
*/
static int fts3PutColNumber(char **pp, int iCol){
  int n = 0;                      /* Number of bytes written */
  if( iCol ){
    char *p = *pp;                /* Output pointer */
    n = 1 + sqlite3Fts3PutVarint(&p[1], iCol);
    *p = 0x01;
    *pp = &p[n];
  }
  return n;
}

/*
** Compute the union of two position lists.  The output written
** into *pp contains all positions of both *pp1 and *pp2 in sorted
** order and with any duplicates removed.  All pointers are
** updated appropriately.   The caller is responsible for insuring
** that there is enough space in *pp to hold the complete output.
*/
static void fts3PoslistMerge(
  char **pp,                      /* Output buffer */
  char **pp1,                     /* Left input list */
  char **pp2                      /* Right input list */
){
  char *p = *pp;
  char *p1 = *pp1;
  char *p2 = *pp2;

  while( *p1 || *p2 ){
    int iCol1;         /* The current column index in pp1 */
    int iCol2;         /* The current column index in pp2 */

    if( *p1==POS_COLUMN ) fts3GetVarint32(&p1[1], &iCol1);
    else if( *p1==POS_END ) iCol1 = POSITION_LIST_END;
    else iCol1 = 0;

    if( *p2==POS_COLUMN ) fts3GetVarint32(&p2[1], &iCol2);
    else if( *p2==POS_END ) iCol2 = POSITION_LIST_END;
    else iCol2 = 0;

    if( iCol1==iCol2 ){
      sqlite3_int64 i1 = 0;       /* Last position from pp1 */
      sqlite3_int64 i2 = 0;       /* Last position from pp2 */
      sqlite3_int64 iPrev = 0;
      int n = fts3PutColNumber(&p, iCol1);
      p1 += n;
      p2 += n;

      /* At this point, both p1 and p2 point to the start of column-lists
      ** for the same column (the column with index iCol1 and iCol2).
      ** A column-list is a list of non-negative delta-encoded varints, each 
      ** incremented by 2 before being stored. Each list is terminated by a
      ** POS_END (0) or POS_COLUMN (1). The following block merges the two lists
      ** and writes the results to buffer p. p is left pointing to the byte
      ** after the list written. No terminator (POS_END or POS_COLUMN) is
      ** written to the output.
      */
      fts3GetDeltaVarint(&p1, &i1);
      fts3GetDeltaVarint(&p2, &i2);
      do {
        fts3PutDeltaVarint(&p, &iPrev, (i1<i2) ? i1 : i2); 
        iPrev -= 2;
        if( i1==i2 ){
          fts3ReadNextPos(&p1, &i1);
          fts3ReadNextPos(&p2, &i2);
        }else if( i1<i2 ){
          fts3ReadNextPos(&p1, &i1);
        }else{
          fts3ReadNextPos(&p2, &i2);
        }
      }while( i1!=POSITION_LIST_END || i2!=POSITION_LIST_END );
    }else if( iCol1<iCol2 ){
      p1 += fts3PutColNumber(&p, iCol1);
      fts3ColumnlistCopy(&p, &p1);
    }else{
      p2 += fts3PutColNumber(&p, iCol2);
      fts3ColumnlistCopy(&p, &p2);
    }
  }

  *p++ = POS_END;
  *pp = p;
  *pp1 = p1 + 1;
  *pp2 = p2 + 1;
}

/*
** This function is used to merge two position lists into one. When it is
** called, *pp1 and *pp2 must both point to position lists. A position-list is
** the part of a doclist that follows each document id. For example, if a row
** contains:
**
**     'a b c'|'x y z'|'a b b a'
**
** Then the position list for this row for token 'b' would consist of:
**
**     0x02 0x01 0x02 0x03 0x03 0x00
**
** When this function returns, both *pp1 and *pp2 are left pointing to the
** byte following the 0x00 terminator of their respective position lists.
**
** If isSaveLeft is 0, an entry is added to the output position list for 
** each position in *pp2 for which there exists one or more positions in
** *pp1 so that (pos(*pp2)>pos(*pp1) && pos(*pp2)-pos(*pp1)<=nToken). i.e.
** when the *pp1 token appears before the *pp2 token, but not more than nToken
** slots before it.
**
** e.g. nToken==1 searches for adjacent positions.
*/
static int fts3PoslistPhraseMerge(
  char **pp,                      /* IN/OUT: Preallocated output buffer */
  int nToken,                     /* Maximum difference in token positions */
  int isSaveLeft,                 /* Save the left position */
  int isExact,                    /* If *pp1 is exactly nTokens before *pp2 */
  char **pp1,                     /* IN/OUT: Left input list */
  char **pp2                      /* IN/OUT: Right input list */
){
  char *p = *pp;
  char *p1 = *pp1;
  char *p2 = *pp2;
  int iCol1 = 0;
  int iCol2 = 0;

  /* Never set both isSaveLeft and isExact for the same invocation. */
  assert( isSaveLeft==0 || isExact==0 );

  assert( p!=0 && *p1!=0 && *p2!=0 );
  if( *p1==POS_COLUMN ){ 
    p1++;
    p1 += fts3GetVarint32(p1, &iCol1);
  }
  if( *p2==POS_COLUMN ){ 
    p2++;
    p2 += fts3GetVarint32(p2, &iCol2);
  }

  while( 1 ){
    if( iCol1==iCol2 ){
      char *pSave = p;
      sqlite3_int64 iPrev = 0;
      sqlite3_int64 iPos1 = 0;
      sqlite3_int64 iPos2 = 0;

      if( iCol1 ){
        *p++ = POS_COLUMN;
        p += sqlite3Fts3PutVarint(p, iCol1);
      }

      assert( *p1!=POS_END && *p1!=POS_COLUMN );
      assert( *p2!=POS_END && *p2!=POS_COLUMN );
      fts3GetDeltaVarint(&p1, &iPos1); iPos1 -= 2;
      fts3GetDeltaVarint(&p2, &iPos2); iPos2 -= 2;

      while( 1 ){
        if( iPos2==iPos1+nToken 
         || (isExact==0 && iPos2>iPos1 && iPos2<=iPos1+nToken) 
        ){
          sqlite3_int64 iSave;
          iSave = isSaveLeft ? iPos1 : iPos2;
          fts3PutDeltaVarint(&p, &iPrev, iSave+2); iPrev -= 2;
          pSave = 0;
          assert( p );
        }
        if( (!isSaveLeft && iPos2<=(iPos1+nToken)) || iPos2<=iPos1 ){
          if( (*p2&0xFE)==0 ) break;
          fts3GetDeltaVarint(&p2, &iPos2); iPos2 -= 2;
        }else{
          if( (*p1&0xFE)==0 ) break;
          fts3GetDeltaVarint(&p1, &iPos1); iPos1 -= 2;
        }
      }

      if( pSave ){
        assert( pp && p );
        p = pSave;
      }

      fts3ColumnlistCopy(0, &p1);
      fts3ColumnlistCopy(0, &p2);
      assert( (*p1&0xFE)==0 && (*p2&0xFE)==0 );
      if( 0==*p1 || 0==*p2 ) break;

      p1++;
      p1 += fts3GetVarint32(p1, &iCol1);
      p2++;
      p2 += fts3GetVarint32(p2, &iCol2);
    }

    /* Advance pointer p1 or p2 (whichever corresponds to the smaller of
    ** iCol1 and iCol2) so that it points to either the 0x00 that marks the
    ** end of the position list, or the 0x01 that precedes the next 
    ** column-number in the position list. 
    */
    else if( iCol1<iCol2 ){
      fts3ColumnlistCopy(0, &p1);
      if( 0==*p1 ) break;
      p1++;
      p1 += fts3GetVarint32(p1, &iCol1);
    }else{
      fts3ColumnlistCopy(0, &p2);
      if( 0==*p2 ) break;
      p2++;
      p2 += fts3GetVarint32(p2, &iCol2);
    }
  }

  fts3PoslistCopy(0, &p2);
  fts3PoslistCopy(0, &p1);
  *pp1 = p1;
  *pp2 = p2;
  if( *pp==p ){
    return 0;
  }
  *p++ = 0x00;
  *pp = p;
  return 1;
}

/*
** Merge two position-lists as required by the NEAR operator. The argument
** position lists correspond to the left and right phrases of an expression 
** like:
**
**     "phrase 1" NEAR "phrase number 2"
**
** Position list *pp1 corresponds to the left-hand side of the NEAR 
** expression and *pp2 to the right. As usual, the indexes in the position 
** lists are the offsets of the last token in each phrase (tokens "1" and "2" 
** in the example above).
**
** The output position list - written to *pp - is a copy of *pp2 with those
** entries that are not sufficiently NEAR entries in *pp1 removed.
*/
static int fts3PoslistNearMerge(
  char **pp,                      /* Output buffer */
  char *aTmp,                     /* Temporary buffer space */
  int nRight,                     /* Maximum difference in token positions */
  int nLeft,                      /* Maximum difference in token positions */
  char **pp1,                     /* IN/OUT: Left input list */
  char **pp2                      /* IN/OUT: Right input list */
){
  char *p1 = *pp1;
  char *p2 = *pp2;

  char *pTmp1 = aTmp;
  char *pTmp2;
  char *aTmp2;
  int res = 1;

  fts3PoslistPhraseMerge(&pTmp1, nRight, 0, 0, pp1, pp2);
  aTmp2 = pTmp2 = pTmp1;
  *pp1 = p1;
  *pp2 = p2;
  fts3PoslistPhraseMerge(&pTmp2, nLeft, 1, 0, pp2, pp1);
  if( pTmp1!=aTmp && pTmp2!=aTmp2 ){
    fts3PoslistMerge(pp, &aTmp, &aTmp2);
  }else if( pTmp1!=aTmp ){
    fts3PoslistCopy(pp, &aTmp);
  }else if( pTmp2!=aTmp2 ){
    fts3PoslistCopy(pp, &aTmp2);
  }else{
    res = 0;
  }

  return res;
}

/* 
** An instance of this function is used to merge together the (potentially
** large number of) doclists for each term that matches a prefix query.
** See function fts3TermSelectMerge() for details.
*/
typedef struct TermSelect TermSelect;
struct TermSelect {
  char *aaOutput[16];             /* Malloc'd output buffers */
  int anOutput[16];               /* Size each output buffer in bytes */
};

/*
** This function is used to read a single varint from a buffer. Parameter
** pEnd points 1 byte past the end of the buffer. When this function is
** called, if *pp points to pEnd or greater, then the end of the buffer
** has been reached. In this case *pp is set to 0 and the function returns.
**
** If *pp does not point to or past pEnd, then a single varint is read
** from *pp. *pp is then set to point 1 byte past the end of the read varint.
**
** If bDescIdx is false, the value read is added to *pVal before returning.
** If it is true, the value read is subtracted from *pVal before this 
** function returns.
*/
static void fts3GetDeltaVarint3(
  char **pp,                      /* IN/OUT: Point to read varint from */
  char *pEnd,                     /* End of buffer */
  int bDescIdx,                   /* True if docids are descending */
  sqlite3_int64 *pVal             /* IN/OUT: Integer value */
){
  if( *pp>=pEnd ){
    *pp = 0;
  }else{
    sqlite3_int64 iVal;
    *pp += sqlite3Fts3GetVarint(*pp, &iVal);
    if( bDescIdx ){
      *pVal -= iVal;
    }else{
      *pVal += iVal;
    }
  }
}

/*
** This function is used to write a single varint to a buffer. The varint
** is written to *pp. Before returning, *pp is set to point 1 byte past the
** end of the value written.
**
** If *pbFirst is zero when this function is called, the value written to
** the buffer is that of parameter iVal. 
**
** If *pbFirst is non-zero when this function is called, then the value 
** written is either (iVal-*piPrev) (if bDescIdx is zero) or (*piPrev-iVal)
** (if bDescIdx is non-zero).
**
** Before returning, this function always sets *pbFirst to 1 and *piPrev
** to the value of parameter iVal.
*/
static void fts3PutDeltaVarint3(
  char **pp,                      /* IN/OUT: Output pointer */
  int bDescIdx,                   /* True for descending docids */
  sqlite3_int64 *piPrev,          /* IN/OUT: Previous value written to list */
  int *pbFirst,                   /* IN/OUT: True after first int written */
  sqlite3_int64 iVal              /* Write this value to the list */
){
  sqlite3_int64 iWrite;
  if( bDescIdx==0 || *pbFirst==0 ){
    iWrite = iVal - *piPrev;
  }else{
    iWrite = *piPrev - iVal;
  }
  assert( *pbFirst || *piPrev==0 );
  assert( *pbFirst==0 || iWrite>0 );
  *pp += sqlite3Fts3PutVarint(*pp, iWrite);
  *piPrev = iVal;
  *pbFirst = 1;
}


/*
** This macro is used by various functions that merge doclists. The two
** arguments are 64-bit docid values. If the value of the stack variable
** bDescDoclist is 0 when this macro is invoked, then it returns (i1-i2). 
** Otherwise, (i2-i1).
**
** Using this makes it easier to write code that can merge doclists that are
** sorted in either ascending or descending order.
*/
#define DOCID_CMP(i1, i2) ((bDescDoclist?-1:1) * (i1-i2))

/*
** This function does an "OR" merge of two doclists (output contains all
** positions contained in either argument doclist). If the docids in the 
** input doclists are sorted in ascending order, parameter bDescDoclist
** should be false. If they are sorted in ascending order, it should be
** passed a non-zero value.
**
** If no error occurs, *paOut is set to point at an sqlite3_malloc'd buffer
** containing the output doclist and SQLITE_OK is returned. In this case
** *pnOut is set to the number of bytes in the output doclist.
**
** If an error occurs, an SQLite error code is returned. The output values
** are undefined in this case.
*/
static int fts3DoclistOrMerge(
  int bDescDoclist,               /* True if arguments are desc */
  char *a1, int n1,               /* First doclist */
  char *a2, int n2,               /* Second doclist */
  char **paOut, int *pnOut        /* OUT: Malloc'd doclist */
){
  sqlite3_int64 i1 = 0;
  sqlite3_int64 i2 = 0;
  sqlite3_int64 iPrev = 0;
  char *pEnd1 = &a1[n1];
  char *pEnd2 = &a2[n2];
  char *p1 = a1;
  char *p2 = a2;
  char *p;
  char *aOut;
  int bFirstOut = 0;

  *paOut = 0;
  *pnOut = 0;

  /* Allocate space for the output. Both the input and output doclists
  ** are delta encoded. If they are in ascending order (bDescDoclist==0),
  ** then the first docid in each list is simply encoded as a varint. For
  ** each subsequent docid, the varint stored is the difference between the
  ** current and previous docid (a positive number - since the list is in
  ** ascending order).
  **
  ** The first docid written to the output is therefore encoded using the 
  ** same number of bytes as it is in whichever of the input lists it is
  ** read from. And each subsequent docid read from the same input list 
  ** consumes either the same or less bytes as it did in the input (since
  ** the difference between it and the previous value in the output must
  ** be a positive value less than or equal to the delta value read from 
  ** the input list). The same argument applies to all but the first docid
  ** read from the 'other' list. And to the contents of all position lists
  ** that will be copied and merged from the input to the output.
  **
  ** However, if the first docid copied to the output is a negative number,
  ** then the encoding of the first docid from the 'other' input list may
  ** be larger in the output than it was in the input (since the delta value
  ** may be a larger positive integer than the actual docid).
  **
  ** The space required to store the output is therefore the sum of the
  ** sizes of the two inputs, plus enough space for exactly one of the input
  ** docids to grow. 
  **
  ** A symetric argument may be made if the doclists are in descending 
  ** order.
  */
  aOut = sqlite3_malloc(n1+n2+FTS3_VARINT_MAX-1);
  if( !aOut ) return SQLITE_NOMEM;

  p = aOut;
  fts3GetDeltaVarint3(&p1, pEnd1, 0, &i1);
  fts3GetDeltaVarint3(&p2, pEnd2, 0, &i2);
  while( p1 || p2 ){
    sqlite3_int64 iDiff = DOCID_CMP(i1, i2);

    if( p2 && p1 && iDiff==0 ){
      fts3PutDeltaVarint3(&p, bDescDoclist, &iPrev, &bFirstOut, i1);
      fts3PoslistMerge(&p, &p1, &p2);
      fts3GetDeltaVarint3(&p1, pEnd1, bDescDoclist, &i1);
      fts3GetDeltaVarint3(&p2, pEnd2, bDescDoclist, &i2);
    }else if( !p2 || (p1 && iDiff<0) ){
      fts3PutDeltaVarint3(&p, bDescDoclist, &iPrev, &bFirstOut, i1);
      fts3PoslistCopy(&p, &p1);
      fts3GetDeltaVarint3(&p1, pEnd1, bDescDoclist, &i1);
    }else{
      fts3PutDeltaVarint3(&p, bDescDoclist, &iPrev, &bFirstOut, i2);
      fts3PoslistCopy(&p, &p2);
      fts3GetDeltaVarint3(&p2, pEnd2, bDescDoclist, &i2);
    }
  }

  *paOut = aOut;
  *pnOut = (int)(p-aOut);
  assert( *pnOut<=n1+n2+FTS3_VARINT_MAX-1 );
  return SQLITE_OK;
}

/*
** This function does a "phrase" merge of two doclists. In a phrase merge,
** the output contains a copy of each position from the right-hand input
** doclist for which there is a position in the left-hand input doclist
** exactly nDist tokens before it.
**
** If the docids in the input doclists are sorted in ascending order,
** parameter bDescDoclist should be false. If they are sorted in ascending 
** order, it should be passed a non-zero value.
**
** The right-hand input doclist is overwritten by this function.
*/
static int fts3DoclistPhraseMerge(
  int bDescDoclist,               /* True if arguments are desc */
  int nDist,                      /* Distance from left to right (1=adjacent) */
  char *aLeft, int nLeft,         /* Left doclist */
  char **paRight, int *pnRight    /* IN/OUT: Right/output doclist */
){
  sqlite3_int64 i1 = 0;
  sqlite3_int64 i2 = 0;
  sqlite3_int64 iPrev = 0;
  char *aRight = *paRight;
  char *pEnd1 = &aLeft[nLeft];
  char *pEnd2 = &aRight[*pnRight];
  char *p1 = aLeft;
  char *p2 = aRight;
  char *p;
  int bFirstOut = 0;
  char *aOut;

  assert( nDist>0 );
  if( bDescDoclist ){
    aOut = sqlite3_malloc(*pnRight + FTS3_VARINT_MAX);
    if( aOut==0 ) return SQLITE_NOMEM;
  }else{
    aOut = aRight;
  }
  p = aOut;

  fts3GetDeltaVarint3(&p1, pEnd1, 0, &i1);
  fts3GetDeltaVarint3(&p2, pEnd2, 0, &i2);

  while( p1 && p2 ){
    sqlite3_int64 iDiff = DOCID_CMP(i1, i2);
    if( iDiff==0 ){
      char *pSave = p;
      sqlite3_int64 iPrevSave = iPrev;
      int bFirstOutSave = bFirstOut;

      fts3PutDeltaVarint3(&p, bDescDoclist, &iPrev, &bFirstOut, i1);
      if( 0==fts3PoslistPhraseMerge(&p, nDist, 0, 1, &p1, &p2) ){
        p = pSave;
        iPrev = iPrevSave;
        bFirstOut = bFirstOutSave;
      }
      fts3GetDeltaVarint3(&p1, pEnd1, bDescDoclist, &i1);
      fts3GetDeltaVarint3(&p2, pEnd2, bDescDoclist, &i2);
    }else if( iDiff<0 ){
      fts3PoslistCopy(0, &p1);
      fts3GetDeltaVarint3(&p1, pEnd1, bDescDoclist, &i1);
    }else{
      fts3PoslistCopy(0, &p2);
      fts3GetDeltaVarint3(&p2, pEnd2, bDescDoclist, &i2);
    }
  }

  *pnRight = (int)(p - aOut);
  if( bDescDoclist ){
    sqlite3_free(aRight);
    *paRight = aOut;
  }

  return SQLITE_OK;
}

/*
** Argument pList points to a position list nList bytes in size. This
** function checks to see if the position list contains any entries for
** a token in position 0 (of any column). If so, it writes argument iDelta
** to the output buffer pOut, followed by a position list consisting only
** of the entries from pList at position 0, and terminated by an 0x00 byte.
** The value returned is the number of bytes written to pOut (if any).
*/
SQLITE_PRIVATE int sqlite3Fts3FirstFilter(
  sqlite3_int64 iDelta,           /* Varint that may be written to pOut */
  char *pList,                    /* Position list (no 0x00 term) */
  int nList,                      /* Size of pList in bytes */
  char *pOut                      /* Write output here */
){
  int nOut = 0;
  int bWritten = 0;               /* True once iDelta has been written */
  char *p = pList;
  char *pEnd = &pList[nList];

  if( *p!=0x01 ){
    if( *p==0x02 ){
      nOut += sqlite3Fts3PutVarint(&pOut[nOut], iDelta);
      pOut[nOut++] = 0x02;
      bWritten = 1;
    }
    fts3ColumnlistCopy(0, &p);
  }

  while( p<pEnd && *p==0x01 ){
    sqlite3_int64 iCol;
    p++;
    p += sqlite3Fts3GetVarint(p, &iCol);
    if( *p==0x02 ){
      if( bWritten==0 ){
        nOut += sqlite3Fts3PutVarint(&pOut[nOut], iDelta);
        bWritten = 1;
      }
      pOut[nOut++] = 0x01;
      nOut += sqlite3Fts3PutVarint(&pOut[nOut], iCol);
      pOut[nOut++] = 0x02;
    }
    fts3ColumnlistCopy(0, &p);
  }
  if( bWritten ){
    pOut[nOut++] = 0x00;
  }

  return nOut;
}


/*
** Merge all doclists in the TermSelect.aaOutput[] array into a single
** doclist stored in TermSelect.aaOutput[0]. If successful, delete all
** other doclists (except the aaOutput[0] one) and return SQLITE_OK.
**
** If an OOM error occurs, return SQLITE_NOMEM. In this case it is
** the responsibility of the caller to free any doclists left in the
** TermSelect.aaOutput[] array.
*/
static int fts3TermSelectFinishMerge(Fts3Table *p, TermSelect *pTS){
  char *aOut = 0;
  int nOut = 0;
  int i;

  /* Loop through the doclists in the aaOutput[] array. Merge them all
  ** into a single doclist.
  */
  for(i=0; i<SizeofArray(pTS->aaOutput); i++){
    if( pTS->aaOutput[i] ){
      if( !aOut ){
        aOut = pTS->aaOutput[i];
        nOut = pTS->anOutput[i];
        pTS->aaOutput[i] = 0;
      }else{
        int nNew;
        char *aNew;

        int rc = fts3DoclistOrMerge(p->bDescIdx, 
            pTS->aaOutput[i], pTS->anOutput[i], aOut, nOut, &aNew, &nNew
        );
        if( rc!=SQLITE_OK ){
          sqlite3_free(aOut);
          return rc;
        }

        sqlite3_free(pTS->aaOutput[i]);
        sqlite3_free(aOut);
        pTS->aaOutput[i] = 0;
        aOut = aNew;
        nOut = nNew;
      }
    }
  }

  pTS->aaOutput[0] = aOut;
  pTS->anOutput[0] = nOut;
  return SQLITE_OK;
}

/*
** Merge the doclist aDoclist/nDoclist into the TermSelect object passed
** as the first argument. The merge is an "OR" merge (see function
** fts3DoclistOrMerge() for details).
**
** This function is called with the doclist for each term that matches
** a queried prefix. It merges all these doclists into one, the doclist
** for the specified prefix. Since there can be a very large number of
** doclists to merge, the merging is done pair-wise using the TermSelect
** object.
**
** This function returns SQLITE_OK if the merge is successful, or an
** SQLite error code (SQLITE_NOMEM) if an error occurs.
*/
static int fts3TermSelectMerge(
  Fts3Table *p,                   /* FTS table handle */
  TermSelect *pTS,                /* TermSelect object to merge into */
  char *aDoclist,                 /* Pointer to doclist */
  int nDoclist                    /* Size of aDoclist in bytes */
){
  if( pTS->aaOutput[0]==0 ){
    /* If this is the first term selected, copy the doclist to the output
    ** buffer using memcpy(). 
    **
    ** Add FTS3_VARINT_MAX bytes of unused space to the end of the 
    ** allocation. This is so as to ensure that the buffer is big enough
    ** to hold the current doclist AND'd with any other doclist. If the
    ** doclists are stored in order=ASC order, this padding would not be
    ** required (since the size of [doclistA AND doclistB] is always less
    ** than or equal to the size of [doclistA] in that case). But this is
    ** not true for order=DESC. For example, a doclist containing (1, -1) 
    ** may be smaller than (-1), as in the first example the -1 may be stored
    ** as a single-byte delta, whereas in the second it must be stored as a
    ** FTS3_VARINT_MAX byte varint.
    **
    ** Similar padding is added in the fts3DoclistOrMerge() function.
    */
    pTS->aaOutput[0] = sqlite3_malloc(nDoclist + FTS3_VARINT_MAX + 1);
    pTS->anOutput[0] = nDoclist;
    if( pTS->aaOutput[0] ){
      memcpy(pTS->aaOutput[0], aDoclist, nDoclist);
    }else{
      return SQLITE_NOMEM;
    }
  }else{
    char *aMerge = aDoclist;
    int nMerge = nDoclist;
    int iOut;

    for(iOut=0; iOut<SizeofArray(pTS->aaOutput); iOut++){
      if( pTS->aaOutput[iOut]==0 ){
        assert( iOut>0 );
        pTS->aaOutput[iOut] = aMerge;
        pTS->anOutput[iOut] = nMerge;
        break;
      }else{
        char *aNew;
        int nNew;

        int rc = fts3DoclistOrMerge(p->bDescIdx, aMerge, nMerge, 
            pTS->aaOutput[iOut], pTS->anOutput[iOut], &aNew, &nNew
        );
        if( rc!=SQLITE_OK ){
          if( aMerge!=aDoclist ) sqlite3_free(aMerge);
          return rc;
        }

        if( aMerge!=aDoclist ) sqlite3_free(aMerge);
        sqlite3_free(pTS->aaOutput[iOut]);
        pTS->aaOutput[iOut] = 0;
  
        aMerge = aNew;
        nMerge = nNew;
        if( (iOut+1)==SizeofArray(pTS->aaOutput) ){
          pTS->aaOutput[iOut] = aMerge;
          pTS->anOutput[iOut] = nMerge;
        }
      }
    }
  }
  return SQLITE_OK;
}

/*
** Append SegReader object pNew to the end of the pCsr->apSegment[] array.
*/
static int fts3SegReaderCursorAppend(
  Fts3MultiSegReader *pCsr, 
  Fts3SegReader *pNew
){
  if( (pCsr->nSegment%16)==0 ){
    Fts3SegReader **apNew;
    int nByte = (pCsr->nSegment + 16)*sizeof(Fts3SegReader*);
    apNew = (Fts3SegReader **)sqlite3_realloc(pCsr->apSegment, nByte);
    if( !apNew ){
      sqlite3Fts3SegReaderFree(pNew);
      return SQLITE_NOMEM;
    }
    pCsr->apSegment = apNew;
  }
  pCsr->apSegment[pCsr->nSegment++] = pNew;
  return SQLITE_OK;
}

/*
** Add seg-reader objects to the Fts3MultiSegReader object passed as the
** 8th argument.
**
** This function returns SQLITE_OK if successful, or an SQLite error code
** otherwise.
*/
static int fts3SegReaderCursor(
  Fts3Table *p,                   /* FTS3 table handle */
  int iLangid,                    /* Language id */
  int iIndex,                     /* Index to search (from 0 to p->nIndex-1) */
  int iLevel,                     /* Level of segments to scan */
  const char *zTerm,              /* Term to query for */
  int nTerm,                      /* Size of zTerm in bytes */
  int isPrefix,                   /* True for a prefix search */
  int isScan,                     /* True to scan from zTerm to EOF */
  Fts3MultiSegReader *pCsr        /* Cursor object to populate */
){
  int rc = SQLITE_OK;             /* Error code */
  sqlite3_stmt *pStmt = 0;        /* Statement to iterate through segments */
  int rc2;                        /* Result of sqlite3_reset() */

  /* If iLevel is less than 0 and this is not a scan, include a seg-reader 
  ** for the pending-terms. If this is a scan, then this call must be being
  ** made by an fts4aux module, not an FTS table. In this case calling
  ** Fts3SegReaderPending might segfault, as the data structures used by 
  ** fts4aux are not completely populated. So it's easiest to filter these
  ** calls out here.  */
  if( iLevel<0 && p->aIndex ){
    Fts3SegReader *pSeg = 0;
    rc = sqlite3Fts3SegReaderPending(p, iIndex, zTerm, nTerm, isPrefix||isScan, &pSeg);
    if( rc==SQLITE_OK && pSeg ){
      rc = fts3SegReaderCursorAppend(pCsr, pSeg);
    }
  }

  if( iLevel!=FTS3_SEGCURSOR_PENDING ){
    if( rc==SQLITE_OK ){
      rc = sqlite3Fts3AllSegdirs(p, iLangid, iIndex, iLevel, &pStmt);
    }

    while( rc==SQLITE_OK && SQLITE_ROW==(rc = sqlite3_step(pStmt)) ){
      Fts3SegReader *pSeg = 0;

      /* Read the values returned by the SELECT into local variables. */
      sqlite3_int64 iStartBlock = sqlite3_column_int64(pStmt, 1);
      sqlite3_int64 iLeavesEndBlock = sqlite3_column_int64(pStmt, 2);
      sqlite3_int64 iEndBlock = sqlite3_column_int64(pStmt, 3);
      int nRoot = sqlite3_column_bytes(pStmt, 4);
      char const *zRoot = sqlite3_column_blob(pStmt, 4);

      /* If zTerm is not NULL, and this segment is not stored entirely on its
      ** root node, the range of leaves scanned can be reduced. Do this. */
      if( iStartBlock && zTerm ){
        sqlite3_int64 *pi = (isPrefix ? &iLeavesEndBlock : 0);
        rc = fts3SelectLeaf(p, zTerm, nTerm, zRoot, nRoot, &iStartBlock, pi);
        if( rc!=SQLITE_OK ) goto finished;
        if( isPrefix==0 && isScan==0 ) iLeavesEndBlock = iStartBlock;
      }
 
      rc = sqlite3Fts3SegReaderNew(pCsr->nSegment+1, 
          (isPrefix==0 && isScan==0),
          iStartBlock, iLeavesEndBlock, 
          iEndBlock, zRoot, nRoot, &pSeg
      );
      if( rc!=SQLITE_OK ) goto finished;
      rc = fts3SegReaderCursorAppend(pCsr, pSeg);
    }
  }

 finished:
  rc2 = sqlite3_reset(pStmt);
  if( rc==SQLITE_DONE ) rc = rc2;

  return rc;
}

/*
** Set up a cursor object for iterating through a full-text index or a 
** single level therein.
*/
SQLITE_PRIVATE int sqlite3Fts3SegReaderCursor(
  Fts3Table *p,                   /* FTS3 table handle */
  int iLangid,                    /* Language-id to search */
  int iIndex,                     /* Index to search (from 0 to p->nIndex-1) */
  int iLevel,                     /* Level of segments to scan */
  const char *zTerm,              /* Term to query for */
  int nTerm,                      /* Size of zTerm in bytes */
  int isPrefix,                   /* True for a prefix search */
  int isScan,                     /* True to scan from zTerm to EOF */
  Fts3MultiSegReader *pCsr       /* Cursor object to populate */
){
  assert( iIndex>=0 && iIndex<p->nIndex );
  assert( iLevel==FTS3_SEGCURSOR_ALL
      ||  iLevel==FTS3_SEGCURSOR_PENDING 
      ||  iLevel>=0
  );
  assert( iLevel<FTS3_SEGDIR_MAXLEVEL );
  assert( FTS3_SEGCURSOR_ALL<0 && FTS3_SEGCURSOR_PENDING<0 );
  assert( isPrefix==0 || isScan==0 );

  memset(pCsr, 0, sizeof(Fts3MultiSegReader));
  return fts3SegReaderCursor(
      p, iLangid, iIndex, iLevel, zTerm, nTerm, isPrefix, isScan, pCsr
  );
}

/*
** In addition to its current configuration, have the Fts3MultiSegReader
** passed as the 4th argument also scan the doclist for term zTerm/nTerm.
**
** SQLITE_OK is returned if no error occurs, otherwise an SQLite error code.
*/
static int fts3SegReaderCursorAddZero(
  Fts3Table *p,                   /* FTS virtual table handle */
  int iLangid,
  const char *zTerm,              /* Term to scan doclist of */
  int nTerm,                      /* Number of bytes in zTerm */
  Fts3MultiSegReader *pCsr        /* Fts3MultiSegReader to modify */
){
  return fts3SegReaderCursor(p, 
      iLangid, 0, FTS3_SEGCURSOR_ALL, zTerm, nTerm, 0, 0,pCsr
  );
}

/*
** Open an Fts3MultiSegReader to scan the doclist for term zTerm/nTerm. Or,
** if isPrefix is true, to scan the doclist for all terms for which 
** zTerm/nTerm is a prefix. If successful, return SQLITE_OK and write
** a pointer to the new Fts3MultiSegReader to *ppSegcsr. Otherwise, return
** an SQLite error code.
**
** It is the responsibility of the caller to free this object by eventually
** passing it to fts3SegReaderCursorFree() 
**
** SQLITE_OK is returned if no error occurs, otherwise an SQLite error code.
** Output parameter *ppSegcsr is set to 0 if an error occurs.
*/
static int fts3TermSegReaderCursor(
  Fts3Cursor *pCsr,               /* Virtual table cursor handle */
  const char *zTerm,              /* Term to query for */
  int nTerm,                      /* Size of zTerm in bytes */
  int isPrefix,                   /* True for a prefix search */
  Fts3MultiSegReader **ppSegcsr   /* OUT: Allocated seg-reader cursor */
){
  Fts3MultiSegReader *pSegcsr;    /* Object to allocate and return */
  int rc = SQLITE_NOMEM;          /* Return code */

  pSegcsr = sqlite3_malloc(sizeof(Fts3MultiSegReader));
  if( pSegcsr ){
    int i;
    int bFound = 0;               /* True once an index has been found */
    Fts3Table *p = (Fts3Table *)pCsr->base.pVtab;

    if( isPrefix ){
      for(i=1; bFound==0 && i<p->nIndex; i++){
        if( p->aIndex[i].nPrefix==nTerm ){
          bFound = 1;
          rc = sqlite3Fts3SegReaderCursor(p, pCsr->iLangid, 
              i, FTS3_SEGCURSOR_ALL, zTerm, nTerm, 0, 0, pSegcsr
          );
          pSegcsr->bLookup = 1;
        }
      }

      for(i=1; bFound==0 && i<p->nIndex; i++){
        if( p->aIndex[i].nPrefix==nTerm+1 ){
          bFound = 1;
          rc = sqlite3Fts3SegReaderCursor(p, pCsr->iLangid, 
              i, FTS3_SEGCURSOR_ALL, zTerm, nTerm, 1, 0, pSegcsr
          );
          if( rc==SQLITE_OK ){
            rc = fts3SegReaderCursorAddZero(
                p, pCsr->iLangid, zTerm, nTerm, pSegcsr
            );
          }
        }
      }
    }

    if( bFound==0 ){
      rc = sqlite3Fts3SegReaderCursor(p, pCsr->iLangid, 
          0, FTS3_SEGCURSOR_ALL, zTerm, nTerm, isPrefix, 0, pSegcsr
      );
      pSegcsr->bLookup = !isPrefix;
    }
  }

  *ppSegcsr = pSegcsr;
  return rc;
}

/*
** Free an Fts3MultiSegReader allocated by fts3TermSegReaderCursor().
*/
static void fts3SegReaderCursorFree(Fts3MultiSegReader *pSegcsr){
  sqlite3Fts3SegReaderFinish(pSegcsr);
  sqlite3_free(pSegcsr);
}

/*
** This function retrieves the doclist for the specified term (or term
** prefix) from the database.
*/
static int fts3TermSelect(
  Fts3Table *p,                   /* Virtual table handle */
  Fts3PhraseToken *pTok,          /* Token to query for */
  int iColumn,                    /* Column to query (or -ve for all columns) */
  int *pnOut,                     /* OUT: Size of buffer at *ppOut */
  char **ppOut                    /* OUT: Malloced result buffer */
){
  int rc;                         /* Return code */
  Fts3MultiSegReader *pSegcsr;    /* Seg-reader cursor for this term */
  TermSelect tsc;                 /* Object for pair-wise doclist merging */
  Fts3SegFilter filter;           /* Segment term filter configuration */

  pSegcsr = pTok->pSegcsr;
  memset(&tsc, 0, sizeof(TermSelect));

  filter.flags = FTS3_SEGMENT_IGNORE_EMPTY | FTS3_SEGMENT_REQUIRE_POS
        | (pTok->isPrefix ? FTS3_SEGMENT_PREFIX : 0)
        | (pTok->bFirst ? FTS3_SEGMENT_FIRST : 0)
        | (iColumn<p->nColumn ? FTS3_SEGMENT_COLUMN_FILTER : 0);
  filter.iCol = iColumn;
  filter.zTerm = pTok->z;
  filter.nTerm = pTok->n;

  rc = sqlite3Fts3SegReaderStart(p, pSegcsr, &filter);
  while( SQLITE_OK==rc
      && SQLITE_ROW==(rc = sqlite3Fts3SegReaderStep(p, pSegcsr)) 
  ){
    rc = fts3TermSelectMerge(p, &tsc, pSegcsr->aDoclist, pSegcsr->nDoclist);
  }

  if( rc==SQLITE_OK ){
    rc = fts3TermSelectFinishMerge(p, &tsc);
  }
  if( rc==SQLITE_OK ){
    *ppOut = tsc.aaOutput[0];
    *pnOut = tsc.anOutput[0];
  }else{
    int i;
    for(i=0; i<SizeofArray(tsc.aaOutput); i++){
      sqlite3_free(tsc.aaOutput[i]);
    }
  }

  fts3SegReaderCursorFree(pSegcsr);
  pTok->pSegcsr = 0;
  return rc;
}

/*
** This function counts the total number of docids in the doclist stored
** in buffer aList[], size nList bytes.
**
** If the isPoslist argument is true, then it is assumed that the doclist
** contains a position-list following each docid. Otherwise, it is assumed
** that the doclist is simply a list of docids stored as delta encoded 
** varints.
*/
static int fts3DoclistCountDocids(char *aList, int nList){
  int nDoc = 0;                   /* Return value */
  if( aList ){
    char *aEnd = &aList[nList];   /* Pointer to one byte after EOF */
    char *p = aList;              /* Cursor */
    while( p<aEnd ){
      nDoc++;
      while( (*p++)&0x80 );     /* Skip docid varint */
      fts3PoslistCopy(0, &p);   /* Skip over position list */
    }
  }

  return nDoc;
}

/*
** Advance the cursor to the next row in the %_content table that
** matches the search criteria.  For a MATCH search, this will be
** the next row that matches. For a full-table scan, this will be
** simply the next row in the %_content table.  For a docid lookup,
** this routine simply sets the EOF flag.
**
** Return SQLITE_OK if nothing goes wrong.  SQLITE_OK is returned
** even if we reach end-of-file.  The fts3EofMethod() will be called
** subsequently to determine whether or not an EOF was hit.
*/
static int fts3NextMethod(sqlite3_vtab_cursor *pCursor){
  int rc;
  Fts3Cursor *pCsr = (Fts3Cursor *)pCursor;
  if( pCsr->eSearch==FTS3_DOCID_SEARCH || pCsr->eSearch==FTS3_FULLSCAN_SEARCH ){
    if( SQLITE_ROW!=sqlite3_step(pCsr->pStmt) ){
      pCsr->isEof = 1;
      rc = sqlite3_reset(pCsr->pStmt);
    }else{
      pCsr->iPrevId = sqlite3_column_int64(pCsr->pStmt, 0);
      rc = SQLITE_OK;
    }
  }else{
    rc = fts3EvalNext((Fts3Cursor *)pCursor);
  }
  assert( ((Fts3Table *)pCsr->base.pVtab)->pSegments==0 );
  return rc;
}

/*
** The following are copied from sqliteInt.h.
**
** Constants for the largest and smallest possible 64-bit signed integers.
** These macros are designed to work correctly on both 32-bit and 64-bit
** compilers.
*/
#ifndef SQLITE_AMALGAMATION
# define LARGEST_INT64  (0xffffffff|(((sqlite3_int64)0x7fffffff)<<32))
# define SMALLEST_INT64 (((sqlite3_int64)-1) - LARGEST_INT64)
#endif

/*
** If the numeric type of argument pVal is "integer", then return it
** converted to a 64-bit signed integer. Otherwise, return a copy of
** the second parameter, iDefault.
*/
static sqlite3_int64 fts3DocidRange(sqlite3_value *pVal, i64 iDefault){
  if( pVal ){
    int eType = sqlite3_value_numeric_type(pVal);
    if( eType==SQLITE_INTEGER ){
      return sqlite3_value_int64(pVal);
    }
  }
  return iDefault;
}

/*
** This is the xFilter interface for the virtual table.  See
** the virtual table xFilter method documentation for additional
** information.
**
** If idxNum==FTS3_FULLSCAN_SEARCH then do a full table scan against
** the %_content table.
**
** If idxNum==FTS3_DOCID_SEARCH then do a docid lookup for a single entry
** in the %_content table.
**
** If idxNum>=FTS3_FULLTEXT_SEARCH then use the full text index.  The
** column on the left-hand side of the MATCH operator is column
** number idxNum-FTS3_FULLTEXT_SEARCH, 0 indexed.  argv[0] is the right-hand
** side of the MATCH operator.
*/
static int fts3FilterMethod(
  sqlite3_vtab_cursor *pCursor,   /* The cursor used for this query */
  int idxNum,                     /* Strategy index */
  const char *idxStr,             /* Unused */
  int nVal,                       /* Number of elements in apVal */
  sqlite3_value **apVal           /* Arguments for the indexing scheme */
){
  int rc = SQLITE_OK;
  char *zSql;                     /* SQL statement used to access %_content */
  int eSearch;
  Fts3Table *p = (Fts3Table *)pCursor->pVtab;
  Fts3Cursor *pCsr = (Fts3Cursor *)pCursor;

  sqlite3_value *pCons = 0;       /* The MATCH or rowid constraint, if any */
  sqlite3_value *pLangid = 0;     /* The "langid = ?" constraint, if any */
  sqlite3_value *pDocidGe = 0;    /* The "docid >= ?" constraint, if any */
  sqlite3_value *pDocidLe = 0;    /* The "docid <= ?" constraint, if any */
  int iIdx;

  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(nVal);

  eSearch = (idxNum & 0x0000FFFF);
  assert( eSearch>=0 && eSearch<=(FTS3_FULLTEXT_SEARCH+p->nColumn) );
  assert( p->pSegments==0 );

  /* Collect arguments into local variables */
  iIdx = 0;
  if( eSearch!=FTS3_FULLSCAN_SEARCH ) pCons = apVal[iIdx++];
  if( idxNum & FTS3_HAVE_LANGID ) pLangid = apVal[iIdx++];
  if( idxNum & FTS3_HAVE_DOCID_GE ) pDocidGe = apVal[iIdx++];
  if( idxNum & FTS3_HAVE_DOCID_LE ) pDocidLe = apVal[iIdx++];
  assert( iIdx==nVal );

  /* In case the cursor has been used before, clear it now. */
  sqlite3_finalize(pCsr->pStmt);
  sqlite3_free(pCsr->aDoclist);
  sqlite3Fts3MIBufferFree(pCsr->pMIBuffer);
  sqlite3Fts3ExprFree(pCsr->pExpr);
  memset(&pCursor[1], 0, sizeof(Fts3Cursor)-sizeof(sqlite3_vtab_cursor));

  /* Set the lower and upper bounds on docids to return */
  pCsr->iMinDocid = fts3DocidRange(pDocidGe, SMALLEST_INT64);
  pCsr->iMaxDocid = fts3DocidRange(pDocidLe, LARGEST_INT64);

  if( idxStr ){
    pCsr->bDesc = (idxStr[0]=='D');
  }else{
    pCsr->bDesc = p->bDescIdx;
  }
  pCsr->eSearch = (i16)eSearch;

  if( eSearch!=FTS3_DOCID_SEARCH && eSearch!=FTS3_FULLSCAN_SEARCH ){
    int iCol = eSearch-FTS3_FULLTEXT_SEARCH;
    const char *zQuery = (const char *)sqlite3_value_text(pCons);

    if( zQuery==0 && sqlite3_value_type(pCons)!=SQLITE_NULL ){
      return SQLITE_NOMEM;
    }

    pCsr->iLangid = 0;
    if( pLangid ) pCsr->iLangid = sqlite3_value_int(pLangid);

    assert( p->base.zErrMsg==0 );
    rc = sqlite3Fts3ExprParse(p->pTokenizer, pCsr->iLangid,
        p->azColumn, p->bFts4, p->nColumn, iCol, zQuery, -1, &pCsr->pExpr, 
        &p->base.zErrMsg
    );
    if( rc!=SQLITE_OK ){
      return rc;
    }

    rc = fts3EvalStart(pCsr);
    sqlite3Fts3SegmentsClose(p);
    if( rc!=SQLITE_OK ) return rc;
    pCsr->pNextId = pCsr->aDoclist;
    pCsr->iPrevId = 0;
  }

  /* Compile a SELECT statement for this cursor. For a full-table-scan, the
  ** statement loops through all rows of the %_content table. For a
  ** full-text query or docid lookup, the statement retrieves a single
  ** row by docid.
  */
  if( eSearch==FTS3_FULLSCAN_SEARCH ){
    if( pDocidGe || pDocidLe ){
      zSql = sqlite3_mprintf(
          "SELECT %s WHERE rowid BETWEEN %lld AND %lld ORDER BY rowid %s",
          p->zReadExprlist, pCsr->iMinDocid, pCsr->iMaxDocid,
          (pCsr->bDesc ? "DESC" : "ASC")
      );
    }else{
      zSql = sqlite3_mprintf("SELECT %s ORDER BY rowid %s", 
          p->zReadExprlist, (pCsr->bDesc ? "DESC" : "ASC")
      );
    }
    if( zSql ){
      rc = sqlite3_prepare_v2(p->db, zSql, -1, &pCsr->pStmt, 0);
      sqlite3_free(zSql);
    }else{
      rc = SQLITE_NOMEM;
    }
  }else if( eSearch==FTS3_DOCID_SEARCH ){
    rc = fts3CursorSeekStmt(pCsr, &pCsr->pStmt);
    if( rc==SQLITE_OK ){
      rc = sqlite3_bind_value(pCsr->pStmt, 1, pCons);
    }
  }
  if( rc!=SQLITE_OK ) return rc;

  return fts3NextMethod(pCursor);
}

/* 
** This is the xEof method of the virtual table. SQLite calls this 
** routine to find out if it has reached the end of a result set.
*/
static int fts3EofMethod(sqlite3_vtab_cursor *pCursor){
  return ((Fts3Cursor *)pCursor)->isEof;
}

/* 
** This is the xRowid method. The SQLite core calls this routine to
** retrieve the rowid for the current row of the result set. fts3
** exposes %_content.docid as the rowid for the virtual table. The
** rowid should be written to *pRowid.
*/
static int fts3RowidMethod(sqlite3_vtab_cursor *pCursor, sqlite_int64 *pRowid){
  Fts3Cursor *pCsr = (Fts3Cursor *) pCursor;
  *pRowid = pCsr->iPrevId;
  return SQLITE_OK;
}

/* 
** This is the xColumn method, called by SQLite to request a value from
** the row that the supplied cursor currently points to.
**
** If:
**
**   (iCol <  p->nColumn)   -> The value of the iCol'th user column.
**   (iCol == p->nColumn)   -> Magic column with the same name as the table.
**   (iCol == p->nColumn+1) -> Docid column
**   (iCol == p->nColumn+2) -> Langid column
*/
static int fts3ColumnMethod(
  sqlite3_vtab_cursor *pCursor,   /* Cursor to retrieve value from */
  sqlite3_context *pCtx,          /* Context for sqlite3_result_xxx() calls */
  int iCol                        /* Index of column to read value from */
){
  int rc = SQLITE_OK;             /* Return Code */
  Fts3Cursor *pCsr = (Fts3Cursor *) pCursor;
  Fts3Table *p = (Fts3Table *)pCursor->pVtab;

  /* The column value supplied by SQLite must be in range. */
  assert( iCol>=0 && iCol<=p->nColumn+2 );

  if( iCol==p->nColumn+1 ){
    /* This call is a request for the "docid" column. Since "docid" is an 
    ** alias for "rowid", use the xRowid() method to obtain the value.
    */
    sqlite3_result_int64(pCtx, pCsr->iPrevId);
  }else if( iCol==p->nColumn ){
    /* The extra column whose name is the same as the table.
    ** Return a blob which is a pointer to the cursor.  */
    sqlite3_result_blob(pCtx, &pCsr, sizeof(pCsr), SQLITE_TRANSIENT);
  }else if( iCol==p->nColumn+2 && pCsr->pExpr ){
    sqlite3_result_int64(pCtx, pCsr->iLangid);
  }else{
    /* The requested column is either a user column (one that contains 
    ** indexed data), or the language-id column.  */
    rc = fts3CursorSeek(0, pCsr);

    if( rc==SQLITE_OK ){
      if( iCol==p->nColumn+2 ){
        int iLangid = 0;
        if( p->zLanguageid ){
          iLangid = sqlite3_column_int(pCsr->pStmt, p->nColumn+1);
        }
        sqlite3_result_int(pCtx, iLangid);
      }else if( sqlite3_data_count(pCsr->pStmt)>(iCol+1) ){
        sqlite3_result_value(pCtx, sqlite3_column_value(pCsr->pStmt, iCol+1));
      }
    }
  }

  assert( ((Fts3Table *)pCsr->base.pVtab)->pSegments==0 );
  return rc;
}

/* 
** This function is the implementation of the xUpdate callback used by 
** FTS3 virtual tables. It is invoked by SQLite each time a row is to be
** inserted, updated or deleted.
*/
static int fts3UpdateMethod(
  sqlite3_vtab *pVtab,            /* Virtual table handle */
  int nArg,                       /* Size of argument array */
  sqlite3_value **apVal,          /* Array of arguments */
  sqlite_int64 *pRowid            /* OUT: The affected (or effected) rowid */
){
  return sqlite3Fts3UpdateMethod(pVtab, nArg, apVal, pRowid);
}

/*
** Implementation of xSync() method. Flush the contents of the pending-terms
** hash-table to the database.
*/
static int fts3SyncMethod(sqlite3_vtab *pVtab){

  /* Following an incremental-merge operation, assuming that the input
  ** segments are not completely consumed (the usual case), they are updated
  ** in place to remove the entries that have already been merged. This
  ** involves updating the leaf block that contains the smallest unmerged
  ** entry and each block (if any) between the leaf and the root node. So
  ** if the height of the input segment b-trees is N, and input segments
  ** are merged eight at a time, updating the input segments at the end
  ** of an incremental-merge requires writing (8*(1+N)) blocks. N is usually
  ** small - often between 0 and 2. So the overhead of the incremental
  ** merge is somewhere between 8 and 24 blocks. To avoid this overhead
  ** dwarfing the actual productive work accomplished, the incremental merge
  ** is only attempted if it will write at least 64 leaf blocks. Hence
  ** nMinMerge.
  **
  ** Of course, updating the input segments also involves deleting a bunch
  ** of blocks from the segments table. But this is not considered overhead
  ** as it would also be required by a crisis-merge that used the same input 
  ** segments.
  */
  const u32 nMinMerge = 64;       /* Minimum amount of incr-merge work to do */

  Fts3Table *p = (Fts3Table*)pVtab;
  int rc = sqlite3Fts3PendingTermsFlush(p);

  if( rc==SQLITE_OK 
   && p->nLeafAdd>(nMinMerge/16) 
   && p->nAutoincrmerge && p->nAutoincrmerge!=0xff
  ){
    int mxLevel = 0;              /* Maximum relative level value in db */
    int A;                        /* Incr-merge parameter A */

    rc = sqlite3Fts3MaxLevel(p, &mxLevel);
    assert( rc==SQLITE_OK || mxLevel==0 );
    A = p->nLeafAdd * mxLevel;
    A += (A/2);
    if( A>(int)nMinMerge ) rc = sqlite3Fts3Incrmerge(p, A, p->nAutoincrmerge);
  }
  sqlite3Fts3SegmentsClose(p);
  return rc;
}

/*
** If it is currently unknown whether or not the FTS table has an %_stat
** table (if p->bHasStat==2), attempt to determine this (set p->bHasStat
** to 0 or 1). Return SQLITE_OK if successful, or an SQLite error code
** if an error occurs.
*/
static int fts3SetHasStat(Fts3Table *p){
  int rc = SQLITE_OK;
  if( p->bHasStat==2 ){
    const char *zFmt ="SELECT 1 FROM %Q.sqlite_master WHERE tbl_name='%q_stat'";
    char *zSql = sqlite3_mprintf(zFmt, p->zDb, p->zName);
    if( zSql ){
      sqlite3_stmt *pStmt = 0;
      rc = sqlite3_prepare_v2(p->db, zSql, -1, &pStmt, 0);
      if( rc==SQLITE_OK ){
        int bHasStat = (sqlite3_step(pStmt)==SQLITE_ROW);
        rc = sqlite3_finalize(pStmt);
        if( rc==SQLITE_OK ) p->bHasStat = bHasStat;
      }
      sqlite3_free(zSql);
    }else{
      rc = SQLITE_NOMEM;
    }
  }
  return rc;
}

/*
** Implementation of xBegin() method. 
*/
static int fts3BeginMethod(sqlite3_vtab *pVtab){
  Fts3Table *p = (Fts3Table*)pVtab;
  UNUSED_PARAMETER(pVtab);
  assert( p->pSegments==0 );
  assert( p->nPendingData==0 );
  assert( p->inTransaction!=1 );
  TESTONLY( p->inTransaction = 1 );
  TESTONLY( p->mxSavepoint = -1; );
  p->nLeafAdd = 0;
  return fts3SetHasStat(p);
}

/*
** Implementation of xCommit() method. This is a no-op. The contents of
** the pending-terms hash-table have already been flushed into the database
** by fts3SyncMethod().
*/
static int fts3CommitMethod(sqlite3_vtab *pVtab){
  TESTONLY( Fts3Table *p = (Fts3Table*)pVtab );
  UNUSED_PARAMETER(pVtab);
  assert( p->nPendingData==0 );
  assert( p->inTransaction!=0 );
  assert( p->pSegments==0 );
  TESTONLY( p->inTransaction = 0 );
  TESTONLY( p->mxSavepoint = -1; );
  return SQLITE_OK;
}

/*
** Implementation of xRollback(). Discard the contents of the pending-terms
** hash-table. Any changes made to the database are reverted by SQLite.
*/
static int fts3RollbackMethod(sqlite3_vtab *pVtab){
  Fts3Table *p = (Fts3Table*)pVtab;
  sqlite3Fts3PendingTermsClear(p);
  assert( p->inTransaction!=0 );
  TESTONLY( p->inTransaction = 0 );
  TESTONLY( p->mxSavepoint = -1; );
  return SQLITE_OK;
}

/*
** When called, *ppPoslist must point to the byte immediately following the
** end of a position-list. i.e. ( (*ppPoslist)[-1]==POS_END ). This function
** moves *ppPoslist so that it instead points to the first byte of the
** same position list.
*/
static void fts3ReversePoslist(char *pStart, char **ppPoslist){
  char *p = &(*ppPoslist)[-2];
  char c = 0;

  /* Skip backwards passed any trailing 0x00 bytes added by NearTrim() */
  while( p>pStart && (c=*p--)==0 );

  /* Search backwards for a varint with value zero (the end of the previous 
  ** poslist). This is an 0x00 byte preceded by some byte that does not
  ** have the 0x80 bit set.  */
  while( p>pStart && (*p & 0x80) | c ){ 
    c = *p--; 
  }
  assert( p==pStart || c==0 );

  /* At this point p points to that preceding byte without the 0x80 bit
  ** set. So to find the start of the poslist, skip forward 2 bytes then
  ** over a varint. 
  **
  ** Normally. The other case is that p==pStart and the poslist to return
  ** is the first in the doclist. In this case do not skip forward 2 bytes.
  ** The second part of the if condition (c==0 && *ppPoslist>&p[2])
  ** is required for cases where the first byte of a doclist and the
  ** doclist is empty. For example, if the first docid is 10, a doclist
  ** that begins with:
  **
  **   0x0A 0x00 <next docid delta varint>
  */
  if( p>pStart || (c==0 && *ppPoslist>&p[2]) ){ p = &p[2]; }
  while( *p++&0x80 );
  *ppPoslist = p;
}

/*
** Helper function used by the implementation of the overloaded snippet(),
** offsets() and optimize() SQL functions.
**
** If the value passed as the third argument is a blob of size
** sizeof(Fts3Cursor*), then the blob contents are copied to the 
** output variable *ppCsr and SQLITE_OK is returned. Otherwise, an error
** message is written to context pContext and SQLITE_ERROR returned. The
** string passed via zFunc is used as part of the error message.
*/
static int fts3FunctionArg(
  sqlite3_context *pContext,      /* SQL function call context */
  const char *zFunc,              /* Function name */
  sqlite3_value *pVal,            /* argv[0] passed to function */
  Fts3Cursor **ppCsr              /* OUT: Store cursor handle here */
){
  Fts3Cursor *pRet;
  if( sqlite3_value_type(pVal)!=SQLITE_BLOB 
   || sqlite3_value_bytes(pVal)!=sizeof(Fts3Cursor *)
  ){
    char *zErr = sqlite3_mprintf("illegal first argument to %s", zFunc);
    sqlite3_result_error(pContext, zErr, -1);
    sqlite3_free(zErr);
    return SQLITE_ERROR;
  }
  memcpy(&pRet, sqlite3_value_blob(pVal), sizeof(Fts3Cursor *));
  *ppCsr = pRet;
  return SQLITE_OK;
}

/*
** Implementation of the snippet() function for FTS3
*/
static void fts3SnippetFunc(
  sqlite3_context *pContext,      /* SQLite function call context */
  int nVal,                       /* Size of apVal[] array */
  sqlite3_value **apVal           /* Array of arguments */
){
  Fts3Cursor *pCsr;               /* Cursor handle passed through apVal[0] */
  const char *zStart = "<b>";
  const char *zEnd = "</b>";
  const char *zEllipsis = "<b>...</b>";
  int iCol = -1;
  int nToken = 15;                /* Default number of tokens in snippet */

  /* There must be at least one argument passed to this function (otherwise
  ** the non-overloaded version would have been called instead of this one).
  */
  assert( nVal>=1 );

  if( nVal>6 ){
    sqlite3_result_error(pContext, 
        "wrong number of arguments to function snippet()", -1);
    return;
  }
  if( fts3FunctionArg(pContext, "snippet", apVal[0], &pCsr) ) return;

  switch( nVal ){
    case 6: nToken = sqlite3_value_int(apVal[5]);
    case 5: iCol = sqlite3_value_int(apVal[4]);
    case 4: zEllipsis = (const char*)sqlite3_value_text(apVal[3]);
    case 3: zEnd = (const char*)sqlite3_value_text(apVal[2]);
    case 2: zStart = (const char*)sqlite3_value_text(apVal[1]);
  }
  if( !zEllipsis || !zEnd || !zStart ){
    sqlite3_result_error_nomem(pContext);
  }else if( nToken==0 ){
    sqlite3_result_text(pContext, "", -1, SQLITE_STATIC);
  }else if( SQLITE_OK==fts3CursorSeek(pContext, pCsr) ){
    sqlite3Fts3Snippet(pContext, pCsr, zStart, zEnd, zEllipsis, iCol, nToken);
  }
}

/*
** Implementation of the offsets() function for FTS3
*/
static void fts3OffsetsFunc(
  sqlite3_context *pContext,      /* SQLite function call context */
  int nVal,                       /* Size of argument array */
  sqlite3_value **apVal           /* Array of arguments */
){
  Fts3Cursor *pCsr;               /* Cursor handle passed through apVal[0] */

  UNUSED_PARAMETER(nVal);

  assert( nVal==1 );
  if( fts3FunctionArg(pContext, "offsets", apVal[0], &pCsr) ) return;
  assert( pCsr );
  if( SQLITE_OK==fts3CursorSeek(pContext, pCsr) ){
    sqlite3Fts3Offsets(pContext, pCsr);
  }
}

/* 
** Implementation of the special optimize() function for FTS3. This 
** function merges all segments in the database to a single segment.
** Example usage is:
**
**   SELECT optimize(t) FROM t LIMIT 1;
**
** where 't' is the name of an FTS3 table.
*/
static void fts3OptimizeFunc(
  sqlite3_context *pContext,      /* SQLite function call context */
  int nVal,                       /* Size of argument array */
  sqlite3_value **apVal           /* Array of arguments */
){
  int rc;                         /* Return code */
  Fts3Table *p;                   /* Virtual table handle */
  Fts3Cursor *pCursor;            /* Cursor handle passed through apVal[0] */

  UNUSED_PARAMETER(nVal);

  assert( nVal==1 );
  if( fts3FunctionArg(pContext, "optimize", apVal[0], &pCursor) ) return;
  p = (Fts3Table *)pCursor->base.pVtab;
  assert( p );

  rc = sqlite3Fts3Optimize(p);

  switch( rc ){
    case SQLITE_OK:
      sqlite3_result_text(pContext, "Index optimized", -1, SQLITE_STATIC);
      break;
    case SQLITE_DONE:
      sqlite3_result_text(pContext, "Index already optimal", -1, SQLITE_STATIC);
      break;
    default:
      sqlite3_result_error_code(pContext, rc);
      break;
  }
}

/*
** Implementation of the matchinfo() function for FTS3
*/
static void fts3MatchinfoFunc(
  sqlite3_context *pContext,      /* SQLite function call context */
  int nVal,                       /* Size of argument array */
  sqlite3_value **apVal           /* Array of arguments */
){
  Fts3Cursor *pCsr;               /* Cursor handle passed through apVal[0] */
  assert( nVal==1 || nVal==2 );
  if( SQLITE_OK==fts3FunctionArg(pContext, "matchinfo", apVal[0], &pCsr) ){
    const char *zArg = 0;
    if( nVal>1 ){
      zArg = (const char *)sqlite3_value_text(apVal[1]);
    }
    sqlite3Fts3Matchinfo(pContext, pCsr, zArg);
  }
}

/*
** This routine implements the xFindFunction method for the FTS3
** virtual table.
*/
static int fts3FindFunctionMethod(
  sqlite3_vtab *pVtab,            /* Virtual table handle */
  int nArg,                       /* Number of SQL function arguments */
  const char *zName,              /* Name of SQL function */
  void (**pxFunc)(sqlite3_context*,int,sqlite3_value**), /* OUT: Result */
  void **ppArg                    /* Unused */
){
  struct Overloaded {
    const char *zName;
    void (*xFunc)(sqlite3_context*,int,sqlite3_value**);
  } aOverload[] = {
    { "snippet", fts3SnippetFunc },
    { "offsets", fts3OffsetsFunc },
    { "optimize", fts3OptimizeFunc },
    { "matchinfo", fts3MatchinfoFunc },
  };
  int i;                          /* Iterator variable */

  UNUSED_PARAMETER(pVtab);
  UNUSED_PARAMETER(nArg);
  UNUSED_PARAMETER(ppArg);

  for(i=0; i<SizeofArray(aOverload); i++){
    if( strcmp(zName, aOverload[i].zName)==0 ){
      *pxFunc = aOverload[i].xFunc;
      return 1;
    }
  }

  /* No function of the specified name was found. Return 0. */
  return 0;
}

/*
** Implementation of FTS3 xRename method. Rename an fts3 table.
*/
static int fts3RenameMethod(
  sqlite3_vtab *pVtab,            /* Virtual table handle */
  const char *zName               /* New name of table */
){
  Fts3Table *p = (Fts3Table *)pVtab;
  sqlite3 *db = p->db;            /* Database connection */
  int rc;                         /* Return Code */

  /* At this point it must be known if the %_stat table exists or not.
  ** So bHasStat may not be 2.  */
  rc = fts3SetHasStat(p);
  
  /* As it happens, the pending terms table is always empty here. This is
  ** because an "ALTER TABLE RENAME TABLE" statement inside a transaction 
  ** always opens a savepoint transaction. And the xSavepoint() method 
  ** flushes the pending terms table. But leave the (no-op) call to
  ** PendingTermsFlush() in in case that changes.
  */
  assert( p->nPendingData==0 );
  if( rc==SQLITE_OK ){
    rc = sqlite3Fts3PendingTermsFlush(p);
  }

  if( p->zContentTbl==0 ){
    fts3DbExec(&rc, db,
      "ALTER TABLE %Q.'%q_content'  RENAME TO '%q_content';",
      p->zDb, p->zName, zName
    );
  }

  if( p->bHasDocsize ){
    fts3DbExec(&rc, db,
      "ALTER TABLE %Q.'%q_docsize'  RENAME TO '%q_docsize';",
      p->zDb, p->zName, zName
    );
  }
  if( p->bHasStat ){
    fts3DbExec(&rc, db,
      "ALTER TABLE %Q.'%q_stat'  RENAME TO '%q_stat';",
      p->zDb, p->zName, zName
    );
  }
  fts3DbExec(&rc, db,
    "ALTER TABLE %Q.'%q_segments' RENAME TO '%q_segments';",
    p->zDb, p->zName, zName
  );
  fts3DbExec(&rc, db,
    "ALTER TABLE %Q.'%q_segdir'   RENAME TO '%q_segdir';",
    p->zDb, p->zName, zName
  );
  return rc;
}

/*
** The xSavepoint() method.
**
** Flush the contents of the pending-terms table to disk.
*/
static int fts3SavepointMethod(sqlite3_vtab *pVtab, int iSavepoint){
  int rc = SQLITE_OK;
  UNUSED_PARAMETER(iSavepoint);
  assert( ((Fts3Table *)pVtab)->inTransaction );
  assert( ((Fts3Table *)pVtab)->mxSavepoint < iSavepoint );
  TESTONLY( ((Fts3Table *)pVtab)->mxSavepoint = iSavepoint );
  if( ((Fts3Table *)pVtab)->bIgnoreSavepoint==0 ){
    rc = fts3SyncMethod(pVtab);
  }
  return rc;
}

/*
** The xRelease() method.
**
** This is a no-op.
*/
static int fts3ReleaseMethod(sqlite3_vtab *pVtab, int iSavepoint){
  TESTONLY( Fts3Table *p = (Fts3Table*)pVtab );
  UNUSED_PARAMETER(iSavepoint);
  UNUSED_PARAMETER(pVtab);
  assert( p->inTransaction );
  assert( p->mxSavepoint >= iSavepoint );
  TESTONLY( p->mxSavepoint = iSavepoint-1 );
  return SQLITE_OK;
}

/*
** The xRollbackTo() method.
**
** Discard the contents of the pending terms table.
*/
static int fts3RollbackToMethod(sqlite3_vtab *pVtab, int iSavepoint){
  Fts3Table *p = (Fts3Table*)pVtab;
  UNUSED_PARAMETER(iSavepoint);
  assert( p->inTransaction );
  assert( p->mxSavepoint >= iSavepoint );
  TESTONLY( p->mxSavepoint = iSavepoint );
  sqlite3Fts3PendingTermsClear(p);
  return SQLITE_OK;
}

static const sqlite3_module fts3Module = {
  /* iVersion      */ 2,
  /* xCreate       */ fts3CreateMethod,
  /* xConnect      */ fts3ConnectMethod,
  /* xBestIndex    */ fts3BestIndexMethod,
  /* xDisconnect   */ fts3DisconnectMethod,
  /* xDestroy      */ fts3DestroyMethod,
  /* xOpen         */ fts3OpenMethod,
  /* xClose        */ fts3CloseMethod,
  /* xFilter       */ fts3FilterMethod,
  /* xNext         */ fts3NextMethod,
  /* xEof          */ fts3EofMethod,
  /* xColumn       */ fts3ColumnMethod,
  /* xRowid        */ fts3RowidMethod,
  /* xUpdate       */ fts3UpdateMethod,
  /* xBegin        */ fts3BeginMethod,
  /* xSync         */ fts3SyncMethod,
  /* xCommit       */ fts3CommitMethod,
  /* xRollback     */ fts3RollbackMethod,
  /* xFindFunction */ fts3FindFunctionMethod,
  /* xRename */       fts3RenameMethod,
  /* xSavepoint    */ fts3SavepointMethod,
  /* xRelease      */ fts3ReleaseMethod,
  /* xRollbackTo   */ fts3RollbackToMethod,
};

/*
** This function is registered as the module destructor (called when an
** FTS3 enabled database connection is closed). It frees the memory
** allocated for the tokenizer hash table.
*/
static void hashDestroy(void *p){
  Fts3Hash *pHash = (Fts3Hash *)p;
  sqlite3Fts3HashClear(pHash);
  sqlite3_free(pHash);
}

/*
** The fts3 built-in tokenizers - "simple", "porter" and "icu"- are 
** implemented in files fts3_tokenizer1.c, fts3_porter.c and fts3_icu.c
** respectively. The following three forward declarations are for functions
** declared in these files used to retrieve the respective implementations.
**
** Calling sqlite3Fts3SimpleTokenizerModule() sets the value pointed
** to by the argument to point to the "simple" tokenizer implementation.
** And so on.
*/
SQLITE_PRIVATE void sqlite3Fts3SimpleTokenizerModule(sqlite3_tokenizer_module const**ppModule);
SQLITE_PRIVATE void sqlite3Fts3PorterTokenizerModule(sqlite3_tokenizer_module const**ppModule);
#ifndef SQLITE_DISABLE_FTS3_UNICODE
SQLITE_PRIVATE void sqlite3Fts3UnicodeTokenizer(sqlite3_tokenizer_module const**ppModule);
#endif
#ifdef SQLITE_ENABLE_ICU
SQLITE_PRIVATE void sqlite3Fts3IcuTokenizerModule(sqlite3_tokenizer_module const**ppModule);
#endif

/*
** Initialize the fts3 extension. If this extension is built as part
** of the sqlite library, then this function is called directly by
** SQLite. If fts3 is built as a dynamically loadable extension, this
** function is called by the sqlite3_extension_init() entry point.
*/
SQLITE_PRIVATE int sqlite3Fts3Init(sqlite3 *db){
  int rc = SQLITE_OK;
  Fts3Hash *pHash = 0;
  const sqlite3_tokenizer_module *pSimple = 0;
  const sqlite3_tokenizer_module *pPorter = 0;
#ifndef SQLITE_DISABLE_FTS3_UNICODE
  const sqlite3_tokenizer_module *pUnicode = 0;
#endif

#ifdef SQLITE_ENABLE_ICU
  const sqlite3_tokenizer_module *pIcu = 0;
  sqlite3Fts3IcuTokenizerModule(&pIcu);
#endif

#ifndef SQLITE_DISABLE_FTS3_UNICODE
  sqlite3Fts3UnicodeTokenizer(&pUnicode);
#endif

#ifdef SQLITE_TEST
  rc = sqlite3Fts3InitTerm(db);
  if( rc!=SQLITE_OK ) return rc;
#endif

  rc = sqlite3Fts3InitAux(db);
  if( rc!=SQLITE_OK ) return rc;

  sqlite3Fts3SimpleTokenizerModule(&pSimple);
  sqlite3Fts3PorterTokenizerModule(&pPorter);

  /* Allocate and initialize the hash-table used to store tokenizers. */
  pHash = sqlite3_malloc(sizeof(Fts3Hash));
  if( !pHash ){
    rc = SQLITE_NOMEM;
  }else{
    sqlite3Fts3HashInit(pHash, FTS3_HASH_STRING, 1);
  }

  /* Load the built-in tokenizers into the hash table */
  if( rc==SQLITE_OK ){
    if( sqlite3Fts3HashInsert(pHash, "simple", 7, (void *)pSimple)
     || sqlite3Fts3HashInsert(pHash, "porter", 7, (void *)pPorter) 

#ifndef SQLITE_DISABLE_FTS3_UNICODE
     || sqlite3Fts3HashInsert(pHash, "unicode61", 10, (void *)pUnicode) 
#endif
#ifdef SQLITE_ENABLE_ICU
     || (pIcu && sqlite3Fts3HashInsert(pHash, "icu", 4, (void *)pIcu))
#endif
    ){
      rc = SQLITE_NOMEM;
    }
  }

#ifdef SQLITE_TEST
  if( rc==SQLITE_OK ){
    rc = sqlite3Fts3ExprInitTestInterface(db);
  }
#endif

  /* Create the virtual table wrapper around the hash-table and overload 
  ** the two scalar functions. If this is successful, register the
  ** module with sqlite.
  */
  if( SQLITE_OK==rc 
   && SQLITE_OK==(rc = sqlite3Fts3InitHashTable(db, pHash, "fts3_tokenizer"))
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "snippet", -1))
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "offsets", 1))
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "matchinfo", 1))
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "matchinfo", 2))
   && SQLITE_OK==(rc = sqlite3_overload_function(db, "optimize", 1))
  ){
    rc = sqlite3_create_module_v2(
        db, "fts3", &fts3Module, (void *)pHash, hashDestroy
    );
    if( rc==SQLITE_OK ){
      rc = sqlite3_create_module_v2(
          db, "fts4", &fts3Module, (void *)pHash, 0
      );
    }
    if( rc==SQLITE_OK ){
      rc = sqlite3Fts3InitTok(db, (void *)pHash);
    }
    return rc;
  }


  /* An error has occurred. Delete the hash table and return the error code. */
  assert( rc!=SQLITE_OK );
  if( pHash ){
    sqlite3Fts3HashClear(pHash);
    sqlite3_free(pHash);
  }
  return rc;
}

/*
** Allocate an Fts3MultiSegReader for each token in the expression headed
** by pExpr. 
**
** An Fts3SegReader object is a cursor that can seek or scan a range of
** entries within a single segment b-tree. An Fts3MultiSegReader uses multiple
** Fts3SegReader objects internally to provide an interface to seek or scan
** within the union of all segments of a b-tree. Hence the name.
**
** If the allocated Fts3MultiSegReader just seeks to a single entry in a
** segment b-tree (if the term is not a prefix or it is a prefix for which
** there exists prefix b-tree of the right length) then it may be traversed
** and merged incrementally. Otherwise, it has to be merged into an in-memory 
** doclist and then traversed.
*/
static void fts3EvalAllocateReaders(
  Fts3Cursor *pCsr,               /* FTS cursor handle */
  Fts3Expr *pExpr,                /* Allocate readers for this expression */
  int *pnToken,                   /* OUT: Total number of tokens in phrase. */
  int *pnOr,                      /* OUT: Total number of OR nodes in expr. */
  int *pRc                        /* IN/OUT: Error code */
){
  if( pExpr && SQLITE_OK==*pRc ){
    if( pExpr->eType==FTSQUERY_PHRASE ){
      int i;
      int nToken = pExpr->pPhrase->nToken;
      *pnToken += nToken;
      for(i=0; i<nToken; i++){
        Fts3PhraseToken *pToken = &pExpr->pPhrase->aToken[i];
        int rc = fts3TermSegReaderCursor(pCsr, 
            pToken->z, pToken->n, pToken->isPrefix, &pToken->pSegcsr
        );
        if( rc!=SQLITE_OK ){
          *pRc = rc;
          return;
        }
      }
      assert( pExpr->pPhrase->iDoclistToken==0 );
      pExpr->pPhrase->iDoclistToken = -1;
    }else{
      *pnOr += (pExpr->eType==FTSQUERY_OR);
      fts3EvalAllocateReaders(pCsr, pExpr->pLeft, pnToken, pnOr, pRc);
      fts3EvalAllocateReaders(pCsr, pExpr->pRight, pnToken, pnOr, pRc);
    }
  }
}

/*
** Arguments pList/nList contain the doclist for token iToken of phrase p.
** It is merged into the main doclist stored in p->doclist.aAll/nAll.
**
** This function assumes that pList points to a buffer allocated using
** sqlite3_malloc(). This function takes responsibility for eventually
** freeing the buffer.
**
** SQLITE_OK is returned if successful, or SQLITE_NOMEM if an error occurs.
*/
static int fts3EvalPhraseMergeToken(
  Fts3Table *pTab,                /* FTS Table pointer */
  Fts3Phrase *p,                  /* Phrase to merge pList/nList into */
  int iToken,                     /* Token pList/nList corresponds to */
  char *pList,                    /* Pointer to doclist */
  int nList                       /* Number of bytes in pList */
){
  int rc = SQLITE_OK;
  assert( iToken!=p->iDoclistToken );

  if( pList==0 ){
    sqlite3_free(p->doclist.aAll);
    p->doclist.aAll = 0;
    p->doclist.nAll = 0;
  }

  else if( p->iDoclistToken<0 ){
    p->doclist.aAll = pList;
    p->doclist.nAll = nList;
  }

  else if( p->doclist.aAll==0 ){
    sqlite3_free(pList);
  }

  else {
    char *pLeft;
    char *pRight;
    int nLeft;
    int nRight;
    int nDiff;

    if( p->iDoclistToken<iToken ){
      pLeft = p->doclist.aAll;
      nLeft = p->doclist.nAll;
      pRight = pList;
      nRight = nList;
      nDiff = iToken - p->iDoclistToken;
    }else{
      pRight = p->doclist.aAll;
      nRight = p->doclist.nAll;
      pLeft = pList;
      nLeft = nList;
      nDiff = p->iDoclistToken - iToken;
    }

    rc = fts3DoclistPhraseMerge(
        pTab->bDescIdx, nDiff, pLeft, nLeft, &pRight, &nRight
    );
    sqlite3_free(pLeft);
    p->doclist.aAll = pRight;
    p->doclist.nAll = nRight;
  }

  if( iToken>p->iDoclistToken ) p->iDoclistToken = iToken;
  return rc;
}

/*
** Load the doclist for phrase p into p->doclist.aAll/nAll. The loaded doclist
** does not take deferred tokens into account.
**
** SQLITE_OK is returned if no error occurs, otherwise an SQLite error code.
*/
static int fts3EvalPhraseLoad(
  Fts3Cursor *pCsr,               /* FTS Cursor handle */
  Fts3Phrase *p                   /* Phrase object */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int iToken;
  int rc = SQLITE_OK;

  for(iToken=0; rc==SQLITE_OK && iToken<p->nToken; iToken++){
    Fts3PhraseToken *pToken = &p->aToken[iToken];
    assert( pToken->pDeferred==0 || pToken->pSegcsr==0 );

    if( pToken->pSegcsr ){
      int nThis = 0;
      char *pThis = 0;
      rc = fts3TermSelect(pTab, pToken, p->iColumn, &nThis, &pThis);
      if( rc==SQLITE_OK ){
        rc = fts3EvalPhraseMergeToken(pTab, p, iToken, pThis, nThis);
      }
    }
    assert( pToken->pSegcsr==0 );
  }

  return rc;
}

/*
** This function is called on each phrase after the position lists for
** any deferred tokens have been loaded into memory. It updates the phrases
** current position list to include only those positions that are really
** instances of the phrase (after considering deferred tokens). If this
** means that the phrase does not appear in the current row, doclist.pList
** and doclist.nList are both zeroed.
**
** SQLITE_OK is returned if no error occurs, otherwise an SQLite error code.
*/
static int fts3EvalDeferredPhrase(Fts3Cursor *pCsr, Fts3Phrase *pPhrase){
  int iToken;                     /* Used to iterate through phrase tokens */
  char *aPoslist = 0;             /* Position list for deferred tokens */
  int nPoslist = 0;               /* Number of bytes in aPoslist */
  int iPrev = -1;                 /* Token number of previous deferred token */

  assert( pPhrase->doclist.bFreeList==0 );

  for(iToken=0; iToken<pPhrase->nToken; iToken++){
    Fts3PhraseToken *pToken = &pPhrase->aToken[iToken];
    Fts3DeferredToken *pDeferred = pToken->pDeferred;

    if( pDeferred ){
      char *pList;
      int nList;
      int rc = sqlite3Fts3DeferredTokenList(pDeferred, &pList, &nList);
      if( rc!=SQLITE_OK ) return rc;

      if( pList==0 ){
        sqlite3_free(aPoslist);
        pPhrase->doclist.pList = 0;
        pPhrase->doclist.nList = 0;
        return SQLITE_OK;

      }else if( aPoslist==0 ){
        aPoslist = pList;
        nPoslist = nList;

      }else{
        char *aOut = pList;
        char *p1 = aPoslist;
        char *p2 = aOut;

        assert( iPrev>=0 );
        fts3PoslistPhraseMerge(&aOut, iToken-iPrev, 0, 1, &p1, &p2);
        sqlite3_free(aPoslist);
        aPoslist = pList;
        nPoslist = (int)(aOut - aPoslist);
        if( nPoslist==0 ){
          sqlite3_free(aPoslist);
          pPhrase->doclist.pList = 0;
          pPhrase->doclist.nList = 0;
          return SQLITE_OK;
        }
      }
      iPrev = iToken;
    }
  }

  if( iPrev>=0 ){
    int nMaxUndeferred = pPhrase->iDoclistToken;
    if( nMaxUndeferred<0 ){
      pPhrase->doclist.pList = aPoslist;
      pPhrase->doclist.nList = nPoslist;
      pPhrase->doclist.iDocid = pCsr->iPrevId;
      pPhrase->doclist.bFreeList = 1;
    }else{
      int nDistance;
      char *p1;
      char *p2;
      char *aOut;

      if( nMaxUndeferred>iPrev ){
        p1 = aPoslist;
        p2 = pPhrase->doclist.pList;
        nDistance = nMaxUndeferred - iPrev;
      }else{
        p1 = pPhrase->doclist.pList;
        p2 = aPoslist;
        nDistance = iPrev - nMaxUndeferred;
      }

      aOut = (char *)sqlite3_malloc(nPoslist+8);
      if( !aOut ){
        sqlite3_free(aPoslist);
        return SQLITE_NOMEM;
      }
      
      pPhrase->doclist.pList = aOut;
      if( fts3PoslistPhraseMerge(&aOut, nDistance, 0, 1, &p1, &p2) ){
        pPhrase->doclist.bFreeList = 1;
        pPhrase->doclist.nList = (int)(aOut - pPhrase->doclist.pList);
      }else{
        sqlite3_free(aOut);
        pPhrase->doclist.pList = 0;
        pPhrase->doclist.nList = 0;
      }
      sqlite3_free(aPoslist);
    }
  }

  return SQLITE_OK;
}

/*
** Maximum number of tokens a phrase may have to be considered for the
** incremental doclists strategy.
*/
#define MAX_INCR_PHRASE_TOKENS 4

/*
** This function is called for each Fts3Phrase in a full-text query 
** expression to initialize the mechanism for returning rows. Once this
** function has been called successfully on an Fts3Phrase, it may be
** used with fts3EvalPhraseNext() to iterate through the matching docids.
**
** If parameter bOptOk is true, then the phrase may (or may not) use the
** incremental loading strategy. Otherwise, the entire doclist is loaded into
** memory within this call.
**
** SQLITE_OK is returned if no error occurs, otherwise an SQLite error code.
*/
static int fts3EvalPhraseStart(Fts3Cursor *pCsr, int bOptOk, Fts3Phrase *p){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc = SQLITE_OK;             /* Error code */
  int i;

  /* Determine if doclists may be loaded from disk incrementally. This is
  ** possible if the bOptOk argument is true, the FTS doclists will be
  ** scanned in forward order, and the phrase consists of 
  ** MAX_INCR_PHRASE_TOKENS or fewer tokens, none of which are are "^first"
  ** tokens or prefix tokens that cannot use a prefix-index.  */
  int bHaveIncr = 0;
  int bIncrOk = (bOptOk 
   && pCsr->bDesc==pTab->bDescIdx 
   && p->nToken<=MAX_INCR_PHRASE_TOKENS && p->nToken>0
#ifdef SQLITE_TEST
   && pTab->bNoIncrDoclist==0
#endif
  );
  for(i=0; bIncrOk==1 && i<p->nToken; i++){
    Fts3PhraseToken *pToken = &p->aToken[i];
    if( pToken->bFirst || (pToken->pSegcsr!=0 && !pToken->pSegcsr->bLookup) ){
      bIncrOk = 0;
    }
    if( pToken->pSegcsr ) bHaveIncr = 1;
  }

  if( bIncrOk && bHaveIncr ){
    /* Use the incremental approach. */
    int iCol = (p->iColumn >= pTab->nColumn ? -1 : p->iColumn);
    for(i=0; rc==SQLITE_OK && i<p->nToken; i++){
      Fts3PhraseToken *pToken = &p->aToken[i];
      Fts3MultiSegReader *pSegcsr = pToken->pSegcsr;
      if( pSegcsr ){
        rc = sqlite3Fts3MsrIncrStart(pTab, pSegcsr, iCol, pToken->z, pToken->n);
      }
    }
    p->bIncr = 1;
  }else{
    /* Load the full doclist for the phrase into memory. */
    rc = fts3EvalPhraseLoad(pCsr, p);
    p->bIncr = 0;
  }

  assert( rc!=SQLITE_OK || p->nToken<1 || p->aToken[0].pSegcsr==0 || p->bIncr );
  return rc;
}

/*
** This function is used to iterate backwards (from the end to start) 
** through doclists. It is used by this module to iterate through phrase
** doclists in reverse and by the fts3_write.c module to iterate through
** pending-terms lists when writing to databases with "order=desc".
**
** The doclist may be sorted in ascending (parameter bDescIdx==0) or 
** descending (parameter bDescIdx==1) order of docid. Regardless, this
** function iterates from the end of the doclist to the beginning.
*/
SQLITE_PRIVATE void sqlite3Fts3DoclistPrev(
  int bDescIdx,                   /* True if the doclist is desc */
  char *aDoclist,                 /* Pointer to entire doclist */
  int nDoclist,                   /* Length of aDoclist in bytes */
  char **ppIter,                  /* IN/OUT: Iterator pointer */
  sqlite3_int64 *piDocid,         /* IN/OUT: Docid pointer */
  int *pnList,                    /* OUT: List length pointer */
  u8 *pbEof                       /* OUT: End-of-file flag */
){
  char *p = *ppIter;

  assert( nDoclist>0 );
  assert( *pbEof==0 );
  assert( p || *piDocid==0 );
  assert( !p || (p>aDoclist && p<&aDoclist[nDoclist]) );

  if( p==0 ){
    sqlite3_int64 iDocid = 0;
    char *pNext = 0;
    char *pDocid = aDoclist;
    char *pEnd = &aDoclist[nDoclist];
    int iMul = 1;

    while( pDocid<pEnd ){
      sqlite3_int64 iDelta;
      pDocid += sqlite3Fts3GetVarint(pDocid, &iDelta);
      iDocid += (iMul * iDelta);
      pNext = pDocid;
      fts3PoslistCopy(0, &pDocid);
      while( pDocid<pEnd && *pDocid==0 ) pDocid++;
      iMul = (bDescIdx ? -1 : 1);
    }

    *pnList = (int)(pEnd - pNext);
    *ppIter = pNext;
    *piDocid = iDocid;
  }else{
    int iMul = (bDescIdx ? -1 : 1);
    sqlite3_int64 iDelta;
    fts3GetReverseVarint(&p, aDoclist, &iDelta);
    *piDocid -= (iMul * iDelta);

    if( p==aDoclist ){
      *pbEof = 1;
    }else{
      char *pSave = p;
      fts3ReversePoslist(aDoclist, &p);
      *pnList = (int)(pSave - p);
    }
    *ppIter = p;
  }
}

/*
** Iterate forwards through a doclist.
*/
SQLITE_PRIVATE void sqlite3Fts3DoclistNext(
  int bDescIdx,                   /* True if the doclist is desc */
  char *aDoclist,                 /* Pointer to entire doclist */
  int nDoclist,                   /* Length of aDoclist in bytes */
  char **ppIter,                  /* IN/OUT: Iterator pointer */
  sqlite3_int64 *piDocid,         /* IN/OUT: Docid pointer */
  u8 *pbEof                       /* OUT: End-of-file flag */
){
  char *p = *ppIter;

  assert( nDoclist>0 );
  assert( *pbEof==0 );
  assert( p || *piDocid==0 );
  assert( !p || (p>=aDoclist && p<=&aDoclist[nDoclist]) );

  if( p==0 ){
    p = aDoclist;
    p += sqlite3Fts3GetVarint(p, piDocid);
  }else{
    fts3PoslistCopy(0, &p);
    while( p<&aDoclist[nDoclist] && *p==0 ) p++; 
    if( p>=&aDoclist[nDoclist] ){
      *pbEof = 1;
    }else{
      sqlite3_int64 iVar;
      p += sqlite3Fts3GetVarint(p, &iVar);
      *piDocid += ((bDescIdx ? -1 : 1) * iVar);
    }
  }

  *ppIter = p;
}

/*
** Advance the iterator pDL to the next entry in pDL->aAll/nAll. Set *pbEof
** to true if EOF is reached.
*/
static void fts3EvalDlPhraseNext(
  Fts3Table *pTab,
  Fts3Doclist *pDL,
  u8 *pbEof
){
  char *pIter;                            /* Used to iterate through aAll */
  char *pEnd = &pDL->aAll[pDL->nAll];     /* 1 byte past end of aAll */
 
  if( pDL->pNextDocid ){
    pIter = pDL->pNextDocid;
  }else{
    pIter = pDL->aAll;
  }

  if( pIter>=pEnd ){
    /* We have already reached the end of this doclist. EOF. */
    *pbEof = 1;
  }else{
    sqlite3_int64 iDelta;
    pIter += sqlite3Fts3GetVarint(pIter, &iDelta);
    if( pTab->bDescIdx==0 || pDL->pNextDocid==0 ){
      pDL->iDocid += iDelta;
    }else{
      pDL->iDocid -= iDelta;
    }
    pDL->pList = pIter;
    fts3PoslistCopy(0, &pIter);
    pDL->nList = (int)(pIter - pDL->pList);

    /* pIter now points just past the 0x00 that terminates the position-
    ** list for document pDL->iDocid. However, if this position-list was
    ** edited in place by fts3EvalNearTrim(), then pIter may not actually
    ** point to the start of the next docid value. The following line deals
    ** with this case by advancing pIter past the zero-padding added by
    ** fts3EvalNearTrim().  */
    while( pIter<pEnd && *pIter==0 ) pIter++;

    pDL->pNextDocid = pIter;
    assert( pIter>=&pDL->aAll[pDL->nAll] || *pIter );
    *pbEof = 0;
  }
}

/*
** Helper type used by fts3EvalIncrPhraseNext() and incrPhraseTokenNext().
*/
typedef struct TokenDoclist TokenDoclist;
struct TokenDoclist {
  int bIgnore;
  sqlite3_int64 iDocid;
  char *pList;
  int nList;
};

/*
** Token pToken is an incrementally loaded token that is part of a 
** multi-token phrase. Advance it to the next matching document in the
** database and populate output variable *p with the details of the new
** entry. Or, if the iterator has reached EOF, set *pbEof to true.
**
** If an error occurs, return an SQLite error code. Otherwise, return 
** SQLITE_OK.
*/
static int incrPhraseTokenNext(
  Fts3Table *pTab,                /* Virtual table handle */
  Fts3Phrase *pPhrase,            /* Phrase to advance token of */
  int iToken,                     /* Specific token to advance */
  TokenDoclist *p,                /* OUT: Docid and doclist for new entry */
  u8 *pbEof                       /* OUT: True if iterator is at EOF */
){
  int rc = SQLITE_OK;

  if( pPhrase->iDoclistToken==iToken ){
    assert( p->bIgnore==0 );
    assert( pPhrase->aToken[iToken].pSegcsr==0 );
    fts3EvalDlPhraseNext(pTab, &pPhrase->doclist, pbEof);
    p->pList = pPhrase->doclist.pList;
    p->nList = pPhrase->doclist.nList;
    p->iDocid = pPhrase->doclist.iDocid;
  }else{
    Fts3PhraseToken *pToken = &pPhrase->aToken[iToken];
    assert( pToken->pDeferred==0 );
    assert( pToken->pSegcsr || pPhrase->iDoclistToken>=0 );
    if( pToken->pSegcsr ){
      assert( p->bIgnore==0 );
      rc = sqlite3Fts3MsrIncrNext(
          pTab, pToken->pSegcsr, &p->iDocid, &p->pList, &p->nList
      );
      if( p->pList==0 ) *pbEof = 1;
    }else{
      p->bIgnore = 1;
    }
  }

  return rc;
}


/*
** The phrase iterator passed as the second argument:
**
**   * features at least one token that uses an incremental doclist, and 
**
**   * does not contain any deferred tokens.
**
** Advance it to the next matching documnent in the database and populate
** the Fts3Doclist.pList and nList fields. 
**
** If there is no "next" entry and no error occurs, then *pbEof is set to
** 1 before returning. Otherwise, if no error occurs and the iterator is
** successfully advanced, *pbEof is set to 0.
**
** If an error occurs, return an SQLite error code. Otherwise, return 
** SQLITE_OK.
*/
static int fts3EvalIncrPhraseNext(
  Fts3Cursor *pCsr,               /* FTS Cursor handle */
  Fts3Phrase *p,                  /* Phrase object to advance to next docid */
  u8 *pbEof                       /* OUT: Set to 1 if EOF */
){
  int rc = SQLITE_OK;
  Fts3Doclist *pDL = &p->doclist;
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  u8 bEof = 0;

  /* This is only called if it is guaranteed that the phrase has at least
  ** one incremental token. In which case the bIncr flag is set. */
  assert( p->bIncr==1 );

  if( p->nToken==1 && p->bIncr ){
    rc = sqlite3Fts3MsrIncrNext(pTab, p->aToken[0].pSegcsr, 
        &pDL->iDocid, &pDL->pList, &pDL->nList
    );
    if( pDL->pList==0 ) bEof = 1;
  }else{
    int bDescDoclist = pCsr->bDesc;
    struct TokenDoclist a[MAX_INCR_PHRASE_TOKENS];

    memset(a, 0, sizeof(a));
    assert( p->nToken<=MAX_INCR_PHRASE_TOKENS );
    assert( p->iDoclistToken<MAX_INCR_PHRASE_TOKENS );

    while( bEof==0 ){
      int bMaxSet = 0;
      sqlite3_int64 iMax = 0;     /* Largest docid for all iterators */
      int i;                      /* Used to iterate through tokens */

      /* Advance the iterator for each token in the phrase once. */
      for(i=0; rc==SQLITE_OK && i<p->nToken && bEof==0; i++){
        rc = incrPhraseTokenNext(pTab, p, i, &a[i], &bEof);
        if( a[i].bIgnore==0 && (bMaxSet==0 || DOCID_CMP(iMax, a[i].iDocid)<0) ){
          iMax = a[i].iDocid;
          bMaxSet = 1;
        }
      }
      assert( rc!=SQLITE_OK || (p->nToken>=1 && a[p->nToken-1].bIgnore==0) );
      assert( rc!=SQLITE_OK || bMaxSet );

      /* Keep advancing iterators until they all point to the same document */
      for(i=0; i<p->nToken; i++){
        while( rc==SQLITE_OK && bEof==0 
            && a[i].bIgnore==0 && DOCID_CMP(a[i].iDocid, iMax)<0 
        ){
          rc = incrPhraseTokenNext(pTab, p, i, &a[i], &bEof);
          if( DOCID_CMP(a[i].iDocid, iMax)>0 ){
            iMax = a[i].iDocid;
            i = 0;
          }
        }
      }

      /* Check if the current entries really are a phrase match */
      if( bEof==0 ){
        int nList = 0;
        int nByte = a[p->nToken-1].nList;
        char *aDoclist = sqlite3_malloc(nByte+1);
        if( !aDoclist ) return SQLITE_NOMEM;
        memcpy(aDoclist, a[p->nToken-1].pList, nByte+1);

        for(i=0; i<(p->nToken-1); i++){
          if( a[i].bIgnore==0 ){
            char *pL = a[i].pList;
            char *pR = aDoclist;
            char *pOut = aDoclist;
            int nDist = p->nToken-1-i;
            int res = fts3PoslistPhraseMerge(&pOut, nDist, 0, 1, &pL, &pR);
            if( res==0 ) break;
            nList = (int)(pOut - aDoclist);
          }
        }
        if( i==(p->nToken-1) ){
          pDL->iDocid = iMax;
          pDL->pList = aDoclist;
          pDL->nList = nList;
          pDL->bFreeList = 1;
          break;
        }
        sqlite3_free(aDoclist);
      }
    }
  }

  *pbEof = bEof;
  return rc;
}

/*
** Attempt to move the phrase iterator to point to the next matching docid. 
** If an error occurs, return an SQLite error code. Otherwise, return 
** SQLITE_OK.
**
** If there is no "next" entry and no error occurs, then *pbEof is set to
** 1 before returning. Otherwise, if no error occurs and the iterator is
** successfully advanced, *pbEof is set to 0.
*/
static int fts3EvalPhraseNext(
  Fts3Cursor *pCsr,               /* FTS Cursor handle */
  Fts3Phrase *p,                  /* Phrase object to advance to next docid */
  u8 *pbEof                       /* OUT: Set to 1 if EOF */
){
  int rc = SQLITE_OK;
  Fts3Doclist *pDL = &p->doclist;
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;

  if( p->bIncr ){
    rc = fts3EvalIncrPhraseNext(pCsr, p, pbEof);
  }else if( pCsr->bDesc!=pTab->bDescIdx && pDL->nAll ){
    sqlite3Fts3DoclistPrev(pTab->bDescIdx, pDL->aAll, pDL->nAll, 
        &pDL->pNextDocid, &pDL->iDocid, &pDL->nList, pbEof
    );
    pDL->pList = pDL->pNextDocid;
  }else{
    fts3EvalDlPhraseNext(pTab, pDL, pbEof);
  }

  return rc;
}

/*
**
** If *pRc is not SQLITE_OK when this function is called, it is a no-op.
** Otherwise, fts3EvalPhraseStart() is called on all phrases within the
** expression. Also the Fts3Expr.bDeferred variable is set to true for any
** expressions for which all descendent tokens are deferred.
**
** If parameter bOptOk is zero, then it is guaranteed that the
** Fts3Phrase.doclist.aAll/nAll variables contain the entire doclist for
** each phrase in the expression (subject to deferred token processing).
** Or, if bOptOk is non-zero, then one or more tokens within the expression
** may be loaded incrementally, meaning doclist.aAll/nAll is not available.
**
** If an error occurs within this function, *pRc is set to an SQLite error
** code before returning.
*/
static void fts3EvalStartReaders(
  Fts3Cursor *pCsr,               /* FTS Cursor handle */
  Fts3Expr *pExpr,                /* Expression to initialize phrases in */
  int *pRc                        /* IN/OUT: Error code */
){
  if( pExpr && SQLITE_OK==*pRc ){
    if( pExpr->eType==FTSQUERY_PHRASE ){
      int nToken = pExpr->pPhrase->nToken;
      if( nToken ){
        int i;
        for(i=0; i<nToken; i++){
          if( pExpr->pPhrase->aToken[i].pDeferred==0 ) break;
        }
        pExpr->bDeferred = (i==nToken);
      }
      *pRc = fts3EvalPhraseStart(pCsr, 1, pExpr->pPhrase);
    }else{
      fts3EvalStartReaders(pCsr, pExpr->pLeft, pRc);
      fts3EvalStartReaders(pCsr, pExpr->pRight, pRc);
      pExpr->bDeferred = (pExpr->pLeft->bDeferred && pExpr->pRight->bDeferred);
    }
  }
}

/*
** An array of the following structures is assembled as part of the process
** of selecting tokens to defer before the query starts executing (as part
** of the xFilter() method). There is one element in the array for each
** token in the FTS expression.
**
** Tokens are divided into AND/NEAR clusters. All tokens in a cluster belong
** to phrases that are connected only by AND and NEAR operators (not OR or
** NOT). When determining tokens to defer, each AND/NEAR cluster is considered
** separately. The root of a tokens AND/NEAR cluster is stored in 
** Fts3TokenAndCost.pRoot.
*/
typedef struct Fts3TokenAndCost Fts3TokenAndCost;
struct Fts3TokenAndCost {
  Fts3Phrase *pPhrase;            /* The phrase the token belongs to */
  int iToken;                     /* Position of token in phrase */
  Fts3PhraseToken *pToken;        /* The token itself */
  Fts3Expr *pRoot;                /* Root of NEAR/AND cluster */
  int nOvfl;                      /* Number of overflow pages to load doclist */
  int iCol;                       /* The column the token must match */
};

/*
** This function is used to populate an allocated Fts3TokenAndCost array.
**
** If *pRc is not SQLITE_OK when this function is called, it is a no-op.
** Otherwise, if an error occurs during execution, *pRc is set to an
** SQLite error code.
*/
static void fts3EvalTokenCosts(
  Fts3Cursor *pCsr,               /* FTS Cursor handle */
  Fts3Expr *pRoot,                /* Root of current AND/NEAR cluster */
  Fts3Expr *pExpr,                /* Expression to consider */
  Fts3TokenAndCost **ppTC,        /* Write new entries to *(*ppTC)++ */
  Fts3Expr ***ppOr,               /* Write new OR root to *(*ppOr)++ */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    if( pExpr->eType==FTSQUERY_PHRASE ){
      Fts3Phrase *pPhrase = pExpr->pPhrase;
      int i;
      for(i=0; *pRc==SQLITE_OK && i<pPhrase->nToken; i++){
        Fts3TokenAndCost *pTC = (*ppTC)++;
        pTC->pPhrase = pPhrase;
        pTC->iToken = i;
        pTC->pRoot = pRoot;
        pTC->pToken = &pPhrase->aToken[i];
        pTC->iCol = pPhrase->iColumn;
        *pRc = sqlite3Fts3MsrOvfl(pCsr, pTC->pToken->pSegcsr, &pTC->nOvfl);
      }
    }else if( pExpr->eType!=FTSQUERY_NOT ){
      assert( pExpr->eType==FTSQUERY_OR
           || pExpr->eType==FTSQUERY_AND
           || pExpr->eType==FTSQUERY_NEAR
      );
      assert( pExpr->pLeft && pExpr->pRight );
      if( pExpr->eType==FTSQUERY_OR ){
        pRoot = pExpr->pLeft;
        **ppOr = pRoot;
        (*ppOr)++;
      }
      fts3EvalTokenCosts(pCsr, pRoot, pExpr->pLeft, ppTC, ppOr, pRc);
      if( pExpr->eType==FTSQUERY_OR ){
        pRoot = pExpr->pRight;
        **ppOr = pRoot;
        (*ppOr)++;
      }
      fts3EvalTokenCosts(pCsr, pRoot, pExpr->pRight, ppTC, ppOr, pRc);
    }
  }
}

/*
** Determine the average document (row) size in pages. If successful,
** write this value to *pnPage and return SQLITE_OK. Otherwise, return
** an SQLite error code.
**
** The average document size in pages is calculated by first calculating 
** determining the average size in bytes, B. If B is less than the amount
** of data that will fit on a single leaf page of an intkey table in
** this database, then the average docsize is 1. Otherwise, it is 1 plus
** the number of overflow pages consumed by a record B bytes in size.
*/
static int fts3EvalAverageDocsize(Fts3Cursor *pCsr, int *pnPage){
  if( pCsr->nRowAvg==0 ){
    /* The average document size, which is required to calculate the cost
    ** of each doclist, has not yet been determined. Read the required 
    ** data from the %_stat table to calculate it.
    **
    ** Entry 0 of the %_stat table is a blob containing (nCol+1) FTS3 
    ** varints, where nCol is the number of columns in the FTS3 table.
    ** The first varint is the number of documents currently stored in
    ** the table. The following nCol varints contain the total amount of
    ** data stored in all rows of each column of the table, from left
    ** to right.
    */
    int rc;
    Fts3Table *p = (Fts3Table*)pCsr->base.pVtab;
    sqlite3_stmt *pStmt;
    sqlite3_int64 nDoc = 0;
    sqlite3_int64 nByte = 0;
    const char *pEnd;
    const char *a;

    rc = sqlite3Fts3SelectDoctotal(p, &pStmt);
    if( rc!=SQLITE_OK ) return rc;
    a = sqlite3_column_blob(pStmt, 0);
    assert( a );

    pEnd = &a[sqlite3_column_bytes(pStmt, 0)];
    a += sqlite3Fts3GetVarint(a, &nDoc);
    while( a<pEnd ){
      a += sqlite3Fts3GetVarint(a, &nByte);
    }
    if( nDoc==0 || nByte==0 ){
      sqlite3_reset(pStmt);
      return FTS_CORRUPT_VTAB;
    }

    pCsr->nDoc = nDoc;
    pCsr->nRowAvg = (int)(((nByte / nDoc) + p->nPgsz) / p->nPgsz);
    assert( pCsr->nRowAvg>0 ); 
    rc = sqlite3_reset(pStmt);
    if( rc!=SQLITE_OK ) return rc;
  }

  *pnPage = pCsr->nRowAvg;
  return SQLITE_OK;
}

/*
** This function is called to select the tokens (if any) that will be 
** deferred. The array aTC[] has already been populated when this is
** called.
**
** This function is called once for each AND/NEAR cluster in the 
** expression. Each invocation determines which tokens to defer within
** the cluster with root node pRoot. See comments above the definition
** of struct Fts3TokenAndCost for more details.
**
** If no error occurs, SQLITE_OK is returned and sqlite3Fts3DeferToken()
** called on each token to defer. Otherwise, an SQLite error code is
** returned.
*/
static int fts3EvalSelectDeferred(
  Fts3Cursor *pCsr,               /* FTS Cursor handle */
  Fts3Expr *pRoot,                /* Consider tokens with this root node */
  Fts3TokenAndCost *aTC,          /* Array of expression tokens and costs */
  int nTC                         /* Number of entries in aTC[] */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int nDocSize = 0;               /* Number of pages per doc loaded */
  int rc = SQLITE_OK;             /* Return code */
  int ii;                         /* Iterator variable for various purposes */
  int nOvfl = 0;                  /* Total overflow pages used by doclists */
  int nToken = 0;                 /* Total number of tokens in cluster */

  int nMinEst = 0;                /* The minimum count for any phrase so far. */
  int nLoad4 = 1;                 /* (Phrases that will be loaded)^4. */

  /* Tokens are never deferred for FTS tables created using the content=xxx
  ** option. The reason being that it is not guaranteed that the content
  ** table actually contains the same data as the index. To prevent this from
  ** causing any problems, the deferred token optimization is completely
  ** disabled for content=xxx tables. */
  if( pTab->zContentTbl ){
    return SQLITE_OK;
  }

  /* Count the tokens in this AND/NEAR cluster. If none of the doclists
  ** associated with the tokens spill onto overflow pages, or if there is
  ** only 1 token, exit early. No tokens to defer in this case. */
  for(ii=0; ii<nTC; ii++){
    if( aTC[ii].pRoot==pRoot ){
      nOvfl += aTC[ii].nOvfl;
      nToken++;
    }
  }
  if( nOvfl==0 || nToken<2 ) return SQLITE_OK;

  /* Obtain the average docsize (in pages). */
  rc = fts3EvalAverageDocsize(pCsr, &nDocSize);
  assert( rc!=SQLITE_OK || nDocSize>0 );


  /* Iterate through all tokens in this AND/NEAR cluster, in ascending order 
  ** of the number of overflow pages that will be loaded by the pager layer 
  ** to retrieve the entire doclist for the token from the full-text index.
  ** Load the doclists for tokens that are either:
  **
  **   a. The cheapest token in the entire query (i.e. the one visited by the
  **      first iteration of this loop), or
  **
  **   b. Part of a multi-token phrase.
  **
  ** After each token doclist is loaded, merge it with the others from the
  ** same phrase and count the number of documents that the merged doclist
  ** contains. Set variable "nMinEst" to the smallest number of documents in 
  ** any phrase doclist for which 1 or more token doclists have been loaded.
  ** Let nOther be the number of other phrases for which it is certain that
  ** one or more tokens will not be deferred.
  **
  ** Then, for each token, defer it if loading the doclist would result in
  ** loading N or more overflow pages into memory, where N is computed as:
  **
  **    (nMinEst + 4^nOther - 1) / (4^nOther)
  */
  for(ii=0; ii<nToken && rc==SQLITE_OK; ii++){
    int iTC;                      /* Used to iterate through aTC[] array. */
    Fts3TokenAndCost *pTC = 0;    /* Set to cheapest remaining token. */

    /* Set pTC to point to the cheapest remaining token. */
    for(iTC=0; iTC<nTC; iTC++){
      if( aTC[iTC].pToken && aTC[iTC].pRoot==pRoot 
       && (!pTC || aTC[iTC].nOvfl<pTC->nOvfl) 
      ){
        pTC = &aTC[iTC];
      }
    }
    assert( pTC );

    if( ii && pTC->nOvfl>=((nMinEst+(nLoad4/4)-1)/(nLoad4/4))*nDocSize ){
      /* The number of overflow pages to load for this (and therefore all
      ** subsequent) tokens is greater than the estimated number of pages 
      ** that will be loaded if all subsequent tokens are deferred.
      */
      Fts3PhraseToken *pToken = pTC->pToken;
      rc = sqlite3Fts3DeferToken(pCsr, pToken, pTC->iCol);
      fts3SegReaderCursorFree(pToken->pSegcsr);
      pToken->pSegcsr = 0;
    }else{
      /* Set nLoad4 to the value of (4^nOther) for the next iteration of the
      ** for-loop. Except, limit the value to 2^24 to prevent it from 
      ** overflowing the 32-bit integer it is stored in. */
      if( ii<12 ) nLoad4 = nLoad4*4;

      if( ii==0 || (pTC->pPhrase->nToken>1 && ii!=nToken-1) ){
        /* Either this is the cheapest token in the entire query, or it is
        ** part of a multi-token phrase. Either way, the entire doclist will
        ** (eventually) be loaded into memory. It may as well be now. */
        Fts3PhraseToken *pToken = pTC->pToken;
        int nList = 0;
        char *pList = 0;
        rc = fts3TermSelect(pTab, pToken, pTC->iCol, &nList, &pList);
        assert( rc==SQLITE_OK || pList==0 );
        if( rc==SQLITE_OK ){
          rc = fts3EvalPhraseMergeToken(
              pTab, pTC->pPhrase, pTC->iToken,pList,nList
          );
        }
        if( rc==SQLITE_OK ){
          int nCount;
          nCount = fts3DoclistCountDocids(
              pTC->pPhrase->doclist.aAll, pTC->pPhrase->doclist.nAll
          );
          if( ii==0 || nCount<nMinEst ) nMinEst = nCount;
        }
      }
    }
    pTC->pToken = 0;
  }

  return rc;
}

/*
** This function is called from within the xFilter method. It initializes
** the full-text query currently stored in pCsr->pExpr. To iterate through
** the results of a query, the caller does:
**
**    fts3EvalStart(pCsr);
**    while( 1 ){
**      fts3EvalNext(pCsr);
**      if( pCsr->bEof ) break;
**      ... return row pCsr->iPrevId to the caller ...
**    }
*/
static int fts3EvalStart(Fts3Cursor *pCsr){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc = SQLITE_OK;
  int nToken = 0;
  int nOr = 0;

  /* Allocate a MultiSegReader for each token in the expression. */
  fts3EvalAllocateReaders(pCsr, pCsr->pExpr, &nToken, &nOr, &rc);

  /* Determine which, if any, tokens in the expression should be deferred. */
#ifndef SQLITE_DISABLE_FTS4_DEFERRED
  if( rc==SQLITE_OK && nToken>1 && pTab->bFts4 ){
    Fts3TokenAndCost *aTC;
    Fts3Expr **apOr;
    aTC = (Fts3TokenAndCost *)sqlite3_malloc(
        sizeof(Fts3TokenAndCost) * nToken
      + sizeof(Fts3Expr *) * nOr * 2
    );
    apOr = (Fts3Expr **)&aTC[nToken];

    if( !aTC ){
      rc = SQLITE_NOMEM;
    }else{
      int ii;
      Fts3TokenAndCost *pTC = aTC;
      Fts3Expr **ppOr = apOr;

      fts3EvalTokenCosts(pCsr, 0, pCsr->pExpr, &pTC, &ppOr, &rc);
      nToken = (int)(pTC-aTC);
      nOr = (int)(ppOr-apOr);

      if( rc==SQLITE_OK ){
        rc = fts3EvalSelectDeferred(pCsr, 0, aTC, nToken);
        for(ii=0; rc==SQLITE_OK && ii<nOr; ii++){
          rc = fts3EvalSelectDeferred(pCsr, apOr[ii], aTC, nToken);
        }
      }

      sqlite3_free(aTC);
    }
  }
#endif

  fts3EvalStartReaders(pCsr, pCsr->pExpr, &rc);
  return rc;
}

/*
** Invalidate the current position list for phrase pPhrase.
*/
static void fts3EvalInvalidatePoslist(Fts3Phrase *pPhrase){
  if( pPhrase->doclist.bFreeList ){
    sqlite3_free(pPhrase->doclist.pList);
  }
  pPhrase->doclist.pList = 0;
  pPhrase->doclist.nList = 0;
  pPhrase->doclist.bFreeList = 0;
}

/*
** This function is called to edit the position list associated with
** the phrase object passed as the fifth argument according to a NEAR
** condition. For example:
**
**     abc NEAR/5 "def ghi"
**
** Parameter nNear is passed the NEAR distance of the expression (5 in
** the example above). When this function is called, *paPoslist points to
** the position list, and *pnToken is the number of phrase tokens in, the
** phrase on the other side of the NEAR operator to pPhrase. For example,
** if pPhrase refers to the "def ghi" phrase, then *paPoslist points to
** the position list associated with phrase "abc".
**
** All positions in the pPhrase position list that are not sufficiently
** close to a position in the *paPoslist position list are removed. If this
** leaves 0 positions, zero is returned. Otherwise, non-zero.
**
** Before returning, *paPoslist is set to point to the position lsit 
** associated with pPhrase. And *pnToken is set to the number of tokens in
** pPhrase.
*/
static int fts3EvalNearTrim(
  int nNear,                      /* NEAR distance. As in "NEAR/nNear". */
  char *aTmp,                     /* Temporary space to use */
  char **paPoslist,               /* IN/OUT: Position list */
  int *pnToken,                   /* IN/OUT: Tokens in phrase of *paPoslist */
  Fts3Phrase *pPhrase             /* The phrase object to trim the doclist of */
){
  int nParam1 = nNear + pPhrase->nToken;
  int nParam2 = nNear + *pnToken;
  int nNew;
  char *p2; 
  char *pOut; 
  int res;

  assert( pPhrase->doclist.pList );

  p2 = pOut = pPhrase->doclist.pList;
  res = fts3PoslistNearMerge(
    &pOut, aTmp, nParam1, nParam2, paPoslist, &p2
  );
  if( res ){
    nNew = (int)(pOut - pPhrase->doclist.pList) - 1;
    assert( pPhrase->doclist.pList[nNew]=='\0' );
    assert( nNew<=pPhrase->doclist.nList && nNew>0 );
    memset(&pPhrase->doclist.pList[nNew], 0, pPhrase->doclist.nList - nNew);
    pPhrase->doclist.nList = nNew;
    *paPoslist = pPhrase->doclist.pList;
    *pnToken = pPhrase->nToken;
  }

  return res;
}

/*
** This function is a no-op if *pRc is other than SQLITE_OK when it is called.
** Otherwise, it advances the expression passed as the second argument to
** point to the next matching row in the database. Expressions iterate through
** matching rows in docid order. Ascending order if Fts3Cursor.bDesc is zero,
** or descending if it is non-zero.
**
** If an error occurs, *pRc is set to an SQLite error code. Otherwise, if
** successful, the following variables in pExpr are set:
**
**   Fts3Expr.bEof                (non-zero if EOF - there is no next row)
**   Fts3Expr.iDocid              (valid if bEof==0. The docid of the next row)
**
** If the expression is of type FTSQUERY_PHRASE, and the expression is not
** at EOF, then the following variables are populated with the position list
** for the phrase for the visited row:
**
**   FTs3Expr.pPhrase->doclist.nList        (length of pList in bytes)
**   FTs3Expr.pPhrase->doclist.pList        (pointer to position list)
**
** It says above that this function advances the expression to the next
** matching row. This is usually true, but there are the following exceptions:
**
**   1. Deferred tokens are not taken into account. If a phrase consists
**      entirely of deferred tokens, it is assumed to match every row in
**      the db. In this case the position-list is not populated at all. 
**
**      Or, if a phrase contains one or more deferred tokens and one or
**      more non-deferred tokens, then the expression is advanced to the 
**      next possible match, considering only non-deferred tokens. In other
**      words, if the phrase is "A B C", and "B" is deferred, the expression
**      is advanced to the next row that contains an instance of "A * C", 
**      where "*" may match any single token. The position list in this case
**      is populated as for "A * C" before returning.
**
**   2. NEAR is treated as AND. If the expression is "x NEAR y", it is 
**      advanced to point to the next row that matches "x AND y".
** 
** See sqlite3Fts3EvalTestDeferred() for details on testing if a row is
** really a match, taking into account deferred tokens and NEAR operators.
*/
static void fts3EvalNextRow(
  Fts3Cursor *pCsr,               /* FTS Cursor handle */
  Fts3Expr *pExpr,                /* Expr. to advance to next matching row */
  int *pRc                        /* IN/OUT: Error code */
){
  if( *pRc==SQLITE_OK ){
    int bDescDoclist = pCsr->bDesc;         /* Used by DOCID_CMP() macro */
    assert( pExpr->bEof==0 );
    pExpr->bStart = 1;

    switch( pExpr->eType ){
      case FTSQUERY_NEAR:
      case FTSQUERY_AND: {
        Fts3Expr *pLeft = pExpr->pLeft;
        Fts3Expr *pRight = pExpr->pRight;
        assert( !pLeft->bDeferred || !pRight->bDeferred );

        if( pLeft->bDeferred ){
          /* LHS is entirely deferred. So we assume it matches every row.
          ** Advance the RHS iterator to find the next row visited. */
          fts3EvalNextRow(pCsr, pRight, pRc);
          pExpr->iDocid = pRight->iDocid;
          pExpr->bEof = pRight->bEof;
        }else if( pRight->bDeferred ){
          /* RHS is entirely deferred. So we assume it matches every row.
          ** Advance the LHS iterator to find the next row visited. */
          fts3EvalNextRow(pCsr, pLeft, pRc);
          pExpr->iDocid = pLeft->iDocid;
          pExpr->bEof = pLeft->bEof;
        }else{
          /* Neither the RHS or LHS are deferred. */
          fts3EvalNextRow(pCsr, pLeft, pRc);
          fts3EvalNextRow(pCsr, pRight, pRc);
          while( !pLeft->bEof && !pRight->bEof && *pRc==SQLITE_OK ){
            sqlite3_int64 iDiff = DOCID_CMP(pLeft->iDocid, pRight->iDocid);
            if( iDiff==0 ) break;
            if( iDiff<0 ){
              fts3EvalNextRow(pCsr, pLeft, pRc);
            }else{
              fts3EvalNextRow(pCsr, pRight, pRc);
            }
          }
          pExpr->iDocid = pLeft->iDocid;
          pExpr->bEof = (pLeft->bEof || pRight->bEof);
          if( pExpr->eType==FTSQUERY_NEAR && pExpr->bEof ){
            if( pRight->pPhrase && pRight->pPhrase->doclist.aAll ){
              Fts3Doclist *pDl = &pRight->pPhrase->doclist;
              while( *pRc==SQLITE_OK && pRight->bEof==0 ){
                memset(pDl->pList, 0, pDl->nList);
                fts3EvalNextRow(pCsr, pRight, pRc);
              }
            }
            if( pLeft->pPhrase && pLeft->pPhrase->doclist.aAll ){
              Fts3Doclist *pDl = &pLeft->pPhrase->doclist;
              while( *pRc==SQLITE_OK && pLeft->bEof==0 ){
                memset(pDl->pList, 0, pDl->nList);
                fts3EvalNextRow(pCsr, pLeft, pRc);
              }
            }
          }
        }
        break;
      }
  
      case FTSQUERY_OR: {
        Fts3Expr *pLeft = pExpr->pLeft;
        Fts3Expr *pRight = pExpr->pRight;
        sqlite3_int64 iCmp = DOCID_CMP(pLeft->iDocid, pRight->iDocid);

        assert( pLeft->bStart || pLeft->iDocid==pRight->iDocid );
        assert( pRight->bStart || pLeft->iDocid==pRight->iDocid );

        if( pRight->bEof || (pLeft->bEof==0 && iCmp<0) ){
          fts3EvalNextRow(pCsr, pLeft, pRc);
        }else if( pLeft->bEof || (pRight->bEof==0 && iCmp>0) ){
          fts3EvalNextRow(pCsr, pRight, pRc);
        }else{
          fts3EvalNextRow(pCsr, pLeft, pRc);
          fts3EvalNextRow(pCsr, pRight, pRc);
        }

        pExpr->bEof = (pLeft->bEof && pRight->bEof);
        iCmp = DOCID_CMP(pLeft->iDocid, pRight->iDocid);
        if( pRight->bEof || (pLeft->bEof==0 &&  iCmp<0) ){
          pExpr->iDocid = pLeft->iDocid;
        }else{
          pExpr->iDocid = pRight->iDocid;
        }

        break;
      }

      case FTSQUERY_NOT: {
        Fts3Expr *pLeft = pExpr->pLeft;
        Fts3Expr *pRight = pExpr->pRight;

        if( pRight->bStart==0 ){
          fts3EvalNextRow(pCsr, pRight, pRc);
          assert( *pRc!=SQLITE_OK || pRight->bStart );
        }

        fts3EvalNextRow(pCsr, pLeft, pRc);
        if( pLeft->bEof==0 ){
          while( !*pRc 
              && !pRight->bEof 
              && DOCID_CMP(pLeft->iDocid, pRight->iDocid)>0 
          ){
            fts3EvalNextRow(pCsr, pRight, pRc);
          }
        }
        pExpr->iDocid = pLeft->iDocid;
        pExpr->bEof = pLeft->bEof;
        break;
      }

      default: {
        Fts3Phrase *pPhrase = pExpr->pPhrase;
        fts3EvalInvalidatePoslist(pPhrase);
        *pRc = fts3EvalPhraseNext(pCsr, pPhrase, &pExpr->bEof);
        pExpr->iDocid = pPhrase->doclist.iDocid;
        break;
      }
    }
  }
}

/*
** If *pRc is not SQLITE_OK, or if pExpr is not the root node of a NEAR
** cluster, then this function returns 1 immediately.
**
** Otherwise, it checks if the current row really does match the NEAR 
** expression, using the data currently stored in the position lists 
** (Fts3Expr->pPhrase.doclist.pList/nList) for each phrase in the expression. 
**
** If the current row is a match, the position list associated with each
** phrase in the NEAR expression is edited in place to contain only those
** phrase instances sufficiently close to their peers to satisfy all NEAR
** constraints. In this case it returns 1. If the NEAR expression does not 
** match the current row, 0 is returned. The position lists may or may not
** be edited if 0 is returned.
*/
static int fts3EvalNearTest(Fts3Expr *pExpr, int *pRc){
  int res = 1;

  /* The following block runs if pExpr is the root of a NEAR query.
  ** For example, the query:
  **
  **         "w" NEAR "x" NEAR "y" NEAR "z"
  **
  ** which is represented in tree form as:
  **
  **                               |
  **                          +--NEAR--+      <-- root of NEAR query
  **                          |        |
  **                     +--NEAR--+   "z"
  **                     |        |
  **                +--NEAR--+   "y"
  **                |        |
  **               "w"      "x"
  **
  ** The right-hand child of a NEAR node is always a phrase. The 
  ** left-hand child may be either a phrase or a NEAR node. There are
  ** no exceptions to this - it's the way the parser in fts3_expr.c works.
  */
  if( *pRc==SQLITE_OK 
   && pExpr->eType==FTSQUERY_NEAR 
   && pExpr->bEof==0
   && (pExpr->pParent==0 || pExpr->pParent->eType!=FTSQUERY_NEAR)
  ){
    Fts3Expr *p; 
    int nTmp = 0;                 /* Bytes of temp space */
    char *aTmp;                   /* Temp space for PoslistNearMerge() */

    /* Allocate temporary working space. */
    for(p=pExpr; p->pLeft; p=p->pLeft){
      nTmp += p->pRight->pPhrase->doclist.nList;
    }
    nTmp += p->pPhrase->doclist.nList;
    if( nTmp==0 ){
      res = 0;
    }else{
      aTmp = sqlite3_malloc(nTmp*2);
      if( !aTmp ){
        *pRc = SQLITE_NOMEM;
        res = 0;
      }else{
        char *aPoslist = p->pPhrase->doclist.pList;
        int nToken = p->pPhrase->nToken;

        for(p=p->pParent;res && p && p->eType==FTSQUERY_NEAR; p=p->pParent){
          Fts3Phrase *pPhrase = p->pRight->pPhrase;
          int nNear = p->nNear;
          res = fts3EvalNearTrim(nNear, aTmp, &aPoslist, &nToken, pPhrase);
        }

        aPoslist = pExpr->pRight->pPhrase->doclist.pList;
        nToken = pExpr->pRight->pPhrase->nToken;
        for(p=pExpr->pLeft; p && res; p=p->pLeft){
          int nNear;
          Fts3Phrase *pPhrase;
          assert( p->pParent && p->pParent->pLeft==p );
          nNear = p->pParent->nNear;
          pPhrase = (
              p->eType==FTSQUERY_NEAR ? p->pRight->pPhrase : p->pPhrase
              );
          res = fts3EvalNearTrim(nNear, aTmp, &aPoslist, &nToken, pPhrase);
        }
      }

      sqlite3_free(aTmp);
    }
  }

  return res;
}

/*
** This function is a helper function for sqlite3Fts3EvalTestDeferred().
** Assuming no error occurs or has occurred, It returns non-zero if the
** expression passed as the second argument matches the row that pCsr 
** currently points to, or zero if it does not.
**
** If *pRc is not SQLITE_OK when this function is called, it is a no-op.
** If an error occurs during execution of this function, *pRc is set to 
** the appropriate SQLite error code. In this case the returned value is 
** undefined.
*/
static int fts3EvalTestExpr(
  Fts3Cursor *pCsr,               /* FTS cursor handle */
  Fts3Expr *pExpr,                /* Expr to test. May or may not be root. */
  int *pRc                        /* IN/OUT: Error code */
){
  int bHit = 1;                   /* Return value */
  if( *pRc==SQLITE_OK ){
    switch( pExpr->eType ){
      case FTSQUERY_NEAR:
      case FTSQUERY_AND:
        bHit = (
            fts3EvalTestExpr(pCsr, pExpr->pLeft, pRc)
         && fts3EvalTestExpr(pCsr, pExpr->pRight, pRc)
         && fts3EvalNearTest(pExpr, pRc)
        );

        /* If the NEAR expression does not match any rows, zero the doclist for 
        ** all phrases involved in the NEAR. This is because the snippet(),
        ** offsets() and matchinfo() functions are not supposed to recognize 
        ** any instances of phrases that are part of unmatched NEAR queries. 
        ** For example if this expression:
        **
        **    ... MATCH 'a OR (b NEAR c)'
        **
        ** is matched against a row containing:
        **
        **        'a b d e'
        **
        ** then any snippet() should ony highlight the "a" term, not the "b"
        ** (as "b" is part of a non-matching NEAR clause).
        */
        if( bHit==0 
         && pExpr->eType==FTSQUERY_NEAR 
         && (pExpr->pParent==0 || pExpr->pParent->eType!=FTSQUERY_NEAR)
        ){
          Fts3Expr *p;
          for(p=pExpr; p->pPhrase==0; p=p->pLeft){
            if( p->pRight->iDocid==pCsr->iPrevId ){
              fts3EvalInvalidatePoslist(p->pRight->pPhrase);
            }
          }
          if( p->iDocid==pCsr->iPrevId ){
            fts3EvalInvalidatePoslist(p->pPhrase);
          }
        }

        break;

      case FTSQUERY_OR: {
        int bHit1 = fts3EvalTestExpr(pCsr, pExpr->pLeft, pRc);
        int bHit2 = fts3EvalTestExpr(pCsr, pExpr->pRight, pRc);
        bHit = bHit1 || bHit2;
        break;
      }

      case FTSQUERY_NOT:
        bHit = (
            fts3EvalTestExpr(pCsr, pExpr->pLeft, pRc)
         && !fts3EvalTestExpr(pCsr, pExpr->pRight, pRc)
        );
        break;

      default: {
#ifndef SQLITE_DISABLE_FTS4_DEFERRED
        if( pCsr->pDeferred 
         && (pExpr->iDocid==pCsr->iPrevId || pExpr->bDeferred)
        ){
          Fts3Phrase *pPhrase = pExpr->pPhrase;
          assert( pExpr->bDeferred || pPhrase->doclist.bFreeList==0 );
          if( pExpr->bDeferred ){
            fts3EvalInvalidatePoslist(pPhrase);
          }
          *pRc = fts3EvalDeferredPhrase(pCsr, pPhrase);
          bHit = (pPhrase->doclist.pList!=0);
          pExpr->iDocid = pCsr->iPrevId;
        }else
#endif
        {
          bHit = (pExpr->bEof==0 && pExpr->iDocid==pCsr->iPrevId);
        }
        break;
      }
    }
  }
  return bHit;
}

/*
** This function is called as the second part of each xNext operation when
** iterating through the results of a full-text query. At this point the
** cursor points to a row that matches the query expression, with the
** following caveats:
**
**   * Up until this point, "NEAR" operators in the expression have been
**     treated as "AND".
**
**   * Deferred tokens have not yet been considered.
**
** If *pRc is not SQLITE_OK when this function is called, it immediately
** returns 0. Otherwise, it tests whether or not after considering NEAR
** operators and deferred tokens the current row is still a match for the
** expression. It returns 1 if both of the following are true:
**
**   1. *pRc is SQLITE_OK when this function returns, and
**
**   2. After scanning the current FTS table row for the deferred tokens,
**      it is determined that the row does *not* match the query.
**
** Or, if no error occurs and it seems the current row does match the FTS
** query, return 0.
*/
SQLITE_PRIVATE int sqlite3Fts3EvalTestDeferred(Fts3Cursor *pCsr, int *pRc){
  int rc = *pRc;
  int bMiss = 0;
  if( rc==SQLITE_OK ){

    /* If there are one or more deferred tokens, load the current row into
    ** memory and scan it to determine the position list for each deferred
    ** token. Then, see if this row is really a match, considering deferred
    ** tokens and NEAR operators (neither of which were taken into account
    ** earlier, by fts3EvalNextRow()). 
    */
    if( pCsr->pDeferred ){
      rc = fts3CursorSeek(0, pCsr);
      if( rc==SQLITE_OK ){
        rc = sqlite3Fts3CacheDeferredDoclists(pCsr);
      }
    }
    bMiss = (0==fts3EvalTestExpr(pCsr, pCsr->pExpr, &rc));

    /* Free the position-lists accumulated for each deferred token above. */
    sqlite3Fts3FreeDeferredDoclists(pCsr);
    *pRc = rc;
  }
  return (rc==SQLITE_OK && bMiss);
}

/*
** Advance to the next document that matches the FTS expression in
** Fts3Cursor.pExpr.
*/
static int fts3EvalNext(Fts3Cursor *pCsr){
  int rc = SQLITE_OK;             /* Return Code */
  Fts3Expr *pExpr = pCsr->pExpr;
  assert( pCsr->isEof==0 );
  if( pExpr==0 ){
    pCsr->isEof = 1;
  }else{
    do {
      if( pCsr->isRequireSeek==0 ){
        sqlite3_reset(pCsr->pStmt);
      }
      assert( sqlite3_data_count(pCsr->pStmt)==0 );
      fts3EvalNextRow(pCsr, pExpr, &rc);
      pCsr->isEof = pExpr->bEof;
      pCsr->isRequireSeek = 1;
      pCsr->isMatchinfoNeeded = 1;
      pCsr->iPrevId = pExpr->iDocid;
    }while( pCsr->isEof==0 && sqlite3Fts3EvalTestDeferred(pCsr, &rc) );
  }

  /* Check if the cursor is past the end of the docid range specified
  ** by Fts3Cursor.iMinDocid/iMaxDocid. If so, set the EOF flag.  */
  if( rc==SQLITE_OK && (
        (pCsr->bDesc==0 && pCsr->iPrevId>pCsr->iMaxDocid)
     || (pCsr->bDesc!=0 && pCsr->iPrevId<pCsr->iMinDocid)
  )){
    pCsr->isEof = 1;
  }

  return rc;
}

/*
** Restart interation for expression pExpr so that the next call to
** fts3EvalNext() visits the first row. Do not allow incremental 
** loading or merging of phrase doclists for this iteration.
**
** If *pRc is other than SQLITE_OK when this function is called, it is
** a no-op. If an error occurs within this function, *pRc is set to an
** SQLite error code before returning.
*/
static void fts3EvalRestart(
  Fts3Cursor *pCsr,
  Fts3Expr *pExpr,
  int *pRc
){
  if( pExpr && *pRc==SQLITE_OK ){
    Fts3Phrase *pPhrase = pExpr->pPhrase;

    if( pPhrase ){
      fts3EvalInvalidatePoslist(pPhrase);
      if( pPhrase->bIncr ){
        int i;
        for(i=0; i<pPhrase->nToken; i++){
          Fts3PhraseToken *pToken = &pPhrase->aToken[i];
          assert( pToken->pDeferred==0 );
          if( pToken->pSegcsr ){
            sqlite3Fts3MsrIncrRestart(pToken->pSegcsr);
          }
        }
        *pRc = fts3EvalPhraseStart(pCsr, 0, pPhrase);
      }
      pPhrase->doclist.pNextDocid = 0;
      pPhrase->doclist.iDocid = 0;
      pPhrase->pOrPoslist = 0;
    }

    pExpr->iDocid = 0;
    pExpr->bEof = 0;
    pExpr->bStart = 0;

    fts3EvalRestart(pCsr, pExpr->pLeft, pRc);
    fts3EvalRestart(pCsr, pExpr->pRight, pRc);
  }
}

/*
** After allocating the Fts3Expr.aMI[] array for each phrase in the 
** expression rooted at pExpr, the cursor iterates through all rows matched
** by pExpr, calling this function for each row. This function increments
** the values in Fts3Expr.aMI[] according to the position-list currently
** found in Fts3Expr.pPhrase->doclist.pList for each of the phrase 
** expression nodes.
*/
static void fts3EvalUpdateCounts(Fts3Expr *pExpr){
  if( pExpr ){
    Fts3Phrase *pPhrase = pExpr->pPhrase;
    if( pPhrase && pPhrase->doclist.pList ){
      int iCol = 0;
      char *p = pPhrase->doclist.pList;

      assert( *p );
      while( 1 ){
        u8 c = 0;
        int iCnt = 0;
        while( 0xFE & (*p | c) ){
          if( (c&0x80)==0 ) iCnt++;
          c = *p++ & 0x80;
        }

        /* aMI[iCol*3 + 1] = Number of occurrences
        ** aMI[iCol*3 + 2] = Number of rows containing at least one instance
        */
        pExpr->aMI[iCol*3 + 1] += iCnt;
        pExpr->aMI[iCol*3 + 2] += (iCnt>0);
        if( *p==0x00 ) break;
        p++;
        p += fts3GetVarint32(p, &iCol);
      }
    }

    fts3EvalUpdateCounts(pExpr->pLeft);
    fts3EvalUpdateCounts(pExpr->pRight);
  }
}

/*
** Expression pExpr must be of type FTSQUERY_PHRASE.
**
** If it is not already allocated and populated, this function allocates and
** populates the Fts3Expr.aMI[] array for expression pExpr. If pExpr is part
** of a NEAR expression, then it also allocates and populates the same array
** for all other phrases that are part of the NEAR expression.
**
** SQLITE_OK is returned if the aMI[] array is successfully allocated and
** populated. Otherwise, if an error occurs, an SQLite error code is returned.
*/
static int fts3EvalGatherStats(
  Fts3Cursor *pCsr,               /* Cursor object */
  Fts3Expr *pExpr                 /* FTSQUERY_PHRASE expression */
){
  int rc = SQLITE_OK;             /* Return code */

  assert( pExpr->eType==FTSQUERY_PHRASE );
  if( pExpr->aMI==0 ){
    Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
    Fts3Expr *pRoot;                /* Root of NEAR expression */
    Fts3Expr *p;                    /* Iterator used for several purposes */

    sqlite3_int64 iPrevId = pCsr->iPrevId;
    sqlite3_int64 iDocid;
    u8 bEof;

    /* Find the root of the NEAR expression */
    pRoot = pExpr;
    while( pRoot->pParent && pRoot->pParent->eType==FTSQUERY_NEAR ){
      pRoot = pRoot->pParent;
    }
    iDocid = pRoot->iDocid;
    bEof = pRoot->bEof;
    assert( pRoot->bStart );

    /* Allocate space for the aMSI[] array of each FTSQUERY_PHRASE node */
    for(p=pRoot; p; p=p->pLeft){
      Fts3Expr *pE = (p->eType==FTSQUERY_PHRASE?p:p->pRight);
      assert( pE->aMI==0 );
      pE->aMI = (u32 *)sqlite3_malloc(pTab->nColumn * 3 * sizeof(u32));
      if( !pE->aMI ) return SQLITE_NOMEM;
      memset(pE->aMI, 0, pTab->nColumn * 3 * sizeof(u32));
    }

    fts3EvalRestart(pCsr, pRoot, &rc);

    while( pCsr->isEof==0 && rc==SQLITE_OK ){

      do {
        /* Ensure the %_content statement is reset. */
        if( pCsr->isRequireSeek==0 ) sqlite3_reset(pCsr->pStmt);
        assert( sqlite3_data_count(pCsr->pStmt)==0 );

        /* Advance to the next document */
        fts3EvalNextRow(pCsr, pRoot, &rc);
        pCsr->isEof = pRoot->bEof;
        pCsr->isRequireSeek = 1;
        pCsr->isMatchinfoNeeded = 1;
        pCsr->iPrevId = pRoot->iDocid;
      }while( pCsr->isEof==0 
           && pRoot->eType==FTSQUERY_NEAR 
           && sqlite3Fts3EvalTestDeferred(pCsr, &rc) 
      );

      if( rc==SQLITE_OK && pCsr->isEof==0 ){
        fts3EvalUpdateCounts(pRoot);
      }
    }

    pCsr->isEof = 0;
    pCsr->iPrevId = iPrevId;

    if( bEof ){
      pRoot->bEof = bEof;
    }else{
      /* Caution: pRoot may iterate through docids in ascending or descending
      ** order. For this reason, even though it seems more defensive, the 
      ** do loop can not be written:
      **
      **   do {...} while( pRoot->iDocid<iDocid && rc==SQLITE_OK );
      */
      fts3EvalRestart(pCsr, pRoot, &rc);
      do {
        fts3EvalNextRow(pCsr, pRoot, &rc);
        assert( pRoot->bEof==0 );
      }while( pRoot->iDocid!=iDocid && rc==SQLITE_OK );
    }
  }
  return rc;
}

/*
** This function is used by the matchinfo() module to query a phrase 
** expression node for the following information:
**
**   1. The total number of occurrences of the phrase in each column of 
**      the FTS table (considering all rows), and
**
**   2. For each column, the number of rows in the table for which the
**      column contains at least one instance of the phrase.
**
** If no error occurs, SQLITE_OK is returned and the values for each column
** written into the array aiOut as follows:
**
**   aiOut[iCol*3 + 1] = Number of occurrences
**   aiOut[iCol*3 + 2] = Number of rows containing at least one instance
**
** Caveats:
**
**   * If a phrase consists entirely of deferred tokens, then all output 
**     values are set to the number of documents in the table. In other
**     words we assume that very common tokens occur exactly once in each 
**     column of each row of the table.
**
**   * If a phrase contains some deferred tokens (and some non-deferred 
**     tokens), count the potential occurrence identified by considering
**     the non-deferred tokens instead of actual phrase occurrences.
**
**   * If the phrase is part of a NEAR expression, then only phrase instances
**     that meet the NEAR constraint are included in the counts.
*/
SQLITE_PRIVATE int sqlite3Fts3EvalPhraseStats(
  Fts3Cursor *pCsr,               /* FTS cursor handle */
  Fts3Expr *pExpr,                /* Phrase expression */
  u32 *aiOut                      /* Array to write results into (see above) */
){
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  int rc = SQLITE_OK;
  int iCol;

  if( pExpr->bDeferred && pExpr->pParent->eType!=FTSQUERY_NEAR ){
    assert( pCsr->nDoc>0 );
    for(iCol=0; iCol<pTab->nColumn; iCol++){
      aiOut[iCol*3 + 1] = (u32)pCsr->nDoc;
      aiOut[iCol*3 + 2] = (u32)pCsr->nDoc;
    }
  }else{
    rc = fts3EvalGatherStats(pCsr, pExpr);
    if( rc==SQLITE_OK ){
      assert( pExpr->aMI );
      for(iCol=0; iCol<pTab->nColumn; iCol++){
        aiOut[iCol*3 + 1] = pExpr->aMI[iCol*3 + 1];
        aiOut[iCol*3 + 2] = pExpr->aMI[iCol*3 + 2];
      }
    }
  }

  return rc;
}

/*
** The expression pExpr passed as the second argument to this function
** must be of type FTSQUERY_PHRASE. 
**
** The returned value is either NULL or a pointer to a buffer containing
** a position-list indicating the occurrences of the phrase in column iCol
** of the current row. 
**
** More specifically, the returned buffer contains 1 varint for each 
** occurrence of the phrase in the column, stored using the normal (delta+2) 
** compression and is terminated by either an 0x01 or 0x00 byte. For example,
** if the requested column contains "a b X c d X X" and the position-list
** for 'X' is requested, the buffer returned may contain:
**
**     0x04 0x05 0x03 0x01   or   0x04 0x05 0x03 0x00
**
** This function works regardless of whether or not the phrase is deferred,
** incremental, or neither.
*/
SQLITE_PRIVATE int sqlite3Fts3EvalPhrasePoslist(
  Fts3Cursor *pCsr,               /* FTS3 cursor object */
  Fts3Expr *pExpr,                /* Phrase to return doclist for */
  int iCol,                       /* Column to return position list for */
  char **ppOut                    /* OUT: Pointer to position list */
){
  Fts3Phrase *pPhrase = pExpr->pPhrase;
  Fts3Table *pTab = (Fts3Table *)pCsr->base.pVtab;
  char *pIter;
  int iThis;
  sqlite3_int64 iDocid;

  /* If this phrase is applies specifically to some column other than 
  ** column iCol, return a NULL pointer.  */
  *ppOut = 0;
  assert( iCol>=0 && iCol<pTab->nColumn );
  if( (pPhrase->iColumn<pTab->nColumn && pPhrase->iColumn!=iCol) ){
    return SQLITE_OK;
  }

  iDocid = pExpr->iDocid;
  pIter = pPhrase->doclist.pList;
  if( iDocid!=pCsr->iPrevId || pExpr->bEof ){
    int rc = SQLITE_OK;
    int bDescDoclist = pTab->bDescIdx;      /* For DOCID_CMP macro */
    int bOr = 0;
    u8 bTreeEof = 0;
    Fts3Expr *p;                  /* Used to iterate from pExpr to root */
    Fts3Expr *pNear;              /* Most senior NEAR ancestor (or pExpr) */
    int bMatch;

    /* Check if this phrase descends from an OR expression node. If not, 
    ** return NULL. Otherwise, the entry that corresponds to docid 
    ** pCsr->iPrevId may lie earlier in the doclist buffer. Or, if the
    ** tree that the node is part of has been marked as EOF, but the node
    ** itself is not EOF, then it may point to an earlier entry. */
    pNear = pExpr;
    for(p=pExpr->pParent; p; p=p->pParent){
      if( p->eType==FTSQUERY_OR ) bOr = 1;
      if( p->eType==FTSQUERY_NEAR ) pNear = p;
      if( p->bEof ) bTreeEof = 1;
    }
    if( bOr==0 ) return SQLITE_OK;

    /* This is the descendent of an OR node. In this case we cannot use
    ** an incremental phrase. Load the entire doclist for the phrase
    ** into memory in this case.  */
    if( pPhrase->bIncr ){
      int bEofSave = pNear->bEof;
      fts3EvalRestart(pCsr, pNear, &rc);
      while( rc==SQLITE_OK && !pNear->bEof ){
        fts3EvalNextRow(pCsr, pNear, &rc);
        if( bEofSave==0 && pNear->iDocid==iDocid ) break;
      }
      assert( rc!=SQLITE_OK || pPhrase->bIncr==0 );
    }
    if( bTreeEof ){
      while( rc==SQLITE_OK && !pNear->bEof ){
        fts3EvalNextRow(pCsr, pNear, &rc);
      }
    }
    if( rc!=SQLITE_OK ) return rc;

    bMatch = 1;
    for(p=pNear; p; p=p->pLeft){
      u8 bEof = 0;
      Fts3Expr *pTest = p;
      Fts3Phrase *pPh;
      assert( pTest->eType==FTSQUERY_NEAR || pTest->eType==FTSQUERY_PHRASE );
      if( pTest->eType==FTSQUERY_NEAR ) pTest = pTest->pRight;
      assert( pTest->eType==FTSQUERY_PHRASE );
      pPh = pTest->pPhrase;

      pIter = pPh->pOrPoslist;
      iDocid = pPh->iOrDocid;
      if( pCsr->bDesc==bDescDoclist ){
        bEof = !pPh->doclist.nAll ||
          (pIter >= (pPh->doclist.aAll + pPh->doclist.nAll));
        while( (pIter==0 || DOCID_CMP(iDocid, pCsr->iPrevId)<0 ) && bEof==0 ){
          sqlite3Fts3DoclistNext(
              bDescDoclist, pPh->doclist.aAll, pPh->doclist.nAll, 
              &pIter, &iDocid, &bEof
          );
        }
      }else{
        bEof = !pPh->doclist.nAll || (pIter && pIter<=pPh->doclist.aAll);
        while( (pIter==0 || DOCID_CMP(iDocid, pCsr->iPrevId)>0 ) && bEof==0 ){
          int dummy;
          sqlite3Fts3DoclistPrev(
              bDescDoclist, pPh->doclist.aAll, pPh->doclist.nAll, 
              &pIter, &iDocid, &dummy, &bEof
              );
        }
      }
      pPh->pOrPoslist = pIter;
      pPh->iOrDocid = iDocid;
      if( bEof || iDocid!=pCsr->iPrevId ) bMatch = 0;
    }

    if( bMatch ){
      pIter = pPhrase->pOrPoslist;
    }else{
      pIter = 0;
    }
  }
  if( pIter==0 ) return SQLITE_OK;

  if( *pIter==0x01 ){
    pIter++;
    pIter += fts3GetVarint32(pIter, &iThis);
  }else{
    iThis = 0;
  }
  while( iThis<iCol ){
    fts3ColumnlistCopy(0, &pIter);
    if( *pIter==0x00 ) return SQLITE_OK;
    pIter++;
    pIter += fts3GetVarint32(pIter, &iThis);
  }
  if( *pIter==0x00 ){
    pIter = 0;
  }

  *ppOut = ((iCol==iThis)?pIter:0);
  return SQLITE_OK;
}

/*
** Free all components of the Fts3Phrase structure that were allocated by
** the eval module. Specifically, this means to free:
**
**   * the contents of pPhrase->doclist, and
**   * any Fts3MultiSegReader objects held by phrase tokens.
*/
SQLITE_PRIVATE void sqlite3Fts3EvalPhraseCleanup(Fts3Phrase *pPhrase){
  if( pPhrase ){
    int i;
    sqlite3_free(pPhrase->doclist.aAll);
    fts3EvalInvalidatePoslist(pPhrase);
    memset(&pPhrase->doclist, 0, sizeof(Fts3Doclist));
    for(i=0; i<pPhrase->nToken; i++){
      fts3SegReaderCursorFree(pPhrase->aToken[i].pSegcsr);
      pPhrase->aToken[i].pSegcsr = 0;
    }
  }
}


/*
** Return SQLITE_CORRUPT_VTAB.
*/
#ifdef SQLITE_DEBUG
SQLITE_PRIVATE int sqlite3Fts3Corrupt(){
  return SQLITE_CORRUPT_VTAB;
}
#endif

#if !SQLITE_CORE
/*
** Initialize API pointer table, if required.
*/
#ifdef _WIN32
__declspec(dllexport)
#endif
SQLITE_API int SQLITE_STDCALL sqlite3_fts3_init(
  sqlite3 *db, 
  char **pzErrMsg,
  const sqlite3_api_routines *pApi
){
  SQLITE_EXTENSION_INIT2(pApi)
  return sqlite3Fts3Init(db);
}
#endif

#endif

/************** End of fts3.c ************************************************/
/************** Begin file fts3_aux.c ****************************************/
/*
** 2011 Jan 27
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <string.h> */
/* #include <assert.h> */

typedef struct Fts3auxTable Fts3auxTable;
typedef struct Fts3auxCursor Fts3auxCursor;

struct Fts3auxTable {
  sqlite3_vtab base;              /* Base class used by SQLite core */
  Fts3Table *pFts3Tab;
};

struct Fts3auxCursor {
  sqlite3_vtab_cursor base;       /* Base class used by SQLite core */
  Fts3MultiSegReader csr;        /* Must be right after "base" */
  Fts3SegFilter filter;
  char *zStop;
  int nStop;                      /* Byte-length of string zStop */
  int iLangid;                    /* Language id to query */
  int isEof;                      /* True if cursor is at EOF */
  sqlite3_int64 iRowid;           /* Current rowid */

  int iCol;                       /* Current value of 'col' column */
  int nStat;                      /* Size of aStat[] array */
  struct Fts3auxColstats {
    sqlite3_int64 nDoc;           /* 'documents' values for current csr row */
    sqlite3_int64 nOcc;           /* 'occurrences' values for current csr row */
  } *aStat;
};

/*
** Schema of the terms table.
*/
#define FTS3_AUX_SCHEMA \
  "CREATE TABLE x(term, col, documents, occurrences, languageid HIDDEN)"

/*
** This function does all the work for both the xConnect and xCreate methods.
** These tables have no persistent representation of their own, so xConnect
** and xCreate are identical operations.
*/
static int fts3auxConnectMethod(
  sqlite3 *db,                    /* Database connection */
  void *pUnused,                  /* Unused */
  int argc,                       /* Number of elements in argv array */
  const char * const *argv,       /* xCreate/xConnect argument array */
  sqlite3_vtab **ppVtab,          /* OUT: New sqlite3_vtab object */
  char **pzErr                    /* OUT: sqlite3_malloc'd error message */
){
  char const *zDb;                /* Name of database (e.g. "main") */
  char const *zFts3;              /* Name of fts3 table */
  int nDb;                        /* Result of strlen(zDb) */
  int nFts3;                      /* Result of strlen(zFts3) */
  int nByte;                      /* Bytes of space to allocate here */
  int rc;                         /* value returned by declare_vtab() */
  Fts3auxTable *p;                /* Virtual table object to return */

  UNUSED_PARAMETER(pUnused);

  /* The user should invoke this in one of two forms:
  **
  **     CREATE VIRTUAL TABLE xxx USING fts4aux(fts4-table);
  **     CREATE VIRTUAL TABLE xxx USING fts4aux(fts4-table-db, fts4-table);
  */
  if( argc!=4 && argc!=5 ) goto bad_args;

  zDb = argv[1]; 
  nDb = (int)strlen(zDb);
  if( argc==5 ){
    if( nDb==4 && 0==sqlite3_strnicmp("temp", zDb, 4) ){
      zDb = argv[3]; 
      nDb = (int)strlen(zDb);
      zFts3 = argv[4];
    }else{
      goto bad_args;
    }
  }else{
    zFts3 = argv[3];
  }
  nFts3 = (int)strlen(zFts3);

  rc = sqlite3_declare_vtab(db, FTS3_AUX_SCHEMA);
  if( rc!=SQLITE_OK ) return rc;

  nByte = sizeof(Fts3auxTable) + sizeof(Fts3Table) + nDb + nFts3 + 2;
  p = (Fts3auxTable *)sqlite3_malloc(nByte);
  if( !p ) return SQLITE_NOMEM;
  memset(p, 0, nByte);

  p->pFts3Tab = (Fts3Table *)&p[1];
  p->pFts3Tab->zDb = (char *)&p->pFts3Tab[1];
  p->pFts3Tab->zName = &p->pFts3Tab->zDb[nDb+1];
  p->pFts3Tab->db = db;
  p->pFts3Tab->nIndex = 1;

  memcpy((char *)p->pFts3Tab->zDb, zDb, nDb);
  memcpy((char *)p->pFts3Tab->zName, zFts3, nFts3);
  sqlite3Fts3Dequote((char *)p->pFts3Tab->zName);

  *ppVtab = (sqlite3_vtab *)p;
  return SQLITE_OK;

 bad_args:
  sqlite3Fts3ErrMsg(pzErr, "invalid arguments to fts4aux constructor");
  return SQLITE_ERROR;
}

/*
** This function does the work for both the xDisconnect and xDestroy methods.
** These tables have no persistent representation of their own, so xDisconnect
** and xDestroy are identical operations.
*/
static int fts3auxDisconnectMethod(sqlite3_vtab *pVtab){
  Fts3auxTable *p = (Fts3auxTable *)pVtab;
  Fts3Table *pFts3 = p->pFts3Tab;
  int i;

  /* Free any prepared statements held */
  for(i=0; i<SizeofArray(pFts3->aStmt); i++){
    sqlite3_finalize(pFts3->aStmt[i]);
  }
  sqlite3_free(pFts3->zSegmentsTbl);
  sqlite3_free(p);
  return SQLITE_OK;
}

#define FTS4AUX_EQ_CONSTRAINT 1
#define FTS4AUX_GE_CONSTRAINT 2
#define FTS4AUX_LE_CONSTRAINT 4

/*
** xBestIndex - Analyze a WHERE and ORDER BY clause.
*/
static int fts3auxBestIndexMethod(
  sqlite3_vtab *pVTab, 
  sqlite3_index_info *pInfo
){
  int i;
  int iEq = -1;
  int iGe = -1;
  int iLe = -1;
  int iLangid = -1;
  int iNext = 1;                  /* Next free argvIndex value */

  UNUSED_PARAMETER(pVTab);

  /* This vtab delivers always results in "ORDER BY term ASC" order. */
  if( pInfo->nOrderBy==1 
   && pInfo->aOrderBy[0].iColumn==0 
   && pInfo->aOrderBy[0].desc==0
  ){
    pInfo->orderByConsumed = 1;
  }

  /* Search for equality and range constraints on the "term" column. 
  ** And equality constraints on the hidden "languageid" column. */
  for(i=0; i<pInfo->nConstraint; i++){
    if( pInfo->aConstraint[i].usable ){
      int op = pInfo->aConstraint[i].op;
      int iCol = pInfo->aConstraint[i].iColumn;

      if( iCol==0 ){
        if( op==SQLITE_INDEX_CONSTRAINT_EQ ) iEq = i;
        if( op==SQLITE_INDEX_CONSTRAINT_LT ) iLe = i;
        if( op==SQLITE_INDEX_CONSTRAINT_LE ) iLe = i;
        if( op==SQLITE_INDEX_CONSTRAINT_GT ) iGe = i;
        if( op==SQLITE_INDEX_CONSTRAINT_GE ) iGe = i;
      }
      if( iCol==4 ){
        if( op==SQLITE_INDEX_CONSTRAINT_EQ ) iLangid = i;
      }
    }
  }

  if( iEq>=0 ){
    pInfo->idxNum = FTS4AUX_EQ_CONSTRAINT;
    pInfo->aConstraintUsage[iEq].argvIndex = iNext++;
    pInfo->estimatedCost = 5;
  }else{
    pInfo->idxNum = 0;
    pInfo->estimatedCost = 20000;
    if( iGe>=0 ){
      pInfo->idxNum += FTS4AUX_GE_CONSTRAINT;
      pInfo->aConstraintUsage[iGe].argvIndex = iNext++;
      pInfo->estimatedCost /= 2;
    }
    if( iLe>=0 ){
      pInfo->idxNum += FTS4AUX_LE_CONSTRAINT;
      pInfo->aConstraintUsage[iLe].argvIndex = iNext++;
      pInfo->estimatedCost /= 2;
    }
  }
  if( iLangid>=0 ){
    pInfo->aConstraintUsage[iLangid].argvIndex = iNext++;
    pInfo->estimatedCost--;
  }

  return SQLITE_OK;
}

/*
** xOpen - Open a cursor.
*/
static int fts3auxOpenMethod(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCsr){
  Fts3auxCursor *pCsr;            /* Pointer to cursor object to return */

  UNUSED_PARAMETER(pVTab);

  pCsr = (Fts3auxCursor *)sqlite3_malloc(sizeof(Fts3auxCursor));
  if( !pCsr ) return SQLITE_NOMEM;
  memset(pCsr, 0, sizeof(Fts3auxCursor));

  *ppCsr = (sqlite3_vtab_cursor *)pCsr;
  return SQLITE_OK;
}

/*
** xClose - Close a cursor.
*/
static int fts3auxCloseMethod(sqlite3_vtab_cursor *pCursor){
  Fts3Table *pFts3 = ((Fts3auxTable *)pCursor->pVtab)->pFts3Tab;
  Fts3auxCursor *pCsr = (Fts3auxCursor *)pCursor;

  sqlite3Fts3SegmentsClose(pFts3);
  sqlite3Fts3SegReaderFinish(&pCsr->csr);
  sqlite3_free((void *)pCsr->filter.zTerm);
  sqlite3_free(pCsr->zStop);
  sqlite3_free(pCsr->aStat);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

static int fts3auxGrowStatArray(Fts3auxCursor *pCsr, int nSize){
  if( nSize>pCsr->nStat ){
    struct Fts3auxColstats *aNew;
    aNew = (struct Fts3auxColstats *)sqlite3_realloc(pCsr->aStat, 
        sizeof(struct Fts3auxColstats) * nSize
    );
    if( aNew==0 ) return SQLITE_NOMEM;
    memset(&aNew[pCsr->nStat], 0, 
        sizeof(struct Fts3auxColstats) * (nSize - pCsr->nStat)
    );
    pCsr->aStat = aNew;
    pCsr->nStat = nSize;
  }
  return SQLITE_OK;
}

/*
** xNext - Advance the cursor to the next row, if any.
*/
static int fts3auxNextMethod(sqlite3_vtab_cursor *pCursor){
  Fts3auxCursor *pCsr = (Fts3auxCursor *)pCursor;
  Fts3Table *pFts3 = ((Fts3auxTable *)pCursor->pVtab)->pFts3Tab;
  int rc;

  /* Increment our pretend rowid value. */
  pCsr->iRowid++;

  for(pCsr->iCol++; pCsr->iCol<pCsr->nStat; pCsr->iCol++){
    if( pCsr->aStat[pCsr->iCol].nDoc>0 ) return SQLITE_OK;
  }

  rc = sqlite3Fts3SegReaderStep(pFts3, &pCsr->csr);
  if( rc==SQLITE_ROW ){
    int i = 0;
    int nDoclist = pCsr->csr.nDoclist;
    char *aDoclist = pCsr->csr.aDoclist;
    int iCol;

    int eState = 0;

    if( pCsr->zStop ){
      int n = (pCsr->nStop<pCsr->csr.nTerm) ? pCsr->nStop : pCsr->csr.nTerm;
      int mc = memcmp(pCsr->zStop, pCsr->csr.zTerm, n);
      if( mc<0 || (mc==0 && pCsr->csr.nTerm>pCsr->nStop) ){
        pCsr->isEof = 1;
        return SQLITE_OK;
      }
    }

    if( fts3auxGrowStatArray(pCsr, 2) ) return SQLITE_NOMEM;
    memset(pCsr->aStat, 0, sizeof(struct Fts3auxColstats) * pCsr->nStat);
    iCol = 0;

    while( i<nDoclist ){
      sqlite3_int64 v = 0;

      i += sqlite3Fts3GetVarint(&aDoclist[i], &v);
      switch( eState ){
        /* State 0. In this state the integer just read was a docid. */
        case 0:
          pCsr->aStat[0].nDoc++;
          eState = 1;
          iCol = 0;
          break;

        /* State 1. In this state we are expecting either a 1, indicating
        ** that the following integer will be a column number, or the
        ** start of a position list for column 0.  
        ** 
        ** The only difference between state 1 and state 2 is that if the
        ** integer encountered in state 1 is not 0 or 1, then we need to
        ** increment the column 0 "nDoc" count for this term.
        */
        case 1:
          assert( iCol==0 );
          if( v>1 ){
            pCsr->aStat[1].nDoc++;
          }
          eState = 2;
          /* fall through */

        case 2:
          if( v==0 ){       /* 0x00. Next integer will be a docid. */
            eState = 0;
          }else if( v==1 ){ /* 0x01. Next integer will be a column number. */
            eState = 3;
          }else{            /* 2 or greater. A position. */
            pCsr->aStat[iCol+1].nOcc++;
            pCsr->aStat[0].nOcc++;
          }
          break;

        /* State 3. The integer just read is a column number. */
        default: assert( eState==3 );
          iCol = (int)v;
          if( fts3auxGrowStatArray(pCsr, iCol+2) ) return SQLITE_NOMEM;
          pCsr->aStat[iCol+1].nDoc++;
          eState = 2;
          break;
      }
    }

    pCsr->iCol = 0;
    rc = SQLITE_OK;
  }else{
    pCsr->isEof = 1;
  }
  return rc;
}

/*
** xFilter - Initialize a cursor to point at the start of its data.
*/
static int fts3auxFilterMethod(
  sqlite3_vtab_cursor *pCursor,   /* The cursor used for this query */
  int idxNum,                     /* Strategy index */
  const char *idxStr,             /* Unused */
  int nVal,                       /* Number of elements in apVal */
  sqlite3_value **apVal           /* Arguments for the indexing scheme */
){
  Fts3auxCursor *pCsr = (Fts3auxCursor *)pCursor;
  Fts3Table *pFts3 = ((Fts3auxTable *)pCursor->pVtab)->pFts3Tab;
  int rc;
  int isScan = 0;
  int iLangVal = 0;               /* Language id to query */

  int iEq = -1;                   /* Index of term=? value in apVal */
  int iGe = -1;                   /* Index of term>=? value in apVal */
  int iLe = -1;                   /* Index of term<=? value in apVal */
  int iLangid = -1;               /* Index of languageid=? value in apVal */
  int iNext = 0;

  UNUSED_PARAMETER(nVal);
  UNUSED_PARAMETER(idxStr);

  assert( idxStr==0 );
  assert( idxNum==FTS4AUX_EQ_CONSTRAINT || idxNum==0
       || idxNum==FTS4AUX_LE_CONSTRAINT || idxNum==FTS4AUX_GE_CONSTRAINT
       || idxNum==(FTS4AUX_LE_CONSTRAINT|FTS4AUX_GE_CONSTRAINT)
  );

  if( idxNum==FTS4AUX_EQ_CONSTRAINT ){
    iEq = iNext++;
  }else{
    isScan = 1;
    if( idxNum & FTS4AUX_GE_CONSTRAINT ){
      iGe = iNext++;
    }
    if( idxNum & FTS4AUX_LE_CONSTRAINT ){
      iLe = iNext++;
    }
  }
  if( iNext<nVal ){
    iLangid = iNext++;
  }

  /* In case this cursor is being reused, close and zero it. */
  testcase(pCsr->filter.zTerm);
  sqlite3Fts3SegReaderFinish(&pCsr->csr);
  sqlite3_free((void *)pCsr->filter.zTerm);
  sqlite3_free(pCsr->aStat);
  memset(&pCsr->csr, 0, ((u8*)&pCsr[1]) - (u8*)&pCsr->csr);

  pCsr->filter.flags = FTS3_SEGMENT_REQUIRE_POS|FTS3_SEGMENT_IGNORE_EMPTY;
  if( isScan ) pCsr->filter.flags |= FTS3_SEGMENT_SCAN;

  if( iEq>=0 || iGe>=0 ){
    const unsigned char *zStr = sqlite3_value_text(apVal[0]);
    assert( (iEq==0 && iGe==-1) || (iEq==-1 && iGe==0) );
    if( zStr ){
      pCsr->filter.zTerm = sqlite3_mprintf("%s", zStr);
      pCsr->filter.nTerm = sqlite3_value_bytes(apVal[0]);
      if( pCsr->filter.zTerm==0 ) return SQLITE_NOMEM;
    }
  }

  if( iLe>=0 ){
    pCsr->zStop = sqlite3_mprintf("%s", sqlite3_value_text(apVal[iLe]));
    pCsr->nStop = sqlite3_value_bytes(apVal[iLe]);
    if( pCsr->zStop==0 ) return SQLITE_NOMEM;
  }
  
  if( iLangid>=0 ){
    iLangVal = sqlite3_value_int(apVal[iLangid]);

    /* If the user specified a negative value for the languageid, use zero
    ** instead. This works, as the "languageid=?" constraint will also
    ** be tested by the VDBE layer. The test will always be false (since
    ** this module will not return a row with a negative languageid), and
    ** so the overall query will return zero rows.  */
    if( iLangVal<0 ) iLangVal = 0;
  }
  pCsr->iLangid = iLangVal;

  rc = sqlite3Fts3SegReaderCursor(pFts3, iLangVal, 0, FTS3_SEGCURSOR_ALL,
      pCsr->filter.zTerm, pCsr->filter.nTerm, 0, isScan, &pCsr->csr
  );
  if( rc==SQLITE_OK ){
    rc = sqlite3Fts3SegReaderStart(pFts3, &pCsr->csr, &pCsr->filter);
  }

  if( rc==SQLITE_OK ) rc = fts3auxNextMethod(pCursor);
  return rc;
}

/*
** xEof - Return true if the cursor is at EOF, or false otherwise.
*/
static int fts3auxEofMethod(sqlite3_vtab_cursor *pCursor){
  Fts3auxCursor *pCsr = (Fts3auxCursor *)pCursor;
  return pCsr->isEof;
}

/*
** xColumn - Return a column value.
*/
static int fts3auxColumnMethod(
  sqlite3_vtab_cursor *pCursor,   /* Cursor to retrieve value from */
  sqlite3_context *pCtx,          /* Context for sqlite3_result_xxx() calls */
  int iCol                        /* Index of column to read value from */
){
  Fts3auxCursor *p = (Fts3auxCursor *)pCursor;

  assert( p->isEof==0 );
  switch( iCol ){
    case 0: /* term */
      sqlite3_result_text(pCtx, p->csr.zTerm, p->csr.nTerm, SQLITE_TRANSIENT);
      break;

    case 1: /* col */
      if( p->iCol ){
        sqlite3_result_int(pCtx, p->iCol-1);
      }else{
        sqlite3_result_text(pCtx, "*", -1, SQLITE_STATIC);
      }
      break;

    case 2: /* documents */
      sqlite3_result_int64(pCtx, p->aStat[p->iCol].nDoc);
      break;

    case 3: /* occurrences */
      sqlite3_result_int64(pCtx, p->aStat[p->iCol].nOcc);
      break;

    default: /* languageid */
      assert( iCol==4 );
      sqlite3_result_int(pCtx, p->iLangid);
      break;
  }

  return SQLITE_OK;
}

/*
** xRowid - Return the current rowid for the cursor.
*/
static int fts3auxRowidMethod(
  sqlite3_vtab_cursor *pCursor,   /* Cursor to retrieve value from */
  sqlite_int64 *pRowid            /* OUT: Rowid value */
){
  Fts3auxCursor *pCsr = (Fts3auxCursor *)pCursor;
  *pRowid = pCsr->iRowid;
  return SQLITE_OK;
}

/*
** Register the fts3aux module with database connection db. Return SQLITE_OK
** if successful or an error code if sqlite3_create_module() fails.
*/
SQLITE_PRIVATE int sqlite3Fts3InitAux(sqlite3 *db){
  static const sqlite3_module fts3aux_module = {
     0,                           /* iVersion      */
     fts3auxConnectMethod,        /* xCreate       */
     fts3auxConnectMethod,        /* xConnect      */
     fts3auxBestIndexMethod,      /* xBestIndex    */
     fts3auxDisconnectMethod,     /* xDisconnect   */
     fts3auxDisconnectMethod,     /* xDestroy      */
     fts3auxOpenMethod,           /* xOpen         */
     fts3auxCloseMethod,          /* xClose        */
     fts3auxFilterMethod,         /* xFilter       */
     fts3auxNextMethod,           /* xNext         */
     fts3auxEofMethod,            /* xEof          */
     fts3auxColumnMethod,         /* xColumn       */
     fts3auxRowidMethod,          /* xRowid        */
     0,                           /* xUpdate       */
     0,                           /* xBegin        */
     0,                           /* xSync         */
     0,                           /* xCommit       */
     0,                           /* xRollback     */
     0,                           /* xFindFunction */
     0,                           /* xRename       */
     0,                           /* xSavepoint    */
     0,                           /* xRelease      */
     0                            /* xRollbackTo   */
  };
  int rc;                         /* Return code */

  rc = sqlite3_create_module(db, "fts4aux", &fts3aux_module, 0);
  return rc;
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_aux.c ********************************************/
/************** Begin file fts3_expr.c ***************************************/
/*
** 2008 Nov 28
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This module contains code that implements a parser for fts3 query strings
** (the right-hand argument to the MATCH operator). Because the supported 
** syntax is relatively simple, the whole tokenizer/parser system is
** hand-coded. 
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/*
** By default, this module parses the legacy syntax that has been 
** traditionally used by fts3. Or, if SQLITE_ENABLE_FTS3_PARENTHESIS
** is defined, then it uses the new syntax. The differences between
** the new and the old syntaxes are:
**
**  a) The new syntax supports parenthesis. The old does not.
**
**  b) The new syntax supports the AND and NOT operators. The old does not.
**
**  c) The old syntax supports the "-" token qualifier. This is not 
**     supported by the new syntax (it is replaced by the NOT operator).
**
**  d) When using the old syntax, the OR operator has a greater precedence
**     than an implicit AND. When using the new, both implicity and explicit
**     AND operators have a higher precedence than OR.
**
** If compiled with SQLITE_TEST defined, then this module exports the
** symbol "int sqlite3_fts3_enable_parentheses". Setting this variable
** to zero causes the module to use the old syntax. If it is set to 
** non-zero the new syntax is activated. This is so both syntaxes can
** be tested using a single build of testfixture.
**
** The following describes the syntax supported by the fts3 MATCH
** operator in a similar format to that used by the lemon parser
** generator. This module does not use actually lemon, it uses a
** custom parser.
**
**   query ::= andexpr (OR andexpr)*.
**
**   andexpr ::= notexpr (AND? notexpr)*.
**
**   notexpr ::= nearexpr (NOT nearexpr|-TOKEN)*.
**   notexpr ::= LP query RP.
**
**   nearexpr ::= phrase (NEAR distance_opt nearexpr)*.
**
**   distance_opt ::= .
**   distance_opt ::= / INTEGER.
**
**   phrase ::= TOKEN.
**   phrase ::= COLUMN:TOKEN.
**   phrase ::= "TOKEN TOKEN TOKEN...".
*/

#ifdef SQLITE_TEST
SQLITE_API int sqlite3_fts3_enable_parentheses = 0;
#else
# ifdef SQLITE_ENABLE_FTS3_PARENTHESIS 
#  define sqlite3_fts3_enable_parentheses 1
# else
#  define sqlite3_fts3_enable_parentheses 0
# endif
#endif

/*
** Default span for NEAR operators.
*/
#define SQLITE_FTS3_DEFAULT_NEAR_PARAM 10

/* #include <string.h> */
/* #include <assert.h> */

/*
** isNot:
**   This variable is used by function getNextNode(). When getNextNode() is
**   called, it sets ParseContext.isNot to true if the 'next node' is a 
**   FTSQUERY_PHRASE with a unary "-" attached to it. i.e. "mysql" in the
**   FTS3 query "sqlite -mysql". Otherwise, ParseContext.isNot is set to
**   zero.
*/
typedef struct ParseContext ParseContext;
struct ParseContext {
  sqlite3_tokenizer *pTokenizer;      /* Tokenizer module */
  int iLangid;                        /* Language id used with tokenizer */
  const char **azCol;                 /* Array of column names for fts3 table */
  int bFts4;                          /* True to allow FTS4-only syntax */
  int nCol;                           /* Number of entries in azCol[] */
  int iDefaultCol;                    /* Default column to query */
  int isNot;                          /* True if getNextNode() sees a unary - */
  sqlite3_context *pCtx;              /* Write error message here */
  int nNest;                          /* Number of nested brackets */
};

/*
** This function is equivalent to the standard isspace() function. 
**
** The standard isspace() can be awkward to use safely, because although it
** is defined to accept an argument of type int, its behavior when passed
** an integer that falls outside of the range of the unsigned char type
** is undefined (and sometimes, "undefined" means segfault). This wrapper
** is defined to accept an argument of type char, and always returns 0 for
** any values that fall outside of the range of the unsigned char type (i.e.
** negative values).
*/
static int fts3isspace(char c){
  return c==' ' || c=='\t' || c=='\n' || c=='\r' || c=='\v' || c=='\f';
}

/*
** Allocate nByte bytes of memory using sqlite3_malloc(). If successful,
** zero the memory before returning a pointer to it. If unsuccessful, 
** return NULL.
*/
static void *fts3MallocZero(int nByte){
  void *pRet = sqlite3_malloc(nByte);
  if( pRet ) memset(pRet, 0, nByte);
  return pRet;
}

SQLITE_PRIVATE int sqlite3Fts3OpenTokenizer(
  sqlite3_tokenizer *pTokenizer,
  int iLangid,
  const char *z,
  int n,
  sqlite3_tokenizer_cursor **ppCsr
){
  sqlite3_tokenizer_module const *pModule = pTokenizer->pModule;
  sqlite3_tokenizer_cursor *pCsr = 0;
  int rc;

  rc = pModule->xOpen(pTokenizer, z, n, &pCsr);
  assert( rc==SQLITE_OK || pCsr==0 );
  if( rc==SQLITE_OK ){
    pCsr->pTokenizer = pTokenizer;
    if( pModule->iVersion>=1 ){
      rc = pModule->xLanguageid(pCsr, iLangid);
      if( rc!=SQLITE_OK ){
        pModule->xClose(pCsr);
        pCsr = 0;
      }
    }
  }
  *ppCsr = pCsr;
  return rc;
}

/*
** Function getNextNode(), which is called by fts3ExprParse(), may itself
** call fts3ExprParse(). So this forward declaration is required.
*/
static int fts3ExprParse(ParseContext *, const char *, int, Fts3Expr **, int *);

/*
** Extract the next token from buffer z (length n) using the tokenizer
** and other information (column names etc.) in pParse. Create an Fts3Expr
** structure of type FTSQUERY_PHRASE containing a phrase consisting of this
** single token and set *ppExpr to point to it. If the end of the buffer is
** reached before a token is found, set *ppExpr to zero. It is the
** responsibility of the caller to eventually deallocate the allocated 
** Fts3Expr structure (if any) by passing it to sqlite3_free().
**
** Return SQLITE_OK if successful, or SQLITE_NOMEM if a memory allocation
** fails.
*/
static int getNextToken(
  ParseContext *pParse,                   /* fts3 query parse context */
  int iCol,                               /* Value for Fts3Phrase.iColumn */
  const char *z, int n,                   /* Input string */
  Fts3Expr **ppExpr,                      /* OUT: expression */
  int *pnConsumed                         /* OUT: Number of bytes consumed */
){
  sqlite3_tokenizer *pTokenizer = pParse->pTokenizer;
  sqlite3_tokenizer_module const *pModule = pTokenizer->pModule;
  int rc;
  sqlite3_tokenizer_cursor *pCursor;
  Fts3Expr *pRet = 0;
  int i = 0;

  /* Set variable i to the maximum number of bytes of input to tokenize. */
  for(i=0; i<n; i++){
    if( sqlite3_fts3_enable_parentheses && (z[i]=='(' || z[i]==')') ) break;
    if( z[i]=='"' ) break;
  }

  *pnConsumed = i;
  rc = sqlite3Fts3OpenTokenizer(pTokenizer, pParse->iLangid, z, i, &pCursor);
  if( rc==SQLITE_OK ){
    const char *zToken;
    int nToken = 0, iStart = 0, iEnd = 0, iPosition = 0;
    int nByte;                               /* total space to allocate */

    rc = pModule->xNext(pCursor, &zToken, &nToken, &iStart, &iEnd, &iPosition);
    if( rc==SQLITE_OK ){
      nByte = sizeof(Fts3Expr) + sizeof(Fts3Phrase) + nToken;
      pRet = (Fts3Expr *)fts3MallocZero(nByte);
      if( !pRet ){
        rc = SQLITE_NOMEM;
      }else{
        pRet->eType = FTSQUERY_PHRASE;
        pRet->pPhrase = (Fts3Phrase *)&pRet[1];
        pRet->pPhrase->nToken = 1;
        pRet->pPhrase->iColumn = iCol;
        pRet->pPhrase->aToken[0].n = nToken;
        pRet->pPhrase->aToken[0].z = (char *)&pRet->pPhrase[1];
        memcpy(pRet->pPhrase->aToken[0].z, zToken, nToken);

        if( iEnd<n && z[iEnd]=='*' ){
          pRet->pPhrase->aToken[0].isPrefix = 1;
          iEnd++;
        }

        while( 1 ){
          if( !sqlite3_fts3_enable_parentheses 
           && iStart>0 && z[iStart-1]=='-' 
          ){
            pParse->isNot = 1;
            iStart--;
          }else if( pParse->bFts4 && iStart>0 && z[iStart-1]=='^' ){
            pRet->pPhrase->aToken[0].bFirst = 1;
            iStart--;
          }else{
            break;
          }
        }

      }
      *pnConsumed = iEnd;
    }else if( i && rc==SQLITE_DONE ){
      rc = SQLITE_OK;
    }

    pModule->xClose(pCursor);
  }
  
  *ppExpr = pRet;
  return rc;
}


/*
** Enlarge a memory allocation.  If an out-of-memory allocation occurs,
** then free the old allocation.
*/
static void *fts3ReallocOrFree(void *pOrig, int nNew){
  void *pRet = sqlite3_realloc(pOrig, nNew);
  if( !pRet ){
    sqlite3_free(pOrig);
  }
  return pRet;
}

/*
** Buffer zInput, length nInput, contains the contents of a quoted string
** that appeared as part of an fts3 query expression. Neither quote character
** is included in the buffer. This function attempts to tokenize the entire
** input buffer and create an Fts3Expr structure of type FTSQUERY_PHRASE 
** containing the results.
**
** If successful, SQLITE_OK is returned and *ppExpr set to point at the
** allocated Fts3Expr structure. Otherwise, either SQLITE_NOMEM (out of memory
** error) or SQLITE_ERROR (tokenization error) is returned and *ppExpr set
** to 0.
*/
static int getNextString(
  ParseContext *pParse,                   /* fts3 query parse context */
  const char *zInput, int nInput,         /* Input string */
  Fts3Expr **ppExpr                       /* OUT: expression */
){
  sqlite3_tokenizer *pTokenizer = pParse->pTokenizer;
  sqlite3_tokenizer_module const *pModule = pTokenizer->pModule;
  int rc;
  Fts3Expr *p = 0;
  sqlite3_tokenizer_cursor *pCursor = 0;
  char *zTemp = 0;
  int nTemp = 0;

  const int nSpace = sizeof(Fts3Expr) + sizeof(Fts3Phrase);
  int nToken = 0;

  /* The final Fts3Expr data structure, including the Fts3Phrase,
  ** Fts3PhraseToken structures token buffers are all stored as a single 
  ** allocation so that the expression can be freed with a single call to
  ** sqlite3_free(). Setting this up requires a two pass approach.
  **
  ** The first pass, in the block below, uses a tokenizer cursor to iterate
  ** through the tokens in the expression. This pass uses fts3ReallocOrFree()
  ** to assemble data in two dynamic buffers:
  **
  **   Buffer p: Points to the Fts3Expr structure, followed by the Fts3Phrase
  **             structure, followed by the array of Fts3PhraseToken 
  **             structures. This pass only populates the Fts3PhraseToken array.
  **
  **   Buffer zTemp: Contains copies of all tokens.
  **
  ** The second pass, in the block that begins "if( rc==SQLITE_DONE )" below,
  ** appends buffer zTemp to buffer p, and fills in the Fts3Expr and Fts3Phrase
  ** structures.
  */
  rc = sqlite3Fts3OpenTokenizer(
      pTokenizer, pParse->iLangid, zInput, nInput, &pCursor);
  if( rc==SQLITE_OK ){
    int ii;
    for(ii=0; rc==SQLITE_OK; ii++){
      const char *zByte;
      int nByte = 0, iBegin = 0, iEnd = 0, iPos = 0;
      rc = pModule->xNext(pCursor, &zByte, &nByte, &iBegin, &iEnd, &iPos);
      if( rc==SQLITE_OK ){
        Fts3PhraseToken *pToken;

        p = fts3ReallocOrFree(p, nSpace + ii*sizeof(Fts3PhraseToken));
        if( !p ) goto no_mem;

        zTemp = fts3ReallocOrFree(zTemp, nTemp + nByte);
        if( !zTemp ) goto no_mem;

        assert( nToken==ii );
        pToken = &((Fts3Phrase *)(&p[1]))->aToken[ii];
        memset(pToken, 0, sizeof(Fts3PhraseToken));

        memcpy(&zTemp[nTemp], zByte, nByte);
        nTemp += nByte;

        pToken->n = nByte;
        pToken->isPrefix = (iEnd<nInput && zInput[iEnd]=='*');
        pToken->bFirst = (iBegin>0 && zInput[iBegin-1]=='^');
        nToken = ii+1;
      }
    }

    pModule->xClose(pCursor);
    pCursor = 0;
  }

  if( rc==SQLITE_DONE ){
    int jj;
    char *zBuf = 0;

    p = fts3ReallocOrFree(p, nSpace + nToken*sizeof(Fts3PhraseToken) + nTemp);
    if( !p ) goto no_mem;
    memset(p, 0, (char *)&(((Fts3Phrase *)&p[1])->aToken[0])-(char *)p);
    p->eType = FTSQUERY_PHRASE;
    p->pPhrase = (Fts3Phrase *)&p[1];
    p->pPhrase->iColumn = pParse->iDefaultCol;
    p->pPhrase->nToken = nToken;

    zBuf = (char *)&p->pPhrase->aToken[nToken];
    if( zTemp ){
      memcpy(zBuf, zTemp, nTemp);
      sqlite3_free(zTemp);
    }else{
      assert( nTemp==0 );
    }

    for(jj=0; jj<p->pPhrase->nToken; jj++){
      p->pPhrase->aToken[jj].z = zBuf;
      zBuf += p->pPhrase->aToken[jj].n;
    }
    rc = SQLITE_OK;
  }

  *ppExpr = p;
  return rc;
no_mem:

  if( pCursor ){
    pModule->xClose(pCursor);
  }
  sqlite3_free(zTemp);
  sqlite3_free(p);
  *ppExpr = 0;
  return SQLITE_NOMEM;
}

/*
** The output variable *ppExpr is populated with an allocated Fts3Expr 
** structure, or set to 0 if the end of the input buffer is reached.
**
** Returns an SQLite error code. SQLITE_OK if everything works, SQLITE_NOMEM
** if a malloc failure occurs, or SQLITE_ERROR if a parse error is encountered.
** If SQLITE_ERROR is returned, pContext is populated with an error message.
*/
static int getNextNode(
  ParseContext *pParse,                   /* fts3 query parse context */
  const char *z, int n,                   /* Input string */
  Fts3Expr **ppExpr,                      /* OUT: expression */
  int *pnConsumed                         /* OUT: Number of bytes consumed */
){
  static const struct Fts3Keyword {
    char *z;                              /* Keyword text */
    unsigned char n;                      /* Length of the keyword */
    unsigned char parenOnly;              /* Only valid in paren mode */
    unsigned char eType;                  /* Keyword code */
  } aKeyword[] = {
    { "OR" ,  2, 0, FTSQUERY_OR   },
    { "AND",  3, 1, FTSQUERY_AND  },
    { "NOT",  3, 1, FTSQUERY_NOT  },
    { "NEAR", 4, 0, FTSQUERY_NEAR }
  };
  int ii;
  int iCol;
  int iColLen;
  int rc;
  Fts3Expr *pRet = 0;

  const char *zInput = z;
  int nInput = n;

  pParse->isNot = 0;

  /* Skip over any whitespace before checking for a keyword, an open or
  ** close bracket, or a quoted string. 
  */
  while( nInput>0 && fts3isspace(*zInput) ){
    nInput--;
    zInput++;
  }
  if( nInput==0 ){
    return SQLITE_DONE;
  }

  /* See if we are dealing with a keyword. */
  for(ii=0; ii<(int)(sizeof(aKeyword)/sizeof(struct Fts3Keyword)); ii++){
    const struct Fts3Keyword *pKey = &aKeyword[ii];

    if( (pKey->parenOnly & ~sqlite3_fts3_enable_parentheses)!=0 ){
      continue;
    }

    if( nInput>=pKey->n && 0==memcmp(zInput, pKey->z, pKey->n) ){
      int nNear = SQLITE_FTS3_DEFAULT_NEAR_PARAM;
      int nKey = pKey->n;
      char cNext;

      /* If this is a "NEAR" keyword, check for an explicit nearness. */
      if( pKey->eType==FTSQUERY_NEAR ){
        assert( nKey==4 );
        if( zInput[4]=='/' && zInput[5]>='0' && zInput[5]<='9' ){
          nNear = 0;
          for(nKey=5; zInput[nKey]>='0' && zInput[nKey]<='9'; nKey++){
            nNear = nNear * 10 + (zInput[nKey] - '0');
          }
        }
      }

      /* At this point this is probably a keyword. But for that to be true,
      ** the next byte must contain either whitespace, an open or close
      ** parenthesis, a quote character, or EOF. 
      */
      cNext = zInput[nKey];
      if( fts3isspace(cNext) 
       || cNext=='"' || cNext=='(' || cNext==')' || cNext==0
      ){
        pRet = (Fts3Expr *)fts3MallocZero(sizeof(Fts3Expr));
        if( !pRet ){
          return SQLITE_NOMEM;
        }
        pRet->eType = pKey->eType;
        pRet->nNear = nNear;
        *ppExpr = pRet;
        *pnConsumed = (int)((zInput - z) + nKey);
        return SQLITE_OK;
      }

      /* Turns out that wasn't a keyword after all. This happens if the
      ** user has supplied a token such as "ORacle". Continue.
      */
    }
  }

  /* See if we are dealing with a quoted phrase. If this is the case, then
  ** search for the closing quote and pass the whole string to getNextString()
  ** for processing. This is easy to do, as fts3 has no syntax for escaping
  ** a quote character embedded in a string.
  */
  if( *zInput=='"' ){
    for(ii=1; ii<nInput && zInput[ii]!='"'; ii++);
    *pnConsumed = (int)((zInput - z) + ii + 1);
    if( ii==nInput ){
      return SQLITE_ERROR;
    }
    return getNextString(pParse, &zInput[1], ii-1, ppExpr);
  }

  if( sqlite3_fts3_enable_parentheses ){
    if( *zInput=='(' ){
      int nConsumed = 0;
      pParse->nNest++;
      rc = fts3ExprParse(pParse, zInput+1, nInput-1, ppExpr, &nConsumed);
      if( rc==SQLITE_OK && !*ppExpr ){ rc = SQLITE_DONE; }
      *pnConsumed = (int)(zInput - z) + 1 + nConsumed;
      return rc;
    }else if( *zInput==')' ){
      pParse->nNest--;
      *pnConsumed = (int)((zInput - z) + 1);
      *ppExpr = 0;
      return SQLITE_DONE;
    }
  }

  /* If control flows to this point, this must be a regular token, or 
  ** the end of the input. Read a regular token using the sqlite3_tokenizer
  ** interface. Before doing so, figure out if there is an explicit
  ** column specifier for the token. 
  **
  ** TODO: Strangely, it is not possible to associate a column specifier
  ** with a quoted phrase, only with a single token. Not sure if this was
  ** an implementation artifact or an intentional decision when fts3 was
  ** first implemented. Whichever it was, this module duplicates the 
  ** limitation.
  */
  iCol = pParse->iDefaultCol;
  iColLen = 0;
  for(ii=0; ii<pParse->nCol; ii++){
    const char *zStr = pParse->azCol[ii];
    int nStr = (int)strlen(zStr);
    if( nInput>nStr && zInput[nStr]==':' 
     && sqlite3_strnicmp(zStr, zInput, nStr)==0 
    ){
      iCol = ii;
      iColLen = (int)((zInput - z) + nStr + 1);
      break;
    }
  }
  rc = getNextToken(pParse, iCol, &z[iColLen], n-iColLen, ppExpr, pnConsumed);
  *pnConsumed += iColLen;
  return rc;
}

/*
** The argument is an Fts3Expr structure for a binary operator (any type
** except an FTSQUERY_PHRASE). Return an integer value representing the
** precedence of the operator. Lower values have a higher precedence (i.e.
** group more tightly). For example, in the C language, the == operator
** groups more tightly than ||, and would therefore have a higher precedence.
**
** When using the new fts3 query syntax (when SQLITE_ENABLE_FTS3_PARENTHESIS
** is defined), the order of the operators in precedence from highest to
** lowest is:
**
**   NEAR
**   NOT
**   AND (including implicit ANDs)
**   OR
**
** Note that when using the old query syntax, the OR operator has a higher
** precedence than the AND operator.
*/
static int opPrecedence(Fts3Expr *p){
  assert( p->eType!=FTSQUERY_PHRASE );
  if( sqlite3_fts3_enable_parentheses ){
    return p->eType;
  }else if( p->eType==FTSQUERY_NEAR ){
    return 1;
  }else if( p->eType==FTSQUERY_OR ){
    return 2;
  }
  assert( p->eType==FTSQUERY_AND );
  return 3;
}

/*
** Argument ppHead contains a pointer to the current head of a query 
** expression tree being parsed. pPrev is the expression node most recently
** inserted into the tree. This function adds pNew, which is always a binary
** operator node, into the expression tree based on the relative precedence
** of pNew and the existing nodes of the tree. This may result in the head
** of the tree changing, in which case *ppHead is set to the new root node.
*/
static void insertBinaryOperator(
  Fts3Expr **ppHead,       /* Pointer to the root node of a tree */
  Fts3Expr *pPrev,         /* Node most recently inserted into the tree */
  Fts3Expr *pNew           /* New binary node to insert into expression tree */
){
  Fts3Expr *pSplit = pPrev;
  while( pSplit->pParent && opPrecedence(pSplit->pParent)<=opPrecedence(pNew) ){
    pSplit = pSplit->pParent;
  }

  if( pSplit->pParent ){
    assert( pSplit->pParent->pRight==pSplit );
    pSplit->pParent->pRight = pNew;
    pNew->pParent = pSplit->pParent;
  }else{
    *ppHead = pNew;
  }
  pNew->pLeft = pSplit;
  pSplit->pParent = pNew;
}

/*
** Parse the fts3 query expression found in buffer z, length n. This function
** returns either when the end of the buffer is reached or an unmatched 
** closing bracket - ')' - is encountered.
**
** If successful, SQLITE_OK is returned, *ppExpr is set to point to the
** parsed form of the expression and *pnConsumed is set to the number of
** bytes read from buffer z. Otherwise, *ppExpr is set to 0 and SQLITE_NOMEM
** (out of memory error) or SQLITE_ERROR (parse error) is returned.
*/
static int fts3ExprParse(
  ParseContext *pParse,                   /* fts3 query parse context */
  const char *z, int n,                   /* Text of MATCH query */
  Fts3Expr **ppExpr,                      /* OUT: Parsed query structure */
  int *pnConsumed                         /* OUT: Number of bytes consumed */
){
  Fts3Expr *pRet = 0;
  Fts3Expr *pPrev = 0;
  Fts3Expr *pNotBranch = 0;               /* Only used in legacy parse mode */
  int nIn = n;
  const char *zIn = z;
  int rc = SQLITE_OK;
  int isRequirePhrase = 1;

  while( rc==SQLITE_OK ){
    Fts3Expr *p = 0;
    int nByte = 0;

    rc = getNextNode(pParse, zIn, nIn, &p, &nByte);
    assert( nByte>0 || (rc!=SQLITE_OK && p==0) );
    if( rc==SQLITE_OK ){
      if( p ){
        int isPhrase;

        if( !sqlite3_fts3_enable_parentheses 
            && p->eType==FTSQUERY_PHRASE && pParse->isNot 
        ){
          /* Create an implicit NOT operator. */
          Fts3Expr *pNot = fts3MallocZero(sizeof(Fts3Expr));
          if( !pNot ){
            sqlite3Fts3ExprFree(p);
            rc = SQLITE_NOMEM;
            goto exprparse_out;
          }
          pNot->eType = FTSQUERY_NOT;
          pNot->pRight = p;
          p->pParent = pNot;
          if( pNotBranch ){
            pNot->pLeft = pNotBranch;
            pNotBranch->pParent = pNot;
          }
          pNotBranch = pNot;
          p = pPrev;
        }else{
          int eType = p->eType;
          isPhrase = (eType==FTSQUERY_PHRASE || p->pLeft);

          /* The isRequirePhrase variable is set to true if a phrase or
          ** an expression contained in parenthesis is required. If a
          ** binary operator (AND, OR, NOT or NEAR) is encounted when
          ** isRequirePhrase is set, this is a syntax error.
          */
          if( !isPhrase && isRequirePhrase ){
            sqlite3Fts3ExprFree(p);
            rc = SQLITE_ERROR;
            goto exprparse_out;
          }

          if( isPhrase && !isRequirePhrase ){
            /* Insert an implicit AND operator. */
            Fts3Expr *pAnd;
            assert( pRet && pPrev );
            pAnd = fts3MallocZero(sizeof(Fts3Expr));
            if( !pAnd ){
              sqlite3Fts3ExprFree(p);
              rc = SQLITE_NOMEM;
              goto exprparse_out;
            }
            pAnd->eType = FTSQUERY_AND;
            insertBinaryOperator(&pRet, pPrev, pAnd);
            pPrev = pAnd;
          }

          /* This test catches attempts to make either operand of a NEAR
           ** operator something other than a phrase. For example, either of
           ** the following:
           **
           **    (bracketed expression) NEAR phrase
           **    phrase NEAR (bracketed expression)
           **
           ** Return an error in either case.
           */
          if( pPrev && (
            (eType==FTSQUERY_NEAR && !isPhrase && pPrev->eType!=FTSQUERY_PHRASE)
         || (eType!=FTSQUERY_PHRASE && isPhrase && pPrev->eType==FTSQUERY_NEAR)
          )){
            sqlite3Fts3ExprFree(p);
            rc = SQLITE_ERROR;
            goto exprparse_out;
          }

          if( isPhrase ){
            if( pRet ){
              assert( pPrev && pPrev->pLeft && pPrev->pRight==0 );
              pPrev->pRight = p;
              p->pParent = pPrev;
            }else{
              pRet = p;
            }
          }else{
            insertBinaryOperator(&pRet, pPrev, p);
          }
          isRequirePhrase = !isPhrase;
        }
        pPrev = p;
      }
      assert( nByte>0 );
    }
    assert( rc!=SQLITE_OK || (nByte>0 && nByte<=nIn) );
    nIn -= nByte;
    zIn += nByte;
  }

  if( rc==SQLITE_DONE && pRet && isRequirePhrase ){
    rc = SQLITE_ERROR;
  }

  if( rc==SQLITE_DONE ){
    rc = SQLITE_OK;
    if( !sqlite3_fts3_enable_parentheses && pNotBranch ){
      if( !pRet ){
        rc = SQLITE_ERROR;
      }else{
        Fts3Expr *pIter = pNotBranch;
        while( pIter->pLeft ){
          pIter = pIter->pLeft;
        }
        pIter->pLeft = pRet;
        pRet->pParent = pIter;
        pRet = pNotBranch;
      }
    }
  }
  *pnConsumed = n - nIn;

exprparse_out:
  if( rc!=SQLITE_OK ){
    sqlite3Fts3ExprFree(pRet);
    sqlite3Fts3ExprFree(pNotBranch);
    pRet = 0;
  }
  *ppExpr = pRet;
  return rc;
}

/*
** Return SQLITE_ERROR if the maximum depth of the expression tree passed 
** as the only argument is more than nMaxDepth.
*/
static int fts3ExprCheckDepth(Fts3Expr *p, int nMaxDepth){
  int rc = SQLITE_OK;
  if( p ){
    if( nMaxDepth<0 ){ 
      rc = SQLITE_TOOBIG;
    }else{
      rc = fts3ExprCheckDepth(p->pLeft, nMaxDepth-1);
      if( rc==SQLITE_OK ){
        rc = fts3ExprCheckDepth(p->pRight, nMaxDepth-1);
      }
    }
  }
  return rc;
}

/*
** This function attempts to transform the expression tree at (*pp) to
** an equivalent but more balanced form. The tree is modified in place.
** If successful, SQLITE_OK is returned and (*pp) set to point to the 
** new root expression node. 
**
** nMaxDepth is the maximum allowable depth of the balanced sub-tree.
**
** Otherwise, if an error occurs, an SQLite error code is returned and 
** expression (*pp) freed.
*/
static int fts3ExprBalance(Fts3Expr **pp, int nMaxDepth){
  int rc = SQLITE_OK;             /* Return code */
  Fts3Expr *pRoot = *pp;          /* Initial root node */
  Fts3Expr *pFree = 0;            /* List of free nodes. Linked by pParent. */
  int eType = pRoot->eType;       /* Type of node in this tree */

  if( nMaxDepth==0 ){
    rc = SQLITE_ERROR;
  }

  if( rc==SQLITE_OK && (eType==FTSQUERY_AND || eType==FTSQUERY_OR) ){
    Fts3Expr **apLeaf;
    apLeaf = (Fts3Expr **)sqlite3_malloc(sizeof(Fts3Expr *) * nMaxDepth);
    if( 0==apLeaf ){
      rc = SQLITE_NOMEM;
    }else{
      memset(apLeaf, 0, sizeof(Fts3Expr *) * nMaxDepth);
    }

    if( rc==SQLITE_OK ){
      int i;
      Fts3Expr *p;

      /* Set $p to point to the left-most leaf in the tree of eType nodes. */
      for(p=pRoot; p->eType==eType; p=p->pLeft){
        assert( p->pParent==0 || p->pParent->pLeft==p );
        assert( p->pLeft && p->pRight );
      }

      /* This loop runs once for each leaf in the tree of eType nodes. */
      while( 1 ){
        int iLvl;
        Fts3Expr *pParent = p->pParent;     /* Current parent of p */

        assert( pParent==0 || pParent->pLeft==p );
        p->pParent = 0;
        if( pParent ){
          pParent->pLeft = 0;
        }else{
          pRoot = 0;
        }
        rc = fts3ExprBalance(&p, nMaxDepth-1);
        if( rc!=SQLITE_OK ) break;

        for(iLvl=0; p && iLvl<nMaxDepth; iLvl++){
          if( apLeaf[iLvl]==0 ){
            apLeaf[iLvl] = p;
            p = 0;
          }else{
            assert( pFree );
            pFree->pLeft = apLeaf[iLvl];
            pFree->pRight = p;
            pFree->pLeft->pParent = pFree;
            pFree->pRight->pParent = pFree;

            p = pFree;
            pFree = pFree->pParent;
            p->pParent = 0;
            apLeaf[iLvl] = 0;
          }
        }
        if( p ){
          sqlite3Fts3ExprFree(p);
          rc = SQLITE_TOOBIG;
          break;
        }

        /* If that was the last leaf node, break out of the loop */
        if( pParent==0 ) break;

        /* Set $p to point to the next leaf in the tree of eType nodes */
        for(p=pParent->pRight; p->eType==eType; p=p->pLeft);

        /* Remove pParent from the original tree. */
        assert( pParent->pParent==0 || pParent->pParent->pLeft==pParent );
        pParent->pRight->pParent = pParent->pParent;
        if( pParent->pParent ){
          pParent->pParent->pLeft = pParent->pRight;
        }else{
          assert( pParent==pRoot );
          pRoot = pParent->pRight;
        }

        /* Link pParent into the free node list. It will be used as an
        ** internal node of the new tree.  */
        pParent->pParent = pFree;
        pFree = pParent;
      }

      if( rc==SQLITE_OK ){
        p = 0;
        for(i=0; i<nMaxDepth; i++){
          if( apLeaf[i] ){
            if( p==0 ){
              p = apLeaf[i];
              p->pParent = 0;
            }else{
              assert( pFree!=0 );
              pFree->pRight = p;
              pFree->pLeft = apLeaf[i];
              pFree->pLeft->pParent = pFree;
              pFree->pRight->pParent = pFree;

              p = pFree;
              pFree = pFree->pParent;
              p->pParent = 0;
            }
          }
        }
        pRoot = p;
      }else{
        /* An error occurred. Delete the contents of the apLeaf[] array 
        ** and pFree list. Everything else is cleaned up by the call to
        ** sqlite3Fts3ExprFree(pRoot) below.  */
        Fts3Expr *pDel;
        for(i=0; i<nMaxDepth; i++){
          sqlite3Fts3ExprFree(apLeaf[i]);
        }
        while( (pDel=pFree)!=0 ){
          pFree = pDel->pParent;
          sqlite3_free(pDel);
        }
      }

      assert( pFree==0 );
      sqlite3_free( apLeaf );
    }
  }

  if( rc!=SQLITE_OK ){
    sqlite3Fts3ExprFree(pRoot);
    pRoot = 0;
  }
  *pp = pRoot;
  return rc;
}

/*
** This function is similar to sqlite3Fts3ExprParse(), with the following
** differences:
**
**   1. It does not do expression rebalancing.
**   2. It does not check that the expression does not exceed the 
**      maximum allowable depth.
**   3. Even if it fails, *ppExpr may still be set to point to an 
**      expression tree. It should be deleted using sqlite3Fts3ExprFree()
**      in this case.
*/
static int fts3ExprParseUnbalanced(
  sqlite3_tokenizer *pTokenizer,      /* Tokenizer module */
  int iLangid,                        /* Language id for tokenizer */
  char **azCol,                       /* Array of column names for fts3 table */
  int bFts4,                          /* True to allow FTS4-only syntax */
  int nCol,                           /* Number of entries in azCol[] */
  int iDefaultCol,                    /* Default column to query */
  const char *z, int n,               /* Text of MATCH query */
  Fts3Expr **ppExpr                   /* OUT: Parsed query structure */
){
  int nParsed;
  int rc;
  ParseContext sParse;

  memset(&sParse, 0, sizeof(ParseContext));
  sParse.pTokenizer = pTokenizer;
  sParse.iLangid = iLangid;
  sParse.azCol = (const char **)azCol;
  sParse.nCol = nCol;
  sParse.iDefaultCol = iDefaultCol;
  sParse.bFts4 = bFts4;
  if( z==0 ){
    *ppExpr = 0;
    return SQLITE_OK;
  }
  if( n<0 ){
    n = (int)strlen(z);
  }
  rc = fts3ExprParse(&sParse, z, n, ppExpr, &nParsed);
  assert( rc==SQLITE_OK || *ppExpr==0 );

  /* Check for mismatched parenthesis */
  if( rc==SQLITE_OK && sParse.nNest ){
    rc = SQLITE_ERROR;
  }
  
  return rc;
}

/*
** Parameters z and n contain a pointer to and length of a buffer containing
** an fts3 query expression, respectively. This function attempts to parse the
** query expression and create a tree of Fts3Expr structures representing the
** parsed expression. If successful, *ppExpr is set to point to the head
** of the parsed expression tree and SQLITE_OK is returned. If an error
** occurs, either SQLITE_NOMEM (out-of-memory error) or SQLITE_ERROR (parse
** error) is returned and *ppExpr is set to 0.
**
** If parameter n is a negative number, then z is assumed to point to a
** nul-terminated string and the length is determined using strlen().
**
** The first parameter, pTokenizer, is passed the fts3 tokenizer module to
** use to normalize query tokens while parsing the expression. The azCol[]
** array, which is assumed to contain nCol entries, should contain the names
** of each column in the target fts3 table, in order from left to right. 
** Column names must be nul-terminated strings.
**
** The iDefaultCol parameter should be passed the index of the table column
** that appears on the left-hand-side of the MATCH operator (the default
** column to match against for tokens for which a column name is not explicitly
** specified as part of the query string), or -1 if tokens may by default
** match any table column.
*/
SQLITE_PRIVATE int sqlite3Fts3ExprParse(
  sqlite3_tokenizer *pTokenizer,      /* Tokenizer module */
  int iLangid,                        /* Language id for tokenizer */
  char **azCol,                       /* Array of column names for fts3 table */
  int bFts4,                          /* True to allow FTS4-only syntax */
  int nCol,                           /* Number of entries in azCol[] */
  int iDefaultCol,                    /* Default column to query */
  const char *z, int n,               /* Text of MATCH query */
  Fts3Expr **ppExpr,                  /* OUT: Parsed query structure */
  char **pzErr                        /* OUT: Error message (sqlite3_malloc) */
){
  int rc = fts3ExprParseUnbalanced(
      pTokenizer, iLangid, azCol, bFts4, nCol, iDefaultCol, z, n, ppExpr
  );
  
  /* Rebalance the expression. And check that its depth does not exceed
  ** SQLITE_FTS3_MAX_EXPR_DEPTH.  */
  if( rc==SQLITE_OK && *ppExpr ){
    rc = fts3ExprBalance(ppExpr, SQLITE_FTS3_MAX_EXPR_DEPTH);
    if( rc==SQLITE_OK ){
      rc = fts3ExprCheckDepth(*ppExpr, SQLITE_FTS3_MAX_EXPR_DEPTH);
    }
  }

  if( rc!=SQLITE_OK ){
    sqlite3Fts3ExprFree(*ppExpr);
    *ppExpr = 0;
    if( rc==SQLITE_TOOBIG ){
      sqlite3Fts3ErrMsg(pzErr,
          "FTS expression tree is too large (maximum depth %d)", 
          SQLITE_FTS3_MAX_EXPR_DEPTH
      );
      rc = SQLITE_ERROR;
    }else if( rc==SQLITE_ERROR ){
      sqlite3Fts3ErrMsg(pzErr, "malformed MATCH expression: [%s]", z);
    }
  }

  return rc;
}

/*
** Free a single node of an expression tree.
*/
static void fts3FreeExprNode(Fts3Expr *p){
  assert( p->eType==FTSQUERY_PHRASE || p->pPhrase==0 );
  sqlite3Fts3EvalPhraseCleanup(p->pPhrase);
  sqlite3_free(p->aMI);
  sqlite3_free(p);
}

/*
** Free a parsed fts3 query expression allocated by sqlite3Fts3ExprParse().
**
** This function would be simpler if it recursively called itself. But
** that would mean passing a sufficiently large expression to ExprParse()
** could cause a stack overflow.
*/
SQLITE_PRIVATE void sqlite3Fts3ExprFree(Fts3Expr *pDel){
  Fts3Expr *p;
  assert( pDel==0 || pDel->pParent==0 );
  for(p=pDel; p && (p->pLeft||p->pRight); p=(p->pLeft ? p->pLeft : p->pRight)){
    assert( p->pParent==0 || p==p->pParent->pRight || p==p->pParent->pLeft );
  }
  while( p ){
    Fts3Expr *pParent = p->pParent;
    fts3FreeExprNode(p);
    if( pParent && p==pParent->pLeft && pParent->pRight ){
      p = pParent->pRight;
      while( p && (p->pLeft || p->pRight) ){
        assert( p==p->pParent->pRight || p==p->pParent->pLeft );
        p = (p->pLeft ? p->pLeft : p->pRight);
      }
    }else{
      p = pParent;
    }
  }
}

/****************************************************************************
*****************************************************************************
** Everything after this point is just test code.
*/

#ifdef SQLITE_TEST

/* #include <stdio.h> */

/*
** Function to query the hash-table of tokenizers (see README.tokenizers).
*/
static int queryTestTokenizer(
  sqlite3 *db, 
  const char *zName,  
  const sqlite3_tokenizer_module **pp
){
  int rc;
  sqlite3_stmt *pStmt;
  const char zSql[] = "SELECT fts3_tokenizer(?)";

  *pp = 0;
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  sqlite3_bind_text(pStmt, 1, zName, -1, SQLITE_STATIC);
  if( SQLITE_ROW==sqlite3_step(pStmt) ){
    if( sqlite3_column_type(pStmt, 0)==SQLITE_BLOB ){
      memcpy((void *)pp, sqlite3_column_blob(pStmt, 0), sizeof(*pp));
    }
  }

  return sqlite3_finalize(pStmt);
}

/*
** Return a pointer to a buffer containing a text representation of the
** expression passed as the first argument. The buffer is obtained from
** sqlite3_malloc(). It is the responsibility of the caller to use 
** sqlite3_free() to release the memory. If an OOM condition is encountered,
** NULL is returned.
**
** If the second argument is not NULL, then its contents are prepended to 
** the returned expression text and then freed using sqlite3_free().
*/
static char *exprToString(Fts3Expr *pExpr, char *zBuf){
  if( pExpr==0 ){
    return sqlite3_mprintf("");
  }
  switch( pExpr->eType ){
    case FTSQUERY_PHRASE: {
      Fts3Phrase *pPhrase = pExpr->pPhrase;
      int i;
      zBuf = sqlite3_mprintf(
          "%zPHRASE %d 0", zBuf, pPhrase->iColumn);
      for(i=0; zBuf && i<pPhrase->nToken; i++){
        zBuf = sqlite3_mprintf("%z %.*s%s", zBuf, 
            pPhrase->aToken[i].n, pPhrase->aToken[i].z,
            (pPhrase->aToken[i].isPrefix?"+":"")
        );
      }
      return zBuf;
    }

    case FTSQUERY_NEAR:
      zBuf = sqlite3_mprintf("%zNEAR/%d ", zBuf, pExpr->nNear);
      break;
    case FTSQUERY_NOT:
      zBuf = sqlite3_mprintf("%zNOT ", zBuf);
      break;
    case FTSQUERY_AND:
      zBuf = sqlite3_mprintf("%zAND ", zBuf);
      break;
    case FTSQUERY_OR:
      zBuf = sqlite3_mprintf("%zOR ", zBuf);
      break;
  }

  if( zBuf ) zBuf = sqlite3_mprintf("%z{", zBuf);
  if( zBuf ) zBuf = exprToString(pExpr->pLeft, zBuf);
  if( zBuf ) zBuf = sqlite3_mprintf("%z} {", zBuf);

  if( zBuf ) zBuf = exprToString(pExpr->pRight, zBuf);
  if( zBuf ) zBuf = sqlite3_mprintf("%z}", zBuf);

  return zBuf;
}

/*
** This is the implementation of a scalar SQL function used to test the 
** expression parser. It should be called as follows:
**
**   fts3_exprtest(<tokenizer>, <expr>, <column 1>, ...);
**
** The first argument, <tokenizer>, is the name of the fts3 tokenizer used
** to parse the query expression (see README.tokenizers). The second argument
** is the query expression to parse. Each subsequent argument is the name
** of a column of the fts3 table that the query expression may refer to.
** For example:
**
**   SELECT fts3_exprtest('simple', 'Bill col2:Bloggs', 'col1', 'col2');
*/
static void fts3ExprTest(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  sqlite3_tokenizer_module const *pModule = 0;
  sqlite3_tokenizer *pTokenizer = 0;
  int rc;
  char **azCol = 0;
  const char *zExpr;
  int nExpr;
  int nCol;
  int ii;
  Fts3Expr *pExpr;
  char *zBuf = 0;
  sqlite3 *db = sqlite3_context_db_handle(context);

  if( argc<3 ){
    sqlite3_result_error(context, 
        "Usage: fts3_exprtest(tokenizer, expr, col1, ...", -1
    );
    return;
  }

  rc = queryTestTokenizer(db,
                          (const char *)sqlite3_value_text(argv[0]), &pModule);
  if( rc==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(context);
    goto exprtest_out;
  }else if( !pModule ){
    sqlite3_result_error(context, "No such tokenizer module", -1);
    goto exprtest_out;
  }

  rc = pModule->xCreate(0, 0, &pTokenizer);
  assert( rc==SQLITE_NOMEM || rc==SQLITE_OK );
  if( rc==SQLITE_NOMEM ){
    sqlite3_result_error_nomem(context);
    goto exprtest_out;
  }
  pTokenizer->pModule = pModule;

  zExpr = (const char *)sqlite3_value_text(argv[1]);
  nExpr = sqlite3_value_bytes(argv[1]);
  nCol = argc-2;
  azCol = (char **)sqlite3_malloc(nCol*sizeof(char *));
  if( !azCol ){
    sqlite3_result_error_nomem(context);
    goto exprtest_out;
  }
  for(ii=0; ii<nCol; ii++){
    azCol[ii] = (char *)sqlite3_value_text(argv[ii+2]);
  }

  if( sqlite3_user_data(context) ){
    char *zDummy = 0;
    rc = sqlite3Fts3ExprParse(
        pTokenizer, 0, azCol, 0, nCol, nCol, zExpr, nExpr, &pExpr, &zDummy
    );
    assert( rc==SQLITE_OK || pExpr==0 );
    sqlite3_free(zDummy);
  }else{
    rc = fts3ExprParseUnbalanced(
        pTokenizer, 0, azCol, 0, nCol, nCol, zExpr, nExpr, &pExpr
    );
  }

  if( rc!=SQLITE_OK && rc!=SQLITE_NOMEM ){
    sqlite3Fts3ExprFree(pExpr);
    sqlite3_result_error(context, "Error parsing expression", -1);
  }else if( rc==SQLITE_NOMEM || !(zBuf = exprToString(pExpr, 0)) ){
    sqlite3_result_error_nomem(context);
  }else{
    sqlite3_result_text(context, zBuf, -1, SQLITE_TRANSIENT);
    sqlite3_free(zBuf);
  }

  sqlite3Fts3ExprFree(pExpr);

exprtest_out:
  if( pModule && pTokenizer ){
    rc = pModule->xDestroy(pTokenizer);
  }
  sqlite3_free(azCol);
}

/*
** Register the query expression parser test function fts3_exprtest() 
** with database connection db. 
*/
SQLITE_PRIVATE int sqlite3Fts3ExprInitTestInterface(sqlite3* db){
  int rc = sqlite3_create_function(
      db, "fts3_exprtest", -1, SQLITE_UTF8, 0, fts3ExprTest, 0, 0
  );
  if( rc==SQLITE_OK ){
    rc = sqlite3_create_function(db, "fts3_exprtest_rebalance", 
        -1, SQLITE_UTF8, (void *)1, fts3ExprTest, 0, 0
    );
  }
  return rc;
}

#endif
#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_expr.c *******************************************/
/************** Begin file fts3_hash.c ***************************************/
/*
** 2001 September 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This is the implementation of generic hash-tables used in SQLite.
** We've modified it slightly to serve as a standalone hash table
** implementation for the full-text indexing module.
*/

/*
** The code in this file is only compiled if:
**
**     * The FTS3 module is being built as an extension
**       (in which case SQLITE_CORE is not defined), or
**
**     * The FTS3 module is being built into the core of
**       SQLite (in which case SQLITE_ENABLE_FTS3 is defined).
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <assert.h> */
/* #include <stdlib.h> */
/* #include <string.h> */

/* #include "fts3_hash.h" */

/*
** Malloc and Free functions
*/
static void *fts3HashMalloc(int n){
  void *p = sqlite3_malloc(n);
  if( p ){
    memset(p, 0, n);
  }
  return p;
}
static void fts3HashFree(void *p){
  sqlite3_free(p);
}

/* Turn bulk memory into a hash table object by initializing the
** fields of the Hash structure.
**
** "pNew" is a pointer to the hash table that is to be initialized.
** keyClass is one of the constants 
** FTS3_HASH_BINARY or FTS3_HASH_STRING.  The value of keyClass 
** determines what kind of key the hash table will use.  "copyKey" is
** true if the hash table should make its own private copy of keys and
** false if it should just use the supplied pointer.
*/
SQLITE_PRIVATE void sqlite3Fts3HashInit(Fts3Hash *pNew, char keyClass, char copyKey){
  assert( pNew!=0 );
  assert( keyClass>=FTS3_HASH_STRING && keyClass<=FTS3_HASH_BINARY );
  pNew->keyClass = keyClass;
  pNew->copyKey = copyKey;
  pNew->first = 0;
  pNew->count = 0;
  pNew->htsize = 0;
  pNew->ht = 0;
}

/* Remove all entries from a hash table.  Reclaim all memory.
** Call this routine to delete a hash table or to reset a hash table
** to the empty state.
*/
SQLITE_PRIVATE void sqlite3Fts3HashClear(Fts3Hash *pH){
  Fts3HashElem *elem;         /* For looping over all elements of the table */

  assert( pH!=0 );
  elem = pH->first;
  pH->first = 0;
  fts3HashFree(pH->ht);
  pH->ht = 0;
  pH->htsize = 0;
  while( elem ){
    Fts3HashElem *next_elem = elem->next;
    if( pH->copyKey && elem->pKey ){
      fts3HashFree(elem->pKey);
    }
    fts3HashFree(elem);
    elem = next_elem;
  }
  pH->count = 0;
}

/*
** Hash and comparison functions when the mode is FTS3_HASH_STRING
*/
static int fts3StrHash(const void *pKey, int nKey){
  const char *z = (const char *)pKey;
  unsigned h = 0;
  if( nKey<=0 ) nKey = (int) strlen(z);
  while( nKey > 0  ){
    h = (h<<3) ^ h ^ *z++;
    nKey--;
  }
  return (int)(h & 0x7fffffff);
}
static int fts3StrCompare(const void *pKey1, int n1, const void *pKey2, int n2){
  if( n1!=n2 ) return 1;
  return strncmp((const char*)pKey1,(const char*)pKey2,n1);
}

/*
** Hash and comparison functions when the mode is FTS3_HASH_BINARY
*/
static int fts3BinHash(const void *pKey, int nKey){
  int h = 0;
  const char *z = (const char *)pKey;
  while( nKey-- > 0 ){
    h = (h<<3) ^ h ^ *(z++);
  }
  return h & 0x7fffffff;
}
static int fts3BinCompare(const void *pKey1, int n1, const void *pKey2, int n2){
  if( n1!=n2 ) return 1;
  return memcmp(pKey1,pKey2,n1);
}

/*
** Return a pointer to the appropriate hash function given the key class.
**
** The C syntax in this function definition may be unfamilar to some 
** programmers, so we provide the following additional explanation:
**
** The name of the function is "ftsHashFunction".  The function takes a
** single parameter "keyClass".  The return value of ftsHashFunction()
** is a pointer to another function.  Specifically, the return value
** of ftsHashFunction() is a pointer to a function that takes two parameters
** with types "const void*" and "int" and returns an "int".
*/
static int (*ftsHashFunction(int keyClass))(const void*,int){
  if( keyClass==FTS3_HASH_STRING ){
    return &fts3StrHash;
  }else{
    assert( keyClass==FTS3_HASH_BINARY );
    return &fts3BinHash;
  }
}

/*
** Return a pointer to the appropriate hash function given the key class.
**
** For help in interpreted the obscure C code in the function definition,
** see the header comment on the previous function.
*/
static int (*ftsCompareFunction(int keyClass))(const void*,int,const void*,int){
  if( keyClass==FTS3_HASH_STRING ){
    return &fts3StrCompare;
  }else{
    assert( keyClass==FTS3_HASH_BINARY );
    return &fts3BinCompare;
  }
}

/* Link an element into the hash table
*/
static void fts3HashInsertElement(
  Fts3Hash *pH,            /* The complete hash table */
  struct _fts3ht *pEntry,  /* The entry into which pNew is inserted */
  Fts3HashElem *pNew       /* The element to be inserted */
){
  Fts3HashElem *pHead;     /* First element already in pEntry */
  pHead = pEntry->chain;
  if( pHead ){
    pNew->next = pHead;
    pNew->prev = pHead->prev;
    if( pHead->prev ){ pHead->prev->next = pNew; }
    else             { pH->first = pNew; }
    pHead->prev = pNew;
  }else{
    pNew->next = pH->first;
    if( pH->first ){ pH->first->prev = pNew; }
    pNew->prev = 0;
    pH->first = pNew;
  }
  pEntry->count++;
  pEntry->chain = pNew;
}


/* Resize the hash table so that it cantains "new_size" buckets.
** "new_size" must be a power of 2.  The hash table might fail 
** to resize if sqliteMalloc() fails.
**
** Return non-zero if a memory allocation error occurs.
*/
static int fts3Rehash(Fts3Hash *pH, int new_size){
  struct _fts3ht *new_ht;          /* The new hash table */
  Fts3HashElem *elem, *next_elem;  /* For looping over existing elements */
  int (*xHash)(const void*,int);   /* The hash function */

  assert( (new_size & (new_size-1))==0 );
  new_ht = (struct _fts3ht *)fts3HashMalloc( new_size*sizeof(struct _fts3ht) );
  if( new_ht==0 ) return 1;
  fts3HashFree(pH->ht);
  pH->ht = new_ht;
  pH->htsize = new_size;
  xHash = ftsHashFunction(pH->keyClass);
  for(elem=pH->first, pH->first=0; elem; elem = next_elem){
    int h = (*xHash)(elem->pKey, elem->nKey) & (new_size-1);
    next_elem = elem->next;
    fts3HashInsertElement(pH, &new_ht[h], elem);
  }
  return 0;
}

/* This function (for internal use only) locates an element in an
** hash table that matches the given key.  The hash for this key has
** already been computed and is passed as the 4th parameter.
*/
static Fts3HashElem *fts3FindElementByHash(
  const Fts3Hash *pH, /* The pH to be searched */
  const void *pKey,   /* The key we are searching for */
  int nKey,
  int h               /* The hash for this key. */
){
  Fts3HashElem *elem;            /* Used to loop thru the element list */
  int count;                     /* Number of elements left to test */
  int (*xCompare)(const void*,int,const void*,int);  /* comparison function */

  if( pH->ht ){
    struct _fts3ht *pEntry = &pH->ht[h];
    elem = pEntry->chain;
    count = pEntry->count;
    xCompare = ftsCompareFunction(pH->keyClass);
    while( count-- && elem ){
      if( (*xCompare)(elem->pKey,elem->nKey,pKey,nKey)==0 ){ 
        return elem;
      }
      elem = elem->next;
    }
  }
  return 0;
}

/* Remove a single entry from the hash table given a pointer to that
** element and a hash on the element's key.
*/
static void fts3RemoveElementByHash(
  Fts3Hash *pH,         /* The pH containing "elem" */
  Fts3HashElem* elem,   /* The element to be removed from the pH */
  int h                 /* Hash value for the element */
){
  struct _fts3ht *pEntry;
  if( elem->prev ){
    elem->prev->next = elem->next; 
  }else{
    pH->first = elem->next;
  }
  if( elem->next ){
    elem->next->prev = elem->prev;
  }
  pEntry = &pH->ht[h];
  if( pEntry->chain==elem ){
    pEntry->chain = elem->next;
  }
  pEntry->count--;
  if( pEntry->count<=0 ){
    pEntry->chain = 0;
  }
  if( pH->copyKey && elem->pKey ){
    fts3HashFree(elem->pKey);
  }
  fts3HashFree( elem );
  pH->count--;
  if( pH->count<=0 ){
    assert( pH->first==0 );
    assert( pH->count==0 );
    fts3HashClear(pH);
  }
}

SQLITE_PRIVATE Fts3HashElem *sqlite3Fts3HashFindElem(
  const Fts3Hash *pH, 
  const void *pKey, 
  int nKey
){
  int h;                          /* A hash on key */
  int (*xHash)(const void*,int);  /* The hash function */

  if( pH==0 || pH->ht==0 ) return 0;
  xHash = ftsHashFunction(pH->keyClass);
  assert( xHash!=0 );
  h = (*xHash)(pKey,nKey);
  assert( (pH->htsize & (pH->htsize-1))==0 );
  return fts3FindElementByHash(pH,pKey,nKey, h & (pH->htsize-1));
}

/* 
** Attempt to locate an element of the hash table pH with a key
** that matches pKey,nKey.  Return the data for this element if it is
** found, or NULL if there is no match.
*/
SQLITE_PRIVATE void *sqlite3Fts3HashFind(const Fts3Hash *pH, const void *pKey, int nKey){
  Fts3HashElem *pElem;            /* The element that matches key (if any) */

  pElem = sqlite3Fts3HashFindElem(pH, pKey, nKey);
  return pElem ? pElem->data : 0;
}

/* Insert an element into the hash table pH.  The key is pKey,nKey
** and the data is "data".
**
** If no element exists with a matching key, then a new
** element is created.  A copy of the key is made if the copyKey
** flag is set.  NULL is returned.
**
** If another element already exists with the same key, then the
** new data replaces the old data and the old data is returned.
** The key is not copied in this instance.  If a malloc fails, then
** the new data is returned and the hash table is unchanged.
**
** If the "data" parameter to this function is NULL, then the
** element corresponding to "key" is removed from the hash table.
*/
SQLITE_PRIVATE void *sqlite3Fts3HashInsert(
  Fts3Hash *pH,        /* The hash table to insert into */
  const void *pKey,    /* The key */
  int nKey,            /* Number of bytes in the key */
  void *data           /* The data */
){
  int hraw;                 /* Raw hash value of the key */
  int h;                    /* the hash of the key modulo hash table size */
  Fts3HashElem *elem;       /* Used to loop thru the element list */
  Fts3HashElem *new_elem;   /* New element added to the pH */
  int (*xHash)(const void*,int);  /* The hash function */

  assert( pH!=0 );
  xHash = ftsHashFunction(pH->keyClass);
  assert( xHash!=0 );
  hraw = (*xHash)(pKey, nKey);
  assert( (pH->htsize & (pH->htsize-1))==0 );
  h = hraw & (pH->htsize-1);
  elem = fts3FindElementByHash(pH,pKey,nKey,h);
  if( elem ){
    void *old_data = elem->data;
    if( data==0 ){
      fts3RemoveElementByHash(pH,elem,h);
    }else{
      elem->data = data;
    }
    return old_data;
  }
  if( data==0 ) return 0;
  if( (pH->htsize==0 && fts3Rehash(pH,8))
   || (pH->count>=pH->htsize && fts3Rehash(pH, pH->htsize*2))
  ){
    pH->count = 0;
    return data;
  }
  assert( pH->htsize>0 );
  new_elem = (Fts3HashElem*)fts3HashMalloc( sizeof(Fts3HashElem) );
  if( new_elem==0 ) return data;
  if( pH->copyKey && pKey!=0 ){
    new_elem->pKey = fts3HashMalloc( nKey );
    if( new_elem->pKey==0 ){
      fts3HashFree(new_elem);
      return data;
    }
    memcpy((void*)new_elem->pKey, pKey, nKey);
  }else{
    new_elem->pKey = (void*)pKey;
  }
  new_elem->nKey = nKey;
  pH->count++;
  assert( pH->htsize>0 );
  assert( (pH->htsize & (pH->htsize-1))==0 );
  h = hraw & (pH->htsize-1);
  fts3HashInsertElement(pH, &pH->ht[h], new_elem);
  new_elem->data = data;
  return 0;
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_hash.c *******************************************/
/************** Begin file fts3_porter.c *************************************/
/*
** 2006 September 30
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** Implementation of the full-text-search tokenizer that implements
** a Porter stemmer.
*/

/*
** The code in this file is only compiled if:
**
**     * The FTS3 module is being built as an extension
**       (in which case SQLITE_CORE is not defined), or
**
**     * The FTS3 module is being built into the core of
**       SQLite (in which case SQLITE_ENABLE_FTS3 is defined).
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <assert.h> */
/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */

/* #include "fts3_tokenizer.h" */

/*
** Class derived from sqlite3_tokenizer
*/
typedef struct porter_tokenizer {
  sqlite3_tokenizer base;      /* Base class */
} porter_tokenizer;

/*
** Class derived from sqlite3_tokenizer_cursor
*/
typedef struct porter_tokenizer_cursor {
  sqlite3_tokenizer_cursor base;
  const char *zInput;          /* input we are tokenizing */
  int nInput;                  /* size of the input */
  int iOffset;                 /* current position in zInput */
  int iToken;                  /* index of next token to be returned */
  char *zToken;                /* storage for current token */
  int nAllocated;              /* space allocated to zToken buffer */
} porter_tokenizer_cursor;


/*
** Create a new tokenizer instance.
*/
static int porterCreate(
  int argc, const char * const *argv,
  sqlite3_tokenizer **ppTokenizer
){
  porter_tokenizer *t;

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);

  t = (porter_tokenizer *) sqlite3_malloc(sizeof(*t));
  if( t==NULL ) return SQLITE_NOMEM;
  memset(t, 0, sizeof(*t));
  *ppTokenizer = &t->base;
  return SQLITE_OK;
}

/*
** Destroy a tokenizer
*/
static int porterDestroy(sqlite3_tokenizer *pTokenizer){
  sqlite3_free(pTokenizer);
  return SQLITE_OK;
}

/*
** Prepare to begin tokenizing a particular string.  The input
** string to be tokenized is zInput[0..nInput-1].  A cursor
** used to incrementally tokenize this string is returned in 
** *ppCursor.
*/
static int porterOpen(
  sqlite3_tokenizer *pTokenizer,         /* The tokenizer */
  const char *zInput, int nInput,        /* String to be tokenized */
  sqlite3_tokenizer_cursor **ppCursor    /* OUT: Tokenization cursor */
){
  porter_tokenizer_cursor *c;

  UNUSED_PARAMETER(pTokenizer);

  c = (porter_tokenizer_cursor *) sqlite3_malloc(sizeof(*c));
  if( c==NULL ) return SQLITE_NOMEM;

  c->zInput = zInput;
  if( zInput==0 ){
    c->nInput = 0;
  }else if( nInput<0 ){
    c->nInput = (int)strlen(zInput);
  }else{
    c->nInput = nInput;
  }
  c->iOffset = 0;                 /* start tokenizing at the beginning */
  c->iToken = 0;
  c->zToken = NULL;               /* no space allocated, yet. */
  c->nAllocated = 0;

  *ppCursor = &c->base;
  return SQLITE_OK;
}

/*
** Close a tokenization cursor previously opened by a call to
** porterOpen() above.
*/
static int porterClose(sqlite3_tokenizer_cursor *pCursor){
  porter_tokenizer_cursor *c = (porter_tokenizer_cursor *) pCursor;
  sqlite3_free(c->zToken);
  sqlite3_free(c);
  return SQLITE_OK;
}
/*
** Vowel or consonant
*/
static const char cType[] = {
   0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0,
   1, 1, 1, 2, 1
};

/*
** isConsonant() and isVowel() determine if their first character in
** the string they point to is a consonant or a vowel, according
** to Porter ruls.  
**
** A consonate is any letter other than 'a', 'e', 'i', 'o', or 'u'.
** 'Y' is a consonant unless it follows another consonant,
** in which case it is a vowel.
**
** In these routine, the letters are in reverse order.  So the 'y' rule
** is that 'y' is a consonant unless it is followed by another
** consonent.
*/
static int isVowel(const char*);
static int isConsonant(const char *z){
  int j;
  char x = *z;
  if( x==0 ) return 0;
  assert( x>='a' && x<='z' );
  j = cType[x-'a'];
  if( j<2 ) return j;
  return z[1]==0 || isVowel(z + 1);
}
static int isVowel(const char *z){
  int j;
  char x = *z;
  if( x==0 ) return 0;
  assert( x>='a' && x<='z' );
  j = cType[x-'a'];
  if( j<2 ) return 1-j;
  return isConsonant(z + 1);
}

/*
** Let any sequence of one or more vowels be represented by V and let
** C be sequence of one or more consonants.  Then every word can be
** represented as:
**
**           [C] (VC){m} [V]
**
** In prose:  A word is an optional consonant followed by zero or
** vowel-consonant pairs followed by an optional vowel.  "m" is the
** number of vowel consonant pairs.  This routine computes the value
** of m for the first i bytes of a word.
**
** Return true if the m-value for z is 1 or more.  In other words,
** return true if z contains at least one vowel that is followed
** by a consonant.
**
** In this routine z[] is in reverse order.  So we are really looking
** for an instance of a consonant followed by a vowel.
*/
static int m_gt_0(const char *z){
  while( isVowel(z) ){ z++; }
  if( *z==0 ) return 0;
  while( isConsonant(z) ){ z++; }
  return *z!=0;
}

/* Like mgt0 above except we are looking for a value of m which is
** exactly 1
*/
static int m_eq_1(const char *z){
  while( isVowel(z) ){ z++; }
  if( *z==0 ) return 0;
  while( isConsonant(z) ){ z++; }
  if( *z==0 ) return 0;
  while( isVowel(z) ){ z++; }
  if( *z==0 ) return 1;
  while( isConsonant(z) ){ z++; }
  return *z==0;
}

/* Like mgt0 above except we are looking for a value of m>1 instead
** or m>0
*/
static int m_gt_1(const char *z){
  while( isVowel(z) ){ z++; }
  if( *z==0 ) return 0;
  while( isConsonant(z) ){ z++; }
  if( *z==0 ) return 0;
  while( isVowel(z) ){ z++; }
  if( *z==0 ) return 0;
  while( isConsonant(z) ){ z++; }
  return *z!=0;
}

/*
** Return TRUE if there is a vowel anywhere within z[0..n-1]
*/
static int hasVowel(const char *z){
  while( isConsonant(z) ){ z++; }
  return *z!=0;
}

/*
** Return TRUE if the word ends in a double consonant.
**
** The text is reversed here. So we are really looking at
** the first two characters of z[].
*/
static int doubleConsonant(const char *z){
  return isConsonant(z) && z[0]==z[1];
}

/*
** Return TRUE if the word ends with three letters which
** are consonant-vowel-consonent and where the final consonant
** is not 'w', 'x', or 'y'.
**
** The word is reversed here.  So we are really checking the
** first three letters and the first one cannot be in [wxy].
*/
static int star_oh(const char *z){
  return
    isConsonant(z) &&
    z[0]!='w' && z[0]!='x' && z[0]!='y' &&
    isVowel(z+1) &&
    isConsonant(z+2);
}

/*
** If the word ends with zFrom and xCond() is true for the stem
** of the word that preceeds the zFrom ending, then change the 
** ending to zTo.
**
** The input word *pz and zFrom are both in reverse order.  zTo
** is in normal order. 
**
** Return TRUE if zFrom matches.  Return FALSE if zFrom does not
** match.  Not that TRUE is returned even if xCond() fails and
** no substitution occurs.
*/
static int stem(
  char **pz,             /* The word being stemmed (Reversed) */
  const char *zFrom,     /* If the ending matches this... (Reversed) */
  const char *zTo,       /* ... change the ending to this (not reversed) */
  int (*xCond)(const char*)   /* Condition that must be true */
){
  char *z = *pz;
  while( *zFrom && *zFrom==*z ){ z++; zFrom++; }
  if( *zFrom!=0 ) return 0;
  if( xCond && !xCond(z) ) return 1;
  while( *zTo ){
    *(--z) = *(zTo++);
  }
  *pz = z;
  return 1;
}

/*
** This is the fallback stemmer used when the porter stemmer is
** inappropriate.  The input word is copied into the output with
** US-ASCII case folding.  If the input word is too long (more
** than 20 bytes if it contains no digits or more than 6 bytes if
** it contains digits) then word is truncated to 20 or 6 bytes
** by taking 10 or 3 bytes from the beginning and end.
*/
static void copy_stemmer(const char *zIn, int nIn, char *zOut, int *pnOut){
  int i, mx, j;
  int hasDigit = 0;
  for(i=0; i<nIn; i++){
    char c = zIn[i];
    if( c>='A' && c<='Z' ){
      zOut[i] = c - 'A' + 'a';
    }else{
      if( c>='0' && c<='9' ) hasDigit = 1;
      zOut[i] = c;
    }
  }
  mx = hasDigit ? 3 : 10;
  if( nIn>mx*2 ){
    for(j=mx, i=nIn-mx; i<nIn; i++, j++){
      zOut[j] = zOut[i];
    }
    i = j;
  }
  zOut[i] = 0;
  *pnOut = i;
}


/*
** Stem the input word zIn[0..nIn-1].  Store the output in zOut.
** zOut is at least big enough to hold nIn bytes.  Write the actual
** size of the output word (exclusive of the '\0' terminator) into *pnOut.
**
** Any upper-case characters in the US-ASCII character set ([A-Z])
** are converted to lower case.  Upper-case UTF characters are
** unchanged.
**
** Words that are longer than about 20 bytes are stemmed by retaining
** a few bytes from the beginning and the end of the word.  If the
** word contains digits, 3 bytes are taken from the beginning and
** 3 bytes from the end.  For long words without digits, 10 bytes
** are taken from each end.  US-ASCII case folding still applies.
** 
** If the input word contains not digits but does characters not 
** in [a-zA-Z] then no stemming is attempted and this routine just 
** copies the input into the input into the output with US-ASCII
** case folding.
**
** Stemming never increases the length of the word.  So there is
** no chance of overflowing the zOut buffer.
*/
static void porter_stemmer(const char *zIn, int nIn, char *zOut, int *pnOut){
  int i, j;
  char zReverse[28];
  char *z, *z2;
  if( nIn<3 || nIn>=(int)sizeof(zReverse)-7 ){
    /* The word is too big or too small for the porter stemmer.
    ** Fallback to the copy stemmer */
    copy_stemmer(zIn, nIn, zOut, pnOut);
    return;
  }
  for(i=0, j=sizeof(zReverse)-6; i<nIn; i++, j--){
    char c = zIn[i];
    if( c>='A' && c<='Z' ){
      zReverse[j] = c + 'a' - 'A';
    }else if( c>='a' && c<='z' ){
      zReverse[j] = c;
    }else{
      /* The use of a character not in [a-zA-Z] means that we fallback
      ** to the copy stemmer */
      copy_stemmer(zIn, nIn, zOut, pnOut);
      return;
    }
  }
  memset(&zReverse[sizeof(zReverse)-5], 0, 5);
  z = &zReverse[j+1];


  /* Step 1a */
  if( z[0]=='s' ){
    if(
     !stem(&z, "sess", "ss", 0) &&
     !stem(&z, "sei", "i", 0)  &&
     !stem(&z, "ss", "ss", 0)
    ){
      z++;
    }
  }

  /* Step 1b */  
  z2 = z;
  if( stem(&z, "dee", "ee", m_gt_0) ){
    /* Do nothing.  The work was all in the test */
  }else if( 
     (stem(&z, "gni", "", hasVowel) || stem(&z, "de", "", hasVowel))
      && z!=z2
  ){
     if( stem(&z, "ta", "ate", 0) ||
         stem(&z, "lb", "ble", 0) ||
         stem(&z, "zi", "ize", 0) ){
       /* Do nothing.  The work was all in the test */
     }else if( doubleConsonant(z) && (*z!='l' && *z!='s' && *z!='z') ){
       z++;
     }else if( m_eq_1(z) && star_oh(z) ){
       *(--z) = 'e';
     }
  }

  /* Step 1c */
  if( z[0]=='y' && hasVowel(z+1) ){
    z[0] = 'i';
  }

  /* Step 2 */
  switch( z[1] ){
   case 'a':
     if( !stem(&z, "lanoita", "ate", m_gt_0) ){
       stem(&z, "lanoit", "tion", m_gt_0);
     }
     break;
   case 'c':
     if( !stem(&z, "icne", "ence", m_gt_0) ){
       stem(&z, "icna", "ance", m_gt_0);
     }
     break;
   case 'e':
     stem(&z, "rezi", "ize", m_gt_0);
     break;
   case 'g':
     stem(&z, "igol", "log", m_gt_0);
     break;
   case 'l':
     if( !stem(&z, "ilb", "ble", m_gt_0) 
      && !stem(&z, "illa", "al", m_gt_0)
      && !stem(&z, "iltne", "ent", m_gt_0)
      && !stem(&z, "ile", "e", m_gt_0)
     ){
       stem(&z, "ilsuo", "ous", m_gt_0);
     }
     break;
   case 'o':
     if( !stem(&z, "noitazi", "ize", m_gt_0)
      && !stem(&z, "noita", "ate", m_gt_0)
     ){
       stem(&z, "rota", "ate", m_gt_0);
     }
     break;
   case 's':
     if( !stem(&z, "msila", "al", m_gt_0)
      && !stem(&z, "ssenevi", "ive", m_gt_0)
      && !stem(&z, "ssenluf", "ful", m_gt_0)
     ){
       stem(&z, "ssensuo", "ous", m_gt_0);
     }
     break;
   case 't':
     if( !stem(&z, "itila", "al", m_gt_0)
      && !stem(&z, "itivi", "ive", m_gt_0)
     ){
       stem(&z, "itilib", "ble", m_gt_0);
     }
     break;
  }

  /* Step 3 */
  switch( z[0] ){
   case 'e':
     if( !stem(&z, "etaci", "ic", m_gt_0)
      && !stem(&z, "evita", "", m_gt_0)
     ){
       stem(&z, "ezila", "al", m_gt_0);
     }
     break;
   case 'i':
     stem(&z, "itici", "ic", m_gt_0);
     break;
   case 'l':
     if( !stem(&z, "laci", "ic", m_gt_0) ){
       stem(&z, "luf", "", m_gt_0);
     }
     break;
   case 's':
     stem(&z, "ssen", "", m_gt_0);
     break;
  }

  /* Step 4 */
  switch( z[1] ){
   case 'a':
     if( z[0]=='l' && m_gt_1(z+2) ){
       z += 2;
     }
     break;
   case 'c':
     if( z[0]=='e' && z[2]=='n' && (z[3]=='a' || z[3]=='e')  && m_gt_1(z+4)  ){
       z += 4;
     }
     break;
   case 'e':
     if( z[0]=='r' && m_gt_1(z+2) ){
       z += 2;
     }
     break;
   case 'i':
     if( z[0]=='c' && m_gt_1(z+2) ){
       z += 2;
     }
     break;
   case 'l':
     if( z[0]=='e' && z[2]=='b' && (z[3]=='a' || z[3]=='i') && m_gt_1(z+4) ){
       z += 4;
     }
     break;
   case 'n':
     if( z[0]=='t' ){
       if( z[2]=='a' ){
         if( m_gt_1(z+3) ){
           z += 3;
         }
       }else if( z[2]=='e' ){
         if( !stem(&z, "tneme", "", m_gt_1)
          && !stem(&z, "tnem", "", m_gt_1)
         ){
           stem(&z, "tne", "", m_gt_1);
         }
       }
     }
     break;
   case 'o':
     if( z[0]=='u' ){
       if( m_gt_1(z+2) ){
         z += 2;
       }
     }else if( z[3]=='s' || z[3]=='t' ){
       stem(&z, "noi", "", m_gt_1);
     }
     break;
   case 's':
     if( z[0]=='m' && z[2]=='i' && m_gt_1(z+3) ){
       z += 3;
     }
     break;
   case 't':
     if( !stem(&z, "eta", "", m_gt_1) ){
       stem(&z, "iti", "", m_gt_1);
     }
     break;
   case 'u':
     if( z[0]=='s' && z[2]=='o' && m_gt_1(z+3) ){
       z += 3;
     }
     break;
   case 'v':
   case 'z':
     if( z[0]=='e' && z[2]=='i' && m_gt_1(z+3) ){
       z += 3;
     }
     break;
  }

  /* Step 5a */
  if( z[0]=='e' ){
    if( m_gt_1(z+1) ){
      z++;
    }else if( m_eq_1(z+1) && !star_oh(z+1) ){
      z++;
    }
  }

  /* Step 5b */
  if( m_gt_1(z) && z[0]=='l' && z[1]=='l' ){
    z++;
  }

  /* z[] is now the stemmed word in reverse order.  Flip it back
  ** around into forward order and return.
  */
  *pnOut = i = (int)strlen(z);
  zOut[i] = 0;
  while( *z ){
    zOut[--i] = *(z++);
  }
}

/*
** Characters that can be part of a token.  We assume any character
** whose value is greater than 0x80 (any UTF character) can be
** part of a token.  In other words, delimiters all must have
** values of 0x7f or lower.
*/
static const char porterIdChar[] = {
/* x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 xA xB xC xD xE xF */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 3x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 4x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,  /* 5x */
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 6x */
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  /* 7x */
};
#define isDelim(C) (((ch=C)&0x80)==0 && (ch<0x30 || !porterIdChar[ch-0x30]))

/*
** Extract the next token from a tokenization cursor.  The cursor must
** have been opened by a prior call to porterOpen().
*/
static int porterNext(
  sqlite3_tokenizer_cursor *pCursor,  /* Cursor returned by porterOpen */
  const char **pzToken,               /* OUT: *pzToken is the token text */
  int *pnBytes,                       /* OUT: Number of bytes in token */
  int *piStartOffset,                 /* OUT: Starting offset of token */
  int *piEndOffset,                   /* OUT: Ending offset of token */
  int *piPosition                     /* OUT: Position integer of token */
){
  porter_tokenizer_cursor *c = (porter_tokenizer_cursor *) pCursor;
  const char *z = c->zInput;

  while( c->iOffset<c->nInput ){
    int iStartOffset, ch;

    /* Scan past delimiter characters */
    while( c->iOffset<c->nInput && isDelim(z[c->iOffset]) ){
      c->iOffset++;
    }

    /* Count non-delimiter characters. */
    iStartOffset = c->iOffset;
    while( c->iOffset<c->nInput && !isDelim(z[c->iOffset]) ){
      c->iOffset++;
    }

    if( c->iOffset>iStartOffset ){
      int n = c->iOffset-iStartOffset;
      if( n>c->nAllocated ){
        char *pNew;
        c->nAllocated = n+20;
        pNew = sqlite3_realloc(c->zToken, c->nAllocated);
        if( !pNew ) return SQLITE_NOMEM;
        c->zToken = pNew;
      }
      porter_stemmer(&z[iStartOffset], n, c->zToken, pnBytes);
      *pzToken = c->zToken;
      *piStartOffset = iStartOffset;
      *piEndOffset = c->iOffset;
      *piPosition = c->iToken++;
      return SQLITE_OK;
    }
  }
  return SQLITE_DONE;
}

/*
** The set of routines that implement the porter-stemmer tokenizer
*/
static const sqlite3_tokenizer_module porterTokenizerModule = {
  0,
  porterCreate,
  porterDestroy,
  porterOpen,
  porterClose,
  porterNext,
  0
};

/*
** Allocate a new porter tokenizer.  Return a pointer to the new
** tokenizer in *ppModule
*/
SQLITE_PRIVATE void sqlite3Fts3PorterTokenizerModule(
  sqlite3_tokenizer_module const**ppModule
){
  *ppModule = &porterTokenizerModule;
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_porter.c *****************************************/
/************** Begin file fts3_tokenizer.c **********************************/
/*
** 2007 June 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This is part of an SQLite module implementing full-text search.
** This particular file implements the generic tokenizer interface.
*/

/*
** The code in this file is only compiled if:
**
**     * The FTS3 module is being built as an extension
**       (in which case SQLITE_CORE is not defined), or
**
**     * The FTS3 module is being built into the core of
**       SQLite (in which case SQLITE_ENABLE_FTS3 is defined).
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <assert.h> */
/* #include <string.h> */

/*
** Implementation of the SQL scalar function for accessing the underlying 
** hash table. This function may be called as follows:
**
**   SELECT <function-name>(<key-name>);
**   SELECT <function-name>(<key-name>, <pointer>);
**
** where <function-name> is the name passed as the second argument
** to the sqlite3Fts3InitHashTable() function (e.g. 'fts3_tokenizer').
**
** If the <pointer> argument is specified, it must be a blob value
** containing a pointer to be stored as the hash data corresponding
** to the string <key-name>. If <pointer> is not specified, then
** the string <key-name> must already exist in the has table. Otherwise,
** an error is returned.
**
** Whether or not the <pointer> argument is specified, the value returned
** is a blob containing the pointer stored as the hash data corresponding
** to string <key-name> (after the hash-table is updated, if applicable).
*/
static void scalarFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  Fts3Hash *pHash;
  void *pPtr = 0;
  const unsigned char *zName;
  int nName;

  assert( argc==1 || argc==2 );

  pHash = (Fts3Hash *)sqlite3_user_data(context);

  zName = sqlite3_value_text(argv[0]);
  nName = sqlite3_value_bytes(argv[0])+1;

  if( argc==2 ){
    void *pOld;
    int n = sqlite3_value_bytes(argv[1]);
    if( zName==0 || n!=sizeof(pPtr) ){
      sqlite3_result_error(context, "argument type mismatch", -1);
      return;
    }
    pPtr = *(void **)sqlite3_value_blob(argv[1]);
    pOld = sqlite3Fts3HashInsert(pHash, (void *)zName, nName, pPtr);
    if( pOld==pPtr ){
      sqlite3_result_error(context, "out of memory", -1);
      return;
    }
  }else{
    if( zName ){
      pPtr = sqlite3Fts3HashFind(pHash, zName, nName);
    }
    if( !pPtr ){
      char *zErr = sqlite3_mprintf("unknown tokenizer: %s", zName);
      sqlite3_result_error(context, zErr, -1);
      sqlite3_free(zErr);
      return;
    }
  }

  sqlite3_result_blob(context, (void *)&pPtr, sizeof(pPtr), SQLITE_TRANSIENT);
}

SQLITE_PRIVATE int sqlite3Fts3IsIdChar(char c){
  static const char isFtsIdChar[] = {
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 0x */
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 1x */
      0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  /* 2x */
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,  /* 3x */
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 4x */
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,  /* 5x */
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  /* 6x */
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,  /* 7x */
  };
  return (c&0x80 || isFtsIdChar[(int)(c)]);
}

SQLITE_PRIVATE const char *sqlite3Fts3NextToken(const char *zStr, int *pn){
  const char *z1;
  const char *z2 = 0;

  /* Find the start of the next token. */
  z1 = zStr;
  while( z2==0 ){
    char c = *z1;
    switch( c ){
      case '\0': return 0;        /* No more tokens here */
      case '\'':
      case '"':
      case '`': {
        z2 = z1;
        while( *++z2 && (*z2!=c || *++z2==c) );
        break;
      }
      case '[':
        z2 = &z1[1];
        while( *z2 && z2[0]!=']' ) z2++;
        if( *z2 ) z2++;
        break;

      default:
        if( sqlite3Fts3IsIdChar(*z1) ){
          z2 = &z1[1];
          while( sqlite3Fts3IsIdChar(*z2) ) z2++;
        }else{
          z1++;
        }
    }
  }

  *pn = (int)(z2-z1);
  return z1;
}

SQLITE_PRIVATE int sqlite3Fts3InitTokenizer(
  Fts3Hash *pHash,                /* Tokenizer hash table */
  const char *zArg,               /* Tokenizer name */
  sqlite3_tokenizer **ppTok,      /* OUT: Tokenizer (if applicable) */
  char **pzErr                    /* OUT: Set to malloced error message */
){
  int rc;
  char *z = (char *)zArg;
  int n = 0;
  char *zCopy;
  char *zEnd;                     /* Pointer to nul-term of zCopy */
  sqlite3_tokenizer_module *m;

  zCopy = sqlite3_mprintf("%s", zArg);
  if( !zCopy ) return SQLITE_NOMEM;
  zEnd = &zCopy[strlen(zCopy)];

  z = (char *)sqlite3Fts3NextToken(zCopy, &n);
  if( z==0 ){
    assert( n==0 );
    z = zCopy;
  }
  z[n] = '\0';
  sqlite3Fts3Dequote(z);

  m = (sqlite3_tokenizer_module *)sqlite3Fts3HashFind(pHash,z,(int)strlen(z)+1);
  if( !m ){
    sqlite3Fts3ErrMsg(pzErr, "unknown tokenizer: %s", z);
    rc = SQLITE_ERROR;
  }else{
    char const **aArg = 0;
    int iArg = 0;
    z = &z[n+1];
    while( z<zEnd && (NULL!=(z = (char *)sqlite3Fts3NextToken(z, &n))) ){
      int nNew = sizeof(char *)*(iArg+1);
      char const **aNew = (const char **)sqlite3_realloc((void *)aArg, nNew);
      if( !aNew ){
        sqlite3_free(zCopy);
        sqlite3_free((void *)aArg);
        return SQLITE_NOMEM;
      }
      aArg = aNew;
      aArg[iArg++] = z;
      z[n] = '\0';
      sqlite3Fts3Dequote(z);
      z = &z[n+1];
    }
    rc = m->xCreate(iArg, aArg, ppTok);
    assert( rc!=SQLITE_OK || *ppTok );
    if( rc!=SQLITE_OK ){
      sqlite3Fts3ErrMsg(pzErr, "unknown tokenizer");
    }else{
      (*ppTok)->pModule = m; 
    }
    sqlite3_free((void *)aArg);
  }

  sqlite3_free(zCopy);
  return rc;
}


#ifdef SQLITE_TEST

#include <tcl.h>
/* #include <string.h> */

/*
** Implementation of a special SQL scalar function for testing tokenizers 
** designed to be used in concert with the Tcl testing framework. This
** function must be called with two or more arguments:
**
**   SELECT <function-name>(<key-name>, ..., <input-string>);
**
** where <function-name> is the name passed as the second argument
** to the sqlite3Fts3InitHashTable() function (e.g. 'fts3_tokenizer')
** concatenated with the string '_test' (e.g. 'fts3_tokenizer_test').
**
** The return value is a string that may be interpreted as a Tcl
** list. For each token in the <input-string>, three elements are
** added to the returned list. The first is the token position, the 
** second is the token text (folded, stemmed, etc.) and the third is the
** substring of <input-string> associated with the token. For example, 
** using the built-in "simple" tokenizer:
**
**   SELECT fts_tokenizer_test('simple', 'I don't see how');
**
** will return the string:
**
**   "{0 i I 1 dont don't 2 see see 3 how how}"
**   
*/
static void testFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  Fts3Hash *pHash;
  sqlite3_tokenizer_module *p;
  sqlite3_tokenizer *pTokenizer = 0;
  sqlite3_tokenizer_cursor *pCsr = 0;

  const char *zErr = 0;

  const char *zName;
  int nName;
  const char *zInput;
  int nInput;

  const char *azArg[64];

  const char *zToken;
  int nToken = 0;
  int iStart = 0;
  int iEnd = 0;
  int iPos = 0;
  int i;

  Tcl_Obj *pRet;

  if( argc<2 ){
    sqlite3_result_error(context, "insufficient arguments", -1);
    return;
  }

  nName = sqlite3_value_bytes(argv[0]);
  zName = (const char *)sqlite3_value_text(argv[0]);
  nInput = sqlite3_value_bytes(argv[argc-1]);
  zInput = (const char *)sqlite3_value_text(argv[argc-1]);

  pHash = (Fts3Hash *)sqlite3_user_data(context);
  p = (sqlite3_tokenizer_module *)sqlite3Fts3HashFind(pHash, zName, nName+1);

  if( !p ){
    char *zErr2 = sqlite3_mprintf("unknown tokenizer: %s", zName);
    sqlite3_result_error(context, zErr2, -1);
    sqlite3_free(zErr2);
    return;
  }

  pRet = Tcl_NewObj();
  Tcl_IncrRefCount(pRet);

  for(i=1; i<argc-1; i++){
    azArg[i-1] = (const char *)sqlite3_value_text(argv[i]);
  }

  if( SQLITE_OK!=p->xCreate(argc-2, azArg, &pTokenizer) ){
    zErr = "error in xCreate()";
    goto finish;
  }
  pTokenizer->pModule = p;
  if( sqlite3Fts3OpenTokenizer(pTokenizer, 0, zInput, nInput, &pCsr) ){
    zErr = "error in xOpen()";
    goto finish;
  }

  while( SQLITE_OK==p->xNext(pCsr, &zToken, &nToken, &iStart, &iEnd, &iPos) ){
    Tcl_ListObjAppendElement(0, pRet, Tcl_NewIntObj(iPos));
    Tcl_ListObjAppendElement(0, pRet, Tcl_NewStringObj(zToken, nToken));
    zToken = &zInput[iStart];
    nToken = iEnd-iStart;
    Tcl_ListObjAppendElement(0, pRet, Tcl_NewStringObj(zToken, nToken));
  }

  if( SQLITE_OK!=p->xClose(pCsr) ){
    zErr = "error in xClose()";
    goto finish;
  }
  if( SQLITE_OK!=p->xDestroy(pTokenizer) ){
    zErr = "error in xDestroy()";
    goto finish;
  }

finish:
  if( zErr ){
    sqlite3_result_error(context, zErr, -1);
  }else{
    sqlite3_result_text(context, Tcl_GetString(pRet), -1, SQLITE_TRANSIENT);
  }
  Tcl_DecrRefCount(pRet);
}

static
int registerTokenizer(
  sqlite3 *db, 
  char *zName, 
  const sqlite3_tokenizer_module *p
){
  int rc;
  sqlite3_stmt *pStmt;
  const char zSql[] = "SELECT fts3_tokenizer(?, ?)";

  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  sqlite3_bind_text(pStmt, 1, zName, -1, SQLITE_STATIC);
  sqlite3_bind_blob(pStmt, 2, &p, sizeof(p), SQLITE_STATIC);
  sqlite3_step(pStmt);

  return sqlite3_finalize(pStmt);
}

static
int queryTokenizer(
  sqlite3 *db, 
  char *zName,  
  const sqlite3_tokenizer_module **pp
){
  int rc;
  sqlite3_stmt *pStmt;
  const char zSql[] = "SELECT fts3_tokenizer(?)";

  *pp = 0;
  rc = sqlite3_prepare_v2(db, zSql, -1, &pStmt, 0);
  if( rc!=SQLITE_OK ){
    return rc;
  }

  sqlite3_bind_text(pStmt, 1, zName, -1, SQLITE_STATIC);
  if( SQLITE_ROW==sqlite3_step(pStmt) ){
    if( sqlite3_column_type(pStmt, 0)==SQLITE_BLOB ){
      memcpy((void *)pp, sqlite3_column_blob(pStmt, 0), sizeof(*pp));
    }
  }

  return sqlite3_finalize(pStmt);
}

SQLITE_PRIVATE void sqlite3Fts3SimpleTokenizerModule(sqlite3_tokenizer_module const**ppModule);

/*
** Implementation of the scalar function fts3_tokenizer_internal_test().
** This function is used for testing only, it is not included in the
** build unless SQLITE_TEST is defined.
**
** The purpose of this is to test that the fts3_tokenizer() function
** can be used as designed by the C-code in the queryTokenizer and
** registerTokenizer() functions above. These two functions are repeated
** in the README.tokenizer file as an example, so it is important to
** test them.
**
** To run the tests, evaluate the fts3_tokenizer_internal_test() scalar
** function with no arguments. An assert() will fail if a problem is
** detected. i.e.:
**
**     SELECT fts3_tokenizer_internal_test();
**
*/
static void intTestFunc(
  sqlite3_context *context,
  int argc,
  sqlite3_value **argv
){
  int rc;
  const sqlite3_tokenizer_module *p1;
  const sqlite3_tokenizer_module *p2;
  sqlite3 *db = (sqlite3 *)sqlite3_user_data(context);

  UNUSED_PARAMETER(argc);
  UNUSED_PARAMETER(argv);

  /* Test the query function */
  sqlite3Fts3SimpleTokenizerModule(&p1);
  rc = queryTokenizer(db, "simple", &p2);
  assert( rc==SQLITE_OK );
  assert( p1==p2 );
  rc = queryTokenizer(db, "nosuchtokenizer", &p2);
  assert( rc==SQLITE_ERROR );
  assert( p2==0 );
  assert( 0==strcmp(sqlite3_errmsg(db), "unknown tokenizer: nosuchtokenizer") );

  /* Test the storage function */
  rc = registerTokenizer(db, "nosuchtokenizer", p1);
  assert( rc==SQLITE_OK );
  rc = queryTokenizer(db, "nosuchtokenizer", &p2);
  assert( rc==SQLITE_OK );
  assert( p2==p1 );

  sqlite3_result_text(context, "ok", -1, SQLITE_STATIC);
}

#endif

/*
** Set up SQL objects in database db used to access the contents of
** the hash table pointed to by argument pHash. The hash table must
** been initialized to use string keys, and to take a private copy 
** of the key when a value is inserted. i.e. by a call similar to:
**
**    sqlite3Fts3HashInit(pHash, FTS3_HASH_STRING, 1);
**
** This function adds a scalar function (see header comment above
** scalarFunc() in this file for details) and, if ENABLE_TABLE is
** defined at compilation time, a temporary virtual table (see header 
** comment above struct HashTableVtab) to the database schema. Both 
** provide read/write access to the contents of *pHash.
**
** The third argument to this function, zName, is used as the name
** of both the scalar and, if created, the virtual table.
*/
SQLITE_PRIVATE int sqlite3Fts3InitHashTable(
  sqlite3 *db, 
  Fts3Hash *pHash, 
  const char *zName
){
  int rc = SQLITE_OK;
  void *p = (void *)pHash;
  const int any = SQLITE_ANY;

#ifdef SQLITE_TEST
  char *zTest = 0;
  char *zTest2 = 0;
  void *pdb = (void *)db;
  zTest = sqlite3_mprintf("%s_test", zName);
  zTest2 = sqlite3_mprintf("%s_internal_test", zName);
  if( !zTest || !zTest2 ){
    rc = SQLITE_NOMEM;
  }
#endif

  if( SQLITE_OK==rc ){
    rc = sqlite3_create_function(db, zName, 1, any, p, scalarFunc, 0, 0);
  }
  if( SQLITE_OK==rc ){
    rc = sqlite3_create_function(db, zName, 2, any, p, scalarFunc, 0, 0);
  }
#ifdef SQLITE_TEST
  if( SQLITE_OK==rc ){
    rc = sqlite3_create_function(db, zTest, -1, any, p, testFunc, 0, 0);
  }
  if( SQLITE_OK==rc ){
    rc = sqlite3_create_function(db, zTest2, 0, any, pdb, intTestFunc, 0, 0);
  }
#endif

#ifdef SQLITE_TEST
  sqlite3_free(zTest);
  sqlite3_free(zTest2);
#endif

  return rc;
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_tokenizer.c **************************************/
/************** Begin file fts3_tokenizer1.c *********************************/
/*
** 2006 Oct 10
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** Implementation of the "simple" full-text-search tokenizer.
*/

/*
** The code in this file is only compiled if:
**
**     * The FTS3 module is being built as an extension
**       (in which case SQLITE_CORE is not defined), or
**
**     * The FTS3 module is being built into the core of
**       SQLite (in which case SQLITE_ENABLE_FTS3 is defined).
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <assert.h> */
/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */

/* #include "fts3_tokenizer.h" */

typedef struct simple_tokenizer {
  sqlite3_tokenizer base;
  char delim[128];             /* flag ASCII delimiters */
} simple_tokenizer;

typedef struct simple_tokenizer_cursor {
  sqlite3_tokenizer_cursor base;
  const char *pInput;          /* input we are tokenizing */
  int nBytes;                  /* size of the input */
  int iOffset;                 /* current position in pInput */
  int iToken;                  /* index of next token to be returned */
  char *pToken;                /* storage for current token */
  int nTokenAllocated;         /* space allocated to zToken buffer */
} simple_tokenizer_cursor;


static int simpleDelim(simple_tokenizer *t, unsigned char c){
  return c<0x80 && t->delim[c];
}
static int fts3_isalnum(int x){
  return (x>='0' && x<='9') || (x>='A' && x<='Z') || (x>='a' && x<='z');
}

/*
** Create a new tokenizer instance.
*/
static int simpleCreate(
  int argc, const char * const *argv,
  sqlite3_tokenizer **ppTokenizer
){
  simple_tokenizer *t;

  t = (simple_tokenizer *) sqlite3_malloc(sizeof(*t));
  if( t==NULL ) return SQLITE_NOMEM;
  memset(t, 0, sizeof(*t));

  /* TODO(shess) Delimiters need to remain the same from run to run,
  ** else we need to reindex.  One solution would be a meta-table to
  ** track such information in the database, then we'd only want this
  ** information on the initial create.
  */
  if( argc>1 ){
    int i, n = (int)strlen(argv[1]);
    for(i=0; i<n; i++){
      unsigned char ch = argv[1][i];
      /* We explicitly don't support UTF-8 delimiters for now. */
      if( ch>=0x80 ){
        sqlite3_free(t);
        return SQLITE_ERROR;
      }
      t->delim[ch] = 1;
    }
  } else {
    /* Mark non-alphanumeric ASCII characters as delimiters */
    int i;
    for(i=1; i<0x80; i++){
      t->delim[i] = !fts3_isalnum(i) ? -1 : 0;
    }
  }

  *ppTokenizer = &t->base;
  return SQLITE_OK;
}

/*
** Destroy a tokenizer
*/
static int simpleDestroy(sqlite3_tokenizer *pTokenizer){
  sqlite3_free(pTokenizer);
  return SQLITE_OK;
}

/*
** Prepare to begin tokenizing a particular string.  The input
** string to be tokenized is pInput[0..nBytes-1].  A cursor
** used to incrementally tokenize this string is returned in 
** *ppCursor.
*/
static int simpleOpen(
  sqlite3_tokenizer *pTokenizer,         /* The tokenizer */
  const char *pInput, int nBytes,        /* String to be tokenized */
  sqlite3_tokenizer_cursor **ppCursor    /* OUT: Tokenization cursor */
){
  simple_tokenizer_cursor *c;

  UNUSED_PARAMETER(pTokenizer);

  c = (simple_tokenizer_cursor *) sqlite3_malloc(sizeof(*c));
  if( c==NULL ) return SQLITE_NOMEM;

  c->pInput = pInput;
  if( pInput==0 ){
    c->nBytes = 0;
  }else if( nBytes<0 ){
    c->nBytes = (int)strlen(pInput);
  }else{
    c->nBytes = nBytes;
  }
  c->iOffset = 0;                 /* start tokenizing at the beginning */
  c->iToken = 0;
  c->pToken = NULL;               /* no space allocated, yet. */
  c->nTokenAllocated = 0;

  *ppCursor = &c->base;
  return SQLITE_OK;
}

/*
** Close a tokenization cursor previously opened by a call to
** simpleOpen() above.
*/
static int simpleClose(sqlite3_tokenizer_cursor *pCursor){
  simple_tokenizer_cursor *c = (simple_tokenizer_cursor *) pCursor;
  sqlite3_free(c->pToken);
  sqlite3_free(c);
  return SQLITE_OK;
}

/*
** Extract the next token from a tokenization cursor.  The cursor must
** have been opened by a prior call to simpleOpen().
*/
static int simpleNext(
  sqlite3_tokenizer_cursor *pCursor,  /* Cursor returned by simpleOpen */
  const char **ppToken,               /* OUT: *ppToken is the token text */
  int *pnBytes,                       /* OUT: Number of bytes in token */
  int *piStartOffset,                 /* OUT: Starting offset of token */
  int *piEndOffset,                   /* OUT: Ending offset of token */
  int *piPosition                     /* OUT: Position integer of token */
){
  simple_tokenizer_cursor *c = (simple_tokenizer_cursor *) pCursor;
  simple_tokenizer *t = (simple_tokenizer *) pCursor->pTokenizer;
  unsigned char *p = (unsigned char *)c->pInput;

  while( c->iOffset<c->nBytes ){
    int iStartOffset;

    /* Scan past delimiter characters */
    while( c->iOffset<c->nBytes && simpleDelim(t, p[c->iOffset]) ){
      c->iOffset++;
    }

    /* Count non-delimiter characters. */
    iStartOffset = c->iOffset;
    while( c->iOffset<c->nBytes && !simpleDelim(t, p[c->iOffset]) ){
      c->iOffset++;
    }

    if( c->iOffset>iStartOffset ){
      int i, n = c->iOffset-iStartOffset;
      if( n>c->nTokenAllocated ){
        char *pNew;
        c->nTokenAllocated = n+20;
        pNew = sqlite3_realloc(c->pToken, c->nTokenAllocated);
        if( !pNew ) return SQLITE_NOMEM;
        c->pToken = pNew;
      }
      for(i=0; i<n; i++){
        /* TODO(shess) This needs expansion to handle UTF-8
        ** case-insensitivity.
        */
        unsigned char ch = p[iStartOffset+i];
        c->pToken[i] = (char)((ch>='A' && ch<='Z') ? ch-'A'+'a' : ch);
      }
      *ppToken = c->pToken;
      *pnBytes = n;
      *piStartOffset = iStartOffset;
      *piEndOffset = c->iOffset;
      *piPosition = c->iToken++;

      return SQLITE_OK;
    }
  }
  return SQLITE_DONE;
}

/*
** The set of routines that implement the simple tokenizer
*/
static const sqlite3_tokenizer_module simpleTokenizerModule = {
  0,
  simpleCreate,
  simpleDestroy,
  simpleOpen,
  simpleClose,
  simpleNext,
  0,
};

/*
** Allocate a new simple tokenizer.  Return a pointer to the new
** tokenizer in *ppModule
*/
SQLITE_PRIVATE void sqlite3Fts3SimpleTokenizerModule(
  sqlite3_tokenizer_module const**ppModule
){
  *ppModule = &simpleTokenizerModule;
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_tokenizer1.c *************************************/
/************** Begin file fts3_tokenize_vtab.c ******************************/
/*
** 2013 Apr 22
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
******************************************************************************
**
** This file contains code for the "fts3tokenize" virtual table module.
** An fts3tokenize virtual table is created as follows:
**
**   CREATE VIRTUAL TABLE <tbl> USING fts3tokenize(
**       <tokenizer-name>, <arg-1>, ...
**   );
**
** The table created has the following schema:
**
**   CREATE TABLE <tbl>(input, token, start, end, position)
**
** When queried, the query must include a WHERE clause of type:
**
**   input = <string>
**
** The virtual table module tokenizes this <string>, using the FTS3 
** tokenizer specified by the arguments to the CREATE VIRTUAL TABLE 
** statement and returns one row for each token in the result. With
** fields set as follows:
**
**   input:   Always set to a copy of <string>
**   token:   A token from the input.
**   start:   Byte offset of the token within the input <string>.
**   end:     Byte offset of the byte immediately following the end of the
**            token within the input string.
**   pos:     Token offset of token within input.
**
*/
/* #include "fts3Int.h" */
#if !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3)

/* #include <string.h> */
/* #include <assert.h> */

typedef struct Fts3tokTable Fts3tokTable;
typedef struct Fts3tokCursor Fts3tokCursor;

/*
** Virtual table structure.
*/
struct Fts3tokTable {
  sqlite3_vtab base;              /* Base class used by SQLite core */
  const sqlite3_tokenizer_module *pMod;
  sqlite3_tokenizer *pTok;
};

/*
** Virtual table cursor structure.
*/
struct Fts3tokCursor {
  sqlite3_vtab_cursor base;       /* Base class used by SQLite core */
  char *zInput;                   /* Input string */
  sqlite3_tokenizer_cursor *pCsr; /* Cursor to iterate through zInput */
  int iRowid;                     /* Current 'rowid' value */
  const char *zToken;             /* Current 'token' value */
  int nToken;                     /* Size of zToken in bytes */
  int iStart;                     /* Current 'start' value */
  int iEnd;                       /* Current 'end' value */
  int iPos;                       /* Current 'pos' value */
};

/*
** Query FTS for the tokenizer implementation named zName.
*/
static int fts3tokQueryTokenizer(
  Fts3Hash *pHash,
  const char *zName,
  const sqlite3_tokenizer_module **pp,
  char **pzErr
){
  sqlite3_tokenizer_module *p;
  int nName = (int)strlen(zName);

  p = (sqlite3_tokenizer_module *)sqlite3Fts3HashFind(pHash, zName, nName+1);
  if( !p ){
    sqlite3Fts3ErrMsg(pzErr, "unknown tokenizer: %s", zName);
    return SQLITE_ERROR;
  }

  *pp = p;
  return SQLITE_OK;
}

/*
** The second argument, argv[], is an array of pointers to nul-terminated
** strings. This function makes a copy of the array and strings into a 
** single block of memory. It then dequotes any of the strings that appear
** to be quoted.
**
** If successful, output parameter *pazDequote is set to point at the
** array of dequoted strings and SQLITE_OK is returned. The caller is
** responsible for eventually calling sqlite3_free() to free the array
** in this case. Or, if an error occurs, an SQLite error code is returned.
** The final value of *pazDequote is undefined in this case.
*/
static int fts3tokDequoteArray(
  int argc,                       /* Number of elements in argv[] */
  const char * const *argv,       /* Input array */
  char ***pazDequote              /* Output array */
){
  int rc = SQLITE_OK;             /* Return code */
  if( argc==0 ){
    *pazDequote = 0;
  }else{
    int i;
    int nByte = 0;
    char **azDequote;

    for(i=0; i<argc; i++){
      nByte += (int)(strlen(argv[i]) + 1);
    }

    *pazDequote = azDequote = sqlite3_malloc(sizeof(char *)*argc + nByte);
    if( azDequote==0 ){
      rc = SQLITE_NOMEM;
    }else{
      char *pSpace = (char *)&azDequote[argc];
      for(i=0; i<argc; i++){
        int n = (int)strlen(argv[i]);
        azDequote[i] = pSpace;
        memcpy(pSpace, argv[i], n+1);
        sqlite3Fts3Dequote(pSpace);
        pSpace += (n+1);
      }
    }
  }

  return rc;
}

/*
** Schema of the tokenizer table.
*/
#define FTS3_TOK_SCHEMA "CREATE TABLE x(input, token, start, end, position)"

/*
** This function does all the work for both the xConnect and xCreate methods.
** These tables have no persistent representation of their own, so xConnect
** and xCreate are identical operations.
**
**   argv[0]: module name
**   argv[1]: database name 
**   argv[2]: table name
**   argv[3]: first argument (tokenizer name)
*/
static int fts3tokConnectMethod(
  sqlite3 *db,                    /* Database connection */
  void *pHash,                    /* Hash table of tokenizers */
  int argc,                       /* Number of elements in argv array */
  const char * const *argv,       /* xCreate/xConnect argument array */
  sqlite3_vtab **ppVtab,          /* OUT: New sqlite3_vtab object */
  char **pzErr                    /* OUT: sqlite3_malloc'd error message */
){
  Fts3tokTable *pTab = 0;
  const sqlite3_tokenizer_module *pMod = 0;
  sqlite3_tokenizer *pTok = 0;
  int rc;
  char **azDequote = 0;
  int nDequote;

  rc = sqlite3_declare_vtab(db, FTS3_TOK_SCHEMA);
  if( rc!=SQLITE_OK ) return rc;

  nDequote = argc-3;
  rc = fts3tokDequoteArray(nDequote, &argv[3], &azDequote);

  if( rc==SQLITE_OK ){
    const char *zModule;
    if( nDequote<1 ){
      zModule = "simple";
    }else{
      zModule = azDequote[0];
    }
    rc = fts3tokQueryTokenizer((Fts3Hash*)pHash, zModule, &pMod, pzErr);
  }

  assert( (rc==SQLITE_OK)==(pMod!=0) );
  if( rc==SQLITE_OK ){
    const char * const *azArg = (const char * const *)&azDequote[1];
    rc = pMod->xCreate((nDequote>1 ? nDequote-1 : 0), azArg, &pTok);
  }

  if( rc==SQLITE_OK ){
    pTab = (Fts3tokTable *)sqlite3_malloc(sizeof(Fts3tokTable));
    if( pTab==0 ){
      rc = SQLITE_NOMEM;
    }
  }

  if( rc==SQLITE_OK ){
    memset(pTab, 0, sizeof(Fts3tokTable));
    pTab->pMod = pMod;
    pTab->pTok = pTok;
    *ppVtab = &pTab->base;
  }else{
    if( pTok ){
      pMod->xDestroy(pTok);
    }
  }

  sqlite3_free(azDequote);
  return rc;
}

/*
** This function does the work for both the xDisconnect and xDestroy methods.
** These tables have no persistent representation of their own, so xDisconnect
** and xDestroy are identical operations.
*/
static int fts3tokDisconnectMethod(sqlite3_vtab *pVtab){
  Fts3tokTable *pTab = (Fts3tokTable *)pVtab;

  pTab->pMod->xDestroy(pTab->pTok);
  sqlite3_free(pTab);
  return SQLITE_OK;
}

/*
** xBestIndex - Analyze a WHERE and ORDER BY clause.
*/
static int fts3tokBestIndexMethod(
  sqlite3_vtab *pVTab, 
  sqlite3_index_info *pInfo
){
  int i;
  UNUSED_PARAMETER(pVTab);

  for(i=0; i<pInfo->nConstraint; i++){
    if( pInfo->aConstraint[i].usable 
     && pInfo->aConstraint[i].iColumn==0 
     && pInfo->aConstraint[i].op==SQLITE_INDEX_CONSTRAINT_EQ 
    ){
      pInfo->idxNum = 1;
      pInfo->aConstraintUsage[i].argvIndex = 1;
      pInfo->aConstraintUsage[i].omit = 1;
      pInfo->estimatedCost = 1;
      return SQLITE_OK;
    }
  }

  pInfo->idxNum = 0;
  assert( pInfo->estimatedCost>1000000.0 );

  return SQLITE_OK;
}

/*
** xOpen - Open a cursor.
*/
static int fts3tokOpenMethod(sqlite3_vtab *pVTab, sqlite3_vtab_cursor **ppCsr){
  Fts3tokCursor *pCsr;
  UNUSED_PARAMETER(pVTab);

  pCsr = (Fts3tokCursor *)sqlite3_malloc(sizeof(Fts3tokCursor));
  if( pCsr==0 ){
    return SQLITE_NOMEM;
  }
  memset(pCsr, 0, sizeof(Fts3tokCursor));

  *ppCsr = (sqlite3_vtab_cursor *)pCsr;
  return SQLITE_OK;
}

/*
** Reset the tokenizer cursor passed as the only argument. As if it had
** just been returned by fts3tokOpenMethod().
*/
static void fts3tokResetCursor(Fts3tokCursor *pCsr){
  if( pCsr->pCsr ){
    Fts3tokTable *pTab = (Fts3tokTable *)(pCsr->base.pVtab);
    pTab->pMod->xClose(pCsr->pCsr);
    pCsr->pCsr = 0;
  }
  sqlite3_free(pCsr->zInput);
  pCsr->zInput = 0;
  pCsr->zToken = 0;
  pCsr->nToken = 0;
  pCsr->iStart = 0;
  pCsr->iEnd = 0;
  pCsr->iPos = 0;
  pCsr->iRowid = 0;
}

/*
** xClose - Close a cursor.
*/
static int fts3tokCloseMethod(sqlite3_vtab_cursor *pCursor){
  Fts3tokCursor *pCsr = (Fts3tokCursor *)pCursor;

  fts3tokResetCursor(pCsr);
  sqlite3_free(pCsr);
  return SQLITE_OK;
}

/*
** xNext - Advance the cursor to the next row, if any.
*/
static int fts3tokNextMethod(sqlite3_vtab_cursor *pCursor){
  Fts3tokCursor *pCsr = (Fts3tokCursor *)pCursor;
  Fts3tokTable *pTab = (Fts3tokTable *)(pCursor->pVtab);
  int rc;                         /* Return code */

  pCsr->iRowid++;
  rc = pTab->pMod->xNext(pCsr->pCsr,
      &pCsr->zToken, &pCsr->nToken,
      &pCsr->iStart, &pCsr->iEnd, &pCsr->iPos
  );

  if( rc!=SQLITE_OK ){
    fts3tokResetCursor(pCsr);
    if( rc==SQLITE_DONE ) rc = SQLITE_OK;
  }

  return rc;
}

/*
** xFilter - Initialize a cursor to point at the start of its data.
*/
static int fts3tokFilterMethod(
  sqlite3_vtab_cursor *pCursor,   /* The cursor used for this query */
  int idxNum,                     /* Strategy index */
  const char *idxStr,             /* Unused */
  int nVal,                       /* Number of elements in apVal */
  sqlite3_value **apVal           /* Arguments for the indexing scheme */
){
  int rc = SQLITE_ERROR;
  Fts3tokCursor *pCsr = (Fts3tokCursor *)pCursor;
  Fts3tokTable *pTab = (Fts3tokTable *)(pCursor->pVtab);
  UNUSED_PARAMETER(idxStr);
  UNUSED_PARAMETER(nVal);

  fts3tokResetCursor(pCsr);
  if( idxNum==1 ){
    const char *zByte = (const char *)sqlite3_value_text(apVal[0]);
    int nByte = sqlite3_value_bytes(apVal[0]);
    pCsr->zInput = sqlite3_malloc(nByte+1);
    if( pCsr->zInput==0 ){
      rc = SQLITE_NOMEM;
    }else{
      memcpy(pCsr->zInput, zByte, nByte);
      pCsr->zInput[nByte] = 0;
      rc = pTab->pMod->xOpen(pTab->pTok, pCsr->zInput, nByte, &pCsr->pCsr);
      if( rc==SQLITE_OK ){
        pCsr->pCsr->pTokenizer = pTab->pTok;
      }
    }
  }

  if( rc!=SQLITE_OK ) return rc;
  return fts3tokNextMethod(pCursor);
}

/*
** xEof - Return true if the cursor is at EOF, or false otherwise.
*/
static int fts3tokEofMethod(sqlite3_vtab_cursor *pCursor){
  Fts3tokCursor *pCsr = (Fts3tokCursor *)pCursor;
  return (pCsr->zToken==0);
}

/*
** xColumn - Return a column value.
*/
static int fts3tokColumnMethod(
  sqlite3_vtab_cursor *pCursor,   /* Cursor to retrieve value from */
  sqlite3_context *pCtx,          /* Context for sqlite3_result_xxx() calls */
  int iCol                        /* Index of column to read value from */
){
  Fts3tokCursor *pCsr = (Fts3tokCursor *)pCursor;

  /* CREATE TABLE x(input, token, start, end, position) */
  switch( iCol ){
    case 0:
      sqlite3_result_text(pCtx, pCsr->zInput, -1, SQLITE_TRANSIENT);
      break;
    case 1:
      sqlite3_result_text(pCtx, pCsr->zToken, pCsr->nToken, SQLITE_TRANSIENT);
      break;
    case 2:
      sqlite3_result_int(pCtx, pCsr->iStart);
      break;
    case 3:
      sqlite3_result_int(pCtx, pCsr->iEnd);
      break;
    default:
      assert( iCol==4 );
      sqlite3_result_int(pCtx, pCsr->iPos);
      break;
  }
  return SQLITE_OK;
}

/*
** xRowid - Return the current rowid for the cursor.
*/
static int fts3tokRowidMethod(
  sqlite3_vtab_cursor *pCursor,   /* Cursor to retrieve value from */
  sqlite_int64 *pRowid            /* OUT: Rowid value */
){
  Fts3tokCursor *pCsr = (Fts3tokCursor *)pCursor;
  *pRowid = (sqlite3_int64)pCsr->iRowid;
  return SQLITE_OK;
}

/*
** Register the fts3tok module with database connection db. Return SQLITE_OK
** if successful or an error code if sqlite3_create_module() fails.
*/
SQLITE_PRIVATE int sqlite3Fts3InitTok(sqlite3 *db, Fts3Hash *pHash){
  static const sqlite3_module fts3tok_module = {
     0,                           /* iVersion      */
     fts3tokConnectMethod,        /* xCreate       */
     fts3tokConnectMethod,        /* xConnect      */
     fts3tokBestIndexMethod,      /* xBestIndex    */
     fts3tokDisconnectMethod,     /* xDisconnect   */
     fts3tokDisconnectMethod,     /* xDestroy      */
     fts3tokOpenMethod,           /* xOpen         */
     fts3tokCloseMethod,          /* xClose        */
     fts3tokFilterMethod,         /* xFilter       */
     fts3tokNextMethod,           /* xNext         */
     fts3tokEofMethod,            /* xEof          */
     fts3tokColumnMethod,         /* xColumn       */
     fts3tokRowidMethod,          /* xRowid        */
     0,                           /* xUpdate       */
     0,                           /* xBegin        */
     0,                           /* xSync         */
     0,                           /* xCommit       */
     0,                           /* xRollback     */
     0,                           /* xFindFunction */
     0,                           /* xRename       */
     0,                           /* xSavepoint    */
     0,                           /* xRelease      */
     0                            /* xRollbackTo   */
  };
  int rc;                         /* Return code */

  rc = sqlite3_create_module(db, "fts3tokenize", &fts3tok_module, (void*)pHash);
  return rc;
}

#endif /* !defined(SQLITE_CORE) || defined(SQLITE_ENABLE_FTS3) */

/************** End of fts3_tokenize_vtab.c **********************************/
