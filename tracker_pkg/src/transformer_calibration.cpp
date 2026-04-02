#include <ros/ros.h>
#include <ros/package.h>
#include <std_msgs/UInt32MultiArray.h>
#include <std_msgs/Bool.h>
#include <iostream>
#include <fstream>
#include <cmath>

class TransformerCalibrator
{
public:
	TransformerCalibrator()
		: done(false)
	{
		sub_tracker_ = node_handle_.subscribe("tracker/positions_stamped", 10,
											  &TransformerCalibrator::calibratorCallback, this);
		pub_alive_ = node_handle_.advertise<std_msgs::Bool>("alive", 10);

		std::string path = ros::package::getPath("tracker_pkg") + "/config";
		std::string ros_namespace = ros::this_node::getNamespace();
		std::string filename = path + ros_namespace + "_transformer.yaml";
		yaml_.open(filename, std::fstream::out | std::fstream::trunc);
		if (yaml_.is_open())
		{
			ROS_INFO("Writing configuration to %s", filename.c_str());
		}
		else
		{
			ROS_ERROR("Could not open %s", filename.c_str());
		}
	}

	~TransformerCalibrator()
	{
		yaml_.close();
	}

	struct point
	{
		double x;
		double y;
	};

	bool done;

	// NUEVOS TAGS
	const int center_tag = 13;
	const int tag_tr = 14;
	const int tag_tl = 17;
	const int tag_bl = 18;
	const int tag_br = 19;

	void calibratorCallback(const std_msgs::UInt32MultiArrayConstPtr &msg)
	{
		int size = msg->data.size();

		// 🔥 CORREGIDO (antes era == 83)
		if (size >= 47)
		{
			std::map<int, struct point> tags;

			for (int i = 2; i < size; i += 9)
			{
				struct point tag_centre = {0.0};
				for (int j = 0; j < 4; ++j)
				{
					tag_centre.x -= static_cast<double>(msg->data.at(2 * j + 1 + i)) / 4.0;
					tag_centre.y += static_cast<double>(msg->data.at(2 * j + 2 + i)) / 4.0;
				}

				tags[msg->data.at(i)] = tag_centre;
			}

			// Validación mínima
			if (!(tags.count(center_tag) && tags.count(tag_tr) && tags.count(tag_tl) && tags.count(tag_bl)))
				return;

			// --- CALCULO ---
			double angular_offset = 0.0;
			double scale_factor = 0.0;

			// PAR 1: Horizontal (14 ↔ 17)
			{
				double x1 = tags[tag_tr].x;
				double y1 = tags[tag_tr].y;
				double x2 = tags[tag_tl].x;
				double y2 = tags[tag_tl].y;

				double orientation = std::atan2(y2 - y1, x2 - x1);
				angular_offset += (orientation - 0.0) / 2.0;

				double size = std::sqrt(std::pow(y2 - y1, 2) + std::pow(x2 - x1, 2));
				scale_factor += (1.75 / size) / 2.0;
			}

			// PAR 2: Vertical (17 ↔ 18)
			{
				double x1 = tags[tag_tl].x;
				double y1 = tags[tag_tl].y;
				double x2 = tags[tag_bl].x;
				double y2 = tags[tag_bl].y;

				double orientation = std::atan2(y2 - y1, x2 - x1);
				angular_offset += (orientation - M_PI_2) / 2.0;

				double size = std::sqrt(std::pow(y2 - y1, 2) + std::pow(x2 - x1, 2));
				scale_factor += (1.37 / size) / 2.0;
			}

			// Guardar YAML
			yaml_ << "x_offset : " << tags[center_tag].x << "\n";
			yaml_ << "y_offset : " << tags[center_tag].y << "\n";
			yaml_ << "angular_offset : " << angular_offset << "\n";
			yaml_ << "scale_factor : " << scale_factor << "\n";

			done = true;
		}

		std_msgs::Bool alive_msg;
		alive_msg.data = !done;
		pub_alive_.publish(alive_msg);
	}

private:
	ros::NodeHandle node_handle_;
	ros::Subscriber sub_tracker_;
	ros::Publisher pub_alive_;
	std::ofstream yaml_;
};

int main(int argc, char **argv)
{
	ros::init(argc, argv, "calibrator");
	TransformerCalibrator calibrator_object;

	ros::Rate rate(30);
	while (ros::ok() && !calibrator_object.done)
	{
		ros::spinOnce();
		rate.sleep();
	}

	return 0;
}