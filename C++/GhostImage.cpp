#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;


int main(int argc, char** argv)
{
	const string keys =
		"{ @video |  | path to image file }";

	//CommandLineParser parser(argc, argv, keys);
	//string filename = samples::findFile(parser.get<string>("@video"));
	//VideoCapture capture(samples::findFile(filename));

	VideoCapture capture("TownCenter5_second_ghost.avi");

	if (!capture.isOpened()) {
		//error in opening the video input
		cerr << "Unable to open file!" << endl;
	}

	Mat frame1, frame_gray, blured_frame, result_frame;
	int n = 15;


	vector<Mat> all_frames;
	vector<Mat> all_blurred;
	vector<uchar> current_xy_pixels;

	all_frames.reserve(n + 1);
	all_blurred.reserve(n + 1);
	current_xy_pixels.reserve(n + 1);

	int frame_num = 0;

	Size frame_size(capture.get(CAP_PROP_FRAME_WIDTH), capture.get(CAP_PROP_FRAME_HEIGHT));
	VideoWriter oVideoWriter("./TownCenter5_second_third.avi", VideoWriter::fourcc('D', 'I', 'V', 'X'), 25, frame_size, false);

	while (true) {
		capture >> frame1;

		if (frame1.empty())
			break;

		cvtColor(frame1, frame_gray, COLOR_RGB2GRAY); //перекрашиваем цветное в ч\б 


		all_frames.push_back(frame_gray.clone());
		if (all_frames.size() >= n)
			all_frames.erase(all_frames.begin());

		if (result_frame.rows == 0)
			result_frame = frame_gray.clone();

		if (blured_frame.rows == 0)
			blured_frame = frame_gray.clone();

		if (frame_num % n == 0)
			all_blurred.push_back(blured_frame.clone());

		if (all_blurred.size() >= n)
			all_blurred.erase(all_blurred.begin());

		for (int y = 0; y < frame_gray.rows; y++)
		{
			for (int x = 0; x < frame_gray.cols; x++)
			{

				uint8_t* pixelPtr0 = (uint8_t*)frame_gray.data;
				uchar Pixel0 = pixelPtr0[y * frame_gray.cols + x];

				uint8_t* pixelPtr1 = (uint8_t*)blured_frame.data;
				uchar& Pixel1 = pixelPtr1[y * blured_frame.cols + x];
				float dif = (Pixel0 - int(Pixel1));
				int blurxy = int(round(dif / 6));
				Pixel1 += blurxy;

				current_xy_pixels.clear();
				if (frame_num % n == 0)
					for (int i = 0; i < all_blurred.size(); i++) {

						uint8_t* pixelPtr_ab = (uint8_t*)all_blurred[i].data;
						uchar& Pixel_ab = pixelPtr_ab[y * all_blurred[i].cols + x];

						current_xy_pixels.push_back(Pixel_ab);

						int median = 0;

						if (current_xy_pixels.size() > 0) {

							sort(current_xy_pixels.begin(), current_xy_pixels.end());

							if (current_xy_pixels.size() % 2)
								median = (current_xy_pixels[current_xy_pixels.size() / 2] + current_xy_pixels[current_xy_pixels.size() / 2 - 1]) / 2;
							else
								median = current_xy_pixels[current_xy_pixels.size() / 2];

							uint8_t* pixelPtr2 = (uint8_t*)result_frame.data;
							uchar& Pixel2 = pixelPtr2[y * result_frame.cols + x];
							Pixel2 = median;

						}
					}
			}
		}

		oVideoWriter.write(result_frame);

		imshow("frame", frame_gray);
		imshow("frame_result", result_frame);
		//imshow("frame_blured", blured_frame);

		frame_num++;

		int keyboard = waitKey(1);
		if (keyboard == 'q' || keyboard == 27)
			break;
	}

	oVideoWriter.release();
	return 0;
}