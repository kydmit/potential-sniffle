#include "LineCManager.h"
#include <iostream>
#include <fstream>
#include <sstream> 

void LineCompareManager::GetContours(vector<Vec4f> lines_hough) {

	string contour_string = "[";

	for (int i = 0; i < lines_hough.size(); i++) { //плюсуем найденные hough линии, белыми
		contour_string +=
			"{"
				"\"line\":"
				"{"
					"\"points\":"
						"["
							"{"
								"\"x\":" + to_string(lines_hough[i][0]) + "," +
								"\"y\":" + to_string(lines_hough[i][1]) +
							"},"
							"{"
								"\"x\":" + to_string(lines_hough[i][2]) + "," +
								"\"y\":" + to_string(lines_hough[i][3]) +
							"}"
						"],"
			"\"border_color\":\"#FFFFFF\","
			"\"color\":\"#FFFFFF\","
			"\"thickness\":1,"
			"\"type_graphic\":4,"
			"\"is_filled\":0"
			"}"
			"},";
	}

	for (int j = 0; j < lines.size(); j++) { //плюсуем линии юзера, красными
		contour_string +=
			"{"
			"\"line\":"
			"{"
			"\"points\":"
			"["
			"{"
			"\"x\":" + to_string(lines[j].start.x) + "," +
			"\"y\":" + to_string(lines[j].start.y) +
			"},"
			"{"
			"\"x\":" + to_string(lines[j].finish.x) + "," +
			"\"y\":" + to_string(lines[j].finish.y) +
			"}"
			"],"
			"\"border_color\":\"#FF0000\","
			"\"color\":\"#FF0000\","
			"\"thickness\":1,"
			"\"type_graphic\":4,"
			"\"is_filled\":0"
			"}"
			"},";
	}

	for (int k = 0; k < lines_per_global.size() && k < 5; k++) { //плюсуем линии проекции, зелеными
		contour_string +=
			"{"
			"\"line\":"
			"{"
			"\"points\":"
			"["
			"{"
			"\"x\":" + to_string(lines_per_global[k].start.x) + "," +
			"\"y\":" + to_string(lines_per_global[k].start.y) +
			"},"
			"{"
			"\"x\":" + to_string(lines_per_global[k].finish.x) + "," +
			"\"y\":" + to_string(lines_per_global[k].finish.y) +
			"}"
			"],"
			"\"border_color\":\"#FF0000\","
			"\"color\":\"#FF0000\","
			"\"thickness\":1,"
			"\"type_graphic\":4,"
			"\"is_filled\":0"
			"}"
			"},";
	}
	contour_string += "]";

	std::cout << contour_string << endl;
}

void LineCompareManager::Save_logs(string input_str) {
	string filePath = "./log.txt";
	ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app);
	ofs << input_str << '\n';
	ofs.close();
}

Rect2i LineCompareManager::SetPoints(vector<Point> points) { // по указанным точкам вычисляем зону интереса
	pts = points;

	int x_min = pts[0].x, y_min = pts[0].y, x_max = pts[0].x, y_max = pts[0].y;

	for (int i = 0; i < pts.size(); i++) {
		if (pts[i].x <= x_min) 
			x_min = pts[i].x;

		if (pts[i].x >= x_max)
			x_max = pts[i].x;

		if (pts[i].y <= y_min)
			y_min = pts[i].y;

		if (pts[i].y >= y_max)
			y_max = pts[i].y;

	}
	ROI = Rect(x_min, y_min, x_max - x_min, y_max - y_min);

	return Rect(x_min, y_min, x_max - x_min, y_max - y_min);
}

void LineCompareManager::ProcessFrame(Mat frame_cutted){ //детектор линий
	count++;

	std::cout << "ProcessFrame" << count << endl;
	vector<Vec4f> lines_hough;

	//imshow("frame cutted", frame_cutted);

	if (count == 1) //на первом кадре считываем высоту 
		height = frame_cutted.rows;

	//подготавливаем кадр для houghlines
	frame_for_hough = prepare_frame(frame_cutted, canny_thres1, canny_thres2, contrast, brightness, height);

	// Apply Hough Transform
	HoughLinesP(frame_for_hough, lines_hough, 1, CV_PI / 180, hough_thres, 5, 250);

	//вызываем функцию подбирающую парамерты при выполнении условий
	if (count % 20 == 0)
		if (cur_pers < 50 || lines_hough.size() > 40) {
			Save_logs("before choose params");
			choose_params(frame_cutted, canny_thres1, canny_thres2, hough_thres, contrast, brightness, height);
			Save_logs("after choose_params");
			log_flag = 1;
		}

	if (log_flag == 1)
		Save_logs("flag=1: 1");

	//переводим абсолютные координаты в относительные
	coord_transform_to(lines_hough, frame_for_hough.cols, frame_for_hough.rows);

	if (log_flag == 1)
		Save_logs("flag=1: 2");

	// Draw lines on the image
	for (int i = 0; i < lines_hough.size(); i++)
		line(frame_cutted, Point(lines_hough[i][0] * frame_cutted.cols, lines_hough[i][1] * frame_cutted.rows), Point(lines_hough[i][2] * frame_cutted.cols, lines_hough[i][3] * frame_cutted.rows), Scalar(255, 0, 0), 2, LINE_AA);

	if (log_flag == 1)
		Save_logs("flag=1: 3");

	img = frame_cutted;
	img.copyTo(img_1);

	if (log_flag == 1)
		Save_logs("flag=1: 4");

	//расчитываем процент прокрытия если есть указанные линии юзера
	if (!lines.empty())
		cur_pers_pre = get_all_lines_per(lines_hough);

	if (log_flag == 1)
		Save_logs("flag=1: 5");

	if (cur_pers_pre <= 100)
		cur_pers += (cur_pers_pre - cur_pers) / 2;

	if (count % 30 == 0)
		GetContours(lines_hough);

	if (log_flag == 1)
		Save_logs("flag=1: 6");

	Draw_current();

	if (log_flag == 1)
		Save_logs("flag=1: 7");
}

void LineCompareManager::SetLines(vector<Lines_list> _lines) { //копируем линии юзера из мейна
	lines = _lines;
}

void LineCompareManager::coord_transform_to(vector<Vec4f>& hough_lines_choose, float width, float height) { //переводим абсолютные координаты в относительные
	for (int i = 0; i < hough_lines_choose.size(); i++) {
		hough_lines_choose[i][0] = hough_lines_choose[i][0] / width;
		hough_lines_choose[i][1] = hough_lines_choose[i][1] / height;
		hough_lines_choose[i][2] = hough_lines_choose[i][2] / width;
		hough_lines_choose[i][3] = hough_lines_choose[i][3] / height;
	}
}

vector<Vec4f> coord_transform_back(vector<Vec4f> hough_lines, int width, int height) { 	//переводим относительные координаты в абсолютные
	vector<Vec4f> hough_lines_temp;
	for (int i = 0; i < hough_lines.size(); i++) {
		hough_lines[i][0] = hough_lines[i][0] * width;
		hough_lines[i][1] = hough_lines[i][1] * height;
		hough_lines[i][2] = hough_lines[i][2] * width;
		hough_lines[i][3] = hough_lines[i][3] * height;
		hough_lines_temp.push_back(i);
	}

	return hough_lines_temp;
}

static double angleBetween2Lines(float x_a, float y_a, float x_b, float y_b, float per_x1, float per_y1, float per_x2, float per_y2) {
	//вычисляем угол между прямыми
	const double pi = 3.1415926535897932384626433832795;

	double angle1 = atan2(y_a - y_b, x_a - x_b);
	double angle2 = atan2(per_y1 - per_y2, per_x1 - per_x2);

	double adif = abs(angle1 - angle2);
	double adif_degr = adif * (180 / pi);

	return adif_degr;
}

bool CheckBorder(float ax, float ay, float bx, float by, float mx, float my) { //проверка граничных условий для объединения
	if (abs(((mx - ax) * (by - ay)) - ((bx - ax) * (my - ay))) < 2)
		if (min(ay, by) <= my && (max(ay, by) >= my) && min(ax, bx) <= mx && max(ax, bx) >= mx) {
			return true;
		}
	return false;
}

Mat LineCompareManager::prepare_frame(Mat ghosted_frame_in, int thres1, int thres2, double c, int b, float h) { //подготовка кадра для houghlines

	Mat pr_gray, pr_blurred, pr_resized, pr_converted, pr_adaptive, pr_blurred2;

	float width = round((h / ghosted_frame_in.rows) * ghosted_frame_in.cols);

	vector<Point> mask_pts; //координаты для исходного разрешения

	for (int i = 0; i < pts.size(); i++) {
		//Из координат для оригинального видео перевели в координаты того h который сейчас задан
		mask_pts.push_back(Point((pts[i].x - ROI.x) * (width / ghosted_frame_in.cols), (pts[i].y - ROI.y) * (h / ghosted_frame_in.rows)));
	}

	if (mask.empty()) {
		//подгоняем маску под размер картинки
		mask = Mat::zeros(Size(width, h), CV_8UC1);
		fillPoly(mask, mask_pts, Scalar(255, 255, 255), 8, 0);
	}
	else if (mask.cols != width || mask.rows != h) {   
		//ресайзим маску если разрешение изменилось и заполняем
		resize(mask, mask, Size(width, h));
		fillPoly(mask, mask_pts, Scalar(255, 255, 255), 8, 0);
	}

	medianBlur(ghosted_frame_in, pr_blurred, 7);

	//уменьшаем размер картинки
	resize(pr_blurred, pr_resized, Size(width, h));

	pr_resized.convertTo(pr_converted, -1, c, b); //изменение контраста и яркости

	adaptiveThreshold(pr_converted, pr_adaptive, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, thres1, thres2);

	medianBlur(pr_adaptive, pr_blurred2, 1);

	bitwise_and(pr_blurred2, mask, ghosted_frame_out);

	return ghosted_frame_out;
}

float LineCompareManager::get_persent_per_one_line(vector<Vec4f> lines_hough, Lines_list lines) {  //line линии от пользователя и одна lines_hough, возвращаем процент заполнения этой hough_lines

	float len_main = 0, persent = 0;
	float x_a, y_a, x_b, y_b, x_r1, y_r1, x_r2, y_r2;

	x_a = lines.start.x; //start_x
	y_a = lines.start.y; //start_y
	x_b = lines.finish.x; //finish_x
	y_b = lines.finish.y; //finish_y

	vector<Lines_list> lines_per;

	for (int i = 0; i < lines_hough.size(); i++) {

		x_r1 = lines_hough[i][0];
		y_r1 = lines_hough[i][1];
		x_r2 = lines_hough[i][2];
		y_r2 = lines_hough[i][3];

		per_x1 = x_a + ((x_b - x_a) * (((x_b - x_a) * (x_r1 - x_a) + (y_b - y_a) * (y_r1 - y_a)) / (pow(x_b - x_a, 2) + pow(y_b - y_a, 2))));
		per_y1 = y_a + ((y_b - y_a) * (((x_b - x_a) * (x_r1 - x_a) + (y_b - y_a) * (y_r1 - y_a)) / (pow(x_b - x_a, 2) + pow(y_b - y_a, 2))));
		per_x2 = x_a + ((x_b - x_a) * (((x_b - x_a) * (x_r2 - x_a) + (y_b - y_a) * (y_r2 - y_a)) / (pow(x_b - x_a, 2) + pow(y_b - y_a, 2))));
		per_y2 = y_a + ((y_b - y_a) * (((x_b - x_a) * (x_r2 - x_a) + (y_b - y_a) * (y_r2 - y_a)) / (pow(x_b - x_a, 2) + pow(y_b - y_a, 2))));

		//ищем длину проекций
		float pr1 = sqrt(pow(per_x1 - x_r1, 2) + pow(per_y1 - y_r1, 2));
		float pr2 = sqrt(pow(per_x2 - x_r2, 2) + pow(per_y2 - y_r2, 2));

		bool check_border1_ok = 0;
		bool check_border2_ok = 0;

		if (CheckBorder(x_a, y_a, x_b, y_b, per_x1, per_y1))
			check_border1_ok = 1;

		if (CheckBorder(x_a, y_a, x_b, y_b, per_x2, per_y2))
			check_border2_ok = 1;

		if (!(check_border1_ok) && !(check_border2_ok)) { //если И ПЕРВАЯ И ВТОРАЯ точка выходят за границы линии
			per_x1 = x_a;
			per_y1 = y_a;
			per_x2 = x_b;
			per_y2 = y_b;
		}

		if (!(check_border1_ok) && (check_border2_ok)) { //если ТОЛЬКО ПЕРВАЯ точка выходит за границы линии
			int dist_to_border1 = sqrt(pow(per_x1 - x_a, 2) + pow(per_y1 - y_a, 2));
			int dist_to_border2 = sqrt(pow(per_x1 - x_b, 2) + pow(per_y1 - y_b, 2));

			if (dist_to_border1 < dist_to_border2) {
				per_x1 = x_a;
				per_y1 = y_a;
			}
			else {
				per_x1 = x_b;
				per_y1 = y_b;
			}
		}

		if ((check_border1_ok) && !(check_border2_ok)) { //если ТОЛЬКО ВТОРАЯ точка выходит за границы линии
			int dist_to_border1 = sqrt(pow(per_x2 - x_a, 2) + pow(per_y2 - y_a, 2));
			int dist_to_border2 = sqrt(pow(per_x2 - x_b, 2) + pow(per_y2 - y_b, 2));

			if (dist_to_border1 < dist_to_border2) {
				per_x2 = x_a;
				per_y2 = y_a;
			}
			else {
				per_x2 = x_b;
				per_y2 = y_b;
			}
		}

		//считаем угол между линией опенцв и нарисованной юзероми 
		double a = angleBetween2Lines(x_a, y_a, x_b, y_b, x_r1, y_r1, x_r2, y_r2);
		double b = angleBetween2Lines(x_a, y_a, x_b, y_b, x_r2, y_r2, x_r1, y_r1);
		double c = min(a, b);

		if (min(pr1, pr2) < 0.025) { //если прямая слишком далеко от исходной то не добавляем
			if (c < 15) { //если угол между прямыми больше 15 то не добавляем
				Lines_list ps = Lines_list(Point2f(per_x1, per_y1), Point2f(per_x2, per_y2));
				lines_per.push_back(ps);
			}
		}
	}

	for (int i = 0; i < lines_per.size(); i++)
		lines_per_global.push_back(lines_per[i]); //to show lines

	len_main = sqrt(pow(x_b - x_a, 2) + pow(y_b - y_a, 2));

	//make 1d
	vector<Pers_1D> per_1D;

	for (int i = 0; i < lines_per.size(); i++) {
		//convert 2D to 1D
		float left_border = sqrt(pow(lines_per[i].start.x - x_a, 2) + pow(lines_per[i].start.y - y_a, 2));
		float right_border = sqrt(pow(x_a - lines_per[i].finish.x, 2) + pow(y_a - lines_per[i].finish.y, 2));

		//if (left_border > x_a)
		per_1D.push_back(Pers_1D(left_border, right_border));
	}

	//объединить проекции одномерные
	int t = 0;
	bool united = true;

	if (lines_per.size() > 1) { //если существует более двух проекций
		while (united == true && t < per_1D.size() - 1) {
			united = false;

			for (int t2 = 0; t2 < per_1D.size(); t2++) {
				//ищем с кем бы обьедениться
				if (t != t2)
					if (per_1D[t].left < per_1D[t2].right && per_1D[t].right > per_1D[t2].left) {
						per_1D[t].left = min(per_1D[t].left, per_1D[t2].left);
						per_1D[t].right = max(per_1D[t].right, per_1D[t2].right);

						per_1D.erase(per_1D.begin() + t2);
						united = true;
						break;
					}
			}
			if (!united)
				t++;
		}
	}

	float coord_min, coord_max;
	//считаем процент
	for (int i = 0; i < per_1D.size(); i++) {
		coord_min = min(per_1D[i].left, per_1D[i].right);
		coord_max = max(per_1D[i].left, per_1D[i].right);
		persent += (coord_max - coord_min) / float(len_main) * 100;
	}
	return persent;
}

int LineCompareManager::get_all_lines_per(vector<Vec4f> lines_hough) { //считаем общий процент покрытия
	float persent = 0;
	int len_total = 0;
	lines_per_global.clear();

	for (int j = 0; j < lines_hough.size(); j++) {
		len_total += sqrt(pow(lines_hough[j][2] - lines_hough[j][0], 2) + pow(lines_hough[j][3] - lines_hough[j][1], 2));
	}

	for (int i = 0; i < lines.size(); i++)
		persent += get_persent_per_one_line(lines_hough, lines[i]);

	return persent / lines.size();
}

bool LineCompareManager::choose_params(Mat frame_cutted_choose, int& thres1, int& thres2, int& thres3, double& c, int& b, int& h) {
	//подбираем лучшие параметры для данного кадра с данными параметрами

	vector<Vec4f> hough_lines_choose;
	vector<Vec8i> persent_choose;
	vector<Vec8i> hough_lines_choose_max_pers;

	vector <int> contr = { 1, 2 }; //1.5 2.5
	vector <int> bright = { 0, 20, 40, 60, 80, 100 };
	vector <int> thr_1 = { 3, 5, 7, 9, 11 };
	vector <int> thr_2 = { 2, 4, 6, 7, 8, 9, 10 };
	vector <int> thr_3 = { 40, 60, 90, 120, 150 };
	vector <int> height = { 100, 200, 300, 400, 600, 800 };

	int current_frame_pers = 0;

	Mat frame_for_hou_c;

	if (lines.size() == 0)
		return 0;

	Save_logs("in choose params, before 5for");
	//перебираем параметры и для каждой комбинации считаем линии и процент
	for (int h : height) {
		if (frame_cutted_choose.rows < h & h > height[0])
			continue;
		for (double c : contr) //1.0 0.2
			for (int b : bright) { //0 10
				for (int i : thr_1)
					for (int j : thr_2) {

						frame_for_hou_c = prepare_frame(frame_cutted_choose, i, j, c, b, h);

						for (int k : thr_3) {

							imshow("before hough", frame_for_hou_c);
							waitKey(1);

							HoughLinesP(frame_for_hou_c, hough_lines_choose, 1, CV_PI / 180, k, 5, 250);

							coord_transform_to(hough_lines_choose, ((float(h) / frame_cutted_choose.rows) * frame_cutted_choose.cols), h);

							current_frame_pers = get_all_lines_per(hough_lines_choose);

							if (current_frame_pers <= 100) {
								//записываем в вектор значение параметров
								persent_choose.push_back(Vec8i(current_frame_pers, int(hough_lines_choose.size()), i, j, k, c, b, h));
							}
						}
					}
			}
	}

	Save_logs("in choose params, after 5for");

	//вычисляем максимальный процент прокрытия
	int max_pers = persent_choose[0][0];
	for (int k = 0; k < persent_choose.size(); k++)
		if (persent_choose[k][0] > max_pers)
			max_pers = persent_choose[k][0];

	//cout << max_pers << endl;
	Save_logs("in choose params, after max pers");

	//если процент покрытия равен максимуму или макс-2 то добавляем во временный вектор все попавшие комбинации
	for (int k = 0; k < persent_choose.size(); k++)
		if (persent_choose[k][0] >= (max_pers - 2) && persent_choose[k][1] < 70)
			hough_lines_choose_max_pers.push_back(Vec8i(persent_choose[k][0], persent_choose[k][1], persent_choose[k][2], persent_choose[k][3], persent_choose[k][4], persent_choose[k][5], persent_choose[k][6], persent_choose[k][7]));

	Save_logs("in choose params, after max pers-2");

	//вычисляем минимальное количество линий среди комбинаций с максимальным покрытием и берем соответствующие параметры
	int min_lines = hough_lines_choose_max_pers[0][1];
	thres1 = hough_lines_choose_max_pers[0][2];
	thres2 = hough_lines_choose_max_pers[0][3];
	thres3 = hough_lines_choose_max_pers[0][4];
	c = hough_lines_choose_max_pers[0][5];
	b = hough_lines_choose_max_pers[0][6];
	h = hough_lines_choose_max_pers[0][7];

	int best_k = 0;

	for (int k = 0; k < hough_lines_choose_max_pers.size(); k++)
		if (hough_lines_choose_max_pers[k][1] < min_lines) {
			min_lines = hough_lines_choose_max_pers[k][1];
			thres1 = hough_lines_choose_max_pers[k][2];
			thres2 = hough_lines_choose_max_pers[k][3];
			thres3 = hough_lines_choose_max_pers[k][4];
			c = hough_lines_choose_max_pers[k][5];
			b = hough_lines_choose_max_pers[k][6];
			h = hough_lines_choose_max_pers[k][7];
			best_k = k;
		}

	//cout << "choose params before return" << endl;
	Save_logs("in choose params, before return");

	return max_pers < 20;
}

void LineCompareManager::Draw_current() { //отрисовываем линии пользователя и перпендикуляры на них
	img.copyTo(img_1);

	for (int i = 0; i < lines.size(); i++) {
		line(img_1, lines[i].Start(img_1.cols, img_1.rows), lines[i].Finish(img_1.cols, img_1.rows), Scalar(0, 255, 0), 2);
	}

	for (int i = 0; i < lines_per_global.size(); i++) {
		line(img_1, lines_per_global[i].Start(img_1.cols, img_1.rows), lines_per_global[i].Finish(img_1.cols, img_1.rows), Scalar(0, 0, 255), 2);
	}

	putText(img_1, to_string(cur_pers), Point(5, 28), FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 2);

	imshow("Result Image", img_1);
}