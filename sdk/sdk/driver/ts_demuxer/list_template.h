#ifndef __LIST_TEMPLETE_H_7VAF2XA4_VDUC_SNYC_15UH_VSL3DK2AS4QO__
#define __LIST_TEMPLETE_H_7VAF2XA4_VDUC_SNYC_15UH_VSL3DK2AS4QO__

#ifdef __cplusplus
extern "C" {
#endif

#define DEFINE_LIST_TEMPLATE(type)                         \
    typedef int (*compare)(int cmpMode,                       \
                           type *pNode,void *pattern);        \
    static type*                                              \
    list_find_##type##_node(                                  \
        int (*compare)(int cmpMode,                           \
                       type *pNode,void *pattern),            \
        int cmpMode,type *pNode,void *pattern){               \
        type    *pCurNode = pNode;                            \
        type    *pEndNode = pNode;                            \
        while(pCurNode){                                      \
            if( compare(cmpMode, pCurNode, pattern) )         \
                return pCurNode;                              \
            pCurNode = pCurNode->next;                        \
            if(pCurNode == pEndNode) break;                   \
        }                                                     \
        return 0;                                             \
    }                                                         \
    static type*                                              \
    list_add_##type##_node(                                   \
        type *pCurNode,type *pInsNode){                       \
        if( pCurNode && pInsNode ){                           \
            pCurNode->next->prev = pInsNode;                  \
            pInsNode->next = pCurNode->next;                  \
            pInsNode->prev = pCurNode;                        \
            pCurNode->next = pInsNode;                        \
            return pInsNode;                                  \
        }                                                     \
        return 0;                                             \
    }                                                         \
    static type*                                              \
    list_del_##type##_node(                                   \
        type *pCurNode){                                      \
        if( pCurNode ){                                       \
            pCurNode->prev->next = pCurNode->next;            \
            pCurNode->next->prev = pCurNode->prev;            \
            if( pCurNode->prev != pCurNode )                  \
                return pCurNode->prev;                        \
        }                                                     \
        return 0;                                             \
    }                                                         \
    static void                                               \
    list_swap_##type##_node(                                  \
        type *pNode1, type *pNode2){                          \
        pNode2->next->prev = pNode1->next;                    \
        pNode1->next->prev = pNode2;                          \
        pNode1->next = pNode2->next;                          \
        pNode2->next = pNode1->next->prev;                    \
        pNode1->next->prev = pNode1;                          \
        pNode2->prev->next = pNode1;                          \
        pNode1->prev->next = pNode2->prev;                    \
        pNode2->prev = pNode1->prev;                          \
        pNode1->prev = pNode2->prev->next;                    \
        pNode2->prev->next = pNode2;                          \
    }

#define LIST_INIT(ptr)                                      do {(ptr)->next = (ptr); (ptr)->prev = (ptr);} while (0)
#define LIST_ADD(type, pCur, pIns)                          list_add_##type##_node(pCur, pIns)
#define LIST_DEL(type, pCur)                                list_del_##type##_node(pCur)
#define LIST_FIND(type, cmpFunc, cmpMode, pStart, pattern)  list_find_##type##_node(cmpFunc, cmpMode, pStart, (void*)pattern)
#define LIST_SWAP(type, pItm1, pItm2)                       list_swap_##type##_node(pItm1, pItm2)


#ifdef __cplusplus
}
#endif

#endif
