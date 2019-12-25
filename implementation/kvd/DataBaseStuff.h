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

#define TABLE_MAX_PAGES 100
#define COLUMN_USERNAME_SIZE 32
#define COLUMN_EMAIL_SIZE 255

#define KEY_MAX_SIZE 255
#define VALUE_MAX_SIZE 255

struct InputBuffer
{
   char* buffer;
   size_t buffer_length;
   ssize_t input_length;
};

struct Row
{
   uint32_t id;
   char username[COLUMN_USERNAME_SIZE + 1];
   char email[COLUMN_EMAIL_SIZE + 1];

   char key[KEY_MAX_SIZE];
   char value[VALUE_MAX_SIZE];
};

struct Statement
{
   enum class eStatementType eType;
   Row row_to_insert;  // only used by insert statement
};

struct Pager
{
   int file_descriptor;
   uint32_t file_length;
   uint32_t num_pages;
   void* pages[TABLE_MAX_PAGES];
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
