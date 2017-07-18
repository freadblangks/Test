#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "Chat.h"
#include "Player.h"
#include "Creature.h"
#include "ObjectGuid.h"
#include "Group.h"
#include "GossipDef.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "Util.h"
#include "ScriptedCreature.h"

using namespace std;

std::vector<std::string> Path_IconSelectGameType =
{
    "INV_Misc_Grouplooking",
    "T_Roboticon"
};

std::vector<std::string> Path_Icon =
{
    "INV_Misc_Toy_04",
    "INV_Misc_Toy_02",
    "INV_Jewelry_TrinketPVP_02",
    "INV_Jewelry_TrinketPVP_01",
    "INV_Misc_Tournaments_Banner_Orc",
    "INV_Misc_Tournaments_Banner_Human",
    "INV_Bannerpvp_01",
    "INV_Bannerpvp_02",
    "Achievement_Reputation_03",
    "Achievement_Reputation_01",
    "Ability_Mount_Cockatricemountelite_purple",
    "Ability_Mount_Cockatricemountelite_blue",
    "Ability_Mount_Rocketmount",
    "Ability_Mount_Rocketmountblue",
    "INV_Chest_Cloth_55",
    "INV_Chest_Cloth_52",
    "Achievement_Bg_Interruptx_Flagcapture_Attempts_1game",
    "Achievement_Bg_Interruptx_Flagcapture_Attempts",
    "INV_Drink_Waterskin_09",
    "INV_Drink_Waterskin_01",
    "INV_Jewelcrafting_Crimsoncrab",
    "INV_Jewelcrafting_Azurecrab",
    "INV_Jewelcrafting_Dragonseye05",
    "INV_Jewelcrafting_Dragonseye04",
    "INV_Misc_Bag_07_Red",
    "INV_Misc_Bag_07_Blue",
    "INV_Misc_Rabbit_2",
    "INV_Misc_Rabbit",
    "INV_Pet_Pinkmurlocegg",
    "INV_Pet_Bluemurlocegg",
    "INV_Misc_Questionmark"
};

std::string COLOR_WHITE = "|cffffffff";
std::string COLOR_RED = "|cffFF0000";
std::string COLOR_BLUE = "|cff6666FF";
std::string COLOR_GREEN = "|cff00cc00";

std::string Grid_Letters = "ABC";

struct NpcTicTacToeInfo
{
    std::string namePlayer01;
    ObjectGuid guidPlayer01;
    std::string namePlayer02;
    ObjectGuid guidPlayer02;
    uint32 themeTypeSelected;
    std::string messageGameTypeSelected;
    uint32 gameTypeSelected;
    ObjectGuid nextGuidPlayerMove;
    uint32 nextIdPlayerMove;
    uint32 gameStatus;
    uint32 gameGrid[3][3];
    uint32 AIgameGrid[3][3];
    bool needRefreshGossipP1;
    bool needRefreshGossipP2;
    uint32 computerStateText;
};

// NpcTicTacToeData Map
std::map<ObjectGuid, NpcTicTacToeInfo> NpcTicTacToeData;

enum EventsTicTactToe
{
    WAIT_HUMAN_PLAYER_01 = 1,
    WAIT_HUMAN_PLAYER_02 = 2,
    WAIT_COMPUTER_PLAYER_TEXT_01 = 3,
    WAIT_COMPUTER_PLAYER_TEXT_02 = 4,
    WAIT_COMPUTER_PLAYER_TEXT_03 = 5,
    WAIT_COMPUTER_PLAYER_TEXT_04 = 6
};

enum GossipMenuTicTacToe
{
    GOSSIP_MENU_TICTACTOE_WELCOME = 150150,
    GOSSIP_MENU_TICTACTOE_EMPTY = 150151,
    GOSSIP_MENU_TICTACTOE_INGAME = 150152,
    GOSSIP_MENU_TICTACTOE_SETUP = 150153,
};

enum MoneyRewardTicTacToe
{
    MONEY_WIN_GAME_HUMAN = 1000000,
    MONEY_LOOSE_GAME_HUMAN = 1000000,
    MONEY_DRAW_GAME_HUMAN = 10000,
    MONEY_WIN_GAME_NPC = 10000,
    MONEY_LOOSE_GAME_NPC = 10000,
    MONEY_DRAW_GAME_NPC = 10000,
    MONEY_WIN_GAME_IA_NPC = 100000,
    MONEY_LOOSE_GAME_IA_NPC = 100000,
    MONEY_DRAW_GAME_IA_NPC = 100000
};

class npc_tic_tac_toe : public CreatureScript
{
public:
    npc_tic_tac_toe() : CreatureScript("npc_tic_tac_toe") { }

    static NpcTicTacToeInfo const* GetNpcTitcTacToeData(ObjectGuid playerGuid)
    {
        std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator itr = NpcTicTacToeData.find(playerGuid);
        if (itr != NpcTicTacToeData.end())
            return &(itr->second);
        return NULL;
    }

    static string GetIconePathBySize(string iconName, uint32 size)
    {
        std::string pathBuilded = "";

        pathBuilded = "|TInterface\\icons\\" + iconName;
        pathBuilded += ":";
        pathBuilded += std::to_string(size);
        pathBuilded += ":";
        pathBuilded += std::to_string(size);
        pathBuilded += "|t";

        return pathBuilded;
    }

    static string GetIconeNameByPlayerTheme(std::vector<std::string> Path_Icon, uint32 playerId, uint32 themeId)
    {
        std::string iconName = "";

        switch (playerId)
        {
            case 1:
                iconName = Path_Icon[themeId * 2 - 2];
                break;
            case 2:
                iconName = Path_Icon[themeId * 2 - 1];
                break;
            default:
                iconName = Path_Icon[(Path_Icon.size() - 1)];
                break;
        }

        return iconName;
    }

    struct npc_tic_tac_toeAI : public ScriptedAI
    {
        npc_tic_tac_toeAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap _events;

        void UpdateAI(uint32 diff) override
        {
            if (_events.Empty())
                return;

            _events.Update(diff);

            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case WAIT_HUMAN_PLAYER_01:
                    case WAIT_COMPUTER_PLAYER_TEXT_04:
                    {
                        if (!NpcTicTacToeData.empty())
                        {
                            // Check for each information if player found as player 01 or 02
                            for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                            {
                                if (i->second.gameStatus >= 5 && i->second.needRefreshGossipP1 == true)
                                {
                                    if (Player* playerFind01 = ObjectAccessor::FindConnectedPlayer(i->second.guidPlayer01))
                                    {
                                        NpcTicTacToeData[i->second.guidPlayer01].needRefreshGossipP1 = false;
                                        npc_tic_tac_toe::OnGossipSelectTicTacToeHuman(playerFind01, me, GOSSIP_SENDER_MAIN, (GOSSIP_ACTION_INFO_DEF + 60), NULL);
                                    }
                                }
                            }
                        }
                    } break;

                    case WAIT_HUMAN_PLAYER_02:
                    {
                        if (!NpcTicTacToeData.empty())
                        {
                            // Check for each information if player found as player 01 or 02
                            for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                            {
                                if (i->second.gameStatus >= 5 && i->second.needRefreshGossipP2 == true)
                                {
                                    if (Player* playerFind02 = ObjectAccessor::FindConnectedPlayer(i->second.guidPlayer02))
                                    {
                                        NpcTicTacToeData[i->second.guidPlayer01].needRefreshGossipP2 = false;
                                        npc_tic_tac_toe::OnGossipSelectTicTacToeHuman(playerFind02, me, GOSSIP_SENDER_MAIN, (GOSSIP_ACTION_INFO_DEF + 60), NULL);
                                    }
                                }
                            }
                        }
                    } break;

                    case WAIT_COMPUTER_PLAYER_TEXT_01:
                    {
                        if (!NpcTicTacToeData.empty())
                        {
                            // Check for each information if player found as player 01 or 02
                            for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                            {
                                if (i->second.gameStatus >= 5 && i->second.computerStateText == 1)
                                {
                                    if (Player* playerFind01 = ObjectAccessor::FindConnectedPlayer(i->second.guidPlayer01))
                                    {
                                        me->Whisper("vamos a pensar en mis movimiento.", LANG_UNIVERSAL, playerFind01, true);
                                    }

                                    NpcTicTacToeData[i->second.guidPlayer01].computerStateText = 2;
                                }
                            }
                        }
                    } break;

                    case WAIT_COMPUTER_PLAYER_TEXT_02:
                    {
                        if (!NpcTicTacToeData.empty())
                        {
                            // Check for each information if player found as player 01 or 02
                            for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                            {
                                if (i->second.gameStatus >= 5 && i->second.computerStateText == 2)
                                {
                                    if (Player* playerFind01 = ObjectAccessor::FindConnectedPlayer(i->second.guidPlayer01))
                                    {
                                        me->Whisper("Vamos a ver lo que puedo jugar ahora.", LANG_UNIVERSAL, playerFind01, true);
                                    }

                                    NpcTicTacToeData[i->second.guidPlayer01].computerStateText = 3;
                                }
                            }
                        }
                    } break;

                    case WAIT_COMPUTER_PLAYER_TEXT_03:
                    {
                        if (!NpcTicTacToeData.empty())
                        {
                            // Check for each information if player found as player 01 or 02
                            for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                            {
                                if (i->second.gameStatus >= 5 && i->second.computerStateText == 3)
                                {
                                    if (Player* playerFind01 = ObjectAccessor::FindConnectedPlayer(i->second.guidPlayer01))
                                    {
                                        NpcTicTacToeData[i->second.guidPlayer01].computerStateText = 4;
                                        npc_tic_tac_toe::OnGossipSelectTicTacToeCumputer(playerFind01, me, GOSSIP_SENDER_MAIN, 0, NULL);
                                    }
                                }
                            }
                        }
                    } break;
                }
            }

        }
    };

    // Check if game win or draw (return true if)
    static bool checkIfGameWinOrDraw(NpcTicTacToeInfo const* NpcTicTacToeGameExist)
    {
        if (!NpcTicTacToeGameExist)
            return false;

        bool winnerFound = false;
        bool gameDraw = true;

        // Check if winner
        for (int i = 0; i < 3; i++){
            if ((NpcTicTacToeGameExist->gameGrid[i][0] != 0 && NpcTicTacToeGameExist->gameGrid[i][0] == NpcTicTacToeGameExist->gameGrid[i][1] && NpcTicTacToeGameExist->gameGrid[i][1] == NpcTicTacToeGameExist->gameGrid[i][2]) ||
                (NpcTicTacToeGameExist->gameGrid[0][i] != 0 && NpcTicTacToeGameExist->gameGrid[0][i] == NpcTicTacToeGameExist->gameGrid[1][i] && NpcTicTacToeGameExist->gameGrid[1][i] == NpcTicTacToeGameExist->gameGrid[2][i]) ||
                (NpcTicTacToeGameExist->gameGrid[0][0] != 0 && NpcTicTacToeGameExist->gameGrid[0][0] == NpcTicTacToeGameExist->gameGrid[1][1] && NpcTicTacToeGameExist->gameGrid[1][1] == NpcTicTacToeGameExist->gameGrid[2][2]) ||
                (NpcTicTacToeGameExist->gameGrid[0][2] != 0 && NpcTicTacToeGameExist->gameGrid[0][2] == NpcTicTacToeGameExist->gameGrid[1][1] && NpcTicTacToeGameExist->gameGrid[1][1] == NpcTicTacToeGameExist->gameGrid[2][0]))
            {
                winnerFound = true;
            }
        }

        // Check if game draw ?
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++){
                if (NpcTicTacToeGameExist->gameGrid[i][j] != 1 && NpcTicTacToeGameExist->gameGrid[i][j] != 2)
                {
                    gameDraw = false;
                }
            }
        }

        // GAME END
        if (winnerFound == true || gameDraw == true)
        {
            // Maj status only the first time
            if (NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameStatus < 20)
            {
                if (winnerFound == true)
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameStatus = 20;

                if (winnerFound == false && gameDraw == true)
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameStatus = 30;
            }

            return true;
        }
        else
        {
            return false;
        }
    }

    // AI SOURCE IDEA
    // http://code.runnable.com/Vnjjxm6IJit89Bpu/tic-tac-toe-unbeatable-ai-for-c%2B%2B
    static char AIcheck_win(NpcTicTacToeInfo const* NpcTicTacToeGameExist)
    {
        // Check horizontal, vertical & diagonal through [0][0]
        if (NpcTicTacToeGameExist->AIgameGrid[0][0] != 0 && (NpcTicTacToeGameExist->AIgameGrid[0][0] == NpcTicTacToeGameExist->AIgameGrid[0][1] && NpcTicTacToeGameExist->AIgameGrid[0][0] == NpcTicTacToeGameExist->AIgameGrid[0][2] ||
            NpcTicTacToeGameExist->AIgameGrid[0][0] == NpcTicTacToeGameExist->AIgameGrid[1][0] && NpcTicTacToeGameExist->AIgameGrid[0][0] == NpcTicTacToeGameExist->AIgameGrid[2][0] ||
            NpcTicTacToeGameExist->AIgameGrid[0][0] == NpcTicTacToeGameExist->AIgameGrid[1][1] && NpcTicTacToeGameExist->AIgameGrid[0][0] == NpcTicTacToeGameExist->AIgameGrid[2][2]))

            return NpcTicTacToeGameExist->AIgameGrid[0][0];

        // Check horizontal, vertical & diagonal through [1][1]
        if (NpcTicTacToeGameExist->AIgameGrid[1][1] != 0 && (NpcTicTacToeGameExist->AIgameGrid[1][1] == NpcTicTacToeGameExist->AIgameGrid[1][0] && NpcTicTacToeGameExist->AIgameGrid[1][1] == NpcTicTacToeGameExist->AIgameGrid[1][2] ||
            NpcTicTacToeGameExist->AIgameGrid[1][1] == NpcTicTacToeGameExist->AIgameGrid[0][1] && NpcTicTacToeGameExist->AIgameGrid[1][1] == NpcTicTacToeGameExist->AIgameGrid[2][1] ||
            NpcTicTacToeGameExist->AIgameGrid[1][1] == NpcTicTacToeGameExist->AIgameGrid[2][0] && NpcTicTacToeGameExist->AIgameGrid[1][1] == NpcTicTacToeGameExist->AIgameGrid[0][2]))

            return NpcTicTacToeGameExist->AIgameGrid[1][1];

        // Check horizontal, vertical & diagonal through [2][2]
        if (NpcTicTacToeGameExist->AIgameGrid[2][2] != 0 && (NpcTicTacToeGameExist->AIgameGrid[2][2] == NpcTicTacToeGameExist->AIgameGrid[0][2] && NpcTicTacToeGameExist->AIgameGrid[2][2] == NpcTicTacToeGameExist->AIgameGrid[1][2] ||
            NpcTicTacToeGameExist->AIgameGrid[2][2] == NpcTicTacToeGameExist->AIgameGrid[2][0] && NpcTicTacToeGameExist->AIgameGrid[2][2] == NpcTicTacToeGameExist->AIgameGrid[2][1]))

            return NpcTicTacToeGameExist->AIgameGrid[2][2];

        return 0;
    }

    static int AIpick_best_move(NpcTicTacToeInfo const* NpcTicTacToeGameExist, int player1, int player2)
    {
        int best_move_score = -9999;
        int best_move_row = -9999;
        int best_move_col = -9999;
        int score_for_this_move = 0;

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (NpcTicTacToeGameExist->AIgameGrid[r][c] == 0) {
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].AIgameGrid[r][c] = player1; //Try test move.
                    score_for_this_move = -(AInegamax(NpcTicTacToeGameExist, player2, player1));
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].AIgameGrid[r][c] = 0; //Put back test move.
                    if (score_for_this_move >= best_move_score) {
                        best_move_score = score_for_this_move;
                        best_move_row = r;
                        best_move_col = c;
                    }
                }
            }
        }

        return (10 * best_move_row + best_move_col);
    }

    static int AInegamax(NpcTicTacToeInfo const* NpcTicTacToeGameExist, int player1, int player2)
    {
        int best_move_score = -9999;
        int score_for_this_move = 0;

        //If player 1 wins, then the score is high (good for player1)
        if (AIcheck_win(NpcTicTacToeGameExist) == player1)
            return 1000;

        //If player 2 loses, then the score is low (bad for player1)
        else if (AIcheck_win(NpcTicTacToeGameExist) == player2)
            return -1000;

        for (int r = 0; r < 3; r++) {
            for (int c = 0; c < 3; c++) {
                if (NpcTicTacToeGameExist->AIgameGrid[r][c] == 0) {
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].AIgameGrid[r][c] = player1; //Try test move.
                    score_for_this_move = -(AInegamax(NpcTicTacToeGameExist, player2, player1));
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].AIgameGrid[r][c] = 0; //Put back test move.
                    if (score_for_this_move >= best_move_score) {
                        best_move_score = score_for_this_move;
                    }
                }
            }
        }

        if (best_move_score == -9999 || best_move_score == 0)
            return 0;
        else if (best_move_score < 0)
            return best_move_score + 1;
        //As the game goes longer, and the recursion goes deeper, the moves near the end are less favorable than in the beginning.
        else if (best_move_score > 0)
            return best_move_score - 1;
        else
            return 0;
    }

    static bool OnGossipSelectTicTacToeCumputer(Player* player, Creature* creature, uint32 sender, uint32 action, NpcTicTacToeInfo const* NpcTicTacToeGameExistFound)
    {
        if (!player)
            return false;

        if (!creature)
            return false;

        player->PlayerTalkClass->ClearMenus();

        NpcTicTacToeInfo const* NpcTicTacToeGameExist = NULL;

        if (!NpcTicTacToeGameExistFound)
        {
            // Found current game for this player
            bool gameExistFound = false;

            if (!NpcTicTacToeData.empty())
            {
                // Check for each information if player found as player 01 or 02
                for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                {
                    if (i->second.guidPlayer01 == player->GetGUID() || i->second.guidPlayer02 == player->GetGUID())
                    {
                        NpcTicTacToeGameExist = GetNpcTitcTacToeData(i->second.guidPlayer01);
                        if (NpcTicTacToeGameExist)
                        {
                            gameExistFound = true;
                        }
                        break;
                    }
                }
            }

            if (gameExistFound == false)
            {
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
        }
        else
        {
            NpcTicTacToeGameExist = NpcTicTacToeGameExistFound;
        }

        uint32 aiFoundI = -1;
        uint32 aiFoundJ = -1;

        // Master AI
        if (NpcTicTacToeGameExist->gameTypeSelected == 3)
        {
            // Copy information in board AI
            for (int i = 0; i < 3; i++){
                for (int j = 0; j < 3; j++){
                    if (NpcTicTacToeGameExist->gameGrid[i][j] == 1)
                    {
                        // board[i][j] = '1';
                        NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].AIgameGrid[i][j] = 1;
                    }
                    else if (NpcTicTacToeGameExist->gameGrid[i][j] == 2)
                    {
                        // board[i][j] = '2';
                        NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].AIgameGrid[i][j] = 2;
                    }
                    else if (NpcTicTacToeGameExist->gameGrid[i][j] == 0)
                    {
                        NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].AIgameGrid[i][j] = 0;
                    }
                }
            }

            int AImove = AIpick_best_move(NpcTicTacToeGameExist, 2, 1);
            aiFoundI = AImove / 10;
            aiFoundJ = AImove % 10;
        }
        else
        {
            // Random AI
            std::vector<std::string> listStringCellEmpty;
            listStringCellEmpty.clear();

            // Find cell empty
            for (int i = 0; i < 3; i++){
                for (int j = 0; j < 3; j++){
                    if (NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameGrid[i][j] == 0)
                    {
                        listStringCellEmpty.push_back(std::to_string(i) + std::to_string(j));
                    }
                }
            }

            if (!listStringCellEmpty.empty())
            {
                uint16 stringCellFoud = urand(0, listStringCellEmpty.size() - 1);
                aiFoundI = atoi(listStringCellEmpty[stringCellFoud].substr(0, 1).c_str());
                aiFoundJ = atoi(listStringCellEmpty[stringCellFoud].substr(1, 1).c_str());
            }
        }

        if (aiFoundI != -1 && aiFoundJ != -1)
        {
            std::string whisperMessage = "Creo que acabo de encontrar mi mejor movimiento en'";
            whisperMessage += Grid_Letters[aiFoundI];
            whisperMessage += std::to_string(aiFoundJ + 1);
            whisperMessage += "'.";

            creature->Whisper(whisperMessage, LANG_UNIVERSAL, player, true);

            NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameGrid[aiFoundI][aiFoundJ] = NpcTicTacToeGameExist->nextIdPlayerMove;

            if (NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextIdPlayerMove == 1)
            {
                NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextGuidPlayerMove = NpcTicTacToeGameExist->guidPlayer02;
                NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextIdPlayerMove = 2;
            }
            else
            {
                NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextGuidPlayerMove = NpcTicTacToeGameExist->guidPlayer01;
                NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextIdPlayerMove = 1;
            }

            // Refresh only gossip for player01
            NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP1 = true;

            ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_COMPUTER_PLAYER_TEXT_04, 500);
            return true;
        }
        else
        {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }
    }

    static bool OnGossipSelectTicTacToeHuman(Player* player, Creature* creature, uint32 sender, uint32 action, NpcTicTacToeInfo const* NpcTicTacToeGameExistFound)
    {
        if (!player)
            return false;

        if (!creature)
            return false;

        player->PlayerTalkClass->ClearMenus();

        NpcTicTacToeInfo const* NpcTicTacToeGameExist = NULL;
        Player* playerFind02 = nullptr;
        Player* playerFind01 = nullptr;
        std::string notificationMessage = "";
        std::string whisperMessage = "";

        if (!NpcTicTacToeGameExistFound)
        {
            // Found current game for this player
            bool gameExistFound = false;

            if (!NpcTicTacToeData.empty())
            {
                // Check for each information if player found as player 01 or 02
                for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                {
                    if (i->second.guidPlayer01 == player->GetGUID() || i->second.guidPlayer02 == player->GetGUID())
                    {
                        NpcTicTacToeGameExist = GetNpcTitcTacToeData(i->second.guidPlayer01);
                        if (NpcTicTacToeGameExist)
                        {
                            gameExistFound = true;
                        }
                        break;
                    }
                }
            }

            if (gameExistFound == false)
            {
                player->CLOSE_GOSSIP_MENU();
                return true;
            }
        }
        else
        {
            NpcTicTacToeGameExist = NpcTicTacToeGameExistFound;
        }

        // Game created (roll who start only the first time)
        if (action == (GOSSIP_ACTION_INFO_DEF + 60) && NpcTicTacToeGameExist->gameStatus == 2)
        {
            // Send notification about game started
            notificationMessage = "Game '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r' Vs '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r' empezado.";
            ChatHandler(player->GetSession()).PSendSysMessage(notificationMessage.c_str());

            // If player game send notification to second player
            if (NpcTicTacToeGameExist->gameTypeSelected == 1)
            {
                // If current player isn't player02
                if (player->GetGUID() != NpcTicTacToeGameExist->guidPlayer02)
                {
                    if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer02))
                    {
                        playerFind02->GetSession()->SendNotification(notificationMessage.c_str());
                        ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage.c_str());
                    }
                }
                else
                {
                    if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer01))
                    {
                        playerFind02->GetSession()->SendNotification(notificationMessage.c_str());
                        ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage.c_str());
                    }
                }
            }

            notificationMessage = "Después de un rollo , jugador'";

            // Roll who start game
            switch (urand(1, 2))
            {
                case 1:
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextGuidPlayerMove = NpcTicTacToeGameExist->guidPlayer01;
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextIdPlayerMove = 1;
                    notificationMessage += "" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r";
                    break;

                case 2:
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextGuidPlayerMove = NpcTicTacToeGameExist->guidPlayer02;
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextIdPlayerMove = 2;
                    notificationMessage += "" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r";
                    break;
            }

            notificationMessage += "'comenzará este nuevo juego en la primera.";

            // Send notification about roll result
            player->GetSession()->SendNotification(notificationMessage.c_str());
            ChatHandler(player->GetSession()).PSendSysMessage(notificationMessage.c_str());

            // If player game send notification to second player
            if (NpcTicTacToeGameExist->gameTypeSelected == 1)
            {
                // If current player isn't player02
                if (player->GetGUID() != NpcTicTacToeGameExist->guidPlayer02)
                {
                    if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer02))
                    {
                        playerFind02->GetSession()->SendNotification(notificationMessage.c_str());
                        ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage.c_str());
                    }
                }
                else
                {
                    if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer01))
                    {
                        playerFind02->GetSession()->SendNotification(notificationMessage.c_str());
                        ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage.c_str());
                    }
                }
            }
            else
            {
                if (NpcTicTacToeGameExist->nextIdPlayerMove == 1)
                {
                    creature->Whisper("Tiene suerte con la rollo, esperar a ver qué va a hacer saber.", LANG_UNIVERSAL, player, true);
                }
                else
                {
                    creature->Whisper("Comienzo a ganar este partido antes de que comience , no llores.", LANG_UNIVERSAL, player, true);
                    action = GOSSIP_ACTION_INFO_DEF + 76;
                }
            }

            NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameStatus = 5;

            // Refresh for player02 launch if find (so only human game)
            if (playerFind02)
            {
                // If current player isn't player02
                if (player->GetGUID() != NpcTicTacToeGameExist->guidPlayer02)
                {
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP2 = true;
                    ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_HUMAN_PLAYER_02, 500);
                }
                else
                {
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP1 = true;
                    ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_HUMAN_PLAYER_01, 500);
                }
            }
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Game '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r' Vs '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r' Comienza.\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "          1                 2                 3", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 63);

        std::string path_icon_string_display = "";
        for (int i(0); i < 3; i++){
            path_icon_string_display = " ";
            for (int j(0); j < 3; j++){
                path_icon_string_display += " " + npc_tic_tac_toe::GetIconePathBySize(npc_tic_tac_toe::GetIconeNameByPlayerTheme(Path_Icon, NpcTicTacToeGameExist->gameGrid[i][j], NpcTicTacToeGameExist->themeTypeSelected), 75);
            }
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, path_icon_string_display, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 63);
        }

        if (checkIfGameWinOrDraw(NpcTicTacToeGameExist) == false)
        {
            // Computer game
            if (NpcTicTacToeGameExist->gameTypeSelected != 1 && NpcTicTacToeGameExist->nextIdPlayerMove == 2)
            {
                // Only if action launch by last player01 move
                if (action == (GOSSIP_ACTION_INFO_DEF + 76))
                {
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].computerStateText = 1;
                    ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_COMPUTER_PLAYER_TEXT_01, 1000);
                    ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_COMPUTER_PLAYER_TEXT_02, 3000);
                    ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_COMPUTER_PLAYER_TEXT_03, 5000);
                }

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "\nsegundo jugador '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r' playing |TInterface\\icons\\Spell_Holy_Borrowedtime:45|t.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
            }
            // Next player move
            else
            {
                // If current player talking is the next who should play
                if (NpcTicTacToeGameExist->nextGuidPlayerMove == player->GetGUID())
                {
                    // Send notification message
                    if (action == (GOSSIP_ACTION_INFO_DEF + 63))
                    {
                        notificationMessage = "\n\nEs necesario hacer clic en : Seleccione celda para utilizar su " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, NpcTicTacToeGameExist->nextIdPlayerMove, NpcTicTacToeGameExist->themeTypeSelected), 45) + ".";
                        player->GetSession()->SendNotification(notificationMessage.c_str());
                    }

                    // Send notification and gossip details for wrong cell
                    if (action == (GOSSIP_ACTION_INFO_DEF + 67))
                    {
                        player->GetSession()->SendNotification("\nel número de celda equivocado, tendrá que utilizar (A1,A2,A3,B1,B2,B3,C1,C2,C3).");
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "\nel número de celda equivocado, tendrá que utilizar(A1,A2,A3,B1,B2,B3,C1,C2,C3).", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 63);

                    }
                    else if (action == (GOSSIP_ACTION_INFO_DEF + 68))
                    {
                        player->GetSession()->SendNotification("\nel número de celda mal, esta celda ya se juega.");
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "\nel número de celda mal, esta celda ya se juega.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 63);

                    }

                    player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_TALK, "\nSeleccionar celda para utilizar su" + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, NpcTicTacToeGameExist->nextIdPlayerMove, NpcTicTacToeGameExist->themeTypeSelected), 40) + ".", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 75, "Write a cell name (for exemple : a1)", 0, true);
                }
                else
                {
                    if (NpcTicTacToeGameExist->nextIdPlayerMove == 1)
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "\nEl primer jugador '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r' playing |TInterface\\icons\\Spell_Holy_Borrowedtime:45|t.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
                    }
                    else
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "\nEl Segundo jugador '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r' playing |TInterface\\icons\\Spell_Holy_Borrowedtime:45|t.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
                    }
                }
            }
        }
        else
        {
            // Game win
            if (NpcTicTacToeGameExist->gameStatus == 20 || NpcTicTacToeGameExist->gameStatus == 21)
            {
                // Gossip win message
                if (NpcTicTacToeGameExist->nextIdPlayerMove == 2)
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "\nlotería de premio por el primer jugador '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r'.\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
                }
                else
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "\nlotería de premio por el segundo jugador'" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r'.\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
                }

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "Borrar este juego ?\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

                // REWARD GIVE only the first time
                if (NpcTicTacToeGameExist->gameStatus == 20)
                {
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameStatus = 21;

                    // Game Type player
                    if (NpcTicTacToeGameExist->gameTypeSelected == 1)
                    {
                        // Player01 win (player02 loose)
                        if (NpcTicTacToeGameExist->nextIdPlayerMove == 2)
                        {
                            notificationMessage = "Este juego se gana por el primer jugador '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r'.";

                            if (playerFind01 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer01))
                            {
                                ChatHandler(playerFind01->GetSession()).PSendSysMessage(notificationMessage.c_str());
                                creature->Whisper("Buen juego , usted es mejor que este humano ;) !", LANG_UNIVERSAL, playerFind01, true);

                                // REWARD GIVE ? (human game - player01 WIN - player02 LOOSE)
                                // Give 100 PO to player01
                                playerFind01->ModifyMoney(MONEY_WIN_GAME_HUMAN);
                            }

                            if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer02))
                            {
                                ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage.c_str());
                                creature->Whisper("Usted puede tratar de hacer lo mejor posible en un próximo juego !", LANG_UNIVERSAL, playerFind02, true);

                                // REWARD GIVE ? (human game - player01 WIN - player02 LOOSE)
                                // Take 100 PO from player02
                                if (playerFind02->GetMoney() >= MONEY_LOOSE_GAME_HUMAN)
                                    playerFind02->ModifyMoney(-MONEY_LOOSE_GAME_HUMAN);
                            }
                        }
                        else
                        {
                            notificationMessage = "Este juego se gana por el segundo jugador '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r'.";

                            if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer02))
                            {
                                ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage.c_str());
                                creature->Whisper("Buen juego , usted es mejor que este humano;) !", LANG_UNIVERSAL, playerFind02, true);

                                // REWARD GIVE ? (human game - player01 LOOSE - player02 WIN)
                                // Give 100 PO to player02
                                playerFind02->ModifyMoney(MONEY_WIN_GAME_HUMAN);
                            }

                            if (playerFind01 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer01))
                            {
                                ChatHandler(playerFind01->GetSession()).PSendSysMessage(notificationMessage.c_str());
                                creature->Whisper("Usted puede tratar de hacer lo mejor posible en un próximo juego !", LANG_UNIVERSAL, playerFind01, true);

                                // REWARD GIVE ? (human game - player01 LOOSE - player02 WIN)
                                // Take 100 PO from player01
                                if (playerFind01->GetMoney() >= MONEY_LOOSE_GAME_HUMAN)
                                    playerFind01->ModifyMoney(-MONEY_LOOSE_GAME_HUMAN);
                            }
                        }
                    }
                    else if (NpcTicTacToeGameExist->gameTypeSelected == 2)
                    {
                        // Player01 win (computer loose)
                        if (NpcTicTacToeGameExist->nextIdPlayerMove == 2)
                        {
                            notificationMessage = "Este juego se gana por el primer jugador '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r'.";
                            whisperMessage = "Buen juego , usted es mejor que un NPC;) !";

                            // REWARD GIVE ? (computer game - player01 WIN - computer LOOSE)
                            // Give 1 PO
                            player->ModifyMoney(MONEY_WIN_GAME_NPC);
                        }
                        else
                        {
                            notificationMessage = "Este juego se gana por el segundo jugador '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer01 + "|r'.";
                            whisperMessage = "Usted puede tratar de hacer lo mejor posible en un próximo juego , sólo soy un NPC ;) !";

                            // REWARD GIVE ? (computer game - player02 (computer) WIN - player01 LOOSE)
                            // Take 1 PO from player01
                            if (player->GetMoney() >= MONEY_LOOSE_GAME_NPC)
                                player->ModifyMoney(-MONEY_LOOSE_GAME_NPC);
                        }

                        ChatHandler(player->GetSession()).PSendSysMessage(notificationMessage.c_str());
                        creature->Whisper(whisperMessage, LANG_UNIVERSAL, player, true);
                    }
                    else if (NpcTicTacToeGameExist->gameTypeSelected == 3)
                    {
                        // Player01 win (master computer loose)
                        if (NpcTicTacToeGameExist->nextIdPlayerMove == 2)
                        {
                            notificationMessage = "Este juego se gana por el primer jugador '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r'.";
                            whisperMessage = "Buen juego , usted es mejor que un maestro de los NPC ;) !";

                            // REWARD GIVE ? (master computer game - player01 WIN - master computer LOOSE)
                            // Give 10 PO
                            player->ModifyMoney(MONEY_WIN_GAME_IA_NPC);
                        }
                        else
                        {
                            notificationMessage = "Este juego es gana por el segundo jugador '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer01 + "|r'.";
                            whisperMessage = "Usted puede tratar de hacer lo mejor posible en un próximo juego , sólo soy un maestro de los NPC ;) !";

                            // REWARD GIVE ? (master computer game - player02 (master computer) WIN - player01 LOOSE)
                            // Take 10 PO from player01
                            if (player->GetMoney() >= MONEY_LOOSE_GAME_IA_NPC)
                                player->ModifyMoney(-MONEY_LOOSE_GAME_IA_NPC);
                        }

                        ChatHandler(player->GetSession()).PSendSysMessage(notificationMessage.c_str());
                        creature->Whisper(whisperMessage, LANG_UNIVERSAL, player, true);
                    }

                    // Launch gossip refresh only for human game player02
                    if (NpcTicTacToeGameExist->gameTypeSelected == 1)
                    {
                        NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP2 = true;
                        ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_HUMAN_PLAYER_02, 500);
                    }

                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP1 = true;
                    ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_HUMAN_PLAYER_01, 500);
                }
            }
            else
            {
                // Gossip draw message
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "\nEste juego es un empate ,no hay un ganador\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "Borrar este juego ?\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

                // REWARD GIVE only the first time
                if (NpcTicTacToeGameExist->gameStatus == 30)
                {
                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameStatus = 31;

                    // Game Type player
                    if (NpcTicTacToeGameExist->gameTypeSelected == 1)
                    {
                        // DRAW
                        if (NpcTicTacToeGameExist->nextIdPlayerMove == 1 || NpcTicTacToeGameExist->nextIdPlayerMove == 2)
                        {
                            if (playerFind01 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer01))
                            {
                                ChatHandler(playerFind01->GetSession()).PSendSysMessage("Este juego es un empate.");
                                creature->Whisper("Buen juego , pero no suficiente  para uno de ustedes puede ganar !", LANG_UNIVERSAL, playerFind01, true);

                                // REWARD GIVE ? (human game - DRAW)
                                // Give 1 PO to player01
                                playerFind01->ModifyMoney(MONEY_DRAW_GAME_HUMAN);
                            }

                            if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer02))
                            {
                                ChatHandler(playerFind02->GetSession()).PSendSysMessage("Este juego es un empate .");
                                creature->Whisper("Buen juego , pero no suficiente  para uno de ustedes puede ganar!", LANG_UNIVERSAL, playerFind02, true);

                                // REWARD GIVE ? (human game - DRAW)
                                // Give 1 PO to player02
                                playerFind02->ModifyMoney(MONEY_DRAW_GAME_HUMAN);
                            }
                        }
                    }
                    else if (NpcTicTacToeGameExist->gameTypeSelected == 2)
                    {
                        // DRAW
                        if (NpcTicTacToeGameExist->nextIdPlayerMove == 1 || NpcTicTacToeGameExist->nextIdPlayerMove == 2)
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("Este juego es un empate ..");
                            creature->Whisper("Buen juego , usted es un NPC ;) ?", LANG_UNIVERSAL, player, true);

                            // REWARD GIVE ? (computer game - DRAW)
                            // Give 1 PO
                            player->ModifyMoney(MONEY_DRAW_GAME_NPC);
                        }
                    }
                    else if (NpcTicTacToeGameExist->gameTypeSelected == 3)
                    {
                        // DRAW
                        if (NpcTicTacToeGameExist->nextIdPlayerMove == 1 || NpcTicTacToeGameExist->nextIdPlayerMove == 2)
                        {
                            ChatHandler(player->GetSession()).PSendSysMessage("Este juego es un empate.");
                            creature->Whisper("Buen juego , usted es un maestro de los NPC ;) ?", LANG_UNIVERSAL, player, true);

                            // REWARD GIVE ? (master computer game - DRAW)
                            // Give 10 PO
                            player->ModifyMoney(MONEY_DRAW_GAME_IA_NPC);
                        }
                    }

                    // Launch gossip refresh only for human game player02
                    if (NpcTicTacToeGameExist->gameTypeSelected == 1)
                    {
                        NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP2 = true;
                        ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_HUMAN_PLAYER_02, 500);
                    }

                    NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP1 = true;
                    ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_HUMAN_PLAYER_01, 500);
                }
            }
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "\nMain menu", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        player->SEND_GOSSIP_MENU(GOSSIP_MENU_TICTACTOE_INGAME, creature->GetGUID());
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!player)
            return false;

        if (!creature)
            return false;

        player->PlayerTalkClass->ClearMenus();

        bool gameExistFound = false;

        if (!NpcTicTacToeData.empty())
        {
            // Check for each information if player found as player 01 or 02
            for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
            {
                if (i->second.guidPlayer01 == player->GetGUID() || i->second.guidPlayer02 == player->GetGUID())
                {
                    gameExistFound = true;

                    // If theme already selected (only the player who start build the game can continue)
                    if (i->second.gameStatus == 1 && i->second.guidPlayer01 == player->GetGUID())
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "Usted ya está en una configuración del juego.\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 20);
                    }
                    else if (i->second.gameStatus == 2 && i->second.guidPlayer01 == player->GetGUID())
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "Ya estás en un juego creado .\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                    }
                    else if (i->second.gameStatus < 2 && i->second.guidPlayer02 == player->GetGUID() && i->second.gameTypeSelected == 1)
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "Usted ya está en una configuración del juego (create by '" + COLOR_RED + "" + i->second.namePlayer01 + "|r').\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                    }
                    else if (i->second.gameStatus == 2 && i->second.guidPlayer02 == player->GetGUID() && i->second.gameTypeSelected == 1)
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "Ya estás en un juego creado por '" + COLOR_RED + "" + i->second.namePlayer01 + "|r', unirse a esto ?\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                    }
                    else if (i->second.gameStatus >= 5 && i->second.gameStatus < 20)
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "Ya estás en un juego existentee '" + COLOR_RED + "" + i->second.namePlayer01 + "|r' Vs '" + COLOR_BLUE + "" + i->second.namePlayer02 + "|r', unirse a esto ?\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
                    }
                    else if (i->second.gameStatus >= 20)
                    {
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, "Este juego'" + COLOR_RED + "" + i->second.namePlayer01 + "|r' Vs '" + COLOR_BLUE + "" + i->second.namePlayer02 + "|r' ya está hecho.\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);
                    }

                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TABARD, "Borrar el juego existente ?\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    break;
                }
            }
        }

        // No game found for current player (as player01 or player02
        if (gameExistFound == false)
        {
            // If no game already started
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "Sin juego ya existe con usted, para empezar uno nuevo que necesita primero elige un tema.\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15);
        }

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, "\nMostrar reglas", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
        player->SEND_GOSSIP_MENU(GOSSIP_MENU_TICTACTOE_WELCOME, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        if (!player)
            return false;

        if (!creature)
            return false;

        player->PlayerTalkClass->ClearMenus();

        // Found current game for this player
        NpcTicTacToeInfo const* NpcTicTacToeGameExist = NULL;
        bool gameExistFound = false;

        if (!NpcTicTacToeData.empty())
        {
            // Check for each information if player found as player 01 or 02
            for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
            {
                if (i->second.guidPlayer01 == player->GetGUID() || i->second.guidPlayer02 == player->GetGUID())
                {
                    NpcTicTacToeGameExist = GetNpcTitcTacToeData(i->second.guidPlayer01);
                    if (NpcTicTacToeGameExist)
                    {
                        gameExistFound = true;
                    }
                    break;
                }
            }
        }

        uint32 themeTypeSelected = 0;

        switch (action)
        {
            // Gossip Hello welcome
            case GOSSIP_ACTION_INFO_DEF + 1:
            {
                OnGossipHello(player, creature);
                return true;
            } break;

            // Erase game
            case GOSSIP_ACTION_INFO_DEF + 3:
            {
                if (gameExistFound == false)
                {
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }

                player->ADD_GOSSIP_ITEM_EXTENDED(GOSSIP_ICON_TALK, "Tiene un juego existente '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r' Vs '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r', Qué desea borrarlo ?\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4, "Estás seguro ?", 0, false);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "\nMain menu", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(GOSSIP_MENU_TICTACTOE_INGAME, creature->GetGUID());
            } break;

            // Erase game CONFIRM
            case GOSSIP_ACTION_INFO_DEF + 4:
            {
                if (gameExistFound == false)
                {
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }

                std::string notificationMessage01 = "Game '" + NpcTicTacToeGameExist->namePlayer01 + "' Vs '" + NpcTicTacToeGameExist->namePlayer02 + "' deleted ";
                std::string notificationMessage02 = "Game '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r' Vs '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r' deleted ";

                std::string notificationColor = (player->GetGUID() == NpcTicTacToeGameExist->guidPlayer01) ? "cffFF0000" : "cff6666FF";
                notificationMessage01 += "by '|" + notificationColor + player->GetName() + "|r'.";
                notificationMessage02 += "by '|" + notificationColor + player->GetName() + "|r'.";

                player->GetSession()->SendNotification(notificationMessage01.c_str());
                ChatHandler(player->GetSession()).PSendSysMessage(notificationMessage02.c_str());

                Player* playerFind02 = nullptr;

                // If player game send notification to second player
                if (NpcTicTacToeGameExist->gameTypeSelected == 1)
                {
                    // If player 01 talking, find player 02
                    if (player->GetGUID() == NpcTicTacToeGameExist->guidPlayer01)
                    {
                        if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer02))
                        {
                            playerFind02->GetSession()->SendNotification(notificationMessage01.c_str());
                            ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage02.c_str());
                        }
                    }
                    else if (player->GetGUID() == NpcTicTacToeGameExist->guidPlayer02)
                    {
                        if (playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer01))
                        {
                            playerFind02->GetSession()->SendNotification(notificationMessage01.c_str());
                            ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage02.c_str());
                        }
                    }
                }

                NpcTicTacToeData.erase(NpcTicTacToeGameExist->guidPlayer01);
                OnGossipHello(player, creature);

                if (playerFind02)
                    OnGossipHello(playerFind02, creature);
            } break;

            // Rules and exemmple game win
            case GOSSIP_ACTION_INFO_DEF + 10:
            {
                // Select a random theme and player win
                uint32 themeExemple = urand(1, 8);
                uint32 winPlayerExemple01 = urand(1, 2);
                uint32 winPlayerExemple02 = (winPlayerExemple01 == 1) ? 2 : 1;

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "reglas de juego de tres en raya : El jugador que consigue colocar tres de sus marcas en una" + COLOR_GREEN + "horizontal|r, " + COLOR_GREEN + "vertical|r, or " + COLOR_GREEN + "diagonal|r fila , gana el juego . Las líneas son letras ( ABC) y las columnas son los números ( 123 ).\nSe Trata de un Ejemplo de loteria ganado por " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 45) + ":\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "         1                2                3", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);

                switch (urand(0,2))
                {
                    case 0:
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        break;
                    case 1:
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        break;
                    case 2:
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_INTERACT_1, "  " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple02, themeExemple), 70) + " " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, winPlayerExemple01, themeExemple), 70) + "", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);
                        break;
                }

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "\nMain menu", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(GOSSIP_MENU_TICTACTOE_EMPTY, creature->GetGUID());
            } break;

            // Choose theme
            case GOSSIP_ACTION_INFO_DEF + 15:
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "Seleccione el tema que desea jugar con:\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 15);

                for (std::size_t i(1); i <= ((Path_Icon.size() - 1) / 2); ++i)
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "   " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, 1, i), 70) + " Vs " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, 2, i), 70), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 20 + i);
                }

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "\nMain menu", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(GOSSIP_MENU_TICTACTOE_SETUP, creature->GetGUID());
            } break;

            // Theme selected, choose game type now
            // Theme already selected before and save now
            case GOSSIP_ACTION_INFO_DEF + 20:
            case GOSSIP_ACTION_INFO_DEF + 21:
            case GOSSIP_ACTION_INFO_DEF + 22:
            case GOSSIP_ACTION_INFO_DEF + 23:
            case GOSSIP_ACTION_INFO_DEF + 24:
            case GOSSIP_ACTION_INFO_DEF + 25:
            case GOSSIP_ACTION_INFO_DEF + 26:
            case GOSSIP_ACTION_INFO_DEF + 27:
            case GOSSIP_ACTION_INFO_DEF + 28:
            case GOSSIP_ACTION_INFO_DEF + 29:
            case GOSSIP_ACTION_INFO_DEF + 30:
            case GOSSIP_ACTION_INFO_DEF + 31:
            case GOSSIP_ACTION_INFO_DEF + 32:
            case GOSSIP_ACTION_INFO_DEF + 33:
            case GOSSIP_ACTION_INFO_DEF + 34:
            case GOSSIP_ACTION_INFO_DEF + 35:
            case GOSSIP_ACTION_INFO_DEF + 36:
            {
                // Check if not already in create game (check done in GossipHello)
                // OK

                uint32 gossipActionthemeTypeSelected = 0;

                if (action != (GOSSIP_ACTION_INFO_DEF + 20))
                {
                    themeTypeSelected = action - GOSSIP_ACTION_INFO_DEF - 20;
                    gossipActionthemeTypeSelected = action - GOSSIP_ACTION_INFO_DEF;

                    NpcTicTacToeData[player->GetGUID()].namePlayer01 = player->GetName().c_str();
                    NpcTicTacToeData[player->GetGUID()].guidPlayer01 = player->GetGUID();
                    NpcTicTacToeData[player->GetGUID()].themeTypeSelected = themeTypeSelected;
                    NpcTicTacToeData[player->GetGUID()].gameStatus = 1;
                }
                else
                {
                    gossipActionthemeTypeSelected = 20;
                }

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "Theme " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, 1, NpcTicTacToeData[player->GetGUID()].themeTypeSelected), 60) + " Vs " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, 2, NpcTicTacToeData[player->GetGUID()].themeTypeSelected), 60) + " selected.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + gossipActionthemeTypeSelected);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "\nYou need now to select your adversary :\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + gossipActionthemeTypeSelected);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "|TInterface\\icons\\" + Path_IconSelectGameType[0] + ":45|t Take groupmate", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 51);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "|TInterface\\icons\\" + Path_IconSelectGameType[0] + ":45|t Take your target", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 52);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "|TInterface\\icons\\" + Path_IconSelectGameType[1] + ":45|t Take NPC (random move)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 53);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "|TInterface\\icons\\" + Path_IconSelectGameType[1] + ":45|t Take NPC (AI)", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 54);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "\nMain menu", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(GOSSIP_MENU_TICTACTOE_SETUP, creature->GetGUID());
            } break;

            // Theme selected, game type selected                
            case GOSSIP_ACTION_INFO_DEF + 51:
            case GOSSIP_ACTION_INFO_DEF + 52:
            case GOSSIP_ACTION_INFO_DEF + 53:
            case GOSSIP_ACTION_INFO_DEF + 54:
            {
                // Check if not already in create game (check done in GossipHello)
                // OK

                if (gameExistFound == false)
                {
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }

                // Can be a player02 too in field
                // string namePlayer01 = player->GetName().c_str();
                ObjectGuid guidPlayer01 = player->GetGUID();

                string namePlayer02 = "";
                ObjectGuid guidPlayer02;
                uint32 gameTypeSelected = 0;
                string messageGameTypeSelected = "";

                // Human game (with group mate)
                if (action == GOSSIP_ACTION_INFO_DEF + 51)
                {
                    // Check if player in group and if have only one mate
                    if (player->GetGroup() && player->GetGroup()->GetMembersCount() == 2)
                    {
                        // if (group->IsLeader(GetPlayer()->GetGUID()) || group->GetMembersCount() < 2)
                        for (GroupReference* groupRef = player->GetGroup()->GetFirstMember(); groupRef != NULL; groupRef = groupRef->next())
                        {
                            if (Player* member = groupRef->GetSource())
                            {
                                if (member->IsInWorld() && member->GetGUID() != guidPlayer01)
                                {
                                    bool gameExistFound = false;

                                    if (!NpcTicTacToeData.empty())
                                    {
                                        // Check for each information if player found as player 01 or 02
                                        for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                                        {
                                            if (i->second.guidPlayer01 == member->GetGUID() || i->second.guidPlayer02 == member->GetGUID())
                                            {
                                                gameExistFound = true;
                                            }
                                        }
                                    }

                                    if (gameExistFound == false)
                                    {
                                        namePlayer02 = member->GetName().c_str();
                                        guidPlayer02 = member->GetGUID();
                                    }
                                    else
                                    {
                                        std::string notificationMessage = "Tu compañero '" + member->GetName() + "' que ya está en un juego.";
                                        player->GetSession()->SendNotification(notificationMessage.c_str());
                                    }
                                }
                            }
                        }

                        if (namePlayer02 != "")
                        {
                            gameTypeSelected = 1;
                            messageGameTypeSelected = "adversario humano seleccionado.\n";
                        }
                        else
                        {
                            player->CLOSE_GOSSIP_MENU();
                            return true;
                        }
                    }
                    else
                    {
                        player->GetSession()->SendNotification("Usted necesita estar en un grupo (sólo con un compañero).");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    }
                }
                // Human game (with target mate)
                else if (action == GOSSIP_ACTION_INFO_DEF + 52)
                {
                    Player* targetPlayer = ObjectAccessor::FindPlayer(player->GetTarget());
                    if (targetPlayer && targetPlayer != player)
                    {
                        if (targetPlayer->IsInWorld())
                        {
                            bool gameExistFound = false;

                            if (!NpcTicTacToeData.empty())
                            {
                                // Check for each information if player found as player 01 or 02
                                for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
                                {
                                    if (i->second.guidPlayer01 == targetPlayer->GetGUID() || i->second.guidPlayer02 == targetPlayer->GetGUID())
                                    {
                                        gameExistFound = true;
                                    }
                                }
                            }

                            if (gameExistFound == false)
                            {
                                namePlayer02 = targetPlayer->GetName().c_str();
                                guidPlayer02 = targetPlayer->GetGUID();
                            }
                            else
                            {
                                std::string notificationMessage = "tu seleccion '" + targetPlayer->GetName() + "' que ya está en un juego.";
                                player->GetSession()->SendNotification(notificationMessage.c_str());
                            }
                        }

                        if (namePlayer02 != "")
                        {
                            gameTypeSelected = 1;
                            messageGameTypeSelected = "Adversario humano seleccionado.\n";
                        }
                        else
                        {
                            player->CLOSE_GOSSIP_MENU();
                            return true;
                        }
                    }
                    else
                    {
                        player->GetSession()->SendNotification("Es necesario seleccionar un jugador.");
                        player->CLOSE_GOSSIP_MENU();
                        return true;
                    }
                }
                // Computer game
                else if (action == GOSSIP_ACTION_INFO_DEF + 53)
                {
                    namePlayer02 = "NPC";
                    // If none second player get same guid for the second computer player
                    guidPlayer02 = player->GetGUID();
                    gameTypeSelected = 2;
                    messageGameTypeSelected = "NPCadversario ( movimiento aleatorio) seleccionado.\n";
                }
                // Master Computer game
                else if (action == GOSSIP_ACTION_INFO_DEF + 54)
                {
                    namePlayer02 = "MasterNPC";
                    // If none second player get same guid for the second computer player
                    guidPlayer02 = player->GetGUID();
                    gameTypeSelected = 3;
                    messageGameTypeSelected = "NPC adversary (AI) selected.\n";
                }

                NpcTicTacToeData[guidPlayer01].namePlayer01 = player->GetName().c_str();
                NpcTicTacToeData[guidPlayer01].guidPlayer01 = guidPlayer01;
                NpcTicTacToeData[guidPlayer01].namePlayer02 = namePlayer02;
                NpcTicTacToeData[guidPlayer01].guidPlayer02 = guidPlayer02;
                NpcTicTacToeData[guidPlayer01].messageGameTypeSelected = messageGameTypeSelected;
                NpcTicTacToeData[guidPlayer01].gameTypeSelected = gameTypeSelected;
                NpcTicTacToeData[guidPlayer01].gameStatus = 2;
                NpcTicTacToeData[guidPlayer01].needRefreshGossipP1 = false;
                NpcTicTacToeData[guidPlayer01].needRefreshGossipP2 = false;

                // Init game grid default falue
                for (int i(0); i < 3; i++){
                    for (int j(0); j < 3; j++){
                        NpcTicTacToeData[guidPlayer01].gameGrid[i][j] = 0;
                    }
                }

                std::string notificationMessage = "Game '" + COLOR_RED + "" + NpcTicTacToeData[guidPlayer01].namePlayer01 + "|r' Vs '" + COLOR_BLUE + "" + NpcTicTacToeData[guidPlayer01].namePlayer02 + "|r' created.";
                ChatHandler(player->GetSession()).PSendSysMessage(notificationMessage.c_str());

                OnGossipSelect(player, creature, GOSSIP_SENDER_MAIN, (GOSSIP_ACTION_INFO_DEF + 56));

                // If player game send notification to second player
                if (NpcTicTacToeGameExist->gameTypeSelected == 1)
                {
                    if (Player* playerFind02 = ObjectAccessor::FindConnectedPlayer(NpcTicTacToeGameExist->guidPlayer02))
                    {
                        playerFind02->GetSession()->SendNotification(notificationMessage.c_str());
                        ChatHandler(playerFind02->GetSession()).PSendSysMessage(notificationMessage.c_str());

                        OnGossipSelect(playerFind02, creature, GOSSIP_SENDER_MAIN, (GOSSIP_ACTION_INFO_DEF + 56));
                    }
                }
            } break;

            // Show game details for the two players
            case GOSSIP_ACTION_INFO_DEF + 56:
            {
                if (gameExistFound == false)
                {
                    player->CLOSE_GOSSIP_MENU();
                    return true;
                }

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "Theme " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, 1, NpcTicTacToeGameExist->themeTypeSelected), 60) + " Vs " + GetIconePathBySize(GetIconeNameByPlayerTheme(Path_Icon, 2, NpcTicTacToeGameExist->themeTypeSelected), 60) + " selected.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, NpcTicTacToeGameExist->messageGameTypeSelected, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);

                switch (NpcTicTacToeGameExist->gameTypeSelected)
                {
                    case 1:
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si tu " + COLOR_GREEN + "win|r este juego , que tendrá " + std::to_string(MONEY_WIN_GAME_HUMAN / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si tu  " + COLOR_RED + "loose|r este juego, se va a perder " + std::to_string(MONEY_LOOSE_GAME_HUMAN / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si juego " + COLOR_WHITE + "draw|r, usted tendrá " + std::to_string(MONEY_DRAW_GAME_HUMAN / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        break;
                    case 2:
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si tu  " + COLOR_GREEN + "win|r este juego , que tendrá " + std::to_string(MONEY_WIN_GAME_NPC / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si tu " + COLOR_RED + "loose|r este juego, se va a perder " + std::to_string(MONEY_LOOSE_GAME_NPC / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si juego " + COLOR_WHITE + "draw|r, usted tendrá " + std::to_string(MONEY_DRAW_GAME_NPC / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        break;
                    case 3:
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si tu  " + COLOR_GREEN + "win|r este juego , que tendrá " + std::to_string(MONEY_WIN_GAME_IA_NPC / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si tu  " + COLOR_RED + "loose|r este juego, se va a perder " + std::to_string(MONEY_LOOSE_GAME_IA_NPC / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, "si juego " + COLOR_WHITE + "draw|r, usted tendrá " + std::to_string(MONEY_DRAW_GAME_IA_NPC / 10000) + " Gold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 56);
                        break;
                }

                // All player can't start this new game
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "\nGame '" + COLOR_RED + "" + NpcTicTacToeGameExist->namePlayer01 + "|r' Vs '" + COLOR_BLUE + "" + NpcTicTacToeGameExist->namePlayer02 + "|r' created, start this new game ?\n", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 60);

                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_TALK, "\nMain menu", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(GOSSIP_MENU_TICTACTOE_INGAME, creature->GetGUID());
            } break;

            // Game started
            case GOSSIP_ACTION_INFO_DEF + 60:
            // Send notification rules
            case GOSSIP_ACTION_INFO_DEF + 63:
            // Wrong cell number
            case GOSSIP_ACTION_INFO_DEF + 67:
            // Wrong cell number : not empty
            case GOSSIP_ACTION_INFO_DEF + 68:
            // Select move for computer
            case GOSSIP_ACTION_INFO_DEF + 76:
                OnGossipSelectTicTacToeHuman(player, creature, GOSSIP_SENDER_MAIN, action, NpcTicTacToeGameExist);
                break;
        }
        return true;
    }

    // Called when a player select gossip with a code
    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code)
    {
        if (!player)
            return false;

        if (!creature)
            return false;

        player->PlayerTalkClass->ClearMenus();

        // Found current game for this player
        NpcTicTacToeInfo const* NpcTicTacToeGameExist = NULL;
        bool gameExistFound = false;

        if (!NpcTicTacToeData.empty())
        {
            // Check for each information if player found as player 01 or 02
            for (std::map<ObjectGuid, NpcTicTacToeInfo>::const_iterator i = NpcTicTacToeData.begin(); i != NpcTicTacToeData.end(); ++i)
            {
                if (i->second.guidPlayer01 == player->GetGUID() || i->second.guidPlayer02 == player->GetGUID())
                {
                    NpcTicTacToeGameExist = GetNpcTitcTacToeData(i->second.guidPlayer01);
                    if (NpcTicTacToeGameExist)
                    {
                        gameExistFound = true;
                    }
                    break;
                }
            }
        }

        // If no game found for current player
        if (gameExistFound == false)
        {
            player->CLOSE_GOSSIP_MENU();
            return true;
        }

        // Select move for player
        if (action == GOSSIP_ACTION_INFO_DEF + 75)
        {
            // only 1 letter and 1 number need (2 characters)
            if (strlen(code) == 2)
            {
                std::string codeString = std::string(code);
                Utf8ToUpperOnlyLatin(codeString);

                // Check if letter and number given are correct
                if (codeString.substr(0, 1).find_first_of("ABC") != std::string::npos && codeString.substr(1, 1).find_first_of("123") != std::string::npos)
                {
                    // If cell empty
                    if (NpcTicTacToeGameExist->gameGrid[static_cast<int>(Grid_Letters.find(codeString.substr(0, 1)))][atoi(codeString.substr(1, 1).c_str()) - 1] == 0)
                    {
                        // Set player id in cell choosen
                        NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameGrid[static_cast<int>(Grid_Letters.find(codeString.substr(0, 1)))][atoi(codeString.substr(1, 1).c_str()) - 1] = NpcTicTacToeGameExist->nextIdPlayerMove;

                        // Set the next player move and refresh
                        if (NpcTicTacToeGameExist->nextIdPlayerMove == 1)
                        {
                            NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextGuidPlayerMove = NpcTicTacToeGameExist->guidPlayer02;
                            NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextIdPlayerMove = 2;

                            // Launch gossip refresh only for human game
                            if (NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameTypeSelected == 1)
                            {
                                NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP2 = true;
                                ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_HUMAN_PLAYER_02, 500);

                                // Main game check
                                OnGossipSelect(player, creature, sender, (GOSSIP_ACTION_INFO_DEF + 60));
                                return true;
                            }
                            else
                            {
                                // Check if win (action for computer move)
                                OnGossipSelect(player, creature, sender, (GOSSIP_ACTION_INFO_DEF + 76));
                                return true;
                            }
                        }
                        else
                        {
                            NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextGuidPlayerMove = NpcTicTacToeGameExist->guidPlayer01;
                            NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].nextIdPlayerMove = 1;

                            // Launch gossip refresh only for human game
                            if (NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].gameTypeSelected == 1)
                            {
                                NpcTicTacToeData[NpcTicTacToeGameExist->guidPlayer01].needRefreshGossipP1 = true;
                                ENSURE_AI(npc_tic_tac_toe::npc_tic_tac_toeAI, creature->AI())->_events.ScheduleEvent(WAIT_HUMAN_PLAYER_01, 500);
                            }

                            // Main game check
                            OnGossipSelect(player, creature, sender, (GOSSIP_ACTION_INFO_DEF + 60));
                            return true;
                        }
                    }
                    else
                    {
                        // Send notification and gossip details for wrong cell (already play)
                        OnGossipSelect(player, creature, sender, (GOSSIP_ACTION_INFO_DEF + 68));
                        return true;
                    }
                }
                else
                {
                    // Send notification and gossip details for wrong cell
                    OnGossipSelect(player, creature, sender, (GOSSIP_ACTION_INFO_DEF + 67));
                    return true;
                }
            }
            else
            {
                // Send notification and gossip details for wrong cell
                OnGossipSelect(player, creature, sender, (GOSSIP_ACTION_INFO_DEF + 67));
                return true;
            }
        }

        player->CLOSE_GOSSIP_MENU();
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_tic_tac_toeAI(creature);
    }

};

void AddSC_npc_tic_tac_toe()
{
    new npc_tic_tac_toe();
}
