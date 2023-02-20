#ifndef CS_h
#define CS_h

#include<opencv2/dnn.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>
#include <video/tracking.hpp>
#include<iostream>

//#include<chrono>
//#include<random>
//#include<set>
//#include<cmath>

using namespace cv;
using namespace std;


class PTZ_AI_Tracker {

private:
	//Обявление всех private переменных и методов класса, все вектора хранящие линии, все нужные переменные, все функции обработки
	int PTZcoof = 4; //8
	float fps = 0, DSpeed = 0;
	int frame_num = 0;
	float Dx = 0, Dy = 0, ideal_Dx = 0, ideal_Dy = 0;
	float CurMSpeed = 128; //скорость при запуске

	int skip_frames = 1; //пропуск кадров

	UMat frame_cutted; //обрезанный кадр по РОИ

	Ptr<Tracker> tracker; //трекер


public:
	//Обявление всех public переменных и методов класса, конструктор деструктор
	int PTZDx = 0, PTZDy = 0;

	PTZ_AI_Tracker() {};
	~PTZ_AI_Tracker() {};

	void PTZCommand(int XSpeed, int YSpeed, int frame_num); //Speed 0 .. 255
	void ProcessFrame(UMat frame_cutted, Rect bbox);
	void InitTrackerAndVideo(UMat frame_cutted, Rect bbox);
};
#endif