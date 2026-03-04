

import rospy
from sensor_msgs.msg import Image
from cv_bridge import CvBridge 
import cv2

from track_sys.tracking_system.Detector import Detector


detector = Detector()

def callback(data):
    br = CvBridge()
    #rospy.loginfo("receiving video frame")
    current_frame = br.imgmsg_to_cv2(data)

    current_frame = detector.detect_trail(current_frame)

    cv2.imshow("camera", current_frame)
    cv2.waitKey(1)


def receive_message():
    rospy.init_node('video_sub_py', anonymous=True)
    rospy.Subscriber('/video_frames', Image, callback)
    rospy.spin()
   




if __name__ == '__main__':
    receive_message()
    