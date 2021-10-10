#include <assert.h>
#include <string.h>
#include "ite/itu.h"
#include "itu_cfg.h"

void itcTreePushFront(ITCTree* tree, void* node)
{
    ITCTree* thisNode;

    assert(tree);
    assert(node);

    thisNode = (ITCTree*) node;
    if (tree->child == NULL)
    {
        // This is the first item we add
        thisNode->sibling = NULL;
    }
    else
    {
        // We already have items in the tree
        thisNode->sibling = tree->child;
    }
    tree->child = thisNode;
    thisNode->parent = tree;
}

void itcTreePushBack(ITCTree* tree, void* node)
{
    ITCTree* thisNode;

    assert(tree);
    assert(node);

    thisNode = (ITCTree*) node;
    if (tree->child == NULL)
    {
        // This is the first item we add
        tree->child = thisNode;
    }
    else
    {
        // We already have items in the tree
        ITCTree* child;

        for (child = tree->child; child->sibling; child = child->sibling);

        child->sibling = thisNode;
    }
    thisNode->parent = tree;
    thisNode->sibling = NULL;
}

void itcTreeRemove(void* node)
{
    ITCTree*  thisNode;
    ITCTree*  prevSibling;

    assert(node);

    thisNode = (ITCTree*) node;

    if (thisNode->parent == NULL)
        return;

    prevSibling = thisNode->parent->child;

    if (prevSibling == thisNode)
    {
        // This is the first child node
        thisNode->parent->child = thisNode->sibling;
    }
    else
    {
        // Find the previous sibling node
        for (; prevSibling->sibling != thisNode;
             prevSibling = prevSibling->sibling);

        prevSibling->sibling = thisNode->sibling;
    }

    thisNode->parent = thisNode->sibling = NULL;
}

void* itcTreeGetChildAtImpl(ITCTree* tree, int index)
{
    int i = 0;
    ITCTree* child;
    assert(tree);
    assert(index >= 0);

    for (child = tree->child; child; child = child->sibling)
    {
        if (i++ == index)
            return (void*) child;
    }
    return NULL;
}

int itcTreeGetChildCountImpl(ITCTree* tree)
{
    int i = 0;
    ITCTree* child;
    assert(tree);

    for (child = tree->child; child; child = child->sibling)
    {
        i++;
    }
    return i;
}

// Move the last child to the first one
void itcTreeRotateFront(ITCTree* tree)
{
    if (tree->child && tree->child->sibling)
    {
        ITCTree* child;
        ITCTree* prevChild = NULL;

        for (child = tree->child; child->sibling; child = child->sibling)
        {
            prevChild = child;
        }

        child->sibling      = tree->child;
        tree->child         = child;
        prevChild->sibling  = NULL;
    }
}

// Move the first child to the last one
void itcTreeRotateBack(ITCTree* tree)
{
    if (tree->child && tree->child->sibling)
    {
        ITCTree* child;
        ITCTree* firstNode = tree->child;

        for (child = tree->child; child->sibling; child = child->sibling);

        tree->child         = firstNode->sibling;
        child->sibling      = firstNode;
        firstNode->sibling  = NULL;
    }
}
