#include <assert.h>
#include <string.h>
#include "ite/itc.h"
#include "itc_cfg.h"

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

void itcTreeSwap(void* node1, void* node2)
{
    ITCTree *firstNode, *secondNode, *prevSibling, *tempSibling;
    assert(node1);
    assert(node2);
    assert(node1 != node2);

    firstNode = (ITCTree*)node1;
    secondNode = (ITCTree*)node2;
    
    if (firstNode->parent == NULL || secondNode->parent == NULL)
        return;

    if (firstNode->parent == secondNode->parent)
    {
        prevSibling = firstNode->parent->child;

        if (prevSibling == firstNode)
        {
            prevSibling->parent->child = secondNode;

            if (firstNode->sibling == secondNode)
            {
                firstNode->sibling = secondNode->sibling;
                secondNode->sibling = firstNode;
            }
            else
            {
                prevSibling = firstNode->sibling;

                // Find the previous sibling node
                for (; prevSibling->sibling != secondNode; prevSibling = prevSibling->sibling);

                prevSibling->sibling = firstNode;
                tempSibling = firstNode->sibling;
                firstNode->sibling = secondNode->sibling;
                secondNode->sibling = tempSibling;
            }
        }
        else if (prevSibling == secondNode)
        {
            prevSibling->parent->child = firstNode;

            if (secondNode->sibling == firstNode)
            {
                secondNode->sibling = firstNode->sibling;
                firstNode->sibling = secondNode;
            }
            else
            {
                prevSibling = secondNode->sibling;

                // Find the previous sibling node
                for (; prevSibling->sibling != firstNode; prevSibling = prevSibling->sibling);

                prevSibling->sibling = secondNode;
                tempSibling = secondNode->sibling;
                secondNode->sibling = firstNode->sibling;
                firstNode->sibling = tempSibling;
            }
        }
        else
        {
            // Find the previous sibling node
            for (; prevSibling->sibling != firstNode && prevSibling->sibling != secondNode;
                prevSibling = prevSibling->sibling);

            if (prevSibling->sibling == firstNode)
            {
                prevSibling->sibling = secondNode;

                if (firstNode->sibling == secondNode)
                {
                    firstNode->sibling = secondNode->sibling;
                    secondNode->sibling = firstNode;
                }
                else
                {
                    prevSibling = firstNode->sibling;

                    // Find the previous sibling node
                    for (; prevSibling->sibling != secondNode; prevSibling = prevSibling->sibling);

                    prevSibling->sibling = firstNode;
                    tempSibling = firstNode->sibling;
                    firstNode->sibling = secondNode->sibling;
                    secondNode->sibling = tempSibling;
                }
            }
            else if (prevSibling->sibling == secondNode)
            {
                prevSibling->sibling = firstNode;

                if (secondNode->sibling == firstNode)
                {
                    secondNode->sibling = firstNode->sibling;
                    firstNode->sibling = secondNode;
                }
                else
                {
                    prevSibling = secondNode->sibling;

                    // Find the previous sibling node
                    for (; prevSibling->sibling != firstNode; prevSibling = prevSibling->sibling);

                    prevSibling->sibling = secondNode;
                    tempSibling = secondNode->sibling;
                    secondNode->sibling = firstNode->sibling;
                    firstNode->sibling = tempSibling;
                }
            }
        }
    }
    else
    {
        ITCTree* tempParent;

        prevSibling = firstNode->parent->child;

        if (prevSibling == firstNode)
        {
            prevSibling->parent->child = secondNode;
        }
        else
        {
            // Find the previous sibling node
            for (; prevSibling->sibling != firstNode; prevSibling = prevSibling->sibling);

            prevSibling->sibling = secondNode;
        }
        firstNode->parent->child = secondNode;

        prevSibling = secondNode->parent->child;

        if (prevSibling == secondNode)
        {
            prevSibling->parent->child = firstNode;
        }
        else
        {
            // Find the previous sibling node
            for (; prevSibling->sibling != secondNode; prevSibling = prevSibling->sibling);

            prevSibling->sibling = firstNode;
        }
        secondNode->parent->child = firstNode;

        tempParent = firstNode->parent;
        tempSibling = firstNode->sibling;

        firstNode->parent = secondNode->parent;
        firstNode->sibling = secondNode->sibling;

        secondNode->parent = tempParent;
        secondNode->sibling = tempSibling;
    }
}