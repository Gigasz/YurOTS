//#include "preheaders.h"

#include <algorithm>
#include <functional>
#include <string>
#include <fstream>

#include "spells.h"

Spells::Spells(Game* igame): game(igame)
{

}

bool Spells::loadFromXml(const std::string &datadir)
{
    std::string name, words;
    bool enabled = false;
    int32_t vocId, maglv = 0, mana = 0, id = 0, charges = 0;
    this->loaded = false;

    std::string filename = datadir + "spells/spells.xml";
    //std::transform(filename.begin(), filename.end(), filename.begin(), tolower);
    toLowerCaseString(filename);
    xmlDocPtr doc = xmlParseFile(filename.c_str());

    if (doc)
    {
        this->loaded=true;
        xmlNodePtr root, p, tmp;
        char* nodeValue = NULL;
        root = xmlDocGetRootElement(doc);

        if (xmlStrcmp(root->name,(const xmlChar*) "spells"))
        {
            //TODO: use exceptions here
            std::cerr << "Malformed XML" << std::endl;
        }

        nodeValue = (char*)xmlGetProp(root, (const xmlChar *)"maxVoc");
        if(nodeValue)
        {
            maxVoc = atoi(nodeValue);
            xmlFreeOTSERV(nodeValue);
        }

        for(int32_t i =0; i<=this->maxVoc; i++)
        {
            std::map<std::string, Spell*> voc;
            vocationSpells.push_back(voc);
        }

        p = root->children;

        while (p)
        {
            const char* str = (char*)p->name;

            if (strcmp(str, "spell") == 0)
            {
                nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"enabled");
                if(nodeValue)
                {
                    enabled = (bool)(atoi(nodeValue) > 0);
                    xmlFreeOTSERV(nodeValue);
                }

                if (enabled)
                {
                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"name");
                    if(nodeValue)
                    {
                        name = nodeValue;
                        xmlFreeOTSERV(nodeValue);
                        //std::transform(name.begin(), name.end(), name.begin(), tolower);
                        toLowerCaseString(name);
                    }

                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"words");
                    if(nodeValue)
                    {
                        words = nodeValue;
                        xmlFreeOTSERV(nodeValue);
                    }

                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"maglv");
                    if(nodeValue)
                    {
                        maglv = atoi(nodeValue);
                        xmlFreeOTSERV(nodeValue);
                    }

                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"mana");
                    if(nodeValue)
                    {
                        mana = atoi(nodeValue);
                        xmlFreeOTSERV(nodeValue);
                    }

                    Spell* spell = new InstantSpell(datadir, name, words, maglv, mana, game);

                    tmp=p->children;
                    while (tmp)
                    {
                        if (strcmp((const char*)tmp->name, "vocation") == 0)
                        {
                            nodeValue = (char*)xmlGetProp(tmp, (const xmlChar *)"id");
                            if(nodeValue)
                            {
                                vocId = atoi(nodeValue);
                                xmlFreeOTSERV(nodeValue);

                                if (vocId<=this->maxVoc)
                                {
                                    (vocationSpells.at(vocId))[words] = spell;
                                }
                            }
                        }

                        tmp = tmp->next;
                    }

                    allSpells[words] = spell;
                }
            }
            else if (strcmp(str, "rune") == 0)
            {
                nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"enabled");
                if(nodeValue)
                {
                    enabled = (bool)(atoi(nodeValue) > 0);
                    xmlFreeOTSERV(nodeValue);
                }

                if (enabled)
                {
                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"name");
                    if(nodeValue)
                    {
                        name = nodeValue;
                        xmlFreeOTSERV(nodeValue);
                        toLowerCaseString(name);
                        //std::transform(name.begin(), name.end(), name.begin(), tolower);
                    }

                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"id");
                    if(nodeValue)
                    {
                        id = atoi(nodeValue);
                        xmlFreeOTSERV(nodeValue);
                    }

                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"charges");
                    if(nodeValue)
                    {
                        charges = atoi(nodeValue);
                        xmlFreeOTSERV(nodeValue);
                    }

                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"maglv");
                    if(nodeValue)
                    {
                        maglv = atoi(nodeValue);
                        xmlFreeOTSERV(nodeValue);
                    }

                    nodeValue = (char*)xmlGetProp(p, (const xmlChar *)"mana");
                    if(nodeValue)
                    {
                        mana = atoi(nodeValue);
                        xmlFreeOTSERV(nodeValue);
                    }

                    Spell* spell = new RuneSpell(datadir, name, id, charges, maglv, mana, game);

                    tmp=p->children;
                    while (tmp)
                    {
                        if (strcmp((const char*)tmp->name, "vocation") == 0)
                        {
                            nodeValue = (char*)xmlGetProp(tmp, (const xmlChar *)"id");
                            if(nodeValue)
                            {
                                vocId = atoi(nodeValue);
                                xmlFreeOTSERV(nodeValue);

                                if (vocId<=this->maxVoc)
                                {
                                    (vocationRuneSpells.at(vocId))[id] = spell;
                                }
                            }
                        }

                        tmp = tmp->next;
                    }

                    allRuneSpells[id] = spell;
                }
            }

            p = p->next;
        }

        xmlFreeDoc(doc);
    }

    return this->loaded;
}
Spells::~Spells()
{
    std::map<std::string, Spell*>::iterator it = allSpells.begin();

    while(it != allSpells.end())
    {
        delete it->second;
        allSpells.erase(it);
        it = allSpells.begin();
    }
}

Spell::Spell(std::string iname, int32_t imagLv, int32_t imana, Game* igame)
    :  game(igame), name(iname),magLv(imagLv) , mana(imana)
{
    this->loaded=false;
    this->script = NULL;
}

Spell::~Spell()
{
    if(script)
    {
        delete script;
        script = NULL;
    }
}

InstantSpell::InstantSpell(const std::string &datadir, std::string iname, std::string iwords, int32_t magLv, int32_t mana, Game* game)
    : Spell(iname, magLv, mana, game), words(iwords)
{
    this->script = new SpellScript(datadir, std::string(datadir + "spells/instant/")+(this->words)+std::string(".lua"), this);
    if(!this->script->isLoaded())
        this->loaded=false;
}


RuneSpell::RuneSpell(const std::string &datadir, std::string iname, uint16_t id, uint16_t charges, int32_t magLv, int32_t mana, Game* game)
    : Spell(iname, magLv, mana, game)
{
    this->id = id;
    this->charges = charges;

    this->script = new SpellScript(datadir, std::string(datadir + "spells/runes/")+(this->name)+std::string(".lua"), this);
    if(!this->script->isLoaded())
        this->loaded=false;
}


SpellScript::SpellScript(const std::string &datadir, std::string scriptname, Spell* spell)
{
    this->loaded = false;
    if(scriptname == "")
        return;
    luaState = lua_open();
    luaopen_loadlib(luaState);
    luaopen_base(luaState);
    luaopen_math(luaState);
    luaopen_string(luaState);
    luaopen_io(luaState);
    lua_dofile(luaState, std::string(datadir + "spells/lib/spells.lua").c_str());

#ifdef USING_VISUAL_2005
    FILE* in = NULL;
    fopen_s(&in, scriptname.c_str(), "r");
#else
    FILE* in=fopen(scriptname.c_str(), "r");
#endif //USING_VISUAL_2005

    if(!in)
        return;
    else
        fclose(in);
    lua_dofile(luaState, scriptname.c_str());
    this->loaded=true;
    this->spell=spell;
    this->setGlobalNumber("addressOfSpell", (int32_t)spell); //VISUAL FIX
    this->registerFunctions();
}

int32_t SpellScript::registerFunctions()
{

    lua_register(luaState, "doTargetMagic", SpellScript::luaActionDoTargetSpell);
    lua_register(luaState, "doTargetExMagic", SpellScript::luaActionDoTargetExSpell);
    lua_register(luaState, "doTargetGroundMagic", SpellScript::luaActionDoTargetGroundSpell);
    lua_register(luaState, "doAreaMagic", SpellScript::luaActionDoAreaSpell);
    lua_register(luaState, "doAreaExMagic", SpellScript::luaActionDoAreaExSpell);
    lua_register(luaState, "doAreaGroundMagic", SpellScript::luaActionDoAreaGroundSpell);
    lua_register(luaState, "changeOutfit", SpellScript::luaActionChangeOutfit);
    lua_register(luaState, "manaShield", SpellScript::luaActionManaShield);
    lua_register(luaState, "getPosition", SpellScript::luaActionGetPos);
    lua_register(luaState, "getSpeed", SpellScript::luaActionGetSpeed);
    lua_register(luaState, "changeSpeed", SpellScript::luaActionChangeSpeed);
    lua_register(luaState, "Paralyze", SpellScript::luaActionParalyze);
    lua_register(luaState, "challenge", SpellScript::luaChallenge);
    lua_register(luaState, "changeSpeedMonster", SpellScript::luaActionChangeSpeedMonster);
    lua_register(luaState, "makeRune", SpellScript::luaActionMakeRune);
    lua_register(luaState, "makeArrows", SpellScript::luaActionMakeArrows);
    lua_register(luaState, "makeFood", SpellScript::luaActionMakeFood);
    lua_register(luaState, "RemoveCond", SpellScript::luaActionRemoveCondition);
    lua_register(luaState, "Drunk", SpellScript::luaActionDrunk);
    lua_register(luaState, "setPlayerLightLevel", SpellScript::luaSetPlayerLightLevel);
    lua_register(luaState, "invisible", SpellScript::luaActionInvisible);
    return true;
}

bool SpellScript::castSpell(Creature* creature, const Position& pos, std::string var)
{
    lua_pushstring(luaState, "onCast");
    lua_gettable(luaState, LUA_GLOBALSINDEX);
    lua_pushnumber(luaState, creature->getID());

    lua_newtable(luaState);
    setField("z", pos.z);
    setField("y", pos.y);
    setField("x", pos.x);

    lua_pushnumber(luaState, creature->level);
    lua_pushnumber(luaState, creature->maglevel);
    lua_pushstring(luaState, var.c_str());

    lua_pcall(luaState, 5, 1, 0);

    bool ret = (bool)(lua_toboolean(luaState, -1) > 0);
    lua_pop(luaState, 1);

    return ret;
}

Spell* SpellScript::getSpell(lua_State *L)
{
    lua_getglobal(L, "addressOfSpell");
    int32_t val = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);
    Spell* myspell = (Spell*)val;

    if(!myspell)
    {
        return 0;
    }
    return myspell;
}


void SpellScript::internalGetArea(lua_State *L, MagicEffectAreaClass &magicArea)
{
    std::vector<unsigned char> col;

    int32_t i=0, j = 0;
    lua_pushnil(L);  /* first key */

    while (lua_next(L, -2) != 0)
    {
        lua_pushnil(L);
        col.clear();
        while (lua_next(L, -2) != 0)
        {
            col.push_back((unsigned char)lua_tonumber(L, -1));

            lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
            j++;
        }

        magicArea.areaVec.push_back(col);

        j=0;
        lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
        i++;
    }

    lua_pop(L, 1);

    magicArea.areaEffect = (char)lua_tonumber(L, -1);
    lua_pop(L,1);
}

void SpellScript::internalGetPosition(lua_State *L, Position& pos)
{
    lua_pushstring(L, "z");
    lua_gettable(L, -2);
    pos.z = (int32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "y");
    lua_gettable(L, -2);
    pos.y = (int32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pushstring(L, "x");
    lua_gettable(L, -2);
    pos.x = (int32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pop(L, 1); //table
}

void SpellScript::internalGetMagicEffect(lua_State *L, MagicEffectClass& me)
{
    lua_pushnil(L);

    lua_next(L, -2);
    int32_t attackType = (int32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    switch(attackType)
    {
    case 0:
        me.attackType = ATTACK_NONE;
        break;
    case 1:
        me.attackType = ATTACK_ENERGY;
        break;
    case 2:
        me.attackType = ATTACK_BURST;
        break;
    case 3:
        me.attackType = ATTACK_FIRE;
        break;
    case 4:
        me.attackType = ATTACK_PHYSICAL;
        break;
    case 5:
        me.attackType = ATTACK_POISON;
        break;
    case 6:
        me.attackType = ATTACK_PARALYZE;
        break;
    case 7:
        me.attackType = ATTACK_DRUNKNESS;
        break;
    case 8:
        me.attackType = ATTACK_LIFEDRAIN;
        break;
    case 9:
        me.attackType = ATTACK_MANADRAIN;
        break;

    default:
#ifdef __DEBUG__
        std::cerr << "WARNING: internalGetMagicEffect(), attackType out of range!" << std::endl;
#endif
        break;
    }

    lua_next(L, -2);
    me.animationEffect = (char)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_next(L, -2);
    me.hitEffect = (char)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_next(L, -2);
    me.damageEffect = (char)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_next(L, -2);
    me.animationColor = (char)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_next(L, -2);
    me.offensive = (bool)(lua_toboolean(L, -1) > 0);
    lua_pop(L, 1);

    lua_next(L, -2);
    me.drawblood = (bool)(lua_toboolean(L, -1) > 0);
    lua_pop(L, 1);

    lua_next(L, -2);
    me.minDamage = (int32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_next(L, -2);
    me.maxDamage = (int32_t)lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_pop(L, 1); // last key

    lua_pop(L, 1); //end of table
}

int32_t SpellScript::luaActionDoTargetSpell(lua_State *L)
{
    MagicEffectTargetClass magicTarget;

    internalGetMagicEffect(L, magicTarget);

    Position centerpos;
    internalGetPosition(L, centerpos);

    Spell* spell = getSpell(L);
    magicTarget.manaCost = spell->getMana();

    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);
    if(!creature)
    {
        lua_pushboolean(L, false);
        return 1;
    }
    bool isSuccess = spell->game->creatureThrowRune(creature, centerpos, magicTarget);
    lua_pushboolean(L, isSuccess);
    return 1;
}

int32_t SpellScript::luaActionDoTargetExSpell(lua_State *L)
{
    ConditionVec condvec;
    internalLoadDamageVec(L, condvec);

    MagicEffectTargetExClass magicTargetEx(/*md,*/ condvec);

    internalGetMagicEffect(L, magicTargetEx);

    Position centerpos;
    internalGetPosition(L, centerpos);

    Spell* spell = getSpell(L);
    magicTargetEx.manaCost = spell->getMana();

    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);
    if(!creature)
    {
        lua_pushboolean(L, false);
        return 1;
    }
    bool isSuccess = spell->game->creatureThrowRune(creature, centerpos, magicTargetEx);
    lua_pushboolean(L, isSuccess);
    return 1;
}

int32_t SpellScript::luaActionDoTargetGroundSpell(lua_State *L)
{
    TransformMap transformMap;
    internalLoadTransformVec(L, transformMap);

    MagicEffectItem* fieldItem = new MagicEffectItem(transformMap);
    MagicEffectTargetGroundClass magicGround(fieldItem);

    magicGround.offensive = (bool)(lua_toboolean(L, -1) > 0);
    lua_pop(L,1);

    magicGround.animationEffect = (char)lua_tonumber(L, -1);
    lua_pop(L,1);

    Position centerpos;
    internalGetPosition(L, centerpos);

    Spell* spell = getSpell(L);
    magicGround.manaCost = spell->getMana();

    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    if(!creature)
    {
        lua_pushboolean(L, false);
        return 1;
    }
    bool isSuccess = spell->game->creatureThrowRune(creature, centerpos, magicGround);
    lua_pushboolean(L, isSuccess);
    return 1;
}

int32_t SpellScript::luaActionDoAreaSpell(lua_State *L)
{
    MagicEffectAreaClass magicArea;
    internalGetMagicEffect(L, magicArea);

    internalGetArea(L, magicArea);

    bool needDirection = (bool)(lua_toboolean(L, -1) > 0);
    lua_pop(L,1);

    Position centerpos;
    internalGetPosition(L, centerpos);

    Spell* spell = getSpell(L);
    magicArea.manaCost = spell->getMana();

    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);
    if(!creature)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    if(needDirection)
    {
        creature->oneSiteAttack = true;
        switch(creature->getDirection())
        {
        case NORTH:
            magicArea.direction = 1;
            break;
        case WEST:
            magicArea.direction = 2;
            break;
        case EAST:
            magicArea.direction = 3;
            break;
        case SOUTH:
            magicArea.direction = 4;
            break;
        };
    }
    else
    {
        magicArea.direction = 1;
    }
    RuneSpell* runeSpell = dynamic_cast<RuneSpell*>(spell);
    bool isSuccess;
    if(runeSpell)
        isSuccess = spell->game->creatureThrowRune(creature, centerpos, magicArea);
    else
        isSuccess = spell->game->creatureCastSpell(creature, centerpos, magicArea);
    lua_pushboolean(L, isSuccess);
    return 1;
}

int32_t SpellScript::luaActionDoAreaExSpell(lua_State *L)
{
    ConditionVec condvec;

    int32_t count = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    for(int32_t n = 0; n < count; ++n)
    {
        internalLoadDamageVec(L, condvec);
    }

    MagicEffectAreaExClass magicAreaEx(/*md,*/ condvec);

    internalGetMagicEffect(L, magicAreaEx);

    internalGetArea(L, magicAreaEx);

    bool needDirection = (bool)(lua_toboolean(L, -1) > 0);
    lua_pop(L,1);

    Position centerpos;
    internalGetPosition(L, centerpos);

    Spell* spell = getSpell(L);
    magicAreaEx.manaCost = spell->getMana();

    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);
    if(!creature)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    if(needDirection)
    {
        switch(creature->getDirection())
        {
        case NORTH:
            magicAreaEx.direction = 1;
            break;
        case WEST:
            magicAreaEx.direction = 2;
            break;
        case EAST:
            magicAreaEx.direction = 3;
            break;
        case SOUTH:
            magicAreaEx.direction = 4;
            break;
        };
    }
    else
    {
        magicAreaEx.direction = 1;
    }

    RuneSpell* runeSpell = dynamic_cast<RuneSpell*>(spell);
    bool isSuccess;
    if(runeSpell)
        isSuccess = spell->game->creatureThrowRune(creature, centerpos, magicAreaEx);
    else
        isSuccess = spell->game->creatureCastSpell(creature, centerpos, magicAreaEx);

    lua_pushboolean(L, isSuccess);
    return 1;
}

int32_t SpellScript::luaActionDoAreaGroundSpell(lua_State *L)
{
    TransformMap transformMap;
    internalLoadTransformVec(L, transformMap);

    MagicEffectItem* fieldItem = new MagicEffectItem(/*md,*/ transformMap);
    MagicEffectAreaGroundClass magicGroundEx(fieldItem);

    internalGetMagicEffect(L, magicGroundEx);

    internalGetArea(L, magicGroundEx);

    bool needDirection = (bool)(lua_toboolean(L, -1) > 0);
    lua_pop(L,1);

    Position centerpos;
    internalGetPosition(L, centerpos);

    Spell* spell = getSpell(L);
    magicGroundEx.manaCost = spell->getMana();

    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L, 1);
    if(!creature)
    {
        lua_pushboolean(L, false);
        return 1;
    }

    if(needDirection)
    {
        switch(creature->getDirection())
        {
        case NORTH:
            magicGroundEx.direction = 1;
            break;
        case WEST:
            magicGroundEx.direction = 2;
            break;
        case EAST:
            magicGroundEx.direction = 3;
            break;
        case SOUTH:
            magicGroundEx.direction = 4;
            break;
        };
    }
    else
    {
        magicGroundEx.direction = 1;
    }

    bool isSuccess = spell->game->creatureThrowRune(creature, centerpos, magicGroundEx);
    lua_pushboolean(L, isSuccess);
    return 1;
}

int32_t SpellScript::luaActionChangeOutfit(lua_State *L)
{
    int32_t looktype = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);
    int32_t time = (int32_t)lua_tonumber(L, -1)*1000;
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);

    creature->looktype = looktype;
    spell->game->creatureChangeOutfit(creature);

    spell->game->changeOutfitAfter(creature->getID(), creature->lookmaster, time);
    return 0;
}

void SpellScript::internalLoadDamageVec(lua_State *L, ConditionVec& condvec)
{
    //cid
    uint32_t cid = (int32_t)lua_tonumber(L, 1);

    MagicEffectTargetCreatureCondition magicTargetCondition(cid);

    internalGetMagicEffect(L, magicTargetCondition);

    //conditionTimeCount
    int32_t condCount = (int32_t)lua_tonumber(L, -1); //conditionCount
    lua_pop(L, 1);

    int32_t ticks = (int32_t)lua_tonumber(L, -1); //delayTicks
    lua_pop(L, 1);

    condvec.insert(condvec.begin(), CreatureCondition(ticks, condCount, magicTargetCondition));
}

void SpellScript::internalLoadTransformVec(lua_State *L, TransformMap& transformMap)
{
    TransformItem ti;
    ConditionVec condvec;

    int32_t stateCount = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    for(int32_t s = 0; s < stateCount; ++s)
    {
        condvec.clear();

        int32_t id = (int32_t)lua_tonumber(L, -1);
        lua_pop(L,1);

        ti.first = (int32_t)lua_tonumber(L, -1);
        lua_pop(L,1);

        int32_t count = (int32_t)lua_tonumber(L, -1);
        lua_pop(L,1);

        for(int32_t n = 0; n < count; ++n)
        {
            internalLoadDamageVec(L, condvec);
        }

        ti.second = condvec;
        transformMap[id] = ti;
    }
}

int32_t SpellScript::luaActionManaShield(lua_State *L)
{
    int32_t time = (int32_t)lua_tonumber(L, -1)*1000;
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);
    creature->manaShieldTicks = time;

    Player* p = dynamic_cast<Player*>(creature);
    if(p)
        p->sendIcons();
    return 0;
}

int32_t SpellScript::luaActionChangeSpeed(lua_State *L)
{
    int32_t time = (int32_t)lua_tonumber(L, -1)*1000;
    lua_pop(L,1);

    int32_t speed = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);

    spell->game->addEvent(makeTask(time, boost::bind(&Game::changeSpeed, spell->game,creature->getID(), creature->getNormalSpeed()) ) );
    Player* p = dynamic_cast<Player*>(creature);
    if(p)
    {
        spell->game->changeSpeed(creature->getID(), creature->getNormalSpeed()+speed);
        p->sendIcons();
    }
    creature->hasteTicks = time;
    return 0;
}

int32_t SpellScript::luaActionParalyze(lua_State *L)
{
    int32_t speed = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    int32_t count = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    int32_t ticks = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    Position centerpos;
    internalGetPosition(L, centerpos);

    uint32_t cid = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByPosition(centerpos.x, centerpos.y, centerpos.z);
    Creature* attacker = spell->game->getCreatureByID(cid);

    if(attacker && creature && creature->getImmunities() != ATTACK_PARALYZE && creature->access == 0)
    {
        MagicEffectTargetCreatureCondition magicCondition = MagicEffectTargetCreatureCondition(cid);
        magicCondition.animationColor = 0xB4;
        magicCondition.damageEffect = NM_ME_MAGIC_BLOOD;
        magicCondition.hitEffect = NM_ME_MAGIC_BLOOD;
        magicCondition.attackType = ATTACK_PARALYZE;
        magicCondition.maxDamage = 0;
        magicCondition.minDamage = 0;
        magicCondition.offensive = true;
        CreatureCondition condition = CreatureCondition(ticks, count, magicCondition);
        creature->addCondition(condition, true);
        spell->game->changeSpeed(creature->getID(), speed);
        creature->hasteSpeed = 0;
        creature->hasteTicks = 0;
        Player* p = dynamic_cast<Player*>(creature);
        Player* attackerplayer = dynamic_cast<Player*>(attacker);
        if(p)
        {
            p->sendTextMessage(MSG_SMALLINFO, "You are paralyzed.");
            p->inFightTicks = (int32_t)g_config.PZ_LOCKED;
            p->sendIcons();
        }
        if(attackerplayer && attackerplayer->access == 0)
        {
            attackerplayer->pzLocked = true;
            attackerplayer->inFightTicks = (int32_t)g_config.PZ_LOCKED;
            attackerplayer->sendIcons();
        }
    } // because 0 damage is done you dont become pzlocked. (supports paralyzed more then once (kinda like fire))
    return 0;
}

int32_t SpellScript::luaChallenge(lua_State* L)
{
    int32_t id = (int32_t)lua_tonumber(L, -1);
    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID(id);

    SpectatorVec list;
    SpectatorVec::iterator it;
    spell->game->getSpectators(Range(creature->pos), list);
    for(it = list.begin(); it != list.end(); ++it)
    {
        Monster* monster = dynamic_cast<Monster*>(*it);
        if(monster && !monster->isSummon())
        {
            monster->changeTarget(creature);
        }
    }
    return 0;
}
int32_t SpellScript::luaActionChangeSpeedMonster(lua_State *L)
{
    int32_t time = (int32_t)lua_tonumber(L, -1)*1000;
    lua_pop(L,1);

    int32_t speed = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);

    if(creature)
    {
        spell->game->addEvent(makeTask(time, boost::bind(&Game::changeSpeed, spell->game,creature->getID(), creature->getSpeed())));
        spell->game->changeSpeed(creature->getID(), speed);
        creature->hasteTicks = time;
    }
    return 0;
}


int32_t SpellScript::luaActionGetSpeed(lua_State *L)
{
    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);

    lua_pushnumber(L, creature->getNormalSpeed());
    return 1;
}

int32_t SpellScript::luaActionGetPos(lua_State *L)
{
    const char* s = lua_tostring(L, -1);
    lua_pop(L,1);
    Spell* spell = getSpell(L);
    Creature* c = spell->game->getCreatureByName(std::string(s));
    Player* p = dynamic_cast<Player*>(c);
    if(!c || !p)
    {
        lua_newtable(L);
        lua_pushstring(L, "x");
        lua_pushnil(L);
        lua_settable(L, -3);

        lua_pushstring(L, "y");
        lua_pushnil(L);
        lua_settable(L, -3);

        lua_pushstring(L, "z");
        lua_pushnil(L);
        lua_settable(L, -3);
    }
    else
    {
        lua_newtable(L);
        lua_pushstring(L, "x");
        lua_pushnumber(L, c->pos.x);
        lua_settable(L, -3);

        lua_pushstring(L, "y");
        lua_pushnumber(L, c->pos.y);
        lua_settable(L, -3);

        lua_pushstring(L, "z");
        lua_pushnumber(L, c->pos.z);
        lua_settable(L, -3);
    }
    return 1;
}

int32_t SpellScript::luaActionMakeRune(lua_State *L)
{
    unsigned char charges = (unsigned char)lua_tonumber(L, -1);
    lua_pop(L,1);

    uint16_t type = (uint16_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);

    Player* player = dynamic_cast<Player*>(creature);
    if(player)
    {
        MagicEffectTargetClass magicTarget;

        magicTarget.offensive = false;
        magicTarget.drawblood = false;
        magicTarget.animationEffect = 0;
        magicTarget.attackType = ATTACK_NONE;
        magicTarget.hitEffect = 255; //NM_ME_NONE
        magicTarget.animationColor = 19; //GREEN

        int32_t a = -1;
        int32_t b = -1;

        //try to create rune 1
        a = internalMakeRune(player,SLOT_RIGHT,spell,type,charges);
        if(a == 1)
        {
            magicTarget.manaCost = spell->getMana();
        }

        //check if we got enough mana for the left hand
        if(player->getMana() - magicTarget.manaCost >= magicTarget.manaCost)
        {
            //try to create rune 2
            b = internalMakeRune(player,SLOT_LEFT,spell,type,charges);
            if(b == 1)
            {
                magicTarget.manaCost += spell->getMana();
            }
        }

        if(a == -1 && b == -1)  //not enough mana
        {
            magicTarget.damageEffect = 2; //NM_ME_PUFF
            magicTarget.manaCost = player->getPlayerInfo(PLAYERINFO_MAXMANA) + 1; //force not enough mana
        }
        else if( a == 0 && b == 0)  //not create any rune
        {
            magicTarget.damageEffect = 2; //NM_ME_PUFF
            magicTarget.manaCost = 0;
        }
        else if(a == 1 || b == 1)
        {
            magicTarget.damageEffect = 12; //NM_ME_MAGIC_ENERGIE = 12
        }

        /*else{
        	magicTarget.damageEffect = 12; //NM_ME_MAGIC_ENERGIE = 12

        	if(b==1){
        		magicTarget.manaCost = spell->getMana();
        	}
        	else{	//only create 1 rune
        		magicTarget.manaCost = 0;
        	}
        }*/

        bool isSuccess = spell->game->creatureThrowRune(player, player->pos, magicTarget);

        lua_pushnumber(L, 1);
        return 1;
    }
    lua_pushnumber(L, 0);
    return 1;
}

//create new runes and delete blank ones
int32_t SpellScript::internalMakeRune(Player *p,uint16_t sl_id,Spell *S,uint16_t id, unsigned char charges)
{
    //check mana
    if(p->mana < S->getMana() || p->exhaustedTicks >= 1000)
        return -1;
    Item *item = p->getItem(sl_id);
    if(item)
    {
        if(item->getID() == ITEM_RUNE_BLANK)
        {
            p->addItemInventory(Item::CreateItem(id, charges ),sl_id);
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    return 0;
}

int32_t SpellScript::luaActionMakeArrows(lua_State *L)
{
    unsigned char count = (unsigned char)lua_tonumber(L, -1);
    lua_pop(L,1);

    uint16_t id = (uint16_t)lua_tonumber(L, -1);
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);

    Player* player = dynamic_cast<Player*>(creature);
    if(player)
    {
        MagicEffectTargetClass magicTarget;

        magicTarget.offensive = false;
        magicTarget.drawblood = false;
        magicTarget.animationEffect = 0;
        magicTarget.attackType = ATTACK_NONE;
        magicTarget.hitEffect = 255; //NM_ME_NONE
        magicTarget.animationColor = 19; //GREEN

        if(player->mana < spell->getMana())
        {
            magicTarget.damageEffect = 2; //NM_ME_PUFF
            magicTarget.manaCost = player->getPlayerInfo(PLAYERINFO_MAXMANA) + 1; //force not enough mana
        }
        else
        {
            magicTarget.manaCost = spell->getMana();
            magicTarget.damageEffect = 12; //NM_ME_MAGIC_ENERGIE = 12
        }

        bool isSuccess = spell->game->creatureThrowRune(player, player->pos, magicTarget);

        if(isSuccess)
        {
            Item* new_item = Item::CreateItem(id,count);
            if(!player->addItem(new_item))
            {
                spell->game->addThing(NULL,player->pos,new_item);
            }
        }

        lua_pushnumber(L, 1);
        return 1;
    }
    lua_pushnumber(L, 0);
    return 1;
}

int32_t SpellScript::luaActionMakeFood(lua_State *L)
{
    unsigned char count = (unsigned char)lua_tonumber(L, -1);
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);

    Player* player = dynamic_cast<Player*>(creature);
    if(player)
    {
        MagicEffectTargetClass magicTarget;

        magicTarget.offensive = false;
        magicTarget.drawblood = false;
        magicTarget.animationEffect = 0;
        magicTarget.attackType = ATTACK_NONE;
        magicTarget.hitEffect = 255; //NM_ME_NONE
        magicTarget.animationColor = 19; //GREEN

        if(player->mana < spell->getMana())
        {
            magicTarget.damageEffect = 2; //NM_ME_PUFF
            magicTarget.manaCost = player->getPlayerInfo(PLAYERINFO_MAXMANA) + 1; //force not enough mana
        }
        else
        {
            magicTarget.manaCost = spell->getMana();
            magicTarget.damageEffect = 12; //NM_ME_MAGIC_ENERGIE = 12
        }

        bool isSuccess = spell->game->creatureThrowRune(player, player->pos, magicTarget);

        if(isSuccess)
        {
            int32_t r,foodtype;
            r = rand()%7;
            if(r == 0) foodtype = ITEM_MEAT;
            if(r == 1) foodtype = ITEM_HAM;
            if(r == 2) foodtype = ITEM_GRAPE;
            if(r == 3) foodtype = ITEM_APPLE;
            if(r == 4) foodtype = ITEM_BREAD;
            if(r == 5) foodtype = ITEM_CHEESE;
            if(r == 6) foodtype = ITEM_ROLL;
            if(r == 7) foodtype = ITEM_BREAD;

            Item* new_item = Item::CreateItem(foodtype,count);
            if(!player->addItem(new_item))
            {
                //add item on the ground
                spell->game->addThing(NULL,player->pos,new_item);
            }
        }

        lua_pushnumber(L, 1);
        return 1;
    }
    lua_pushnumber(L, 0);
    return 1;
}

int32_t SpellScript::luaSetPlayerLightLevel(lua_State* L)
{
    int32_t id = (int32_t)lua_tonumber(L, -2);
    int32_t lvl = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,2);

    Spell* spell = getSpell(L);
    Creature* c = spell->game->getCreatureByID(id);
    Player* player = c? dynamic_cast<Player*>(c) : NULL;

    if (player)
        spell->game->creatureChangeLight(player, 0, lvl, 0xD7);

    return 0;
}

int32_t SpellScript::luaActionInvisible(lua_State *L)
{
    int32_t time = (int32_t)lua_tonumber(L, -1)*1000;
    lua_pop(L,1);

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);

    creature->setInvisible(time);
    spell->game->creatureChangeOutfit(creature);
    return 0;
}

int32_t SpellScript::luaActionRemoveCondition(lua_State *L)
{

    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByID((uint32_t)lua_tonumber(L, -1));
    lua_pop(L,1);
    creature->removeCondition(ATTACK_POISON);
    return 0;
}

int32_t SpellScript::luaActionDrunk(lua_State *L)
{
    int32_t ticks = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);
    int32_t count = (int32_t)lua_tonumber(L, -1);
    lua_pop(L,1);
    Position centerpos;
    internalGetPosition(L, centerpos);
    uint32_t cid = (uint32_t)lua_tonumber(L, -1);
    lua_pop(L,1);
    Spell* spell = getSpell(L);
    Creature* creature = spell->game->getCreatureByPosition(centerpos.x, centerpos.y, centerpos.z);
    Creature* attacker = spell->game->getCreatureByID(cid);

    if(attacker && creature && creature->getImmunities() != ATTACK_DRUNKNESS && creature->access == 0)
    {
        MagicEffectTargetCreatureCondition magicCondition = MagicEffectTargetCreatureCondition(cid);
        magicCondition.animationColor = 0xB4;
        magicCondition.damageEffect = NM_ME_MAGIC_BLOOD;
        magicCondition.hitEffect = NM_ME_MAGIC_BLOOD;
        magicCondition.attackType = ATTACK_DRUNKNESS;
        magicCondition.maxDamage = 0;
        magicCondition.minDamage = 0;
        magicCondition.offensive = true;
        CreatureCondition condition = CreatureCondition(ticks, count, magicCondition);
        creature->addCondition(condition, true);
//creature->drunkTicks = 10000;
        Player* p = dynamic_cast<Player*>(creature);
        Player* attackerplayer = dynamic_cast<Player*>(attacker);
        if(p)
        {
            p->sendTextMessage(MSG_SMALLINFO, "You are drunk.");
            p->inFightTicks = (int32_t)g_config.PZ_LOCKED;
            p->sendIcons();
        }
        if(attackerplayer && attackerplayer->access == 0)
        {
            attackerplayer->pzLocked = true;
            attackerplayer->inFightTicks = (int32_t)g_config.PZ_LOCKED;
            attackerplayer->sendIcons();
        }
    } // because 0 damage is done you dont become pzlocked. (supports paralyzed more then once (kinda like fire))
    return 0;
}
