#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

//Thrift includes
#include <exception>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <protocol/TBinaryProtocol.h>
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <transport/TTransportUtils.h>
#include "thriftData/ImageService.h"

using namespace boost;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

static const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";
static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i){
      for(j = i; j < 3; j++)
          char_array_3[j] = '\0';

      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for (j = 0; (j < i + 1); j++)
        ret += base64_chars[char_array_4[j]];

      while((i++ < 3))
        ret += '=';

   }
    return ret;
}


//this is a sample for foreground detection functions
int main(int argc, char** argv){

    shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
    shared_ptr<TTransport> transport(new TFramedTransport(socket));
    shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    ImageServiceClient client(protocol);

    cv::VideoCapture cap(0);

    if( !cap.isOpened() ){
        std::cout << "No se puede abrir la camara"<<std::endl;
        return -1;
    }


    for(;;){
        cv::Mat frame;
        cap >> frame;
        /********************* PROCESAR ***********************/
        /*******************************************************/
        cv::Mat img_enviar;
        //Insertar la imagen hacia el servidor nodejs para el streamingMat img_enviar;
        cv::resize(frame,img_enviar,cv::Size(160,120));

        try{
            transport->open();
            if(transport->isOpen()){std::vector<uchar>buff;
                std::vector<int> p;
                p.push_back(CV_IMWRITE_JPEG_QUALITY);
                p.push_back(70);
                cv::imencode(".jpeg",img_enviar,buff,p);
                std::string s(buff.begin(),buff.end());
                //Encode64
                std::string encoded = base64_encode(reinterpret_cast<const unsigned char*>(s.c_str()),s.length());
                //Send
                ImageData data(encoded,img_enviar.cols,img_enviar.rows);
                client.receiveImage(data);
                transport->close();
             }
        }catch(TException &t){
            std::cerr << "Error: " << t.what() << std::endl;
            break;
        }
    }
    socket->close();
    return 0;
}
