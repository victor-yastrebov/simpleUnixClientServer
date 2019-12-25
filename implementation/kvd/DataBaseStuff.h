/**
 *
 * @author <va.yastrebov>
 * @description
 * <pre>
 *    These file contains structures that are used by DataBase for working
  * </pre>
 *
 * @class DataBaseStuff
 */
#pragma once

#include<stdlib.h> // for size_t and ssize_t

#include"DataBaseEnums.h"

namespace dbs
{
   const int nTableMaxPages = 100;
   const int nColumnUserNameSize = 32;
   const int nColumnEmailSize = 255;
   const int nKeyMaxSize = 255;
   const int nValueMaxSize = 255;

   const size_t nEtalonKeySize = 10;
}

struct InputBuffer
{
   char* buffer;
   size_t buffer_length;
   ssize_t input_length;
};

struct Row
{
   uint32_t id;
   char username[dbs::nColumnUserNameSize + 1];
   char email[dbs::nColumnEmailSize + 1];

   char key[dbs::nKeyMaxSize];
   char value[dbs::nValueMaxSize];
};

struct Statement
{
   enum eStatementType eType;
   Row row_to_insert;  // only used by insert statement
};

struct Pager
{
   int file_descriptor;
   uint32_t file_length;
   uint32_t num_pages;
   void* pages[dbs::nTableMaxPages];
};

struct Table
{
   Pager* pager;
   uint32_t root_page_num;
};

struct Cursor
{
   Table* table;
   uint32_t page_num;
   uint32_t cell_num;
   bool end_of_table;  // Indicates a position one past the last element
};
