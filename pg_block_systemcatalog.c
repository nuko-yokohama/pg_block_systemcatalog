/*------------------------------------------------------------------------------
 * pg_block_systemcatalog.c
 *
 * PostgreSQL extension to block references to system catalogs.
 *
 * author @nuko_yokohama
 *
 *------------------------------------------------------------------------------
 */
#include "postgres.h"

#include "executor/executor.h"
#include "executor/spi.h"
#include "miscadmin.h"
#include "libpq/auth.h"
#include "tcop/utility.h"
#include "utils/builtins.h"
#include "utils/guc.h"
#include "utils/memutils.h"
#include "utils/timestamp.h"
#include "libpq/auth.h"
#include "port.h"
#include "catalog/catalog.h"


#include "pgtime.h"
#include <time.h>

PG_MODULE_MAGIC;

extern bool superuser();

/*
 * GUC variable fot pg_block_systemcatalog
 *
 * Role name to allow a reference to the system catalog.
 *
 */
char *allow_role = NULL;

void _PG_init(void);


/*
 * Hook functions
 */
static ExecutorCheckPerms_hook_type next_ExecutorCheckPerms_hook = NULL;

static void checkTables(List * rangeTabls)
{
    ListCell *lr;
    foreach(lr, rangeTabls)
    {
        RangeTblEntry *rte = lfirst(lr);
        Relation rel;
        Oid relOid;

        if (!superuser()) {
            relOid = rte->relid;
            rel = relation_open(relOid, NoLock);

            if (IsSystemNamespace(RelationGetNamespace(rel)))
            {
                relation_close(rel, NoLock);
                ereport(ERROR, (errcode(ERRCODE_INVALID_SCHEMA_DEFINITION),
                    errmsg("pg_block_systemcatalog: Reference to the system catalog is not permitted.")));
            }
            relation_close(rel, NoLock);
        }
    } 
}

/*
 * Hook ExecutorCheckPerms_hook.
 */
static bool
pg_bsc_ExecutorCheckPerms_hook(List *rangeTabls, bool abort)
{

    checkTables(rangeTabls);

    /* Call the next hook function */
    if (next_ExecutorCheckPerms_hook &&
        !(*next_ExecutorCheckPerms_hook) (rangeTabls, abort))
        return false;

    return true;
}


void
_PG_init(void)
{
    /* Must be loaded with shared_preload_libaries */
    if (IsUnderPostmaster)
        ereport(ERROR, (errcode(ERRCODE_OBJECT_NOT_IN_PREREQUISITE_STATE),
                errmsg("pg_block_systemcatalog must be loaded via shared_preload_libraries")));

    /*
     * pg_block_systemcatalog.allow_role
     */
    DefineCustomStringVariable(
        "pg_block_systemcatalog.allow_role",
        "Role name to allow a reference to the system catalog.",
        NULL,
        &allow_role,
        NULL,
        PGC_SUSET,
        GUC_NOT_IN_SAMPLE,
        NULL, NULL, NULL);

    /*
     * Install our hook functions after saving the existing pointers to
     * preserve the chains.
     */
    next_ExecutorCheckPerms_hook = ExecutorCheckPerms_hook;
    ExecutorCheckPerms_hook = pg_bsc_ExecutorCheckPerms_hook;

    /* Log that the extension has completed initialization */
    ereport(LOG, (errmsg("pg_block_systemcatalog.")));

}

