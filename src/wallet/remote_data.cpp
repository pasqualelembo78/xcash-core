#include "remote_data.h"

std::string current_public_address;

void remote_data_sync_minutes_and_seconds(const int settings, const bool display_settings)
{
  // Variables
  std::time_t current_date_and_time;
  std::tm* current_UTC_date_and_time;

  !display_settings ? std::cout << "Waiting until the next valid data interval, this will be less than 10 minutes. Please leave the wallet open until this time and you receive a confirmation" : tools::scoped_message_writer(console_color_yellow, false) << "Waiting until the next valid data interval, this will be less than 10 minutes. Please leave the wallet open until this time and you receive a confirmation";

  do
  {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    current_date_and_time = std::time(0);
    current_UTC_date_and_time = std::gmtime(&current_date_and_time);
  } while (current_UTC_date_and_time->tm_min % 60 >= 0 && current_UTC_date_and_time->tm_min % 60 < 10); 

  // check for each block producer change
  if (settings == 1)
  {
    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      current_date_and_time = std::time(0);
      current_UTC_date_and_time = std::gmtime(&current_date_and_time);
    } while (current_UTC_date_and_time->tm_min % BLOCK_TIME != 1 && current_UTC_date_and_time->tm_min % BLOCK_TIME != 2 && current_UTC_date_and_time->tm_min % BLOCK_TIME != 3);
  } 
 
  // wait a random amount of time, so all messages from delegates that have been waiting dont get sent at the same time
  std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (SOCKET_CONNECTION_MAXIMUM_BUFFER_SETTINGS - SOCKET_CONNECTION_MINIMUM_BUFFER_SETTINGS + 1) + SOCKET_CONNECTION_MINIMUM_BUFFER_SETTINGS));
  return;
}

std::string get_remote_data_address_settings(std::string public_address)
{
  // structures
  struct network_data_nodes_list {
    std::string network_data_nodes_public_address[NETWORK_DATA_NODES_AMOUNT]; // The network data nodes public address
    std::string network_data_nodes_IP_address[NETWORK_DATA_NODES_AMOUNT]; // The network data nodes IP address
};

  // Variables
  std::string string = "";
  std::string data = "";
  struct network_data_nodes_list network_data_nodes_list; // The network data nodes
  std::size_t count = 0;
  int random_network_data_node;
  int network_data_nodes_array[NETWORK_DATA_NODES_AMOUNT];
  
 
    if (public_address.length() != XCASH_WALLET_LENGTH || public_address.substr(0,sizeof(XCASH_WALLET_PREFIX)-1) != XCASH_WALLET_PREFIX)
    {
      exit(0); 
    }

    // create the data
    data = "NODE_TO_NETWORK_DATA_NODES_GET_ADDRESS_SETTINGS|" + public_address + "|";
    
    // initialize the network_data_nodes_list struct
    INITIALIZE_NETWORK_DATA_NODES_LIST_STRUCT;

  // send the message to a random network data node
  for (count = 0; string.find("|") == std::string::npos && count < NETWORK_DATA_NODES_AMOUNT; count++)
  {
    // check if they need to reset the network_data_nodes_array
    if (network_data_nodes_array[NETWORK_DATA_NODES_AMOUNT-1] != 0)
    {
      std::fill(network_data_nodes_array, network_data_nodes_array+NETWORK_DATA_NODES_AMOUNT, 0);
    }

    do
    {
      // get a random network data node
      random_network_data_node = (int)(rand() % NETWORK_DATA_NODES_AMOUNT + 1);
    } while (std::any_of(std::begin(network_data_nodes_array), std::end(network_data_nodes_array), [&](int number){return number == random_network_data_node;}));

    network_data_nodes_array[count] = random_network_data_node;

    // get the address settings from the network data node
    string = send_and_receive_data(network_data_nodes_list.network_data_nodes_IP_address[random_network_data_node-1],data);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  if (string.find("Error:") != std::string::npos)
  {
    exit(0); 
  }

  return string;
}



bool process_remote_data_transactions(const uint64_t height, const std::string remote_data_address_settings,const cryptonote::transaction& tx)
{
   // get the tx etra
   std::string public_tx_extra_str = string_tools::buff_to_hex_nodelimer(std::string(reinterpret_cast<const char*>(tx.extra.data()), tx.extra.size()));

   if (remote_data_address_settings == "saddress" && public_tx_extra_str.find("7c5369675631") != std::string::npos && public_tx_extra_str.find("7c584341") != std::string::npos)
   {
     // block all public tx
     return true;
   }
   else if (remote_data_address_settings == "paddress" && public_tx_extra_str.find("7c5369675631") == std::string::npos && public_tx_extra_str.find("7c584341") == std::string::npos)
   {
     // block all private tx
     return true;
   }
   return false;
}



std::string get_address_from_name(const std::string name)
{
  // structures
  struct network_data_nodes_list {
    std::string network_data_nodes_public_address[NETWORK_DATA_NODES_AMOUNT]; // The network data nodes public address
    std::string network_data_nodes_IP_address[NETWORK_DATA_NODES_AMOUNT]; // The network data nodes IP address
};

  // Variables
  std::string string = "";
  std::string data = "";
  struct network_data_nodes_list network_data_nodes_list; // The network data nodes
  std::size_t count = 0;
  int random_network_data_node;
  int network_data_nodes_array[NETWORK_DATA_NODES_AMOUNT];

  if (name.length() > 50 || name.length() < 1 || (name.find(".xcash") == std::string::npos && name.find(".sxcash") == std::string::npos && name.find(".pxcash") == std::string::npos))
  {
    return "Name is invalid"; 
  }

  // get the name settings
  string = name.substr(name.length() - (name.length() - name.find("."))); 

  if (string != ".xcash" && string != ".sxcash" && string != ".pxcash")
  {
    return "Name is invalid";
  } 

    // create the data
    data = "NODE_TO_NETWORK_DATA_NODES_GET_ADDRESS_FROM_NAME|" + name + "|";
    
    // initialize the network_data_nodes_list struct
    INITIALIZE_NETWORK_DATA_NODES_LIST_STRUCT;

  // send the message to a random network data node
  for (count = 0; string.find("|") == std::string::npos && count < NETWORK_DATA_NODES_AMOUNT; count++)
  {
    // check if they need to reset the network_data_nodes_array
    if (network_data_nodes_array[NETWORK_DATA_NODES_AMOUNT-1] != 0)
    {
      std::fill(network_data_nodes_array, network_data_nodes_array+NETWORK_DATA_NODES_AMOUNT, 0);
    }

    do
    {
      // get a random network data node
      random_network_data_node = (int)(rand() % NETWORK_DATA_NODES_AMOUNT + 1);
    } while (std::any_of(std::begin(network_data_nodes_array), std::end(network_data_nodes_array), [&](int number){return number == random_network_data_node;}));

    network_data_nodes_array[count] = random_network_data_node;

    // get the address settings from the network data node
    string = send_and_receive_data(network_data_nodes_list.network_data_nodes_IP_address[random_network_data_node-1],data);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  if (string.find("Error:") != std::string::npos)
  {
    return "The name is not registered"; 
  }

  return string;
}



std::string remote_data_display_remote_data(std::string name)
{
  // structures
  struct network_data_nodes_list {
    std::string network_data_nodes_public_address[NETWORK_DATA_NODES_AMOUNT]; // The network data nodes public address
    std::string network_data_nodes_IP_address[NETWORK_DATA_NODES_AMOUNT]; // The network data nodes IP address
};

  // Variables
  std::string string = "";
  std::string data = "";
  struct network_data_nodes_list network_data_nodes_list; // The network data nodes
  std::size_t count = 0;
  int random_network_data_node;
  int network_data_nodes_array[NETWORK_DATA_NODES_AMOUNT];

  if (name.length() > 50 || name.length() < 1 || (name.find(".xcash") == std::string::npos && name.find(".sxcash") == std::string::npos && name.find(".pxcash") == std::string::npos))
  {
    return "Name is invalid"; 
  }

  // get the name
  data = name.substr(0,name.find("."));

    // create the data
    data = "NODE_TO_NETWORK_DATA_NODES_GET_INFORMATION_FROM_NAME|" + data + "|";
    
    // initialize the network_data_nodes_list struct
    INITIALIZE_NETWORK_DATA_NODES_LIST_STRUCT;

  // send the message to a random network data node
  for (count = 0; string.find("|") == std::string::npos && count < NETWORK_DATA_NODES_AMOUNT; count++)
  {
    // check if they need to reset the network_data_nodes_array
    if (network_data_nodes_array[NETWORK_DATA_NODES_AMOUNT-1] != 0)
    {
      std::fill(network_data_nodes_array, network_data_nodes_array+NETWORK_DATA_NODES_AMOUNT, 0);
    }

    do
    {
      // get a random network data node
      random_network_data_node = (int)(rand() % NETWORK_DATA_NODES_AMOUNT + 1);
    } while (std::any_of(std::begin(network_data_nodes_array), std::end(network_data_nodes_array), [&](int number){return number == random_network_data_node;}));

    network_data_nodes_array[count] = random_network_data_node;

    // get the address settings from the network data node
    string = send_and_receive_data(network_data_nodes_list.network_data_nodes_IP_address[random_network_data_node-1],data);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  if (string.find("Error:") != std::string::npos)
  {
    return "The name is not registered"; 
  }

  return string;
}
