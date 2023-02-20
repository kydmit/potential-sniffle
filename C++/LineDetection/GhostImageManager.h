#include <vector>
#include <string>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

class GhostImageManager {

private:
	//Обявление всех private переменных и методов класса, все вектора хранящие линии, все нужные переменные, все функции обработки

	Mat frame_gray,
		blured_frame; //накопительные кадры по формуле x+=(x1-x)/8 (дожен заполняться каждый кадр)
	
	int frame_num = 0;

	vector<Mat> all_blurred; //вектор тех накопительных кадров из котороых будем получать медиану
	vector<uchar> current_xy_pixels; //временный вектор пикселей этих x y в пространстве вектора all_blurred

public:
	int history_size = 30; //размер истории кадров all_blurred

	//Обявление всех public переменных и методов класса,конструктор деструктор
	GhostImageManager() { Reserve_vectors(); }
	~GhostImageManager(){}
	
	void Reserve_vectors();

	Mat Ghost_frame(Mat frame);
};