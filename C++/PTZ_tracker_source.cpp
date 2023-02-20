#include <opencv.hpp>
#include <video/tracking.hpp>
#include <core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <cstring>
#include "PTZ_AI_Tracker.h"


using namespace cv;
using namespace std;


int PTZDx = 0, PTZDy = 0; //на сколько сместится РОИ за шаг
UMat frame; //основной полный кадр
UMat frame_temp; //временный кадр чтобы обрезать огромные видео
UMat frame_cutted; //обрезанный кадр по РОИ

PTZ_AI_Tracker PAT;


int main(int argc, char** argv) {

	string video_path = argv[1]; //входное видео

	VideoCapture video(video_path);

	//проверка открытия
	bool ok = video.read(frame);

	if (ok == 0) {
		cout << "Can't open video" << endl;
		return -1;
	}

	cout << "First frame opened" << endl;

	//ресайзим полный кадр чтобы умещались большие кадры
	resize(frame, frame_temp, Size(frame.cols / 2.5, frame.rows / 2.5));

	//выбираем РОИ для вырезки
	Rect ROI = selectROI(frame_temp, false);

	//возвращаем размеры РОИ к исходным
	ROI.x *= 2.5;
	ROI.y *= 2.5;
	ROI.width *= 2.5;
	ROI.height *= 2.5;

	//вырезаем РОИ из целого кадра
	frame_cutted = frame(ROI);

	//размеры для записи и инициализация объекта записи
	int frame_width = frame_cutted.cols;
	int frame_height = frame_cutted.rows;
	Size frame_size(frame_width, frame_height);

	VideoWriter oVideoWriter("./Tracker__1.mp4", VideoWriter::fourcc('H', '2', '6', '4'), 25, frame_size, true);

	//выбираем объект для трекера
	Rect bbox = selectROI(frame_cutted, false);

	// Display bounding box. 
	rectangle(frame_cutted, bbox, Scalar(255, 0, 0), 2, 1);

	imshow("Tracking", frame_cutted);

	//инициализация трекера
	PAT.InitTrackerAndVideo(frame_cutted, bbox);

	while (video.read(frame)) {

		//вырезаем нужную область
		frame_cutted = frame(ROI);

		PAT.ProcessFrame(frame_cutted, bbox);

		//смещаем РОИ
		ROI.x += PAT.PTZDx;
		ROI.y += PAT.PTZDy;

		//условия остановки зоны если она подошла к границе кадра
		if (ROI.x + ROI.width > frame.cols)
			ROI.x = frame.cols - ROI.width;

		if (ROI.y + ROI.height > frame.rows)
			ROI.y = frame.rows - ROI.height;

		if (ROI.x < 0)
			ROI.x = 0;

		if (ROI.y < 0)
			ROI.y = 0;

		oVideoWriter.write(frame_cutted);

		// Exit if ESC pressed.
		int k = waitKey(10);
		if (k == 27)
			break;
	}
	oVideoWriter.release();
}