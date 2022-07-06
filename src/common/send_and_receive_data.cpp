#include "common/send_and_receive_data.h"
#include "common/blocking_tcp_client.h"

std::string send_and_receive_data(std::string IP_address,std::string data2, int send_or_receive_socket_data_timeout_settings)
{
  // Variables
  std::string string;
  auto connection_timeout = boost::posix_time::milliseconds(CONNECTION_TIMEOUT_SETTINGS);
  auto send_and_receive_data_timeout = boost::posix_time::milliseconds(send_or_receive_socket_data_timeout_settings);

  try
  {
    // add the end string to the data
    data2 += SOCKET_END_STRING;

    client c;
    c.connect(IP_address, SEND_DATA_PORT, connection_timeout);
    
    // send the message and read the response
    c.write_line(data2, send_and_receive_data_timeout);
    string = c.read_until('}', send_and_receive_data_timeout);
  }
  catch (std::exception &ex)
  {
    return "";
  }
  return string;
}

std::string send_and_receive_http_data(std::string data2, int send_or_receive_socket_data_timeout_settings)
{
  // define macros
  #define HOST "turbotx.xcash.foundation"
  #define TURBO_TX_URL "http://" HOST "/"

  // Variables
  std::string string;
  auto connection_timeout = boost::posix_time::milliseconds(CONNECTION_TIMEOUT_SETTINGS);
  auto send_and_receive_data_timeout = boost::posix_time::milliseconds(send_or_receive_socket_data_timeout_settings);

  try
  {
    // create the post request data
    string = "POST /processturbotx?" + data2 + " HTTP/1.1\r\nHost: " HOST "\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n";

    client c;
    c.connect(HOST, "80", connection_timeout);
    
    // send the message and read the response
    c.write_line(string, send_and_receive_data_timeout);
    string = c.read_until('}', send_and_receive_data_timeout);

    string = string.substr(string.find(TURBO_TX_URL));
  }
  catch (std::exception &ex)
  {
    return "";
  }
  return string;

  #undef HOST
  #undef TURBO_TX_URL
}
