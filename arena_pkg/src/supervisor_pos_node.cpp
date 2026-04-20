#include <ros/ros.h>
#include <nav_msgs/Odometry.h>
#include <ros/master.h>
#include <map>
#include <string>
#include <boost/bind.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <std_msgs/UInt8MultiArray.h>
#include <tracker_pkg/RobotInfo.h>

class SupervisorRobot
{
public:
    SupervisorRobot()
        : arena_status(18, 0), time_work(18, 0) // inicializar vectores
    {
        ROS_INFO("SUpervisor Robot has been initilized");
        // Publicador de arena status
        pubStatusArena_ = node_handle_.advertise<std_msgs::UInt8MultiArray>("arena_status", 10);

        // Leer puntos del parámetro
        std::vector<int> point;
        int i = 0;
        while (true)
        {
            std::string param_name = "/supervisor_pos_node/point_" + std::to_string(i);
            if (!node_handle_.getParam(param_name, point))
            {
                ROS_INFO("No se encontraron más parametros de puntos");
                break;
            }

            if (point.size() >= 2)
                points_tam.push_back(cv::Point(point[0], point[1]));

            i++;
        }

        // Detectar robots y crear suscriptores dinámicamente
        discoverRobots();
    }

    // Callback para cada robot
    void supervisorCallback(const tracker_pkg::RobotInfo::ConstPtr &msg, const std::string &robot_name)
    {
        // Copia del mensaje
        tracker_pkg::RobotInfo msg_copy = *msg;

        // Posición del robot
        double x = msg->odom.pose.pose.position.x;
        double y = msg->odom.pose.pose.position.y;

        // Tiempo del mensaje
        uint32_t time = msg->odom.header.stamp.sec;

        // Guardamos el estado del robot
        robot_states[robot_name] = msg->odom;

        double ratio = 0.02; // 1 = 1 m, 0.02 2 cm
        uint8_t index = 0;

        // Comprobar zona
        for (const auto &p : points_tam)
        {
            if ((x <= p.x + ratio && x >= p.x - ratio) && (y <= p.y + ratio && y >= p.y - ratio))
            {
                ROS_INFO("Robot %s llegó a la zona!", robot_name.c_str());
                pub_arena_status(index, time, msg->timework);
                msg_copy.robotstate = msg->WORKING;
            }
            else
            {
                msg_copy.robotstate = msg->RANDOMWALK;
            }
            index++;
        }

        // Publicar estado del arena
        std_msgs::UInt8MultiArray msgarray;
        msgarray.data = arena_status;
        pubStatusArena_.publish(msgarray);
    }
    void discoverRobots()
    {
        ros::master::V_TopicInfo master_topics;
        ros::master::getTopics(master_topics);

        for (auto &topic_info : master_topics)
        {
            std::string topic_name = topic_info.name;

            // Filtramos topics que sean de RobotInfo
            if (topic_info.datatype == "tracker_pkg/RobotInfo")
            {
                // Extraer nombre del robot, por ejemplo "/robot1/robot_info"
                std::size_t first_slash = topic_name.find('/');
                std::size_t second_slash = topic_name.find('/', first_slash + 1);
                std::string robot_name = topic_name.substr(first_slash + 1, second_slash - first_slash - 1);

                if (robot_subs.find(robot_name) == robot_subs.end())
                {
                    // Crear suscriptor dinámico
                    robot_subs[robot_name] = node_handle_.subscribe<tracker_pkg::RobotInfo>(
                        topic_name, 10, boost::bind(&SupervisorRobot::supervisorCallback, this, _1, robot_name));

                    ROS_INFO("Subscriber creado para %s (%s)", robot_name.c_str(), topic_name.c_str());
                }
            }
        }
    }

private:
    ros::NodeHandle node_handle_;
    ros::Publisher pubStatusArena_;
    std::map<std::string, nav_msgs::Odometry> robot_states;
    std::map<std::string, ros::Subscriber> robot_subs;
    std::vector<cv::Point> points_tam;
    std::vector<uint8_t> arena_status;
    std::vector<uint32_t> time_work;

    // Publicar estado de arena y manejar tiempos
    void pub_arena_status(uint8_t index, uint32_t secs, uint32_t timework)
    {
        if (time_work[index] == 0)
        {
            time_work[index] = secs;
            arena_status[index] = 1;
        }

        if (secs - time_work[index] >= timework)
            arena_status[index] = 0;
        else if (secs - time_work[index] >= timework + 2)
            time_work[index] = 0;
    }

    // Detecta robots y suscribe a sus topics de RobotInfo
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "SupervisorRobot");
    SupervisorRobot supervisor;

    ros::Rate rate(1); // 15 Hz

    while (ros::ok())
    {
        supervisor.discoverRobots(); // buscar nuevos robots

        ros::spinOnce(); // procesar callbacks

        rate.sleep();
    }

    return 0;
}