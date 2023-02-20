#include "PTZ_AI_Tracker.h"
#include <iostream>
#include <fstream>
#include <sstream> 
#include <video/tracking.hpp>
#include <opencv.hpp>
#include <core/ocl.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

void PTZ_AI_Tracker::InitTrackerAndVideo(UMat frame_cutted, Rect bbox) {

	tracker = TrackerCSRT::create();
	ocl::setUseOpenCL(false);

	tracker->init(frame_cutted, bbox);
}

void PTZ_AI_Tracker::PTZCommand(int XSpeed, int YSpeed, int frame_num) { //Speed 0 .. 255
	cout << "X_Speed: " << XSpeed << " Y_Speed: " << YSpeed << endl;

	//пропуск кадров для симуляции затупов живой камеры
	if (frame_num % 6 == 0) {
		PTZDx = XSpeed / PTZcoof;
		PTZDy = YSpeed / PTZcoof;
	}
}

void PTZ_AI_Tracker::ProcessFrame(UMat frame_cutted, Rect bbox){

	frame_num++;

	//координаты середины РОИ
	float x_center_frame = frame_cutted.cols / 2;
	float y_center_frame = frame_cutted.rows / 2;

	if (frame_num % skip_frames == 0) {

		// Start timer
		double timer = (double)getTickCount();

		// Update the tracking result
		bool ok = tracker->update(frame_cutted, bbox);

		//координаты середины области искомого объекта
		float x_center_bbox = bbox.x + (bbox.width / 2);
		float y_center_bbox = bbox.y + (bbox.height / 2);

		//на сколько надо изменить координаты до идеального положения(центра)
		ideal_Dx = (x_center_bbox - x_center_frame);
		ideal_Dy = (y_center_bbox - y_center_frame);

		//смещение за шаг
		Dx = ideal_Dx / frame_cutted.cols * 2; //в процентах от 0.0 .. 1.0
		Dy = ideal_Dy / frame_cutted.rows * 2;

		//расстояние которое нужно проехать до заветного центра
		float DxDyDist = sqrt(Dx * Dx + Dy * Dy);

		if (DxDyDist > 0.2) { //если объект дальше чем 0.2 увеличиваем скорость перемещения РОИ
			DSpeed = 5 * DxDyDist + 1;
			CurMSpeed += DSpeed;
			if (CurMSpeed > 255)
				CurMSpeed = 255;
			//cout << "Speed ++ " << CurMSpeed << "d: "  << DSpeed << endl;
		}
		else if (DxDyDist < 0.2) { //если объект ближе чем 0.2 уменьшаем скорость перемещения РОИ
			DSpeed = -(5 * (1 - DxDyDist) + 1);
			CurMSpeed = DSpeed;
			if (CurMSpeed < 128)
				CurMSpeed = 128;
			//cout << "speed -- " << CurMSpeed << "d: " << DSpeed << endl;
		}

		//считаем скорости по осям
		int XSpeed = round(Dx * CurMSpeed);
		int YSpeed = round(Dy * CurMSpeed);

		//задаем максимум =255
		if (XSpeed > 255)
			XSpeed = 255;

		if (YSpeed > 255)
			YSpeed = 255;

		cout << "Dx: " << Dx << " Dy: " << Dy << endl;

		//вызываем функцию считающую изменение координат за данный шаг
		PTZCommand(XSpeed, YSpeed, frame_num);

		// Calculate Frames per second (FPS)
		fps = getTickFrequency() / ((double)getTickCount() - timer);

		if (ok) // Tracking success : Draw the tracked object 
			rectangle(frame_cutted, bbox, Scalar(255, 0, 0), 2, 1);

		else // Tracking failure detected.
			putText(frame_cutted, "Tracking failure detected", Point(20, 50), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 0, 255), 2);

		//рисуем линию показывающую вектор движения
		line(frame_cutted, Point(x_center_frame, y_center_frame), Point(x_center_frame + ideal_Dx * 5, y_center_frame + ideal_Dy * 5), Scalar(0, 0, 200), 2);

		// Display FPS on frame
		//putText(frame, "FPS : " + to_string(fps), Point(100, 50), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50, 170, 50), 2);
		putText(frame_cutted, "Speed : " + to_string(int(CurMSpeed)) + " deltaSpeed: " + to_string(round(DSpeed * 100) / 100), Point(20, 25), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(0, 255, 0), 2);

		//отрисовка РОИ на большом кадре
		//rectangle(frame, ROI, Scalar(255, 0, 0), 2, 1);

		//imshow("Tracking_full", frame);
		imshow("Tracking", frame_cutted);

		if (!ok)
			waitKey(0);

	}
}
