
/*

        Status:
            E_LOCKPICK_SUCCESS
            E_LOCKPICK_ERROR

*/



            // Definers
#define        MAX_LOCKPICK_PINS        (8)

            // Includes
#include <..\YSI-Includes\YSI_Coding\y_hooks>

            // Forwards
forward OnLockPick_Tick(playerid);
forward OnLockPick_PinAnimation(playerid);

enum E_LOCKPICK_STATUS {
    E_LOCKPICK_SUCCESS,
    E_LOCKPICK_ERROR
};

static const 
    e_LockPick_Pinos[MAX_LOCKPICK_PINS][2] = {

        /*
            [0] - Pin Limit
            [1] - Pin
        */
        {1, 2}, 
        {3, 4},
        {5, 6},
        {7, 8},
        {9, 10},
        {11, 12},
        {13, 14},
        {15, 16}
    },
    
    // configurations
    updateTicks = 50,
    Float:pinTicks = 1.0,           // steps pin
    Float:dynamicPinSteps = 5.0,    // steps up
    Float:pinDistance = 10.0;       // steps for pin limit 

static enum E_LOCKPICK {
    E_LOCKPICK_CALLBACK[30 + 1],
    E_LOCKPICK_TIMER,
    E_LOCKPICK_TIMER_ANIM,
    E_LOCKPICK_PINTICK,
    E_LOCKPICK_PINTYPE,
    E_LOCKPICK_SECONDS,
    E_LOCKPICK_PROGRESS,
    Float:E_LOCKPICK_POS_Y, // backup y pos
    bool:E_LOCKPICK_PINS[MAX_LOCKPICK_PINS],
    bool:E_LOCKPICK_ANIM_PIN,
    bool:E_LOCKPICK_BACKWARD
};

static enum E_TEXTDRAW_DATA {
    E_TEXTDRAW_DATA_TEXTO[68 + 1],
    Float:E_TEXTDRAW_DATA_POS[2],
    Float:E_TEXTDRAW_DATA_SIZE[2],
    E_TEXTDRAW_DATA_ALIGN,
    E_TEXTDRAW_DATA_COLOR,
    E_TEXTDRAW_DATA_SHADOW,
    E_TEXTDRAW_DATA_OUTLINE,
    E_TEXTDRAW_DATA_BACKCOLOR,  // BackgroundColor
    E_TEXTDRAW_DATA_FONT,
    bool:E_TEXTDRAW_DATA_PROPOR  // Proportional
};

static
    e_Lockpick[MAX_PLAYERS][E_LOCKPICK],
    e_TextdrawData[MAX_PLAYERS][MAX_PLAYER_TEXT_DRAWS][E_TEXTDRAW_DATA],
    PlayerText:tLockpick[MAX_PLAYERS][24];

hook OnPlayerDisconnect(playerid, reason) {

    Lockpick_Reset(playerid);
    return Y_HOOKS_CONTINUE_RETURN_1;
}

hook OnPlayerKeyStateChange(playerid, KEY:newkeys, KEY:oldkeys) {

    if(newkeys == KEY_YES && IsPlayerInLockPick(playerid) && !e_Lockpick[playerid][E_LOCKPICK_TIMER_ANIM]) {
        Lockpick_CheckPins(playerid);
        return Y_HOOKS_BREAK_RETURN_1;
    }
    return Y_HOOKS_CONTINUE_RETURN_1;
}



/*

                    ooooooooo.   ooooo     ooo oooooooooo.  ooooo        ooooo   .oooooo.   
                    `888   `Y88. `888'     `8' `888'   `Y8b `888'        `888'  d8P'  `Y8b  
                    888   .d88'  888       8   888     888  888          888  888          
                    888ooo88P'   888       8   888oooo888'  888          888  888          
                    888          888       8   888    `88b  888          888  888          
                    888          `88.    .8'   888    .88P  888       o  888  `88b    ooo  
                    o888o           `YbodP'    o888bood8P'  o888ooooood8 o888o  `Y8bood8P'  

*/

public OnLockPick_Tick(playerid) {

    // progress
    e_Lockpick[playerid][E_LOCKPICK_SECONDS]++;
    PlayerTextDrawTextSize(playerid, tLockpick[playerid][21], 119.000 - ((119.000 / e_Lockpick[playerid][E_LOCKPICK_PROGRESS]) * e_Lockpick[playerid][E_LOCKPICK_SECONDS]), 7.0);
    PlayerTextDrawShow(playerid, tLockpick[playerid][21]);

    if(e_Lockpick[playerid][E_LOCKPICK_SECONDS] >= e_Lockpick[playerid][E_LOCKPICK_PROGRESS]) {
        Lockpick_Response(playerid, E_LOCKPICK_ERROR);
        return true;
    }

    // wait for anim pin
    if(e_Lockpick[playerid][E_LOCKPICK_ANIM_PIN])
        return true;

    new Float:position[2];
    TextDraw_GetPosition(playerid, tLockpick[playerid][23], position[0], position[1]);

    if(e_Lockpick[playerid][E_LOCKPICK_BACKWARD]) position[0] -= pinTicks;
    else position[0] += pinTicks;

    new Float:mainBoxPos, Float:mainBoxSize;
    TextDraw_GetPosition(playerid, tLockpick[playerid][0], mainBoxPos);
    TextDraw_GetTextSize(playerid, tLockpick[playerid][0], mainBoxSize);
    
    if(position[0] <= mainBoxPos) e_Lockpick[playerid][E_LOCKPICK_BACKWARD] = false;                           // min limit
    else if(position[0] >= (mainBoxPos + mainBoxSize)) e_Lockpick[playerid][E_LOCKPICK_BACKWARD] = true;       // max limit

    SetLockPick_Pin(playerid, position[0], position[1]);
    return true;
}

public OnLockPick_PinAnimation(playerid) {

    new Float:position[2], Float:position_y = e_Lockpick[playerid][E_LOCKPICK_POS_Y];
    TextDraw_GetPosition(playerid, tLockpick[playerid][23], position[0], position[1]);

    switch(e_Lockpick[playerid][E_LOCKPICK_PINTYPE]) {
        case 0: // pin up
        {
            if(position[1] <= (position_y - pinDistance))
                e_Lockpick[playerid][E_LOCKPICK_PINTYPE] = 1;

            position[1] -= pinTicks;
        }
        
        case 1: // checkpin and move
        {
            new pin_index = e_Lockpick[playerid][E_LOCKPICK_PINTICK];
            if(pin_index != -1 && !e_Lockpick[playerid][E_LOCKPICK_PINS][pin_index])
            {
                // get positions
                new Float:dynamicPosition[2], Float:dynamicLimitPos[2];
                TextDraw_GetPosition(playerid, tLockpick[playerid][e_LockPick_Pinos[pin_index][0]], dynamicLimitPos[0], dynamicLimitPos[1]);
                TextDraw_GetPosition(playerid, tLockpick[playerid][e_LockPick_Pinos[pin_index][1]], dynamicPosition[0], dynamicPosition[1]);

                // set up pin animation
                dynamicPosition[1] -= dynamicPinSteps;
                TextDraw_SetPosition(playerid, tLockpick[playerid][e_LockPick_Pinos[pin_index][1]], dynamicPosition[0], dynamicPosition[1]);

                e_Lockpick[playerid][E_LOCKPICK_PINS][pin_index] = true;  // set pin finish
            }

            e_Lockpick[playerid][E_LOCKPICK_PINTYPE] = 2;
        }

        case 2: // back pin main
        {
            if(position[1] >= position_y) {
                e_Lockpick[playerid][E_LOCKPICK_PINTYPE] = 3;
                position[1] = position_y;
            }
            else position[1] += pinTicks;
        }

        case 3: // check and continue anims
        {
            KillTimer(e_Lockpick[playerid][E_LOCKPICK_TIMER_ANIM]);
            e_Lockpick[playerid][E_LOCKPICK_TIMER_ANIM] = 0;
            e_Lockpick[playerid][E_LOCKPICK_PINTYPE] = 0;
            e_Lockpick[playerid][E_LOCKPICK_ANIM_PIN] = false;

            new bool:hasPin;
            for(new i = 0; i < MAX_LOCKPICK_PINS; i++) {
                if(e_Lockpick[playerid][E_LOCKPICK_PINS][i])
                    continue;

                hasPin = true;
                break;
            }

            if(!hasPin)
                Lockpick_Response(playerid, E_LOCKPICK_SUCCESS);
        }
    }

    SetLockPick_Pin(playerid, position[0], position[1]);
    return true;
}



/*

                    .oooooo..o ooooooooooooo   .oooooo.     .oooooo.   oooo    oooo  .oooooo..o 
                    d8P'    `Y8 8'   888   `8  d8P'  `Y8b   d8P'  `Y8b  `888   .8P'  d8P'    `Y8 
                    Y88bo.           888      888      888 888           888  d8'    Y88bo.      
                    `"Y8888o.       888      888      888 888           88888[       `"Y8888o.  
                        `"Y88b      888      888      888 888           888`88b.         `"Y88b 
                    oo     .d8P      888      `88b    d88' `88b    ooo   888  `88b.  oo     .d8P 
                    8""88888P'      o888o      `Y8bood8P'   `Y8bood8P'  o888o  o888o 8""88888P'  

*/

stock CreateLockPick(playerid, const callback[], seconds) {

    // Block
    if(IsPlayerInLockPick(playerid))
        return false;

    ShowLockPick(playerid);
    SetLockPick_Pin(playerid, 252.000, 249.000);

    format(e_Lockpick[playerid][E_LOCKPICK_CALLBACK], 30, callback);
    e_Lockpick[playerid][E_LOCKPICK_PROGRESS] = (seconds * 1000) / 100;
    e_Lockpick[playerid][E_LOCKPICK_TIMER] = SetTimerEx("OnLockPick_Tick", updateTicks, true, "i", playerid);
    return true;
}

stock CancelLockPick(playerid) {

    if(!IsPlayerInLockPick(playerid))
        return false;
    
    Lockpick_Reset(playerid);
    return true;
}

stock IsPlayerInLockPick(playerid)
    return e_Lockpick[playerid][E_LOCKPICK_TIMER] != 0;

static stock Lockpick_CheckPins(playerid) {

    new 
        Float:dynamicPosition,
        Float:dynamicPositionSize,
        Float:position[2],
        pinFound = -1;

    e_Lockpick[playerid][E_LOCKPICK_ANIM_PIN] = true;
    TextDraw_GetPosition(playerid, tLockpick[playerid][23], position[0], position[1]);

    for(new i = 0; i < sizeof e_LockPick_Pinos; i++) {
        TextDraw_GetPosition(playerid, tLockpick[playerid][e_LockPick_Pinos[i][1]], dynamicPosition);
        TextDraw_GetTextSize(playerid, tLockpick[playerid][e_LockPick_Pinos[i][1]], dynamicPositionSize);

        if(position[0] >= dynamicPosition && position[0] <= dynamicPosition + dynamicPositionSize) {
            pinFound = i;
            break;
        }
    }

    e_Lockpick[playerid][E_LOCKPICK_PINTYPE] = 0;
    e_Lockpick[playerid][E_LOCKPICK_PINTICK] = pinFound;
    e_Lockpick[playerid][E_LOCKPICK_POS_Y] = position[1];
    e_Lockpick[playerid][E_LOCKPICK_TIMER_ANIM] = SetTimerEx("OnLockPick_PinAnimation", 50, true, "i", playerid);
}

static stock SetLockPick_Pin(playerid, Float:x, Float:y) {

    // Distance between of point pin
    TextDraw_SetPosition(playerid, tLockpick[playerid][22], x - 45.0, y);
    TextDraw_SetPosition(playerid, tLockpick[playerid][23], x, y);
}

static stock Lockpick_Response(playerid, E_LOCKPICK_STATUS:status) {

    new callback[30 + 1];
    strcat(callback, e_Lockpick[playerid][E_LOCKPICK_CALLBACK]);

    Lockpick_Reset(playerid);
    CallLocalFunction(callback, "ii", playerid, _:status);
}

static stock Lockpick_Reset(playerid) {

    if(e_Lockpick[playerid][E_LOCKPICK_TIMER_ANIM])
        KillTimer(e_Lockpick[playerid][E_LOCKPICK_TIMER_ANIM]);    

    if(e_Lockpick[playerid][E_LOCKPICK_TIMER])
        KillTimer(e_Lockpick[playerid][E_LOCKPICK_TIMER]); 

    HideLockPick(playerid);

    static const e_Clear[E_LOCKPICK];
    e_Lockpick[playerid] = e_Clear;
}



/*

                    ooooo              .o.       oooooo   oooo   .oooooo.   ooooo     ooo ooooooooooooo 
                    `888'             .888.       `888.   .8'   d8P'  `Y8b  `888'     `8' 8'   888   `8 
                    888             .8"888.       `888. .8'   888      888  888       8       888      
                    888            .8' `888.       `888.8'    888      888  888       8       888      
                    888           .88ooo8888.       `888'     888      888  888       8       888      
                    888       o  .8'     `888.       888      `88b    d88'  `88.    .8'       888      
                    o888ooooood8 o88o     o8888o     o888o      `Y8bood8P'     `YbodP'        o888o     

*/

static stock ShowLockPick(playerid) {

    tLockpick[playerid][0] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 252.000, 186.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][0], 119.000, 76.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][0], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][0], 303174399);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][0], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][0], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][0], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][0], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][0], true);

    tLockpick[playerid][1] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 270.000, 196.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][1], 6.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][1], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][1], 1211993087);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][1], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][1], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][1], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][1], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][1], true);

    tLockpick[playerid][2] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 270.000, 204.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][2], 6.000, 33.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][2], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][2], 2070474495);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][2], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][2], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][2], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][2], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][2], true);

    tLockpick[playerid][3] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 281.000, 196.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][3], 6.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][3], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][3], 1211993087);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][3], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][3], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][3], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][3], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][3], true);

    tLockpick[playerid][4] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 281.000, 204.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][4], 6.000, 33.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][4], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][4], 2070474495);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][4], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][4], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][4], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][4], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][4], true);

    tLockpick[playerid][5] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 292.000, 196.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][5], 6.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][5], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][5], 1211993087);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][5], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][5], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][5], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][5], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][5], true);

    tLockpick[playerid][6] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 292.000, 204.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][6], 6.000, 33.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][6], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][6], 2070474495);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][6], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][6], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][6], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][6], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][6], true);

    tLockpick[playerid][7] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 303.000, 196.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][7], 6.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][7], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][7], 1211993087);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][7], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][7], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][7], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][7], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][7], true);

    tLockpick[playerid][8] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 303.000, 204.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][8], 6.000, 33.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][8], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][8], 2070474495);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][8], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][8], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][8], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][8], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][8], true);

    tLockpick[playerid][9] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 314.000, 196.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][9], 6.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][9], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][9], 1211993087);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][9], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][9], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][9], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][9], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][9], true);

    tLockpick[playerid][10] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 314.000, 204.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][10], 6.000, 33.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][10], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][10], 2070474495);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][10], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][10], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][10], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][10], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][10], true);

    tLockpick[playerid][11] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 325.000, 196.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][11], 6.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][11], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][11], 1211993087);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][11], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][11], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][11], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][11], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][11], true);

    tLockpick[playerid][12] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 325.000, 204.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][12], 6.000, 33.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][12], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][12], 2070474495);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][12], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][12], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][12], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][12], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][12], true);

    tLockpick[playerid][13] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 336.000, 196.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][13], 6.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][13], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][13], 1211993087);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][13], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][13], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][13], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][13], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][13], true);

    tLockpick[playerid][14] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 336.000, 204.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][14], 6.000, 33.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][14], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][14], 2070474495);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][14], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][14], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][14], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][14], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][14], true);

    tLockpick[playerid][15] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 347.000, 196.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][15], 6.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][15], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][15], 1211993087);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][15], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][15], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][15], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][15], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][15], true);

    tLockpick[playerid][16] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 347.000, 204.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][16], 6.000, 33.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][16], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][16], 2070474495);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][16], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][16], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][16], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][16], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][16], true);

    tLockpick[playerid][17] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 252.000, 160.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][17], 119.000, 24.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][17], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][17], 303174399);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][17], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][17], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][17], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][17], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][17], true);

    tLockpick[playerid][18] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 257.000, 164.000, "LOCKPICK");
    PlayerTextDrawLetterSize(playerid, tLockpick[playerid][18], 0.200, 0.999);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][18], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][18], -1);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][18], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][18], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][18], 150);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][18], TEXT_DRAW_FONT_1);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][18], true);

    tLockpick[playerid][19] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 257.000, 173.000, "Aperte Y para empurrar os pinos para cima");
    PlayerTextDrawLetterSize(playerid, tLockpick[playerid][19], 0.129, 0.699);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][19], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][19], 1768516095);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][19], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][19], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][19], 150);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][19], TEXT_DRAW_FONT_1);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][19], true);

    tLockpick[playerid][20] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 252.000, 264.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][20], 119.000, 7.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][20], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][20], 303174399);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][20], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][20], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][20], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][20], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][20], true);

    tLockpick[playerid][21] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 252.000, 264.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][21], 24.000, 7.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][21], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][21], 1784335871);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][21], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][21], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][21], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][21], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][21], true);

    tLockpick[playerid][22] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 218.000, 249.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][22], 47.000, 3.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][22], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][22], -1);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][22], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][22], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][22], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][22], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][22], true);

    tLockpick[playerid][23] = PlayerText:Lock_CreatePlayerTextDraw(playerid, 263.000, 249.000, "LD_SPAC:white");
    Lock_PlayerTextDrawTextSize(playerid, tLockpick[playerid][23], 2.000, -8.000);
    Lock_PlayerTextDrawAlignment(playerid, tLockpick[playerid][23], 1);
    Lock_PlayerTextDrawColor(playerid, tLockpick[playerid][23], -1);
    Lock_PlayerTextDrawSetShadow(playerid, tLockpick[playerid][23], 0);
    Lock_PlayerTextDrawSetOutline(playerid, tLockpick[playerid][23], 0);
    Lock_PlayerTextDrawBackgColor(playerid, tLockpick[playerid][23], 255);
    Lock_PlayerTextDrawFont(playerid, tLockpick[playerid][23], TEXT_DRAW_FONT_SPRITE_DRAW);
    Lock_PlayerTextDrawProp(playerid, tLockpick[playerid][23], true);

    for(new i = 0; i < sizeof tLockpick[]; i++)
        PlayerTextDrawShow(playerid, tLockpick[playerid][i]);
}

static stock HideLockPick(playerid) {

    for(new i = 0; i < sizeof tLockpick[]; i++) {
        PlayerTextDrawDestroy(playerid, tLockpick[playerid][i]);
        tLockpick[playerid][i] = PlayerText:INVALID_TEXT_DRAW;
    }
}



/*

                    ooo        ooooo       .o.         .oooooo.   ooooooooo.     .oooooo.    .oooooo..o 
                    `88.       .888'      .888.       d8P'  `Y8b  `888   `Y88.  d8P'  `Y8b  d8P'    `Y8 
                    888b     d'888      .8"888.     888           888   .d88' 888      888 Y88bo.      
                    8 Y88. .P  888     .8' `888.    888           888ooo88P'  888      888  `"Y8888o.  
                    8  `888'   888    .88ooo8888.   888           888`88b.    888      888      `"Y88b 
                    8    Y     888   .8'     `888.  `88b    ooo   888  `88b.  `88b    d88' oo     .d8P 
                    o8o        o888o o88o     o8888o  `Y8bood8P'  o888o  o888o  `Y8bood8P'  8""88888P'  

*/

static stock TextDraw_SetPosition(playerid, &PlayerText:playertextid, Float:x, Float:y) {

    if(playertextid == INVALID_PLAYER_TEXT_DRAW)
        return;

    new data[E_TEXTDRAW_DATA];
    data = e_TextdrawData[playerid][playertextid];  // create data backup

    PlayerTextDrawDestroy(playerid, playertextid);  // destroy current textdraw

    new PlayerText:textid = CreatePlayerTextDraw(playerid, x, y, data[E_TEXTDRAW_DATA_TEXTO]);
    PlayerTextDrawTextSize(playerid, textid, data[E_TEXTDRAW_DATA_SIZE][0], data[E_TEXTDRAW_DATA_SIZE][1]);
    PlayerTextDrawAlignment(playerid, textid, TEXT_DRAW_ALIGN:data[E_TEXTDRAW_DATA_ALIGN]);
    PlayerTextDrawColour(playerid, textid, data[E_TEXTDRAW_DATA_COLOR]);
    PlayerTextDrawSetShadow(playerid, textid, data[E_TEXTDRAW_DATA_SHADOW]);
    PlayerTextDrawSetOutline(playerid, textid, data[E_TEXTDRAW_DATA_OUTLINE]);
    PlayerTextDrawBackgroundColour(playerid, textid, data[E_TEXTDRAW_DATA_COLOR]);
    PlayerTextDrawFont(playerid, textid, TEXT_DRAW_FONT:data[E_TEXTDRAW_DATA_FONT]);
    PlayerTextDrawSetProportional(playerid, textid, data[E_TEXTDRAW_DATA_PROPOR]);
    PlayerTextDrawShow(playerid, textid);

    e_TextdrawData[playerid][textid] = data;                        // Rebuild data
    e_TextdrawData[playerid][textid][E_TEXTDRAW_DATA_POS][0] = x;   // Set new offsets
    e_TextdrawData[playerid][textid][E_TEXTDRAW_DATA_POS][1] = y;   // Set new offsets
    playertextid = textid;
}

static stock TextDraw_GetPosition(playerid, PlayerText:playertextid, &Float:x, &Float:y = 0.0) {

    x = e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_POS][0];
    y = e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_POS][1];
}

static stock TextDraw_GetTextSize(playerid, PlayerText:playertextid, &Float:x, &Float:y = 0.0) {

    x = e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_SIZE][0];
    y = e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_SIZE][1];
}

static stock TextDraw_ResetData(playerid, PlayerText:playertextid) {

    static const e_Clear[E_TEXTDRAW_DATA];
    e_TextdrawData[playerid][playertextid] = e_Clear;
}

static stock Lock_CreatePlayerTextDraw(playerid, Float:x, Float:y, const text[]) {
    new PlayerText:playertextid = CreatePlayerTextDraw(playerid, x, y, text);

    TextDraw_ResetData(playerid, playertextid);

    format(e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_TEXTO], 68, text);
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_POS][0] = x;
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_POS][1] = y;
    return _:playertextid;
}

static stock Lock_PlayerTextDrawTextSize(playerid, PlayerText:playertextid, Float:x, Float:y) {
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_SIZE][0] = x;
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_SIZE][1] = y;
    PlayerTextDrawTextSize(playerid, playertextid, x, y);
}

static stock Lock_PlayerTextDrawAlignment(playerid, PlayerText:playertextid, alignment)
{
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_ALIGN] = alignment;
    PlayerTextDrawAlignment(playerid, playertextid, TEXT_DRAW_ALIGN:alignment);
}

static stock Lock_PlayerTextDrawColor(playerid, PlayerText:playertextid, color) {
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_COLOR] = color;
    PlayerTextDrawColour(playerid, playertextid, color);
}

static stock Lock_PlayerTextDrawSetShadow(playerid, PlayerText:playertextid, size) {
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_SHADOW] = size;
    PlayerTextDrawSetShadow(playerid, playertextid, size);
}

static stock Lock_PlayerTextDrawSetOutline(playerid, PlayerText:playertextid, size) {
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_OUTLINE] = size;
    PlayerTextDrawSetOutline(playerid, playertextid, size);
}

static stock Lock_PlayerTextDrawBackgColor(playerid, PlayerText:playertextid, color) {
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_BACKCOLOR] = color;
    PlayerTextDrawBackgroundColour(playerid, playertextid, color);
}

static stock Lock_PlayerTextDrawFont(playerid, PlayerText:playertextid, font) {
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_FONT] = font;
    PlayerTextDrawFont(playerid, playertextid, TEXT_DRAW_FONT:font);
}

static stock Lock_PlayerTextDrawProp(playerid, PlayerText:playertextid, bool:set) {
    e_TextdrawData[playerid][playertextid][E_TEXTDRAW_DATA_PROPOR] = set;
    PlayerTextDrawSetProportional(playerid, playertextid, set);
}
