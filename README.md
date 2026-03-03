
# TRACKING SYSTEM


making the program structure (~ it mean relative path, use only /catkin_ws into desired file)

```
mkdir -p ~/catkin_ws/src
cd ~/catkin_ws/src
git clone https://github.com/Walalex04/tracking-system-noetic.git
```


install dependences 

```
cd ~/catkin_ws
rosdep install --from-paths src --ignore-src -r -y
```


build and execute 

```
cd ~/catkin_ws
catkin_make
. devel/setup.bash
roslaunch track-sys track-sys_py.launch
```