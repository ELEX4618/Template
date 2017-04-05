///////////////////////////////////////////////////////////////////
// Prepared for BCIT ELEX4618, April 2017, by Craig Hennessey
///////////////////////////////////////////////////////////////////
#pragma once

// OpenCV Include
#include "opencv.hpp"

#include "Winsock2.h"

#define RECV_BUFF_SIZE 256

class Server
{
private:
	int _port;
  bool _exit;

public:
  Server(int port);
	~Server();

  void start();
};


