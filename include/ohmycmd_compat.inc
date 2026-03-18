#if defined _INC_ohmycmd_compat
    #endinput
#endif
#define _INC_ohmycmd_compat

#include <ohmycmd>

/*
    Compatibility helper layer for incremental migrations.

    Targeted legacy patterns:
    - ycmd-style CMD_<name>
    - zcmd-style cmd_<name>
    - custom legacy public names
*/

stock bool:OhmyCmd_MigrateYCMD(const name[], flags = 0, const description[] = "", const usage[] = "")
{
    return OhmyCmd_RegisterCompatEx(name, flags, description, usage);
}

stock bool:OhmyCmd_MigrateZCMD(const name[], flags = 0, const description[] = "", const usage[] = "")
{
    return OhmyCmd_RegisterCompatEx(name, flags, description, usage);
}

stock bool:OhmyCmd_MigrateLegacy(const name[], const handler[], flags = 0, const description[] = "", const usage[] = "")
{
    return OhmyCmd_RegisterEx(name, handler, flags, description, usage);
}
