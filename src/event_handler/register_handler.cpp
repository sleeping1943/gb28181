#include "register_handler.h"
#include <iostream>
#include "/usr/local/include/osipparser2/osip_message.h"
#include <string.h>

namespace Xzm
{

RegisterHandler::RegisterHandler()
{

}

RegisterHandler::~RegisterHandler()
{

}

bool RegisterHandler::Process(eXosip_event_t *evtp, int code)
{
    std::cout << "register_handler Process!!!" << std::endl;
    if (MSG_IS_REGISTER(evtp->request)) {
        std::cout << "register msg!!" << std::endl;
    } else if (MSG_IS_MESSAGE(evtp->request)) {
        std::cout << "message msg!!" << std::endl;
    } else if (MSG_IS_BYE(evtp->request)) {
        std::cout << "bye message msg!!" << std::endl;
    } else if (strncmp(evtp->request->sip_method, "BYE", 3) != 0) {
        std::cout << "bye" << std::endl;
    } else {
        std::cout << "unsuported msg!!" << std::endl;
    }
    return true;
}
};