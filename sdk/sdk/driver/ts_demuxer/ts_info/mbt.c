
#include <stdio.h>
#include "mbt.h"
//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_Left_Rotate_Tree(
    MBT_NODE      **ppMBT_root_node)
{
    MBT_NODE    *pMBT_c_node = *ppMBT_root_node;
    MBT_NODE    *pMBT_r_node = (*ppMBT_root_node)->right;

    if( ppMBT_root_node && pMBT_c_node )
    {
        pMBT_c_node->right = pMBT_r_node->left;
        if( pMBT_r_node->left )
            pMBT_r_node->left->parent = pMBT_c_node;

        pMBT_r_node->left     = pMBT_c_node;
        pMBT_r_node->parent   = pMBT_c_node->parent;
        pMBT_c_node->parent   = pMBT_r_node;
        pMBT_r_node->size     = pMBT_c_node->size;
        pMBT_c_node->size     = (pMBT_c_node->left ? pMBT_c_node->left->size : 0)
                                + (pMBT_c_node->right ? pMBT_c_node->right->size : 0) + 1;

        *ppMBT_root_node      = pMBT_r_node;
    }
}

static void
_Right_Rotate_Tree(
    MBT_NODE      **ppMBT_root_node)
{
    MBT_NODE    *pMBT_c_node = *ppMBT_root_node;
    MBT_NODE    *pMBT_l_node = (*ppMBT_root_node)->left;

    if( ppMBT_root_node && pMBT_c_node )
    {
        pMBT_c_node->left = pMBT_l_node->right;
        if( pMBT_l_node->right )
            pMBT_l_node->right->parent = pMBT_c_node;

        pMBT_l_node->right    = pMBT_c_node;
        pMBT_l_node->parent   = pMBT_c_node->parent;
        pMBT_c_node->parent   = pMBT_l_node;
        pMBT_l_node->size     = pMBT_c_node->size;
        pMBT_c_node->size     = (pMBT_c_node->left ? pMBT_c_node->left->size : 0)
                                + (pMBT_c_node->right? pMBT_c_node->right->size : 0) + 1;

        *ppMBT_root_node      = pMBT_l_node;
    }
}

static void
_Refresh_Tree(
    MBT_NODE      **ppMBT_root_node,
    bool          bLeft)
{
    MBT_NODE    *pMBT_c_node = *ppMBT_root_node;

    if( ppMBT_root_node && pMBT_c_node )
    {
        if( bLeft )
        {
            if( pMBT_c_node->left && pMBT_c_node->left->left &&
                (!pMBT_c_node->right || pMBT_c_node->left->left->size > pMBT_c_node->right->size) )
                _Right_Rotate_Tree(ppMBT_root_node);
            else if( pMBT_c_node->left  && pMBT_c_node->left->right &&
                     (!pMBT_c_node->right || pMBT_c_node->left->right->size > pMBT_c_node->right->size) )
            {
                _Left_Rotate_Tree(&(pMBT_c_node->left));
                _Right_Rotate_Tree(ppMBT_root_node);
            }
            else
                return;
        }
        else
        {
            if( pMBT_c_node->right && pMBT_c_node->right->right &&
                (!pMBT_c_node->left || pMBT_c_node->right->right->size > pMBT_c_node->left->size) )
                _Left_Rotate_Tree(ppMBT_root_node);
            else if( pMBT_c_node->right && pMBT_c_node->right->left &&
                     (!pMBT_c_node->left || pMBT_c_node->right->left->size > pMBT_c_node->left->size) )
            {
                _Right_Rotate_Tree(&(pMBT_c_node->right));
                _Left_Rotate_Tree(ppMBT_root_node);
            }
            else
                return;
        }

        _Refresh_Tree(&(pMBT_c_node->left), true);
        _Refresh_Tree(&(pMBT_c_node->right), false);
        _Refresh_Tree(ppMBT_root_node, true);
        _Refresh_Tree(ppMBT_root_node, false);
    }
}

static MBT_NODE*
_Get_Extremum_Node(
    MBT_NODE       *pMBT_root_node,
    bool           bFindMax,
    int            resize)
{
    MBT_NODE       *pMBT_c_node = pMBT_root_node;

    if( pMBT_c_node )
    {
        MBT_NODE       *pMBT_sub_node = 0;

        while( pMBT_sub_node = (bFindMax ?
                    pMBT_c_node->right : pMBT_c_node->left) )
        {
            pMBT_c_node->size += resize; // ??
            pMBT_c_node = pMBT_sub_node;
        }
    }

    return pMBT_c_node;
}

static int
_Gen_Size_Value(
    MBT_VISIT_INFO  *visitInfo,
    MBT_NODE        *pMbt_c_nod)
{
    if( pMbt_c_nod->left == 0 && pMbt_c_nod->right == 0 )
        pMbt_c_nod->size = 1;
    else
    {
        pMbt_c_nod->size = (pMbt_c_nod->left?pMbt_c_nod->left->size:0)
                           + (pMbt_c_nod->right?pMbt_c_nod->right->size:0) + 1;
    }
    return 0;
}

static int
_Destroy_Tree_Node(
    MBT_VISIT_INFO  *visitInfo,
    MBT_NODE        *pMbt_c_nod)
{
    MBT_NODE_OPRATOR    *pNodeOp = (MBT_NODE_OPRATOR*)visitInfo->privData;

    if( pNodeOp && pNodeOp->destroy )
    {
        pNodeOp->destroy(pNodeOp, pMbt_c_nod);
    }
    return 0;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
MBT_NODE*
mbt_Get_Node_by_Rank(
    MBT_NODE       *pMBT_root_node,
    bool           bRootNode,
    uint32_t       index)
{
    uint32_t    r;
    MBT_NODE    *pMBT_c_node = 0;

    // the first index is '0'
    if( pMBT_root_node && index <= pMBT_root_node->size )
    {
        r = (pMBT_root_node->left ? pMBT_root_node->left->size : 0);
        if( index == r )        pMBT_c_node = pMBT_root_node;
        else if( index < r )    pMBT_c_node = mbt_Get_Node_by_Rank(pMBT_root_node->left, false, index);
        else                    pMBT_c_node = mbt_Get_Node_by_Rank(pMBT_root_node->right, false, index - r - 1);
    }

    return pMBT_c_node;
}

void
mbt_Insert_Node(
    MBT_NODE       **ppMBT_root_node,
    MBT_NODE       *pMBT_insert_node,
    void           *extraData,
    cmp_callback   compare_func)
{
    MBT_NODE       *pMBT_c_node = *ppMBT_root_node;

    if( ppMBT_root_node )
    {
        if( pMBT_c_node )
        {
            int cmp = compare_func(pMBT_c_node, pMBT_insert_node, extraData);

            pMBT_c_node->size++;
            pMBT_insert_node->parent = pMBT_c_node;

            if( cmp > 0 )
            {
                mbt_Insert_Node(&pMBT_c_node->right, pMBT_insert_node, extraData, compare_func);
                _Refresh_Tree(ppMBT_root_node, false);
            }
            else if( cmp < 0 )
            {
                mbt_Insert_Node(&pMBT_c_node->left, pMBT_insert_node, extraData, compare_func);
                _Refresh_Tree(ppMBT_root_node, true);
            }
        }
        else
        {
            *ppMBT_root_node = pMBT_insert_node;
        }
    }
}

MBT_NODE*
mbt_Find_Node(
    MBT_NODE        *pMBT_root_node,
    MBT_NODE        *pMBT_pattern_node,
    void            *extraData,
    cmp_callback    compare_func)
{
    MBT_NODE       *pMBT_c_node = pMBT_root_node;

    while( pMBT_c_node )
    {
        int cmp = compare_func(pMBT_c_node, pMBT_pattern_node, extraData);

        if( cmp > 0 )       pMBT_c_node = pMBT_c_node->right;
        else if( cmp < 0 )  pMBT_c_node = pMBT_c_node->left;
        else                break;
    }

    return  pMBT_c_node;
}

void
mbt_Visit_Order(
    MBT_VISIT_INFO  *pVisitInfo,
    MBT_NODE        *pMbt_root_node)
{
    if( pMbt_root_node && pVisitInfo )
    {
        if( (pVisitInfo->visitType == MBT_VISIT_PREORDER) && pVisitInfo->act_func )
            pVisitInfo->act_func(pVisitInfo, pMbt_root_node);

        mbt_Visit_Order(pVisitInfo, ((pVisitInfo->bMaxFirst)?pMbt_root_node->right : pMbt_root_node->left));

        if( (pVisitInfo->visitType == MBT_VISIT_INORDER) && pVisitInfo->act_func )
            pVisitInfo->act_func(pVisitInfo, pMbt_root_node);

        mbt_Visit_Order(pVisitInfo, ((pVisitInfo->bMaxFirst)?pMbt_root_node->left : pMbt_root_node->right));

        if( (pVisitInfo->visitType == MBT_VISIT_POSTORDER) && pVisitInfo->act_func )
            pVisitInfo->act_func(pVisitInfo, pMbt_root_node);
    }
}

MBT_NODE*
mbt_Del_Node(
    MBT_NODE       **ppMBT_root_node,
    MBT_NODE       *pMBT_pattern_node,
    void           *extraData,
    cmp_callback   compare_func)
{
#define UPDATE_SUBTREE(bLeft, parentNode, subNode)   \
    do{ if( parentNode ){   \
            if(bLeft) parentNode->left = subNode; \
            else      parentNode->right = subNode; \
        }else subNode->parent = parentNode; \
    }while(0)

    int            cmp = 0, prev_cmp = 0;
    MBT_NODE       *pMBT_c_node = *ppMBT_root_node;
    MBT_NODE       *pMBT_del_node = 0;

    // find pMBT_del_node
    while( pMBT_c_node )
    {
        cmp = compare_func(pMBT_c_node, pMBT_pattern_node, extraData);

        if( cmp > 0 )       pMBT_c_node = pMBT_c_node->right;
        else if( cmp < 0 )  pMBT_c_node = pMBT_c_node->left;
        else                break;

        prev_cmp = cmp;
    }

    // del pMBT_root_node and only one node case
    if( !pMBT_c_node->parent && !pMBT_c_node->left && !pMBT_c_node->right )
    {
        //*ppMBT_root_node = 0;
        //pMBT_del_node = pMBT_c_node;
        //return pMBT_del_node;
        return 0;
    }
    
    // get pMBT_del_node
    if( pMBT_c_node )
    {
        pMBT_del_node = pMBT_c_node;

        // update size
        while( pMBT_c_node )
        {
            pMBT_c_node->size--;
            pMBT_c_node = pMBT_c_node->parent;
        }

        pMBT_c_node = pMBT_del_node;
        if( pMBT_c_node->right == 0 )
        {
            UPDATE_SUBTREE((prev_cmp < 0), pMBT_c_node->parent, pMBT_c_node->left);
            if( pMBT_c_node->left )
                pMBT_c_node->left->parent = pMBT_c_node->parent;
            
            if( pMBT_del_node == (*ppMBT_root_node) )
            {
                if( pMBT_c_node->left )
                {
                    (*ppMBT_root_node) = pMBT_c_node->left;
                    // clc size
                    (*ppMBT_root_node)->size--;
                }
                else 
                    (*ppMBT_root_node) = 0;
            }
        }
        else
        {
            MBT_NODE    *pMBT_cRt_node = pMBT_c_node->right;
            if( pMBT_cRt_node->left == 0 )
            {
                pMBT_cRt_node->left = pMBT_c_node->left;
                if( pMBT_c_node->left )     
                    pMBT_c_node->left->parent = pMBT_cRt_node;

                UPDATE_SUBTREE((prev_cmp < 0), pMBT_c_node->parent, pMBT_cRt_node);

                // clc size
                pMBT_cRt_node->size = (pMBT_cRt_node->right ? pMBT_cRt_node->right->size : 0)
                                      + (pMBT_cRt_node->left ? pMBT_cRt_node->left->size : 0) + 1;

                pMBT_cRt_node->parent = pMBT_c_node->parent;
                
                if( pMBT_del_node == (*ppMBT_root_node) )
                    (*ppMBT_root_node) = pMBT_cRt_node;
            }
            else
            {
                MBT_NODE        *pMBT_sLt_node;
                MBT_NODE        *pMBT_tmp_node = 0;
                MBT_VISIT_INFO  visitInfo = {0};

                for(;;)
                {
                    pMBT_sLt_node = pMBT_cRt_node->left;
                    if( pMBT_sLt_node->left == 0 )  break;
                    pMBT_cRt_node = pMBT_sLt_node;
                }

                pMBT_cRt_node->left  = pMBT_sLt_node->right; 
                if( pMBT_sLt_node->right )
                    pMBT_sLt_node->right->parent = pMBT_cRt_node;

                pMBT_sLt_node->left  = pMBT_c_node->left;
                pMBT_sLt_node->right = pMBT_c_node->right;
                if( pMBT_c_node->right )    pMBT_c_node->right->parent = pMBT_sLt_node;
                if( pMBT_c_node->left )     pMBT_c_node->left->parent = pMBT_sLt_node;

                UPDATE_SUBTREE((prev_cmp < 0), pMBT_c_node->parent, pMBT_sLt_node);

                pMBT_sLt_node->parent = pMBT_c_node->parent;
                
                if( pMBT_del_node == (*ppMBT_root_node) )
                    (*ppMBT_root_node) = pMBT_sLt_node;
                    
                // clc size
                pMBT_tmp_node = pMBT_sLt_node;
                visitInfo.visitType = MBT_VISIT_POSTORDER;
                visitInfo.act_func  = _Gen_Size_Value;
                mbt_Visit_Order(&visitInfo, pMBT_sLt_node);

                // if root->right_sub_tree, don't need to minus 1
                cmp = compare_func(pMBT_sLt_node, (*ppMBT_root_node), 0);
                while( (cmp >= 0) && pMBT_tmp_node )
                {
                    if( pMBT_tmp_node->parent )
                        pMBT_tmp_node->size -= 1;
                    pMBT_tmp_node = pMBT_tmp_node->right;
                }
            }
        }
    }

    return pMBT_del_node; // if return 0, no match node in tree.
}

void
mbt_Del_Tree(
    MBT_NODE            **ppMBT_root_node,
    MBT_NODE_OPRATOR    *pNodeOp)
{
    MBT_NODE       *pMBT_c_node = *ppMBT_root_node;
    MBT_VISIT_INFO  visitInfo = {0};

    visitInfo.visitType = MBT_VISIT_POSTORDER;
    visitInfo.act_func  = _Destroy_Tree_Node;
    visitInfo.privData  = (void*)pNodeOp;
    mbt_Visit_Order(&visitInfo, pMBT_c_node);

    *ppMBT_root_node = 0;

}
