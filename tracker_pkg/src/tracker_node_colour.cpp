#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <std_msgs/UInt32MultiArray.h>
#include <sensor_msgs/image_encodings.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/aruco.hpp>
#include <vector>
static const std::string OPENCV_WINDOW = "Image window";

class Tracker
{
    ros::NodeHandle nh_;
    image_transport::ImageTransport it_;
    image_transport::Subscriber image_sub_;
    ros::Publisher pub;
    cv::Mat camera_matrix, dist_coeff; // These can be set after camera calibration to undistort image
    cv::Ptr<cv::aruco::DetectorParameters> parameters;
    cv::Ptr<cv::aruco::Dictionary> dictionary;
    // cv::Mat kernel; // for sharpening; negligible effect observed

public:
    Tracker()
        : it_(nh_)
    {
        // Subscribe to camera
        bool crop = false; // true for cropper use
        nh_.getParam("tracker/crop", crop);
        if (crop)
        {
            image_sub_ = it_.subscribe("image_cropped", 20, &Tracker::imageCb, this);
        }
        else
        {
            image_sub_ = it_.subscribe("/image_rect_color", 20, &Tracker::imageCb, this);
        }

        lowerRedColor_ = {136, 87, 111};
        upperRedColor_ = {180, 255, 255};

        lowerYellowColor_ = {25, 52, 72};
        upperYellowColor_ = {102, 255, 255};

        // Advertise a list of integers
        pub = nh_.advertise<std_msgs::UInt32MultiArray>("tracker/positions_stamped", 20);

        // Set aruco dictionary to use for marker detection
        dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);

        parameters = cv::aruco::DetectorParameters::create();

        // Modify parameters to improve marker detection
        parameters->adaptiveThreshWinSizeMin = 3;
        parameters->adaptiveThreshWinSizeMax = 53; // wider range handles more lighting variation
        parameters->adaptiveThreshWinSizeStep = 2;
        parameters->minDistanceToBorder = 1;
        parameters->minMarkerPerimeterRate = 0.01;     // detect smaller/farther markers
        parameters->perspectiveRemovePixelPerCell = 4; // lower = more tolerant for small markers
        parameters->errorCorrectionRate = 0.9;         // more lenient dictionary matching
        parameters->cornerRefinementMethod = cv::aruco::CORNER_REFINE_SUBPIX;

        //// Kernel for image sharpening
        // float entries[9] = {0, -1, 0, -1, 5, -1, 0, -1, 0};
        // kernel = cv::Mat(3, 3, CV_32F, entries);
    }

    // Subscriber callback function
    void imageCb(const sensor_msgs::ImageConstPtr &msg)
    {
        cv_bridge::CvImagePtr cv_ptr, cv_ptr_contours;

        try
        {
            cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
        }
        catch (cv_bridge::Exception &e)
        {
            ROS_ERROR("cv_bridge exception: %s", e.what());
        }

        // Get time stamp
        ros::Time stamp = cv_ptr->header.stamp;

        // List that will store detection results
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> corners;

        cv::aruco::detectMarkers(cv_ptr->image, dictionary, corners, ids, parameters);

        std_msgs::UInt32MultiArray positions_stamped;
        positions_stamped.data.push_back(stamp.sec);
        positions_stamped.data.push_back(stamp.nsec);
        for (int i = 0; i < corners.size(); i++)
        {
            positions_stamped.data.push_back(ids.at(i));
            for (int j = 0; j < corners.at(i).size(); j++)
            {
                positions_stamped.data.push_back(corners.at(i).at(j).x);
                positions_stamped.data.push_back(corners.at(i).at(j).y);
            }
        }

        // Detect color
        cv::inRange(cv_ptr, Scalar(136, 87, 111), Scalar(180, 255, 255), cv_ptr_contours);

        // Publish
        pub.publish(positions_stamped);
    }

private:
    std::vector<int> lowerRedColor_[3];
    std::vector<int> upperRedColor_[3];

    std::vector<int> lowerYellowColor_[3];
    std::vector<int> upperYellowColor_[3];

    cv::Mat redMask_;
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "tracker");
    Tracker tracker_object;
    ros::spin();
    return 0;
}
