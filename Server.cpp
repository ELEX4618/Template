///////////////////////////////////////////////////////////////////
// Prepared for BCIT ELEX4618 & ELEX4699, April 2017, by Craig Hennessey
///////////////////////////////////////////////////////////////////
#include "stdafx.h"

#define WIN4618
//#define PI4618

#include <iostream>
#include <string>

#include "server.h"

// OpenCV Include
#ifdef WIN4618
#include "opencv.hpp"
#endif

#ifdef PI4618
#include <opencv2/opencv.hpp>
#endif

#ifdef WIN4618
#include "Winsock2.h"
#pragma comment(lib, "ws2_32.lib")
#endif

#ifdef PI4618
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#define RECV_BUFF_SIZE 256

#define BACKLOG 5
#define BUFFER 16000

Server::Server(int port)
{
  _port = port;
}

Server::~Server()
{
  _exit = true;
  cv::waitKey(150);
}

void Server::start()
{
  // VideoCapture here only for demo purposes
  // Remove and replace with actual image to transmit (processsed by image proc system?) 
  // Remember to protect images with mutex if multi-threaded
  cv::VideoCapture vid(0);
  cv::Mat frame;

  // Image compression parameters
  std::vector<unsigned char> image_buffer;
  std::vector<int> compression_params;
  compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
  compression_params.push_back(30); // 1 to 100 (100 = highest quality/largest image)

  int ret;
  struct sockaddr_in server_addr, client_addr;

#ifdef WIN4618
  WSADATA wsdat;
  SOCKET serversock;
  SOCKET clientsock = NULL;
  int addressSize = sizeof(server_addr);
#endif

#ifdef PI4618
  int serversock;
  int clientsock = 0;
  unsigned int addressSize = sizeof(server_addr);
#endif


  char buff[BUFFER + 1]; // +1 for null
  u_long polling = 1;

#ifdef WIN4618
  if (WSAStartup(0x0101, &wsdat))
  {
    WSACleanup();
    return;
  }
#endif

  serversock = socket(AF_INET, SOCK_STREAM, 0);
  if (serversock == SOCKET_ERROR)
  {
#ifdef WIN4618
    WSACleanup();
#endif
    return;
  }

#ifdef WIN4618
  if (ioctlsocket(serversock, FIONBIO, &polling) == SOCKET_ERROR)
  {
    WSACleanup();
    return;
  }
#endif

#ifdef PI4618
  int opt = 1;
  ioctl(serversock, FIONBIO, &opt);
#endif

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(_port);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  if (bind(serversock, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
  {
#ifdef WIN4618
    closesocket(serversock);
    WSACleanup();
#endif
#ifdef PI4618
    close(serversock);
#endif  
    return;
  }

  listen(serversock, BACKLOG);


  while (_exit == false)
  {
    std::cout << "\nWaiting for client....";

    clientsock = accept(serversock, (struct sockaddr *) &client_addr, &addressSize);

    if (clientsock != INVALID_SOCKET)
    {
      do
      {
        ret = recv(clientsock, buff, BUFFER, 0);

        // If socket was shut down orderly (client disconnected)
        if (ret == 0)
        {
#ifdef PI4618
          close(clientsock);
#endif
#ifdef WIN4618
          closesocket(clientsock);
#endif
          clientsock = INVALID_SOCKET;
        }
        // Else some other error
        else if (ret == SOCKET_ERROR)
        {
#ifdef PI4618
          //unsigned int len = sizeof(errno);
          //getsockopt(clientsock, SOL_SOCKET, SO_ERROR, &errno, &len);

          if (errno == EWOULDBLOCK)
          {
            // no data to recieve, go check again
          }
          else
          {
            std::cout << "\nSO_ERROR";
            close(clientsock);
            clientsock = INVALID_SOCKET;
          }
#endif

#ifdef WIN4618
          if (WSAGetLastError() == WSAEWOULDBLOCK)
          { 
            // no data to recieve, go check again
          } 
          else
          {
            closesocket(clientsock);
            clientsock = INVALID_SOCKET;
          }
#endif
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

            // The client sent "cmd" as a message
            if (str == "cmd")
            {
              int send_returnval;
              std::string reply = "Hi there";
              send_returnval = send(clientsock, reply.c_str(), reply.length(), 0);
            }
            // The client sent "im" as a message
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
      } while (clientsock != INVALID_SOCKET && _exit != true);
    }
  }

#ifdef WIN4618
  closesocket(serversock);
  WSACleanup();
#endif

#ifdef PI4618
  close(serversock);
#endif  
}