/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for processing incoming connections from clients
  * </pre>
 *
 * @class DataBase
 */

#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "DataBase.h"

/**
 * CTOR
 */
DataBase::DataBase( const std::string &s_db_file_path ) :
   sDbFilePath( s_db_file_path )
{
   // sysLogger.LogToSyslog( "DataBase CTOR DONE (1)" );
}

/**
 * DTOR
 */
DataBase::~DataBase()
{
   // sysLogger.LogToSyslog( "DataBase DTOR DONE12" );
}

eNodeType DataBase::getNodeType(void* node)
{
  uint8_t value = *((uint8_t*)(node + BTree::nNodeTypeOffset));
  return (eNodeType)value;
}

void DataBase::setNodeType(void* node, eNodeType type)
{
  uint8_t value = type;
  *((uint8_t*)(node + BTree::nNodeTypeOffset)) = value;
}

bool DataBase::isNodeRoot(void* node)
{
  uint8_t value = *((uint8_t*)(node + BTree::nIsRootOffset));
  return (bool)value;
}

void DataBase::setNodeRoot(void* node, bool is_root)
{
  uint8_t value = is_root;
  *((uint8_t*)(node + BTree::nIsRootOffset)) = value;
}

uint32_t* DataBase::nodeParent(void* node)
{
   return static_cast<uint32_t*>( node ) + BTree::nParentPointerOffset;
}

uint32_t* DataBase::internalNodeNumKeys(void* node)
{
  return static_cast<uint32_t*>( node ) + BTree::nInternalNodeNumKeysOffset;
}

uint32_t* DataBase::internalNodeRightChild(void* node)
{
  return static_cast<uint32_t*>( node ) + BTree::nInternalNodeRightChildOffset;
}

uint32_t* DataBase::internalNodeCell(void* node, uint32_t cell_num)
{
  return static_cast<uint32_t*>( node ) + BTree::nInternalNodeHeaderSize + cell_num * BTree::nInternalNodeCellSize;
}

uint32_t* DataBase::internalNodeChild(void* node, uint32_t child_num)
{
  uint32_t num_keys = *internalNodeNumKeys(node);
  if (child_num > num_keys) {
    sysLogger.LogToSyslog( "Tried to access child_num %d > num_keys %d\n", child_num, num_keys );
    exit(EXIT_FAILURE);
  } else if (child_num == num_keys) {
    return internalNodeRightChild(node);
  } else {
    return internalNodeCell(node, child_num);
  }
}

uint32_t* DataBase::internalNodeKey(void* node, uint32_t key_num)
{
  return (uint32_t*)internalNodeCell(node, key_num) + BTree::nInternalNodeChildSize;
}

uint32_t* DataBase::leafNodeNumCells(void* node)
{
  uint32_t* val = static_cast<uint32_t*>( node ) + BTree::nLeafNodeNumCellsOffset;
  return static_cast<uint32_t*>( node ) + BTree::nLeafNodeNumCellsOffset;
}

uint32_t* DataBase::leafNodeNextLeaf(void* node)
{
  return static_cast<uint32_t*>( node ) + BTree::nLeafNodeNextLeafOffset;
}

void* DataBase::leafNodeCellId(void* node, uint32_t cell_num)
{
  return static_cast<uint32_t*>( node ) + BTree::nLeafNodeHeaderSize + cell_num * BTree::nLeafNodeCellSize;
}

/*
 * Получить указатель на область памяти cell (на page их может быть несколько),
 *  в котором хранится key для навигации по BTree
*/
uint32_t* DataBase::leafNodeKeyId(void* node, uint32_t cell_num)
{
  return static_cast<uint32_t*>( leafNodeCellId(node, cell_num) );
}

// получить указатель на начало памяти в page, где хранится cell с индексом cell_num
// Cell - это пара: {ключ, значение}
void* DataBase::leafNodeCell(void* node, uint32_t cell_num)
{
   // void* node - указатель на начало page
  return static_cast<char*>( node ) + BTree::nLeafNodeHeaderSize + cell_num * BTree::nLeafNodeCellSize;
}

char* DataBase::leafNodeKey(void* node, uint32_t cell_num)
{
  return static_cast<char*>( leafNodeCell(node, cell_num) );
}

void* DataBase::leafNodeValue(void* node, uint32_t cell_num)
{
  // в будущем LEAF_NODE_KEY_SIZE должна стать переменным числом
  return leafNodeCell(node, cell_num) + BTree::nLeafNodeStrKeySize;
}

void* DataBase::leafNodeValueId(void* node, uint32_t cell_num)
{
  return leafNodeCellId(node, cell_num) + BTree::nLeafNodeKeySize;
}

uint32_t DataBase::getNodeMaxKey(void* node)
{
  switch (getNodeType(node))
  {
    case eNodeType::ntInternal:
      return *internalNodeKey(node, *internalNodeNumKeys(node) - 1);
    case eNodeType::ntLeaf:
      return *leafNodeKeyId(node, *leafNodeNumCells(node) - 1);
  }
}

void DataBase::printConstants()
{
  sysLogger.LogToSyslog("nRowSize: ", BTree::nRowSize);
  sysLogger.LogToSyslog("nCommonNodeHeaderSize: ", BTree::nCommonNodeHeaderSize);
  sysLogger.LogToSyslog("nLeafNodeHeaderSize: ", BTree::nLeafNodeHeaderSize);
  sysLogger.LogToSyslog("nLeafNodeCellSize: ", BTree::nLeafNodeCellSize);
  sysLogger.LogToSyslog("nLeafNodeSpaceForCells: ", BTree::nLeafNodeSpaceForCells);
  sysLogger.LogToSyslog("nLeafNodeMaxCells: ", BTree::nLeafNodeMaxCells);
}

void* DataBase::getPage( Pager* pager, uint32_t page_num )
{
  if( page_num > dbs::nTableMaxPages )
  {
   sysLogger.LogToSyslog( "Tried to fetch page number out of bounds. ",
      page_num, " > ", dbs::nTableMaxPages );
   exit(EXIT_FAILURE);
  }

  if( pager->pages[page_num] == NULL )
  {
    // Cache miss. Allocate memory and load from file.
    void* page = malloc( BTree::nPageSize );
    uint32_t num_pages = pager->file_length / BTree::nPageSize;

    // We might save a partial page at the end of the file
    if( pager->file_length % BTree::nPageSize )
    {
      num_pages += 1;
    }

    if( page_num <= num_pages )
    {
      lseek( pager->file_descriptor, page_num * BTree::nPageSize, SEEK_SET );
      ssize_t bytes_read = read( pager->file_descriptor, page, BTree::nPageSize );
      if (bytes_read == -1) {
        sysLogger.LogToSyslog("Error reading file: ", errno);
        exit(EXIT_FAILURE);
      }
    }

    pager->pages[page_num] = page;

    if (page_num >= pager->num_pages) {
      pager->num_pages = page_num + 1;
    }
  }

  return pager->pages[page_num];
}

void DataBase::indent(uint32_t level)
{
  /*for (uint32_t i = 0; i < level; i++) {
    sysLogger.LogToSyslog("  ");
  }*/
}

void DataBase::printTree(Pager* pager, uint32_t page_num, uint32_t indentation_level)
{
  void* node = getPage(pager, page_num);
  uint32_t num_keys, child;

  switch (getNodeType(node)) {
    case (eNodeType::ntLeaf):
      num_keys = *leafNodeNumCells(node);
      indent(indentation_level);
      sysLogger.LogToSyslog("- leaf (size", num_keys, ")");
      for (uint32_t i = 0; i < num_keys; i++)
      {
        indent(indentation_level + 1);
        sysLogger.LogToSyslog("  - ", *leafNodeKey(node, i));
      }
      break;
    case (eNodeType::ntInternal):
      num_keys = *internalNodeNumKeys(node);
      indent(indentation_level);
      sysLogger.LogToSyslog("- internal (size ", num_keys, ")");
      for (uint32_t i = 0; i < num_keys; i++) {
        child = *internalNodeChild(node, i);
        printTree(pager, child, indentation_level + 1);

        indent(indentation_level + 1);
        sysLogger.LogToSyslog("- key ", *internalNodeKey(node, i));
      }
      child = *internalNodeRightChild(node);
      printTree(pager, child, indentation_level + 1);
      break;
  }
}

void DataBase::serializeRow(Row* source, void* destination)
{
  memcpy(destination + BTree::nValueStrOffset, &(source->value), BTree::nValueStrSize );
  // memcpy(destination + USERNAME_OFFSET, &(source->username), BTree::nUserNameSize);
  // memcpy(destination + EMAIL_OFFSET, &(source->email), BTree::nEmailSize);
}

std::string DataBase::getKey( Cursor* p_cursor )
{
   std::string str;
   str.resize( BTree::nKeyStrSize );

   void *p_key = cursorKey( p_cursor );

   memcpy( (void*)str.data(), p_key, BTree::nKeyStrSize );
   str.at(dbs::nEtalonKeySize) = '\0';

   return str;
}

void DataBase::deserializeRow( void *key, void* source, Row* destination )
{
  memcpy( &( destination->value ), source + BTree::nValueStrOffset, BTree::nValueStrSize );
  memcpy( &( destination->key ), key, BTree::nKeyStrSize );
  destination->key[dbs::nEtalonKeySize] = '\0';
  // memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  // memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  // memcpy(&(destination->email), source + EMAIL_OFFSET, BTree::nEmailSize);
}

void DataBase::initializeLeafNode(void* node)
{
  setNodeType(node, eNodeType::ntLeaf );
  setNodeRoot(node, false);
  *leafNodeNumCells(node) = 0;
  *leafNodeNextLeaf(node) = 0;  // 0 represents no sibling
}

void DataBase::initializeInternalNode(void* node)
{
  setNodeType(node, eNodeType::ntInternal);
  setNodeRoot(node, false);
  *internalNodeNumKeys(node) = 0;
}

/**
 * Find position in leaf node
 * This function will return:
 * - the position of the key OR
 * - the position of another key that we’ll need to move if we want to insert the new key OR
 * - the position one past the last key
 */
Cursor* DataBase::leafNodeFind(Table* table, uint32_t page_num, char* key)
{
  void* node = getPage(table->pager, page_num);
  uint32_t num_cells = *leafNodeNumCells(node);

  Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));
  cursor->table = table;
  cursor->page_num = page_num;
  cursor->end_of_table = false;

  // Binary search
  uint32_t min_index = 0;
  uint32_t one_past_max_index = num_cells;
  while( one_past_max_index != min_index )
  {
    // cell sequential number in leaf node
    // take middle index
    uint32_t index = (min_index + one_past_max_index) / 2;
    // take key of the middle cell
    char* key_at_index = leafNodeKey(node, index);

    const int compare_result = compareKeys( key, key_at_index );
    // keys are equal
    if( 0 == compare_result )
    {
       cursor->cell_num = index;
       return cursor;
    }

    // continue search in left part
    if( compare_result < 0 )
    {
       one_past_max_index = index;
    }
    // continue search in right part
    else
    {
       min_index = index + 1;
    }
  }

  cursor->cell_num = min_index;
  return cursor;
}

int DataBase::compareKeys( char *key1, char* key2 ) const
{
   const std::string s1( key1 );
   const std::string s2( key2 );

   return s1.compare( s2 );
}

uint32_t DataBase::internalNodeFindChild(void* node, uint32_t key)
{
  /*
  Return the index of the child which should contain
  the given key.
  */

  uint32_t num_keys = *internalNodeNumKeys(node);

  /* Binary search */
  uint32_t min_index = 0;
  uint32_t max_index = num_keys; /* there is one more child than key */

  while (min_index != max_index) {
    uint32_t index = (min_index + max_index) / 2;
    uint32_t key_to_right = *internalNodeKey(node, index);
    if (key_to_right >= key) {
      max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  return min_index;
}

Cursor* DataBase::internalNodeFind(Table* table, uint32_t page_num, uint32_t key)
{
  void* node = getPage(table->pager, page_num);

  uint32_t child_index = internalNodeFindChild(node, key);
  uint32_t child_num = *internalNodeChild(node, child_index);
  void* child = getPage(table->pager, child_num);
  switch (getNodeType(child)) {
    case eNodeType::ntLeaf:
      // return leafNodeFind(table, child_num, key);
    case eNodeType::ntInternal:
      return internalNodeFind(table, child_num, key);
  }
}

/*
Return the position of the given key.for BTree, not key in the row
If the key is not present, return the position
where it should be inserted
*/
Cursor* DataBase::tableFind(Table* table, char* key)
{
  uint32_t root_page_num = table->root_page_num;
  void* root_node = getPage(table->pager, root_page_num);

  if (getNodeType(root_node) == eNodeType::ntLeaf )
  {
     // найди в таблице, начиная с ее первой страницы такой вот ключ
     return leafNodeFind(table, root_page_num, key);
  } else {
    sysLogger.LogToSyslog( "!!!!!!! INTERNAL NODE FIND IS NOT IMPLEMENTED !!!");
    return NULL;
    // return internalNodeFind(table, root_page_num, key);
  }
}

Cursor* DataBase::createCursorForFirstCell( Table* table ) const
{
   Cursor* cursor = (Cursor*)malloc(sizeof(Cursor));
   cursor->table = table;
   cursor->page_num = table->root_page_num;
   cursor->end_of_table = false;

   cursor->cell_num = 0;
   return cursor;
}

Cursor* DataBase::tableStart(Table* table)
{
   // надо установить курсор на самую первую строку
   Cursor* cursor = createCursorForFirstCell( table );

   void* node = getPage( table->pager, cursor->page_num );
   uint32_t num_cells = *leafNodeNumCells( node );
   cursor->end_of_table = ( num_cells == 0 );

   return cursor;
}

void* DataBase::cursorValue(Cursor* cursor)
{
  uint32_t page_num = cursor->page_num;
  void* page = getPage(cursor->table->pager, page_num);
  return leafNodeValue(page, cursor->cell_num);
}

void* DataBase::cursorKey(Cursor* cursor)
{
   uint32_t page_num = cursor->page_num;
   void* page = getPage(cursor->table->pager, page_num);
   return leafNodeKey(page, cursor->cell_num);
}


void DataBase::cursorAdvance(Cursor* cursor)
{
  uint32_t page_num = cursor->page_num;
  void* node = getPage(cursor->table->pager, page_num);

  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leafNodeNumCells(node)))
  {
    /* Advance to next leaf node */
    uint32_t next_page_num = *leafNodeNextLeaf(node);
    if (next_page_num == 0) {
      /* This was rightmost leaf */
      cursor->end_of_table = true;
    } else {
      cursor->page_num = next_page_num;
      cursor->cell_num = 0;
    }
  }
}

Pager* DataBase::pagerOpen(const char* filename)
{
  int fd = open(filename,
                O_RDWR |      // Read/Write mode
                    O_CREAT,  // Create file if it does not exist
                S_IWUSR |     // User write permission
                    S_IRUSR   // User read permission
                );

  if( fd == -1 )
  {
    sysLogger.LogToSyslog("Unable to open file");
    exit(EXIT_FAILURE);
  }

  off_t file_length = lseek( fd, 0, SEEK_END );

  Pager* pager = ( Pager * )malloc( sizeof( Pager ) );
  pager->file_descriptor = fd;
  pager->file_length = file_length;
  pager->num_pages = (file_length / BTree::nPageSize);

  if( file_length % BTree::nPageSize != 0 )
  {
    sysLogger.LogToSyslog( "Db file is not a whole number of pages. Corrupt file" );
    exit(EXIT_FAILURE);
  }

  for( uint32_t i = 0; i < dbs::nTableMaxPages; i++ )
  {
    pager->pages[i] = NULL;
  }

  return pager;
}

void DataBase::dbOpen()
{
  Pager* pager = pagerOpen( sDbFilePath.c_str() );

  table = (Table *)malloc(sizeof(Table));
  table->pager = pager;
  table->root_page_num = 0;

  if (pager->num_pages == 0)
  {
    // New database file. Initialize page 0 as leaf node.
    void* root_node = getPage(pager, 0);
    initializeLeafNode(root_node);
    setNodeRoot(root_node, true);
  }
}

InputBuffer* DataBase::newInputBuffer()
{
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

void DataBase::readInput( InputBuffer* input_buffer, const std::string &s_query )
{
  sCurQuery = s_query;

  // ssize_t bytes_read =
  //     getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  ssize_t bytes_read = sCurQuery.size();
  input_buffer->buffer = const_cast<char*>( sCurQuery.c_str() );
  input_buffer->buffer_length = sCurQuery.size();

  // getline(&(input_buffer->buffer), &(input_buffer->buffer_length), stdin);

  if (bytes_read <= 0) {
    sysLogger.LogToSyslog("Error reading input\n");
    exit(EXIT_FAILURE);
  }

  // Ignore trailing newline
  // input_buffer->input_length = bytes_read - 1;
  // input_buffer->buffer[bytes_read - 1] = 0;
}

void DataBase::closeInputBuffer(InputBuffer* input_buffer)
{
  // free(input_buffer->buffer);
  free(input_buffer);
}

void DataBase::pagerFlush(Pager* pager, uint32_t page_num)
{
  if (pager->pages[page_num] == NULL)
  {
    sysLogger.LogToSyslog("Tried to flush null page");
    exit(EXIT_FAILURE);
  }

  off_t offset = lseek(pager->file_descriptor, page_num * BTree::nPageSize, SEEK_SET);

  if (offset == -1) {
    sysLogger.LogToSyslog("Error seeking: %d\n", errno);
    exit(EXIT_FAILURE);
  }

  // запись данных из pager'a на ЖД
  ssize_t bytes_written =
      write(pager->file_descriptor, pager->pages[page_num], BTree::nPageSize);

  if (bytes_written == -1)
  {
    sysLogger.LogToSyslog( "Error writing: ", errno );
    exit(EXIT_FAILURE);
  }
}

void DataBase::dbClose() {
  Pager* pager = table->pager;

  // flushes the page cache to disk
  for (uint32_t i = 0; i < pager->num_pages; i++) {
    if (pager->pages[i] == NULL) {
      continue;
    }
    pagerFlush(pager, i);
    free(pager->pages[i]);
    pager->pages[i] = NULL;
  }

  // closes the database file
  int result = close(pager->file_descriptor);
  if (result == -1) {
    sysLogger.LogToSyslog("Error closing db file.\n");
    exit(EXIT_FAILURE);
  }
  else {
     // LogToSyslog( "close DB success" );
  }

  // frees the memory for the Pager and Table data structures
  for (uint32_t i = 0; i < dbs::nTableMaxPages; i++) {
    void* page = pager->pages[i];
    if (page) {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}

eMetaCmdResult DataBase::doMetaCommand(InputBuffer* input_buffer, Table* table)
{
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    closeInputBuffer(input_buffer);
    dbClose();
    return eMetaCmdResult::mcrSuccess;
  } else if (strcmp(input_buffer->buffer, ".btree") == 0) {
    sysLogger.LogToSyslog( "Tree:" );
    printTree(table->pager, 0, 0);
    return eMetaCmdResult::mcrSuccess;
  } else if (strcmp(input_buffer->buffer, ".constants") == 0) {
    sysLogger.LogToSyslog("Constants:\n");
    printConstants();
    return eMetaCmdResult::mcrSuccess;
  } else {
    return eMetaCmdResult::mcrUnrecognisedCmd;
  }
}

ePrepareResult DataBase::prepareInsert(InputBuffer* input_buffer, Statement* statement)
{
  statement->eType = eStatementType::stPut;

  char* keyword = strtok(input_buffer->buffer, " ");

  char* key = strtok(NULL, " ");
  char* value = strtok(NULL, " ");

  if (key == NULL || value == NULL)
  {
    return ePrepareResult::prSyntaxError;
  }

  const size_t key_len = strlen( key );
  if( key_len != dbs::nEtalonKeySize )
  {
     return ePrepareResult::prStringTooLong;
  }

  if (strlen(value) > dbs::nValueMaxSize)
  {
     return ePrepareResult::prStringTooLong;
  }

  strcpy(statement->row_to_insert.key, key);
  strcpy(statement->row_to_insert.value, value);

  return ePrepareResult::prSuccess;
}

ePrepareResult DataBase::prepareStatement(InputBuffer* input_buffer,
                                Statement* statement)
{
  if (strncmp(input_buffer->buffer, "put", 3) == 0) {
    return prepareInsert(input_buffer, statement);
  }
  if (strcmp(input_buffer->buffer, "list") == 0)
  {
    statement->eType = eStatementType::stList;
    return ePrepareResult::prSuccess;;
  }

  return ePrepareResult::prUnrecognisedStatement;
}

/*
Until we start recycling free pages, new pages will always
go onto the end of the database file
*/
uint32_t DataBase::getUnusedPageNum(Pager* pager)
{
    return pager->num_pages;
}

void DataBase::createNewRoot(Table* table, uint32_t right_child_page_num)
{
  /*
  Handle splitting the root.
  Old root copied to new page, becomes left child.
  Address of right child passed in.
  Re-initialize root page to contain the new root node.
  New root node points to two children.
  */

  void* root = getPage(table->pager, table->root_page_num);
  void* right_child = getPage(table->pager, right_child_page_num);
  uint32_t left_child_page_num = getUnusedPageNum(table->pager);
  void* left_child = getPage(table->pager, left_child_page_num);

  /* Left child has data copied from old root */
  memcpy(left_child, root, BTree::nPageSize);
  setNodeRoot(left_child, false);

  /* Root node is a new internal node with one key and two children */
  initializeInternalNode(root);
  setNodeRoot(root, true);
  *internalNodeNumKeys(root) = 1;
  *internalNodeChild(root, 0) = left_child_page_num;
  uint32_t left_child_max_key = getNodeMaxKey(left_child);
  *internalNodeKey(root, 0) = left_child_max_key;
  *internalNodeRightChild(root) = right_child_page_num;
  *nodeParent(left_child) = table->root_page_num;
  *nodeParent(right_child) = table->root_page_num;
}

void DataBase::internalNodeInsert(Table* table, uint32_t parent_page_num,
                          uint32_t child_page_num)
{
  /*
  Add a new child/key pair to parent that corresponds to child
  */

  void* parent = getPage(table->pager, parent_page_num);
  void* child = getPage(table->pager, child_page_num);
  uint32_t child_max_key = getNodeMaxKey(child);
  uint32_t index = internalNodeFindChild(parent, child_max_key);

  uint32_t original_num_keys = *internalNodeNumKeys(parent);
  *internalNodeNumKeys(parent) = original_num_keys + 1;

  if (original_num_keys >= BTree::nInternalNodeMaxCells) {
    sysLogger.LogToSyslog("Need to implement splitting internal node\n");
    exit(EXIT_FAILURE);
  }

  uint32_t right_child_page_num = *internalNodeRightChild(parent);
  void* right_child = getPage(table->pager, right_child_page_num);

  if (child_max_key > getNodeMaxKey(right_child)) {
    /* Replace right child */
    *internalNodeChild(parent, original_num_keys) = right_child_page_num;
    *internalNodeKey(parent, original_num_keys) =
        getNodeMaxKey(right_child);
    *internalNodeRightChild(parent) = child_page_num;
  } else {
    /* Make room for the new cell */
    for (uint32_t i = original_num_keys; i > index; i--) {
      void* destination = internalNodeCell(parent, i);
      void* source = internalNodeCell(parent, i - 1);
      memcpy(destination, source, BTree::nInternalNodeCellSize);
    }
    *internalNodeChild(parent, index) = child_page_num;
    *internalNodeKey(parent, index) = child_max_key;
  }
}

void DataBase::updateInternalNodeKey(void* node, uint32_t old_key, uint32_t new_key)
{
  uint32_t old_child_index = internalNodeFindChild(node, old_key);
  *internalNodeKey(node, old_child_index) = new_key;
}

void DataBase::leafNodeSplitAndInsert( Cursor* cursor, char* key, Row* value )
{
  /*
  Create a new node and move half the cells over.
  Insert the new value in one of the two nodes.
  Update parent or create a new parent.
  */
  void* old_node = getPage(cursor->table->pager, cursor->page_num);

  // получить ключ с наибольшим значением
  // !!! надо править !!!
  uint32_t old_max = getNodeMaxKey(old_node);

  // получаем номер страницы, которую надо будет создать
  uint32_t new_page_num = getUnusedPageNum(cursor->table->pager);
  // создали страницу в памяти
  void* new_node = getPage(cursor->table->pager, new_page_num);

  initializeLeafNode(new_node);
  *nodeParent(new_node) = *nodeParent(old_node);
  *leafNodeNextLeaf(new_node) = *leafNodeNextLeaf(old_node);
  *leafNodeNextLeaf(old_node) = new_page_num;

  /*
  All existing keys plus new key should should be divided
  evenly between old (left) and new (right) nodes.
  Starting from the right, move each key to correct position.
  */
  for (int32_t i = BTree::nLeafNodeMaxCells; i >= 0; i--)
  {
    void* destination_node;
    if (i >= BTree::nLeafNodeLeftSplitCount)
    {
      destination_node = new_node;
    }
    else
    {
      destination_node = old_node;
    }

    uint32_t index_within_node = i % BTree::nLeafNodeLeftSplitCount;
    // void* destination = leafNodeCellId(destination_node, index_within_node);
    void* destination = leafNodeCell(destination_node, index_within_node);

    if (i == cursor->cell_num)
    {
      serializeRow(value,
                    leafNodeValueId(destination_node, index_within_node));
      // *leafNodeKeyId(destination_node, index_within_node) = key;
      memcpy(leafNodeKey(destination_node, index_within_node), key, strlen(key));
    }
    else if (i > cursor->cell_num)
    {
      memcpy(destination, leafNodeCellId(old_node, i - 1), BTree::nLeafNodeCellSize);
    }
    else
    {
      memcpy(destination, leafNodeCellId(old_node, i), BTree::nLeafNodeCellSize);
    }
  }

  /* Update cell count on both leaf nodes */
  *(leafNodeNumCells(old_node)) = BTree::nLeafNodeLeftSplitCount;
  *(leafNodeNumCells(new_node)) = BTree::nLeafNodeRightSplitCount;

  // if original node is root, than it has no parent
  if (isNodeRoot(old_node))
  {
    return createNewRoot(cursor->table, new_page_num);
  }
  else
  {
    uint32_t parent_page_num = *nodeParent(old_node);
    uint32_t new_max = getNodeMaxKey(old_node);
    void* parent = getPage(cursor->table->pager, parent_page_num);

    updateInternalNodeKey(parent, old_max, new_max);
    internalNodeInsert(cursor->table, parent_page_num, new_page_num);
    return;
  }
}

void DataBase::leafNodeSplitAndInsert(Cursor* cursor, uint32_t key, Row* value)
{
  /*
  Create a new node and move half the cells over.
  Insert the new value in one of the two nodes.
  Update parent or create a new parent.
  */

  // нода = page = 4096 Mb  

  // получение ноды, на которой сейчас находится курсор. Ее надо разбить на две
  void* old_node = getPage(cursor->table->pager, cursor->page_num);
  // получение значения максимального ключа среди cells, которые хранятся в ноде
  uint32_t old_max = getNodeMaxKey(old_node);
        
  // получаем порядковый номер новой ноды, которая будет добалена в конец файла
  uint32_t new_page_num = getUnusedPageNum(cursor->table->pager);
  // физически создали новый объект ноды
  void* new_node = getPage(cursor->table->pager, new_page_num);

  // проинициализировали новый созданный объект как LEAF_NODE
  initializeLeafNode(new_node);

  // ссылку на родителя для новой ноды выставляем такой же как и для старой ноды
  *nodeParent(new_node) = *nodeParent(old_node);
  // ссылку на своего брата для новой ноды выставляем такой же как и для старой ноды
  *leafNodeNextLeaf(new_node) = *leafNodeNextLeaf(old_node);
  // Internal nodes will point to their children by storing the page number that stores the child
  *leafNodeNextLeaf(old_node) = new_page_num;

  /*
  All existing keys plus new key should should be divided
  evenly between old (left) and new (right) nodes.
  Starting from the right, move each key to correct position.
  */
  for (int32_t i = BTree::nLeafNodeMaxCells; i >= 0; i--) {
    void* destination_node;
    // BTree::nLeafNodeLeftSplitCount - это половина от возможного числа детей у ноды
    if (i >= BTree::nLeafNodeLeftSplitCount) {
      destination_node = new_node;
    } else {
      destination_node = old_node;
    }
    // это просто порядковый номер, а не ключ
    uint32_t index_within_node = i % BTree::nLeafNodeLeftSplitCount;
    // это уже destination_cell
    void* destination = leafNodeCellId(destination_node, index_within_node);

    // при ранее выполненном поиске выяснили, что надо вставить новое значение на эту позицию
    if (i == cursor->cell_num) {
      // представили в бинарном виде
      serializeRow(value,
                    leafNodeValueId(destination_node, index_within_node));
      // выставили новый ключ
      *leafNodeKeyId(destination_node, index_within_node) = key;
    } else if (i > cursor->cell_num) {
      memcpy(destination, leafNodeCellId(old_node, i - 1), BTree::nLeafNodeCellSize);
    } else {
      memcpy(destination, leafNodeCellId(old_node, i), BTree::nLeafNodeCellSize);
    }
  }

  /* Update cell count on both leaf nodes */
  *(leafNodeNumCells(old_node)) = BTree::nLeafNodeLeftSplitCount;
  *(leafNodeNumCells(new_node)) = BTree::nLeafNodeRightSplitCount;

  // если у разбиваемой ноды не было родителя, то надо его создать
  if (isNodeRoot(old_node)) {
    return createNewRoot(cursor->table, new_page_num);
  } else {
    // смотрим на какой странице располагался родитель ноды, которую сейчас разбили
    uint32_t parent_page_num = *nodeParent(old_node);
    // получение значения максимального ключа среди cells, которые хранятся в ноде
    uint32_t new_max = getNodeMaxKey(old_node);
    // получение ноды на которой хранится родитель
    void* parent = getPage(cursor->table->pager, parent_page_num);

    // обновляем ключ в интернал ноде. !!!! надо править !!!
    // внутри надо делать memcpy()
    updateInternalNodeKey(parent, old_max, new_max);
    // вставка internal ноды. !!!! надо править !!!
    internalNodeInsert(cursor->table, parent_page_num, new_page_num);
    return;
  }
}

void DataBase::leafNodeInsert(Cursor* cursor, char* key, Row* value)
{
  void* node = getPage(cursor->table->pager, cursor->page_num);

  uint32_t num_cells = *leafNodeNumCells(node);
  if (num_cells >= BTree::nLeafNodeMaxCells)
  {
    // Node full
    // leafNodeSplitAndInsert(cursor, key, value);
    sysLogger.LogToSyslog( "Or no, its not imlemented yet((" );
    return;
  }

  if( cursor->cell_num < num_cells )
  {
    // Make room for new cell
    for( uint32_t i = num_cells; i > cursor->cell_num; i-- )
    {
      memcpy(leafNodeCellId(node, i), leafNodeCellId(node, i - 1),
         BTree::nLeafNodeCellSize);
    }
  }

  *(leafNodeNumCells(node)) += 1;
  // выставить ключ для конкретной ячейки, расположенной на page
  // *(leafNodeKeyId(node, cursor->cell_num)) = key;
  // we need +1 for terminateing zero in the string end ???
  memcpy(leafNodeKey(node, cursor->cell_num), key, strlen(key) /*+ 1*/);
  serializeRow(value, leafNodeValue(node, cursor->cell_num));
}

QueryResult DataBase::executeInsert(Statement* statement, Table* table)
{
  Row* row_to_insert = &(statement->row_to_insert);

  // key_to_insert - это ключ не в raw, а ключ для BTree дерева
  char* key_to_insert = row_to_insert->key;
  Cursor* cursor= tableFind(table, key_to_insert);

  void* node = getPage(table->pager, cursor->page_num);
  uint32_t num_cells = *leafNodeNumCells(node);

  // cell_num - индекс ячейки в кот. надо разместить элемент
  // если надо вставить по-середине, то убеждаемся, что такого ключа раньше не было
  if( cursor->cell_num < num_cells )
  {
    char* key_at_index = leafNodeKey(node, cursor->cell_num);
    if( 0 == compareKeys( key_at_index, key_to_insert ) )
    {
       return QueryResult( eQueryStatus::esDuplicateKey );
    }
  }

  leafNodeInsert( cursor, row_to_insert->key, row_to_insert );

  free(cursor);

  return QueryResult( eQueryStatus::esSuccss );;
}

QueryResult DataBase::executeList(Statement* statement, Table* table)
{
  Cursor* cursor = tableStart( table );

  Row row;
  std::string s_key;
  std::stringstream ss;
  while( ! ( cursor->end_of_table ) )
  {
    s_key = getKey( cursor );

    // deserializeRow( cursorKey(cursor), cursorValue(cursor), &row );
    ss << s_key;
    ss << std::endl;

    cursorAdvance( cursor );
  }

  free(cursor);

  std::string s = ss.str();
  if( ! s.empty() )
  {
     // s.pop_back();
     s.resize( s.size() - 1 );
  }

  QueryResult query_result {eQueryStatus::esSuccss, ss.str()};
  return query_result;
}

QueryResult DataBase::executeStatement(Statement* statement, Table* table)
{
   switch (statement->eType)
   {
      case( eStatementType::stPut ) :
      {
         return executeInsert(statement, table);
      }
      case( eStatementType::stList ) :
      {
         return executeList(statement, table);
      }
   }
}

QueryResult DataBase::executeQuery( const std::string &s_query )
{
   input_buffer = newInputBuffer();
   readInput(input_buffer, s_query );

   QueryResult query_result;

   if( input_buffer->buffer[0] == '.' )
   {
      dbOpen();

      switch( doMetaCommand( input_buffer, table ) )
      {
         case eMetaCmdResult::mcrSuccess :
            query_result.queryStatus = eQueryStatus::esSuccss;
            break;
         case eMetaCmdResult::mcrUnrecognisedCmd :
            sysLogger.LogToSyslog("Unrecognized command '%s'\n", input_buffer->buffer);
            query_result.queryStatus = eQueryStatus::esInvalidCmd;
            break;
      }

      dbClose();

      return query_result;
   }

   Statement statement;
   switch( prepareStatement( input_buffer, &statement ) )
   {
     case ePrepareResult::prSuccess:
       break;
     case ePrepareResult::prStringTooLong:
       sysLogger.LogToSyslog("String is too long");
       query_result.queryStatus = eQueryStatus::esInvalidCmd;
       return query_result;
     case ePrepareResult::prSyntaxError:
       sysLogger.LogToSyslog("Syntax error. Could not parse statement");
       query_result.queryStatus = eQueryStatus::esInvalidCmd;
       return query_result;
     case ePrepareResult::prUnrecognisedStatement:
       sysLogger.LogToSyslog("Unrecognized keyword at start of ", input_buffer->buffer);
       query_result.queryStatus = eQueryStatus::esInvalidCmd;
       return query_result;
   }

   dbOpen();

   query_result =
      executeStatement(&statement, table);

   switch( query_result.queryStatus )
   {
     case( eQueryStatus::esSuccss ):
       sysLogger.LogToSyslog( "Executed" );
       break;
     case( eQueryStatus::esDuplicateKey ):
       sysLogger.LogToSyslog( "Error: Duplicate key" );
       break;
   }

   dbClose();

   return query_result;
}
