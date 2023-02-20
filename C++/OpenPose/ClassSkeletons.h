#ifndef CS_h
#define CS_h

#include<opencv2/dnn.hpp>
#include<opencv2/imgproc.hpp>
#include<opencv2/highgui.hpp>

#include<iostream>
#include<chrono>
#include<random>
#include<set>
#include<cmath>

using namespace std;

#define IMGSHOW 1

struct KeyPointN {
	KeyPointN(cv::Point point, float probability) {
		this->id = -1;
		this->point = point;
		this->probability = probability;
	}

	int id;
	cv::Point point;
	float probability;
};

struct ValidPair {
	ValidPair(int aId, int bId, float score) {
		this->aId = aId;
		this->bId = bId;
		this->score = score;
	}

	int aId;
	int bId;
	float score;
};

const std::vector<std::pair<int, int>> mapIdx = {
	{31,32}, {39,40}, {33,34}, {35,36}, {41,42}, {43,44},
	{19,20}, {21,22}, {23,24}, {25,26}, {27,28}, {29,30},
	{47,48}, {49,50}, {53,54}, {51,52}, {55,56}, {37,38},
	{45,46}
};

const std::vector<std::pair<int, int>> posePairs = {
{1,2}, {1,5}, {2,3}, {3,4}, {5,6}, {6,7},
{1,8}, {8,9}, {9,10}, {1,11}, {11,12}, {12,13},
{1,0}, {0,14}, {14,16}, {0,15}, {15,17}, {2,17},
{5,16}
};


class ClassSkeletons {

private:
	//ќб¤вление всех private переменных и методов класса, все вектора хран¤щие линии, все нужные переменные, все функции обработки

	cv::dnn::Net inputNet;
	const int nPoints = 18;

	//std::ostream& operator << (std::ostream& os, const KeyPointN& kp)
	//{
	//	os << "Id:" << kp.id << ", Point:" << kp.point << ", Prob:" << kp.probability << std::endl;
	//	return os;
	//}

	//std::ostream& operator << (std::ostream& os, const ValidPair& vp)
	//{
	//	os << "A:" << vp.aId << ", B:" << vp.bId << ", score:" << vp.score << std::endl;
	//	return os;
	//}

	//template < class T > std::ostream& operator << (std::ostream& os, const std::vector<T>& v)
	//{
	//	os << "[";
	//	bool first = true;
	//	for (typename std::vector<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii, first = false)
	//	{
	//		if (!first) os << ",";
	//		os << " " << *ii;
	//	}
	//	os << "]";
	//	return os;
	//}

	//template < class T > std::ostream& operator << (std::ostream& os, const std::set<T>& v)
	//{
	//	os << "[";
	//	bool first = true;
	//	for (typename std::set<T>::const_iterator ii = v.begin(); ii != v.end(); ++ii, first = false)
	//	{
	//		if (!first) os << ",";
	//		os << " " << *ii;
	//	}
	//	os << "]";
	//	return os;
	//}

	/*const std::string keypointsMapping[] = {
	"Nose", "Neck",
	"R-Sho", "R-Elb", "R-Wr",
	"L-Sho", "L-Elb", "L-Wr",
	"R-Hip", "R-Knee", "R-Ank",
	"L-Hip", "L-Knee", "L-Ank",
	"R-Eye", "L-Eye", "R-Ear", "L-Ear"
	};*/

public:
	//ќб¤вление всех public переменных и методов класса, конструктор деструктор

	ClassSkeletons() {};
	~ClassSkeletons() {};

	cv::Mat ProcessFrame(cv::Mat frame);

	void LoadSetNet(string model_txt, string model_file, string device);

	void splitNetOutputBlobToParts(cv::Mat& netOutputBlob, const cv::Size& targetSize, std::vector<cv::Mat>& netOutputParts);

	void getKeyPoints(cv::Mat& probMap, double threshold, std::vector<KeyPointN>& keyPoints);

	void populateColorPalette(std::vector<cv::Scalar>& colors, int nColors);

	void populateInterpPoints(const cv::Point& a, const cv::Point& b, int numPoints, std::vector<cv::Point>& interpCoords);

	void getValidPairs(const std::vector<cv::Mat>& netOutputParts, const std::vector<std::vector<KeyPointN>>& detectedKeypoints,
		std::vector<std::vector<ValidPair>>& validPairs, std::set<int>& invalidPairs);

	void getPersonwiseKeypoints(const std::vector<std::vector<ValidPair>>& validPairs, const std::set<int>& invalidPairs,
		std::vector<std::vector<int>>& personwiseKeypoints);
};
#endif