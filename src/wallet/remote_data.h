#include <thread>
#include <future>
#include <algorithm>
#include <cstdio>
#include <cmath>
#include <stdlib.h>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/asio.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/thread.hpp>

#include <boost/asio/connect.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/system/system_error.hpp>
#include <boost/asio/write.hpp>
#include <cstdlib>
#include <iostream>
#include <string>

using boost::asio::deadline_timer;
using boost::asio::ip::tcp;

#include "cryptonote_config.h"

void process_remote_data_transactions(uint64_t height);
