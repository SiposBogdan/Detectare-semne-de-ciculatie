#include <opencv2/opencv.hpp>
#include <iostream>
using namespace std;
using namespace cv;


typedef struct{
    int size;
    int* di;
    int* dj;
} neighborhood_structure;



bool IsInside(int img_rows, int img_cols, int i, int j){

    return (i >= 0 && i < img_rows && j >= 0 && j < img_cols);
}

Mat dilation(Mat source, neighborhood_structure neighborhood, int no_iter){

    Mat dst, aux;
    int rows, cols;

    dst = source.clone();
    rows = source.rows;
    cols = source.cols;
    for (int n = 0; n < no_iter; n++) {
        aux = dst.clone();
        for (int i=0; i < rows; i++) {
            for (int j=0; j < cols; j++) {
                dst.at<uchar>(i, j) = 255;
            }
        }

        for (int i=0; i < rows; i++) {
            for (int j = 0; j < cols;j++) {
                if (aux.at<uchar>(i, j) == 0) {
                    for (int k=0; k < neighborhood.size; k++) {
                        int ni = i + neighborhood.di[k];
                        int nj = j + neighborhood.dj[k];
                        if (IsInside(rows, cols, ni, nj)) {
                            dst.at<uchar>(ni, nj) = 0;
                        }
                    }
                }
            }
        }
    }

    return dst;

}

Mat erosion(Mat source, neighborhood_structure neighborhood, int no_iter){

    Mat dst, aux;
    int rows, cols;

    dst = source.clone();
    rows = source.rows;
    cols = source.cols;


    for (int n = 0; n < no_iter; n++) {
        aux = dst.clone();
        for (int i=0; i < rows; i++) {
            for (int j=0; j < cols; j++) {
                dst.at<uchar>(i, j) = 255;
            }
        }

        for (int i=0; i < rows; i++) {
            for (int j = 0; j < cols;j++) {
                int ctrl = 0;
                for (int k=0; k < neighborhood.size; k++) {
                    int ni = i + neighborhood.di[k];
                    int nj = j + neighborhood.dj[k];
                    if (!IsInside(rows, cols, ni, nj) || aux.at<uchar>(ni, nj) == 255) {
                        ctrl = 1;
                        break;
                    }
                }
                if (ctrl) {
                    dst.at<uchar>(i, j) = 0;
                }
            }

        }

    }


    return dst;
}

Mat opening(Mat source, neighborhood_structure neighborhood, int no_iter) {

    Mat dst, aux;

    return dilation(erosion(std::move(source), neighborhood, no_iter), neighborhood, no_iter);

}

Mat closing(Mat source, neighborhood_structure neighborhood, int no_iter) {

    Mat dst, aux;

    return erosion(dilation(std::move(source), neighborhood, no_iter), neighborhood, no_iter);

}




Mat bgrToHsv(const Mat& bgr) {
    CV_Assert(bgr.type() == CV_8UC3);
    Mat hsv(bgr.size(), CV_8UC3);

    for (int i = 0; i < bgr.rows; ++i) {
        for (int j = 0; j < bgr.cols; ++j) {
            Vec3b pixel = bgr.at<Vec3b>(i, j);
            float b = pixel[0] / 255.0f;
            float g = pixel[1] / 255.0f;
            float r = pixel[2] / 255.0f;

            float maxc = max({ r, g, b });
            float minc = min({ r, g, b });
            float delta = maxc - minc;

            float h = 0, s = 0, v = maxc;

            if (delta != 0) {
                if (maxc == r)
                    h = 60 * fmod(((g - b) / delta), 6.0f);
                else if (maxc == g)
                    h = 60 * (((b - r) / delta) + 2);
                else if (maxc == b)
                    h = 60 * (((r - g) / delta) + 4);
            }

            if (h < 0) h += 360;
            if (maxc != 0)
                s = delta / maxc;

            hsv.at<Vec3b>(i, j)[0] = static_cast<uchar>(h / 2);       // [0,180]
            hsv.at<Vec3b>(i, j)[1] = static_cast<uchar>(s * 255);     // [0,255]
            hsv.at<Vec3b>(i, j)[2] = static_cast<uchar>(v * 255);     // [0,255]
        }
    }

    return hsv;
}


double contourAreaNaive(const std::vector<cv::Point>& contour) {
    double area = 0.0;
    int n = contour.size();

    if (n < 3) return 0.0;

    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        area += (contour[i].x * contour[j].y) - (contour[j].x * contour[i].y);
    }

    return std::abs(area) / 2.0;
}

double arcLengthNaive(const vector<Point>& contour, bool closed = true) {
    double length = 0.0;
    int n = contour.size();

    if (n < 2) return 0.0;

    for (int i = 0; i < n - 1; ++i) {
        double dx = contour[i + 1].x - contour[i].x;
        double dy = contour[i + 1].y - contour[i].y;
        length += std::sqrt(dx * dx + dy * dy);
    }

    if (closed) {
        double dx = contour[0].x - contour[n - 1].x;
        double dy = contour[0].y - contour[n - 1].y;
        length += std::sqrt(dx * dx + dy * dy);
    }

    return length;
}


Rect boundingBoxManual(const vector<Point>& points) {
    if (points.empty()) return {0, 0, 0, 0};

    int minX = points[0].x;
    int maxX = points[0].x;
    int minY = points[0].y;
    int maxY = points[0].y;

    for (const auto& p : points) {
        if (p.x < minX) minX = p.x;
        if (p.x > maxX) maxX = p.x;
        if (p.y < minY) minY = p.y;
        if (p.y > maxY) maxY = p.y;
    }

    return Rect(minX, minY, maxX - minX, maxY - minY);
}


bool esteOctogon(const vector<Point>& aproximat) {
    if (aproximat.size() < 7 || aproximat.size() > 9)
        return false;
    Rect rect = boundingBoxManual(aproximat);
    float aspect = (float) rect.width / rect.height;
    return aspect > 0.8 && aspect < 1.6;
}



bool esteTriunghi(const vector<Point>& aproximat) {
    if (aproximat.size() == 3)
        return true;

    if (aproximat.size() == 4) {
        Rect rect = boundingBoxManual(aproximat);
        float aspect = (float) rect.width / rect.height;
        return aspect > 0.6 && aspect < 0.9;
    }
    return false;
}

bool esteCerc(const vector<Point>& contur, const vector<Point>& aproximat) {


    double arie = contourAreaNaive(contur);
    double perimetru = arcLengthNaive(contur, true);

    if (perimetru == 0) return false;

    double circular = 4 * CV_PI * arie / (perimetru * perimetru);

    return circular > 0.75 && aproximat.size() > 6;
}

bool estePatrat(const vector<Point>& aproximat) {

    if (aproximat.size() != 4)
        return false;
    Rect rect = boundingBoxManual(aproximat);
    float aspect = (float) rect.width / rect.height;
    return aspect > 0.9 && aspect < 1.4;
}



bool areCuloare(const Mat& hsv, const vector<Point>& contur, Scalar hmin, Scalar hmax, float prag = 0.2) {

    Mat masca = Mat::zeros(hsv.size(), CV_8UC1);
    drawContours(masca, vector<vector<Point>>{contur}, 0, Scalar(255), FILLED);

    Mat hsv_taiat;
    hsv.copyTo(hsv_taiat, masca);

    Mat mask_color;
    inRange(hsv_taiat, hmin, hmax, mask_color);

    double total = countNonZero(masca);
    double culoare_dorita = countNonZero(mask_color);

    return (total > 0) && (culoare_dorita / total > prag);
}

bool equal_mat(Mat A, Mat B){
    //TODO: Implement a function that returns true is all elements from A are equal to all elements in B
    // assumption that A and B has the same size

    //*****START OF YOUR CODE (DO NOT DELETE/MODIFY THIS LINE)*****
    for (int i=0; i < A.rows; i++) {
        for (int j=0; j < A.cols; j++) {
            if (A.at<uchar>(i, j) != B.at<uchar>(i, j)) {
                return false;
            }
        }
    }


    //*****END OF YOUR CODE(DO NOT DELETE / MODIFY THIS LINE) *****

    return true;

}

Mat region_filling(Mat source, neighborhood_structure neighborhood) {

    //TODO: Implement the region filling algorithm for no_iter times using the structuring element defined by
    // the neighborhood argument

    Mat dst;
    int rows, cols;

    //*****START OF YOUR CODE (DO NOT DELETE/MODIFY THIS LINE)*****
    dst = source.clone();
    Mat m;
    bitwise_not(dst, m);
    Mat n = Mat::zeros(source.rows, source.cols, CV_8UC1);
    n.at<uchar>(0, 0) = 255;
    while (true) {
        Mat prev = n.clone();
        n = dilation(prev, neighborhood, 1) & m;
        if (equal_mat(prev, n)) {
            break;
        }
    }
    //*****END OF YOUR CODE(DO NOT DELETE / MODIFY THIS LINE) *****

    return dst;

}

bool overlapsWithOtherColor(const vector<Point>& contour, const vector<vector<Point>>& alte_contururi) {
    Rect r1 = boundingBoxManual(contour);

    for (const vector<Point>& alte : alte_contururi) {
        Rect r2 = boundingBoxManual(alte);

        Rect intersectie = r1 & r2;
        double zonaInter = intersectie.area();
        double zonaMin = min(r1.area(), r2.area());

        if (zonaInter > 0.1 * zonaMin)
            return true;
    }
    return false;
}


int main() {
    Mat img = imread("D:/sign detection/images/imag3.jpg");
    Mat copie_original = img.clone();
    if (img.empty()) {
        cerr << "Nu s-a putut incarca imaginea!\n";
        return -1;
    }

    Mat hsv, blurr;
    GaussianBlur(img, blurr, Size(5, 5), 1.5);

    hsv = bgrToHsv(blurr);

    Scalar red_low1(0, 100, 40), red_high1(10, 255, 255);
    Scalar red_low2(170, 110, 40), red_high2(180, 255, 255);
    Scalar blue_low(85, 100, 100);
    Scalar blue_high(130, 255, 240);

    Mat mask_red1, mask_red2, mask_red, mask_blue;
    inRange(hsv, red_low1, red_high1, mask_red1);
    inRange(hsv, red_low2, red_high2, mask_red2);
    mask_red = mask_red1 | mask_red2;
    inRange(hsv, blue_low, blue_high, mask_blue);

    int di[4] = {-1, 1, 0, 0};
    int dj[4] = {0, 0, -1, 1};

    neighborhood_structure neighborhood;
    neighborhood.size = 4;
    neighborhood.di = di;
    neighborhood.dj = dj;

    mask_red = opening(mask_red, neighborhood, 1);
    mask_red = closing(mask_red, neighborhood, 1);
    mask_red = dilation(mask_red, neighborhood, 1);

    Mat flood_red = mask_red.clone();
    floodFill(flood_red, Point(0, 0), Scalar(255));
    bitwise_not(flood_red, flood_red);
    mask_red = mask_red | flood_red;

    mask_blue = opening(mask_blue, neighborhood, 1);
    mask_blue = closing(mask_blue, neighborhood, 1);
    mask_blue = dilation(mask_blue, neighborhood, 1);

    Mat flood_blue = mask_blue.clone();
    floodFill(flood_blue, Point(0, 0), Scalar(255));
    bitwise_not(flood_blue, flood_blue);
    mask_blue = mask_blue | flood_blue;

    //imshow("Mask Red", mask_red);
    //imshow("Mask Blue", mask_blue);

    // SEMNELE ROSII
    vector<vector<Point>> contururi_red;
    findContours(mask_red, contururi_red, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (const auto& contur : contururi_red) {

        if (contourAreaNaive(contur) < 400) continue;

        vector<Point> aproximat;

        approxPolyDP(contur, aproximat, 0.03 * arcLengthNaive(contur, true), true);

        Rect r = boundingBoxManual(aproximat);

        bool isRed = areCuloare(hsv, contur, red_low1, red_high1) || areCuloare(hsv, contur, red_low2, red_high2);

        if (esteTriunghi(aproximat) && isRed) {
            rectangle(img, r, Scalar(0, 255, 255), 2);
            putText(img, "TRIUNGHI ROSU", r.tl(), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);

        } else if (esteCerc(contur, aproximat) && isRed) {
            rectangle(img, r, Scalar(255, 0, 255), 2);
            putText(img, "CERC ROSU", r.tl(), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 0, 255), 2);

        } else if (esteOctogon(aproximat) && isRed) {
            rectangle(img, r, Scalar(0, 255, 255), 2);
            putText(img, "STOP", r.tl(), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 0, 255), 2);
        }
    }



    //SEMNE ALBASTRE
    vector<vector<Point>> contururi_blue;
    findContours(mask_blue, contururi_blue, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    for (const auto& contur : contururi_blue) {

        if (contourAreaNaive(contur) < 400) continue;

        vector<Point> aproximat;
        approxPolyDP(contur, aproximat, 0.03 * arcLengthNaive(contur, true), true);
        Rect r = boundingBoxManual(aproximat);

        if (esteCerc(contur, aproximat) && areCuloare(hsv, contur, blue_low, blue_high)) {
            rectangle(img, r, Scalar(255, 255, 0), 2);
            putText(img, "CERC ALBASTRU", r.tl(), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(255, 0, 0), 2);

        } else if (estePatrat(aproximat) && areCuloare(hsv, contur, blue_low, blue_high)) {
            rectangle(img, r, Scalar(0, 255, 0), 2);

            putText(img, "PATRAT ALBASTRU", r.tl(), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(0, 150, 0), 2);
        }
    }


    imshow("Imagine originala", copie_original);
    imshow("Semne detectate", img);
    waitKey(0);
    return 0;
}
