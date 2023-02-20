#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream> 
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "LineCManager.h"
#include "GhostImageManager.h"
#include <filesystem>

using namespace cv;
using namespace std;

Point line_start, line_finish, current_point;
bool lkm = 0;

vector<Lines_list> lines; //линии пользователя
LineCompareManager LCM;
GhostImageManager GIM;


bool exists_test1(const std::string& name) { //проверка существования видеофайла
	if (FILE* file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	}
	else {
		return false;
	}
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata) { //колбэки на мышку для рисования линий

	if (event == EVENT_LBUTTONDOWN) {
		line_start = Point2f(x, y);
		lkm = 1;
	}

	else if (event == EVENT_LBUTTONUP) {
		line_finish = Point2f(x, y);
		Lines_list ls = Lines_list(line_start, line_finish, LCM.img.cols, LCM.img.rows);
		lines.push_back(ls);
		LCM.SetLines(lines);
		LCM.Draw_current();
		lkm = 0;
	}

	else if (event == EVENT_RBUTTONDOWN) {
		LCM.lines_per_global.clear();
		lines.clear();
		LCM.lines.clear();
		LCM.Draw_current();
	}

	else if (event == EVENT_MOUSEMOVE) {

		if (lkm == 1)
		{
			LCM.Draw_current();
			current_point = Point(x, y);
			line(LCM.img_1, line_start, current_point, Scalar(0, 255, 0), 2);
			imshow("Result Image", LCM.img_1);
		}
	}
}

void Save_lines(string out_file, vector<Lines_list>lines_to_save) { //сохранение линий
	ofstream txt_file_out(out_file);
	for (int i = 0; i < lines_to_save.size(); i++) {
		txt_file_out << lines_to_save[i].start << endl;
		txt_file_out << lines_to_save[i].finish << endl;
	}
	cout << "Data saved" << endl;
}

void Set_load_ROI(string video_name, VideoCapture capture, Mat frame, vector<Point>& pts) { //указание зоны интереса

	string ROI_file = video_name + "_ROI.txt";
	string String_ROI_in;
	bool flag1 = 0;

	if (!exists_test1(ROI_file)) { //проверка существования файла
		cout << "ROI file didn't exist!\n";

		while (true) {
			capture >> frame;

			if (frame.empty())
				break;

			if (!flag1) {
				cout << "Select ROI and press 'S' " << endl;
				flag1 = 1;
			}

			frame.copyTo(LCM.img);

			LCM.Draw_current();

			waitKey(20);

			int keyboard = waitKey(1);
			if (keyboard == 's') {
				Save_lines(ROI_file, LCM.lines);
				LCM.lines.clear();
				lines.clear();
				break;
			}
		}
	}

	ifstream txt_ROI_file_in(ROI_file);

	if (txt_ROI_file_in.good()) {

		vector <string> temp_pts;
		cout << "ROI file is exist" << endl;

		while (getline(txt_ROI_file_in, String_ROI_in, '\n'))
			temp_pts.push_back(String_ROI_in);

		string delimiter = ",";
		string x_str1, y_str1;
		float x_float1, y_float1;

		for (int i = 0; i < temp_pts.size(); i++) {

			x_str1 = temp_pts[i].substr(0, temp_pts[i].find(delimiter));
			x_str1.erase(0, 1);
			x_float1 = stof(x_str1);

			y_str1 = temp_pts[i].erase(0, temp_pts[i].find(delimiter) + 2);
			y_str1.pop_back();
			y_float1 = stof(y_str1);

			pts.push_back(Point(x_float1 * capture.get(CAP_PROP_FRAME_WIDTH), y_float1 * capture.get(CAP_PROP_FRAME_HEIGHT)));
		}
	}
}

void Set_load_user_lines(string video_name, VideoCapture capture, Mat frame, Mat frame_cutted, Rect ROI) { //установка линий юзера
	
	string lines_file = video_name + "_user_lines.txt";
	string String_lines_in;
	bool flag2 = 0;

	if (!exists_test1(lines_file)) { //проверка существования файла
		cout << "User lines file didn't exist!\n";

		while (true) {
			capture >> frame;

			if (frame.empty())
				break;

			if (!flag2) {
				cout << "Select lines and press 'S' " << endl;
				flag2 = 1;
			}

			// Crop image
			frame_cutted = frame(ROI);

			frame_cutted.copyTo(LCM.img);

			LCM.Draw_current();

			waitKey(20);

			int keyboard = waitKey(1);
			if (keyboard == 's') {
				Save_lines(lines_file, LCM.lines);
				break;
			}
		}
	}

	ifstream txt_lines_file_in(lines_file);
	if (txt_lines_file_in.good()) {

		vector <string> temp_pts2;
		cout << "User lines file is exist";

		while (getline(txt_lines_file_in, String_lines_in, '\n'))
			temp_pts2.push_back(String_lines_in);

		string delimiter = ",";
		string x_str1, x_str2, y_str1, y_str2;
		float x_float1, x_float2, y_float1, y_float2;

		for (int i = 0; i < temp_pts2.size(); i += 2) {

			x_str1 = temp_pts2[i].substr(0, temp_pts2[i].find(delimiter));
			x_str1.erase(0, 1);
			x_float1 = stof(x_str1);

			y_str1 = temp_pts2[i].erase(0, temp_pts2[i].find(delimiter) + 2);
			y_str1.pop_back();
			y_float1 = stof(y_str1);

			x_str2 = temp_pts2[i + 1].substr(0, temp_pts2[i + 1].find(delimiter));
			x_str2.erase(0, 1);
			x_float2 = stof(x_str2);

			y_str2 = temp_pts2[i + 1].erase(0, temp_pts2[i + 1].find(delimiter) + 2);
			y_str2.pop_back();
			y_float2 = stof(y_str2);

			Lines_list line = Lines_list(Point2f(x_float1, y_float1), Point2f(x_float2, y_float2));
			lines.push_back(line);
		}
	}
}


int main(int argc, char** argv) {

	const string keys =
		"{ @video     |  | path to video file }";

	CommandLineParser parser(argc, argv, keys);
	//string filename = parser.get<string>("@video");
	
	string logfile = "./log.txt";
	string filename = "D:\\road_markers.mp4";

	if (exists_test1(filename))
		cout << endl << "Videofile exist" << endl;

	if (exists_test1(logfile)) {
		ofstream ofs;
		ofs.open(logfile, ofstream::out | ofstream::trunc);
		ofs.close();
	}

	size_t found = filename.find_last_of("/\\"); //выковыриваем название файла
	string delimiter2 = ".";

	string file = filename.substr(found + 1);
	string video_name = file.substr(0, file.find(delimiter2));

	//cout << "before videocapture" << endl;
	//LCM.Save_logs("before videocapture");

	VideoCapture capture(filename);
	Mat frame, ghost_frame, frame_cutted;
	int frame_num = 0, n = 30;
	vector<Point> pts;

	//cout << "after videocapture" << endl;
	//LCM.Save_logs("after videocapture");

	namedWindow("Result Image", WINDOW_AUTOSIZE); // Create Window
	setMouseCallback("Result Image", CallBackFunc);

	//LCM.Save_logs("before set roi");

	Set_load_ROI(video_name, capture, frame, pts);

	//LCM.Save_logs("after set roi");

	Rect ROI = LCM.SetPoints(pts); // по указанным точкам вычисляем зону интереса

	Set_load_user_lines(video_name, capture, frame, frame_cutted, ROI);

	LCM.SetLines(lines); //передаем линии пользователя в функцию

	//LCM.Save_logs("before main while");

	while (true) {

		// Read the image
		capture >> frame;

		if (frame.empty())
			continue;

		// Crop image
		frame_cutted = frame(ROI);

		imshow("frame_cutted", frame_cutted);

		ghost_frame = GIM.Ghost_frame(frame_cutted); //отдаем кадр и делаем его ghost'ом

		//imshow("frame_ghost", ghost_frame);
		if (frame_num % n == 0) { //каждый 30ый кадр делаем детект
			//imshow("ghost_result", ghost_frame);

			LCM.ProcessFrame(ghost_frame); //детектор линий

			if (LCM.log_flag == 1)
				LCM.Save_logs("flag=1: 8");

			cout << "Total frame number" << frame_num << endl;
			waitKey(1); //30
		}

		if (LCM.log_flag == 1)
			LCM.Save_logs("flag=1: 9");

		int keyboard = waitKey(1);
		if (keyboard == 's')
			Save_lines(video_name, LCM.lines);

		frame_num++;
	}
	return 0;
}