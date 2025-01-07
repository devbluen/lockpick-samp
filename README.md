# 🫧 LOCK-PICK
This is a fully dynamic lockpick include for your SAMP server. You can configure the layout and how it will function. If you encounter any issues or have any questions, please open an issue.

# Dependencies
⚠️ [YSI-Includes](https://github.com/pawn-lang/YSI-Includes)

# Functions
```pawn
CreateLockPick(playerid, const callback[], seconds);
CancelLockPick(playerid);
IsPlayerInLockPick(playerid);
```
### Status
```
E_LOCKPICK_SUCCESS
E_LOCKPICK_ERROR
```

# Usage Example
```pawn
forward lockpick(playerid, E_LOCKPICK_STATUS:status);
public lockpick(playerid, E_LOCKPICK_STATUS:status) {

    if(status == E_LOCKPICK_SUCCESS) SendClientMessage(playerid, -1, "LOCKPICK: Success");
    else if(status == E_LOCKPICK_ERROR) SendClientMessage(playerid, -1, "LOCKPICK: Error");
    return true;
}

CMD:lockpick(playerid, params[]) {

    new time;
    if(sscanf(params, "d", time))
        return true;

    CreateLockPick(playerid, "lockpick", time);
    return true;
}
```

# 📼 Gif
