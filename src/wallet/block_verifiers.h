#pragma once

#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <string>

#include "common/scoped_message_writer.h"
#include "cryptonote_config.h"
#include "../contrib/epee/include/misc_log_ex.h"
#include "common/send_and_receive_data.h"

using namespace epee;

void sync_minutes_and_seconds(const int SETTINGS, const bool display_settings);
std::string get_current_block_verifiers_list();
