#ifndef LCM_h
#define LCM_h

#include <vector>
#include <string>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "Lines_list.h"
#include "LineCManager.h"


using namespace cv;
using namespace std;

#define IMGSHOW 1

class LineCompareManager {

private:
	//ќб¤вление всех private переменных и методов класса, все вектора хран¤щие линии, все нужные переменные, все функции обработки

	Mat frame_for_hough, mask, ghosted_frame_out;
	float per_x1, per_y1, per_x2, per_y2;

	vector<Point> pts; //точки интереса
	Rect ROI;

	int hough_thres = 100, canny_thres1 = 15, canny_thres2 = 3, brightness = 0, height, count = 0;
	double contrast = 1.0;
	float cur_pers = 0, cur_pers_pre = 0;

public:
	//ќб¤вление всех public переменных и методов класса, конструктор деструктор

	Mat img, img_1;
	vector<Lines_list> lines; //линии пользовател¤
	vector<Lines_list> lines_per_global; //основани¤ перпендикул¤ров дл¤ линий пользовател¤

	bool log_flag = 0;

	LineCompareManager() {};
	~LineCompareManager() {};

	void Save_logs(string input_str);

	void GetContours(vector<Vec4f> lines_hough);

	Rect2i SetPoints(vector<Point> points);

	void SetLines(vector<Lines_list> _lines); 

	void ProcessFrame(Mat frame_cutted);

	void coord_transform_to(vector<Vec4f>& hough_lines_choose, float width, float height);

	//vector<Vec4f> coord_transform_back(vector<Vec4f> hough_lines, int width, int height);

	//double angleBetween2Lines(float x_a, float y_a, float x_b, float y_b, float per_x1, float per_y1, float per_x2, float per_y2);

	//bool CheckBorder(float ax, float ay, float bx, float by, float mx, float my);

	Mat prepare_frame(Mat ghosted_frame_in, int thres1, int thres2, double c, int b, float h);
	
	float get_persent_per_one_line(vector<Vec4f> lines_hough, Lines_list lines);

	int get_all_lines_per(vector<Vec4f> lines_hough);

	bool choose_params(Mat frame_cutted_choose, int& thres1, int& thres2, int& thres3, double& c, int& b, int& h);

	void Draw_current();
};
#endif