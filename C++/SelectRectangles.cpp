#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include "Contours_list.h"
#include "string"
//#include "conio"


using namespace std;
using namespace cv;

Mat img, img_1;
vector<Point> coord;
int Selected_i = -1;
int dif = 30;
Point top_left_corner, bottom_right_corner, current_point;
ofstream outp;
ifstream inp;
bool lkm = 0;

vector<vector<Contours_list>> contour;
int Curr_Vec = 0;

void Draw_current()
{	
	img.copyTo(img_1);

	if (Curr_Vec < contour.size())
	{
		for (int i = 0; i < contour[Curr_Vec].size(); i++)
			rectangle(img_1, contour[Curr_Vec][i].top_left, contour[Curr_Vec][i].bottom_right, Scalar(0, 255, 0), 2, 8);

		if (Selected_i != -1)
			rectangle(img_1, contour[Curr_Vec][Selected_i].top_left, contour[Curr_Vec][Selected_i].bottom_right, Scalar(255, 0, 0), 2, 8);
	}

	string s1 = to_string(Curr_Vec + 1);
	string s2 = to_string(contour.size());
	string s = s1 + "/" + s2;
	putText(img_1, s, cvPoint(5, 28), FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2);

	imshow("Window", img_1);
}

void Find_selected()
{
	Selected_i = -1;
	for (int i = 0; i < contour[Curr_Vec].size(); i++)
	{
		if ((contour[Curr_Vec][i].top_left.x <= top_left_corner.x) && (top_left_corner.x <= contour[Curr_Vec][i].bottom_right.x))
			if ((contour[Curr_Vec][i].top_left.y <= top_left_corner.y) && (top_left_corner.y <= contour[Curr_Vec][i].bottom_right.y))
			{
				Selected_i = i;
			}
	}
	imshow("Window", img_1);
}

vector<string> String_slicer(string str, string delim)
{
	vector<string> v;
	size_t prev = 0;
	size_t next;
	size_t delta = delim.length();
	while ((next = str.find(delim, prev)) != string::npos) {
		string tmp = str.substr(prev, next - prev);
		v.push_back(str.substr(prev, next - prev));
		prev = next + delta;
	}
	string tmp = str.substr(prev);
	v.push_back(str.substr(prev));
	v.pop_back();
	return v;
}

void Load_from_file() //загружаем все из файла
{
	ifstream inp("example.txt"); // файл из которого читаем

	contour.clear();

	if (inp.is_open()) //проверка открытия входного файла
	{
		vector<string> v;
		int j = 0;
		string str;
		while (getline(inp, str)) { // пока не достигнут конец файла класть очередную строку в переменную (str)

			v = String_slicer(str, " "); //разбить строки из текстового файла на вектор строк

			Point tl;
			Point br;

			if (j == contour.size())
				contour.push_back(vector<Contours_list>());

			for (int i = 0; i < v.size(); i += 4)
			{
				tl.x = stoi(v[i]);
				tl.y = stoi(v[i + 1]);
				br.x = stoi(v[i + 2]);
				br.y = stoi(v[i + 3]);
				contour[j].push_back(Contours_list(tl, br));
			}
			j++;
		}
		inp.close(); // закрываем файл 

		Draw_current();
	}
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{

	if (event == EVENT_LBUTTONDOWN)
	{
		top_left_corner = Point(x, y);
		lkm = 1;
		Draw_current();

		Find_selected();
	}

	else if (event == EVENT_LBUTTONUP)
	{
		bottom_right_corner = Point(x, y);

		if (abs(bottom_right_corner.x - top_left_corner.x) > dif && abs(bottom_right_corner.y - top_left_corner.y) > dif)
			contour[Curr_Vec].push_back(Contours_list(top_left_corner, bottom_right_corner));

		Draw_current();
		lkm = 0;
	}

	else if (event == EVENT_RBUTTONDOWN)
	{
		contour[Curr_Vec].clear();
	}

	else if (event == EVENT_MBUTTONDOWN)
	{

	}

	else if (event == EVENT_MOUSEMOVE) {
		
		Draw_current();

		if (lkm == 1)
		{
			current_point = Point(x, y);
			rectangle(img_1, top_left_corner, current_point, Scalar(0, 255, 0), 2, 8);
			imshow("Window", img_1);
		}
	}

	/*else if (event == event_mousemove)
	{
		cout << "mouse move over the window - position (" << x << ", " << y << ")" << endl;
		point p1(x, y);

		coord.push_back(p1);

		if (coord.size() > 100)
		{
			coord.erase(coord.begin());
		}

		int start = 0;
		bool all_right = 1;

		vector<point> coord_new;

		if (coord.size() > 2)
		{
			while (all_right)
			{
				all_right = false;
				for (int i = start; i < coord.size() - 1; i++)
				{
					if ((abs(coord[start].x - coord[i + 1].x) > dif) || (abs(coord[start].y - coord[i + 1].y) > dif))
					{
						coord_new.push_back(coord[start]);
						start = i + 1;
						all_right = true;
						break;
					}
				}
				if (!all_right)
					coord_new.push_back(coord[coord.size() - 1]);
			}
		}

		if (coord_new.size() > 2)
		{
			for (int j = 0; j < coord.size() - 1; j++)
			{
				line(img_1, coord[j], coord[j + 1], scalar(0, 255, 0), 3);
				imshow("window", img_1);
			}

			for (int j = 0; j < coord_new.size() - 1; j++)
			{
				line(img_1, coord_new[j], coord_new[j + 1], scalar(255, 0, 255), 2);
				imshow("window", img_1);
			}
		}
	}*/
}

int main(int argc, char** argv)
{
	// Read image from file 
	img = imread("1.JPG");
	imshow("Window", img);

	//if fail to read the image
	if (img.empty())
	{
		cout << "Error loading the image" << endl;
		return -1;
	}

	//Create a window
	namedWindow("Window");

	img.copyTo(img_1);
	Load_from_file();

	//set the callback function for any mouse event
	setMouseCallback("Window", CallBackFunc);

	int k = 0;

	// loop until q character is pressed
	while (k != 113)
	{
		k = waitKeyEx(0);

		//удаление последнего контура
		if (k == 122)
		{
			if (contour[Curr_Vec].size() > 0)
			{
				contour[Curr_Vec].pop_back();

				Draw_current();

			}
		}

		//удаление выделенного
		if (k == 3014656)
		{
			if (Selected_i != -1)
			{
				contour[Curr_Vec].erase(contour[Curr_Vec].begin() + Selected_i);
				Selected_i = -1;
				imshow("Window", img_1);
			}
		}

		//сохранение вектора в текстовый файл
		if (k == 115)
		{
			ofstream outp("example.txt");
			if (outp.is_open())
			{
				for (int i = 0; i < contour[Curr_Vec].size(); i++)
					outp << contour[Curr_Vec][i].top_left.x << " " << contour[Curr_Vec][i].top_left.y << " " << contour[Curr_Vec][i].bottom_right.x << " " << contour[Curr_Vec][i].bottom_right.y << " ";
			}
			outp.close();
		}

		//загрузка вектора из текстового
		if (k == 111)
			Load_from_file();

		//переключение строк вектора
		if (k == 2424832)
		{
			if (Curr_Vec > 0 && contour[Curr_Vec].size() != 0)
				Curr_Vec -= 1;
			Draw_current();
		}

		if (k == 2555904)
		{	
			if (contour[Curr_Vec].size() != 0) //если текущий вектор не пустой
				Curr_Vec += 1; //перелистнуть на следующую страницу

			if (Curr_Vec == contour.size()) //если размер вектора соответствует текущей странице
				contour.push_back(vector<Contours_list>()); //добавить следующую пустую страницу

			Draw_current();
		}
	}
	return 0;
}
