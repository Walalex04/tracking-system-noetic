#!/usr/bin/env python3

import rospy # Python library for ROS
from sensor_msgs.msg import Image # Image is the message type
from cv_bridge import CvBridge # Package to convert between ROS and OpenCV Images
import cv2 # OpenCV library


  
def publish_message():
 
  rospy.init_node('camera_node', anonymous=True)

  pub = rospy.Publisher('/video_frames', Image, queue_size=1)
  rate = rospy.Rate(30) 
  cap = cv2.VideoCapture(0)

  br = CvBridge()


  while not rospy.is_shutdown():

      ret, frame = cap.read()
         
      if ret == True:

        pub.publish(br.cv2_to_imgmsg(frame))
             
      rate.sleep()
         
if __name__ == '__main__':
  try:
    publish_message()
  except rospy.ROSInterruptException:
    pass