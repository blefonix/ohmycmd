#if defined _INC_ohmycmd
    #endinput
#endif
#define _INC_ohmycmd

native OhmyCmd_Register(const name[], const handler[], flags = 0);
native OhmyCmd_AddAlias(const name[], const alias[]);
native OhmyCmd_SetDescription(const name[], const text[]);
native OhmyCmd_SetUsage(const name[], const usage[]);
native OhmyCmd_Execute(playerid, const input[]);
native OhmyCmd_Count();

stock bool:OhmyCmd_RegisterEx(const name[], const handler[], flags = 0, const description[] = "", const usage[] = "")
{
    if (!OhmyCmd_Register(name, handler, flags))
    {
        return false;
    }

    if (description[0] != '\0')
    {
        OhmyCmd_SetDescription(name, description);
    }

    if (usage[0] != '\0')
    {
        OhmyCmd_SetUsage(name, usage);
    }

    return true;
}
