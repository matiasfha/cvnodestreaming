struct ImageData{
   1: binary data,
   2: i32 width,
   3: i32 height
 }
service ImageService {
	
	bool receiveImage(1:ImageData image)
}
