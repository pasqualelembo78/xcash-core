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

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <unordered_map>

#include "include_base_utils.h"
using namespace epee;
#include "wallet/wallet2.h"
using namespace cryptonote;

namespace
{
  uint64_t const TEST_FEE = 5000000000; // 5 * 10^9
  uint64_t const TEST_DUST_THRESHOLD = 5000000000; // 5 * 10^9
}

std::string generate_random_wallet_name()
{
  std::stringstream ss;
  ss << boost::uuids::random_generator()();
  return ss.str();
}

inline uint64_t random(const uint64_t max_value) {
  return (uint64_t(rand()) ^
          (uint64_t(rand())<<16) ^
          (uint64_t(rand())<<32) ^
          (uint64_t(rand())<<48)) % max_value;
}

bool do_send_money(tools::wallet2& w1, tools::wallet2& w2, size_t mix_in_factor, uint64_t amount_to_transfer, transaction& tx, size_t parts=1)
{
  CHECK_AND_ASSERT_MES(parts > 0, false, "parts must be > 0");

  std::vector<cryptonote::tx_destination_entry> dsts;
  dsts.reserve(parts);
  uint64_t amount_used = 0;
  uint64_t max_part = amount_to_transfer / parts;

  for (size_t i = 0; i < parts; ++i)
  {
    cryptonote::tx_destination_entry de;
    de.addr = w2.get_account().get_keys().m_account_address;

    if (i < parts - 1)
      de.amount = random(max_part);
    else
      de.amount = amount_to_transfer - amount_used;
    amount_used += de.amount;

    //std::cout << "PARTS (" << amount_to_transfer << ") " << amount_used << " " << de.amount << std::endl;

    dsts.push_back(de);
  }

  try
  {
    std::vector<tools::wallet2::pending_tx> ptx;
    ptx = w1.create_transactions_2(dsts, mix_in_factor, 0, "private", 0, std::vector<uint8_t>(), 0, {});
    for (auto &p: ptx)
      w1.commit_tx(p);
    return true;
  }
  catch (const std::exception&)
  {
    return false;
  }
}

uint64_t get_money_in_first_transfers(const tools::wallet2::transfer_container& incoming_transfers, size_t n_transfers)
{
  uint64_t summ = 0;
  size_t count = 0;
  BOOST_FOREACH(const tools::wallet2::transfer_details& td, incoming_transfers)
  {
    summ += td.m_tx.vout[td.m_internal_output_index].amount;
    if(++count >= n_transfers)
      return summ;
  }
  return summ;
}

#define FIRST_N_TRANSFERS 10*10

bool transactions_flow_test(std::string& working_folder,
  std::string path_source_wallet,
  std::string path_target_wallet,
  std::string& daemon_addr_a,
  std::string& daemon_addr_b,
  uint64_t amount_to_transfer, size_t mix_in_factor, size_t transactions_count, size_t transactions_per_second)
{
  return true;
}

