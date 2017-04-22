///////////////////////////////////////////////////////////////////
// Prepared for BCIT ELEX4618, April 2017, by Craig Hennessey
///////////////////////////////////////////////////////////////////
#pragma once

#include <mutex>

class Server
{
private:
	int _port;
  bool _exit;

  cv::Mat _im;  
  std::mutex _mutex;
  
public:
  Server(int port);
	~Server();

  void start();
  void stop();

  void set_image(cv::Mat &im);
};


