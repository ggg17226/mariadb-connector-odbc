/************************************************************************************
   Copyright (C) 2013 SkySQL AB
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
   
   You should have received a copy of the GNU Library General Public
   License along with this library; if not see <http://www.gnu.org/licenses>
   or write to the Free Software Foundation, Inc., 
   51 Franklin St., Fifth Floor, Boston, MA 02110, USA
*************************************************************************************/
#include <ma_odbc.h>

struct st_madb_isolation MADB_IsolationLevel[] =
{
  {SQL_TRANSACTION_REPEATABLE_READ, "REPEATABLE READ"},
  {SQL_TRANSACTION_READ_COMMITTED, "READ COMMITTED"},
  {SQL_TRANSACTION_READ_UNCOMMITTED, "READ UNCOMMITTED"},
  {SQL_TRANSACTION_SERIALIZABLE, "SERIALIZABLE"},
  {0, 0}
};

/* used by SQLGetFunctions */
SQLUSMALLINT MADB_supported_api[]=
{
  SQL_API_SQLALLOCCONNECT,
  SQL_API_SQLALLOCENV,
  SQL_API_SQLALLOCHANDLE,
  SQL_API_SQLALLOCSTMT,
  SQL_API_SQLBINDCOL,
  SQL_API_SQLBINDPARAM,
  SQL_API_SQLCANCEL,
  SQL_API_SQLCLOSECURSOR,
  SQL_API_SQLCOLATTRIBUTE,
  SQL_API_SQLCOLUMNS,
  SQL_API_SQLCONNECT,
  SQL_API_SQLCOPYDESC,
  SQL_API_SQLDATASOURCES,
  SQL_API_SQLDESCRIBECOL,
  SQL_API_SQLDISCONNECT,
  SQL_API_SQLENDTRAN,
  SQL_API_SQLERROR,
  SQL_API_SQLEXECDIRECT,
  SQL_API_SQLEXECUTE,
  SQL_API_SQLFETCH,
  SQL_API_SQLFETCHSCROLL,
  SQL_API_SQLFREECONNECT,
  SQL_API_SQLFREEENV,
  SQL_API_SQLFREEHANDLE,
  SQL_API_SQLFREESTMT,
  SQL_API_SQLGETCONNECTATTR,
  SQL_API_SQLGETCONNECTOPTION,
  SQL_API_SQLGETCURSORNAME,
  SQL_API_SQLGETDATA,
  SQL_API_SQLGETDESCFIELD,
  SQL_API_SQLGETDESCREC,
  SQL_API_SQLGETDIAGFIELD,
  SQL_API_SQLGETDIAGREC,
  SQL_API_SQLGETENVATTR,
  SQL_API_SQLGETFUNCTIONS,
  SQL_API_SQLGETINFO,
  SQL_API_SQLGETSTMTATTR,
  SQL_API_SQLGETSTMTOPTION,
  SQL_API_SQLGETTYPEINFO,
  SQL_API_SQLNUMRESULTCOLS,
  SQL_API_SQLPARAMDATA,
  SQL_API_SQLPREPARE,
  SQL_API_SQLPUTDATA,
  SQL_API_SQLROWCOUNT,
  SQL_API_SQLSETCONNECTATTR,
  SQL_API_SQLSETCONNECTOPTION,
  SQL_API_SQLSETCURSORNAME,
  SQL_API_SQLSETDESCFIELD,
  SQL_API_SQLSETDESCREC,
  SQL_API_SQLSETENVATTR,
  SQL_API_SQLSETPARAM,
  SQL_API_SQLSETSTMTATTR,
  SQL_API_SQLSETSTMTOPTION,
  SQL_API_SQLSPECIALCOLUMNS,
  SQL_API_SQLSTATISTICS,
  SQL_API_SQLTABLES,
  SQL_API_SQLTRANSACT,
  SQL_API_SQLBULKOPERATIONS,
  SQL_API_SQLBINDPARAMETER,
  SQL_API_SQLBROWSECONNECT,
  SQL_API_SQLCOLATTRIBUTES,
  SQL_API_SQLCOLUMNPRIVILEGES ,
  SQL_API_SQLDESCRIBEPARAM,
  SQL_API_SQLDRIVERCONNECT,
  SQL_API_SQLDRIVERS,
  SQL_API_SQLEXTENDEDFETCH,
  SQL_API_SQLFOREIGNKEYS,
  SQL_API_SQLMORERESULTS,
  SQL_API_SQLNATIVESQL,
  SQL_API_SQLNUMPARAMS,
  SQL_API_SQLPARAMOPTIONS,
  SQL_API_SQLPRIMARYKEYS,
  SQL_API_SQLPROCEDURECOLUMNS,
  SQL_API_SQLPROCEDURES,
  SQL_API_SQLSETPOS,
  SQL_API_SQLSETSCROLLOPTIONS,
  SQL_API_SQLTABLES,
  SQL_API_SQLTABLEPRIVILEGES

};


struct st_ma_connection_methods MADB_Dbc_Methods; /* declared at the end of file */

my_bool CheckConnection(MADB_Dbc *Dbc)
{
  if (!Dbc->mariadb)
    return FALSE;
  if (!Dbc->mariadb->net.vio)
  {
    /* Check if reconnect option was set */
    if (DSN_OPTION(Dbc, MADB_OPT_FLAG_AUTO_RECONNECT))
    {
      if (!mysql_ping(Dbc->mariadb))
        return TRUE;
    }
    return FALSE;
  }
  return TRUE;
}

/* {{{ MADB_DbcSetAttr */
SQLRETURN MADB_DbcSetAttr(MADB_Dbc *Dbc, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER StringLength, my_bool isWChar)
{
  MADB_CLEAR_ERROR(&Dbc->Error);

  if (!Dbc)
  {
    /* Todo: check */
    if (Attribute != SQL_ATTR_TRACE ||
        Attribute != SQL_ATTR_TRACEFILE)
      return SQL_INVALID_HANDLE;
    return SQL_SUCCESS;
  } 

  switch(Attribute) {
  case SQL_ATTR_ACCESS_MODE:
    if ((SQLPOINTER)SQL_MODE_READ_WRITE != ValuePtr)
      MADB_SetError(&Dbc->Error, MADB_ERR_01S02, NULL, 0);
    Dbc->AccessMode= SQL_MODE_READ_WRITE;
    break;
#if (ODBCVER >= 0x0351)
  case SQL_ATTR_ANSI_APP:
    Dbc->IsAnsi= (my_bool)ValuePtr;
    break;
#endif
  case SQL_ATTR_ASYNC_ENABLE:
     if ((SQLPOINTER)SQL_ASYNC_ENABLE_OFF != ValuePtr)
      MADB_SetError(&Dbc->Error, MADB_ERR_01S02, NULL, 0);
     Dbc->AsyncEnable= SQL_ASYNC_ENABLE_OFF;
    break;
  case SQL_ATTR_AUTO_IPD:
    /* read only */
    MADB_SetError(&Dbc->Error, MADB_ERR_HY092, NULL, 0);
    break;
  case SQL_ATTR_AUTOCOMMIT:
    {
      SQLINTEGER ValidAttrs[]= {2, SQL_AUTOCOMMIT_ON, SQL_AUTOCOMMIT_OFF};
      MADB_CHECK_ATTRIBUTE(Dbc, (SQLINTEGER)ValuePtr, ValidAttrs);
      /* if a connection is open, try to apply setting to the connection */
      if (Dbc->mariadb)
      {
        if (Dbc->EnlistInDtc) {
          MADB_SetError(&Dbc->Error, MADB_ERR_25000, NULL, 0);
          return SQL_ERROR;
        }
        if (mysql_autocommit(Dbc->mariadb, (my_bool)ValuePtr))
        {
          MADB_SetError(&Dbc->Error, MADB_ERR_HY001, mysql_error(Dbc->mariadb), mysql_errno(Dbc->mariadb));
          return SQL_ERROR;
        }
      }
      Dbc->AutoCommit= (SQLUINTEGER)ValuePtr;
    }
    break;
  case SQL_ATTR_CONNECTION_DEAD:
    /* read only! */
    MADB_SetError(&Dbc->Error, MADB_ERR_HY092, NULL, 0);
    return Dbc->Error.ReturnValue;
    break;
  case SQL_ATTR_CURRENT_CATALOG:
    {
      SQLINTEGER Length;
      MADB_FREE(Dbc->CatalogName);
      if (isWChar)
        Dbc->CatalogName= MADB_ConvertFromWChar((SQLWCHAR *)ValuePtr, StringLength, &Length, Dbc->CodePage, NULL);
      else
        Dbc->CatalogName= my_strdup((char *)ValuePtr, MYF(0));

      if (Dbc->mariadb &&
          mysql_select_db(Dbc->mariadb, Dbc->CatalogName))
      {
        MADB_SetError(&Dbc->Error, MADB_ERR_HY001, mysql_error(Dbc->mariadb), mysql_errno(Dbc->mariadb));
        return Dbc->Error.ReturnValue;
      }
    }
    break;
  case SQL_ATTR_LOGIN_TIMEOUT:
    Dbc->LoginTimeout= (SQLUINTEGER)ValuePtr;
    break;
  case SQL_ATTR_METADATA_ID:
    Dbc->MetadataId= (SQLUINTEGER)ValuePtr;
    break;
  case SQL_ATTR_ODBC_CURSORS:
    {
      SQLINTEGER ValidAttrs[]= {3, SQL_CUR_USE_IF_NEEDED, SQL_CUR_USE_ODBC, SQL_CUR_USE_DRIVER};
      MADB_CHECK_ATTRIBUTE(Dbc, (SQLULEN)ValuePtr, ValidAttrs);
      if ((SQLULEN)ValuePtr != SQL_CUR_USE_ODBC)
        MADB_SetError(&Dbc->Error, MADB_ERR_01S02, NULL, 0);
      Dbc->OdbcCursors= SQL_CUR_USE_ODBC;
    }
    break;
  case SQL_ATTR_ENLIST_IN_DTC:
    /* MS Distributed Transaction Coordinator not supported */
    MADB_SetError(&Dbc->Error, MADB_ERR_HYC00, NULL, 0);
    return Dbc->Error.ReturnValue;
    break;
  case SQL_ATTR_PACKET_SIZE:
    /* if connection was made, return HY001 */
    if (Dbc->mariadb)
    {
      MADB_SetError(&Dbc->Error, MADB_ERR_HY001, NULL, 0);
      return Dbc->Error.ReturnValue;
    }
    Dbc->PacketSize= (SQLUINTEGER)ValuePtr;
    break;
  case SQL_ATTR_QUIET_MODE:
    Dbc->QuietMode= (HWND)ValuePtr;
    break;
  case SQL_ATTR_TRACE:
    break;
  case SQL_ATTR_TRACEFILE:
    break;
  case SQL_ATTR_TRANSLATE_LIB:
    break;
  case SQL_ATTR_TRANSLATE_OPTION:
    break;
  case SQL_ATTR_TXN_ISOLATION:
    if (Dbc->mariadb)
    {
      my_bool ValidTx= FALSE;
      unsigned int i;
      for (i=0; i < 4; i++)
      {
        if (MADB_IsolationLevel[i].SqlIsolation == (SQLLEN)ValuePtr)
        {
          char StmtStr[128];
          my_snprintf(StmtStr, 128, "SET SESSION TRANSACTION ISOLATION LEVEL %s",
                      MADB_IsolationLevel[i].StrIsolation);
          if (mysql_query(Dbc->mariadb, StmtStr))
          {
            MADB_SetError(&Dbc->Error, MADB_ERR_HY001, mysql_error(Dbc->mariadb), mysql_errno(Dbc->mariadb));
            return Dbc->Error.ReturnValue;
          }
          ValidTx= TRUE;
          break;
        }
      }
      if (!ValidTx)
      {
        MADB_SetError(&Dbc->Error, MADB_ERR_HY024, NULL, 0);
        return Dbc->Error.ReturnValue;
      }
    }
    Dbc->TxnIsolation= (SQLLEN)ValuePtr;
    break;
  default:
    break;
  }
  return Dbc->Error.ReturnValue;
}
/* }}} */

/* {{{ MADB_DbcHetAttr */
SQLRETURN MADB_DbcGetAttr(MADB_Dbc *Dbc, SQLINTEGER Attribute, SQLPOINTER ValuePtr, SQLINTEGER BufferLength, SQLINTEGER *StringLengthPtr, my_bool isWChar)
{
  MADB_CLEAR_ERROR(&Dbc->Error);

  if (!Dbc)
    return SQL_INVALID_HANDLE;

  if (!ValuePtr && Attribute != SQL_ATTR_CURRENT_CATALOG)
    return SQL_SUCCESS;
  if (Attribute == SQL_ATTR_CURRENT_CATALOG && !StringLengthPtr && 
      (!ValuePtr || !BufferLength))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_01004, NULL, 0);
    return Dbc->Error.ReturnValue;
  }

  switch(Attribute) {
  case SQL_ATTR_ACCESS_MODE:
    *(SQLUINTEGER *)ValuePtr= SQL_MODE_READ_WRITE;
    break;
  case SQL_ATTR_ASYNC_ENABLE:
    *(SQLULEN *)ValuePtr= SQL_ASYNC_ENABLE_OFF;
    break;
  case SQL_ATTR_AUTO_IPD:
    *(SQLUINTEGER *)ValuePtr= SQL_FALSE;
    break;
  case SQL_ATTR_AUTOCOMMIT:
    *(SQLUINTEGER *)ValuePtr= Dbc->AutoCommit;
    break;
  case SQL_ATTR_CONNECTION_DEAD:
    /* ping may fail if status isn't ready, so we need to check errors */
    if (mysql_ping(Dbc->mariadb))
      *(SQLUINTEGER *)ValuePtr= (mysql_errno(Dbc->mariadb) == CR_SERVER_GONE_ERROR ||
                                 mysql_errno(Dbc->mariadb) == CR_SERVER_LOST) ? SQL_CD_TRUE : SQL_CD_FALSE;
    else
      *(SQLUINTEGER *)ValuePtr= SQL_CD_FALSE;
    break;
  case SQL_ATTR_CURRENT_CATALOG:
  {
    SQLSMALLINT StrLen;
    SQLRETURN ret;
    ret= MADB_Dbc_GetCurrentDB(Dbc, ValuePtr, BufferLength, &StrLen, isWChar);
    /* if we weren't able to determine the current db, we will return the cached catalog name */
    if (!SQL_SUCCEEDED(ret) && Dbc->CatalogName)
    {
      MADB_CLEAR_ERROR(&Dbc->Error);
      StrLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, ValuePtr, BufferLength, 
                                          Dbc->CatalogName, strlen(Dbc->CatalogName), &Dbc->Error);
      ret= SQL_SUCCESS;
    }
    if (StringLengthPtr)
      *StringLengthPtr= (SQLINTEGER)StrLen;
    return ret;
  }
  case SQL_ATTR_LOGIN_TIMEOUT:
    *(SQLUINTEGER *)ValuePtr= Dbc->LoginTimeout;
    break;
  case SQL_ATTR_CONNECTION_TIMEOUT:
    *(SQLUINTEGER *)ValuePtr= 0;
    break;
  case SQL_ATTR_METADATA_ID:
    *(SQLINTEGER *)ValuePtr= Dbc->MetadataId;
  case SQL_ATTR_ODBC_CURSORS:
    *(SQLINTEGER *)ValuePtr= SQL_CUR_USE_ODBC;
    break;
  case SQL_ATTR_ENLIST_IN_DTC:
    /* MS Distributed Transaction Coordinator not supported */
    MADB_SetError(&Dbc->Error, MADB_ERR_HYC00, NULL, 0);
    break;
  case SQL_ATTR_PACKET_SIZE:
    *(SQLINTEGER *)ValuePtr= Dbc->PacketSize;
    break;
  case SQL_ATTR_QUIET_MODE:
    Dbc->QuietMode= (HWND)ValuePtr;
    break;
  case SQL_ATTR_TRACE:
    break;
  case SQL_ATTR_TRACEFILE:
    break;
  case SQL_ATTR_TRANSLATE_LIB:
    break;
  case SQL_ATTR_TRANSLATE_OPTION:
    break;
  case SQL_ATTR_TXN_ISOLATION:
    /* TxnIsolation wasn't set before we retrieve it from open connection or
       assume a default of REPETABLE_READ */
    if (!Dbc->TxnIsolation)
    {
      *(SQLULEN *)ValuePtr= SQL_TRANSACTION_REPEATABLE_READ;
      if (Dbc->mariadb)
      {
        MYSQL_RES *result;
        MYSQL_ROW row;
        const char *StmtString= "SELECT VARIABLE_VALUE FROM INFORMATION_SCHEMA.SESSION_VARIABLES WHERE VARIABLE_NAME='TX_ISOLATION'";

        if (mysql_query(Dbc->mariadb, StmtString))
        {
          MADB_SetNativeError(&Dbc->Error, SQL_HANDLE_DBC, Dbc->mariadb);
          return Dbc->Error.ReturnValue;
        }
        result= mysql_store_result(Dbc->mariadb);
        if ((row = mysql_fetch_row(result)))
        {
          unsigned int i;
          for (i=0; i < 4; i++)
            if (!strcmp(row[0], MADB_IsolationLevel[i].StrIsolation))
            {
              *(SQLULEN *)ValuePtr= MADB_IsolationLevel[i].SqlIsolation;
              break;
            }
        }
        mysql_free_result(result);
      }
    }
    else 
      *(SQLULEN *)ValuePtr= Dbc->TxnIsolation;
    break;

  default:
    MADB_SetError(&Dbc->Error, MADB_ERR_HYC00, NULL, 0);
    break;
  }
  return Dbc->Error.ReturnValue;
}
/* }}} */


/* {{{ MADB_DbcInit() */
MADB_Dbc *MADB_DbcInit(SQLHANDLE OutputHandle)
{
  MADB_Dbc *Connection= NULL;
  MADB_Env *Env= (MADB_Env *)OutputHandle;

  MADB_CLEAR_ERROR(&Env->Error);

  if (!(Connection = (MADB_Dbc *)MADB_CALLOC(sizeof(MADB_Dbc))))
    goto cleanup;

  Connection->AutoCommit= 4;
  Connection->Environment= Env;
  Connection->Methods= &MADB_Dbc_Methods;
  InitializeCriticalSection(&Connection->cs);
  EnterCriticalSection(&Connection->cs);

  /* Save connection in Environment list */
  Connection->ListItem.data= (void *)Connection;
  Connection->Environment->Dbcs= list_add(Connection->Environment->Dbcs, &Connection->ListItem);
  Connection->CodePage= GetACP();
  LeaveCriticalSection(&Connection->cs);

  return Connection;      
cleanup:
  if (Connection)
    free(Connection);
  else
    MADB_SetError(&Env->Error, MADB_ERR_HY001, NULL, 0);

  return NULL;
}
/* }}} */

/* {{{ MADB_DbcFree() */
SQLRETURN MADB_DbcFree(MADB_Dbc *Connection)
{
  if (!Connection)
    return SQL_ERROR;

  if (Connection->mariadb)
    mysql_close(Connection->mariadb);

  /* todo: delete all descriptors */

  Connection->Environment->Dbcs= list_delete(Connection->Environment->Dbcs, &Connection->ListItem);

  MADB_FREE(Connection->CatalogName);
  MADB_FREE(Connection->CharacterSet);
  MADB_FREE(Connection->DataBase);
  MADB_DSN_Free(Connection->Dsn);
  DeleteCriticalSection(&Connection->cs);

  free(Connection);
  return SQL_SUCCESS;
}
/* }}} */

/* {{{ MADB_Dbc_GetCurrentDB */
SQLRETURN MADB_Dbc_GetCurrentDB(MADB_Dbc *Connection, SQLPOINTER CurrentDB, SQLINTEGER CurrentDBLength, 
                                SQLSMALLINT *StringLengthPtr, my_bool isWChar) 
{
  SQLHANDLE Stmt;
  SQLRETURN ret;
  SQLLEN Size;
  char Buffer[65 * sizeof(WCHAR)];

  MADB_CLEAR_ERROR(&Connection->Error);
  ret= SQLAllocHandle(SQL_HANDLE_STMT, (SQLHANDLE) Connection, &Stmt);
  if (!SQL_SUCCEEDED(ret))
    return ret;
  if (!SQL_SUCCEEDED(SQLExecDirect(Stmt, (SQLCHAR *)"SELECT IF(DATABASE() IS NOT NULL,DATABASE(),'null')", SQL_NTS)) ||
      !SQL_SUCCEEDED(SQLFetch(Stmt)))
  {
    MADB_CopyError(&Connection->Error, &((MADB_Stmt *)Stmt)->Error);
    goto end;
  }
  
  ret= SQLGetData(Stmt, 1, SQL_CHAR, Buffer, 65, &Size);
 /* Size= (SQLINTEGER)MADB_SetString(isWChar ? Connection->CodePage : 0, CurrentDB, 
                      (isWChar) ? (int)(MIN(Size + sizeof(SQLWCHAR), CurrentDBLength) / sizeof(SQLWCHAR)) : 
                      (int)(MIN(Size + 1, CurrentDBLength)),
                       Buffer, SQL_NTS, &Connection->Error); */
  Size= (SQLSMALLINT)MADB_SetString(isWChar ? Connection->CodePage : 0, 
                                     (void *)CurrentDB, BUFFER_CHAR_LEN(CurrentDBLength, isWChar), Buffer,
                                     SQL_NTS, &Connection->Error);
  if (StringLengthPtr)
    *StringLengthPtr= isWChar ? (SQLSMALLINT)Size * sizeof(SQLWCHAR) : (SQLSMALLINT)Size;
  
end:
  SQLFreeStmt(Stmt, SQL_CLOSE);
  return Connection->Error.ReturnValue;
}
/* }}} */

/* {{{ MADB_DbcEndTran */
SQLRETURN MADB_DbcEndTran(MADB_Dbc *Dbc, SQLSMALLINT CompletionType)
{
  MADB_CLEAR_ERROR(&Dbc->Error);
  if (!Dbc)
    return SQL_INVALID_HANDLE;

  switch (CompletionType) {
  case SQL_ROLLBACK:
    if (Dbc->mariadb && mysql_rollback(Dbc->mariadb))
      MADB_SetNativeError(&Dbc->Error, SQL_HANDLE_DBC, Dbc->mariadb);
    break;
  case SQL_COMMIT:
    if (Dbc->mariadb && mysql_commit(Dbc->mariadb))
      MADB_SetNativeError(&Dbc->Error, SQL_HANDLE_DBC, Dbc->mariadb);
    break;
  default:
    MADB_SetError(&Dbc->Error, MADB_ERR_HY012, NULL, 0);
  }
  return Dbc->Error.ReturnValue;
}
/* }}} */

/* {{{ MADB_Dbc_ConnectDB
       Mind that this function is used for establishing connection from the setup lib
*/
SQLRETURN MADB_DbcConnectDB(MADB_Dbc *Connection,
    MADB_Dsn *Dsn)
{
  char StmtStr[128];
  unsigned ReportDataTruncation= 1;
  unsigned int i;
  unsigned long client_flags= 0L;
  my_bool my_reconnect= 1;
  
  if (!Connection || !Dsn)
    return SQL_ERROR;

  MADB_CLEAR_ERROR(&Connection->Error);

  if (!(Connection->mariadb= mysql_init(NULL)))
  {
    MADB_SetError(&Connection->Error, MADB_ERR_HY001, NULL, 0);
    goto end;
  }

  /* If a client character set was specified in DSN, we will always use it.
     Otherwise for ANSI applications we will use the current character set,
     for unicode connections we use utf8
  */
  if (Dsn->CharacterSet && Dsn->CharacterSet[0])
  {
    Connection->CharacterSet= my_strdup(Dsn->CharacterSet, MYF(0));
  }
  else if (Connection->IsAnsi)
  {
    char *os_charset= madb_get_os_character_set();
    Connection->CharacterSet= my_strdup(os_charset ? os_charset : "utf8", MYF(0));
  }
  else
    Connection->CharacterSet= my_strdup("utf8", MYF(0));

  Connection->CodePage= madb_get_windows_cp(Connection->CharacterSet);
  
  /* todo: error handling */
  mysql_options(Connection->mariadb, MYSQL_SET_CHARSET_NAME, Connection->CharacterSet);

  if (Dsn->InitCommand && Dsn->InitCommand[0])
    mysql_options(Connection->mariadb, MYSQL_INIT_COMMAND, Dsn->InitCommand);
 
  if (Dsn->ConnectionTimeout)
    mysql_options(Connection->mariadb, MYSQL_OPT_CONNECT_TIMEOUT, (const char *)&Dsn->ConnectionTimeout);

  Connection->Options= Dsn->Options;
  /* TODO: set DSN FLags (Options):
           Some of them (like reconnect) can be set via mysql_options, some of them are connection
           attributes
  */
  if (DSN_OPTION(Connection, MADB_OPT_FLAG_AUTO_RECONNECT))
    mysql_options(Connection->mariadb, MYSQL_OPT_RECONNECT, &my_reconnect);

  if(DSN_OPTION(Connection, MADB_OPT_FLAG_NAMED_PIPE))
    mysql_options(Connection->mariadb, MYSQL_OPT_NAMED_PIPE, (void *)Dsn->ServerName);

  if (DSN_OPTION(Connection, MADB_OPT_FLAG_NO_SCHEMA))
    client_flags|= CLIENT_NO_SCHEMA;
  if (DSN_OPTION(Connection, MADB_OPT_FLAG_PAD_SPACE))
    client_flags|= CLIENT_IGNORE_SPACE;

  if (DSN_OPTION(Connection, MADB_OPT_FLAG_FOUND_ROWS))
    client_flags|= CLIENT_FOUND_ROWS;
  if (DSN_OPTION(Connection, MADB_OPT_FLAG_COMPRESSED_PROTO))
    client_flags|= CLIENT_COMPRESS;
  if (DSN_OPTION(Connection, MADB_OPT_FLAG_MULTI_STATEMENTS))
    client_flags|= CLIENT_MULTI_STATEMENTS;

  /* enable truncation reporting */
   mysql_options(Connection->mariadb, MYSQL_REPORT_DATA_TRUNCATION, &ReportDataTruncation);
  
  if (!mysql_real_connect(Connection->mariadb,
        Dsn->ServerName, Dsn->UserName, Dsn->Password,
        Dsn->Catalog && Dsn->Catalog[0] ? Dsn->Catalog : NULL, Dsn->Port, NULL, client_flags))
  {
    goto err;
  }
  
  if (Connection->PacketSize)
  {
    my_snprintf(StmtStr, 128, "SET max_allowed_packet_size=%ld", Connection->PacketSize);
    if (mysql_query(Connection->mariadb, StmtStr))
      goto err;
  }

  if (!Connection->CatalogName && Dsn->Catalog)
    Connection->CatalogName= my_strdup(Dsn->Catalog, MYF(0));

  /* set default catalog */
  if (Connection->CatalogName && Connection->CatalogName[0])
  {
    if (mysql_select_db(Connection->mariadb, Connection->CatalogName))
      goto err;
  }

  /* Turn sql_auto_is_null behavior off.
     For more details see: http://bugs.mysql.com/bug.php?id=47005 */
  if (mysql_query(Connection->mariadb, "SET SESSION SQL_AUTO_IS_NULL=0"))
    goto err;

  /* set autocommit behavior */
  if (mysql_autocommit(Connection->mariadb, (my_bool)Connection->AutoCommit))
    goto err;

  /* Set isolation level */
  if (Connection->IsolationLevel)
    for (i=0; i < 4; i++)
    {
      if (MADB_IsolationLevel[i].SqlIsolation == Connection->IsolationLevel)
      {
        my_snprintf(StmtStr, 128, "SET SESSION TRANSACTION ISOLATION LEVEL %s",
                    MADB_IsolationLevel[i].StrIsolation);
        if (mysql_query(Connection->mariadb, StmtStr))
          goto err;
        break;
      }
    }

  goto end;

err:
  MADB_SetNativeError(&Connection->Error, SQL_HANDLE_DBC, Connection->mariadb);
      
end:
  if (Connection->Error.ReturnValue == SQL_ERROR && Connection->mariadb)
  {
    mysql_close(Connection->mariadb);
    Connection->mariadb= NULL;
  }

  return Connection->Error.ReturnValue;
}
/* }}} */

/* {{{ MADB_DbcGetFunctions */
SQLRETURN MADB_DbcGetFunctions(MADB_Dbc *Dbc, SQLUSMALLINT FunctionId, SQLUSMALLINT *SupportedPtr)
{
  unsigned int i, Elements= sizeof(MADB_supported_api) / sizeof(SQLUSMALLINT);
  
  switch(FunctionId) {
  case SQL_API_ODBC3_ALL_FUNCTIONS:
    /* clear ptr */ 
    memset(SupportedPtr, 0, sizeof(SQLUSMALLINT) * SQL_API_ODBC3_ALL_FUNCTIONS_SIZE);
    for (i=0; i < Elements; ++i)
    {
      SQLUSMALLINT function= MADB_supported_api[i]; 
      SupportedPtr[function >> 4]|= (1 << (function & 0x000F));
    }
    break;
  case SQL_API_ALL_FUNCTIONS:
    /* Set all to SQL_FALSE (0) */
    memset(SupportedPtr, 0, sizeof(SQLUSMALLINT) * 100);
    for (i=0; i < Elements; i++)
      if (MADB_supported_api[i] < 100)
        SupportedPtr[MADB_supported_api[i]]= SQL_TRUE;
    break;
  default:
    *SupportedPtr= SQL_FALSE;
    for (i=0; i < Elements; i++)
      if (MADB_supported_api[i] == FunctionId)
      {
        *SupportedPtr= SQL_TRUE;
        break;
      }
    break;
  }
  return SQL_SUCCESS;
}
/* }}} */


/* {{{ MADB_DbcGetInfo */
SQLRETURN MADB_DbcGetInfo(MADB_Dbc *Dbc, SQLUSMALLINT InfoType, SQLPOINTER InfoValuePtr,
                          SQLSMALLINT BufferLength, SQLSMALLINT *StringLengthPtr, my_bool isWChar)
{
  SQLSMALLINT SLen= 0;
  if (!InfoValuePtr && !StringLengthPtr)
    return SQL_SUCCESS;

 
  MADB_CLEAR_ERROR(&Dbc->Error);
  switch(InfoType) {
  case SQL_ACCESSIBLE_PROCEDURES:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_ACCESSIBLE_TABLES:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_ACTIVE_ENVIRONMENTS:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_AGGREGATE_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_AF_ALL | SQL_AF_AVG | SQL_AF_COUNT | SQL_AF_DISTINCT |
                                                SQL_AF_MAX | SQL_AF_MIN | SQL_AF_SUM, StringLengthPtr);
    break;
  case SQL_ALTER_DOMAIN:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_ALTER_TABLE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_AT_ADD_COLUMN | SQL_AT_DROP_COLUMN, StringLengthPtr);
    break;
#ifdef SQL_ASYNC_DBC_FUNCTIONS
  case SQL_ASYNC_DBC_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_ASYNC_DBC_NOT_CAPABLE, StringLengthPtr);
    break;
#endif
#ifdef SQL_ASYNC_MODE
  case SQL_ASYNC_MODE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_AM_NONE, StringLengthPtr);
    break;
#endif
#ifdef SQL_ASYNC_NOTIFICATION
  case SQL_ASYNC_NOTIFICATION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_ASYNC_NOTIFICATION_NOT_CAPABLE, StringLengthPtr);
    break;
#endif
  case SQL_BATCH_ROW_COUNT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_BRC_EXPLICIT, StringLengthPtr);
    break;
  case SQL_BATCH_SUPPORT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_BS_SELECT_EXPLICIT | SQL_BS_ROW_COUNT_EXPLICIT |
                                                SQL_BS_SELECT_PROC | SQL_BS_ROW_COUNT_PROC,
                     StringLengthPtr);
    break;
  case SQL_BOOKMARK_PERSISTENCE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CATALOG_LOCATION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CL_START, StringLengthPtr);
    break;
  case SQL_CATALOG_NAME:
    /* Todo: MyODBC Driver has a DSN configuration for diabling catalog usage:
       but it's not implemented in MAODBC */
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_CATALOG_NAME_SEPARATOR:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), ".", SQL_NTS, &Dbc->Error);
    break;
  case SQL_CATALOG_TERM:
    /* todo: See comment for SQL_CATALOG_NAME */
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "database", SQL_NTS, &Dbc->Error);
    break;
  case SQL_CATALOG_USAGE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CU_DML_STATEMENTS | 
                                                SQL_CU_INDEX_DEFINITION |
                                                SQL_CU_PROCEDURE_INVOCATION | 
                                                SQL_CU_PRIVILEGE_DEFINITION |
                                                SQL_CU_TABLE_DEFINITION,
                      StringLengthPtr);
    break;
  case SQL_COLLATION_SEQ:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, 
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     Dbc->mariadb->charset->name, SQL_NTS, &Dbc->Error);
    break;
  case SQL_COLUMN_ALIAS:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, 
                           (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_CONCAT_NULL_BEHAVIOR:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CB_NULL, StringLengthPtr);
    break;
  case SQL_CONVERT_BIGINT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_BINARY:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CONVERT_BIT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_CHAR:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_GUID:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CONVERT_DATE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_DECIMAL:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_DOUBLE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_FLOAT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_INTEGER:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_INTERVAL_YEAR_MONTH:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CONVERT_INTERVAL_DAY_TIME:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CONVERT_LONGVARBINARY:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CONVERT_LONGVARCHAR:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_NUMERIC:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_REAL:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_SMALLINT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_TIME:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_TIMESTAMP:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_TINYINT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_VARBINARY:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CONVERT_VARCHAR:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, MADB_SUPPORTED_CONVERSIONS, StringLengthPtr);
    break;
  case SQL_CONVERT_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CORRELATION_NAME:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CN_DIFFERENT, StringLengthPtr);
    break;
  case SQL_CREATE_ASSERTION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_CHARACTER_SET:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_COLLATION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_DOMAIN:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_SCHEMA:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_TABLE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CT_COLUMN_COLLATION | SQL_CT_COLUMN_DEFAULT |
                                                SQL_CT_COMMIT_DELETE | SQL_CT_CREATE_TABLE |
                                                SQL_CT_LOCAL_TEMPORARY,
                     StringLengthPtr);
    break;
  case SQL_CREATE_TRANSLATION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_CREATE_VIEW:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CV_CASCADED | SQL_CV_CHECK_OPTION |
                                                SQL_CV_CREATE_VIEW, StringLengthPtr);
    break;
  case SQL_CURSOR_COMMIT_BEHAVIOR:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CB_PRESERVE, StringLengthPtr);
    break;
  case SQL_CURSOR_ROLLBACK_BEHAVIOR:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_CB_PRESERVE, StringLengthPtr);
    break;
  case SQL_CURSOR_SENSITIVITY:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_UNSPECIFIED, StringLengthPtr);
    break;
  case SQL_DATA_SOURCE_NAME:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     Dbc->Dsn ? Dbc->Dsn->DSNName : "", SQL_NTS, &Dbc->Error);
    break;
  case SQL_DATABASE_NAME:
    return MADB_Dbc_GetCurrentDB(Dbc, InfoValuePtr, BufferLength, (SQLSMALLINT *)StringLengthPtr, isWChar);
    break;
  case SQL_DATETIME_LITERALS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DL_SQL92_DATE | SQL_DL_SQL92_TIME |
                                                SQL_DL_SQL92_TIMESTAMP, StringLengthPtr);
    break;
  case SQL_DBMS_NAME:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     Dbc->mariadb ? (char *)mysql_get_server_name(Dbc->mariadb) : "MariaDB",
                                     SQL_NTS, &Dbc->Error);
    break;
  case SQL_DBMS_VER:
    {
      char Version[11];
      ulong ServerVersion= 0L;
      unsigned int Major=0, Minor=0, Patch= 0;
      
      if (Dbc->mariadb)
      {
        ServerVersion= mysql_get_server_version(Dbc->mariadb);
        my_snprintf(Version, 11, "%02u.%02u.%06u", ServerVersion / 10000,
                    (ServerVersion % 10000) / 100, ServerVersion % 100);
      }
      else
        Version[0]= 0;
      SLen= (SQLSMALLINT)MADB_SetString(isWChar ?  CP_UTF8 : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                       Version[0] ? Version : "", SQL_NTS, &Dbc->Error);
    }
    break;
  case SQL_DDL_INDEX:
     MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DI_CREATE_INDEX | SQL_DI_DROP_INDEX, StringLengthPtr);
    break;
  case SQL_DEFAULT_TXN_ISOLATION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DESCRIBE_PARAMETER:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "N", SQL_NTS, &Dbc->Error);
    break;
    /*
  case SQL_DM_VER:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ?  CP_UTF8 : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "ma_odbc.dll", SQL_NTS, &Dbc->Error);
    break;
    */
#ifdef SQL_DRIVER_AWARE_POOLING_SUPPORTED
  case SQL_DRIVER_AWARE_POOLING_SUPPORTED:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DRIVER_AWARE_POOLING_NOT_CAPABLE, StringLengthPtr);
    break;
#endif
    /* Handled by driver manager */
  case SQL_DRIVER_HDBC:
    break;
  case SQL_DRIVER_HENV:
    break;
  case SQL_DRIVER_HLIB:
    break;
  case SQL_DRIVER_HSTMT:
    break;
  case SQL_DRIVER_NAME:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     isWChar ? "maodbcu.dll" : "maodbca.dll", SQL_NTS, &Dbc->Error);
    break;
  case SQL_DRIVER_ODBC_VER:
    {
      char *OdbcVersion = "03.51";
      SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     OdbcVersion, SQL_NTS, &Dbc->Error);
    }
    break;
  case SQL_DRIVER_VER:
     SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0,
                                     (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     MARIADB_ODBC_VERSION, SQL_NTS, &Dbc->Error);
    break;
    /*******************************/
  case SQL_DROP_ASSERTION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_CHARACTER_SET:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_COLLATION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_DOMAIN:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_SCHEMA:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_TABLE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DT_CASCADE | SQL_DT_DROP_TABLE | 
                                                SQL_DT_RESTRICT, StringLengthPtr);
    break;
  case SQL_DROP_TRANSLATION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_DROP_VIEW:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_DV_CASCADE | SQL_DV_DROP_VIEW | 
                                                SQL_DV_RESTRICT, StringLengthPtr);
    break;
  case SQL_DYNAMIC_CURSOR_ATTRIBUTES1:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA1_ABSOLUTE |
                                                SQL_CA1_BULK_ADD |
                                                SQL_CA1_LOCK_NO_CHANGE |
                                                SQL_CA1_NEXT |
                                                SQL_CA1_POSITIONED_DELETE |
                                                SQL_CA1_POSITIONED_UPDATE |
                                                SQL_CA1_POS_DELETE |
                                                SQL_CA1_POS_POSITION |
                                                SQL_CA1_POS_REFRESH |
                                                SQL_CA1_POS_UPDATE |
                                                SQL_CA1_RELATIVE, StringLengthPtr);
    break;
  case SQL_DYNAMIC_CURSOR_ATTRIBUTES2:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA2_CRC_EXACT | 
                                                SQL_CA2_MAX_ROWS_DELETE |
                                                SQL_CA2_MAX_ROWS_INSERT |
                                                SQL_CA2_MAX_ROWS_SELECT |
                                                SQL_CA2_MAX_ROWS_UPDATE |
                                                SQL_CA2_SENSITIVITY_ADDITIONS |
                                                SQL_CA2_SENSITIVITY_DELETIONS |
                                                SQL_CA2_SENSITIVITY_UPDATES |
                                                SQL_CA2_SIMULATE_TRY_UNIQUE, StringLengthPtr);
    break;
  case SQL_EXPRESSIONS_IN_ORDERBY:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_FILE_USAGE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FILE_NOT_SUPPORTED, StringLengthPtr);
    break;
  case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA1_ABSOLUTE |
                                                SQL_CA1_BULK_ADD |
                                                SQL_CA1_LOCK_NO_CHANGE |
                                                SQL_CA1_NEXT |
                                                SQL_CA1_POSITIONED_DELETE |
                                                SQL_CA1_POSITIONED_UPDATE |
                                                SQL_CA1_POS_DELETE |
                                                SQL_CA1_POS_POSITION |
                                                SQL_CA1_POS_REFRESH |
                                                SQL_CA1_POS_UPDATE |
                                                SQL_CA1_RELATIVE, StringLengthPtr);
    break;
  case SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA2_CRC_EXACT |
                                                SQL_CA2_MAX_ROWS_DELETE |
                                                SQL_CA2_MAX_ROWS_INSERT |
                                                SQL_CA2_MAX_ROWS_SELECT |
                                                SQL_CA2_MAX_ROWS_UPDATE, StringLengthPtr);
    break;
  case SQL_GETDATA_EXTENSIONS:
  {
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_GD_ANY_COLUMN | SQL_GD_ANY_ORDER |
                                                SQL_GD_BLOCK | SQL_GD_BOUND, StringLengthPtr);
    break;
  }
  case SQL_GROUP_BY:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_GB_NO_RELATION, StringLengthPtr);
    break;
  case SQL_IDENTIFIER_CASE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_IC_MIXED, StringLengthPtr);
    break;
  case SQL_IDENTIFIER_QUOTE_CHAR:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "`", SQL_NTS, &Dbc->Error);
    break;
  case SQL_INDEX_KEYWORDS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_IK_ALL, StringLengthPtr);
    break;
  case SQL_INFO_SCHEMA_VIEWS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_ISV_CHARACTER_SETS | SQL_ISV_COLLATIONS |
                                                SQL_ISV_COLUMNS | SQL_ISV_COLUMN_PRIVILEGES |
                                                SQL_ISV_KEY_COLUMN_USAGE | SQL_ISV_REFERENTIAL_CONSTRAINTS |
                                                SQL_ISV_TABLES | SQL_ISV_TABLE_PRIVILEGES |
                                                SQL_ISV_TABLE_CONSTRAINTS | SQL_ISV_VIEWS, StringLengthPtr);
    break;
  case SQL_INSERT_STATEMENT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_IS_INSERT_LITERALS | SQL_IS_INSERT_SEARCHED |
                                                SQL_IS_SELECT_INTO, StringLengthPtr);
    break;
  case SQL_INTEGRITY:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_KEYSET_CURSOR_ATTRIBUTES1:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_KEYSET_CURSOR_ATTRIBUTES2:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_KEYWORDS:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "ACCESSIBLE,ANALYZE,ASENSITIVE,BEFORE,BIGINT,BINARY,BLOB,CALL,"
                                     "CHANGE,CONDITION,DATABASE,DATABASES,DAY_HOUR,DAY_MICROSECOND,"
                                     "DAY_MINUTE,DAY_SECOND,DELAYED,DETERMINISTIC,DISTINCTROW,DIV,"
                                     "DUAL,EACH,ELSEIF,ENCLOSED,ESCAPED,EXIT,EXPLAIN,FLOAT4,FLOAT8,"
                                     "FORCE,FULLTEXT,HIGH_PRIORITY,HOUR_MICROSECOND,HOUR_MINUTE,"
                                     "HOUR_SECOND,IF,IGNORE,INFILE,INOUT,INT1,INT2,INT3,INT4,INT8,"
                                     "ITERATE,KEY,KEYS,KILL,LEAVE,LIMIT,LINEAR,LINES,LOAD,LOCALTIME,"
                                     "LOCALTIMESTAMP,LOCK,LONG,LONGBLOB,LONGTEXT,LOOP,LOW_PRIORITY,"
                                     "MEDIUMBLOB,MEDIUMINT,MEDIUMTEXT,MIDDLEINT,MINUTE_MICROSECOND,"
                                     "MINUTE_SECOND,MOD,MODIFIES,NO_WRITE_TO_BINLOG,OPTIMIZE,OPTIONALLY,"
                                     "OUT,OUTFILE,PURGE,RANGE,READS,READ_ONLY,READ_WRITE,REGEXP,RELEASE,"
                                     "RENAME,REPEAT,REPLACE,REQUIRE,RETURN,RLIKE,SCHEMAS,"
                                     "SECOND_MICROSECOND,SENSITIVE,SEPARATOR,SHOW,SPATIAL,SPECIFIC,"
                                     "SQLEXCEPTION,SQL_BIG_RESULT,SQL_CALC_FOUND_ROWS,SQL_SMALL_RESULT,"
                                     "SSL,STARTING,STRAIGHT_JOIN,TERMINATED,TINYBLOB,TINYINT,TINYTEXT,"
                                     "TRIGGER,UNDO,UNLOCK,UNSIGNED,USE,UTC_DATE,UTC_TIME,UTC_TIMESTAMP,"
                                     "VARBINARY,VARCHARACTER,WHILE,X509,XOR,YEAR_MONTH,ZEROFILL,GENERAL,"
                                     "IGNORE_SERVER_IDS,MASTER_HEARTBEAT_PERIOD,MAXVALUE,RESIGNAL,SIGNAL,"
                                     "SLOW", SQL_NTS, &Dbc->Error);
    break;
  case SQL_LIKE_ESCAPE_CLAUSE:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_MAX_ASYNC_CONCURRENT_STATEMENTS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_BINARY_LITERAL_LEN:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_CATALOG_NAME_LEN:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_CHAR_LITERAL_LEN:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_COLUMN_NAME_LEN:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_GROUP_BY:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_INDEX:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 32, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_ORDER_BY:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_SELECT:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_COLUMNS_IN_TABLE:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_CONCURRENT_ACTIVITIES:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_CURSOR_NAME_LEN:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, MADB_MAX_CURSOR_NAME, StringLengthPtr);
    break;
  case SQL_MAX_DRIVER_CONNECTIONS:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_IDENTIFIER_LEN:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_INDEX_SIZE:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 3072, StringLengthPtr);
    break;
  case SQL_MAX_PROCEDURE_NAME_LEN:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_ROW_SIZE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_ROW_SIZE_INCLUDES_LONG:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_MAX_SCHEMA_NAME_LEN:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_MAX_STATEMENT_LEN:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, max_allowed_packet, StringLengthPtr);
    break;
  case SQL_MAX_TABLE_NAME_LEN:
     MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, NAME_CHAR_LEN * SYSTEM_MB_MAX_CHAR_LENGTH, StringLengthPtr);
    break;
  case SQL_MAX_TABLES_IN_SELECT:
     MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, 63, StringLengthPtr);
    break;
  case SQL_MAX_USER_NAME_LEN:
     MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, USERNAME_LENGTH, StringLengthPtr);
    break;
  case SQL_MULT_RESULT_SETS:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_MULTIPLE_ACTIVE_TXN:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_NEED_LONG_DATA_LEN:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_NON_NULLABLE_COLUMNS:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_NNC_NON_NULL, StringLengthPtr);
    break;
  case SQL_NULL_COLLATION:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_NC_LOW, StringLengthPtr);
    break;
  case SQL_NUMERIC_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_NUM_ABS | SQL_FN_NUM_ACOS | SQL_FN_NUM_ASIN |
                                                SQL_FN_NUM_ATAN | SQL_FN_NUM_ATAN2 | SQL_FN_NUM_CEILING |
                                                SQL_FN_NUM_COS | SQL_FN_NUM_COT | SQL_FN_NUM_EXP |
                                                SQL_FN_NUM_FLOOR | SQL_FN_NUM_LOG | SQL_FN_NUM_MOD |
                                                SQL_FN_NUM_SIGN | SQL_FN_NUM_SIN | SQL_FN_NUM_SQRT |
                                                SQL_FN_NUM_TAN | SQL_FN_NUM_PI | SQL_FN_NUM_RAND |
                                                SQL_FN_NUM_DEGREES | SQL_FN_NUM_LOG10 | SQL_FN_NUM_POWER |
                                                SQL_FN_NUM_RADIANS | SQL_FN_NUM_ROUND | SQL_FN_NUM_TRUNCATE,
                     StringLengthPtr);
    break;
  case SQL_ODBC_API_CONFORMANCE:
    MADB_SET_NUM_VAL(SQLSMALLINT, InfoValuePtr, SQL_OAC_LEVEL1, StringLengthPtr);
    break;
  case SQL_ODBC_INTERFACE_CONFORMANCE:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_OIC_CORE, StringLengthPtr);
    break;
  case SQL_ODBC_SQL_CONFORMANCE:
    MADB_SET_NUM_VAL(SQLSMALLINT, InfoValuePtr, SQL_OSC_CORE, StringLengthPtr);
    break;
  case SQL_ODBC_VER:
    break;
  case SQL_OJ_CAPABILITIES:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_OJ_LEFT | SQL_OJ_RIGHT |
                                                SQL_OJ_NESTED | SQL_OJ_INNER, StringLengthPtr);
    break;
  case SQL_ORDER_BY_COLUMNS_IN_SELECT:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_PARAM_ARRAY_ROW_COUNTS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_PARC_NO_BATCH, StringLengthPtr);
    break;
  case SQL_PARAM_ARRAY_SELECTS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_PAS_NO_BATCH, StringLengthPtr);
    break;
  case SQL_PROCEDURE_TERM:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "stored procedure", SQL_NTS, &Dbc->Error);
    break;
  case SQL_PROCEDURES:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "Y", SQL_NTS, &Dbc->Error);
    break;
  case SQL_POS_OPERATIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_QUOTED_IDENTIFIER_CASE:  
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_IC_SENSITIVE, StringLengthPtr);
    break;
  case SQL_ROW_UPDATES:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, 
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  case SQL_SCHEMA_TERM:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr,
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "", SQL_NTS, &Dbc->Error);
    break;
  case SQL_SCHEMA_USAGE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_SCROLL_OPTIONS:
    {
      SQLUINTEGER Options= SQL_SO_FORWARD_ONLY;
      if (!MA_ODBC_CURSOR_FORWARD_ONLY(Dbc))
        Options|= SQL_SO_STATIC;
      if (MA_ODBC_CURSOR_DYNAMIC(Dbc))
        Options|= SQL_SO_DYNAMIC;
      MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, Options, StringLengthPtr);
    }
    break;
  case SQL_SEARCH_PATTERN_ESCAPE:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr,
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "\\", SQL_NTS, &Dbc->Error);
    break;
  case SQL_SERVER_NAME:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, 
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     !(Dbc->mariadb) ? "" : Dbc->mariadb->host_info, SQL_NTS, &Dbc->Error);
    break;
  case SQL_SPECIAL_CHARACTERS:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr, 
                                      BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "\"\\/", SQL_NTS, &Dbc->Error);
    break;
  case SQL_SQL_CONFORMANCE:
     MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SC_SQL92_INTERMEDIATE, StringLengthPtr);
    break;
  case SQL_SQL92_DATETIME_FUNCTIONS:
     MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SDF_CURRENT_DATE | SQL_SDF_CURRENT_TIME |
                                                 SQL_SDF_CURRENT_TIMESTAMP, StringLengthPtr);
    break;
  case SQL_SQL92_FOREIGN_KEY_DELETE_RULE:
     MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_SQL92_FOREIGN_KEY_UPDATE_RULE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_SQL92_GRANT:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SG_DELETE_TABLE | SQL_SG_INSERT_COLUMN |
                                                SQL_SG_INSERT_TABLE | SQL_SG_REFERENCES_COLUMN |
                                                SQL_SG_REFERENCES_TABLE | SQL_SG_SELECT_TABLE |
                                                SQL_SG_UPDATE_COLUMN | SQL_SG_UPDATE_TABLE |
                                                SQL_SG_WITH_GRANT_OPTION, StringLengthPtr);
    break;
  case SQL_SQL92_NUMERIC_VALUE_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SNVF_BIT_LENGTH | SQL_SNVF_CHARACTER_LENGTH |
                                                SQL_SNVF_CHAR_LENGTH | SQL_SNVF_EXTRACT |
                                                SQL_SNVF_OCTET_LENGTH | SQL_SNVF_POSITION,
                     StringLengthPtr);
    break;
  case SQL_SQL92_PREDICATES:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SP_BETWEEN | SQL_SP_COMPARISON |
                                                SQL_SP_EXISTS | SQL_SP_IN | SQL_SP_ISNOTNULL |
                                                SQL_SP_ISNULL | SQL_SP_LIKE | SQL_SP_QUANTIFIED_COMPARISON,
                     StringLengthPtr);
    break;
  case SQL_SQL92_RELATIONAL_JOIN_OPERATORS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SRJO_CROSS_JOIN | SQL_SRJO_INNER_JOIN |
                                                SQL_SRJO_LEFT_OUTER_JOIN | SQL_SRJO_RIGHT_OUTER_JOIN |
                                                SQL_SRJO_NATURAL_JOIN,
                     StringLengthPtr);
    break;
  case SQL_SQL92_REVOKE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SR_DELETE_TABLE | SQL_SR_INSERT_COLUMN |
                                                SQL_SR_INSERT_TABLE | SQL_SR_REFERENCES_COLUMN |
                                                SQL_SR_REFERENCES_TABLE | SQL_SR_SELECT_TABLE |
                                                SQL_SR_UPDATE_COLUMN | SQL_SR_UPDATE_TABLE,
                     StringLengthPtr);
    break;
  case SQL_SQL92_ROW_VALUE_CONSTRUCTOR:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SRVC_DEFAULT | SQL_SRVC_NULL |
                                                 SQL_SRVC_ROW_SUBQUERY | SQL_SRVC_VALUE_EXPRESSION,
                     StringLengthPtr);
    break;
  case SQL_SQL92_STRING_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SSF_CONVERT | SQL_SSF_LOWER |
                                                SQL_SSF_SUBSTRING | SQL_SSF_TRANSLATE |
                                                SQL_SSF_TRIM_BOTH | SQL_SSF_TRIM_LEADING |
                                                SQL_SSF_TRIM_TRAILING | SQL_SSF_UPPER,
                     StringLengthPtr);
    break;
  case SQL_SQL92_VALUE_EXPRESSIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SVE_CASE | SQL_SVE_CAST | SQL_SVE_COALESCE |
                                                SQL_SVE_NULLIF, StringLengthPtr);
    break;
  case SQL_STANDARD_CLI_CONFORMANCE:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SCC_ISO92_CLI, StringLengthPtr);
    break;
  case SQL_STATIC_CURSOR_ATTRIBUTES1:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA1_ABSOLUTE | /* SQL_CA1_BOOKMARK | */
                                                SQL_CA1_NEXT | SQL_CA1_RELATIVE,
                     StringLengthPtr);
    break;
  case SQL_STATIC_CURSOR_ATTRIBUTES2:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_CA2_MAX_ROWS_SELECT, StringLengthPtr);
    break;
  case SQL_STRING_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_STR_ASCII | SQL_FN_STR_BIT_LENGTH |
                                                SQL_FN_STR_CHAR | SQL_FN_STR_CHAR_LENGTH |
                                                SQL_FN_STR_CONCAT | SQL_FN_STR_INSERT | 
                                                SQL_FN_STR_LCASE | SQL_FN_STR_LEFT | 
                                                SQL_FN_STR_LENGTH | SQL_FN_STR_LOCATE |
                                                SQL_FN_STR_LOCATE_2 | SQL_FN_STR_LTRIM |
                                                SQL_FN_STR_OCTET_LENGTH | SQL_FN_STR_POSITION |
                                                SQL_FN_STR_REPEAT | SQL_FN_STR_REPLACE |
                                                SQL_FN_STR_RIGHT | SQL_FN_STR_RTRIM |
                                                SQL_FN_STR_SOUNDEX | SQL_FN_STR_SPACE |
                                                SQL_FN_STR_SUBSTRING | SQL_FN_STR_UCASE,
                     StringLengthPtr);
    break;
  case SQL_SUBQUERIES:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_SQ_COMPARISON | SQL_SQ_CORRELATED_SUBQUERIES | 
                                                SQL_SQ_EXISTS | SQL_SQ_IN | SQL_SQ_QUANTIFIED,
                     StringLengthPtr);
    break;
  case SQL_SYSTEM_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_SYS_DBNAME | SQL_FN_SYS_IFNULL |
                                                SQL_FN_SYS_USERNAME, StringLengthPtr);
    break;
  case SQL_TABLE_TERM:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr,
                                      BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     "table", SQL_NTS, &Dbc->Error);
    break;
  case SQL_TIMEDATE_ADD_INTERVALS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_TIMEDATE_DIFF_INTERVALS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, 0, StringLengthPtr);
    break;
  case SQL_TIMEDATE_FUNCTIONS:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_FN_TD_CURDATE | SQL_FN_TD_CURRENT_DATE | 
                                                SQL_FN_TD_CURRENT_TIME | SQL_FN_TD_CURRENT_TIMESTAMP | 
                                                SQL_FN_TD_CURTIME | SQL_FN_TD_DAYNAME |
                                                SQL_FN_TD_DAYOFMONTH | SQL_FN_TD_DAYOFWEEK |
                                                SQL_FN_TD_DAYOFYEAR | SQL_FN_TD_EXTRACT | 
                                                SQL_FN_TD_HOUR | SQL_FN_TD_MINUTE | 
                                                SQL_FN_TD_MONTH | SQL_FN_TD_MONTHNAME | 
                                                SQL_FN_TD_NOW | SQL_FN_TD_QUARTER | 
                                                SQL_FN_TD_SECOND | SQL_FN_TD_TIMESTAMPADD |
                                                SQL_FN_TD_TIMESTAMPDIFF | SQL_FN_TD_WEEK | SQL_FN_TD_YEAR,
                      StringLengthPtr);
    break;
  case SQL_TXN_CAPABLE:
    MADB_SET_NUM_VAL(SQLUSMALLINT, InfoValuePtr, SQL_TC_DDL_COMMIT, StringLengthPtr);
    break;
  case SQL_TXN_ISOLATION_OPTION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_TXN_READ_COMMITTED | SQL_TXN_READ_UNCOMMITTED |
                                                SQL_TXN_REPEATABLE_READ | SQL_TXN_SERIALIZABLE,
                      StringLengthPtr);
    break;
  case SQL_UNION:
    MADB_SET_NUM_VAL(SQLUINTEGER, InfoValuePtr, SQL_U_UNION | SQL_U_UNION_ALL, StringLengthPtr);
    break;
  case SQL_USER_NAME:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr,
                                     BUFFER_CHAR_LEN(BufferLength, isWChar), 
                                     (Dbc->mariadb) ? Dbc->mariadb->user : "", SQL_NTS, &Dbc->Error);
    break;
  case SQL_XOPEN_CLI_YEAR:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr,
                                     BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "1992", SQL_NTS, &Dbc->Error);
    break;
  case SQL_DATA_SOURCE_READ_ONLY:
    SLen= (SQLSMALLINT)MADB_SetString(isWChar ? Dbc->CodePage : 0, (void *)InfoValuePtr,
                                     BUFFER_CHAR_LEN(BufferLength, isWChar),
                                     "N", SQL_NTS, &Dbc->Error);
    break;
  default:
    MADB_SetError(&Dbc->Error, MADB_ERR_HY096, NULL, 0);
    return Dbc->Error.ReturnValue;
  }
  if (isWChar && SLen)
    SLen*= sizeof(SQLWCHAR);
  if (StringLengthPtr && SLen)
    *StringLengthPtr= SLen;
  
  return SQL_SUCCESS;
}
/* }}} */


/* {{{ MADB_DriverSideMemalocate */
char * MADB_DriverSideAllocate(size_t size)
{
  return (char *)MADB_CALLOC(size);
}
/* }}} */


/* {{{ MADB_DriverSideFree */
void MADB_DriverSideFree(void *ptr)
{
  MADB_FREE(ptr);
}
/* }}} */


/* {{{ MADB_DriverConnect */
SQLRETURN MADB_DriverConnect(MADB_Dbc *Dbc, SQLHWND WindowHandle, SQLCHAR *InConnectionString,
                             SQLSMALLINT StringLength1, SQLCHAR *OutConnectionString,
                             SQLSMALLINT BufferLength, SQLSMALLINT *StringLength2Ptr,
                             SQLUSMALLINT DriverCompletion)
{
  MADB_Dsn   *Dsn;
  MADB_Drv   *Drv=       NULL;
  SQLRETURN   ret=       SQL_SUCCESS;
  MADB_Prompt DSNPrompt= { NULL, NULL };
  SQLSMALLINT Length;

  if (!Dbc)
    return SQL_INVALID_HANDLE;

  MADB_CLEAR_ERROR(&Dbc->Error);

  Dsn= MADB_DSN_Init();

  if (!MADB_ParseDSNString(Dsn, (char *)InConnectionString, StringLength1, ';'))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY000, "Error while parsing DSN", 0);
    goto error;
  }

  /* Read DSN, but don't overwrite values! */
  if (Dsn->DSNName)
    MADB_ReadDSN(Dsn, NULL, FALSE);
    
  /* if DSN prompt is off, adjust DriverCompletion */
  if (Dsn->ConnectPrompt)
    DriverCompletion= SQL_DRIVER_NOPROMPT;

  switch (DriverCompletion) {
  case SQL_DRIVER_COMPLETE:
  case SQL_DRIVER_COMPLETE_REQUIRED:
  case SQL_DRIVER_NOPROMPT:
    {
      SQLRETURN ret= MADB_DbcConnectDB(Dbc, Dsn);
      if (SQL_SUCCEEDED(ret))
        goto end;
      else
        goto error;
    }
    break;
  case SQL_DRIVER_PROMPT:
    Dsn->isPrompt= MAODBC_PROMPT/*_REQUIRED*/;
    break;
  default:
    MADB_SetError(&Dbc->Error, MADB_ERR_HY110, NULL, 0);
    goto error;
    break;
  }

  /* Without window handle we can't show a dialog */
  if (DriverCompletion == SQL_DRIVER_NOPROMPT && !WindowHandle)
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_IM008, NULL, 0);
    goto error;
  }
  
  /* We need to obtain the driver name to load maodbcs.dll, if it's not stored inside DSN, 
     error IM007 (dialog prohibited) will be returned */
  if (!Dsn->Driver)
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_IM007, NULL, 0);
    goto error;
  }
  
  if (!(Drv= MADB_DriverGet(Dsn->Driver)))
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_IM003, NULL, 0);
    goto error;
  }
  if (!Drv->SetupLibrary)
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_HY000, "Couldn't determine setup library", 0);
    goto error;
  }
 
  if (!SQL_SUCCEEDED(DSNPrompt_Lookup(&DSNPrompt, Drv->SetupLibrary, Dbc)))
  {
    goto error;
  }

  Dsn->allocator= MADB_DriverSideAllocate;
  Dsn->free=      MADB_DriverSideFree;

  if (DSNPrompt.Call((HWND)WindowHandle, Dsn) == FALSE)
  {
    /* If user cancels prompt, SQLDriverConnect should return SQL_NO_DATA */
    Dbc->Error.ReturnValue= SQL_NO_DATA;
    goto error;
  }

  DSNPrompt_Free(&DSNPrompt);

  ret= MADB_DbcConnectDB(Dbc, (void *)Dsn);
  if (!SQL_SUCCEEDED(ret))
    goto error;
end:
  Dbc->Dsn= Dsn;
  Length= MADB_DsnToString(Dsn, (char *)OutConnectionString, BufferLength);
  if (StringLength2Ptr)
    *StringLength2Ptr= (SQLSMALLINT)Length;
  if (OutConnectionString && BufferLength && Length > BufferLength)
  {
    MADB_SetError(&Dbc->Error, MADB_ERR_01004, NULL, 0);
    return Dbc->Error.ReturnValue;
  }
  if (StringLength2Ptr)
    *StringLength2Ptr= Length;
  return ret;
error:
  DSNPrompt_Free(&DSNPrompt);
  MADB_DSN_Free(Dsn);
  MADB_DriverFree(Drv);
  return Dbc->Error.ReturnValue;
}
/* }}} */

struct st_ma_connection_methods MADB_Dbc_Methods =
{ 
  MADB_DbcSetAttr,
  MADB_DbcGetAttr,
  MADB_DbcConnectDB,
  MADB_DbcEndTran,
  MADB_DbcGetFunctions,
  MADB_DbcGetInfo,
  MADB_DriverConnect
};
