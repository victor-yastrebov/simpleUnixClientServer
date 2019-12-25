/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for processing incoming connections from clients
  * </pre>
 *
 * @class UDSServer
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

void DataBase::print_row(Row* row)
{
   sysLogger.LogToSyslog( "(", row->key, ", ", row->value, ")" );
}

eNodeType DataBase::get_node_type(void* node) {
  uint8_t value = *((uint8_t*)(node + BTree::nNodeTypeOffset));
  return (eNodeType)value;
}

void DataBase::set_node_type(void* node, eNodeType type) {
  uint8_t value = type;
  *((uint8_t*)(node + BTree::nNodeTypeOffset)) = value;
}

bool DataBase::is_node_root(void* node) {
  uint8_t value = *((uint8_t*)(node + BTree::nIsRootOffset));
  return (bool)value;
}

void DataBase::set_node_root(void* node, bool is_root) {
  uint8_t value = is_root;
  *((uint8_t*)(node + BTree::nIsRootOffset)) = value;
}

uint32_t* DataBase::node_parent(void* node)
{
   return static_cast<uint32_t*>( node ) + BTree::nParentPointerOffset;
}

uint32_t* DataBase::internal_node_num_keys(void* node) {
  return static_cast<uint32_t*>( node ) + BTree::nInternalNodeNumKeysOffset;
}

uint32_t* DataBase::internal_node_right_child(void* node) {
  return static_cast<uint32_t*>( node ) + BTree::nInternalNodeRightChildOffset;
}

uint32_t* DataBase::internal_node_cell(void* node, uint32_t cell_num) {
  return static_cast<uint32_t*>( node ) + BTree::nInternalNodeHeaderSize + cell_num * BTree::nInternalNodeCellSize;
}

uint32_t* DataBase::internal_node_child(void* node, uint32_t child_num) {
  uint32_t num_keys = *internal_node_num_keys(node);
  if (child_num > num_keys) {
    sysLogger.LogToSyslog( "Tried to access child_num %d > num_keys %d\n", child_num, num_keys );
    exit(EXIT_FAILURE);
  } else if (child_num == num_keys) {
    return internal_node_right_child(node);
  } else {
    return internal_node_cell(node, child_num);
  }
}

uint32_t* DataBase::internal_node_key(void* node, uint32_t key_num) {
  return (uint32_t*)internal_node_cell(node, key_num) + BTree::nInternalNodeChildSize;
}

uint32_t* DataBase::leaf_node_num_cells(void* node) {
  uint32_t* val = static_cast<uint32_t*>( node ) + BTree::nLeafNodeNumCellsOffset;
  return static_cast<uint32_t*>( node ) + BTree::nLeafNodeNumCellsOffset;
}

uint32_t* DataBase::leaf_node_next_leaf(void* node) {
  return static_cast<uint32_t*>( node ) + BTree::nLeafNodeNextLeafOffset;
}

void* DataBase::leaf_node_cell(void* node, uint32_t cell_num) {
  return static_cast<uint32_t*>( node ) + BTree::nLeafNodeHeaderSize + cell_num * BTree::nLeafNodeCellSize;
}

/*
 * Получить указатель на область памяти cell (на page их может быть несколько),
 *  в котором хранится key для навигации по BTree
*/
uint32_t* DataBase::leaf_node_key(void* node, uint32_t cell_num) {
  return static_cast<uint32_t*>( leaf_node_cell(node, cell_num) );
}

// получить указатель на начало памяти в page, где хранится cell с индексом cell_num
// Cell - это пара: {ключ, значение}
void* DataBase::leafNodeCell(void* node, uint32_t cell_num) {
   // void* node - указатель на начало page
  return static_cast<char*>( node ) + BTree::nLeafNodeHeaderSize + cell_num * BTree::nLeafNodeCellSize;
}

char* DataBase::leafNodeKey(void* node, uint32_t cell_num) {
  return static_cast<char*>( leafNodeCell(node, cell_num) );
}

void* DataBase::leafNodeValue(void* node, uint32_t cell_num)
{
  // в будущем LEAF_NODE_KEY_SIZE должна стать переменным числом
  return leafNodeCell(node, cell_num) + BTree::nLeafNodeStrKeySize;
}

void* DataBase::leaf_node_value(void* node, uint32_t cell_num)
{
  return leaf_node_cell(node, cell_num) + BTree::nLeafNodeKeySize;
}

uint32_t DataBase::get_node_max_key(void* node)
{
  switch (get_node_type(node))
  {
    case eNodeType::ntInternal:
      return *internal_node_key(node, *internal_node_num_keys(node) - 1);
    case eNodeType::ntLeaf:
      return *leaf_node_key(node, *leaf_node_num_cells(node) - 1);
  }
}

void DataBase::print_constants() {
  sysLogger.LogToSyslog("nRowSize: ", BTree::nRowSize);
  sysLogger.LogToSyslog("nCommonNodeHeaderSize: ", BTree::nCommonNodeHeaderSize);
  sysLogger.LogToSyslog("nLeafNodeHeaderSize: ", BTree::nLeafNodeHeaderSize);
  sysLogger.LogToSyslog("nLeafNodeCellSize: ", BTree::nLeafNodeCellSize);
  sysLogger.LogToSyslog("nLeafNodeSpaceForCells: ", BTree::nLeafNodeSpaceForCells);
  sysLogger.LogToSyslog("nLeafNodeMaxCells: ", BTree::nLeafNodeMaxCells);
}

void* DataBase::get_page( Pager* pager, uint32_t page_num )
{
  if( page_num > TABLE_MAX_PAGES )
  {
   sysLogger.LogToSyslog( "Tried to fetch page number out of bounds. ",
      page_num, " > ", TABLE_MAX_PAGES );
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

void DataBase::indent(uint32_t level) {
  /*for (uint32_t i = 0; i < level; i++) {
    sysLogger.LogToSyslog("  ");
  }*/
}

void DataBase::print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level) {
  void* node = get_page(pager, page_num);
  uint32_t num_keys, child;

  switch (get_node_type(node)) {
    case (eNodeType::ntLeaf):
      num_keys = *leaf_node_num_cells(node);
      indent(indentation_level);
      sysLogger.LogToSyslog("- leaf (size", num_keys, ")");
      for (uint32_t i = 0; i < num_keys; i++)
      {
        indent(indentation_level + 1);
        sysLogger.LogToSyslog("  - ", *leafNodeKey(node, i));
      }
      break;
    case (eNodeType::ntInternal):
      num_keys = *internal_node_num_keys(node);
      indent(indentation_level);
      sysLogger.LogToSyslog("- internal (size ", num_keys, ")");
      for (uint32_t i = 0; i < num_keys; i++) {
        child = *internal_node_child(node, i);
        print_tree(pager, child, indentation_level + 1);

        indent(indentation_level + 1);
        sysLogger.LogToSyslog("- key ", *internal_node_key(node, i));
      }
      child = *internal_node_right_child(node);
      print_tree(pager, child, indentation_level + 1);
      break;
  }
}

void DataBase::serialize_row(Row* source, void* destination) {
  memcpy(destination + BTree::nValueStrOffset, &(source->value), BTree::nValueStrSize );
  // memcpy(destination + USERNAME_OFFSET, &(source->username), BTree::nUserNameSize);
  // memcpy(destination + EMAIL_OFFSET, &(source->email), BTree::nEmailSize);
}

std::string DataBase::getKey( Cursor* p_cursor )
{
   std::string str;
   str.resize( BTree::nKeyStrSize );

   void *p_key = cursor_key( p_cursor );

   memcpy( (void*)str.data(), p_key, BTree::nKeyStrSize );
   str.at(etalonKeySize) = '\0';

   return str;
}

void DataBase::deserialize_row( void *key, void* source, Row* destination )
{
  memcpy( &( destination->value ), source + BTree::nValueStrOffset, BTree::nValueStrSize );
  memcpy( &( destination->key ), key, BTree::nKeyStrSize );
  destination->key[etalonKeySize] = '\0';
  // memcpy(&(destination->id), source + ID_OFFSET, ID_SIZE);
  // memcpy(&(destination->username), source + USERNAME_OFFSET, USERNAME_SIZE);
  // memcpy(&(destination->email), source + EMAIL_OFFSET, BTree::nEmailSize);
}

void DataBase::initialize_leaf_node(void* node) {
  set_node_type(node, eNodeType::ntLeaf );
  set_node_root(node, false);
  *leaf_node_num_cells(node) = 0;
  *leaf_node_next_leaf(node) = 0;  // 0 represents no sibling
}

void DataBase::initialize_internal_node(void* node) {
  set_node_type(node, eNodeType::ntInternal);
  set_node_root(node, false);
  *internal_node_num_keys(node) = 0;
}

/**
 * Find position in leaf node
 * This function will return:
 * - the position of the key OR
 * - the position of another key that we’ll need to move if we want to insert the new key OR
 * - the position one past the last key
 */
Cursor* DataBase::leaf_node_find(Table* table, uint32_t page_num, char* key) {
  void* node = get_page(table->pager, page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

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

uint32_t DataBase::internal_node_find_child(void* node, uint32_t key) {
  /*
  Return the index of the child which should contain
  the given key.
  */

  uint32_t num_keys = *internal_node_num_keys(node);

  /* Binary search */
  uint32_t min_index = 0;
  uint32_t max_index = num_keys; /* there is one more child than key */

  while (min_index != max_index) {
    uint32_t index = (min_index + max_index) / 2;
    uint32_t key_to_right = *internal_node_key(node, index);
    if (key_to_right >= key) {
      max_index = index;
    } else {
      min_index = index + 1;
    }
  }

  return min_index;
}

Cursor* DataBase::internal_node_find(Table* table, uint32_t page_num, uint32_t key) {
  void* node = get_page(table->pager, page_num);

  uint32_t child_index = internal_node_find_child(node, key);
  uint32_t child_num = *internal_node_child(node, child_index);
  void* child = get_page(table->pager, child_num);
  switch (get_node_type(child)) {
    case eNodeType::ntLeaf:
      // return leaf_node_find(table, child_num, key);
    case eNodeType::ntInternal:
      return internal_node_find(table, child_num, key);
  }
}

/*
Return the position of the given key.for BTree, not key in the row
If the key is not present, return the position
where it should be inserted
*/
Cursor* DataBase::table_find(Table* table, char* key) {
  uint32_t root_page_num = table->root_page_num;
  void* root_node = get_page(table->pager, root_page_num);

  if (get_node_type(root_node) == eNodeType::ntLeaf )
  {
     // найди в таблице, начиная с ее первой страницы такой вот ключ
     return leaf_node_find(table, root_page_num, key);
  } else {
    sysLogger.LogToSyslog( "!!!!!!! INTERNAL NODE FIND IS NOT IMPLEMENTED !!!");
    return NULL;
    // return internal_node_find(table, root_page_num, key);
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

Cursor* DataBase::table_start(Table* table) {
   // надо установить курсор на самую первую строку
   Cursor* cursor = createCursorForFirstCell( table );

   void* node = get_page( table->pager, cursor->page_num );
   uint32_t num_cells = *leaf_node_num_cells( node );
   cursor->end_of_table = ( num_cells == 0 );

   return cursor;
}

void* DataBase::cursor_value(Cursor* cursor) {
  uint32_t page_num = cursor->page_num;
  void* page = get_page(cursor->table->pager, page_num);
  return leafNodeValue(page, cursor->cell_num);
}

void* DataBase::cursor_key(Cursor* cursor) {
   uint32_t page_num = cursor->page_num;
   void* page = get_page(cursor->table->pager, page_num);
   return leafNodeKey(page, cursor->cell_num);
}


void DataBase::cursor_advance(Cursor* cursor) {
  uint32_t page_num = cursor->page_num;
  void* node = get_page(cursor->table->pager, page_num);

  cursor->cell_num += 1;
  if (cursor->cell_num >= (*leaf_node_num_cells(node))) {
    /* Advance to next leaf node */
    uint32_t next_page_num = *leaf_node_next_leaf(node);
    if (next_page_num == 0) {
      /* This was rightmost leaf */
      cursor->end_of_table = true;
    } else {
      cursor->page_num = next_page_num;
      cursor->cell_num = 0;
    }
  }
}

Pager* DataBase::pager_open(const char* filename) {
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

  for( uint32_t i = 0; i < TABLE_MAX_PAGES; i++ )
  {
    pager->pages[i] = NULL;
  }

  return pager;
}

void DataBase::db_open()
{
  Pager* pager = pager_open( sDbFilePath.c_str() );

  table = (Table *)malloc(sizeof(Table));
  table->pager = pager;
  table->root_page_num = 0;

  if (pager->num_pages == 0)
  {
    // New database file. Initialize page 0 as leaf node.
    void* root_node = get_page(pager, 0);
    initialize_leaf_node(root_node);
    set_node_root(root_node, true);
  }
}

InputBuffer* DataBase::new_input_buffer() {
  InputBuffer* input_buffer = (InputBuffer*)malloc(sizeof(InputBuffer));
  input_buffer->buffer = NULL;
  input_buffer->buffer_length = 0;
  input_buffer->input_length = 0;

  return input_buffer;
}

void DataBase::print_prompt() { sysLogger.LogToSyslog("db > "); }

void DataBase::read_input(InputBuffer* input_buffer, const std::string &s_query ) {
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

void DataBase::close_input_buffer(InputBuffer* input_buffer) {
  // free(input_buffer->buffer);
  free(input_buffer);
}

void DataBase::pager_flush(Pager* pager, uint32_t page_num) {
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

void DataBase::db_close() {
  Pager* pager = table->pager;

  // flushes the page cache to disk
  for (uint32_t i = 0; i < pager->num_pages; i++) {
    if (pager->pages[i] == NULL) {
      continue;
    }
    pager_flush(pager, i);
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
  for (uint32_t i = 0; i < TABLE_MAX_PAGES; i++) {
    void* page = pager->pages[i];
    if (page) {
      free(page);
      pager->pages[i] = NULL;
    }
  }
  free(pager);
  free(table);
}

eMetaCmdResult DataBase::do_meta_command(InputBuffer* input_buffer, Table* table) {
  if (strcmp(input_buffer->buffer, ".exit") == 0) {
    close_input_buffer(input_buffer);
    db_close();
    return eMetaCmdResult::mcrSuccess;
  } else if (strcmp(input_buffer->buffer, ".btree") == 0) {
    sysLogger.LogToSyslog( "Tree:" );
    print_tree(table->pager, 0, 0);
    return eMetaCmdResult::mcrSuccess;
  } else if (strcmp(input_buffer->buffer, ".constants") == 0) {
    sysLogger.LogToSyslog("Constants:\n");
    print_constants();
    return eMetaCmdResult::mcrSuccess;
  } else {
    return eMetaCmdResult::mcrUnrecognisedCmd;
  }
}

ePrepareResult DataBase::prepare_insert(InputBuffer* input_buffer, Statement* statement) {
  statement->eType = eStatementType::stPut;

  char* keyword = strtok(input_buffer->buffer, " ");

  char* key = strtok(NULL, " ");
  char* value = strtok(NULL, " ");

  if (key == NULL || value == NULL)
  {
    return ePrepareResult::prSyntaxError;
  }

  const size_t key_len = strlen( key );
  if( key_len != etalonKeySize )
  {
     return ePrepareResult::prStringTooLong;
  }

  if (strlen(value) > VALUE_MAX_SIZE)
  {
     return ePrepareResult::prStringTooLong;
  }

  strcpy(statement->row_to_insert.key, key);
  strcpy(statement->row_to_insert.value, value);

  return ePrepareResult::prSuccess;
}

ePrepareResult DataBase::prepare_statement(InputBuffer* input_buffer,
                                Statement* statement) {
  if (strncmp(input_buffer->buffer, "put", 3) == 0) {
    return prepare_insert(input_buffer, statement);
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
uint32_t DataBase::get_unused_page_num(Pager* pager) { return pager->num_pages; }

void DataBase::create_new_root(Table* table, uint32_t right_child_page_num)
{
  /*
  Handle splitting the root.
  Old root copied to new page, becomes left child.
  Address of right child passed in.
  Re-initialize root page to contain the new root node.
  New root node points to two children.
  */

  void* root = get_page(table->pager, table->root_page_num);
  void* right_child = get_page(table->pager, right_child_page_num);
  uint32_t left_child_page_num = get_unused_page_num(table->pager);
  void* left_child = get_page(table->pager, left_child_page_num);

  /* Left child has data copied from old root */
  memcpy(left_child, root, BTree::nPageSize);
  set_node_root(left_child, false);

  /* Root node is a new internal node with one key and two children */
  initialize_internal_node(root);
  set_node_root(root, true);
  *internal_node_num_keys(root) = 1;
  *internal_node_child(root, 0) = left_child_page_num;
  uint32_t left_child_max_key = get_node_max_key(left_child);
  *internal_node_key(root, 0) = left_child_max_key;
  *internal_node_right_child(root) = right_child_page_num;
  *node_parent(left_child) = table->root_page_num;
  *node_parent(right_child) = table->root_page_num;
}

void DataBase::internal_node_insert(Table* table, uint32_t parent_page_num,
                          uint32_t child_page_num) {
  /*
  Add a new child/key pair to parent that corresponds to child
  */

  void* parent = get_page(table->pager, parent_page_num);
  void* child = get_page(table->pager, child_page_num);
  uint32_t child_max_key = get_node_max_key(child);
  uint32_t index = internal_node_find_child(parent, child_max_key);

  uint32_t original_num_keys = *internal_node_num_keys(parent);
  *internal_node_num_keys(parent) = original_num_keys + 1;

  if (original_num_keys >= BTree::nInternalNodeMaxCells) {
    sysLogger.LogToSyslog("Need to implement splitting internal node\n");
    exit(EXIT_FAILURE);
  }

  uint32_t right_child_page_num = *internal_node_right_child(parent);
  void* right_child = get_page(table->pager, right_child_page_num);

  if (child_max_key > get_node_max_key(right_child)) {
    /* Replace right child */
    *internal_node_child(parent, original_num_keys) = right_child_page_num;
    *internal_node_key(parent, original_num_keys) =
        get_node_max_key(right_child);
    *internal_node_right_child(parent) = child_page_num;
  } else {
    /* Make room for the new cell */
    for (uint32_t i = original_num_keys; i > index; i--) {
      void* destination = internal_node_cell(parent, i);
      void* source = internal_node_cell(parent, i - 1);
      memcpy(destination, source, BTree::nInternalNodeCellSize);
    }
    *internal_node_child(parent, index) = child_page_num;
    *internal_node_key(parent, index) = child_max_key;
  }
}

void DataBase::update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key) {
  uint32_t old_child_index = internal_node_find_child(node, old_key);
  *internal_node_key(node, old_child_index) = new_key;
}

void DataBase::leafNodeSplitAndInsert( Cursor* cursor, char* key, Row* value )
{
  /*
  Create a new node and move half the cells over.
  Insert the new value in one of the two nodes.
  Update parent or create a new parent.
  */
  void* old_node = get_page(cursor->table->pager, cursor->page_num);

  // получить ключ с наибольшим значением
  // !!! надо править !!!
  uint32_t old_max = get_node_max_key(old_node);

  // получаем номер страницы, которую надо будет создать
  uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
  // создали страницу в памяти
  void* new_node = get_page(cursor->table->pager, new_page_num);

  initialize_leaf_node(new_node);
  *node_parent(new_node) = *node_parent(old_node);
  *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
  *leaf_node_next_leaf(old_node) = new_page_num;

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
    // void* destination = leaf_node_cell(destination_node, index_within_node);
    void* destination = leafNodeCell(destination_node, index_within_node);

    if (i == cursor->cell_num)
    {
      serialize_row(value,
                    leaf_node_value(destination_node, index_within_node));
      // *leaf_node_key(destination_node, index_within_node) = key;
      memcpy(leafNodeKey(destination_node, index_within_node), key, strlen(key));
    }
    else if (i > cursor->cell_num)
    {
      memcpy(destination, leaf_node_cell(old_node, i - 1), BTree::nLeafNodeCellSize);
    }
    else
    {
      memcpy(destination, leaf_node_cell(old_node, i), BTree::nLeafNodeCellSize);
    }
  }

  /* Update cell count on both leaf nodes */
  *(leaf_node_num_cells(old_node)) = BTree::nLeafNodeLeftSplitCount;
  *(leaf_node_num_cells(new_node)) = BTree::nLeafNodeRightSplitCount;

  // if original node is root, than it has no parent
  if (is_node_root(old_node))
  {
    return create_new_root(cursor->table, new_page_num);
  }
  else
  {
    uint32_t parent_page_num = *node_parent(old_node);
    uint32_t new_max = get_node_max_key(old_node);
    void* parent = get_page(cursor->table->pager, parent_page_num);

    update_internal_node_key(parent, old_max, new_max);
    internal_node_insert(cursor->table, parent_page_num, new_page_num);
    return;
  }
}

void DataBase::leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value) {
  /*
  Create a new node and move half the cells over.
  Insert the new value in one of the two nodes.
  Update parent or create a new parent.
  */

  // нода = page = 4096 Mb  

  // получение ноды, на которой сейчас находится курсор. Ее надо разбить на две
  void* old_node = get_page(cursor->table->pager, cursor->page_num);
  // получение значения максимального ключа среди cells, которые хранятся в ноде
  uint32_t old_max = get_node_max_key(old_node);
        
  // получаем порядковый номер новой ноды, которая будет добалена в конец файла
  uint32_t new_page_num = get_unused_page_num(cursor->table->pager);
  // физически создали новый объект ноды
  void* new_node = get_page(cursor->table->pager, new_page_num);

  // проинициализировали новый созданный объект как LEAF_NODE
  initialize_leaf_node(new_node);

  // ссылку на родителя для новой ноды выставляем такой же как и для старой ноды
  *node_parent(new_node) = *node_parent(old_node);
  // ссылку на своего брата для новой ноды выставляем такой же как и для старой ноды
  *leaf_node_next_leaf(new_node) = *leaf_node_next_leaf(old_node);
  // Internal nodes will point to their children by storing the page number that stores the child
  *leaf_node_next_leaf(old_node) = new_page_num;

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
    void* destination = leaf_node_cell(destination_node, index_within_node);

    // при ранее выполненном поиске выяснили, что надо вставить новое значение на эту позицию
    if (i == cursor->cell_num) {
      // представили в бинарном виде
      serialize_row(value,
                    leaf_node_value(destination_node, index_within_node));
      // выставили новый ключ
      *leaf_node_key(destination_node, index_within_node) = key;
    } else if (i > cursor->cell_num) {
      memcpy(destination, leaf_node_cell(old_node, i - 1), BTree::nLeafNodeCellSize);
    } else {
      memcpy(destination, leaf_node_cell(old_node, i), BTree::nLeafNodeCellSize);
    }
  }

  /* Update cell count on both leaf nodes */
  *(leaf_node_num_cells(old_node)) = BTree::nLeafNodeLeftSplitCount;
  *(leaf_node_num_cells(new_node)) = BTree::nLeafNodeRightSplitCount;

  // если у разбиваемой ноды не было родителя, то надо его создать
  if (is_node_root(old_node)) {
    return create_new_root(cursor->table, new_page_num);
  } else {
    // смотрим на какой странице располагался родитель ноды, которую сейчас разбили
    uint32_t parent_page_num = *node_parent(old_node);
    // получение значения максимального ключа среди cells, которые хранятся в ноде
    uint32_t new_max = get_node_max_key(old_node);
    // получение ноды на которой хранится родитель
    void* parent = get_page(cursor->table->pager, parent_page_num);

    // обновляем ключ в интернал ноде. !!!! надо править !!!
    // внутри надо делать memcpy()
    update_internal_node_key(parent, old_max, new_max);
    // вставка internal ноды. !!!! надо править !!!
    internal_node_insert(cursor->table, parent_page_num, new_page_num);
    return;
  }
}

void DataBase::leaf_node_insert(Cursor* cursor, char* key, Row* value) {
  void* node = get_page(cursor->table->pager, cursor->page_num);

  uint32_t num_cells = *leaf_node_num_cells(node);
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
      memcpy(leaf_node_cell(node, i), leaf_node_cell(node, i - 1),
         BTree::nLeafNodeCellSize);
    }
  }

  *(leaf_node_num_cells(node)) += 1;
  // выставить ключ для конкретной ячейки, расположенной на page
  // *(leaf_node_key(node, cursor->cell_num)) = key;
  // we need +1 for terminateing zero in the string end ???
  memcpy(leafNodeKey(node, cursor->cell_num), key, strlen(key) /*+ 1*/);
  serialize_row(value, leafNodeValue(node, cursor->cell_num));
}

QueryResult DataBase::execute_insert(Statement* statement, Table* table) {
  Row* row_to_insert = &(statement->row_to_insert);

  // key_to_insert - это ключ не в raw, а ключ для BTree дерева
  char* key_to_insert = row_to_insert->key;
  Cursor* cursor= table_find(table, key_to_insert);

  void* node = get_page(table->pager, cursor->page_num);
  uint32_t num_cells = *leaf_node_num_cells(node);

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

  leaf_node_insert( cursor, row_to_insert->key, row_to_insert );

  free(cursor);

  return QueryResult( eQueryStatus::esSuccss );;
}

QueryResult DataBase::execute_list(Statement* statement, Table* table) {
  Cursor* cursor = table_start( table );

  Row row;
  std::string s_key;
  std::stringstream ss;
  while( ! ( cursor->end_of_table ) )
  {
    s_key = getKey( cursor );

    // deserialize_row( cursor_key(cursor), cursor_value(cursor), &row );
    ss << s_key;
    ss << std::endl;
    // print_row(&row);

    cursor_advance( cursor );
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

QueryResult DataBase::execute_statement(Statement* statement, Table* table)
{
   switch (statement->eType)
   {
      case( eStatementType::stPut ) :
      {
         return execute_insert(statement, table);
      }
      case( eStatementType::stList ) :
      {
         return execute_list(statement, table);
      }
   }
}

QueryResult DataBase::ExecuteQuery( const std::string &s_query )
{
   input_buffer = new_input_buffer();
   read_input(input_buffer, s_query );

   QueryResult query_result;

   if( input_buffer->buffer[0] == '.' )
   {
      db_open();

      switch( do_meta_command( input_buffer, table ) )
      {
         case eMetaCmdResult::mcrSuccess :
            query_result.queryStatus = eQueryStatus::esSuccss;
            break;
         case eMetaCmdResult::mcrUnrecognisedCmd :
            sysLogger.LogToSyslog("Unrecognized command '%s'\n", input_buffer->buffer);
            query_result.queryStatus = eQueryStatus::esInvalidCmd;
            break;
      }

      db_close();

      return query_result;
   }

   Statement statement;
   switch( prepare_statement( input_buffer, &statement ) )
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

   db_open();

   query_result =
      execute_statement(&statement, table);

   switch( query_result.queryStatus )
   {
     case( eQueryStatus::esSuccss ):
       sysLogger.LogToSyslog( "Executed" );
       break;
     case( eQueryStatus::esDuplicateKey ):
       sysLogger.LogToSyslog( "Error: Duplicate key" );
       break;
   }

   db_close();

   return query_result;
}

/*
int main(int argc, char* argv[]) {
  if (argc < 2) {
    sysLogger.LogToSyslog("Must supply a database filename.\n");
    exit(EXIT_FAILURE);
  }

  char* filename = argv[1];
  Table* table = db_open(filename);

  InputBuffer* input_buffer = new_input_buffer();
  while (true) {
    print_prompt();
    read_input(input_buffer);

    if (input_buffer->buffer[0] == '.') {
      switch (do_meta_command(input_buffer, table)) {
        case (META_COMMAND_SUCCESS):
          continue;
        case (META_COMMAND_UNRECOGNIZED_COMMAND):
          sysLogger.LogToSyslog("Unrecognized command '%s'\n", input_buffer->buffer);
          continue;
      }
    }

    Statement statement;
    switch (prepare_statement(input_buffer, &statement)) {
      case (PREPARE_SUCCESS):
        break;
      case (PREPARE_NEGATIVE_ID):
        sysLogger.LogToSyslog("ID must be positive.\n");
        continue;
      case (PREPARE_STRING_TOO_LONG):
        sysLogger.LogToSyslog("String is too long.\n");
        continue;
      case (PREPARE_SYNTAX_ERROR):
        sysLogger.LogToSyslog("Syntax error. Could not parse statement.\n");
        continue;
      case (PREPARE_UNRECOGNIZED_STATEMENT):
        sysLogger.LogToSyslog("Unrecognized keyword at start of '%s'.\n",
               input_buffer->buffer);
        continue;
    }

    switch (execute_statement(&statement, table)) {
      case (EXECUTE_SUCCESS):
        sysLogger.LogToSyslog("Executed.\n");
        break;
      case (EXECUTE_DUPLICATE_KEY):
        sysLogger.LogToSyslog("Error: Duplicate key.\n");
        break;
    }
  }
}
*/
