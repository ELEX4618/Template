///////////////////////////////////////////////////////////////////
// Prepared for BCIT ELEX4618, April 2017, by Craig Hennessey
///////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <iostream>

#include "Client.h"

#pragma comment(lib, "ws2_32.lib")

Client::Client(int port, std::string addr)
{
  _socket = NULL;

  sockaddr_in ipaddr;
  sockaddr_in client_addr;
  SOCKET subsock = NULL;

  u_long polling = 1;

  if (WSAStartup(0x0101, &_wsdat))
  {
    WSACleanup();
    return;
  }

  _socket = socket(AF_INET, SOCK_STREAM, 0);
  if (_socket == SOCKET_ERROR)
  {
    WSACleanup();
    return;
  }

  if (ioctlsocket(_socket, FIONBIO, &polling) == SOCKET_ERROR)
  {
    WSACleanup();
    return;
  }

  ipaddr.sin_family = AF_INET;
  ipaddr.sin_port = htons(port);
  ipaddr.sin_addr.S_un.S_addr = inet_addr(addr.c_str());

  connect(_socket, (struct sockaddr*)&ipaddr, sizeof(ipaddr));  
}

Client::~Client()
{
	if (_socket != NULL) 
	{
		closesocket(_socket);
	}

	Sleep (50);
	WSACleanup();
}

void Client::tx_str (std::string str)
{
  send(_socket, str.c_str(), str.length(), 0);
}

bool Client::rx_str (std::string &str)
{
  int rxbytes = recv(_socket, rxbuff, BUFF_SIZE-1, 0); // recvfrom
  
  if (rxbytes > 0)
  {
    rxbuff[rxbytes] = 0; // Add NULL
    str = rxbuff;
    return true;
  }

  return false;
}

// Note probably would work just as well in blocking mode and no need for do/while loops (but then could not do auto-reconnect)
bool Client::rx_im(cv::Mat &im)
{
  int rxbytes;
  int imsize;
  int64 start_time;
  float elapsedtime;

  // Get size of image in bytes
  start_time = cv::getTickCount();
  elapsedtime = 0;
  do
  {
    rxbytes = recv(_socket, (char *)&imsize, sizeof(imsize), 0);
    elapsedtime = (float)(cv::getTickCount() - start_time) / (float)cv::getTickFrequency();
  } 
  while (rxbytes < 0 && elapsedtime < 1.0); // Timeout after 1 second
  
  // If data received (not timeout)
  if (rxbytes > 0)
  {
    Sleep(200);

    start_time = cv::getTickCount();
    elapsedtime = 0;
    rxbytes = -1;
    int byte_offset = 0;

    do
    {
      // Store incoming data into byte array
      rxbytes = recv(_socket, &rxbuff[byte_offset], imsize, 0);

      // If data recieved then offset for next potential read ** DID NOT TEXT LARGER IMAGES **
      if (rxbytes > 0)
      {
        byte_offset = byte_offset + rxbytes;
      }
      elapsedtime = (float)(cv::getTickCount() - start_time) / (float)cv::getTickFrequency();
    } 
    while (rxbytes != imsize && elapsedtime < 999.0);  // Timeout after 1 second

    // If all the bytes were recieved, decode JPEG data to image
    if (rxbytes > 0 && rxbytes == imsize)
    {
      im = imdecode(cv::Mat(imsize, 1, CV_8UC3, rxbuff), 1);
      return true;
    }
  }

  return false;
}
