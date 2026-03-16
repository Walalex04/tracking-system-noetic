#include <ros/ros.h>
#include <std_msgs/UInt8MultiArray.h>

int main(int argc, char** argv) {
    ros::init(argc, argv, "test_sender_cpp");
    ros::NodeHandle nh;

    // Publicador al topic que el Arduino escucha
    ros::Publisher pub = nh.advertise<std_msgs::UInt8MultiArray>("/test_array", 1);

    ros::Rate rate(15); // 2 Hz, seguro para Arduino

    while (ros::ok()) {
        std_msgs::UInt8MultiArray msg;

        // Array pequeño por defecto (8 elementos)
        msg.data = {1, 0, 0, 0, 0, 0, 0, 2, 10, 20, 30, 40, 50, 60, 70, 80, 10, 20, 30, 40, 50, 60, 70, 80};

        pub.publish(msg);
        ROS_INFO_STREAM("Mensaje enviado al Arduino con 8 elementos.");

        ros::spinOnce();
        rate.sleep();
    }

    return 0;
}