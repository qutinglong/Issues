
/* ScriptData
SDName: Boss_Exarch_Maladaar
SD%Complete: 95
SDComment: Most of event implemented, some adjustments to timers remain and possibly make some better code for switching his dark side in to better "images" of player.
SDCategory: Auchindoun, Auchenai Crypts
EndScriptData */

/* ContentData
mob_stolen_soul
boss_exarch_maladaar
mob_avatar_of_martyred
EndContentData */


#include "GameEventMgr.h"

#define SPELL_MOONFIRE          37328
#define SPELL_FIREBALL          37329
#define SPELL_MIND_FLAY         37330
#define SPELL_HEMORRHAGE        37331
#define SPELL_FROSTSHOCK        37332
#define SPELL_CURSE_OF_AGONY    37334
#define SPELL_MORTAL_STRIKE     37335
#define SPELL_FREEZING_TRAP     37368
#define SPELL_HAMMER_OF_JUSTICE 37369


class mob_stolen_soul : public CreatureScript
{
public:
    mob_stolen_soul() : CreatureScript("mob_stolen_soul")
    { }

    class mob_stolen_soulAI : public ScriptedAI
    {
        public:
        mob_stolen_soulAI(Creature *c) : ScriptedAI(c) {}
    
        uint8 myClass;
        uint32 Class_Timer;
    
        void Reset()
        override {
            Class_Timer = 1000;
        }
    
        void EnterCombat(Unit *who)
        override { }
    
        void SetMyClass(uint8 myclass)
        {
            myClass = myclass;
        }
    
        void UpdateAI(const uint32 diff)
        override {
            if (!UpdateVictim())
                return;
    
            if (Class_Timer < diff)
            {
                switch (myClass)
                {
                    case CLASS_WARRIOR:
                        DoCast(me->GetVictim(), SPELL_MORTAL_STRIKE);
                        Class_Timer = 6000;
                        break;
                    case CLASS_PALADIN:
                        DoCast(me->GetVictim(), SPELL_HAMMER_OF_JUSTICE);
                        Class_Timer = 6000;
                        break;
                    case CLASS_HUNTER:
                        DoCast(me->GetVictim(), SPELL_FREEZING_TRAP);
                        Class_Timer = 20000;
                        break;
                    case CLASS_ROGUE:
                        DoCast(me->GetVictim(), SPELL_HEMORRHAGE);
                        Class_Timer = 10000;
                        break;
                    case CLASS_PRIEST:
                        DoCast(me->GetVictim(), SPELL_MIND_FLAY);
                        Class_Timer = 5000;
                        break;
                    case CLASS_SHAMAN:
                        DoCast(me->GetVictim(), SPELL_FROSTSHOCK);
                        Class_Timer = 8000;
                        break;
                    case CLASS_MAGE:
                        DoCast(me->GetVictim(), SPELL_FIREBALL);
                        Class_Timer = 5000;
                        break;
                    case CLASS_WARLOCK:
                        DoCast(me->GetVictim(), SPELL_CURSE_OF_AGONY);
                        Class_Timer = 20000;
                        break;
                    case CLASS_DRUID:
                        DoCast(me->GetVictim(), SPELL_MOONFIRE);
                        Class_Timer = 10000;
                        break;
                }
            } else Class_Timer -= diff;
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_stolen_soulAI(creature);
    }
};


#define SAY_INTRO                   -1558000
#define SAY_SUMMON                  -1558001

#define SAY_AGGRO_1                 -1558002
#define SAY_AGGRO_2                 -1558003
#define SAY_AGGRO_3                 -1558004

#define SAY_ROAR                    -1558005
#define SAY_SOUL_CLEAVE             -1558006

#define SAY_SLAY_1                  -1558007
#define SAY_SLAY_2                  -1558008

#define SAY_DEATH                   -1558009

#define SPELL_RIBBON_OF_SOULS       32422
#define SPELL_SOUL_SCREAM           32421

#define SPELL_STOLEN_SOUL           32346
#define SPELL_STOLEN_SOUL_VISUAL    32395

#define SPELL_SUMMON_AVATAR         32424

#define ENTRY_STOLEN_SOUL           18441


class boss_exarch_maladaar : public CreatureScript
{
public:
    boss_exarch_maladaar() : CreatureScript("boss_exarch_maladaar")
    { }

    class boss_exarch_maladaarAI : public ScriptedAI
    {
        public:
        boss_exarch_maladaarAI(Creature *c) : ScriptedAI(c)
        {
            HasTaunted = false;
        }
    
        uint32 soulmodel;
        uint64 soulholder;
        uint8 soulclass;
    
        uint32 Fear_timer;
        uint32 Ribbon_of_Souls_timer;
        uint32 StolenSoul_Timer;
    
        bool HasTaunted;
        bool Avatar_summoned;
        
        bool isEventActive()
        {
            const GameEventMgr::ActiveEvents& activeEvents = sGameEventMgr->GetActiveEventList();
            bool active = activeEvents.find(57) != activeEvents.end();
    
            return active;
        }
    
        void Reset()
        override {
            soulmodel = 0;
            soulholder = 0;
            soulclass = 0;
    
            Fear_timer = 15000 + rand()% 5000;
            Ribbon_of_Souls_timer = 5000;
            StolenSoul_Timer = 25000 + rand()% 10000;
    
            Avatar_summoned = false;
            
            if (isEventActive())
                me->SetDisplayId(22802);
        }
    
        void MoveInLineOfSight(Unit *who)
        override {
            if (!HasTaunted && me->IsWithinDistInMap(who, 150.0))
            {
                DoScriptText(SAY_INTRO, me);
                HasTaunted = true;
            }
    
            ScriptedAI::MoveInLineOfSight(who);
        }
    
    
        void EnterCombat(Unit *who)
        override {
            switch (rand()%3)
            {
                case 0: DoScriptText(SAY_AGGRO_1, me); break;
                case 1: DoScriptText(SAY_AGGRO_2, me); break;
                case 2: DoScriptText(SAY_AGGRO_3, me); break;
            }
        }
    
        void JustSummoned(Creature *summoned)
        override {
            if (summoned->GetEntry() == ENTRY_STOLEN_SOUL)
            {
                //SPELL_STOLEN_SOUL_VISUAL has shapeshift effect, but not implemented feature in Trinity for this spell.
                summoned->CastSpell(summoned,SPELL_STOLEN_SOUL_VISUAL, TRIGGERED_NONE);
                summoned->SetDisplayId(soulmodel);
                summoned->SetFaction(me->GetFaction());
    
                if (Unit *target = ObjectAccessor::GetUnit(*me,soulholder))
                {
    
                ((mob_stolen_soul::mob_stolen_soulAI*)summoned->AI())->SetMyClass(soulclass);
                 summoned->AI()->AttackStart(target);
                }
            }
        }
    
        void KilledUnit(Unit* victim)
        override {
            if (rand()%2)
                return;
    
            switch (rand()%2)
            {
                case 0: DoScriptText(SAY_SLAY_1, me); break;
                case 1: DoScriptText(SAY_SLAY_2, me); break;
            }
        }
    
        void JustDied(Unit* Killer)
        override {
            DoScriptText(SAY_DEATH, me);
            //When Exarch Maladar is defeated D'ore appear.
            DoSpawnCreature(19412,0,0,0,0, TEMPSUMMON_TIMED_DESPAWN, 600000);
        }
    
        void UpdateAI(const uint32 diff)
        override {
            if (!UpdateVictim())
                return;
    
            if (!Avatar_summoned && (me->GetHealthPct() < 25))
            {
                if (me->IsNonMeleeSpellCast(false))
                    me->InterruptNonMeleeSpells(true);
    
                DoScriptText(SAY_SUMMON, me);
    
                DoCast(me, SPELL_SUMMON_AVATAR);
                Avatar_summoned = true;
                StolenSoul_Timer = 15000 + rand()% 15000;
            }
    
            if (StolenSoul_Timer < diff)
            {
                if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM,0))
                {
                    if (target->GetTypeId() == TYPEID_PLAYER)
                    {
                        if (me->IsNonMeleeSpellCast(false))
                            me->InterruptNonMeleeSpells(true);
    
                        uint32 i = urand(1,2);
                        if (i == 1)
                            DoScriptText(SAY_ROAR, me);
                        else
                            DoScriptText(SAY_SOUL_CLEAVE, me);
    
                        soulmodel = target->GetDisplayId();
                        soulholder = target->GetGUID();
                        soulclass = target->GetClass();
    
                        DoCast(target,SPELL_STOLEN_SOUL);
                        DoSpawnCreature(ENTRY_STOLEN_SOUL,0,0,0,0,TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT,10000);
    
                        StolenSoul_Timer = 20000 + rand()% 10000;
                    } else StolenSoul_Timer = 1000;
                }
            }else StolenSoul_Timer -= diff;
    
            if (Ribbon_of_Souls_timer < diff)
            {
                if (Unit *target = SelectTarget(SELECT_TARGET_RANDOM,0))
                    DoCast(target,SPELL_RIBBON_OF_SOULS);
    
                Ribbon_of_Souls_timer = 5000 + (rand()%20 * 1000);
            }else Ribbon_of_Souls_timer -= diff;
    
            if (Fear_timer < diff)
            {
                DoCast(me,SPELL_SOUL_SCREAM);
                Fear_timer = 15000 + rand()% 15000;
            }else Fear_timer -= diff;
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_exarch_maladaarAI(creature);
    }
};


#define SPELL_AV_MORTAL_STRIKE          16856
//#define SPELL_AV_SUNDER_ARMOR           16145


class mob_avatar_of_martyred : public CreatureScript
{
public:
    mob_avatar_of_martyred() : CreatureScript("mob_avatar_of_martyred")
    { }

    class mob_avatar_of_martyredAI : public ScriptedAI
    {
        public:
        mob_avatar_of_martyredAI(Creature *c) : ScriptedAI(c) {}
    
        uint32 Mortal_Strike_timer;
    //    uint32 sunderArmorTimer;
    
        void Reset()
        override {
            Mortal_Strike_timer = 10000;
    //        sunderArmorTimer = ?;
        }
    
        void EnterCombat(Unit *who)
        override {
        }
    
        void UpdateAI(const uint32 diff)
        override {
            if (!UpdateVictim())
                return;
    
            if (Mortal_Strike_timer < diff)
            {
                DoCast(me->GetVictim(), SPELL_AV_MORTAL_STRIKE);
                Mortal_Strike_timer = 10000 + rand()%20 * 1000;
            } else Mortal_Strike_timer -= diff;
    
    //        if (sunderArmorTimer < diff)
    //        {
    //            DoCast(me->GetVictim(), SPELL_AV_SUNDER_ARMOR);
    //            sunderArmorTimer = ?;
    //        } else sunderArmorTimer -= diff;
    
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new mob_avatar_of_martyredAI(creature);
    }
};


void AddSC_boss_exarch_maladaar()
{
    new boss_exarch_maladaar();
    new mob_avatar_of_martyred();
    new mob_stolen_soul();
}

