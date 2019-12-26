/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    Class is responsible for store/load data from database
  * </pre>
 *
 * @class DataBase
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

class DataBase
{
public:
                 DataBase( const std::string &sDbFilePath );
                ~DataBase();
                 DataBase( const DataBase& ) = delete;
                 DataBase& operator=( const DataBase& ) = delete;
   QueryResult   executeQuery( const std::string &s );

private:
           eNodeType   getNodeType(void* node);
                void   setNodeType(void* node, eNodeType type);
                bool   isNodeRoot(void* node);
                void   setNodeRoot(void* node, bool is_root);
            uint32_t*  nodeParent(void* node);
            uint32_t*  internalNodeNumKeys(void* node);
            uint32_t*  internalNodeRightChild(void* node);
            uint32_t*  internalNodeCell(void* node, uint32_t cell_num);
            uint32_t*  internalNodeChild(void* node, uint32_t child_num);
            uint32_t*  internalNodeKey(void* node, uint32_t key_num);
            uint32_t*  leafNodeNumCells(void* node);
            uint32_t*  leafNodeNextLeaf(void* node);
                void*  leafNodeCellId(void* node, uint32_t cell_num);
            uint32_t*  leafNodeKey(void* node, uint32_t cell_num);
                void*  leafNodeValueId(void* node, uint32_t cell_num);
            uint32_t   getNodeMaxKey(void* node);
                void   printConstants();
                void*  getPage(Pager* pager, uint32_t page_num);
                void   indent(uint32_t level);
                void   printTree(Pager* pager, uint32_t page_num, uint32_t indentation_level);
                void   serializeRow(Row* source, void* destination);
                void   deserializeRow(void* source, Row* destination);
                void   initializeLeafNode(void* node);
                void   initializeInternalNode(void* node);
              Cursor*  leafNodeFind(Table* table, uint32_t page_num, uint32_t key);
            uint32_t   internalNodeFindChild(void* node, uint32_t key);
              Cursor*  internalNodeFind(Table* table, uint32_t page_num, uint32_t key);
              Cursor*  tableFind(Table* table, uint32_t key);
              Cursor*  tableStart(Table* table);
                void*  cursorValue(Cursor* cursor);
                void   cursorAdvance(Cursor* cursor);
               Pager*  pagerOpen(const char* filename);
                void   dbOpen();
         InputBuffer*  newInputBuffer();
                void   readInput(InputBuffer* input_buffer, const std::string &s_query);
                void   closeInputBuffer(InputBuffer* input_buffer);
                void   pagerFlush(Pager* pager, uint32_t page_num);
                void   dbClose();
      eMetaCmdResult   doMetaCommand(InputBuffer* input_buffer, Table* table);
      ePrepareResult   prepareInsert(InputBuffer* input_buffer, Statement* statement);
      ePrepareResult   prepareStatement(InputBuffer* input_buffer, Statement* statement);
            uint32_t   getUnusedPageNum(Pager* pager);
                void   createNewRoot(Table* table, uint32_t right_child_page_num);
                void   internalNodeInsert(Table* table, uint32_t parent_page_num, uint32_t child_page_num);
                void   updateInternalNodeKey(void* node, uint32_t old_key, uint32_t new_key);
                void   leafNodeSplitAndInsert(Cursor* cursor, uint32_t key, Row* value);
                void   leafNodeInsert(Cursor* cursor, uint32_t key, Row* value);
         QueryResult   executeInsert(Statement* statement, Table* table);
         QueryResult   executeList(Statement* statement, Table* table);
         QueryResult   executeStatement(Statement* statement, Table* table);
                 int   compareKeys( char *key1, char* key2 ) const;
                void*  leafNodeCell(void* node, uint32_t cell_num);
                void*  leafNodeValue(void* node, uint32_t cell_num);
              Cursor*  createCursorForFirstCell( Table* table ) const;
                void*  cursorKey(Cursor* cursor);
                void   leafNodeSplitAndInsert(Cursor* cursor, char *key, Row* value);
         std::string   getKey( Cursor* p_cursor );
  
         InputBuffer*  input_buffer;
               Table*  table;
         std::string   sCurQuery;
   const std::string   sDbFilePath;
           SysLogger   sysLogger;
};
