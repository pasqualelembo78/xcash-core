// Copyright (c) 2018 X-CASH Project, Derived from 2014-2018, The Monero Project
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Parts of this file are originally copyright (c) 2012-2013 The Cryptonote developers

#include <sstream>
#include <numeric>
#include <boost/utility/value_init.hpp>
#include <boost/interprocess/detail/atomic.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/limits.hpp>
#include "include_base_utils.h"
#include "misc_language.h"
#include "syncobj.h"
#include "cryptonote_basic_impl.h"
#include "cryptonote_format_utils.h"
#include "file_io_utils.h"
#include "common/command_line.h"
#include "string_coding.h"
#include "string_tools.h"
#include "storages/portable_storage_template_helper.h"
#include "boost/logic/tribool.hpp"

#ifdef __APPLE__
  #include <sys/times.h>
  #include <IOKit/IOKitLib.h>
  #include <IOKit/ps/IOPSKeys.h>
  #include <IOKit/ps/IOPowerSources.h>
  #include <mach/mach_host.h>
  #include <AvailabilityMacros.h>
  #include <TargetConditionals.h>
#endif

#ifdef __FreeBSD__
#include <devstat.h>
#include <errno.h>
#include <fcntl.h>
#include <machine/apm_bios.h>
#include <stdio.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/times.h>
#include <sys/types.h>
#include <unistd.h>
#endif

#undef XCASH_DEFAULT_LOG_CATEGORY
#define XCASH_DEFAULT_LOG_CATEGORY "miner"

using namespace epee;

#include "miner.h"


extern "C" void slow_hash_allocate_state();
extern "C" void slow_hash_free_state();
namespace cryptonote
{

  namespace
  {
    const command_line::arg_descriptor<std::string> arg_extra_messages =  {"extra-messages-file", "Specify file for extra messages to include into coinbase transactions", "", true};
    
  }


  miner::miner(i_miner_handler* phandler):m_stop(1),
    m_template(boost::value_initialized<block>()),
    m_template_no(0),
    m_diffic(0),
    m_thread_index(0),
    m_phandler(phandler),
    m_height(0),
    m_pausers_count(0),
    m_threads_total(0),
    m_starter_nonce(0),
    m_last_hr_merge_time(0),
    m_hashes(0),
    m_current_hash_rate(0),
    m_min_idle_seconds(BACKGROUND_MINING_DEFAULT_MIN_IDLE_INTERVAL_IN_SECONDS),
    m_idle_threshold(BACKGROUND_MINING_DEFAULT_IDLE_THRESHOLD_PERCENTAGE),
    m_miner_extra_sleep(BACKGROUND_MINING_DEFAULT_MINER_EXTRA_SLEEP_MILLIS)
  {

  }
  //-----------------------------------------------------------------------------------------------------
  miner::~miner()
  {
  }
  //-----------------------------------------------------------------------------------------------------
  bool miner::set_block_template(const block& bl, const difficulty_type& di, uint64_t height)
  {
    CRITICAL_REGION_LOCAL(m_template_lock);
    m_template = bl;
    m_diffic = di;
    m_height = height;
    ++m_template_no;
    m_starter_nonce = crypto::rand<uint32_t>();
    return true;
  }
  //-----------------------------------------------------------------------------------------------------
  bool miner::on_block_chain_update()
  {
      return true;
  }
  
  //-----------------------------------------------------------------------------------------------------
  void miner::merge_hr()
  {
    m_last_hr_merge_time = misc_utils::get_tick_count();
    m_hashes = 0;
  }
  //-----------------------------------------------------------------------------------------------------
  void miner::init_options(boost::program_options::options_description& desc)
  {
    command_line::add_arg(desc, arg_extra_messages);
  }
  //-----------------------------------------------------------------------------------------------------
  bool miner::init(const boost::program_options::variables_map& vm, network_type nettype)
  {
    if(command_line::has_arg(vm, arg_extra_messages))
    {
      std::string buff;
      bool r = file_io_utils::load_file_to_string(command_line::get_arg(vm, arg_extra_messages), buff);
      CHECK_AND_ASSERT_MES(r, false, "Failed to load file with extra messages: " << command_line::get_arg(vm, arg_extra_messages));
      std::vector<std::string> extra_vec;
      boost::split(extra_vec, buff, boost::is_any_of("\n"), boost::token_compress_on );
      m_extra_messages.resize(extra_vec.size());
      for(size_t i = 0; i != extra_vec.size(); i++)
      {
        string_tools::trim(extra_vec[i]);
        if(!extra_vec[i].size())
          continue;
        std::string buff = string_encoding::base64_decode(extra_vec[i]);
        if(buff != "0")
          m_extra_messages[i] = buff;
      }
      m_config_folder_path = boost::filesystem::path(command_line::get_arg(vm, arg_extra_messages)).parent_path().string();
      m_config = AUTO_VAL_INIT(m_config);
      epee::serialization::load_t_from_json_file(m_config, m_config_folder_path + "/" + MINER_CONFIG_FILE_NAME);
      MINFO("Loaded " << m_extra_messages.size() << " extra messages, current index " << m_config.current_extra_message_index);
    }


    

    return true;
  }
  
  
  //-----------------------------------------------------------------------------------------------------
  bool miner::find_nonce_for_given_block(block& bl, const difficulty_type& diffic, uint64_t height)
  {
    for(; bl.nonce != std::numeric_limits<uint32_t>::max(); bl.nonce++)
    {
      crypto::hash h;
      get_block_longhash(bl, h, height);

      if(check_hash(h, diffic))
      {
        bl.invalidate_hashes();
        return true;
      }
    }
    bl.invalidate_hashes();
    return false;
  }
  //-----------------------------------------------------------------------------------------------------
  void miner::on_synchronized()
  {
    
  }
  //-----------------------------------------------------------------------------------------------------
  void miner::pause()
  {
    CRITICAL_REGION_LOCAL(m_miners_count_lock);
    MDEBUG("miner::pause: " << m_pausers_count << " -> " << (m_pausers_count + 1));
    ++m_pausers_count;
    
  }
  //-----------------------------------------------------------------------------------------------------
  void miner::resume()
  {
    CRITICAL_REGION_LOCAL(m_miners_count_lock);
    MDEBUG("miner::resume: " << m_pausers_count << " -> " << (m_pausers_count - 1));
    --m_pausers_count;
    if(m_pausers_count < 0)
    {
      m_pausers_count = 0;
      MERROR("Unexpected miner::resume() called");
    }
    
  }
  
  
 
  //-----------------------------------------------------------------------------------------------------
  uint64_t miner::get_min_idle_seconds() const
  {
    return m_min_idle_seconds;
  }
  //-----------------------------------------------------------------------------------------------------
  bool miner::set_min_idle_seconds(uint64_t min_idle_seconds)
  {
    if(min_idle_seconds > BACKGROUND_MINING_MAX_MIN_IDLE_INTERVAL_IN_SECONDS) return false;
    if(min_idle_seconds < BACKGROUND_MINING_MIN_MIN_IDLE_INTERVAL_IN_SECONDS) return false;
    m_min_idle_seconds = min_idle_seconds;
    return true;
  }
  //-----------------------------------------------------------------------------------------------------
  uint8_t miner::get_idle_threshold() const
  {
    return m_idle_threshold;
  }
  //-----------------------------------------------------------------------------------------------------
  bool miner::set_idle_threshold(uint8_t idle_threshold)
  {
    if(idle_threshold > BACKGROUND_MINING_MAX_IDLE_THRESHOLD_PERCENTAGE) return false;
    if(idle_threshold < BACKGROUND_MINING_MIN_IDLE_THRESHOLD_PERCENTAGE) return false;
    m_idle_threshold = idle_threshold;
    return true;
  }
 
  //-----------------------------------------------------------------------------------------------------
  bool miner::get_system_times(uint64_t& total_time, uint64_t& idle_time)
  {
    #ifdef _WIN32

    	FILETIME idleTime;
    	FILETIME kernelTime;
    	FILETIME userTime;
    	if ( GetSystemTimes( &idleTime, &kernelTime, &userTime ) != -1 )
    	{
        total_time =
          ( (((uint64_t)(kernelTime.dwHighDateTime)) << 32) | ((uint64_t)kernelTime.dwLowDateTime) )
          + ( (((uint64_t)(userTime.dwHighDateTime)) << 32) | ((uint64_t)userTime.dwLowDateTime) );

        idle_time = ( (((uint64_t)(idleTime.dwHighDateTime)) << 32) | ((uint64_t)idleTime.dwLowDateTime) );

        return true;
    	}

    #elif defined(__linux__)

      const std::string STAT_FILE_PATH = "/proc/stat";

      if( !epee::file_io_utils::is_file_exist(STAT_FILE_PATH) )
      {
        LOG_ERROR("'" << STAT_FILE_PATH << "' file does not exist");
        return false;
      }

      std::ifstream stat_file_stream(STAT_FILE_PATH);
      if( stat_file_stream.fail() )
      {
        LOG_ERROR("failed to open '" << STAT_FILE_PATH << "'");
        return false;
      }

      std::string line;
      std::getline(stat_file_stream, line);
      std::istringstream stat_file_iss(line);
      stat_file_iss.ignore(65536, ' '); // skip cpu label ...
      uint64_t utime, ntime, stime, itime;
      if( !(stat_file_iss >> utime && stat_file_iss >> ntime && stat_file_iss >> stime && stat_file_iss >> itime) )
      {
        LOG_ERROR("failed to read '" << STAT_FILE_PATH << "'");
        return false;
      }

      idle_time = itime;
      total_time = utime + ntime + stime + itime;

      return true;

    #elif defined(__APPLE__)

      mach_msg_type_number_t count;
      kern_return_t status;
      host_cpu_load_info_data_t stats;      
      count = HOST_CPU_LOAD_INFO_COUNT;
      status = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&stats, &count);
      if(status != KERN_SUCCESS)
      {
        return false;
      }

      idle_time = stats.cpu_ticks[CPU_STATE_IDLE];
      total_time = idle_time + stats.cpu_ticks[CPU_STATE_USER] + stats.cpu_ticks[CPU_STATE_SYSTEM];
      
      return true;

    #elif defined(__FreeBSD__)

      struct statinfo s;
      size_t n = sizeof(s.cp_time);
      if( sysctlbyname("kern.cp_time", s.cp_time, &n, NULL, 0) == -1 )
      {
        LOG_ERROR("sysctlbyname(\"kern.cp_time\"): " << strerror(errno));
        return false;
      }
      if( n != sizeof(s.cp_time) )
      {
        LOG_ERROR("sysctlbyname(\"kern.cp_time\") output is unexpectedly "
          << n << " bytes instead of the expected " << sizeof(s.cp_time)
          << " bytes.");
        return false;
      }

      idle_time = s.cp_time[CP_IDLE];
      total_time =
        s.cp_time[CP_USER] +
        s.cp_time[CP_NICE] +
        s.cp_time[CP_SYS] +
        s.cp_time[CP_INTR] +
        s.cp_time[CP_IDLE];

      return true;

    #endif

    return false; // unsupported system
  }
  //-----------------------------------------------------------------------------------------------------
  bool miner::get_process_time(uint64_t& total_time)
  {
    #ifdef _WIN32

      FILETIME createTime;
      FILETIME exitTime;
      FILETIME kernelTime;
      FILETIME userTime;
      if ( GetProcessTimes( GetCurrentProcess(), &createTime, &exitTime, &kernelTime, &userTime ) != -1 )
      {
        total_time =
          ( (((uint64_t)(kernelTime.dwHighDateTime)) << 32) | ((uint64_t)kernelTime.dwLowDateTime) )
          + ( (((uint64_t)(userTime.dwHighDateTime)) << 32) | ((uint64_t)userTime.dwLowDateTime) );

        return true;
      }

    #elif (defined(__linux__) && defined(_SC_CLK_TCK)) || defined(__APPLE__) || defined(__FreeBSD__)

      struct tms tms;
      if ( times(&tms) != (clock_t)-1 )
      {
    		total_time = tms.tms_utime + tms.tms_stime;
        return true;
      }

    #endif

    return false; // unsupported system
  }
  //-----------------------------------------------------------------------------------------------------  
  uint8_t miner::get_percent_of_total(uint64_t other, uint64_t total)
  {
    return (uint8_t)( ceil( (other * 1.f / total * 1.f) * 100) );    
  }
  
}

