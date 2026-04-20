
# TRACKING SYSTEM

![ROS Noetic](https://img.shields.io/badge/ROS-Noetic-blue?logo=ros) ![Ubuntu](https://img.shields.io/badge/Ubuntu-20.04-orange) ![RViz](https://img.shields.io/badge/Tool-RViz-lightgrey)  
![Maintainer](https://img.shields.io/badge/Maintainer-Walter%20Gonzabay-green) ![Maintainer](https://img.shields.io/badge/Maintainer-Diego%20Ponton-green)


The system is designed to track Qupa robots. It is based on the work from [demiurge-tycho](https://github.com/demiurge-project/demiurge-tycho), which uses OpenCV as its core for image processing and ArUco code detection.It is written in `ROS Noetic`. We adapted the code to work with a general camera and modified the messages to make it compatible with the Qupa robot states.
## 1. 📦 Requirements and Dependencies

This project has been tested with the following setup:

- OS: Ubuntu 20.04 LTS  
- ROS version: ROS Noetic  
- Visualization: RViz  

For the installation of ROS, you can visit the official documentation:  [ROS Installation Guide](https://wiki.ros.org/noetic/Installation/Ubuntu)

### 1.1  Install Dependencies

### 1.1.1  ROS Packages

- roscpp  
- rospy  
- std_msgs  
- nav_msgs  
- tf2  
- tf2_ros  
- tf2_geometry_msgs  
- image_transport  
- cv_bridge  
- camera_info_manager  
- message_generation  
- rosserial  
- sensor_msgs  
- camera_calibration  

Install them using:

```bash
sudo apt update
sudo apt install ros-noetic-cv-bridge \
                 ros-noetic-image-transport \
                 ros-noetic-camera-info-manager \
                 ros-noetic-tf2 \
                 ros-noetic-tf2-ros \
                 ros-noetic-tf2-geometry-msgs \
                 ros-noetic-nav-msgs \
                 ros-noetic-sensor-msgs \
                 ros-noetic-rosserial
```
### 1.1.2 OpenCV and cv_bridge Requirements

The `cv_bridge` package is a core dependency of this project because it enables communication between ROS image messages and OpenCV.

To ensure proper functionality, the following additional libraries are required:

- OpenCV 3 or higher (required for image processing and ArUco detection)  
- python3-numpy (required by OpenCV Python bindings)  
- libboost-python-dev (required by the `cv_bridge` Python backend)  

Install them using:

```bash 
sudo apt install python3-numpy
sudo apt install libboost-python-dev
```

## 2. ⚙️ Building the Project


### 2.1 Clone Repository

``` bash
mkdir -p ~/catkin_ws/src
cd ~/catkin_ws/src
git clone https://github.com/Walalex04/tracking-system-noetic.git src
```

### 2.2 Install dependencies

``` bash
cd ~/catkin_ws
rosdep install --from-paths src --ignore-src -r -y
```

### 2.3 Build Project

``` bash
cd ~/catkin_ws
catkin_make
source devel/setup.bash
```


## 3. 📐 System Calibration

The calibration process for this tracking system involves several important steps to ensure accurate operation. First, the camera calibration is performed to correct lens distortion and ensure proper image processing. Additionally, the camera’s field of view may need to be cropped based on specific use cases. The next step is the calibration of the positions of the TAMs (Tracking and Alignment Modules) to ensure their alignment with the system’s global frame. Finally, the system configuration, which includes several general attributes of the system, is manually written into a YAML file located in the `launchers_pkf` package, inside the `conf` folder. The configuration file, `system_configuration.yaml`, contains key parameters that define the system’s behavior. Below is a description of the key parameters included in the configuration file.


#### 📝 Key Parameters in `system_configuration.yaml`

```yaml id="d5w6kp"
camera_width: 800             # Camera image width (pixels)
camera_height: 600            # Camera image height (pixels)
num_robots: 4                 # Number of robots in the system
ROS_MASTER_URI: "192.168.0.103"  # URI of the ROS Master
ROS_IP: "192.168.0.103"       # IP address of the current machine
port_comunication_dev: "/dev/ttyUSB0"  # Communication port for the device
com_baut: 115200             # Baud rate for serial communication
```

#### 📚 Explanation of Parameters
- **camera_width and camera_height:** Define the resolution of the camera image. These values ensure that the captured image is within the expected resolution for processing and analysis.

- **num_robots:** Specifies how many robots the system is set to track. This value is important for the tracking algorithm to manage multiple robots in the system.
- **ROS_MASTER_URI:** The URI of the ROS Master node. This allows the different ROS nodes to connect and communicate with the master node. The IP address should match the machine running the master node.
- **ROS_IP:** The IP address of the machine running the system. This is needed to ensure proper communication between different machines in the ROS network.
- **port_comunication_dev:** Defines the serial communication port that the system will use to communicate with external devices (e.g., robots or sensors). This must match the actual device port.
- **com_baut:** The baud rate used for serial communication. It must match the rate set on the connected device (e.g., robot or sensor).


### 3.1 📷 Camera Calibration

Camera calibration is crucial for correcting optical distortion and ensuring that the captured images are accurate for processing. In this project, we use the `camera_calibration` node from ROS to perform camera calibration.

#### 3.1.1 Run the Calibration Node
To calibrate the camera, run the following command in the terminal:

``` bash
roslaunch camera_calibration cameracalibrator.py --size 8x6 --square 0.025 image:=/camera/image_raw camera_info:=/camera/camera_info
```

- `--size 8x6`: Specifies the number of inner squares in the checkerboard pattern (adjust this depending on the pattern you are using).
- `--square 0.025`: Defines the size of each square in the checkerboard pattern in meters (adjust to the dimensions of your pattern).
- `image:=/camera/image_raw`: The camera image topic.
- `camera_info:=/camera/camera_info`: The camera info topic.

To generate the aruco launch this

``` bash
roslaunch track_sys creation_marker_py.launch
```
