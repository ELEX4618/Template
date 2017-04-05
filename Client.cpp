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
		closesocket(_socket); // close
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

bool Client::rx_im(cv::Mat &im)
{
  int rxbytes;
  int imsize;
  
  // Should timeout if no data recieved
  do
  {
    rxbytes = recv(_socket, (char *)&imsize, sizeof(imsize), 0);
  } 
  while (rxbytes < 0);
  
  // Should loop recv'ing data until imsize read (packets can be truncated) and timeout if not enough data recieved
  rxbytes = recv(_socket, rxbuff, imsize, 0);
  if (rxbytes > 0)
  {
    im = imdecode(cv::Mat(rxbytes, 1, CV_8UC3, rxbuff), 1);
    return true;
  }

  return false;
}
