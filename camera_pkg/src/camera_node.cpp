#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <opencv2/opencv.hpp>
#include <camera_info_manager/camera_info_manager.h>

int main(int argc, char **argv)
{
    ros::init(argc, argv, "camera_node");
    ros::NodeHandle nh;
    ros::NodeHandle pnh("~");

    image_transport::ImageTransport it(nh);

    image_transport::Publisher image_pub = it.advertise("/image_raw", 1);
    ros::Publisher info_pub = nh.advertise<sensor_msgs::CameraInfo>("/camera_info", 1);

    std::string yaml_path;
    pnh.param<std::string>("camera_info_url", yaml_path, "");

    camera_info_manager::CameraInfoManager cinfo(nh, "camera");

    if (!yaml_path.empty())
    {
        if (!cinfo.loadCameraInfo(yaml_path))
        {
            ROS_WARN("No se pudo cargar el YAML de calibración");
        }
        else
        {
            ROS_INFO("YAML de calibración cargado");
        }
    }
    else
    {
        ROS_WARN("No se especificó camera_info_url");
    }

    cv::VideoCapture cap(2);

    int camera_width, camera_height;
    if (!nh.getParam("camera_width", camera_width))
    {
        ROS_ERROR("No se encontró el parámetro camera_width");
        return 1;
    }

    if (!nh.getParam("camera_height", camera_height))
    {
        ROS_ERROR("No se encontró el parámetro camera_height");
        return 1;
    }

    cap.set(cv::CAP_PROP_FRAME_WIDTH, camera_width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, camera_height);

    if (!cap.isOpened())
    {
        ROS_ERROR("No se pudo abrir la cámara");
        return -1;
    }

    ros::Rate loop_rate(15);

    while (ros::ok())
    {
        cv::Mat frame;
        cap >> frame;

        if (frame.empty())
            continue;

        std_msgs::Header header;
        header.stamp = ros::Time::now();
        header.frame_id = "camera";

        sensor_msgs::ImagePtr img_msg =
            cv_bridge::CvImage(header, "bgr8", frame).toImageMsg();

        sensor_msgs::CameraInfo cam_info = cinfo.getCameraInfo();
        cam_info.header = header;

        image_pub.publish(img_msg);
        info_pub.publish(cam_info);

        ros::spinOnce();
        loop_rate.sleep();
    }

    return 0;
}