#if defined _INC_ohmycmd
    #endinput
#endif
#define _INC_ohmycmd

native OhmyCmd_Register(const name[], const handler[], flags = 0);
native OhmyCmd_AddAlias(const name[], const alias[]);
native OhmyCmd_Execute(playerid, const input[]);
native OhmyCmd_Count();
