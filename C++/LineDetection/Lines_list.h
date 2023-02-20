//#include "opencv2/imgcodecs.hpp"
#include <opencv2/imgproc.hpp>

using namespace cv;

struct Lines_list {
	Point2f start;
	Point2f finish;

	Lines_list(Point line_start, Point line_finish, float width, float height){
		//координаты от 0 до 1
		start.x = line_start.x / width;
		start.y = line_start.y / height;
		finish.x = line_finish.x / width;
		finish.y = line_finish.y / height;
	}

	Lines_list(Point2f line_start, Point2f line_finish) {
		//координаты от 0 до 1
		start.x = line_start.x;
		start.y = line_start.y;
		finish.x = line_finish.x;
		finish.y = line_finish.y;
	}

	Point Start(int w, int h) {
		return Point(int(start.x * w), int(start.y * h));
	}

	Point Finish(int w, int h) {
		return Point(int(finish.x * w), int(finish.y * h));
	}
};

struct Pers_1D {
	float left;
	float right;

	Pers_1D(float _left, float _right) {
		left = min(_left, _right);
		right = max(_left, _right);
	}
};