/*************************************************************************************************************
*** SQLiteWrapper
*** Clear C interface on top of SQLite 3
**
**************************************************************************************************************/
#ifndef __SQLITE_WRAPPER_H__
#define __SQLITE_WRAPPER_H__


/*******************************************
Includes
********************************************/
#ifndef WIN32
#include <fcntl.h>
#else
#include "windows.h"
#endif

#include "cmbs_api.h"
#include "cfr_mssg.h"
#include "sqlite3.h"

/*******************************************
Defines
********************************************/
#define LIST_ACCESS_DB_FILE "LA_DB"

#define SQL_MAX_COLUMNS             30
#define SQL_MAX_COLUMN_NAME_LEN     40

#define SQL_MAX_SIMULTANEOUS_DB_ACCESS  5

#define SQL_QUERY_MAX_LEN   1024

/*******************************************
Types
********************************************/
typedef struct
{
    CFR_CMBS_CRITICALSECTION    cs_mutex;
    sqlite3*                    pDB_DataBase[SQL_MAX_SIMULTANEOUS_DB_ACCESS];

} ST_SQL_DB, * PST_SQL_DB;

typedef struct
{
    char column_map[SQL_MAX_COLUMNS][SQL_MAX_COLUMN_NAME_LEN + 1];
    u32  u32_NumOfColumns;

} ST_COLUMN_MAP, * PST_COLUMN_MAP;

typedef enum
{
    SQL_RC_OK   = 0,
    SQL_RC_FAIL = 1

} SQL_RC;

/*******************************************
Globals
********************************************/
ST_SQL_DB g_Database;


/*******************************************
Interface API
********************************************/
/* Init DataBase */
SQL_RC SQL_InitDatabase(void);

/* Get Database Handle */
SQL_RC SQL_GetDatabaseHandle(OUT void** ppv_DBHandle);

/* Return Database Handle */
SQL_RC SQL_ReturnDatabaseHandle(IN void* pv_DBHandle);

/* Send SQL Query */
SQL_RC SQL_Execute(IN void* pv_DBHandle, IN const char* s_Query);

/* SQL Select from a table
**  
** After a call to Select function, the user should call to FetchRow one or more times (depends on how many rows exist 
** and how many the user wants to read), and finally  ReleaseSelectHandle 
**  
**  Input Args:
**  -----------------
**  pv_DBHandle - pointer to DB
**  s_Query - string SQL query
**  b_PrepareMap - should a map be prepared
**      if b_PrepareMap is non-zero, 
**      the function will return a mapping between column index and column name 
**      the map will be returned on pst_pMap - caller should allocate it! 
** 
** Output Args 
** ------------------- 
**  pst_pMap - pointer to map object 
**  ppv_SelectHandle - pointer to SelectHandle (to be used on subsequent GetRow calls - user is responsible to call to ReleaseSelectHandle !!!
**  */ 
SQL_RC SQL_Select(IN void* pv_DBHandle, IN const char* s_Query, IN bool b_PrepareMap, OUT PST_COLUMN_MAP pst_pMap, OUT void** ppv_SelectHandle);

/* Order the Database to Get The next row, corresponds to the last select query performed 
   Input Arguments
   --------------------
   pv_SelectHandle - Handle to the appropriate select query
 
   Output Argument
   --------------------
  b_NextRowAvailable - if non-zero, it means that there is another row, and the user may extract its data from the data base (using getstr, getuvalue,
  etc. - depends on the data type)
*/
SQL_RC SQL_FetchRow(IN void* pv_SelectHandle, OUT bool* pb_NextRowAvailable);

SQL_RC SQL_ReleaseSelectHandle(IN void* pv_SelectHandle);

/* Get the (string) value in the current row, at the given column index */
SQL_RC SQL_GetStrByIdx(IN void* pv_SelectHandle, IN u32 u32_index, OUT const unsigned char** pps_value);

/* Get the (string) value in the current row, at the given column name */
SQL_RC SQL_GetStrByColumnName(IN void* pv_SelectHandle, IN const char* ps_ColumnName, IN PST_COLUMN_MAP pst_pMap, OUT const unsigned char** pps_value);

/* Get the (unsigned integer) value in the current row, at the given column index */
SQL_RC SQL_GetUValByIdx(  IN void* pv_SelectHandle, IN u32 u32_index, OUT u32* pu32_value);
SQL_RC SQL_GetUVal64ByIdx(IN void* pv_SelectHandle, IN u32 u32_index, OUT u64* pu64_value);

/* Get the (unsigned integer) value in the current row, at the given column name */
SQL_RC SQL_GetUValByColumnName(  IN void* pv_SelectHandle, IN const char* ps_ColumnName, IN PST_COLUMN_MAP pst_pMap, OUT u32* pu32_value);
SQL_RC SQL_GetUVal64ByColumnName(IN void* pv_SelectHandle, IN const char* ps_ColumnName, IN PST_COLUMN_MAP pst_pMap, OUT u64* pu64_value);

/* Get the Id of last insert */
SQL_RC SQL_GetIdOfLastInsert(IN void* pv_DBHandle, OUT u32* pu32_Id);

/*******************************************
Auxiliary
********************************************/
/* searches for the given name in the given map, and return index. return -1 when not found */
s32 GetIdxByName(IN const char* ps_Name, IN PST_COLUMN_MAP pst_pMap);

#endif /* __SQLITE_WRAPPER_H__ */

/* End Of File *****************************************************************************************************************************/

