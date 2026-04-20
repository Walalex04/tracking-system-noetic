#include <ros/ros.h>
#include <std_msgs/UInt32MultiArray.h>
#include <nav_msgs/Odometry.h>
#include <tracker_pkg/RobotInfo.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <geometry_msgs/TransformStamped.h>
#include <cmath>
#include <map>
#include <sstream>

class Transformer
{
public:
	struct transformerData
	{
		std::map<int, ros::Publisher> info_publishers;

		double x_offset;
		double y_offset;
		double angular_offset;
		double scale_factor;
	} transformer;

	Transformer()
	{
		sub_tracker_ = node_handle_.subscribe(
			"tracker/positions_stamped", 20,
			&Transformer::transformerCallback, this);

		node_handle_.param("/transformer_node/x_offset", transformer.x_offset, 0.0);
		node_handle_.param("/transformer_node/y_offset", transformer.y_offset, 0.0);
		node_handle_.param("/transformer_node/angular_offset", transformer.angular_offset, 0.0);
		node_handle_.param("/transformer_node/scale_factor", transformer.scale_factor, 1.0);
	}

	void transformerCallback(const std_msgs::UInt32MultiArrayConstPtr &msg)
	{
		int size = msg->data.size();

		if (size > 2)
		{
			for (int i = 2; i < size; i += 9)
			{
				int robot_id = msg->data.at(i);

				if (robot_id < 0)
				{
					ROS_WARN("ID invalido: %d", robot_id);
					continue;
				}

				if (transformer.info_publishers.count(robot_id) == 0)
				{
					std::stringstream topic_info;
					topic_info << "/epuck_" << robot_id << "/odom";

					transformer.info_publishers[robot_id] =
						node_handle_.advertise<tracker_pkg::RobotInfo>(topic_info.str(), 10);

					ROS_INFO("Creado publisher INFO para robot %d", robot_id);
				}

				nav_msgs::Odometry robot_pose_msg;

				robot_pose_msg.header.frame_id = "camera_optical_frame";
				robot_pose_msg.header.stamp = ros::Time::now();

				std::stringstream child;
				child << "base_link_" << robot_id;
				robot_pose_msg.child_frame_id = child.str();

				std::vector<double> x(4), y(4);

				for (int j = 0; j < 4; ++j)
				{
					int x_tracker = -msg->data.at(2 * j + 1 + i);
					int y_tracker = msg->data.at(2 * j + 2 + i);

					double x_tmp = (x_tracker - transformer.x_offset) * transformer.scale_factor;
					double y_tmp = (y_tracker - transformer.y_offset) * transformer.scale_factor;

					x[j] = x_tmp * cos(-transformer.angular_offset) - y_tmp * sin(-transformer.angular_offset);
					y[j] = x_tmp * sin(-transformer.angular_offset) + y_tmp * cos(-transformer.angular_offset);
				}

				double pose_x = 0.0, pose_y = 0.0;
				for (int j = 0; j < 4; ++j)
				{
					pose_x += x[j] / 4.0;
					pose_y += y[j] / 4.0;
				}

				robot_pose_msg.pose.pose.position.x = pose_x;
				robot_pose_msg.pose.pose.position.y = pose_y;
				robot_pose_msg.pose.pose.position.z = 0.0;

				double dx = x[1] - x[0];
				double dy = y[1] - y[0];

				if (std::fabs(dx) < 1e-6 && std::fabs(dy) < 1e-6)
				{
					ROS_WARN("Orientacion invalida en ID %d", robot_id);
					continue;
				}

				double yaw = atan2(dy, dx);

				tf2::Quaternion q;
				q.setRPY(0, 0, yaw);
				tf2::convert(q, robot_pose_msg.pose.pose.orientation);

				// 🔹 TF (esto sí lo dejas)
				geometry_msgs::TransformStamped t;
				t.header.stamp = robot_pose_msg.header.stamp;
				t.header.frame_id = "camera_optical_frame";
				t.child_frame_id = robot_pose_msg.child_frame_id;

				t.transform.translation.x = pose_x;
				t.transform.translation.y = pose_y;
				t.transform.translation.z = 0.0;
				t.transform.rotation = robot_pose_msg.pose.pose.orientation;

				br_.sendTransform(t);

				tracker_pkg::RobotInfo robot_info_msg;

				robot_info_msg.odom = robot_pose_msg;

				/*TO DO:
					subscriber the correct topic to update this parameters
				*/
				robot_info_msg.robotstate = tracker_pkg::RobotInfo::RANDOMWALK;
				robot_info_msg.typework = tracker_pkg::RobotInfo::NON_SPECIALIST;
				robot_info_msg.timework = 2;

				transformer.info_publishers[robot_id]
					.publish(robot_info_msg);
			}
		}
	}

private:
	ros::NodeHandle node_handle_;
	ros::Subscriber sub_tracker_;
	tf2_ros::TransformBroadcaster br_;
};

int main(int argc, char **argv)
{
	ros::init(argc, argv, "transformer_node");
	Transformer transformer;
	ros::spin();
	return 0;
}