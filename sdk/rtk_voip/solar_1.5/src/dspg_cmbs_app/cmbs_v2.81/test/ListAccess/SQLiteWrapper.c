/*************************************************************************************************************
*** SQLiteWrapper
*** Clear C interface on top of SQLite 3
**
**************************************************************************************************************/

/*******************************************
Includes
********************************************/
#include <stdio.h>
#include "SQLiteWrapper.h"
#include "cmbs_platf.h"

#ifdef __linux
#include <pthread.h>
#endif

/*******************************************
Defines
********************************************/
#define MUTEX 		g_Database.cs_mutex
#define DATABASE(x)	g_Database.pDB_DataBase[x]


/*******************************************
Interface API
********************************************/
SQL_RC SQL_InitDatabase(void)
{
	u32 u32_index = 0;

	/* create mutex */
	CFR_CMBS_INIT_CRITICALSECTION(MUTEX);

	/* init database handles */
	for (u32_index = 0; u32_index < SQL_MAX_SIMULTANEOUS_DB_ACCESS; ++u32_index)
	{
		DATABASE(u32_index) = NULL;
	}

	return SQL_RC_OK;
}

SQL_RC SQL_GetDatabaseHandle(void** ppv_DBHandle)
{
    u32 u32_rc = 0;
	u32 u32_index = 0;


	/* Enter critical section */
	CFR_CMBS_ENTER_CRITICALSECTION(MUTEX);

	/* find free handle */
	for (u32_index = 0; u32_index < SQL_MAX_SIMULTANEOUS_DB_ACCESS; ++u32_index)
	{
		if (DATABASE(u32_index) == NULL)
		{
			/* found! */

			/* open Database */
			u32_rc = sqlite3_open(LIST_ACCESS_DB_FILE, &DATABASE(u32_index));
		
			if (u32_rc)
			{
				printf("Can't open database: %s\n", LIST_ACCESS_DB_FILE);
				sqlite3_close(DATABASE(u32_index));

				/* Leave critical section */
				CFR_CMBS_LEAVE_CRITICALSECTION(MUTEX);
				return SQL_RC_FAIL;
			}
			else
			{
				/* Leave critical section */
				*ppv_DBHandle = DATABASE(u32_index);
				CFR_CMBS_LEAVE_CRITICALSECTION(MUTEX);
				return SQL_RC_OK;
			}
		}
	}

	printf("No more free DB handles\n");

	/* Leave critical section */
	CFR_CMBS_LEAVE_CRITICALSECTION(MUTEX);
	return SQL_RC_FAIL;
}

SQL_RC SQL_ReturnDatabaseHandle(void* pv_DBHandle)
{
    u32 u32_rc = 0;
	u32 u32_index = 0;


	/* Enter critical section */
	CFR_CMBS_ENTER_CRITICALSECTION(MUTEX);

	/* find the handle */
	for (u32_index = 0; u32_index < SQL_MAX_SIMULTANEOUS_DB_ACCESS; ++u32_index)
	{
		if (DATABASE(u32_index) == pv_DBHandle)
		{
			/* found! */

			/* close Database */
			u32_rc = sqlite3_close(pv_DBHandle);
		
			if (u32_rc)
			{
				printf("Can't close database!\n");

				/* Leave critical section */
				CFR_CMBS_LEAVE_CRITICALSECTION(MUTEX);
				return SQL_RC_FAIL;
			}
			else
			{
				/* Leave critical section */
				DATABASE(u32_index) = NULL;
				CFR_CMBS_LEAVE_CRITICALSECTION(MUTEX);
				return SQL_RC_OK;
			}
		}
	}

	printf("Unknown DB Handle %p\n", pv_DBHandle);

	/* Leave critical section */
	CFR_CMBS_LEAVE_CRITICALSECTION(MUTEX);
	return SQL_RC_FAIL;
}


SQL_RC SQL_Execute(void* pv_DBHandle, const char* s_Query)
{
	u32 u32_rc = 0;
	sqlite3_stmt* pResult_res = NULL;

	/* prepare query */
	u32_rc = sqlite3_prepare(pv_DBHandle, s_Query, (s32)strlen(s_Query) + 1, &pResult_res, NULL);
	if (u32_rc != SQLITE_OK)
	{
		printf("execute: prepare query failed\n");
		return SQL_RC_FAIL;
	}
	if (!pResult_res)
	{
		printf("execute: query failed\n");
		return SQL_RC_FAIL;
	}

	/* execute */
	u32_rc = sqlite3_step(pResult_res);

	/* deallocate statement */
	sqlite3_finalize(pResult_res);
	pResult_res = NULL;


	/* check result */
	switch (u32_rc)
	{
	case SQLITE_BUSY:
		printf("execute: database busy\n");
		return SQL_RC_FAIL;

	case SQLITE_DONE:
	case SQLITE_ROW:
		return SQL_RC_OK;

	case SQLITE_ERROR:
		printf("Error %s\n", sqlite3_errmsg(pv_DBHandle));
		return SQL_RC_FAIL;

	case SQLITE_MISUSE:
		printf("execute: database misuse\n");
		return SQL_RC_FAIL;

	default:
		printf("execute: unknown result code\n");
	}

	return SQL_RC_FAIL;
}

SQL_RC SQL_Select(void* pv_DBHandle, const char* s_Query, bool b_PrepareMap, PST_COLUMN_MAP pst_pMap, void** ppv_SelectHandle)
{
	u32 u32_rc = 0;
	sqlite3_stmt* pResult_res = NULL;

	/* prepare query */
	u32_rc = sqlite3_prepare(pv_DBHandle, s_Query, (s32)strlen(s_Query) + 1, &pResult_res, NULL);
	if (u32_rc != SQLITE_OK)
	{
		printf("get_result: prepare query failed\n");
		return SQL_RC_FAIL;
	}
	if (pResult_res == NULL)
	{
		printf("get_result: query failed\n");
		return SQL_RC_FAIL;
	}

	*ppv_SelectHandle = pResult_res;

	/* prepare column map (if requested) */
	if (b_PrepareMap)
	{
		u32 u32_index = 0;
		const char* ps_ColumnName = NULL;

		do
		{
			ps_ColumnName = sqlite3_column_name(pResult_res, u32_index);
			if (ps_ColumnName == NULL)
				break;

			strcpy(pst_pMap->column_map[u32_index++], ps_ColumnName);
		} while (TRUE);
		pst_pMap->u32_NumOfColumns = u32_index;
	}

	return SQL_RC_OK;
}

SQL_RC SQL_FetchRow(void* pv_SelectHandle, bool* pb_NextRowAvailable)
{
	u32 u32_rc = 0;

	*pb_NextRowAvailable = FALSE;

	/* execute the query */
	u32_rc = sqlite3_step(pv_SelectHandle);

	switch (u32_rc)
	{
	case SQLITE_ROW:
	*pb_NextRowAvailable = TRUE;
	return SQL_RC_OK;

	case SQLITE_BUSY:
		printf("execute: database busy\n");
		return SQL_RC_OK;

	case SQLITE_DONE:
		return SQL_RC_OK;

	case SQLITE_ERROR:
		printf("General Error\n");
		return SQL_RC_FAIL;

	case SQLITE_MISUSE:
		printf("execute: database misuse\n");
		return SQL_RC_FAIL;

	default:
		printf("execute: unknown result code\n");
		return SQL_RC_FAIL;
	}
}


SQL_RC SQL_ReleaseSelectHandle(void* pv_SelectHandle)
{
	u32 u32_rc = sqlite3_finalize(pv_SelectHandle);
	if (u32_rc)
	{
		printf("Failed releasing select handle\n");
		return SQL_RC_FAIL;
	}
	else
	{
		return SQL_RC_OK;
	}
}

s32 GetIdxByName(const char* ps_Name, PST_COLUMN_MAP pst_pMap)
{
	u32 u32_index;

	for (u32_index = 0; u32_index < pst_pMap->u32_NumOfColumns; ++u32_index)
	{
		if (strcmp(ps_Name, pst_pMap->column_map[u32_index]) == 0)
		{
			/* found */
			return u32_index;
		}
	}

	return -1;
}


SQL_RC SQL_GetStrByIdx(void* pv_SelectHandle, u32 u32_index, const unsigned char** pps_value)
{
	*pps_value = sqlite3_column_text(pv_SelectHandle, u32_index);

	if (*pps_value == NULL)
	{
		*pps_value = (const unsigned char*)"";
	}

	return SQL_RC_OK;
}

SQL_RC SQL_GetStrByColumnName(void* pv_SelectHandle, const char* ps_ColumnName, PST_COLUMN_MAP pst_pMap, const unsigned char** pps_value)
{
	s32 s32_index = GetIdxByName(ps_ColumnName, pst_pMap);
	if (s32_index >= 0)
	{
		return SQL_GetStrByIdx(pv_SelectHandle, s32_index, pps_value);
	}

	printf("Column name lookup failure: %s", ps_ColumnName);
	return SQL_RC_FAIL;
}

SQL_RC SQL_GetUValByIdx(void* pv_SelectHandle, u32 u32_index, u32* pu32_value)
{
	*pu32_value = sqlite3_column_int(pv_SelectHandle, u32_index);
	return SQL_RC_OK;
}

SQL_RC SQL_GetUVal64ByIdx(void* pv_SelectHandle, u32 u32_index, u64* pu64_value)
{
	*pu64_value = sqlite3_column_int64(pv_SelectHandle, u32_index);
	return SQL_RC_OK;
}

SQL_RC SQL_GetUValByColumnName(void* pv_SelectHandle, const char* ps_ColumnName, PST_COLUMN_MAP pst_pMap, u32* pu32_value)
{
	s32 s32_index = GetIdxByName(ps_ColumnName, pst_pMap);
	if (s32_index >= 0)
	{
		return SQL_GetUValByIdx(pv_SelectHandle, s32_index, pu32_value);
	}

	printf("Column name lookup failure: %s", ps_ColumnName);
	return SQL_RC_FAIL;
}

SQL_RC SQL_GetUVal64ByColumnName(IN void* pv_SelectHandle, IN const char* ps_ColumnName, IN PST_COLUMN_MAP pst_pMap, OUT u64* pu64_value)
{
	s32 s32_index = GetIdxByName(ps_ColumnName, pst_pMap);
	if (s32_index >= 0)
	{
		return SQL_GetUVal64ByIdx(pv_SelectHandle, s32_index, pu64_value);
	}

	printf("Column name lookup failure: %s", ps_ColumnName);
	return SQL_RC_FAIL;
}


SQL_RC SQL_GetIdOfLastInsert(void* pv_DBHandle, u32* pu32_Id)
{
	*pu32_Id = (u32)sqlite3_last_insert_rowid(pv_DBHandle);
	return SQL_RC_OK;
}




/* End Of File *****************************************************************************************************************************/



