////////////////////////////////////////////////////////////////
// ELEX 4618 Template project for BCIT
// Created Oct 5, 2016 by Craig Hennessey
// Last updated March 23, 2017
////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include <string>
#include <iostream>
#include <thread>
#include <time.h>

#include "Client.h"
#include "Server.h"
#include "Serial.h" // Must include Windows.h after Winsock2.h, so Serial must include after Client/Server

// OpenCV Include
#include "opencv.hpp"

// OpenCV Library
#pragma comment(lib,".\\opencv\\lib\\opencv_world310d.lib")

////////////////////////////////////////////////////////////////
// Serial Communication
////////////////////////////////////////////////////////////////
void test_com()
{
  // Comport class
  Serial com;
  com.open("COM51");

  // TX and RX strings
  std::string tx_str = "G 1 1\n";
  std::string rx_str;

  // temporary storage
  char buff[2];
  do
  {
    // Send TX string
		com.write(tx_str.c_str(), tx_str.length());
    Sleep(10); // not needed?
  
    rx_str = "";
    // start timeout count
    float start_time = GetTickCount();

    buff[0] = 0;
		// If 1 byte was read then print to screen, timeout after 1 second
    while (buff[0] != '\n' && GetTickCount() - start_time < 1000)
    {
      if (com.read(buff, 1) > 0)
      {
        rx_str = rx_str + buff[0];
      }
    }

    printf ("\nRX: %s", rx_str.c_str());
    cv::waitKey(1);
  } 
  while (1);
}

////////////////////////////////////////////////////////////////
// Display Image on screen
////////////////////////////////////////////////////////////////
void do_image()
{
  cv::Mat im;

  im = cv::imread("BCIT.jpg");

  srand(time(0));

  for (int i = 0; i < 500; i++)
  {
    float radius = 50 * rand() / RAND_MAX;
    cv::Point center = cv::Point(im.size().width*rand() / RAND_MAX, im.size().height*rand() / RAND_MAX);
    
    cv::circle(im, center, radius, cv::Scalar(200, 200, 200), 1, cv::LINE_AA);
    
    im.at<char>(i,i) = 255;
    
    cv::imshow("Image", im);
    cv::waitKey(1);
  }
}

////////////////////////////////////////////////////////////////
// Display Video on screen
////////////////////////////////////////////////////////////////
void do_video()
{
  cv::VideoCapture vid;

  vid.open(0);

  if (vid.isOpened() == TRUE)
  {
    do
    {
      cv::Mat frame, edges;
      vid >> frame;
      if (frame.empty() == false)
      {
        cv::cvtColor(frame, edges, cv::COLOR_BGR2GRAY);
        cv::GaussianBlur(edges, edges, cv::Size(7, 7), 1.5, 1.5);
        cv::Canny(edges, edges, 0, 30, 3);
        cv::imshow("edges", edges);
      }
    }
    while (cv::waitKey(10) != ' ');
  }      
}		
    

////////////////////////////////////////////////////////////////
// Demo client server communication
////////////////////////////////////////////////////////////////
Server serv(4618);

void serverfunc()
{
  serv.start();
}

void clientserver()
{
  std::string str;
  cv::Mat im;

  std::thread t(&serverfunc);
  t.detach();

  // Wait until server starts up (webcam is slow)
  Sleep(2000);

  // connect
  Client client(4618, "127.0.0.1");

  client.tx_str("cmd");
  do
  {
    client.rx_str(str);
    if (str.length() > 0)
    {
      std::cout << "\nClient Rx: " << str;
    }
  } while (str.length() == 0);

  while (1)
  {
    client.tx_str("im");
    Sleep(100);

    if (client.rx_im(im) == TRUE)
    {
      cv::imshow("rx", im);
      cv::waitKey(10);
    }
  }
}
int main(int argc, _TCHAR* argv[])
{
	//test_com();
	//do_image();
	//do_video ();
  clientserver();
}
