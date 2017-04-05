///////////////////////////////////////////////////////////////////
// Prepared for BCIT ELEX4618, April 2017, by Craig Hennessey
///////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <iostream>
#include <string>

#include "Server.h"

#pragma comment(lib, "ws2_32.lib")

#define BACKLOG 5
#define BUFFER 16000

Server::Server(int port)
{
  _port = port;
}

Server::~Server()
{
  _exit = TRUE;
  Sleep(150);
}

void Server::start()
{
  // VideoCapture here only for demo purposes - remove and replace with actual image to transmit
  cv::VideoCapture vid(0);
  cv::Mat frame;
  std::vector<unsigned char> image_buffer;
  std::vector<int> compression_params;
  compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
  compression_params.push_back(30); // 1 to 100 

  int ret;
  WSADATA wsdat; 
  SOCKET serversock;
	SOCKET clientsock = NULL;

  sockaddr_in addr;
	int addressSize = sizeof(addr);

	char buff[BUFFER+1]; // +1 for null
	u_long polling = 1;
	
  if (WSAStartup(0x0101, &wsdat)) 
  {
    WSACleanup();
    return;
  }

  serversock = socket(AF_INET, SOCK_STREAM, 0);
  if (serversock == SOCKET_ERROR)
  {
    WSACleanup();
    return;
  }

  if (ioctlsocket(serversock, FIONBIO, &polling) == SOCKET_ERROR) 
  {
    WSACleanup();
    return;
  }
  
  addr.sin_family = AF_INET;
  addr.sin_port = htons(_port);
  addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(serversock, (sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
  {
    closesocket(serversock);
    WSACleanup();
    return;
	}

	listen(serversock, BACKLOG);
	
  while (_exit == FALSE)
  {
    Sleep (50);

    clientsock = accept(serversock, (sockaddr *)&addr, &addressSize);
    if (clientsock != INVALID_SOCKET)
		{
			do
			{
				ret = recv(clientsock, buff, BUFFER, 0);

				if (ret == 0)
				{
					closesocket(clientsock);
					clientsock = NULL;
				}
				else if (ret == SOCKET_ERROR)
				{
					int error = WSAGetLastError();

					if (error == WSAEWOULDBLOCK) { } // no data 
					else
					{
						closesocket(clientsock);
						clientsock = NULL;
            ret = 0;
					}
				}
        else
        {
          if (ret < BUFFER)
          {
            // Add NULL terminator to string
            buff[ret] = 0;

            // Processing incoming data
            std::string str = buff;
            std::cout << "\nServer RX: " << str;

            if (str == "cmd")
            {
              int send_returnval;
              std::string reply = "Hi there";
              send_returnval = send(clientsock, reply.c_str(), reply.length(), 0);
            }
            else if (str == "im")
            {
              int send_returnval;

              vid >> frame;
              image_buffer.clear();

              // Compress image to reduce size
              cv::imencode("image.jpg", frame, image_buffer, compression_params);
              
              // First send image size as int
              int size = image_buffer.size();              
              std::cout << "\nServer image size: " << size;
              send_returnval = send(clientsock, (char *)&size, sizeof(size), 0);
              
              // Then send image
              send_returnval = send(clientsock, reinterpret_cast<char*>(&image_buffer[0]), image_buffer.size(), 0);
            }
          }
        }
			}
			while (ret != 0 && _exit != TRUE);
		}
	}

  closesocket(serversock);
  
  Sleep (50);
  WSACleanup();
}

