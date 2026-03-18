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

stock bool:OMC_MigrateYCMD(const name[], flags = 0, const description[] = "", const usage[] = "")
{
    return OMC_RegisterCompatEx(name, flags, description, usage);
}

stock bool:OMC_MigrateZCMD(const name[], flags = 0, const description[] = "", const usage[] = "")
{
    return OMC_RegisterCompatEx(name, flags, description, usage);
}

stock bool:OMC_MigrateLegacy(const name[], const handler[], flags = 0, const description[] = "", const usage[] = "")
{
    return OMC_RegisterEx(name, handler, flags, description, usage);
}

// Temporary legacy aliases.
stock bool:OhmyCmd_MigrateYCMD(const name[], flags = 0, const description[] = "", const usage[] = "")
{
    return OMC_MigrateYCMD(name, flags, description, usage);
}

stock bool:OhmyCmd_MigrateZCMD(const name[], flags = 0, const description[] = "", const usage[] = "")
{
    return OMC_MigrateZCMD(name, flags, description, usage);
}

stock bool:OhmyCmd_MigrateLegacy(const name[], const handler[], flags = 0, const description[] = "", const usage[] = "")
{
    return OMC_MigrateLegacy(name, handler, flags, description, usage);
}
