// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.


#include "FormatTools.h"
#include <cstdio>
#include <ctime>
#include "CryptoNoteCore/Core.h"
#include "Rpc/CoreRpcServerCommandsDefinitions.h"
#include <boost/format.hpp>

namespace Common {

//--------------------------------------------------------------------------------
std::string get_mining_speed(uint32_t hr) {
  if (hr>1e9) return (boost::format("%.2f GH/s") % (hr/1e9)).str();
  if (hr>1e6) return (boost::format("%.2f MH/s") % (hr/1e6)).str();
  if (hr>1e3) return (boost::format("%.2f kH/s") % (hr/1e3)).str();

  return (boost::format("%.0f H/s") % hr).str();
}

//--------------------------------------------------------------------------------
std::string get_sync_percentage(uint64_t height, uint64_t target_height) {
  target_height = target_height ? target_height < height ? height : target_height : height;
  float pc = 100.0f * height / target_height;

  if (height < target_height && pc > 99.99f) {
    pc = 99.99f; // to avoid 100% when not fully synced
  }

  return (boost::format("%.2f") % pc).str();
}

//--------------------------------------------------------------------------------
std::string get_upgrade_time(uint64_t height, std::vector<uint64_t> upgrade_heights, uint64_t supported_height) {
  std::string supported = "(Your version of the software is ready for this fork)";
  std::string ret;

  uint64_t next_fork = 0;

  for (auto upgrade : upgrade_heights)
  {
      /* We have hit an upgrade already that the user cannot support */
      if (height >= upgrade && supported_height < upgrade)
      {
          return " (The network forked at height " + std::to_string(upgrade) + ", please update your software: https://turtlecoin.lol/)";
      }

      /* Get the next fork height */
      if (upgrade > height)
      {
          next_fork = upgrade;
          break;
      }
  }

  /* Software doesn't support the next fork yet */
  if (supported_height < next_fork)
  {
      supported = " (Your software will need to be upgraded to support this fork)";
  }

  /* No more hardforks on the list, don't print anything */
  if (height > next_fork)
  {
      return ret;
  }

  float days = (next_fork - height) / CryptoNote::parameters::EXPECTED_NUMBER_OF_BLOCKS_PER_DAY;

  if (height == next_fork)
  {
      ret = " (forking now) ";
  }
  else if (days < 1)
  {
      ret = (boost::format(" (next fork in %.1f hours) ") % (days * 24)).str();
  }
  else
  {
      ret = (boost::format(" (next fork in %.1f days) ") % days).str();
  }

  return ret + supported;
}

//--------------------------------------------------------------------------------
std::string get_status_string(CryptoNote::COMMAND_RPC_GET_INFO::response iresp) {
  std::stringstream ss;
  std::time_t uptime = std::time(nullptr) - iresp.start_time;

  ss << "Height: " << iresp.height << "/" << iresp.network_height
     << " (" << get_sync_percentage(iresp.height, iresp.network_height) << "%) "
     << "on " << (iresp.testnet ? "testnet, " : "mainnet, ")
     << (iresp.synced ? "synced, " : "syncing, ") 
     << "net hash " << get_mining_speed(iresp.hashrate) << ", "
     << "v" << +iresp.major_version
     << get_upgrade_time(iresp.network_height, iresp.upgrade_heights, iresp.supported_height)
     << ", " << iresp.outgoing_connections_count << "(out)+" << iresp.incoming_connections_count << "(in) connections, "
     << "uptime " << (unsigned int)floor(uptime / 60.0 / 60.0 / 24.0)
     << "d " << (unsigned int)floor(fmod((uptime / 60.0 / 60.0), 24.0))
     << "h " << (unsigned int)floor(fmod((uptime / 60.0), 60.0))
     << "m " << (unsigned int)fmod(uptime, 60.0) << "s";

  return ss.str();
}

}
