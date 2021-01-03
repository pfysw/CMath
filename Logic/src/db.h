/*
 * db.h
 *
 *  Created on: Dec 31, 2020
 *      Author: Administrator
 */

#ifndef DB_H_
#define DB_H_
#include "sqlite3.h"
#include "ast.h"

#define SQL_LEN 1000

struct DbInfo
{
    sqlite3 *db;
    sqlite3_stmt *stmt;
};

typedef struct vector Vector;

int select_callback(void * data, int col_count, char ** col_values, char ** col_Name);
sqlite3 * CreatSqliteConn(char *db_name);
void SqliteWriteNode(AstParse *pParse,sqlite3 *db,char *zProp);
void SqliteReadTable(AstParse *pParse,sqlite3 *db,char *table,Vector *pV);
void WritePropToDb(AstParse *pParse,char azProp[][PROP_STR_LEN]);
void BeginSqliteWrite(AstParse *pParse);
void EndSqliteWrite(AstParse *pParse);
void WriteAxiomToDb(AstParse *pParse,char *zProp);
void GetPropStrParse(AstParse *pParse,char *buf,Vector *pV,int len);

#endif /* DB_H_ */
