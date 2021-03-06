
#include "Player.h"
#include "PlayerDump.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"


using namespace std;

class npc_arena_team_top : public CreatureScript
{
    public:
        npc_arena_team_top() : CreatureScript("npc_arena_team_top") { }

        bool OnGossipHello(Player * player, Creature * creature)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "View top 10 2v2 Arena Teams", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "View top 10 3v3 Arena Teams", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "View top 10 5v5 Arena Teams", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Nevermind", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
            player->SEND_GOSSIP_MENU(1, creature->GetGUID());
            return true;
        }

		bool OnGossipSelect(Player * player, Creature * creature, uint32 sender, uint32 actions)
        {
            if (sender == GOSSIP_SENDER_MAIN)
            {
                switch (actions)
                {
                    case GOSSIP_ACTION_INFO_DEF+1:
                    {
                        QueryResult result = CharacterDatabase.Query("SELECT `name`, `rating` FROM `arena_team` WHERE type = '2' ORDER BY `rating` DESC LIMIT 10");
                        if (!result)
                            return false;

                        Field * fields = NULL;
						//creature->Whisper("|cffFFFF00Here are the Top 10 2v2 arena teams:|r", player->GetGUID());
						player->Whisper("| cffFFFF00Here are the Top 10 2v2 arena teams : | r", LANG_UNIVERSAL, player, true);
                        do
                        {
                            fields = result->Fetch();
                            string arena_name = fields[0].GetString();
                            string rating = fields[1].GetString();
                            char msg[250];
                            snprintf(msg, 250, "|cff00FFE6Team Name:|r |cffFFFF00%s|r, |cff00FFE6Team Rating:|r |cffFFFF00%s|r \n", arena_name.c_str(), rating.c_str());
							//player->Whisper(msg, player->GetGUID());
							player->Whisper(msg, LANG_UNIVERSAL, player, true);
							
                        }

                        while (result->NextRow());
                    } break;

                    case GOSSIP_ACTION_INFO_DEF+2:
                    {
                        QueryResult result = CharacterDatabase.Query("SELECT `name`, `rating` FROM `arena_team` WHERE type = '3' ORDER BY `rating` DESC LIMIT 10");
                        if (!result)
                            return false;

                        Field * fields = NULL;
						player->Whisper("|cffFFFF00Here are the Top 10 3v3 arena teams:|r", LANG_UNIVERSAL, player, true);
                        do
                        {
                            fields = result->Fetch();
                            string arena_name = fields[0].GetString();
                            string rating = fields[1].GetString();
                            char msg[250];
                            snprintf(msg, 250, "|cff00FFE6Team Name:|r |cffFFFF00%s|r, |cff00FFE6Team Rating:|r |cffFFFF00%s|r \n", arena_name.c_str(), rating.c_str());
							player->Whisper(msg, LANG_UNIVERSAL, player, true);
                        }

                        while (result->NextRow());
                    } break;

                    case GOSSIP_ACTION_INFO_DEF+3:
                    {
                        QueryResult result = CharacterDatabase.Query("SELECT `name`, `rating` FROM `arena_team` WHERE type = '5' ORDER BY `rating` DESC LIMIT 10");
                        if (!result)
                            return false;

                        Field * fields = NULL;
						player->Whisper("|cffFFFF00Here are the Top 10 5v5 arena teams:|r", LANG_UNIVERSAL, player, true);
                        do
                        {
                            fields = result->Fetch();
                            string arena_name = fields[0].GetString();
                            string rating = fields[1].GetString();
                            char msg[250];
                            snprintf(msg, 250, "|cff00FFE6Team Name:|r |cffFFFF00%s|r, |cff00FFE6Team Rating:|r |cffFFFF00%s|r \n", arena_name.c_str(), rating.c_str());
							player->Whisper(msg,LANG_UNIVERSAL, player, true);
                        }

                        while (result->NextRow());
                    } break;

                    case GOSSIP_ACTION_INFO_DEF+4:
                    {
                        player->PlayerTalkClass->SendCloseGossip();
                    } break;
                }
            }
            return true;
        }
};

void AddSC_npc_arena_setup()
{
    new npc_arena_team_top;
}
