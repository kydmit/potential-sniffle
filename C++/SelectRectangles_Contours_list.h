//#include "opencv2/imgcodecs.hpp"

using namespace cv;

struct Contours_list {
	Point top_left;
	Point bottom_right;

	Contours_list(Point top_left_corner, Point bottom_right_corner){
		top_left = top_left_corner;
		bottom_right = bottom_right_corner;
	}
};