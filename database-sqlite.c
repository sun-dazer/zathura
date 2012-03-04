/* See LICENSE file for license and copyright information */

#include <glib.h>
#include <sqlite3.h>
#include <girara/utils.h>
#include <girara/datastructures.h>

#include "database.h"

#define DATABASE "bookmarks.sqlite"

struct zathura_database_s
{
  sqlite3* session;
};

zathura_database_t*
zathura_db_init(const char* dir)
{
  if (dir == NULL) {
    return NULL;
  }

  char* path = g_build_filename(dir, DATABASE, NULL);
  if (path == NULL) {
    return NULL;
  }

  zathura_database_t* db = g_malloc0(sizeof(zathura_database_t));

  /* create bookmarks database */
  static const char SQL_BOOKMARK_INIT[] =
    "CREATE TABLE IF NOT EXISTS bookmarks ("
      "file TEXT,"
      "id TEXT,"
      "page INTEGER,"
      "PRIMARY KEY(file, id));";

  static const char SQL_FILEINFO_INIT[] =
    "CREATE TABLE IF NOT EXISTS fileinfo ("
      "file TEXT PRIMARY KEY,"
      "page INTEGER,"
      "offset INTEGER,"
      "scale FLOAT,"
      "rotation INTEGER);";

  if (sqlite3_open(path, &(db->session)) != SQLITE_OK) {
    girara_error("Could not open database: %s\n", path);
    goto error_free;
  }

  if (sqlite3_exec(db->session, SQL_BOOKMARK_INIT, NULL, 0, NULL) != SQLITE_OK) {
    girara_error("Failed to initialize database: %s\n", path);
    goto error_free;
  }

  if (sqlite3_exec(db->session, SQL_FILEINFO_INIT, NULL, 0, NULL) != SQLITE_OK) {
    girara_error("Failed to initialize database: %s\n", path);
    goto error_free;
  }

  return db;

error_free:

  zathura_db_free(db);
  g_free(path);

  return NULL;
}

void
zathura_db_free(zathura_database_t* db)
{
  if (db == NULL) {
    return;
  }

  if (db->session != NULL) {
    sqlite3_close(db->session);
  }

  g_free(db);
}

static sqlite3_stmt*
prepare_statement(sqlite3* session, const char* statement)
{
  if (session == NULL || statement == NULL) {
    return NULL;
  }

  const char* pz_tail   = NULL;
  sqlite3_stmt* pp_stmt = NULL;

  if (sqlite3_prepare_v2(session, statement, -1, &pp_stmt, &pz_tail) != SQLITE_OK) {
    girara_error("Failed to prepare query: %s", statement);
    sqlite3_finalize(pp_stmt);
    return NULL;
  } else if (pz_tail && *pz_tail != '\0') {
    girara_error("Unused portion of statement: %s", pz_tail);
    sqlite3_finalize(pp_stmt);
    return NULL;
  }

  return pp_stmt;
}

bool
zathura_db_add_bookmark(zathura_database_t* db, const char* file,
    zathura_bookmark_t* bookmark)
{
  g_return_val_if_fail(db && file && bookmark, false);

  static const char SQL_BOOKMARK_ADD[] =
    "REPLACE INTO bookmarks (file, id, page) VALUES (?, ?, ?);";

  sqlite3_stmt* stmt = prepare_statement(db->session, SQL_BOOKMARK_ADD);
  if (stmt == NULL) {
    return false;
  }

  if (sqlite3_bind_text(stmt, 1, file, -1, NULL) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 2, bookmark->id, -1, NULL) != SQLITE_OK ||
      sqlite3_bind_int(stmt, 3, bookmark->page) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    girara_error("Failed to bind arguments.");
    return false;
  }

  int res = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return res == SQLITE_DONE;
}

bool
zathura_db_remove_bookmark(zathura_database_t* db, const char* file, const char*
    id)
{
  g_return_val_if_fail(db && file && id, false);

  static const char SQL_BOOKMARK_ADD[] =
    "DELETE FROM bookmarks WHERE file = ? AND id = ?;";

  sqlite3_stmt* stmt = prepare_statement(db->session, SQL_BOOKMARK_ADD);
  if (stmt == NULL) {
    return false;
  }

  if (sqlite3_bind_text(stmt, 1, file, -1, NULL) != SQLITE_OK ||
      sqlite3_bind_text(stmt, 2, id, -1, NULL) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    girara_error("Failed to bind arguments.");
    return false;
  }

  int res = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return res == SQLITE_DONE;
}

girara_list_t*
zathura_db_load_bookmarks(zathura_database_t* db, const char* file)
{
  g_return_val_if_fail(db && file, NULL);

  static const char SQL_BOOKMARK_SELECT[] =
    "SELECT id, page FROM bookmarks WHERE file = ?;";

  sqlite3_stmt* stmt = prepare_statement(db->session, SQL_BOOKMARK_SELECT);
  if (stmt == NULL) {
    return NULL;
  }

  if (sqlite3_bind_text(stmt, 1, file, -1, NULL) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    girara_error("Failed to bind arguments.");
    return NULL;
  }

  girara_list_t* result = girara_sorted_list_new2((girara_compare_function_t) zathura_bookmarks_compare,
      (girara_free_function_t) zathura_bookmark_free);
  if (result == NULL) {
    sqlite3_finalize(stmt);
    return NULL;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    zathura_bookmark_t* bookmark = g_malloc0(sizeof(zathura_bookmark_t));

    bookmark->id   = g_strdup((const char*) sqlite3_column_text(stmt, 0));
    bookmark->page = sqlite3_column_int(stmt, 1);

    girara_list_append(result, bookmark);
  }
  sqlite3_finalize(stmt);
  return result;
}

bool
zathura_db_set_fileinfo(zathura_database_t* db, const char* file, unsigned int
    page, int offset, double scale, int rotation)
{
  g_return_val_if_fail(db && file, false);

  static const char SQL_FILEINFO_SET[] =
    "REPLACE INTO fileinfo (file, page, offset, scale, rotation) VALUES (?, ?, ?, ?, ?);";

  sqlite3_stmt* stmt = prepare_statement(db->session, SQL_FILEINFO_SET);
  if (stmt == NULL) {
    return false;
  }

  if (sqlite3_bind_text(stmt, 1, file, -1, NULL) != SQLITE_OK ||
      sqlite3_bind_int(stmt, 2, page) != SQLITE_OK ||
      sqlite3_bind_int(stmt, 3, offset) != SQLITE_OK ||
      sqlite3_bind_double(stmt, 4, scale) != SQLITE_OK ||
      sqlite3_bind_int(stmt, 5, rotation) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    girara_error("Failed to bind arguments.");
    return false;
  }

  int res = sqlite3_step(stmt);
  sqlite3_finalize(stmt);
  return res == SQLITE_DONE;
}

bool
zathura_db_get_fileinfo(zathura_database_t* db, const char* file, unsigned int*
    page, int* offset, double* scale, int* rotation)
{
  g_return_val_if_fail(db && file && page && offset && scale && rotation, false);

  static const char SQL_FILEINFO_GET[] =
    "SELECT page, offset, scale, rotation FROM fileinfo WHERE file = ?;";

  sqlite3_stmt* stmt = prepare_statement(db->session, SQL_FILEINFO_GET);
  if (stmt == NULL) {
    return false;
  }

  if (sqlite3_bind_text(stmt, 1, file, -1, NULL) != SQLITE_OK) {
    sqlite3_finalize(stmt);
    girara_error("Failed to bind arguments.");
    return false;
  }

  if (sqlite3_step(stmt) != SQLITE_ROW) {
    sqlite3_finalize(stmt);
    girara_info("No info for file %s available.", file);
    return false;
  }

  *page = sqlite3_column_int(stmt, 0);
  *offset = sqlite3_column_int(stmt, 1);
  *scale = sqlite3_column_double(stmt, 2);
  *rotation = sqlite3_column_int(stmt, 3);
  sqlite3_finalize(stmt);
  return true;
}
