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

#include "cryptonote_config.h"
#include "cryptonote_basic/cryptonote_basic.h"
#include "cryptonote_protocol/cryptonote_protocol_defs.h"

std::string get_remote_data_address_settings(std::string public_address);
bool process_remote_data_transactions(const uint64_t height, const std::string remote_data_address_settings,const cryptonote::transaction& tx);
std::string get_address_from_name(const std::string name);
std::string remote_data_display_remote_data(std::string name);
