#include <opencv2/opencv.hpp>
#include <vector>
#include <string>


std::vector<cv::Point2f> getBanknoteContour(cv::FileStorage& json, std::string& filename) {
  std::vector<cv::Point2f> contour;
  for (int i = 0; i < 4; ++i) {
    float x = (float)json[filename]["regions"][0]["shape_attributes"]["all_points_x"][i];
    float y = (float)json[filename]["regions"][0]["shape_attributes"]["all_points_y"][i];
    cv::Point2f point(x, y);
    contour.push_back(point);
  }

  return contour;
}


int main() {
  const std::string IMAGE_FOLDER = "../raw_data/comparison/punk/";
  const std::string JSON_NAME = "comparison_punk.json";
  std::vector<std::vector<std::string>> IMAGES {
//      {"100-copy.png", "100-orig.png"},
//      {"500-copy.png", "500-orig.png"},
      {"1000-copy.png", "1000-orig.png"},
  };
  std::vector<cv::Point2f> standardPerspective{ cv::Point2f(0.0f, 0.0f), cv::Point2f(250.0f, 0.0f), cv::Point2f(250.0f, 110.0f), cv::Point2f(0.0f, 110.0f) };
  cv::FileStorage contours = cv::FileStorage(IMAGE_FOLDER + JSON_NAME, cv::FileStorage::Mode::READ);

  cv::namedWindow("copy");
  for (std::vector<std::string> imgPair: IMAGES) {
    std::string copyFilename = imgPair[0];
    std::string origFilename = imgPair[1];

    cv::Mat copyImg = cv::imread(IMAGE_FOLDER + copyFilename);
    cv::Mat origImg = cv::imread(IMAGE_FOLDER + origFilename);

    std::vector<cv::Point2f> copyContour = getBanknoteContour(contours, copyFilename);
    std::vector<cv::Point2f> origContour = getBanknoteContour(contours, origFilename);

    cv::Mat copyTransformMatrix = cv::getPerspectiveTransform(copyContour, standardPerspective);
    cv::Mat origTransformMatrix = cv::getPerspectiveTransform(origContour, standardPerspective);

    cv::Size imgSize(250, 110);
    cv::Mat copyDst;
    cv::Mat origDst;
    cv::warpPerspective(copyImg, copyDst, copyTransformMatrix, imgSize);
    cv::warpPerspective(origImg, origDst, origTransformMatrix, imgSize);
    cv::imwrite(copyFilename, copyDst);
    cv::imwrite(origFilename, origDst);
  }

  contours.release();
  return 0;
}
