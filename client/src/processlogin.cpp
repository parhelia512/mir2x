#include <cstring>
#include <iostream>
#include <algorithm>

#include "log.hpp"
#include "client.hpp"
#include "message.hpp"
#include "pngtexdb.hpp"
#include "bgmusicdb.hpp"
#include "sdldevice.hpp"
#include "buildconfig.hpp"
#include "notifyboard.hpp"
#include "processlogin.hpp"
#include "clientargparser.hpp"

extern Log *g_log;
extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern BGMusicDB *g_bgmDB;
extern ClientArgParser *g_clientArgParser;

ProcessLogin::ProcessLogin()
	: Process()
        , m_canvas
          {
              DIR_UPLEFT,
              0,
              0,
              800,
              600,
          }

	, m_button1(DIR_UPLEFT, 150, 482, {0X00000005, 0X00000006, 0X00000007}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ doCreateAccount();  }, 0, 0, 0, 0, true, false, true, &m_canvas, false)
	, m_button2(DIR_UPLEFT, 352, 482, {0X00000008, 0X00000009, 0X0000000A}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ doChangePassword(); }, 0, 0, 0, 0, true, false, true, &m_canvas, false)
	, m_button3(DIR_UPLEFT, 554, 482, {0X0000000B, 0X0000000C, 0X0000000D}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ doExit();           }, 0, 0, 0, 0, true, false, true, &m_canvas, false)
        , m_button4(DIR_UPLEFT, 600, 536, {0X0000000E, 0X0000000F, 0X00000010}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ doLogin();          }, 0, 0, 0, 0, true, false, true, &m_canvas, false)

	, m_idBox
          {
              DIR_UPLEFT,
              159,
              540,

              146,
              18,

              false,

              2,
              18,
              0,

              colorf::WHITE_A255,

              2,
              colorf::WHITE_A255,

              [this]()
              {
                  m_idBox      .setFocus(false);
                  m_passwordBox.setFocus(true);
              },

              [this]()
              {
                  doLogin();
              },

              nullptr,

              &m_canvas,
              false,
          }

	, m_passwordBox
          {
              DIR_UPLEFT,
              409,
              540,

              146,
              18,

              true,

              2,
              18,
              0,

              colorf::WHITE_A255,

              2,
              colorf::WHITE_A255,

              [this]()
              {
                  m_idBox      .setFocus(true);
                  m_passwordBox.setFocus(false);
              },

              [this]()
              {
                  doLogin();
              },

              &m_canvas,
              false,
          }

    , m_buildSignature
      {
          DIR_UPLEFT,
          0,
          0,

          [](const Widget *)
          {
              return str_printf("编译版本号:%s", getBuildSignature());
          },

          1,
          14,
          0,

          colorf::YELLOW_A255,
          SDL_BLENDMODE_BLEND,

          &m_canvas,
          false,
      }

    , m_notifyBoardBg
      {
          DIR_UPLEFT,
          0, // need reset
          0, //
          0, //
          0, //

          [](const Widget *self, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(colorf::RGBA(0, 0,   0, 128), drawDstX, drawDstY, self->w(), self->h(), 8);
              g_sdlDevice->drawRectangle(colorf::RGBA(0, 0, 255, 128), drawDstX, drawDstY, self->w(), self->h(), 8);
          },

          &m_canvas,
          false,
      }

    , m_notifyBoard
      {
          DIR_NONE,
          [this]{ return m_canvas.w() / 2; },
          [this]{ return m_canvas.h() / 2; },
          0, // single line

          1,
          15,
          0,

          colorf::YELLOW_A255,

          5000,
          10,

          &m_canvas,
          false,
      }
{
    m_notifyBoard  .setShow([this]{ return !m_notifyBoard.empty(); });
    m_notifyBoardBg.setShow([this]{ return !m_notifyBoard.empty(); });

    m_notifyBoardBg.moveAt(DIR_UPLEFT, [this]{ return m_notifyBoard.dx() - 10; }, [this]{ return m_notifyBoard.dy() - 10; });
    m_notifyBoardBg.setSize(           [this]{ return m_notifyBoard. w() + 20; }, [this]{ return m_notifyBoard. h() + 20; });

    g_sdlDevice->playBGM(g_bgmDB->retrieve(0X00040007));
    if(g_clientArgParser->autoLogin.has_value()){
        sendLogin(g_clientArgParser->autoLogin.value().first, g_clientArgParser->autoLogin.value().second);
    }
}

void ProcessLogin::update(double fUpdateTime)
{
    m_canvas.update(fUpdateTime);
}

void ProcessLogin::draw() const
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000003),   0,  75);
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000004),   0, 465);
    g_sdlDevice->drawTexture(g_progUseDB->retrieve(0X00000011), 103, 536);

    m_canvas.drawRoot({});
}

void ProcessLogin::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            if(true
                                    && !m_idBox      .focus()
                                    && !m_passwordBox.focus()){

                                m_idBox      .setFocus(true);
                                m_passwordBox.setFocus(false);
                                return;
                            }
                        }
                    default:
                        {
                            break;
                        }
                }
            }
        default:
            {
                break;
            }
    }

    m_canvas.processRootEvent(event, true);
}

void ProcessLogin::doLogin()
{
    const auto idStr  = m_idBox.getRawString();
    const auto pwdStr = m_passwordBox.getPasswordString();

    if(idStr.empty() || pwdStr.empty()){
        m_notifyBoard.addLog(u8"无效的账号或密码");
    }
    else{
        // don't check id/password by idstf functions
        // this allows some test account like: (test, 123456)
        // but when creating account, changing password we need to be extremely careful

        sendLogin(idStr, pwdStr);
    }
}

void ProcessLogin::doCreateAccount()
{
    g_client->requestProcess(PROCESSID_CREATEACCOUNT);
}

void ProcessLogin::doChangePassword()
{
    g_client->requestProcess(PROCESSID_CHANGEPASSWORD);
}

void ProcessLogin::doExit()
{
    std::exit(0);
}

void ProcessLogin::sendLogin(const std::string &id, const std::string &password)
{
    CMLogin cmL;
    std::memset(&cmL, 0, sizeof(cmL));

    cmL.id.assign(id);
    cmL.password.assign(password);
    g_client->send({CM_LOGIN, cmL});
}
