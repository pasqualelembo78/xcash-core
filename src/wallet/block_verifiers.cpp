#include "block_verifiers.h"

void sync_minutes_and_seconds(const int SETTINGS, const bool display_settings)
{
  // Variables
  std::time_t current_date_and_time;
  std::tm* current_UTC_date_and_time;

  if (SETTINGS == 0)
  {
    !display_settings ? std::cout << "Waiting until the next valid data interval, this will be less than 5 minutes. Please leave the wallet open until this time and you receive a confirmation" : tools::scoped_message_writer(console_color_yellow, false) << "Waiting until the next valid data interval, this will be less than 5 minutes. Please leave the wallet open until this time and you receive a confirmation";

    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      current_date_and_time = std::time(0);
      current_UTC_date_and_time = std::gmtime(&current_date_and_time);
    } while (current_UTC_date_and_time->tm_min % BLOCK_TIME != 2 && current_UTC_date_and_time->tm_min % BLOCK_TIME != 3);  
  }
  else
  {
    !display_settings ? std::cout << "Sending the vote at the beginning of the hour. Please leave the wallet open until this time and you receive a confirmation" : tools::scoped_message_writer(console_color_yellow, false) << "Sending the vote at the beginning of the hour. Please leave the wallet open until this time and you receive a confirmation";

    do
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      current_date_and_time = std::time(0);
      current_UTC_date_and_time = std::gmtime(&current_date_and_time);
    } while (current_UTC_date_and_time->tm_min != 3); 
  }

  // wait a random amount of time, so all messages from delegates that have been waiting dont get sent at the same time
  std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (SOCKET_CONNECTION_MAXIMUM_BUFFER_SETTINGS - SOCKET_CONNECTION_MINIMUM_BUFFER_SETTINGS + 1) + SOCKET_CONNECTION_MINIMUM_BUFFER_SETTINGS));
  return;
}



std::string get_current_block_verifiers_list()
{
  // structures
  struct network_data_nodes_list {
    std::string network_data_nodes_public_address[NETWORK_DATA_NODES_AMOUNT]; // The network data nodes public address
    std::string network_data_nodes_IP_address[NETWORK_DATA_NODES_AMOUNT]; // The network data nodes IP address
};

  // Variables
  std::string string = "";
  struct network_data_nodes_list network_data_nodes_list; // The network data nodes
  std::size_t count = 0;
  int random_network_data_node;
  int network_data_nodes_array[NETWORK_DATA_NODES_AMOUNT];

  // define macros
  #define MESSAGE "{\r\n \"message_settings\": \"NODE_TO_NETWORK_DATA_NODES_GET_CURRENT_BLOCK_VERIFIERS_LIST\",\r\n}"

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

    // get the block verifiers list from the network data node
    string = send_and_receive_data(network_data_nodes_list.network_data_nodes_IP_address[random_network_data_node-1],MESSAGE);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return count == NETWORK_DATA_NODES_AMOUNT || string.find("\"block_verifiers_IP_address_list\": \"") == std::string::npos ? "" : string;

  #undef MESSAGE
}
