#include<opencv2/dnn.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<iostream>
#include<chrono>
#include<random>
#include<set>
#include<cmath>
#include "ClassSkeletons.h"

using namespace std;

int main(int argc, char** argv) {

	ClassSkeletons CS;

	string videoFile = "C://Users//User//source//repos//Project13_Human_Pose//x64//Release//skeleton_test_2.mp4";
	std::string device = "gpu";
	const std::string model_file = "C:\\Users\\User\\source\\repos\\Project13_Human_Pose\\x64\\Release\\pose\\coco\\pose_iter_440000.caffemodel";
	const std::string model_txt = "C:\\Users\\User\\source\\repos\\Project13_Human_Pose\\x64\\Release\\pose\\coco\\pose_deploy_linevec.prototxt";

	//cv::VideoCapture cap(videoFile);

	//if (!cap.isOpened())
	//{
	//	cerr << "Unable to connect to camera" << endl;
	//	return 1;
	//}

	cv::Mat frame, outputFrame;
	//int frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	//int frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);

	//cv::VideoWriter video("C://Users//User//source//repos//Project13_Human_Pose//x64//Release//skeleton_test_2_out_new111.avi", cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10, cv::Size(frameWidth, frameHeight));
	string inputFile = "C://Users//User//source//repos//Project13_Human_Pose//x64//Release//1.png";
	//std::cout << "USAGE : ./multi-person-openpose <videoFile> <device>" << std::endl;

	if (argc == 2) {
		if ((std::string)argv[1] == "gpu")
			device = "gpu";
		else
			videoFile = argv[1];
	}
	else if (argc == 3) {
		videoFile = argv[1];
		if ((std::string)argv[2] == "gpu")
			device = "gpu";
	}

	//std::chrono::time_point<std::chrono::system_clock> startTP = std::chrono::system_clock::now();

	frame = cv::imread(inputFile, cv::IMREAD_COLOR);

	CS.LoadSetNet(model_txt, model_file, device);

	//while (true) {

		//cap >> frame;

		//if (frame.empty())
			//break;

	outputFrame = CS.ProcessFrame(frame);

		//video.write(outputFrame);
	cv::imshow("Detected Pose", outputFrame);
	cv::waitKey(1);
	//}
	//cap.release();
	//video.release();

	return 0;
}

