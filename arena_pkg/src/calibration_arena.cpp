
#include <ros/ros.h>
#include <ros/package.h>
#include <std_msgs/Bool.h>
#include <opencv2/highgui.hpp>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include <fstream>
#include <vector>

class ArenaCalibration
{
public:
    struct transformerData
    {
        double x_offset;
        double y_offset;
        double angular_offset;
        double scale_factor;
    };

    bool is_finished;
    unsigned int n_points;
    transformerData transformer;

    ArenaCalibration() : image_transport_(node_handle_), is_finished(false), n_points(0)
    {
        image_sub_ = image_transport_.subscribe("/image_rect", 1,
                                                &ArenaCalibration::imageCallback, this);

        std::string path = ros::package::getPath("arena_pkg") + "/config";
        std::string fileName = path + "/tam_points.yaml";

        node_handle_.param("/calibration_arena/x_offset", transformer.x_offset, 0.0);
        node_handle_.param("/calibration_arena/y_offset", transformer.y_offset, 0.0);
        node_handle_.param("/calibration_arena/angular_offset", transformer.angular_offset, 0.0);
        node_handle_.param("/calibration_arena/scale_factor", transformer.scale_factor, 1.0);

        ROS_INFO("the value is %f", transformer.x_offset);

        yaml_.open(fileName, std::fstream::out | std::fstream::trunc);
        if (yaml_.is_open())
        {
            ROS_INFO("The file is open, writing points to %s", fileName.c_str());
        }
        else
        {
            ROS_ERROR("The file cannot open");
        }
    }
    ~ArenaCalibration()
    {
        yaml_.close();
    }

    static void mouseCallback(int event, int x, int y, int flags, void *userdata)
    {
        // Should perhaps check if userdata is not nullptr
        ArenaCalibration *calibrator = reinterpret_cast<ArenaCalibration *>(userdata);
        calibrator->mouseCallback(event, x, y); // Calls class callback function, which is not static
    }

    void mouseCallback(int event, int x, int y)
    {
        if (event == cv::EVENT_RBUTTONDOWN)
        {
            std::cout << "Right mouse button clicked at (" << x << ", " << y << ")" << std::endl;
            if (tam_points_.size() < 1)
            {
                std::cout << "You need a minimum of 1 points!" << std::endl;
                return;
            }

            is_finished = true;

            return;
        }

        if (event == cv::EVENT_LBUTTONDOWN)
        {
            std::cout << "Left mouse button clicked at (" << x << ", " << y << ")" << std::endl;

            cv::drawMarker(img_, cv::Point(x, y), cv::Vec3b(255, 0, 0));
            cv::Point2d newPoints = scalePoints(cv::Point2d(x, y));
            tam_points_.push_back(newPoints);

            // Append to YAML file
            yaml_ << "point_" << n_points << " : [" << newPoints.x << ", " << newPoints.y << "]\n";
            n_points++;

            return;
        }
    }

    void imageCallback(const sensor_msgs::ImageConstPtr &msg)
    {
        cv_bridge::CvImagePtr cv_ptr;
        try
        {
            cv_ptr = cv_bridge::toCvCopy(msg, msg->encoding);
        }
        catch (cv_bridge::Exception &e)
        {
            ROS_ERROR("Cv_brdige execption: %s", e.what());
        }

        img_ = cv_ptr->image;
        // cv::resize(img_, img_resize_, cv::Size(), 0.8, 0.8, cv::INTER_AREA);

        // creating window
        cv::namedWindow("ImageDisplay", cv::WINDOW_AUTOSIZE);

        cv::setMouseCallback("ImageDisplay", mouseCallback, this);

        while (!is_finished)
        {
            cv::imshow("ImageDisplay", img_);
            cv::waitKey(50);
        }
        cv::destroyWindow("ImageDisplay");
    }
    cv::Point2d scalePoints(cv::Point2d point)
    {

        double x = -1 * (-1 * point.x - transformer.x_offset) * transformer.scale_factor;
        double y = -1 * (point.y - transformer.y_offset) * transformer.scale_factor;

        // double x = x_tmp * cos(-transformer.angular_offset) - y_tmp * sin(-transformer.angular_offset);
        // double y = x_tmp * sin(-transformer.angular_offset) + y_tmp * cos(-transformer.angular_offset);
        return cv::Point2d(x, y);
    }

private:
    ros::NodeHandle node_handle_;
    image_transport::ImageTransport image_transport_;
    image_transport::Subscriber image_sub_;
    std::ofstream yaml_;
    std::vector<cv::Point> tam_points_;
    cv::Mat img_, img_resize_;
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "calibratorarareba");
    ArenaCalibration calibrator_object;

    ros::Rate rate(30);
    while (ros::ok() && !calibrator_object.is_finished)
    {
        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}
