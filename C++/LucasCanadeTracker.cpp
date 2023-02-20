#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>


using namespace cv;
using namespace std;

struct path_of_dot {
    int ID;
    Point2f Cur_coord;
    vector<Point2f> Coord_history; // история, здесь еще нет Cur_coord
    bool can_be_clear = false;


    path_of_dot(int id, Point2f cur_coord) {
        ID = id;
        Cur_coord = cur_coord;
    }

    void update(Point2f new_coord) {
        if (abs(new_coord.x - Cur_coord.x) < 5 || abs(new_coord.y - Cur_coord.y) < 5)
            //если точка не сильно двигается на шаге, плюсуем в счетчик, когда счетчик =5, убираем эту точку
        
        //протоколировать когда пора очистить точкó

        Cur_coord = new_coord;
        Coord_history.push_back(Cur_coord);

        if (Coord_history.size() > 100)
            Coord_history.pop_back();
    }

    void draw_history(Mat& img) {
         //line(img, p1[i], Cur_coord, colors[i], 2);
        //рисовать всю историю
        //над цветами подóмать
         circle(img, Cur_coord, 15, Scalar(100, 0, 200), 2);
    }
};


vector<Point2f> p0, p1;
vector<path_of_dot> all_paths; 
int global_id = 0;


void CallBackFunc(int event, int x, int y, int flags, void* userdata) {

    if (event == EVENT_LBUTTONUP) {
        //cerr << "CallBackFunc" << endl;
        //p0.push_back(Point2f(x, y));
        global_id++;
        all_paths.push_back(path_of_dot(global_id, Point2f(x, y)));
    }
}


int main(int argc, char** argv) {
    const string about =
        "This sample demonstrates Lucas-Kanade Optical Flow calculation.\n"
        "The example file can be downloaded from:\n"
        "  https://www.bogotobogo.com/python/OpenCV_Python/images/mean_shift_tracking/slow_traffic_small.mp4";
    const string keys =
        "{ h help |      | print this help message }"
        "{ @image | vtest.avi | path to image file }";

    CommandLineParser parser(argc, argv, keys);

    parser.about(about);
    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }

    string filename = samples::findFile(parser.get<string>("@image"));
    if (!parser.check()) {
        parser.printErrors();
        return 0;
    }

    VideoCapture capture(filename);
    if (!capture.isOpened()) {
        //error in opening the video input
        cerr << "Unable to open file!" << endl;
        return 0;
    }

    // выборка из 100 рандомных цветов
    vector<Scalar> colors;
    RNG rng;
    for (int i = 0; i < 100; i++) {
        int r = rng.uniform(0, 256);
        int g = rng.uniform(0, 256);
        int b = rng.uniform(0, 256);
        colors.push_back(Scalar(r, g, b));
    }

    Mat old_frame, old_gray;

    // Take first frame and find corners in it
    capture >> old_frame;
    cvtColor(old_frame, old_gray, COLOR_BGR2GRAY);
    //goodFeaturesToTrack(old_gray, p0, 100, 0.3, 7, Mat(), 7, false, 0.04);

    // Create a mask image for drawing purposes
    Mat mask = Mat::zeros(old_frame.size(), old_frame.type());

    while (true) {
        Mat frame, frame_gray;

        capture >> frame;

        if (frame.empty())
            break;

        cvtColor(frame, frame_gray, COLOR_BGR2GRAY);

        if (all_paths.size() != 0) {

            // calculate optical flow
            vector<uchar> status; // найдена ли данная точка на следующем кадре
            vector<float> err;
            TermCriteria criteria = TermCriteria((TermCriteria::COUNT)+(TermCriteria::EPS), 10, 0.03);

            p0.clear();
            for (int i = 0; i < all_paths.size(); i++)
                p0.push_back(all_paths[i].Cur_coord);
            

            calcOpticalFlowPyrLK(old_gray, frame_gray, p0, p1, status, err, Size(15, 15), 2, criteria);
            vector<Point2f> good_new;

            for (int i = 0; i < all_paths.size(); i++)
                all_paths[i].update(p1[i]);

            
            for (uint i = 0; i < all_paths.size(); i++) {
                all_paths[i].draw_history(frame);
            }

            // Select good points
                //if (status[i] == 1) {
                   // good_new.push_back(p1[i]);
                    // draw the tracks
                   // line(mask, p1[i], p0[i], colors[i], 2);
                   // circle(frame, p1[i], 5, colors[i], -1);
                //}
            
            p0 = good_new;
            //p0 = p1;
        }

         Mat img;
         add(frame, mask, img);
         imshow("Frame", img);
         //imshow("Mask", mask);

        setMouseCallback("Frame", CallBackFunc);

        int keyboard = waitKey(30);
        if (keyboard == 'q' || keyboard == 27)
            break;

        // Now update the previous frame and previous points
        old_gray = frame_gray.clone();

        //p0 = good_new;
    }
}