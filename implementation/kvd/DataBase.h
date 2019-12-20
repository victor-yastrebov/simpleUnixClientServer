/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for store/load data from database
  * </pre>
 *
 * @class UDSServer
 */

// implemented via:
// https://cstack.github.io/db_tutorial

#include<string>

typedef struct {
  char* buffer;
  size_t buffer_length;
  ssize_t input_length;
} InputBuffer;

typedef enum {
  EXECUTE_SUCCESS,
  EXECUTE_DUPLICATE_KEY,
} ExecuteResult;

typedef enum {
  META_COMMAND_SUCCESS,
  META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

typedef enum {
  PREPARE_SUCCESS,
  PREPARE_NEGATIVE_ID,
  PREPARE_STRING_TOO_LONG,
  PREPARE_SYNTAX_ERROR,
  PREPARE_UNRECOGNIZED_STATEMENT
} PrepareResult;

typedef enum { STATEMENT_INSERT, STATEMENT_SELECT } StatementType;

#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255
typedef struct {
  uint32_t id;
  char username[COLUMN_USERNAME_SIZE + 1];
  char email[COLUMN_EMAIL_SIZE + 1];
} Row;

typedef struct {
  StatementType type;
  Row row_to_insert;  // only used by insert statement
} Statement;

#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

const uint32_t ID_SIZE = size_of_attribute(Row, id);
const uint32_t USERNAME_SIZE = size_of_attribute(Row, username);
const uint32_t EMAIL_SIZE = size_of_attribute(Row, email);
const uint32_t ID_OFFSET = 0;
const uint32_t USERNAME_OFFSET = ID_OFFSET + ID_SIZE;
const uint32_t EMAIL_OFFSET = USERNAME_OFFSET + USERNAME_SIZE;
const uint32_t ROW_SIZE = ID_SIZE + USERNAME_SIZE + EMAIL_SIZE;

const uint32_t PAGE_SIZE = 4096;
#define TABLE_MAX_PAGES 100

typedef struct {
  int file_descriptor;
  uint32_t file_length;
  uint32_t num_pages;
  void* pages[TABLE_MAX_PAGES];
} Pager;

typedef struct {
  Pager* pager;
  uint32_t root_page_num;
} Table;

typedef struct {
  Table* table;
  uint32_t page_num;
  uint32_t cell_num;
  bool end_of_table;  // Indicates a position one past the last element
} Cursor;

typedef enum { NODE_INTERNAL, NODE_LEAF } NodeType;

/*
 * Common Node Header Layout
 */
const uint32_t NODE_TYPE_SIZE = sizeof(uint8_t);
const uint32_t NODE_TYPE_OFFSET = 0;
const uint32_t IS_ROOT_SIZE = sizeof(uint8_t);
const uint32_t IS_ROOT_OFFSET = NODE_TYPE_SIZE;
const uint32_t PARENT_POINTER_SIZE = sizeof(uint32_t);
const uint32_t PARENT_POINTER_OFFSET = IS_ROOT_OFFSET + IS_ROOT_SIZE;
const uint8_t COMMON_NODE_HEADER_SIZE =
    NODE_TYPE_SIZE + IS_ROOT_SIZE + PARENT_POINTER_SIZE;

/*
 * Internal Node Header Layout
 */
const uint32_t INTERNAL_NODE_NUM_KEYS_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_NUM_KEYS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t INTERNAL_NODE_RIGHT_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_RIGHT_CHILD_OFFSET =
    INTERNAL_NODE_NUM_KEYS_OFFSET + INTERNAL_NODE_NUM_KEYS_SIZE;
const uint32_t INTERNAL_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
                                           INTERNAL_NODE_NUM_KEYS_SIZE +
                                           INTERNAL_NODE_RIGHT_CHILD_SIZE;

/*
 * Internal Node Body Layout
 */
const uint32_t INTERNAL_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CHILD_SIZE = sizeof(uint32_t);
const uint32_t INTERNAL_NODE_CELL_SIZE =
    INTERNAL_NODE_CHILD_SIZE + INTERNAL_NODE_KEY_SIZE;
/* Keep this small for testing */
const uint32_t INTERNAL_NODE_MAX_CELLS = 3;

/*
 * Leaf Node Header Layout
 */
const uint32_t LEAF_NODE_NUM_CELLS_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NUM_CELLS_OFFSET = COMMON_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_NEXT_LEAF_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_NEXT_LEAF_OFFSET =
    LEAF_NODE_NUM_CELLS_OFFSET + LEAF_NODE_NUM_CELLS_SIZE;
const uint32_t LEAF_NODE_HEADER_SIZE = COMMON_NODE_HEADER_SIZE +
                                       LEAF_NODE_NUM_CELLS_SIZE +
                                       LEAF_NODE_NEXT_LEAF_SIZE;

/*
 * Leaf Node Body Layout
 */
const uint32_t LEAF_NODE_KEY_SIZE = sizeof(uint32_t);
const uint32_t LEAF_NODE_KEY_OFFSET = 0;
const uint32_t LEAF_NODE_VALUE_SIZE = ROW_SIZE;
const uint32_t LEAF_NODE_VALUE_OFFSET =
    LEAF_NODE_KEY_OFFSET + LEAF_NODE_KEY_SIZE;
const uint32_t LEAF_NODE_CELL_SIZE = LEAF_NODE_KEY_SIZE + LEAF_NODE_VALUE_SIZE;
const uint32_t LEAF_NODE_SPACE_FOR_CELLS = PAGE_SIZE - LEAF_NODE_HEADER_SIZE;
const uint32_t LEAF_NODE_MAX_CELLS =
    LEAF_NODE_SPACE_FOR_CELLS / LEAF_NODE_CELL_SIZE;
const uint32_t LEAF_NODE_RIGHT_SPLIT_COUNT = (LEAF_NODE_MAX_CELLS + 1) / 2;
const uint32_t LEAF_NODE_LEFT_SPLIT_COUNT =
    (LEAF_NODE_MAX_CELLS + 1) - LEAF_NODE_RIGHT_SPLIT_COUNT;

class DataBase
{
public:
          DataBase();
         ~DataBase();
          DataBase( const DataBase& ) = delete;
          DataBase& operator=( const DataBase& ) = delete;
   void   Initialize( char *filename );
    int   ExecuteQuery( const std::string &s );

private:
       void   print_row(Row* row);
   NodeType   get_node_type(void* node);
       void   set_node_type(void* node, NodeType type);
       bool   is_node_root(void* node);
       void   set_node_root(void* node, bool is_root);
   uint32_t*  node_parent(void* node);
   uint32_t*  internal_node_num_keys(void* node);
   uint32_t*  internal_node_right_child(void* node);
   uint32_t*  internal_node_cell(void* node, uint32_t cell_num);
   uint32_t*  internal_node_child(void* node, uint32_t child_num);
   uint32_t*  internal_node_key(void* node, uint32_t key_num);
   uint32_t*  leaf_node_num_cells(void* node);
   uint32_t*  leaf_node_next_leaf(void* node);
       void*  leaf_node_cell(void* node, uint32_t cell_num);
   uint32_t*  leaf_node_key(void* node, uint32_t cell_num);
   void*   leaf_node_value(void* node, uint32_t cell_num);
   uint32_t    get_node_max_key(void* node);
   void    print_constants();
   void*   get_page(Pager* pager, uint32_t page_num);
   void   indent(uint32_t level);
   void   print_tree(Pager* pager, uint32_t page_num, uint32_t indentation_level);
   void   serialize_row(Row* source, void* destination);
   void   deserialize_row(void* source, Row* destination);
   void   initialize_leaf_node(void* node);
   void   initialize_internal_node(void* node);
   Cursor*  leaf_node_find(Table* table, uint32_t page_num, uint32_t key);
   uint32_t   internal_node_find_child(void* node, uint32_t key);
   Cursor*  internal_node_find(Table* table, uint32_t page_num, uint32_t key);
   Cursor*  table_find(Table* table, uint32_t key);
   Cursor*  table_start(Table* table);
   void*  cursor_value(Cursor* cursor);
   void   cursor_advance(Cursor* cursor);
   Pager*  pager_open(const char* filename);
   Table*  db_open(const char* filename);
   InputBuffer*  new_input_buffer();
   void   print_prompt();
   void   read_input(InputBuffer* input_buffer, const std::string &s_query);
   void   close_input_buffer(InputBuffer* input_buffer);
   void   pager_flush(Pager* pager, uint32_t page_num);
   void   db_close(Table* table);
   MetaCommandResult   do_meta_command(InputBuffer* input_buffer, Table* table);
   PrepareResult   prepare_insert(InputBuffer* input_buffer, Statement* statement);
   PrepareResult   prepare_statement(InputBuffer* input_buffer, Statement* statement);
   uint32_t   get_unused_page_num(Pager* pager);
   void   create_new_root(Table* table, uint32_t right_child_page_num);
   void   internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num);
   void   update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key);
   void   leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value);
   void   leaf_node_insert(Cursor* cursor, uint32_t key, Row* value);
   ExecuteResult   execute_insert(Statement* statement, Table* table);
   ExecuteResult   execute_select(Statement* statement, Table* table);
   ExecuteResult   execute_statement(Statement* statement, Table* table);

   InputBuffer* input_buffer;
   Table* table;
   std::string sCurQuery;
};
