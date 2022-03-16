#pragma once

#include <thread>
#include <future>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/thread.hpp>
#include <boost/system/system_error.hpp>
#include <cstdlib>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>

#include "cryptonote_config.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_protocol/cryptonote_protocol_defs.h"
#include "remote_data.h"
#include "common/global_variables.h"
#include "common/send_and_receive_data.h"
#include "string_tools.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_protocol/cryptonote_protocol_defs.h"
#include "common/scoped_message_writer.h"
#include "../contrib/epee/include/misc_log_ex.h"

using namespace epee;

void remote_data_sync_minutes_and_seconds(const int settings, const bool display_settings);
std::string get_remote_data_address_settings(std::string public_address);
bool process_remote_data_transactions(const uint64_t height, const std::string remote_data_address_settings,const cryptonote::transaction& tx);
std::string get_address_from_name(const std::string name);
