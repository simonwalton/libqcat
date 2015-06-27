
#include <math.h>

#include <postgres.h>
#include <fmgr.h>

#ifdef PG_MODULE_MAGIC
PG_MODULE_MAGIC;
#endif

PG_FUNCTION_INFO_V1(_log2);
Datum _log2(PG_FUNCTION_ARGS) {
	PG_RETURN_FLOAT4(log2(PG_GETARG_FLOAT4(0)));
}
