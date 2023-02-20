#include "GhostImageManager.h"

void GhostImageManager::Reserve_vectors() { //резервируем место в векторах под 30 кадров
	all_blurred.reserve(history_size + 1);
	current_xy_pixels.reserve(history_size + 1);
}

Mat GhostImageManager::Ghost_frame(Mat frame) {

	cvtColor(frame, frame_gray, COLOR_RGB2GRAY); //перекрашиваем цветное в ч\б 

	//imshow("frame_gray", frame_gray);

	Mat	result_frame = frame_gray.clone();

	if (blured_frame.rows == 0)
		blured_frame = frame_gray.clone();

	if (frame_num % history_size == 0) //накопив изображение в циклах ниже, добавляем его сюда
		all_blurred.push_back(blured_frame.clone());

	if (all_blurred.size() >= history_size)//храним не более history_size
		all_blurred.erase(all_blurred.begin());

	for (int y = 0; y < frame_gray.rows; y++)
	{
		for (int x = 0; x < frame_gray.cols; x++)
		{
			//это каждый кадр
			uint8_t* pixelPtr0 = (uint8_t*)frame_gray.data;
			uchar Pixel0 = pixelPtr0[y * frame_gray.cols + x];

			uint8_t* pixelPtr1 = (uint8_t*)blured_frame.data;
			uchar& Pixel1 = pixelPtr1[y * blured_frame.cols + x];
			float dif = (Pixel0 - int(Pixel1));
			int blurxy = int(round(dif / 8));
			Pixel1 += blurxy; // накапливаем blured_frame, добавляем к нему 1/8 от разности пикселей с frame

			current_xy_pixels.clear();
			//это раз в history_size
			if (frame_num % history_size == 0) //каждый в history_size считаем медиану по полученным all_blurred изображениеям
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
						Pixel2 = median; //сформировали result_frame
					}
				}
		}
	}

	if (frame_num % history_size == 0)
		imshow("frame_ghost", result_frame);

	frame_num++;

	return result_frame;
}