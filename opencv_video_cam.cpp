// opencv_video_cam.cpp : Defines the entry point for the console application.


#include "stdafx.h"
#include <chrono>
#include <thread>
#include <sstream>
#include <string>
#include "opencv2\opencv.hpp"
#include "opencv2\core.hpp"	
#include "opencv2\imgcodecs.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\highgui.hpp"
#include "opencv2\calib3d.hpp"
#include <stdint.h>
#include <Windows.h>
#include <iostream>
#include <algorithm>
#include <bitset> 

using namespace std;
using namespace cv;

Mat hwnd2mat(HWND hwnd)
{
	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	srcheight = windowsize.bottom;
	srcwidth = windowsize.right;
	height = windowsize.bottom / 1;  //change this to whatever size you want to resize to
	width = windowsize.right / 1;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

																									   // avoid memory leak
	DeleteObject(hbwindow);
	DeleteDC(hwindowCompatibleDC);
	ReleaseDC(hwnd, hwindowDC);

	return src;
}

/***************************************/
const int KERNEL = 5;
RNG rng(12345);
Mat preProcess(Mat src, int tv)
{
	Mat gray;
	cvtColor(src, gray, COLOR_BGR2GRAY);
	Mat blur, out;
	GaussianBlur(gray, blur, Size(KERNEL, KERNEL), 0, 0);
	threshold(blur, out, tv, 255, 0);
	return out;
}

const int MAXAREA = 300;
const int THRESH = 175;

const string imgName = "images/poker/THRESH175/img_";


bool findMatch(string inputname, string inputtest);
void calcOdds(vector<string> cards, int numPlayers);

/// BLACK IS 0!!!!!!!!!!!!!!!!!!!!!!!

int main(int argc, char **argv)
{
	HWND hwndDesktop = GetDesktopWindow();
	//namedWindow("output", WINDOW_NORMAL);
	//namedWindow("a", WINDOW_NORMAL); 
	namedWindow("b", WINDOW_NORMAL); 
	namedWindow("c", WINDOW_NORMAL);


	int key = 0;
	int counter = 0;
	ostringstream name;

	Mat src = hwnd2mat(hwndDesktop);
	/*****************STORED CARDS FOR PARSING*****************/
	
	//for (int T_COUNT = 1; T_COUNT < 8; T_COUNT++)
	//{}
	//string newImg(imgName + to_string(T_COUNT) + ".png");
	//Mat src = imread(samples::findFile(newImg), IMREAD_UNCHANGED);
	
	
	
	
	Mat newM = preProcess(src, THRESH);



	/*
			vector<vector<Point >> contours;
			vector<Vec4i> hierarchy;
			//Mat newM = preProcess(src, 175);
			findContours(newM, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);
			//Mat newM = preProcess(src, THRESH);
			Mat drawing = Mat::zeros(newM.size(), CV_8UC3);
			//sort by size
			for (size_t i = 0; i < contours.size() - 1; ++i)
			{
				for (size_t j = 0; j < contours.size() - i - 1; ++j)
				{
					if (contourArea(contours.at(j)) > contourArea(contours.at(j + 1)))
					{
						vector<Point> temp;
						temp = contours.at(j);
						contours.at(j) = contours.at(j + 1);
						contours.at(j + 1) = temp;
					}
				}

			}
			vector<vector<Point>> cards;
			for (size_t i = 0; i < contours.size(); ++i)
			{
				double areaA = contourArea(contours.at(i));
				if (areaA >(double)15000 && areaA < (double)31000)
				{
					cards.push_back(contours.at(i));

				}
			}
			vector<vector<Point>> hull(cards.size());
			for (int i = 0; i < cards.size(); ++i)
			{
				convexHull(cards[i], hull[i]);
			}
			//moments of card hulls
			vector<Moments> mu(hull.size());
			for (size_t i = 0; i < hull.size(); i++)
			{
				mu[i] = moments(hull[i], false);
			}
			// get the centroid of card hulls
			vector<Point2f> mc(hull.size());
			for (size_t i = 0; i < hull.size(); i++)
			{
				mc[i] = Point2f((mu[i].m10 / mu[i].m00), (mu[i].m01 / mu[i].m00));
			}
			//for (size_t i = 0; i < cards.size(); ++i)
			//{
			//	vector<Point> approx; double area0 = contourArea(cards.at(i));
			//	approxPolyDP(cards.at(i), approx, 5, true);
			//	double area1 = contourArea(approx);
			//	cout << "area 0: " << area0 << " | " << "area 1: " << area1 << endl;
			//}

			for (size_t i = 0; i < hull.size(); ++i)
			{
				Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
				//drawContours(drawing, cards, (int)i, color, 1, LINE_8, noArray(), 0, Point());
				drawContours(drawing, hull, (int)i, color);
				circle(drawing, mc[i], 2, color, -1, 8, 0);
			}
			//imshow("output", drawing);


			int w = 135;
			int h = 165;
			int count = 0;
			for (size_t i = 0; i < mc.size(); i++)
			{
				int x0 = mc[i].x - floor(w / 2);
				int y0 = mc[i].y - floor(h / 2);


				Mat ROI(src, Rect(x0, y0, w, h));
				Mat cropped;
				ROI.copyTo(cropped);
				Mat finalMat = preProcess(cropped, THRESH);
				string name_ = "images/poker/cards/cropped_Ex";
				name_.append(std::to_string(count));
				name_.append(".");
				//name_.append(to_string(T_COUNT));
				name_.append(".png");
				imwrite(name_, finalMat);

				count++;
			}



	
	*/
	
	
	
    const string TESTSTR = "images/poker/cards/test.png";
	Mat test = imread(TESTSTR);
	const string ranks[13] = { "2", "3", "4", "5","6", "7", "8", "9", "T", "J", "Q", "K", "A" };
	const string vals[4] = { "h", "c", "s", "d" };
	string ranksh[13] = { "2h", "3h", "4h", "5h","6h", "7h", "8h", "9h", "Th", "Jh", "Qh", "Kh", "Ah"};
	string ranksc[13] = { "2c", "3c", "4c", "5c","6c", "7c", "8c", "9c", "Tc", "Jc", "Qc", "Kc", "Ac" };
	string ranksd[13] = { "2d", "3d", "4d", "5d","6d", "7d", "8d", "9d", "Td", "Jd", "Qd", "Kd", "Ad" };
	string rankss[13] = { "2s", "3s", "4s", "5s","6s", "7s", "8s", "9s", "Ts", "Js", "Qs", "Ks", "As" };
	string suites[4] = { "hearts/", "clubs/", "spades/", "diamonds/" };

	//spades/3s
	//clubs/3c.png

	/*
	string found="blah";
	for (int i = 0; i < 4; ++i)
	{
		bool isFound = false;
		string temp = "images/poker/cards/fulldeck/";
		if (suites[i] == "hearts/")
		{
			string inTemp = temp+suites[i];
			for (int j = 0; j < 13; ++j)
			{
				
				string inTemp2 = inTemp+ ranksh[j]+".png";
				if (findMatch(TESTSTR, inTemp2))
				{
					isFound = true;
					found = inTemp2;
					break;
				}
				else
				{
					inTemp2 = inTemp;
				}
			}
			if (isFound)
			{
				break;
			}
		}
		else if (suites[i] == "clubs/" )
		{
			string inTemp = temp + suites[i];
			for (int j = 0; j < 13; ++j)
			{
				string inTemp2 = inTemp + ranksc[j] + ".png";
				if (findMatch(TESTSTR, inTemp2))
				{
					isFound = true;
					found = inTemp2;
					break;
				}
				else
				{
					inTemp2 = inTemp;
				}
			}
			if (isFound)
			{
				break;
			}
			inTemp = temp;
		}
		else if (suites[i] == "spades/")
		{
			string inTemp = temp + suites[i];
			for (int j = 0; j < 13; ++j)
			{
				string inTemp2 = inTemp + rankss[j] + ".png";
				if (findMatch(TESTSTR, inTemp2))
				{
					isFound = true;
					found = inTemp2;
					break;
				}
				else
				{
					inTemp2 = inTemp;
				}
			}
			if (isFound)
			{
				break;
			}
		}
		else
		{
			string inTemp = temp + suites[i];
			for (int j = 0; j < 13; ++j)
			{
				string inTemp2 = inTemp + ranksd[j] + ".png";
				if (findMatch(TESTSTR, inTemp2))
				{
					isFound = true;
					found = inTemp2;
					break;
				}
				else
				{
					inTemp2 = inTemp;
				}
			}
			if (isFound)
			{
				break;
			}
		}

	}
	*/
	
	//imshow("output", *test);
	

	while (key != 27)
	{

		
		Mat src = hwnd2mat(hwndDesktop);

		// you can do some image processing here

		Sleep(50);


		/*
		imshow(" press t for image frame, spacebar to exit", src);

		if (GetAsyncKeyState(0x54))
		{
			++counter;
			name.str("");//reset the name string
			name << imgName << counter << ".png";

			
			Mat newM = preProcess(src, THRESH);
			

			imwrite(name.str(), newM);
			cout << "wrote img " << counter << " successfully" << endl;
			Sleep(100);
		}
		*/
		
	
		//imshow(" press t for image frame, spacebar to exit", src);

		//bool tempas = findMatch("images/poker/cards/test.png", "images/poker/cards/fulldeck/spades/5s.png");
		//cout << tempas << endl;
		//if (GetAsyncKeyState(0x54)){}
			
			
		
			
			vector<vector<Point >> contours, contID;
			vector<vector<Point>> cards;

			vector<Vec4i> hierarchy;
			Mat newM = preProcess(src, THRESH);
			findContours(newM, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE);

			Mat drawing = Mat::zeros(newM.size(), CV_8UC3);

			//sort by size
			for (size_t i = 0; i < contours.size() - 1; ++i)
			{
				for (size_t j = 0; j < contours.size() - i - 1; ++j)
				{
					if (contourArea(contours.at(j)) > contourArea(contours.at(j + 1)))
					{
						vector<Point> temp;
						temp = contours.at(j);
						contours.at(j) = contours.at(j + 1);
						contours.at(j + 1) = temp;
					}
				}

			}

			
			//push back cards
			for (size_t i = 0; i < contours.size(); ++i)
			{
				double areaA = contourArea(contours.at(i));
				if (areaA >(double)15000 && areaA < (double)50000)
				{
					cout << areaA << endl;
					cards.push_back(contours.at(i));
				}
				else if (areaA > (double)33000 && areaA < (double)39000)
				{
					contID.push_back(contours.at(i));
				}
				else
				{

				}
			}
		
			

			vector<vector<Point>> idHull(contID.size());
			vector<Rect> boundRect(contID.size());

			for (size_t i = 0; i < contID.size(); i++)
			{
				convexHull(contID[i], idHull[i]);
			}
			//moments of  hulls
			vector<Moments> muh(idHull.size());
			for (size_t i = 0; i < idHull.size(); i++)
			{
				muh[i] = moments(idHull[i], false);
			}
			// get the centroid of  hulls
			vector<Point2f> mch(idHull.size());
			for (size_t i = 0; i < idHull.size(); i++)
			{
				mch[i] = Point2f((muh[i].m10 / muh[i].m00), (muh[i].m01 / muh[i].m00));
			}

			for (size_t i = 0; i < idHull.size(); i++)
			{
				approxPolyDP(contID[i], idHull[i], 3, true);
				boundRect[i] = boundingRect(idHull[i]);
			}

			for (size_t i = 0; i < idHull.size(); ++i)
			{
				Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
				//rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 1);
				//circle(drawing, mch[i], 3, color, -1, 8, 0);
			}
			//imshow("b", drawing);



			
			vector<vector<Point>> hull(cards.size());
			//push back hulls
			for (int i = 0; i < cards.size(); ++i)
			{
				convexHull(cards[i], hull[i]);
			}
			//moments of card hulls
			vector<Moments> mu(hull.size());
			for (size_t i = 0; i < hull.size(); i++)
			{
				mu[i] = moments(hull[i], false);
			}
			// get the centroid of card hulls
			vector<Point2f> mc(hull.size());
			for (size_t i = 0; i < hull.size(); i++)
			{
				mc[i] = Point2f((mu[i].m10 / mu[i].m00), (mu[i].m01 / mu[i].m00));
			}

			for (size_t i = 0; i < hull.size(); ++i)
			{
				Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
				//drawContours(drawing, cards, (int)i, color, 1, LINE_8, noArray(), 0, Point());
				drawContours(src, hull, (int)i, color);
				circle(src, mc[i], 3, color, -1, 8, 0);
			}

			vector<string> foundStr;

			int w = 135;
			int h = 165;
			int count = 0;
			for (size_t i = 0; i < mc.size(); i++)
			{
				int x0 = mc[i].x - round(w / 2);
				int y0 = mc[i].y - round(h / 2);


				Mat ROI(src, Rect(x0, y0, w, h));
				Mat cropped;
				ROI.copyTo(cropped);
				//Mat fin=preProcess(cropped, THRESH);
				imwrite(TESTSTR, cropped);

				

			
				//std::this_thread::sleep_for(std::chrono::milliseconds(20));
				string found = "blah";
				string out = "images/poker/cards/fulldeck/";
				//cout << "...looking for card...";
				bool isFound = false;
				for (int i = 0; i < 4; ++i)
				{
					
					
					string temp = out+suites[i];
				
					for (int j = 0; j < 13; ++j)
					{

						string inTemp = temp + ranks[j]+ suites[i].at(0) + ".png";
						string retS = ranks[j] + suites[i].at(0);
						bool tempBool = findMatch(TESTSTR, inTemp);
						//std::this_thread::sleep_for(std::chrono::milliseconds(10));
						if (tempBool)
						{
							isFound = true;
							found = retS;
							break;
						}
						
					}
					if (isFound)
					{
						break;
					}

				}

				foundStr.push_back(found);
				count++;
			}

			for (auto i = foundStr.begin(); i != foundStr.end(); ++i)
			{
				cout << *i << " | ";
			}
			cout << endl;



			Scalar colOut = Scalar(0,0,0);
			int cnt = 0;
			for (auto fsItr = foundStr.begin(); fsItr != foundStr.end(); fsItr++)
			{
				Point orig;
				orig.x = mc[cnt].x;
				orig.y = mc[cnt].y + 150;
				putText(src, (*fsItr), orig, FONT_HERSHEY_PLAIN, 5., colOut);
				cnt++;
			}
			imshow("output", src);
			
			//calcOdds(foundStr, 2);


	//	Sleep(50);
		

		key = waitKey(60); // you can change wait time
	}

}


//Returns string of matching card (string is the name so '2h' is 2 of hearts
bool findMatch(string inputtest, string inputname)
{
	string match="blah";
	
	//, IMREAD_UNCHANGED
	Mat grayk = imread(inputname, IMREAD_GRAYSCALE); Mat Nknown; Mat Ntest;
	Mat gray = imread(inputtest, IMREAD_GRAYSCALE);
	
	
	Mat blur, testA;

	GaussianBlur(gray, blur, Size(KERNEL, KERNEL), 0, 0);
	threshold(blur, testA, THRESH, 255, 0);

	
	//cvtColor(src, gray, COLOR_BGR2GRAY);
	Mat blurK, known;

	GaussianBlur(grayk, blurK, Size(KERNEL, KERNEL), 0, 0);
	threshold(blurK, known, THRESH, 255, 0);



	bitwise_not(known, Nknown, noArray());
	bitwise_not(testA, Ntest, noArray());
	//cout << "test: " << Ntest.size() << endl;
	//cout << "known: " << Nknown.size() << endl;

	Mat mat;
	//cout << "mat: " << mat.size() << endl;

	bitwise_xor(Ntest, Nknown, mat);

	
	
	
	//imshow("a", Ntest);
	//imshow("b", Nknown);
	//imshow("c", mat);

	std::vector<uchar> array;
	if (mat.isContinuous()) {
		// array.assign(mat.datastart, mat.dataend); // <- has problems for sub-matrix like mat = big_mat.row(i)
		array.assign(mat.data, mat.data + mat.total());
	}
	else {
		for (int i = 0; i < mat.rows; ++i) {
			array.insert(array.end(), mat.ptr<uchar>(i), mat.ptr<uchar>(i) + mat.cols);
		}
	}

	int test4 = countNonZero(array);

	if (test4 < 470)
	{
		
		return true;
	}
	//cout << "count" << test4 << endl;
	return false;
}

void calcOdds(vector<string> cards, int numPlayers)
{
	vector<string> hand;
	string temp;
	cout << "enter hand card1: ";
	cin >> temp;
	hand.push_back(temp);
	cout << "enter hand card2: ";
	cin >> temp;
	hand.push_back(temp);
	cout << endl;
	cout << "calculating" << endl;
	vector<string> Deck = 
	{ 
		"2h", "3h", "4h", "5h","6h", "7h", "8h", "9h", "Th", "Jh", "Qh", "Kh", "Ah",
		"2c", "3c", "4c", "5c","6c", "7c", "8c", "9c", "Tc", "Jc", "Qc", "Kc", "Ac",
		"2d", "3d", "4d", "5d","6d", "7d", "8d", "9d", "Tc", "Jd", "Qd", "Kd", "Ad",
		"2s", "3s", "4s", "5s","6s", "7s", "8s", "9s", "Ts", "Js", "Qs", "Ks", "As" 
	};

	int liveDeckCount = 52;
	liveDeckCount = liveDeckCount - (2 * numPlayers);
	vector<string>::iterator hItr = hand.begin();
	vector<string>::iterator itrC = cards.begin();
	char h1[2];
	char h2[2];
	Deck.erase(std::remove(Deck.begin(), Deck.end(), *hItr), Deck.end());
	Deck.erase(std::remove(Deck.begin(), Deck.end(), *(hItr+1)), Deck.end());
	


	switch (cards.size())
	{
	case 0:
		//odds of something on flop

		//royal flush odds
		
		cout << "No cards on table" << endl;
		//straight flush odds
		//four of a kinds
		//full house
		//flush
		//straight
		//three of a kind
		//two pair
		//pair 
		break;
	case 3:
		liveDeckCount -= 4;

		
		if ((*itrC)[1] != (*(itrC + 1))[1] || (*(itrC + 1))[1] != (*(itrC + 2))[1])
		{
			//no royal flush
		}
		else
		{
			char arr[5] = { 'T', 'J', 'Q', 'K', 'A' };
			std::bitset<3> rF;
			int count = 0;
			while (itrC != cards.end())
			{
				for (int i = 0; i < 5; ++i)
				{
					if (arr[i] == (*itrC)[0])
					{
						rF.set(count, 1);
					}
				}
				count++;
				itrC++;
			}
			if (rF.all())
			{
				//royal flush potential
				cout << "YES" << endl;
			}
			else
			{
				//nope
				cout << "royal flush: low" << " | ";
			}
		}



		//odds of something on turn
		break;
	case 4: 
		liveDeckCount -= 2;
		cout << "4 cards on table" << endl;
		//odds of something on river
		break;
	case 5:
		cout << "5 cards on table" << endl;
		liveDeckCount -= 2;
		//odds done, outcome set
		break;
	default:
		break;
	}
}




















//#include <sstream>
//#include <iostream>
//#include <fstream>
//
//
//using namespace std; 
//using namespace cv;
//
//
//int main(int argc, char ** argv)
//{
//
//	const char* filename = "images/fl.jfif";
//
//	mat i = imread(filename, imread_grayscale);
//	if (i.empty()) {
//		cout << "error opening image" << endl;
//		return -1;
//	}
//
//	//! [expand]
//	mat padded;                            //expand input image to optimal size
//	int m = getoptimaldftsize(i.rows);
//	int n = getoptimaldftsize(i.cols); // on the border add zero values
//	copymakeborder(i, padded, 0, m - i.rows, 0, n - i.cols, border_constant, scalar::all(0));
//	//! [expand]
//
//	//! [complex_and_real]
//	mat planes[] = { mat_<float>(padded), mat::zeros(padded.size(), cv_32f) };
//	mat complexi;
//	merge(planes, 2, complexi);         // add to the expanded another plane with zeros
//										//! [complex_and_real]
//
//										//! [dft]
//	dft(complexi, complexi);            // this way the result may fit in the source matrix
//										//! [dft]
//
//										// compute the magnitude and switch to logarithmic scale
//										// => log(1 + sqrt(re(dft(i))^2 + im(dft(i))^2))
//										//! [magnitude]
//	split(complexi, planes);                   // planes[0] = re(dft(i), planes[1] = im(dft(i))
//	magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
//	mat magi = planes[0];
//	//! [magnitude]
//
//	//! [log]
//	magi += scalar::all(1);                    // switch to logarithmic scale
//	log(magi, magi);
//	//! [log]
//
//	//! [crop_rearrange]
//	// crop the spectrum, if it has an odd number of rows or columns
//	magi = magi(rect(0, 0, magi.cols & -2, magi.rows & -2));
//
//	// rearrange the quadrants of fourier image  so that the origin is at the image center
//	int cx = magi.cols / 2;
//	int cy = magi.rows / 2;
//
//	mat q0(magi, rect(0, 0, cx, cy));   // top-left - create a roi per quadrant
//	mat q1(magi, rect(cx, 0, cx, cy));  // top-right
//	mat q2(magi, rect(0, cy, cx, cy));  // bottom-left
//	mat q3(magi, rect(cx, cy, cx, cy)); // bottom-right
//
//	mat tmp;                           // swap quadrants (top-left with bottom-right)
//	q0.copyto(tmp);
//	q3.copyto(q0);
//	tmp.copyto(q3);
//
//	q1.copyto(tmp);                    // swap quadrant (top-right with bottom-left)
//	q2.copyto(q1);
//	tmp.copyto(q2);
//	//! [crop_rearrange]
//
//	//! [normalize]
//	normalize(magi, magi, 0, 1, norm_minmax); // transform the matrix with float values into a
//											  // viewable image form (float between values 0 and 1).
//											  //! [normalize]
//
//	imshow("input image", i);    // show the result
//	imshow("spectrum magnitude", magi);
//	waitkey();
//
//	return 0;
//}






















//gaussian params
//int ddepth = CV_16S;
//int kernel_size = 3;
//int scale = 1;
//int delta = 0;


//Mat src_gray, mat1, mat2, mat3, mat4;
//int thresh = 100;
//RNG rng(12345);
//Mat thresh_callback(int, Mat mat, void*);
//
//int main(int argc, char* argv[])
//{
//	Mat src = imread("images/image_001.png");
//	cvtColor(src, src_gray, COLOR_BGR2GRAY);
//	blur(src_gray, src_gray, Size(3, 3));
//	const char* source_window = "Source";
//	namedWindow(source_window);
//	//resize(src, src, Size(), 0.33, 0.2, INTER_LINEAR);
//	resize(src, src, Size(), .75, .75, INTER_LINEAR);
//	imshow(source_window, src);
//	const int max_thresh = 255;
//	
//	mat4 = thresh_callback(255, mat4, 0);
//	imshow("255", mat4);
//	imwrite("images/tst_contours_a001.png", mat4);
//
//
//	//Mat resistorImg = imread("images/tst_001.jpg");
//	//Mat lap, gray;
//	//Mat splitChannels[3];
//
//	//split(resistorImg, splitChannels);
//
//	/*//manual image split
//	for (int r = 0; r < modImg.rows; r++)
//	{
//		for (int c = 0; c < modImg.cols; c++)
//		{
//			//choose 0,1,2 channel
//			modImg.at<Vec3b>(r, c)[0] = modImg.at<Vec3b>(r, c)[0] = 0;
//
//		}
//
//	}
//	*/
//	
//	//kill a channel 0,1,2 (b,g,r)
//	//splitChannels[0] = Mat::zeros(splitChannels[0].size(), CV_8UC1);
//	/*
//	GaussianBlur(resistorImg, resistorImg, Size(3, 3), 0, 0, BORDER_DEFAULT);
//	cvtColor(resistorImg, gray, COLOR_BGR2GRAY);
//	Mat abs_dst;
//
//	Laplacian(resistorImg, lap, ddepth, kernel_size, scale, delta, BORDER_DEFAULT);
//
//
//	convertScaleAbs(lap, abs_dst);
//	namedWindow("abs dst", WINDOW_AUTOSIZE);
//	imshow("abs dst", abs_dst);
//	Mat output;
//	resize(abs_dst, output, Size(), 0.33,0.33, INTER_LINEAR);
//	imshow("zoom", output);
//
//
//	//Mat output;
//	//merge(splitChannels, 3, output);
//
//
//	*/
//	//imshow("NO B", output);
//	waitKey();
//	return 1;
//
//}
//




////https://docs.opencv.org/master/da/d0c/tutorial_bounding_rects_circles.html
//
//Mat thresh_callback(int, Mat mat, void*)
//{
//	Mat canny_output;
//	Canny(src_gray, canny_output, thresh, thresh * 2);
//	vector<vector<Point> > contours;
//	vector<Vec4i> hierarchy;
//	findContours(canny_output, contours, hierarchy, RETR_TREE,CHAIN_APPROX_TC89_KCOS);
//	Mat drawing = Mat::zeros(canny_output.size(), CV_8UC3);
//	for (size_t i = 0; i< contours.size(); i++)
//	{
//		Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
//		drawContours(drawing, contours, (int)i, color, 2, LINE_AA, hierarchy, 0);
//	}
//	//resize(drawing, mat, Size(), 0.33, 0.2, INTER_LINEAR);
//	resize(drawing, mat, Size(), .75, .75, INTER_LINEAR);
//	return mat;
//	
//}