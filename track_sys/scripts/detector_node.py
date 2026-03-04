

import rospy
import cv2
from cv_bridge import CvBridge
from sensor_msgs.msg import Image
from std_msgs.msg import String
import json

from track_sys.tracking_system.Detector import Detector



class Detector_node():

    """

    """

    def __init__(self):
        rospy.init_node("dectector_node", anonymous=True)

        self.detector = Detector()


        self.br = CvBridge()
        self.image_actual = []
        
        self.rate = rospy.Rate(10)

        self.pub = rospy.Publisher(
            "/pos_robots",
            String, 
            queue_size=10
        )

        self.sub = rospy.Subscriber(
            "/video_frames",
            Image,
            self.callback
        )


        self.pub_trail_image = rospy.Publisher(
            "/video_procesed",
            Image,
            queue_size=10
        )

        rospy.spin()
        rospy.loginfo("node detector initilized")

    def callback(self, msg):
        current_frame = self.br.imgmsg_to_cv2(msg)
        data_robots = self.detector.detect_position_robots(current_frame)
        self.image_actual = current_frame
        msg = String()
        msg.data = json.dumps(data_robots)
        self.pub.publish(msg)



    

def main():
    Detector_node()

if __name__ == "__main__":
    main()
