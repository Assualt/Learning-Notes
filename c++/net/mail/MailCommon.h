#pragma once
#include "base/fsm/fsm_stateconfigure.h"

namespace mail {
enum class MailState {
    STATE_WELCOME,
    STATE_HELO,
    STATE_EHLO,
    STATE_AUTH,
    STATE_AUTHPASS,
    STATE_AUTHEND,
    STATE_MAILFROM,
    STATE_RCPTTO,
    STATE_DATA,
    STATE_DATAFINISH,
    STATE_DISCONNECT,
    STATE_REST,
};

enum class MailEvent {
    EHLO,
    HELO,
    MAIL_FROM,
    RCPT_TO,
    DATA,
    RESET
};
}