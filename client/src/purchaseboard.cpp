#include "totype.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "purchaseboard.hpp"
#include "inputstringboard.hpp"

extern Client *g_client;
extern PNGTexDB *g_itemDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

PurchaseBoard::PurchaseBoard(ProcessRun *runPtr, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_buttonClose
      {{
          .x = 257,
          .y = 183,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
              setExtendedItemID(0);
          },

          .parent{this},
      }}

    , m_buttonSelect
      {{
          .x = 105,
          .y = 185,

          .texIDList
          {
              .on   = 0X08000003,
              .down = 0X08000004,
          },

          .onTrigger = [this](Widget *, int)
          {
              setExtendedItemID(selectedItemID());
              m_buttonExt1Close.setOff();
              m_buttonExt2Close.setOff();
          },

          .parent{this},
      }}

    , m_buttonExt1Close
      {{
          .x = 448,
          .y = 159,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setExtendedItemID(0);
          },

          .parent{this},
      }}

    , m_buttonExt1Left
      {{
          .x = 315,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000007,
              .down = 0X08000008,
          },

          .onTrigger = [this](Widget *, int)
          {
              m_ext1Page = std::max<int>(0, m_ext1Page - 1);
          },

          .parent{this},
      }}

    , m_buttonExt1Select
      {{
          .x = 357,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000005,
              .down = 0X08000006,
          },

          .onTrigger = [this](Widget *, int)
          {
              const auto [itemID, seqID] = getExtSelectedItemSeqID();
              if(itemID){
                  m_processRun->requestBuy(m_npcUID, itemID, seqID, 1);
              }
          },

          .parent{this},
      }}

    , m_buttonExt1Right
      {{
          .x = 405,
          .y = 163,

          .texIDList
          {
              .on   = 0X08000009,
              .down = 0X0800000A,
          },

          .onTrigger = [this](Widget *, int)
          {
              if(const auto ext1PageCount = extendedPageCount(); ext1PageCount > 0){
                  m_ext1Page = std::min<int>(ext1PageCount - 1, m_ext1Page + 1);
              }
              else{
                  m_ext1Page = 0;
              }
          },

          .parent{this},
      }}

    , m_buttonExt2Close
      {{
          .x = 474,
          .y = 56,

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setExtendedItemID(0);
          },

          .parent{this},
      }}

    , m_buttonExt2Select
      {{
          .x = 366,
          .y = 60,

          .texIDList
          {
              .on   = 0X0800000B,
              .down = 0X0800000C,
          },

          .onTrigger = [this](Widget *, int)
          {
              const auto [itemID, seqID] = getExtSelectedItemSeqID();
              if(!itemID){
                  return;
              }

              if(seqID != 0){
                  throw fflerror("unexpected seqID = %llu", to_llu(seqID));
              }

              auto inputBoardPtr = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));
              const auto headerString = str_printf(u8"<layout><par>请输入你要购买<t color=\"red\">%s</t>的数量</par></layout>", to_cstr(DBCOM_ITEMRECORD(itemID).name));

              inputBoardPtr->setSecurity(false);
              inputBoardPtr->waitInput(headerString, [itemID, npcUID = m_npcUID, this](std::u8string inputString)
              {
                  const auto &ir = DBCOM_ITEMRECORD(itemID);
                  int count = 0;
                  try{
                      count = std::stoi(to_cstr(inputString));
                  }
                  catch(...){
                      m_processRun->addCBLog(CBLOG_ERR, u8"无效输入:%s", to_cstr(inputString));
                  }

                  if(ir && count > 0){
                      m_processRun->requestBuy(npcUID, itemID, 0, count);
                  }
              });
          },

          .parent{this},
      }}

    , m_slider
      {{
          .bar
          {
              .x = 266,
              .y = 27,
              .w = 5,
              .h = 123,
              .v = true,
          },

          .index = 0,
          .parent{this},
      }}

    , m_processRun(runPtr)
{
    setShow(false);
    if(auto texPtr = g_progUseDB->retrieve(0X08000000)){
        setSize(SDLDeviceHelper::getTextureWidth(texPtr), SDLDeviceHelper::getTextureHeight(texPtr));
    }
    else{
        throw fflerror("no valid purchase status board frame texture");
    }
}

void PurchaseBoard::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(auto texPtr = g_progUseDB->retrieve(0X08000000 + extendedBoardGfxID())){
        g_sdlDevice->drawTexture(texPtr, m.x, m.y);
    }

    drawChild(&m_buttonClose , m);
    drawChild(&m_buttonSelect, m);
    drawChild(&m_slider      , m);

    const auto remapX = m.x - m.ro->x;
    const auto remapY = m.y - m.ro->y;

    int startY = m_startY;
    LabelBoard label
    {{
        .font
        {
            .id = 1,
            .size = 12,
            .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        },
    }};

    for(size_t startIndex = getStartIndex(), i = startIndex; i < std::min<size_t>(m_itemList.size(), startIndex + 4); ++i){
        if(const auto &ir = DBCOM_ITEMRECORD(m_itemList[i])){
            drawItemInGrid(ir.type, ir.pkgGfxID, remapX + m_startX, remapY + startY);

            label.setText(u8"%s", to_cstr(ir.name));
            label.draw({.x=remapX + m_startX + m_boxW + 10, .y=remapY + startY + (m_boxH - label.h()) / 2});

            if(m_selected == to_d(i)){
                g_sdlDevice->fillRectangle(colorf::WHITE + colorf::A_SHF(64), remapX + m_startX, remapY + startY, 252 - 19, m_boxH);
            }
            startY += m_lineH;
        }
    }

    switch(extendedBoardGfxID()){
        case 1: drawt1(m); break;
        case 2: drawt2(m); break;
        default: break;
    }
}

bool PurchaseBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_buttonClose .processEventParent(event, valid, m)){ return true; }
    if(m_buttonSelect.processEventParent(event, valid, m)){ return true; }
    if(m_slider      .processEventParent(event, valid, m)){ return true; }

    switch(extendedBoardGfxID()){
        case 1:
            {
                if(m_buttonExt1Close .processEventParent(event, valid, m)){ return true; }
                if(m_buttonExt1Left  .processEventParent(event, valid, m)){ return true; }
                if(m_buttonExt1Select.processEventParent(event, valid, m)){ return true; }
                if(m_buttonExt1Right .processEventParent(event, valid, m)){ return true; }
                break;
            }
        case 2:
            {
                if(m_buttonExt2Close.processEventParent(event, valid, m)){
                    return true;
                }

                if(m_buttonExt2Select.processEventParent(event, valid, m)){
                    if(m_processRun->getWidget("InputStringBoard")->focus()){
                        setFocus(false);
                    }
                    return false;
                }
                break;
            }
        default:
            {
                break;
            }
    }

    const auto remapX = m.x - m.ro->x;
    const auto remapY = m.y - m.ro->y;

    switch(event.type){
        case SDL_MOUSEBUTTONDOWN:
            {
                for(int i = 0; i < 4; ++i){
                    if(mathf::pointInRectangle<int>(event.button.x - remapX, event.button.y - remapY, 19, 15 + (57 - 15) * i, 252 - 19, 53 - 15)){
                        m_selected = i + getStartIndex();
                        if(event.button.clicks >= 2){
                            if(const auto itemID = selectedItemID()){
                                setExtendedItemID(itemID);
                            }
                        }
                        break;
                    }
                }

                if(const int gridIndex = getExt1PageGrid(); gridIndex >= 0 && extendedPageCount() > 0 && m_ext1Page * 3 * 4 + gridIndex < to_d(m_sdSellItemList.list.size())){
                    m_ext1PageGridSelected = m_ext1Page * 3 * 4 + gridIndex;
                }
                break;
            }
        case SDL_MOUSEWHEEL:
            {
                const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
                if(mathf::pointInRectangle<int>(mousePX - remapX, mousePY - remapY, 19, 15, 252 - 19, 15 + (57 - 15) * 4)){
                    if(m_itemList.size() > 4){
                        m_slider.addValue((event.wheel.y > 0 ? -1.0 : 1.0) / (m_itemList.size() - 4), false);
                    }
                }
                else if(extendedBoardGfxID() == 1 && extendedPageCount() > 0 && mathf::pointInRectangle<int>(mousePX - remapX, mousePY - remapY, 313, 41, 152, 114)){
                    if(event.wheel.y > 0){
                        m_ext1Page--;
                    }
                    else{
                        m_ext1Page++;
                    }
                    m_ext1Page = std::max<int>(0, std::min<int>(extendedPageCount() - 1, m_ext1Page));
                }
                break;
            }
        default:
            {
                break;
            }
    }
    return true;
}

void PurchaseBoard::loadSell(uint64_t npcUID, std::vector<uint32_t> itemList)
{
    m_npcUID = npcUID;
    m_itemList = std::move(itemList);
}

size_t PurchaseBoard::getStartIndex() const
{
    if(m_itemList.size() <= 4){
        return 0;
    }
    return std::lround((m_itemList.size() - 4) * m_slider.getValue());
}

uint32_t PurchaseBoard::selectedItemID() const
{
    if((m_selected >= 0) && (m_selected < to_d(m_itemList.size()))){
        return m_itemList.at(m_selected);
    }
    return 0;
}

void PurchaseBoard::setExtendedItemID(uint32_t itemID)
{
    m_ext1PageGridSelected = -1;
    if(itemID){
        CMQuerySellItemList cmQSIL;
        std::memset(&cmQSIL, 0, sizeof(cmQSIL));
        cmQSIL.npcUID = m_npcUID;
        cmQSIL.itemID =   itemID;
        g_client->send({CM_QUERYSELLITEMLIST, cmQSIL});
    }
    else{
        m_sdSellItemList.clear();
    }
}

void PurchaseBoard::setSellItemList(SDSellItemList sdSIL)
{
    if(m_npcUID == sdSIL.npcUID){
        m_sdSellItemList = std::move(sdSIL);
    }
    else{
        m_sdSellItemList.clear();
    }
    m_ext1Page = 0;
}

int PurchaseBoard::extendedBoardGfxID() const
{
    if(m_sdSellItemList.npcUID && !m_sdSellItemList.list.empty()){
        return DBCOM_ITEMRECORD(m_sdSellItemList.list.front().item.itemID).packable() ? 2 : 1;
    }
    return 0;
}

int PurchaseBoard::extendedPageCount() const
{
    if(extendedBoardGfxID() != 1){
        return -1;
    }

    if(m_sdSellItemList.list.empty()){
        return 0;
    }
    return to_d(m_sdSellItemList.list.size() + 11) / 12;
}

int PurchaseBoard::getExt1PageGrid() const
{
    if(extendedBoardGfxID() != 1){
        return -1;
    }

    // const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    // if(mathf::pointInRectangle<int>(mousePX - x(), mousePY - y(), 313, 41, 152, 114)){
    //     const int r = (mousePY - y() -  41) / m_boxH;
    //     const int c = (mousePX - x() - 313) / m_boxW;
    //     return r * 4 + c;
    // }
    return -1;
}

std::tuple<int, int, int, int> PurchaseBoard::getExt1PageGridLoc(int gridX, int gridY)
{
    if(gridX >= 0 && gridX < 4 && gridY >= 0 && gridY < 3){
        return
        {
            313 + m_boxW * gridX,
            41  + m_boxH * gridY,
            m_boxW,
            m_boxH,
        };
    }
    throw fflerror("invalid grid location: (%d, %d)", gridX, gridY);
}

void PurchaseBoard::drawt1GridHoverText(int itemIndex) const
{
    if(extendedPageCount() <= 0){
        throw fflreach();
    }

    fflassert(itemIndex >= 0);
    fflassert(itemIndex < to_d(m_sdSellItemList.list.size()));
    fflassert(DBCOM_ITEMRECORD(selectedItemID()));

    LayoutBoard hoverTextBoard
    {{
        .lineWidth = 200,
        .font
        {
            .id = 1,
            .size = 12,
        },

        .lineAlign = LALIGN_JUSTIFY,
    }};

    const auto goldPrice = getItemPrice(itemIndex);
    hoverTextBoard.loadXML(to_cstr(m_sdSellItemList.list.at(itemIndex).item.getXMLLayout(
    {
        {SDItem::XML_PRICE, std::to_string(goldPrice)},
        {SDItem::XML_PRICECOLOR, (goldPrice > 100) ? std::string("red") : std::string("green")},
    }).c_str()));

    const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
    g_sdlDevice->fillRectangle(colorf::RGBA(0, 0, 0, 200), mousePX, mousePY, std::max<int>(hoverTextBoard.w(), 200) + 20, hoverTextBoard.h() + 20);
    hoverTextBoard.draw({.x=mousePX + 10, .y=mousePY + 10});
}

void PurchaseBoard::drawt1(Widget::ROIMap m) const
{
    if(extendedBoardGfxID() != 1){
        throw fflreach();
    }

    drawChild(&m_buttonExt1Close , m);
    drawChild(&m_buttonExt1Left  , m);
    drawChild(&m_buttonExt1Select, m);
    drawChild(&m_buttonExt1Right , m);

    const int ext1PageCount = extendedPageCount();
    if(ext1PageCount <= 0){
        return;
    }

    if(m_ext1Page >= ext1PageCount){
        throw fflerror("invalid ext1Page: ext1Page = %d, listSize = %zu", m_ext1Page, m_sdSellItemList.list.size());
    }

    const auto remapX = m.x - m.ro->x;
    const auto remapY = m.y - m.ro->y;

    int cursorOnGridIndex = -1;
    for(int r = 0; r < 3; ++r){
        for(int c = 0; c < 4; ++c){
            const size_t i = m_ext1Page * 3 * 4 + r * 4 + c;
            if(i >= m_sdSellItemList.list.size()){
                break;
            }

            const auto &sellItem = m_sdSellItemList.list.at(i);
            const auto &ir = DBCOM_ITEMRECORD(sellItem.item.itemID);
            if(!ir){
                throw fflerror("bad item in sell list: itemID = %llu, seqID = %llu", to_llu(sellItem.item.itemID), to_llu(sellItem.item.seqID));
            }

            constexpr int rightStartX = 313;
            constexpr int rightStartY =  41;
            const int rightBoxX = rightStartX + c * m_boxW;
            const int rightBoxY = rightStartY + r * m_boxH;

            drawItemInGrid(ir.type, ir.pkgGfxID, remapX + rightBoxX, remapY + rightBoxY);
            LabelBoard
            {{
                .label = to_u8cstr(str_ksep(getItemPrice(i))),
                .font
                {
                    .id = 1,
                    .size = 10,
                    .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
                },
            }}.draw({.x=remapX + rightBoxX, .y=remapY + rightBoxY});

            const auto [mousePX, mousePY] = SDLDeviceHelper::getMousePLoc();
            const bool gridSelected = (m_ext1PageGridSelected >= 0) && ((size_t)(m_ext1PageGridSelected) == i);
            const bool cursorOn = [rightBoxX, rightBoxY, mousePX, mousePY, remapX, remapY, this]() -> bool
            {
                return mathf::pointInRectangle<int>(mousePX, mousePY, remapX + rightBoxX, remapY + rightBoxY, m_boxW, m_boxH);
            }();

            if(gridSelected || cursorOn){
                const uint32_t gridColor = gridSelected ? (colorf::BLUE + colorf::A_SHF(96)) : (colorf::WHITE + colorf::A_SHF(96));
                g_sdlDevice->fillRectangle(gridColor, remapX + rightBoxX, remapY + rightBoxY, m_boxW, m_boxH);
            }

            if(cursorOn){
                cursorOnGridIndex = to_d(i);
            }
        }
    }

    LabelBoard
    {{
        .label = str_printf(u8"第%d/%d页", m_ext1Page + 1, ext1PageCount).c_str(),
        .font
        {
            .id = 1,
            .size = 12,
            .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        },
    }}.draw({.dir=DIR_NONE, .x=remapX + 389, .y=remapY + 18});

    if(cursorOnGridIndex >= 0){
        drawt1GridHoverText(cursorOnGridIndex);
    }
}

void PurchaseBoard::drawt2(Widget::ROIMap m) const
{
    if(extendedBoardGfxID() != 2){
        throw fflreach();
    }

    const auto [extItemID, extSeqID] = getExtSelectedItemSeqID();
    if(extSeqID){
        throw fflerror("unexpected extSeqID: %llu", to_llu(extSeqID));
    }

    drawChild(&m_buttonExt2Close , m);
    drawChild(&m_buttonExt2Select, m);

    if(m_sdSellItemList.list.empty()){
        return;
    }

    const auto &sellItem = m_sdSellItemList.list.at(0);
    const auto &ir = DBCOM_ITEMRECORD(sellItem.item.itemID);
    if(!ir){
        throw fflerror("bad item in sell list: itemID = %llu, seqID = %llu", to_llu(sellItem.item.itemID), to_llu(sellItem.item.seqID));
    }

    const auto remapX = m.x - m.ro->x;
    const auto remapY = m.y - m.ro->y;

    constexpr int rightStartX = 303;
    constexpr int rightStartY =  16;
    drawItemInGrid(ir.type, ir.pkgGfxID, remapX + rightStartX, remapY + rightStartY);

    LabelBoard
    {{
        .label = to_u8cstr(str_ksep(getItemPrice(0)) + to_cstr(u8" 金币")),
        .font
        {
            .id = 1,
            .size = 13,
            .color = colorf::RGBA(0XFF, 0XFF, 0X00, 0XFF),
        },
    }}.draw({.dir=DIR_LEFT, .x=remapX + 350 + 3, .y=remapY + 35});
}

std::tuple<uint32_t, uint32_t> PurchaseBoard::getExtSelectedItemSeqID() const
{
    switch(extendedBoardGfxID()){
        case 1:
            {
                if(m_ext1PageGridSelected >= 0 && m_ext1PageGridSelected < to_d(m_sdSellItemList.list.size())){
                    const auto &sellItem = m_sdSellItemList.list.at(m_ext1PageGridSelected);
                    return {sellItem.item.itemID, sellItem.item.seqID};
                }
                return {0, 0};
            }
        case 2:
            {
                if(m_sdSellItemList.list.empty()){
                    return {0, 0};
                }
                return {m_sdSellItemList.list.front().item.itemID, 0};
            }
        default:
            {
                throw fflreach();
            }
    }
}

size_t PurchaseBoard::getItemPrice(int itemIndex) const
{
    if(!(itemIndex >= 0 && itemIndex < to_d(m_sdSellItemList.list.size()))){
        throw fflerror("invalid argument: itemIndex = %d", itemIndex);
    }

    for(const auto &costItem: m_sdSellItemList.list.at(itemIndex).costList){
        if(costItem.isGold()){
            return costItem.count;
        }
    }
    return 0;
}

void PurchaseBoard::onBuySucceed(uint64_t npcUID, uint32_t itemID, uint32_t seqID)
{
    if(npcUID != m_npcUID){
        return;
    }

    if(npcUID != m_sdSellItemList.npcUID){
        return;
    }

    if(DBCOM_ITEMRECORD(itemID).packable()){
        return;
    }

    for(auto p = m_sdSellItemList.list.begin(); p != m_sdSellItemList.list.end(); ++p){
        if(p->item.itemID == itemID && p->item.seqID == seqID){
            m_sdSellItemList.list.erase(p);
            return;
        }
    }
}

void PurchaseBoard::drawItemInGrid(const char8_t *itemType, uint32_t pkgGfxID, int gridX, int gridY) const
{
    if(auto texPtr = getItemTexture(itemType, pkgGfxID)){
        auto [texW, texH] = SDLDeviceHelper::getTextureSize(texPtr);
        const auto resizeRatio = std::max<double>({to_df(texW) / m_boxW, to_df(texH) / m_boxH, 1.0});

        const auto dstW = to_d(std::lround(texW / resizeRatio));
        const auto dstH = to_d(std::lround(texH / resizeRatio));

        const auto drawX = gridX + (m_boxW - dstW) / 2;
        const auto drawY = gridY + (m_boxH - dstH) / 2;
        g_sdlDevice->drawTexture(texPtr, drawX, drawY, dstW, dstH, 0, 0, texW, texH);
    }
}

SDL_Texture *PurchaseBoard::getItemTexture(const char8_t *itemType, uint32_t pkgGfxID)
{
    // for some items the game originally doesn't support to buy/sell them so there is no buy/sell gfx
    // but there is gfx when they get weared, try to reuse

    if(auto texPtr = g_itemDB->retrieve(pkgGfxID | 0X02000000)){
        return texPtr;
    }

    if(str_haschar(itemType)){
        if(false
                || to_u8sv(itemType) == u8"项链"
                || to_u8sv(itemType) == u8"戒指"
                || to_u8sv(itemType) == u8"手镯"
                || to_u8sv(itemType) == u8"鞋"
                || to_u8sv(itemType) == u8"道具"
                || to_u8sv(itemType) == u8"火把"
                || to_u8sv(itemType) == u8"头盔"
                || to_u8sv(itemType) == u8"武器"
                || to_u8sv(itemType) == u8"衣服"){
            return g_itemDB->retrieve(pkgGfxID | 0X01000000);
        }
    }
    return nullptr;
}
