#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    VideoCapture cap(0);
    if (!cap.isOpened())
    {
        cerr << "Error opening camera." << endl;
        return -1;
    }

    Mat frame, prev_frame, diff_frame, threshold_frame;
    Mat background;
    namedWindow("Camera", WINDOW_AUTOSIZE);

    Rect roi(cap.get(CAP_PROP_FRAME_WIDTH) - cap.get(CAP_PROP_FRAME_WIDTH) / 2, 0, cap.get(CAP_PROP_FRAME_WIDTH) / 2, cap.get(CAP_PROP_FRAME_HEIGHT) / 2);

    for (int i = 0; i < 30; i++)
    {
        cap >> background;
        flip(background, background, 1);
        cvtColor(background, background, COLOR_BGR2GRAY);
        background = background(roi);
    }

    while (true)
    {
        cap >> frame;
        flip(frame, frame, 1);

        cvtColor(frame, frame, COLOR_BGR2GRAY);
        frame = frame(roi); // Kameranin sag ust kosesini alir

        // iki kare farki
        absdiff(background, frame, diff_frame);

        // Fark resmini esikler
        threshold(diff_frame, threshold_frame, 25, 255, THRESH_BINARY);

        // Gurultuyu azaltmaya calisir
        erode(threshold_frame, threshold_frame, getStructuringElement(MORPH_RECT, Size(3, 3)));
        dilate(threshold_frame, threshold_frame, getStructuringElement(MORPH_RECT, Size(3, 3)));
        erode(threshold_frame, threshold_frame, getStructuringElement(MORPH_RECT, Size(3, 3)));
        dilate(threshold_frame, threshold_frame, getStructuringElement(MORPH_RECT, Size(3, 3)));

        // Kenarları tespit etmek icin Canny algoritmasi
        Canny(threshold_frame, threshold_frame, 50, 150, 3);

        vector<vector<Point>> contours;
        findContours(threshold_frame, contours, RETR_TREE, CHAIN_APPROX_SIMPLE);

        // En buyuk konturu bulur
        int largest_contour_index = -1;
        double largest_area = 0.0;
        for (int i = 0; i < contours.size(); i++)
        {
            double area = contourArea(contours[i]);
            if (area > largest_area)
            {
                largest_area = area;
                largest_contour_index = i;
            }
        }

        if (largest_contour_index >= 0)
        {
            drawContours(frame, contours, largest_contour_index, Scalar(0, 255, 0), 2);
            Rect bounding_rect = boundingRect(contours[largest_contour_index]);
            Point center = (bounding_rect.tl() + bounding_rect.br()) / 2;
            int x = center.x;
            int y = center.y;
            cout << "x: " << x << ", y: " << y << endl;
        }

        imshow("Camera", frame);

        if (waitKey(1) == 'q')
        {
            break;
        }
    }
}