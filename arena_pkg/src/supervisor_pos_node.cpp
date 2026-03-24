#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <ros/master.h>
#include <map>
#include <string>
#include <boost/bind.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>

class SupervisorRobot
{
public:
    SupervisorRobot()
    {
        // Publicador de ejemplo (si lo quieres usar)
        // pub_status_ = node_handle_.advertise<std_msgs::UInt8MultiArray>("status", 10);
        std::vector<int> point;
        int i = 0;
        while (true)
        {
            std::string param_name = "/supervisor_pos_node/point_" + std::to_string(i);
            ROS_INFO("Param name: %s", param_name.c_str());
            if (!node_handle_.getParam(param_name, point))
            {
                ROS_ERROR("No se encontraron parametros para el taml");
                break;
            }

            points_tam.push_back(cv::Point(point.at(0), point.at(1)));
            i++;
        }
        discoverRobots();
    }

    // Callback de cada robot
    void supervisorCallback(const nav_msgs::Odometry::ConstPtr &msg, const std::string &robot_name)
    {
        double x = -1 * msg->pose.pose.position.x;
        double y = msg->pose.pose.position.y;

        robot_states[robot_name] = *msg; // Guardamos estado actual
        double ratio = 5;
        for (const auto &p : points_tam)
        {
            if ((x <= p.x + ratio && x >= p.x - ratio) && (y <= p.y + ratio && y >= p.y - ratio)) // ejemplo de zona
            {
                ROS_INFO("Robot %s llegó a la zona!", robot_name.c_str());
            }
        }

        // ROS_INFO("el robot %s con x: %f and y: %f", robot_name.c_str(), x, y);
    }

    // Detecta robots y crea subscribers dinámicamente (solo al inicio)
    void discoverRobots()
    {
        ros::master::V_TopicInfo master_topics;
        ros::master::getTopics(master_topics);

        for (auto &topic_info : master_topics)
        {
            std::string topic_name = topic_info.name;

            if (topic_name.find("odometry/filtered") != std::string::npos)
            {
                // Extraer nombre del robot del topic, ejemplo: /robot1/odometry/filtered
                std::size_t first_slash = topic_name.find('/');
                std::size_t second_slash = topic_name.find('/', first_slash + 1);
                std::string robot_name = topic_name.substr(first_slash + 1, second_slash - first_slash - 1);

                if (robot_subs.find(robot_name) == robot_subs.end())
                {
                    // Suscribirse usando boost::bind y pasar puntero this
                    robot_subs[robot_name] = node_handle_.subscribe<nav_msgs::Odometry>(
                        topic_name, 10, boost::bind(&SupervisorRobot::supervisorCallback, this, _1, robot_name));

                    ROS_INFO("Subscriber creado para %s (%s)", robot_name.c_str(), topic_name.c_str());
                }
            }
        }
    }

private:
    ros::Publisher pub_status_;
    ros::NodeHandle node_handle_;
    std::map<std::string, nav_msgs::Odometry> robot_states;
    std::map<std::string, ros::Subscriber> robot_subs;
    std::vector<cv::Point> points_tam;
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "SupervisorRobot");
    SupervisorRobot supervisor;
    ros::spin(); // solo ros::spin(), callbacks manejan todo
    return 0;
}