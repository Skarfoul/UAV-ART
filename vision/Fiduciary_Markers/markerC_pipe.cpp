/*
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "opencv2\opencv.hpp"
using namespace cv;

#include <math.h>
#include <fstream>

#include <iostream>
using namespace std;

#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sys/stat.h>
#include <cstdlib>
#include <stdio.h>

#define _USE_MATH_DEFINES
#include <cmath>
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "opencv2/opencv.hpp"
using namespace cv;
#include <cstdio>
#include <thread>

#include <sys/stat.h>
#include <chrono>
using namespace std;

#define _USE_MATH_DEFINES
#include <cmath>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <sys/stat.h>
#include <cstdlib>

//float camera_matrix[3][3] = { {1.57931456e+03, 0.00000000e+00, 7.37574015e+02},
//								{0.00000000e+00, 1.58393379e+03, 9.31342862e+02},
//								{0.00000000e+00, 0.00000000e+00, 1.00000000e+00} };





cv::Mat four_point_transform(cv::Mat gray, std::vector< Point2f > Candidate) {
	double widthA, widthB, maxWidth, heightA, heightB, maxHeight;
	cv::Mat M, warped;
	widthA = sqrt(pow((Candidate[2].x - Candidate[3].x), 2.0) + pow((Candidate[2].y - Candidate[3].y), 2.0));
	widthB = sqrt(pow((Candidate[1].x - Candidate[0].x), 2.0) + pow((Candidate[1].y - Candidate[0].y), 2.0));
	maxWidth = max(int(widthA), int(widthB));

	heightA = sqrt(pow((Candidate[1].x - Candidate[2].x), 2.0) + pow((Candidate[1].y - Candidate[2].y), 2.0));
	heightB = sqrt(pow((Candidate[0].x - Candidate[3].x), 2.0) + pow((Candidate[0].y - Candidate[3].y), 2.0));
	maxHeight = max(int(heightA), int(heightB));


		std::vector< Point2f > dst;
		dst.resize(4);
		dst[0] = Point2f((float)0, (float)0);
		dst[1] = Point2f((float)(maxWidth - 1), (float)(0));
		dst[2] = Point2f((float)(maxWidth - 1), (float)(maxHeight - 1));
		dst[3] = Point2f((float)(0), (float)(maxHeight - 1));

			M = getPerspectiveTransform(Candidate, dst);
			warpPerspective(gray, warped,M, cv::Size(int(maxWidth), int(maxHeight)));

			return warped;
}


bool compare_distance(std::vector< Point > cnt) {
	float p12, p23, p34, p41, minimum_dist, maximum_dist;
	bool result;

	p12 = sqrt(pow(cnt[0].x - cnt[1].x, 2) + pow(cnt[0].y - cnt[1].y, 2));
	p23 = sqrt(pow(cnt[1].x - cnt[2].x, 2) + pow(cnt[1].y - cnt[2].y, 2));
	p34 = sqrt(pow(cnt[2].x - cnt[3].x, 2) + pow(cnt[2].y - cnt[3].y, 2));
	p41 = sqrt(pow(cnt[0].x - cnt[3].x, 2) + pow(cnt[0].y - cnt[3].y, 2));

	minimum_dist = min(p12, p23);
	minimum_dist = min(minimum_dist, p34);
	minimum_dist = min(minimum_dist, p41);

	maximum_dist = max(p12, p23);
	maximum_dist = max(maximum_dist, p34);
	maximum_dist = max(maximum_dist, p41);

	if (minimum_dist / maximum_dist > 0.9)
		result = true;
	else
		result = false;


	return result;
}


std::vector< std::vector< Point2f > > match_warped(std::vector< std::vector< Point2f > > candidates, cv::Mat gray, std::vector <cv::Mat> warpedCandidates) {
	std::vector< std::vector< Point2f > > markers;
	cv::Mat patch;
	cv::Size sz;
	int patchWidth;
	int patchHeight;
	int white, black, npixel;
	uchar pixel;

	for (unsigned int i = 0; i < candidates.size(); i++) {
		patch = warpedCandidates[i];
		sz = patch.size();
		patchWidth = sz.width;
		patchHeight = sz.height;

		white = 0;
		black = 1;
		npixel = 0;
		for (int h = round(patchHeight*0.40); h < round(patchHeight * 0.60); h++) {
			for (int w = round(patchWidth * 0.40); w < round(patchWidth * 0.60); w++) {
				npixel++;
				pixel = warpedCandidates[i].at<uchar>(h, w);
				if (pixel == 255)
					white++;
				else black++;
			}
		}


		if (float(black) / float(white) < 0.1) {
			black = 0;
			white = 1;

			for (int w = 0; w < patchWidth; w++) {
				for (int h = 0; h < patchHeight; h++) {
					pixel = patch.at<char>(h, w);
					if (pixel == 255)
						white++;
					else black++;
				}
			}
			if (black > white)
			{
				imshow("aq", patch);
				markers.push_back(candidates[i]);
			}

		}


	}

	return markers;

}




cv::Mat compute_marker(cv::Mat img, cv::Mat original, cv::Mat cameraMatrix, cv::Mat distortionMatrix, Mat objPoints, int pipefile)
{
	cv::Mat threshImage, warped;
	std::vector<std::vector<cv::Point> > contours;
	std::vector< std::vector< Point2f > > markers;
	double cnt_len;
	std::vector< std::vector< Point2f > > candidates, biggerCandidates;
	std::vector< std::vector< Point > > contoursOut, biggerContours;


	adaptiveThreshold(img, threshImage, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY, 5, 3);

	findContours(threshImage, contours, RETR_LIST, CHAIN_APPROX_NONE);

	Mat drawing = Mat::zeros(img.size(), CV_8UC3);


	for (unsigned int i = 0; i < contours.size(); i++) {
		std::vector< Point > approxCurve;
		bool issquare;
		cnt_len = arcLength(contours[i], true);


		approxPolyDP(contours[i], approxCurve, double(contours[i].size()) * 0.03, true);

		if (contourArea(approxCurve) > 20 && contourArea(approxCurve) < 0.2 * img.cols * img.rows) {

			if (approxCurve.size() == 4 && isContourConvex(approxCurve)) {
				issquare = compare_distance(approxCurve);
				if (!(issquare)) continue;

				std::vector< Point2f > currentCandidate;
				currentCandidate.resize(4);
				for (int j = 0; j < 4; j++) {
					currentCandidate[j] = Point2f((float)approxCurve[j].x, (float)approxCurve[j].y);
				}
				candidates.push_back(currentCandidate);
				contoursOut.push_back(contours[i]);
			}
		}
	}
	std::vector <cv::Mat> warpedCandidates;

	for (unsigned int i = 0; i < candidates.size(); i++) {
		double dx1 = candidates[i][1].x - candidates[i][0].x;
		double dy1 = candidates[i][1].y - candidates[i][0].y;
		double dx2 = candidates[i][2].x - candidates[i][0].x;
		double dy2 = candidates[i][2].y - candidates[i][0].y;
		double crossProduct = (dx1 * dy2) - (dy1 * dx2);

		if (crossProduct < 0.0) { // not clockwise direction
			swap(candidates[i][1], candidates[i][3]);
		}

		drawContours(drawing, contoursOut, (int)i, (255, 255, 255), 2, LINE_8);
		warped = four_point_transform(img, candidates[i]);
		threshold(warped, warped, 125, 255, THRESH_BINARY | THRESH_OTSU);
		warpedCandidates.push_back(warped);
	}

	markers = match_warped(candidates, img, warpedCandidates);

	cv::Rect rect_aux;

	for (unsigned int i = 0; i < markers.size(); i++)
	{
		rect_aux = boundingRect(markers[i]);
		rectangle(original, rect_aux, 255, 5);

	}

	cv::imshow("Drawing", drawing);

	cv::Mat rVec, rMat, tVec;
	for (unsigned int i = 0; i < markers.size(); i++) //markers.size()
	{
		std::vector<cv::Point2d> tag_image;
		cv::Point2d point;
		tag_image.clear();

		for (size_t ci = 0; ci < 4; ++ci) {
			point.x = markers[i][ci].x;
			point.y = markers[i][ci].y;
			tag_image.push_back(point);

		}

		cv::solvePnP(objPoints, tag_image, cameraMatrix, distortionMatrix, rVec, tVec);
		std::cout << tVec << "\n";
		double Dist = cv::norm(tVec);
		std::string coord_str = "X:" + std::to_string(tVec.at<double>(0)) + "  " + "Y: " + std::to_string(tVec.at<double>(1)) + "  " + "Z: " + std::to_string(tVec.at<double>(2)) + "\n";
		//std::string dist_str = "Distance:" + std::to_string(Dist);

		//std::cout << "TVEC" << tVec << "\n";
		//std::cout << "DISTANCE:" << DIST << "\n";
		cv::putText(original, coord_str, cv::Point(20, 50), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 0, 0), 2, false);
		//std::ofstream outfile;
		//outfile.open("D:/Desktop/IST/UAV/Visao/Aruco/relatorio/ResultsS1bm1920.txt", std::ios_base::app);//std::ios_base::app
		//outfile << float(DIST) << "\n";

		std::string dist_str = std::to_string(Dist);
		std::vector<char> dist_bytes(dist_str.begin(), dist_str.end());
		dist_bytes.push_back('\n');
		dist_bytes.push_back('\0');

		char *ptr = &dist_bytes[0];

		write(pipefile, ptr, sizeof(dist_bytes));

	}




	//cout << camera_matrix[1][1];

	return original;

}



int main()
{
	int pipefile;

	if ((pipefile = open("myfifo", O_WRONLY)) < 0) {
		perror("open");
		exit(-1);
	}




	//VideoCapture cap(0);
	//VideoCapture cap("D:/Desktop/IST/UAV/Visao/Godot/markerC_5h_godot.mp4");
	VideoCapture cap("/home/vasco/Desktop/CODE/UAV_Fiducial_Markers/New_Images/C_fast.MOV");
	cv::Mat img, img_original;
	cap.read(img);
	cv::Size sz = img.size();
	int imageWidth = sz.width;
	int imageHeight = sz.height;
	int dim, width, height, scale_percent = 150;
	width = int(imageWidth * scale_percent / 100);
	height = int(imageHeight * scale_percent / 100);
	bool bSuccess;

	float f = float(imageWidth) / (2.0 * tan(60 * M_PI / 360));
	float Cu = float(imageWidth) / 2.0;
	float Cv = float(imageHeight) / 2.0;

	Mat cameraMatrix = (cv::Mat1d(3, 3) <<  f, 0, Cu, 0,  f,  Cv, 0, 0, 1); //Matrix Godot
	//cameraMatrix.at<double>(0, 0) = f;
	Mat distortionMatrix = (cv::Mat1d(1, 5) << 0.0, 0.0, 0.0, 0.0, 0.0); //Matrix  Godot

	Mat objPoints = cv::Mat(4, 1, CV_32FC3);
	float markerLength = 40.0; //10 cm
	objPoints.ptr< cv::Vec3f >(0)[0] = cv::Vec3f(-markerLength / 2.f, markerLength / 2.f, 0);
	objPoints.ptr< cv::Vec3f >(0)[1] = cv::Vec3f(markerLength / 2.f, markerLength / 2.f, 0);
	objPoints.ptr< cv::Vec3f >(0)[2] = cv::Vec3f(markerLength / 2.f, -markerLength / 2.f, 0);
	objPoints.ptr< cv::Vec3f >(0)[3] = cv::Vec3f(-markerLength / 2.f, -markerLength / 2.f, 0);



	int numframes = 0;
	time_t start, end;
	time(&start);

	while (cap.read(img))
	{
		numframes = numframes+1;

		//cv::resize(img, img, cv::Size(img.cols * 1.5, img.rows * 1.5), 0, 0, cv::INTER_LINEAR);

		img_original = img;
		cvtColor(img, img, COLOR_BGR2GRAY);

		img = compute_marker(img, img_original, cameraMatrix, distortionMatrix, objPoints, pipefile);

		cv::imshow("Window", img);


		if (waitKey(1) == 27)
		{
			std::cout << "Esc key is pressed by user. Stopping the video\n";
			break;
		}
	}
	time(&end);

	double seconds = difftime (end, start);
	double fps  = numframes / seconds;
	cout << "FPS: " << fps;

	if (close(pipefile) < 0) {
		perror("close");
		exit(-1);
	}

	return 0;
}
