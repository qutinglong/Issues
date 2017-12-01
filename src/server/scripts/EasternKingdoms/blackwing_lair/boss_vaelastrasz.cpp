/* Copyright (C) 2006 - 2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* ScriptData
SDName: Boss_Vaelastrasz
SD%Complete: 75
SDComment: Burning Adrenaline not correctly implemented in core
SDCategory: Blackwing Lair
EndScriptData */



#define SAY_LINE1           -1469026
#define SAY_LINE2           -1469027
#define SAY_LINE3           -1469028
#define SAY_HALFLIFE        -1469029
#define SAY_KILLTARGET      -1469030

#define GOSSIP_ITEM         "Start Event <Needs Gossip Text>"

#define SPELL_ESSENCEOFTHERED       23513
#define SPELL_FLAMEBREATH           23461
#define SPELL_FIRENOVA              23462
#define SPELL_TAILSWIPE             15847
#define SPELL_BURNINGADRENALINE     23620
#define SPELL_CLEAVE                20684                   //Chain cleave is most likely named something different and contains a dummy effect

class boss_vaelastrasz : public CreatureScript
{
public:
    boss_vaelastrasz() : CreatureScript("boss_vaelastrasz")
    { }

    class boss_vaelAI : public ScriptedAI
    {
        public:
        boss_vaelAI(Creature *c) : ScriptedAI(c)
        {
            c->SetUInt32Value(UNIT_NPC_FLAGS,1);
            c->SetFaction(FACTION_FRIENDLY);
            c->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }
    
        uint64 PlayerGUID;
        uint32 SpeachTimer;
        uint32 SpeachNum;
        uint32 Cleave_Timer;
        uint32 FlameBreath_Timer;
        uint32 FireNova_Timer;
        uint32 BurningAdrenalineCaster_Timer;
        uint32 BurningAdrenalineTank_Timer;
        uint32 TailSwipe_Timer;
        bool HasYelled;
        bool DoingSpeach;
    
        void Reset()
        override {
            PlayerGUID = 0;
            SpeachTimer = 0;
            SpeachNum = 0;
            Cleave_Timer = 8000;                                //These times are probably wrong
            FlameBreath_Timer = 11000;
            BurningAdrenalineCaster_Timer = 15000;
            BurningAdrenalineTank_Timer = 45000;
            FireNova_Timer = 5000;
            TailSwipe_Timer = 20000;
            HasYelled = false;
            DoingSpeach = false;
    
            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(1, IMMUNITY_EFFECT,SPELL_EFFECT_ATTACK_ME, true);
        }
    
        void BeginSpeach(Unit* target)
        {
            //Stand up and begin speach
            PlayerGUID = target->GetGUID();
    
            //10 seconds
            DoScriptText(SAY_LINE1, me);
    
            SpeachTimer = 10000;
            SpeachNum = 0;
            DoingSpeach = true;
    
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    
        void KilledUnit(Unit *victim)
        override {
            if (rand()%5)
                return;
    
            DoScriptText(SAY_KILLTARGET, me, victim);
        }
    
        void EnterCombat(Unit *who)
        override {
            DoCast(me,SPELL_ESSENCEOFTHERED);
            DoZoneInCombat();
            me->SetHealth(int(me->GetMaxHealth()*.3));
        }
    
        void UpdateAI(const uint32 diff)
        override {
            //Speach
            if (DoingSpeach)
            {
                if (SpeachTimer < diff)
                {
                    switch (SpeachNum)
                    {
                        case 0:
                            //16 seconds till next line
                            DoScriptText(SAY_LINE2, me);
                            SpeachTimer = 16000;
                            SpeachNum++;
                            break;
                        case 1:
                            //This one is actually 16 seconds but we only go to 10 seconds because he starts attacking after he says "I must fight this!"
                            DoScriptText(SAY_LINE3, me);
                            SpeachTimer = 10000;
                            SpeachNum++;
                            break;
                        case 2:
                            me->SetFaction(FACTION_DRAGONFLIGHT_BLACK);
                            if (PlayerGUID && ObjectAccessor::GetUnit((*me),PlayerGUID))
                            {
                                AttackStart(ObjectAccessor::GetUnit((*me),PlayerGUID));
                                DoCast(me,SPELL_ESSENCEOFTHERED);
                            }
                            SpeachTimer = 0;
                            DoingSpeach = false;
                            break;
                    }
                }else SpeachTimer -= diff;
            }
    
            //Return since we have no target
            if (!UpdateVictim() )
                return;
    
            // Yell if hp lower than 15%
            if (me->GetHealthPct()  < 15 && !HasYelled)
            {
                DoScriptText(SAY_HALFLIFE, me);
                HasYelled = true;
            }
    
            //Cleave_Timer
            if (Cleave_Timer < diff)
            {
                DoCast(me->GetVictim(),SPELL_CLEAVE);
                Cleave_Timer = 15000;
            }else Cleave_Timer -= diff;
    
            //FlameBreath_Timer
            if (FlameBreath_Timer < diff)
            {
                DoCast(me->GetVictim(),SPELL_FLAMEBREATH);
                FlameBreath_Timer = 4000 + rand()%4000;
            }else FlameBreath_Timer -= diff;
    
            //BurningAdrenalineCaster_Timer
            if (BurningAdrenalineCaster_Timer < diff)
            {
                Unit* target = nullptr;
    
                int i = 0 ;
                while (i < 3)                                   // max 3 tries to get a random target with power_mana
                {
                    ++i;
                    target = SelectTarget(SELECT_TARGET_RANDOM,1);//not aggro leader
                    if (target)
                        if (target->GetPowerType() == POWER_MANA)
                            i=3;
                }
                if (target)                                     // cast on self (see below)
                    target->CastSpell(target,SPELL_BURNINGADRENALINE, TRIGGERED_FULL_MASK);
    
                BurningAdrenalineCaster_Timer = 15000;
            }else BurningAdrenalineCaster_Timer -= diff;
    
            //BurningAdrenalineTank_Timer
            if (BurningAdrenalineTank_Timer < diff)
            {
                // have the victim cast the spell on himself otherwise the third effect aura will be applied
                // to Vael instead of the player
                me->GetVictim()->CastSpell(me->GetVictim(),SPELL_BURNINGADRENALINE, TRIGGERED_FULL_MASK);
    
                BurningAdrenalineTank_Timer = 45000;
            }else BurningAdrenalineTank_Timer -= diff;
    
            //FireNova_Timer
            if (FireNova_Timer < diff)
            {
                DoCast(me->GetVictim(),SPELL_FIRENOVA);
                FireNova_Timer = 5000;
            }else FireNova_Timer -= diff;
    
            //TailSwipe_Timer
            if (TailSwipe_Timer < diff)
            {
                //Only cast if we are behind
                /*if (!me->HasInArc( M_PI, me->GetVictim()))
                {
                DoCast(me->GetVictim(),SPELL_TAILSWIPE);
                }*/
    
                TailSwipe_Timer = 20000;
            }else TailSwipe_Timer -= diff;
    
            DoMeleeAttackIfReady();
        }

        virtual bool GossipHello(Player* player) override
        {
            player->ADD_GOSSIP_ITEM( GOSSIP_ICON_CHAT, GOSSIP_ITEM        , GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU_TEXTID(907,me->GetGUID());

            return true;

        }


        virtual bool GossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
        {
            uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
            uint32 const sender = player->PlayerTalkClass->GetGossipOptionSender(gossipListId);
            ClearGossipMenuFor(player);
            if (sender == GOSSIP_SENDER_MAIN)
                SendDefaultMenu_boss_vael(player, action);

            return true;

        }

        void SendDefaultMenu_boss_vael(Player *player, uint32 action)
        {
            if (action == GOSSIP_ACTION_INFO_DEF + 1)               //Fight time
            {
                player->CLOSE_GOSSIP_MENU();
                BeginSpeach((Unit*)player);
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_vaelAI(creature);
    }
};


void AddSC_boss_vael()
{
    new boss_vaelastrasz();
}

