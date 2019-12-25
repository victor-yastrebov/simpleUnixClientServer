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

#include"BTree.h"

const uint32_t BTree::nIdSize = size_of_attribute(Row, id);

const uint32_t BTree::nUserNameSize = size_of_attribute(Row, username);
const uint32_t BTree::nEmailSize = size_of_attribute(Row, email);

const uint32_t BTree::nKeyStrSize = size_of_attribute(Row, key);
const uint32_t BTree::nValueStrSize = size_of_attribute(Row, value);

const uint32_t BTree::nValueStrOffset = 0;

// правильно ли рассчитывается, или в моем случае дб просто BTree::nValueStrSize
const uint32_t BTree::nRowSize = BTree::nKeyStrSize + BTree::nValueStrSize;

const uint32_t BTree::nPageSize = 4096;

// common node header layoyt
const uint32_t BTree::nNodeTypeSize = sizeof(uint8_t);
const uint32_t BTree::nNodeTypeOffset = 0;
const uint32_t BTree::nIsRootSize = sizeof(uint8_t);
const uint32_t BTree::nIsRootOffset = BTree::nNodeTypeSize;
const uint32_t BTree::nParentPointerSize = sizeof(uint32_t);
const uint32_t BTree::nParentPointerOffset = BTree::nIsRootOffset + BTree::nIsRootSize;
const uint8_t BTree::nCommonNodeHeaderSize =
   BTree::nNodeTypeSize + BTree::nIsRootSize + BTree::nParentPointerSize;

// internal node header layout
const uint32_t BTree::nInternalNodeNumKeysSize = sizeof( uint32_t );
const uint32_t BTree::nInternalNodeNumKeysOffset = BTree::nCommonNodeHeaderSize;
const uint32_t BTree::nInternalNodeRightChildSize = sizeof( uint32_t );
const uint32_t BTree::nInternalNodeRightChildOffset =
   BTree::nInternalNodeNumKeysOffset + BTree::nInternalNodeNumKeysSize;
const uint32_t BTree::nInternalNodeHeaderSize =
   BTree::nCommonNodeHeaderSize +
   BTree::nInternalNodeNumKeysSize +
   BTree::nInternalNodeRightChildSize;

// internal Node Body Layout
const uint32_t BTree::nInternalNodeKeySize = sizeof( uint32_t );
const uint32_t BTree::nInternalNodeChildSize = sizeof( uint32_t );
const uint32_t BTree::nInternalNodeCellSize =
    BTree::nInternalNodeChildSize + BTree::nInternalNodeKeySize;
// Keep this small for testing
const uint32_t BTree::nInternalNodeMaxCells = 3;

// Leaf Node Header Layout
const uint32_t BTree::nLeafNodeNumCellsSize = sizeof( uint32_t );
const uint32_t BTree::nLeafNodeNumCellsOffset = BTree::nCommonNodeHeaderSize;
const uint32_t BTree::nLeafNodeNextLeafSize = sizeof( uint32_t );
const uint32_t BTree::nLeafNodeNextLeafOffset =
    BTree::nLeafNodeNumCellsOffset + BTree::nLeafNodeNumCellsSize;
const uint32_t BTree::nLeafNodeHeaderSize =
    BTree::nCommonNodeHeaderSize +
    BTree::nLeafNodeNumCellsSize +
    BTree::nLeafNodeNextLeafSize;

// Leaf Node Body Layout
const uint32_t BTree::nLeafNodeKeySize = sizeof( uint32_t );
const uint32_t BTree::nLeafNodeStrKeySize = BTree::nKeyStrSize;
const uint32_t BTree::nLeafNodeKeyOffset = 0;

const uint32_t BTree::nLeafNodeValueSize = BTree::nRowSize;
const uint32_t BTree::nLeafNodeValueOffset =
   BTree::nLeafNodeKeyOffset + BTree::nLeafNodeKeySize;
const uint32_t BTree::nLeafNodeCellSize = BTree::nLeafNodeKeySize + BTree::nLeafNodeValueSize;

const uint32_t BTree::nLeafNodeSpaceForCells = BTree::nPageSize - BTree::nLeafNodeHeaderSize;
const uint32_t BTree::nLeafNodeMaxCells =
   BTree::nLeafNodeSpaceForCells / BTree::nLeafNodeCellSize;
const uint32_t BTree::nLeafNodeRightSplitCount = ( BTree::nLeafNodeMaxCells + 1 ) / 2;
const uint32_t BTree::nLeafNodeLeftSplitCount =
   ( BTree::nLeafNodeMaxCells + 1 ) - BTree::nLeafNodeRightSplitCount;
