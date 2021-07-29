/*
 * =====================================================================================
 *
 *       Filename: clientmonster.cpp
 *        Created: 08/31/2015 08:26:57
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */
#include <SDL2/SDL.h>
#include <algorithm>
#include "log.hpp"
#include "totype.hpp"
#include "dbcomid.hpp"
#include "clientmonster.hpp"
#include "uidf.hpp"
#include "mathf.hpp"
#include "fflerror.hpp"
#include "condcheck.hpp"
#include "processrun.hpp"
#include "protocoldef.hpp"
#include "dbcomrecord.hpp"
#include "pngtexoffdb.hpp"
#include "clientargparser.hpp"
#include "clientpathfinder.hpp"
#include "creaturemovable.hpp"
#include "clienttaodog.hpp"
#include "clientsandcactus.hpp"
#include "clienttaoskeleton.hpp"
#include "clientscarecrow.hpp"
#include "clientbugbatmaggot.hpp"
#include "clientcannibalplant.hpp"
#include "clientdualaxeskeleton.hpp"
#include "clientguard.hpp"
#include "clientsandghost.hpp"
#include "clientsandstoneman.hpp"
#include "clientwedgemoth.hpp"
#include "clientlightboltzombie.hpp"
#include "clientmonkzombie.hpp"
#include "clientrebornzombie.hpp"
#include "clientlightarmoredguard.hpp"
#include "clientanthealer.hpp"
#include "clientnumawizard.hpp"

extern Log *g_log;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_monsterDB;
extern ClientArgParser *g_clientArgParser;

std::optional<uint32_t> MonsterFrameGfxSeq::gfxID(const ClientMonster *monPtr, std::optional<int> frameOpt) const
{
    fflassert(monPtr);
    if(*this){
        // monster graphics retrieving key structure
        //
        //   3322 2222 2222 1111 1111 1100 0000 0000
        //   1098 7654 3210 9876 5432 1098 7654 3210
        //   ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^ ^^^^
        //   |||| |||| |||| |||| |||| |||| |||| ||||
        //             |||| |||| |||| |||| |||+-++++-----------     frame : max =   32
        //             |||| |||| |||| |||| +++----------------- direction : max =    8 -+
        //             |||| |||| |||| ++++---------------------    motion : max =   16 -+
        //             |+++-++++-++++--------------------------      look : max = 2048 -+------> gfxBaseID
        //             +---------------------------------------    shadow : max =    2
        //

        const auto frame = frameOpt.value_or(monPtr->currMotion()->frame);
        fflassert(frame >= 0);
        fflassert(frame < count);

        const auto          gfxFrame = to_u32(begin + frame * (reverse ? -1 : 1));
        const auto      lookGfxIndex = to_u32(gfxLookID.value_or(monPtr->getMR().lookID) - LID_BEGIN);
        const auto    motionGfxIndex = to_u32(gfxMotionID.value_or(monPtr->currMotion()->type) - MOTION_MON_BEGIN);
        const auto directionGfxIndex = to_u32(gfxDirectionID.value_or(monPtr->currMotion()->direction) - DIR_BEGIN);

        return 0
            + ((     lookGfxIndex & 0X07FF) << 12)
            + ((   motionGfxIndex & 0X000F) <<  8)
            + ((directionGfxIndex & 0X0007) <<  5)
            + ((         gfxFrame & 0X001F) <<  0);
    }
    return {};
}

ClientMonster::ClientMonster(uint64_t uid, ProcessRun *proc)
    : CreatureMovable(uid, proc)
{
    if(uidf::getUIDType(uid) != UID_MON){
        throw fflerror("invalid UID for monster type: UIDName = %s", to_cstr(uidf::getUIDString(uid)));
    }

    if(g_clientArgParser->drawUID){
        m_nameBoard.setText(u8"%s(%llu)", DBCOM_MONSTERRECORD(monsterID()).name, to_llu(UID()));
    }
    else{
        m_nameBoard.setText(u8"%s", DBCOM_MONSTERRECORD(monsterID()).name);
    }
}

bool ClientMonster::update(double ms)
{
    updateAttachMagic(ms);
    m_currMotion->updateSpellEffect(ms);

    if(!checkUpdate(ms)){
        return true;
    }

    const CallOnExitHelper motionOnUpdate([this]()
    {
        m_currMotion->update();
    });

    switch(m_currMotion->type){
        case MOTION_MON_STAND:
            {
                if(stayIdle()){
                    return advanceMotionFrame(1);
                }

                // move to next motion will reset frame as 0
                // if current there is no more motion pending
                // it will add a MOTION_MON_STAND
                //
                // we don't want to reset the frame here
                return moveNextMotion();
            }
        case MOTION_MON_DIE:
            {
                const auto frameCount = getFrameCount(m_currMotion->type, m_currMotion->direction);
                if(frameCount <= 0){
                    return false;
                }

                if(m_currMotion->frame + 1 < frameCount){
                    return advanceMotionFrame(1);
                }

                switch(m_currMotion->extParam.die.fadeOut){
                    case 0:
                        {
                            break;
                        }
                    case 255:
                        {
                            // deactivated if fadeOut reach 255
                            // next update will auotmatically delete it
                            break;
                        }
                    default:
                        {
                            int nextFadeOut = 0;
                            nextFadeOut = (std::max<int>)(1, m_currMotion->extParam.die.fadeOut + 10);
                            nextFadeOut = (std::min<int>)(nextFadeOut, 255);

                            m_currMotion->extParam.die.fadeOut = nextFadeOut;
                            break;
                        }
                }
                return true;
            }
        default:
            {
                return updateMotion();
            }
    }
}

void ClientMonster::drawFrame(int viewX, int viewY, int focusMask, int frame, bool frameOnly)
{
    const auto gfxBodyIDOpt = getFrameGfxSeq(m_currMotion->type, m_currMotion->direction).gfxID(this, frame);
    if(!gfxBodyIDOpt.has_value()){
        return;
    }

    const uint32_t   bodyKey = (to_u32(0) << 23) + gfxBodyIDOpt.value(); // body
    const uint32_t shadowKey = (to_u32(1) << 23) + gfxBodyIDOpt.value(); // shadow

    const auto [  bodyFrame,   bodyDX,   bodyDY] = g_monsterDB->retrieve(  bodyKey);
    const auto [shadowFrame, shadowDX, shadowDY] = g_monsterDB->retrieve(shadowKey);
    const auto [shiftX, shiftY] = getShift(frame);

    // always reset the alpha mode for each texture because texture is shared
    // one texture to draw can be configured with different alpha mode for other creatures

    if(bodyFrame){
        SDL_SetTextureAlphaMod(bodyFrame, 255);
    }

    if(shadowFrame){
        SDL_SetTextureAlphaMod(shadowFrame, 128);
    }

    if(true
            && (m_currMotion->type  == MOTION_MON_DIE)
            && (m_currMotion->extParam.die.fadeOut  > 0)){
        // FadeOut :    0 : normal
        //         : 1-255: fadeOut
        if(bodyFrame){
            SDL_SetTextureAlphaMod(bodyFrame, (255 - m_currMotion->extParam.die.fadeOut) / 1);
        }

        if(shadowFrame){
            SDL_SetTextureAlphaMod(shadowFrame, (255 - m_currMotion->extParam.die.fadeOut) / 2);
        }
    }

    const auto fnBlendFrame = [](SDL_Texture *pTexture, int nFocusChan, int nX, int nY)
    {
        if(true
                && pTexture
                && nFocusChan >= 0
                && nFocusChan <  FOCUS_END){

            // if provided channel as 0
            // just blend it using the original color

            const auto stColor = focusColor(nFocusChan);
            if(!SDL_SetTextureColorMod(pTexture, stColor.r, stColor.g, stColor.b)){
                g_sdlDevice->drawTexture(pTexture, nX, nY);
            }
        }
    };

    const int startX = currMotion()->x * SYS_MAPGRIDXP + shiftX - viewX;
    const int startY = currMotion()->y * SYS_MAPGRIDYP + shiftY - viewY;

    // TODO some monter doesn't need to draw seperate shadow texture
    //      the body frame itself has shadow effect, i.e. 洞穴蜈蚣, later should remove the synthesized shadow texture

    if(isMonster(u8"洞穴蜈蚣") || isMonster(u8"栗子树") || isMonster(u8"圣诞树") || isMonster(u8"沙鬼") || isMonster(u8"沙漠鱼魔")){
        // sikp shadow
    }
    else{
        fnBlendFrame(shadowFrame, 0, startX + shadowDX, startY + shadowDY);
    }
    fnBlendFrame(bodyFrame, 0, startX + bodyDX, startY + bodyDY);

    if(!frameOnly){
        if(g_clientArgParser->drawTextureAlignLine){
            g_sdlDevice->drawLine (colorf::RED  + colorf::A_SHF(128), startX, startY, startX + bodyDX, startY + bodyDY);
            g_sdlDevice->drawCross(colorf::BLUE + colorf::A_SHF(128), startX, startY, 5);

            const auto [texW, texH] = SDLDeviceHelper::getTextureSize(bodyFrame);
            g_sdlDevice->drawRectangle(colorf::RED + colorf::A_SHF(128), startX + bodyDX, startY + bodyDY, texW, texH);
        }

        if(g_clientArgParser->drawTargetBox){
            if(const auto box = getTargetBox()){
                g_sdlDevice->drawRectangle(colorf::BLUE + colorf::A_SHF(128), box.x - viewX, box.y - viewY, box.w, box.h);
            }
        }
    }

    for(int nFocusChan = 1; nFocusChan < FOCUS_END; ++nFocusChan){
        if(focusMask & (1 << nFocusChan)){
            fnBlendFrame(bodyFrame, nFocusChan, startX + bodyDX, startY + bodyDY);
        }
    }

    if(!frameOnly){
        for(auto &p: m_attachMagicList){
            p->drawShift(startX, startY, false);
        }

        if(m_currMotion->type != MOTION_MON_DIE && g_clientArgParser->drawHPBar){
            auto pBar0 = g_progUseDB->retrieve(0X00000014);
            auto pBar1 = g_progUseDB->retrieve(0X00000015);

            int nBarW = -1;
            int nBarH = -1;
            SDL_QueryTexture(pBar1, nullptr, nullptr, &nBarW, &nBarH);

            const int drawBarXP = startX +  7;
            const int drawBarYP = startY - 53;
            const int drawBarWidth = to_d(std::lround(nBarW * (m_sdHealth.maxHP ? (std::min<double>)(1.0, (1.0 * m_sdHealth.HP) / m_sdHealth.maxHP) : 1.0)));

            g_sdlDevice->drawTexture(pBar1, drawBarXP, drawBarYP, 0, 0, drawBarWidth, nBarH);
            g_sdlDevice->drawTexture(pBar0, drawBarXP, drawBarYP);

            if(g_clientArgParser->alwaysDrawName || (focusMask & (1 << FOCUS_MOUSE))){
                const int nLW = m_nameBoard.w();
                const int nLH = m_nameBoard.h();
                const int nDrawNameXP = drawBarXP + nBarW / 2 - nLW / 2;
                const int nDrawNameYP = drawBarYP + 20;
                m_nameBoard.drawEx(nDrawNameXP, nDrawNameYP, 0, 0, nLW, nLH);
            }
        }
    }
}

bool ClientMonster::parseAction(const ActionNode &action)
{
    m_lastActive = SDL_GetTicks();
    for(const auto &m: m_forcedMotionQueue){
        if(m->type == MOTION_MON_DIE){
            return true;
        }
    }

    for(const auto &m: m_motionQueue){
        if(m->type == MOTION_MON_DIE){
            throw fflerror("Found MOTION_MON_DIE in pending motion queue");
        }
    }

    m_motionQueue.clear();
    switch(action.type){
        case ACTION_DIE       : return onActionDie       (action) && motionQueueValid();
        case ACTION_STAND     : return onActionStand     (action) && motionQueueValid();
        case ACTION_HITTED    : return onActionHitted    (action) && motionQueueValid();
        case ACTION_JUMP      : return onActionJump      (action) && motionQueueValid();
        case ACTION_MOVE      : return onActionMove      (action) && motionQueueValid();
        case ACTION_ATTACK    : return onActionAttack    (action) && motionQueueValid();
        case ACTION_SPAWN     : return onActionSpawn     (action) && motionQueueValid();
        case ACTION_TRANSF    : return onActionTransf    (action) && motionQueueValid();
        case ACTION_SPACEMOVE2: return onActionSpaceMove2(action) && motionQueueValid();
        default               : return false;
    }
}

bool ClientMonster::onActionDie(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    for(auto &node: makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED)){
        if(!(node && motionValid(node))){
            throw fflerror("current motion node is invalid");
        }
        m_forcedMotionQueue.push_back(std::move(node));
    }

    const auto [dieX, dieY, dieDir] = motionEndGLoc().at(1);
    m_forcedMotionQueue.emplace_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_DIE,
        .direction = dieDir,
        .x = dieX,
        .y = dieY,
    }));

    // set motion fadeOut as 0
    // server later will issue fadeOut on dead body
    return true;
}

bool ClientMonster::onActionStand(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = action.direction,
        .x = action.x,
        .y = action.y,
    }));
    return true;
}

bool ClientMonster::onActionHitted(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    m_motionQueue.emplace_back(std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_HITTED,
        .direction = action.direction,
        .x = action.x,
        .y = action.y,
    }));
    return true;
}

bool ClientMonster::onActionTransf(const ActionNode &)
{
    throw fflerror("unexpected ACTION_TRANSF to uid: %s", uidf::getUIDString(UID()).c_str());
}

bool ClientMonster::onActionSpaceMove2(const ActionNode &action)
{
    flushForcedMotion();
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = m_currMotion->direction,
        .x = action.x,
        .y = action.y,
    });
    return true;
}

bool ClientMonster::onActionJump(const ActionNode &action)
{
    flushForcedMotion();
    m_currMotion.reset(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = action.direction,
        .x = action.x,
        .y = action.y,
    });
    return true;
}

bool ClientMonster::onActionMove(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    if(auto moveNode = makeWalkMotion(action.x, action.y, action.aimX, action.aimY, action.speed); motionValid(moveNode)){
        m_motionQueue.push_back(std::move(moveNode));
        return true;
    }
    return false;
}

bool ClientMonster::onActionSpawn(const ActionNode &action)
{
    if(!m_forcedMotionQueue.empty()){
        throw fflerror("found motion before spawn: %s", uidf::getUIDString(UID()).c_str());
    }

    m_currMotion = std::unique_ptr<MotionNode>(new MotionNode
    {
        .type = MOTION_MON_STAND,
        .direction = [&action]() -> int
        {
            if(directionValid(action.direction)){
                return action.direction;
            }
            return DIR_UP;
        }(),

        .x = action.x,
        .y = action.y,
    });
    return true;
}

bool ClientMonster::onActionAttack(const ActionNode &action)
{
    const auto [endX, endY, endDir] = motionEndGLoc().at(1);
    m_motionQueue = makeWalkMotionQueue(endX, endY, action.x, action.y, SYS_MAXSPEED);
    if(auto coPtr = m_processRun->findUID(action.aimUID)){
        m_motionQueue.push_back(std::unique_ptr<MotionNode>(new MotionNode
        {
            .type = MOTION_MON_ATTACK0,
            .direction = [&action, endDir, coPtr]() -> int
            {
                const auto nX = coPtr->x();
                const auto nY = coPtr->y();
                if(mathf::LDistance2<int>(nX, nY, action.x, action.y) == 0){
                    return endDir;
                }
                return PathFind::GetDirection(action.x, action.y, nX, nY);
            }(),
            .x = action.x,
            .y = action.y,
        }));
        return true;
    }
    return false;
}

bool ClientMonster::motionValid(const std::unique_ptr<MotionNode> &motionPtr) const
{
    if(true
            && motionPtr
            && motionPtr->type >= MOTION_MON_BEGIN
            && motionPtr->type <  MOTION_MON_END

            && motionPtr->direction >= DIR_BEGIN
            && motionPtr->direction <  DIR_END

            && m_processRun
            && m_processRun->onMap(m_processRun->mapID(), motionPtr->x,    motionPtr->y)
            && m_processRun->onMap(m_processRun->mapID(), motionPtr->endX, motionPtr->endY)

            && motionPtr->speed >= SYS_MINSPEED
            && motionPtr->speed <= SYS_MAXSPEED

            && motionPtr->frame >= 0
            && motionPtr->frame <  getFrameCount(motionPtr->type, motionPtr->direction)){

        const auto nLDistance2 = mathf::LDistance2(motionPtr->x, motionPtr->y, motionPtr->endX, motionPtr->endY);
        switch(motionPtr->type){
            case MOTION_MON_STAND:
                {
                    return nLDistance2 == 0;
                }
            case MOTION_MON_WALK:
                {
                    return false
                        || nLDistance2 == 1
                        || nLDistance2 == 2
                        || nLDistance2 == 1 * maxStep() * maxStep()
                        || nLDistance2 == 2 * maxStep() * maxStep();
                }
            case MOTION_MON_ATTACK0:
            case MOTION_MON_ATTACK1:
            case MOTION_MON_SPELL0:
            case MOTION_MON_SPELL1:
            case MOTION_MON_HITTED:
            case MOTION_MON_DIE:
            case MOTION_MON_APPEAR:
                {
                    return nLDistance2 == 0;
                }
            case MOTION_MON_SPECIAL:
                {
                    return true;
                }
            default:
                {
                    break;
                }
        }
    }
    return false;
}

MonsterFrameGfxSeq ClientMonster::getFrameGfxSeq(int motion, int) const
{
    switch(motion){
        case MOTION_MON_STAND  : return {.count =  4};
        case MOTION_MON_WALK   : return {.count =  6};
        case MOTION_MON_ATTACK0: return {.count =  6};
        case MOTION_MON_HITTED : return {.count =  2};
        case MOTION_MON_DIE    : return {.count = 10};
        case MOTION_MON_ATTACK1: return {.count =  6};
        case MOTION_MON_SPELL0 :
        case MOTION_MON_SPELL1 : return {.count = 10};
        case MOTION_MON_APPEAR : return {.count = 10};
        case MOTION_MON_SPECIAL: return {.count =  6};
        default                : return {};
    }
}

std::unique_ptr<MotionNode> ClientMonster::makeWalkMotion(int nX0, int nY0, int nX1, int nY1, int nSpeed) const
{
    if(true
            && m_processRun
            && m_processRun->canMove(true, 0, nX0, nY0)
            && m_processRun->canMove(true, 0, nX1, nY1)

            && nSpeed >= SYS_MINSPEED
            && nSpeed <= SYS_MAXSPEED){

        static const int nDirV[][3] = {
            {DIR_UPLEFT,   DIR_UP,   DIR_UPRIGHT  },
            {DIR_LEFT,     DIR_NONE, DIR_RIGHT    },
            {DIR_DOWNLEFT, DIR_DOWN, DIR_DOWNRIGHT}};

        int nSDX = 1 + (nX1 > nX0) - (nX1 < nX0);
        int nSDY = 1 + (nY1 > nY0) - (nY1 < nY0);

        auto nLDistance2 = mathf::LDistance2(nX0, nY0, nX1, nY1);
        if(false
                || nLDistance2 == 1
                || nLDistance2 == 2
                || nLDistance2 == 1 * maxStep() * maxStep()
                || nLDistance2 == 2 * maxStep() * maxStep()){

            return std::unique_ptr<MotionNode>(new MotionNode
            {
                .type = MOTION_MON_WALK,
                .direction = nDirV[nSDY][nSDX],
                .speed = nSpeed,
                .x = nX0,
                .y = nY0,
                .endX = nX1,
                .endY = nY1,
            });
        }
    }
    return {};
}

ClientCreature::TargetBox ClientMonster::getTargetBox() const
{
    switch(m_currMotion->type){
        case MOTION_MON_DIE:
            {
                return {};
            }
        default:
            {
                break;
            }
    }

    const auto texBodyID = getFrameGfxSeq(m_currMotion->type, m_currMotion->direction).gfxID(this);
    if(!texBodyID.has_value()){
        return {};
    }

    int dx = 0;
    int dy = 0;
    auto bodyFrameTexPtr = g_monsterDB->retrieve(texBodyID.value(), &dx, &dy);

    if(!bodyFrameTexPtr){
        return {};
    }

    const auto [bodyFrameW, bodyFrameH] = SDLDeviceHelper::getTextureSize(bodyFrameTexPtr);

    const auto [shiftX, shiftY] = getShift(m_currMotion->frame);
    const int startX = m_currMotion->x * SYS_MAPGRIDXP + shiftX + dx;
    const int startY = m_currMotion->y * SYS_MAPGRIDYP + shiftY + dy;

    return getTargetBoxHelper(startX, startY, bodyFrameW, bodyFrameH);
}

bool ClientMonster::deadFadeOut()
{
    switch(m_currMotion->type){
        case MOTION_MON_DIE:
            {
                if(getMR().deadFadeOut){
                    if(!m_currMotion->extParam.die.fadeOut){
                        m_currMotion->extParam.die.fadeOut = 1;
                    }
                }
                return true;
            }
        default:
            {
                return false; // TODO push an ActionDie here
            }
    }
}

int ClientMonster::maxStep() const
{
    return 1;
}

int ClientMonster::currStep() const
{
    fflassert(motionValid(m_currMotion));
    switch(m_currMotion->type){
        case MOTION_MON_WALK:
            {
                return 1;
            }
        default:
            {
                return 0;
            }
    }
}

ClientMonster *ClientMonster::create(uint64_t uid, ProcessRun *proc, const ActionNode &action)
{
    switch(const auto monID = uidf::getMonsterID(uid)){
        case DBCOM_MONSTERID(u8"轻甲守卫"):
            {
                return new ClientLightArmoredGuard(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"蚂蚁道士"):
            {
                return new ClientAntHealer(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"诺玛法老"):
        case DBCOM_MONSTERID(u8"诺玛大法老"):
            {
                return new ClientNumaWizard(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"变异骷髅"):
            {
                return new ClientTaoSkeleton(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"神兽"):
            {
                return new ClientTaoDog(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"食人花"):
            {
                return new ClientCannibalPlant(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"角蝇"):
            {
                return new ClientBugbatMaggot(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"稻草人"):
            {
                return new ClientScarecrow(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"沙漠树魔"):
            {
                return new ClientSandCactus(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"掷斧骷髅"):
            {
                return new ClientDualAxeSkeleton(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"楔蛾"):
            {
                return new ClientWedgeMoth(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"沙鬼"):
            {
                return new ClientSandGhost(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"沙漠石人"):
            {
                return new ClientSandStoneMan(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"雷电僵尸"):
            {
                return new ClientLightBoltZombie(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"僧侣僵尸"):
            {
                return new ClientMonkZombie(uid, proc, action);
            }
        case DBCOM_MONSTERID(u8"僵尸_1"):
        case DBCOM_MONSTERID(u8"僵尸_2"):
        case DBCOM_MONSTERID(u8"腐僵"):
            {
                return new ClientRebornZombie(uid, proc, action);
            }
        default:
            {
                if(DBCOM_MONSTERRECORD(monID).behaveMode == BM_GUARD){
                    return new ClientGuard(uid, proc, action);
                }
                else{
                    return new ClientMonster(uid, proc, action);
                }
            }
    }
}
