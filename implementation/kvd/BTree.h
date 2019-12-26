/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for storing B+Tree structure parameters
  * </pre>
 *
 * @class BTree
 */

#include<stdint.h>

#include"DataBaseStuff.h"

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

struct BTree
{
    static const uint32_t nIdSize;
    static const uint32_t nUserNameSize;
    static const uint32_t nEmailSize;

    static const uint32_t nKeyStrSize;
    static const uint32_t nValueStrSize;

    static const uint32_t nValueStrOffset;

    static const uint32_t nRowSize;

    static const uint32_t nPageSize;

    static const uint32_t nIdOffset;
    static const uint32_t nUserNameOffset;
    static const uint32_t nEmailOffset;

    // Common Node Header Layout
    static const uint32_t nNodeTypeSize;
    static const uint32_t nNodeTypeOffset;
    static const uint32_t nIsRootSize;
    static const uint32_t nIsRootOffset;
    static const uint32_t nParentPointerSize;
    static const uint32_t nParentPointerOffset;
    static const  uint8_t nCommonNodeHeaderSize;

    // Internal Node Header Layout
    static const uint32_t nInternalNodeNumKeysSize;
    static const uint32_t nInternalNodeNumKeysOffset;
    static const uint32_t nInternalNodeRightChildSize;
    static const uint32_t nInternalNodeRightChildOffset;
    static const uint32_t nInternalNodeHeaderSize;

    // Internal Node Body Layout
    static const uint32_t nInternalNodeKeySize;
    static const uint32_t nInternalNodeChildSize;
    static const uint32_t nInternalNodeCellSize;

    static const uint32_t nInternalNodeMaxCells;

    // Leaf Node Header Layout
    static const uint32_t nLeafNodeNumCellsSize;
    static const uint32_t nLeafNodeNumCellsOffset;
    static const uint32_t nLeafNodeNextLeafSize;
    static const uint32_t nLeafNodeNextLeafOffset;
    static const uint32_t nLeafNodeHeaderSize;

    // Leaf Node Body Layout
    static const uint32_t nLeafNodeKeySize;
    static const uint32_t nLeafNodeStrKeySize;
    static const uint32_t nLeafNodeKeyOffset;

    static const uint32_t nLeafNodeValueSize;
    static const uint32_t nLeafNodeValueOffset;
    static const uint32_t nLeafNodeCellSize;

    static const uint32_t nLeafNodeSpaceForCells;
    static const uint32_t nLeafNodeMaxCells;
    static const uint32_t nLeafNodeRightSplitCount;
    static const uint32_t nLeafNodeLeftSplitCount;
};
