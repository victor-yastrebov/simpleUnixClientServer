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

#include<string>
#include<iostream>
#include<sstream>
#include<syslog.h>

#include"QueryResult.h"
#include"SysLogger.h"

#include"DataBaseStuff.h"
#include"DataBaseEnums.h"
#include"BTree.h"

const size_t etalonKeySize = 10;




class DataBase
{
public:
                 DataBase( const std::string &sDbFilePath );
                ~DataBase();
                 DataBase( const DataBase& ) = delete;
                 DataBase& operator=( const DataBase& ) = delete;
   QueryResult   ExecuteQuery( const std::string &s );

private:
       void   print_row(Row* row);
   eNodeType   get_node_type(void* node);
       void   set_node_type(void* node, eNodeType type);
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
   void   deserialize_row( void *key, void* source, Row* destination );
   void   initialize_leaf_node(void* node);
   void   initialize_internal_node(void* node);
   Cursor*  leaf_node_find(Table* table, uint32_t page_num, char* key);
   uint32_t   internal_node_find_child(void* node, uint32_t key);
   Cursor*  internal_node_find(Table* table, uint32_t page_num, uint32_t key);
   Cursor*  table_find(Table* table, char* key);
   Cursor*  table_start(Table* table);
   void*  cursor_value(Cursor* cursor);
   void   cursor_advance(Cursor* cursor);
   Pager*  pager_open(const char* filename);
   void  db_open();
   InputBuffer*  new_input_buffer();
   void   print_prompt();
   void   read_input(InputBuffer* input_buffer, const std::string &s_query);
   void   close_input_buffer(InputBuffer* input_buffer);
   void   pager_flush(Pager* pager, uint32_t page_num);
   void   db_close();
   eMetaCmdResult   do_meta_command(InputBuffer* input_buffer, Table* table);
   ePrepareResult   prepare_insert(InputBuffer* input_buffer, Statement* statement);
   ePrepareResult   prepare_statement(InputBuffer* input_buffer, Statement* statement);
   uint32_t   get_unused_page_num(Pager* pager);
   void   create_new_root(Table* table, uint32_t right_child_page_num);
   void   internal_node_insert(Table* table, uint32_t parent_page_num, uint32_t child_page_num);
   void   update_internal_node_key(void* node, uint32_t old_key, uint32_t new_key);
   void   leaf_node_split_and_insert(Cursor* cursor, uint32_t key, Row* value);
   void   leaf_node_insert(Cursor* cursor, char* key, Row* value);
   QueryResult   execute_insert(Statement* statement, Table* table);
   QueryResult   execute_list(Statement* statement, Table* table);
   QueryResult   execute_statement(Statement* statement, Table* table);
   int   compareKeys( char *key1, char* key2 ) const;
   char* leafNodeKey(void* node, uint32_t cell_num);
   void* leafNodeCell(void* node, uint32_t cell_num);
   void*  leafNodeValue(void* node, uint32_t cell_num);
   Cursor*   createCursorForFirstCell( Table* table ) const;
   void* cursor_key(Cursor* cursor);
   void leafNodeSplitAndInsert(Cursor* cursor, char *key, Row* value);
   std::string   getKey( Cursor* p_cursor );

   InputBuffer* input_buffer;
   Table* table;
   std::string sCurQuery;
   const std::string sDbFilePath;

   SysLogger sysLogger;
};
