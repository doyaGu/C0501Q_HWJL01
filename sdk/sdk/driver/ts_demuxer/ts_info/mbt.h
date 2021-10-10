#ifndef __MBT_H_PZC24GSK_AOT4_270S_7VHS_OEGMO8SPB50H__
#define __MBT_H_PZC24GSK_AOT4_270S_7VHS_OEGMO8SPB50H__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 * traversal type of MBT
 **/
typedef enum _MBT_VISIT_TYPE_TAG
{
    MBT_VISIT_UNKNOW    = 0,
    MBT_VISIT_PREORDER,
    MBT_VISIT_INORDER,
    MBT_VISIT_POSTORDER,

}MBT_VISIT_TYPE;


//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct MBT_NODE_TAG
{
    struct MBT_NODE_TAG   *left, *right, *parent;
    uint32_t              size; // node count in current node_tree (include self) for rank api

}MBT_NODE;

/**
 * Visit MBT info
 **/
typedef struct _MBT_VISIT_INFO_TAG
{
    MBT_VISIT_TYPE      visitType;
    bool                bMaxFirst; // falsu: start from minimum, true: start from maximum
    
    void                *privData;
    void                *privData_1;

    int (*act_func)(struct _MBT_VISIT_INFO_TAG *visitInfo, MBT_NODE *pMbt_c_node);

}MBT_VISIT_INFO;

/**
 * MBT node operator (set by AP)
 **/
typedef struct _MBT_NODE_OPRATOR_TAG
{
    void        *privData;
    void        *privData_1;

    void*       (*create)(struct _MBT_NODE_OPRATOR_TAG *oprator, uint32_t struct_size);
    void        (*destroy)(struct _MBT_NODE_OPRATOR_TAG *oprator, MBT_NODE *pMbt_c_node);

}MBT_NODE_OPRATOR;

typedef int (*cmp_callback)(MBT_NODE *node_cur, MBT_NODE *node_pattern, void *param);
//=============================================================================
//                  Global Data Definition
//=============================================================================


//=============================================================================
//                  Private Function Definition
//=============================================================================
MBT_NODE*
mbt_Get_Node_by_Rank(
    MBT_NODE       *pMBT_root_node,
    bool           bRootNode,
    uint32_t       index);


void
mbt_Insert_Node(
    MBT_NODE       **ppMBT_root_node,
    MBT_NODE       *pMBT_insert_node,
    void           *extraData,
    cmp_callback   compare_func);


MBT_NODE*
mbt_Find_Node(
    MBT_NODE        *pMBT_root_node,
    MBT_NODE        *pMBT_pattern_node,
    void            *extraData,
    cmp_callback    compare_func);


void
mbt_Visit_Order(
    MBT_VISIT_INFO  *pVisitInfo,
    MBT_NODE        *pMbt_root_node);


MBT_NODE*
mbt_Del_Node(
    MBT_NODE       **ppMBT_root_node,
    MBT_NODE       *pMBT_pattern_node,
    void           *extraData,
    cmp_callback   compare_func);


void
mbt_Del_Tree(
     MBT_NODE            **ppMBT_root_node,
     MBT_NODE_OPRATOR    *pNodeOp);
//=============================================================================
//                  Public Function Definition
//=============================================================================
#define mbt_get_struct(struct_type, member_name, pMember)    \
        (pMember)?(struct_type*)((uint32_t)pMember - (uint32_t)&(((struct_type *)0)->member_name)):0

#define mbt_get_member(pStruct, member_name)    ((pStruct)->member_name)



#define mbt_init_node(struct_type, member_name, pItm)     \
            memset(&((struct_type*)pItm)->member_name, 0, sizeof(MBT_NODE))

#define mbt_create_node(struct_type, member_name, ppItm, pNodeOp)       \
            do{ if( (MBT_NODE_OPRATOR*)pNodeOp && ((MBT_NODE_OPRATOR*)pNodeOp)->create )  \
                    *(ppItm) = ((MBT_NODE_OPRATOR*)pNodeOp)->create(pNodeOp, sizeof(struct_type)); \
                if( *(ppItm) ) mbt_init_node(struct_type, member_name, (*(ppItm))); \
            }while(0)

#define mbt_add_node(struct_type, member_name, ppRootItm, pInsItm, cmp_func, pExtraData)\
            do{ MBT_NODE *pMBT_root_node = &((struct_type*)(*ppRootItm))->member_name;\
                mbt_Insert_Node(&pMBT_root_node, &((struct_type*)pInsItm)->member_name,\
                                pExtraData, cmp_func);\
                *(ppRootItm) = mbt_get_struct(struct_type, member_name,pMBT_root_node);    \
            }while(0)

#define mbt_find_node(struct_type, member_name, pRootItm, pPatternItm, ppGetItm, cmp_func, pExtraData)\
            do{ MBT_NODE *pMBT_node = mbt_Find_Node(&((struct_type*)pRootItm)->member_name, \
                                &((struct_type*)pPatternItm)->member_name, pExtraData, cmp_func);\
                *(ppGetItm) = (pMBT_node)?mbt_get_struct(struct_type, member_name, pMBT_node):0; \
            }while(0)

#define mbt_get_rank_node(struct_type, member_name, pRootItm, index, ppItm) \
            do{ MBT_NODE *pMBT_node = mbt_Get_Node_by_Rank(&((struct_type*)pRootItm)->member_name, \
                true, index);  \
                *(ppItm) = (pMBT_node)?mbt_get_struct(struct_type, member_name, pMBT_node):0; \
            }while(0)

#define mbt_get_root(struct_type, member_name, pItm, ppRootItm) \
            do{ MBT_NODE *pMBT_node = 0;  \
                *(ppRootItm) = pItm;                            \
                pMBT_node = &(*(ppRootItm))->member_name;    \
                while( pMBT_node ){  \
                    if( pMBT_node->parent == 0 ) break;    \
                    pMBT_node = pMBT_node->parent;          \
                }           \
                *(ppRootItm) = mbt_get_struct(struct_type, member_name, pMBT_node);  \
            }while(0)

#define mbt_visit_node(struct_type, member_name, pRootItm, pVisitInfo)  \
            do{ mbt_Visit_Order((MBT_VISIT_INFO*)pVisitInfo, &((struct_type*)pRootItm)->member_name); \
            }while(0)

#define mbt_del_node(struct_type, member_name, ppRootItm, pPatternItm, cmp_func, pNodeOp)\
            do{ MBT_NODE *pMBT_root_node = &((struct_type*)(*ppRootItm))->member_name;\
                MBT_NODE *pMBT_d_node = mbt_Del_Node(&pMBT_root_node, \
                                            &((struct_type*)pPatternItm)->member_name, 0, cmp_func);\
                (*ppRootItm) = mbt_get_struct(struct_type, member_name, pMBT_root_node); \
                if( pMBT_d_node && (MBT_NODE_OPRATOR*)pNodeOp && ((MBT_NODE_OPRATOR*)pNodeOp)->destroy ) \
                    ((MBT_NODE_OPRATOR*)pNodeOp)->destroy(pNodeOp, pMBT_d_node); \
            }while(0)

#define mbt_del_tree(struct_type, member_name, ppRootItm, pNodeOp)\
            do{ MBT_NODE *pMBT_root_node = &((struct_type*)(*ppRootItm))->member_name; \
                mbt_Del_Tree(&pMBT_root_node, (MBT_NODE_OPRATOR*)pNodeOp);      \
                *ppRootItm = 0;     \
            }while(0)

//#define mbt_destroy_node(struct_type, member_name, ppItm)  \
//            do{ if( *ppItm )    {free((*ppItm)); *ppItm = 0;} \
//            }while(0)





#ifdef __cplusplus
}
#endif

#endif
