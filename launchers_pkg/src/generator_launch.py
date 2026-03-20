import xml.etree.ElementTree as ET
import rospkg
from ruamel.yaml import YAML
import os

rospack = rospkg.RosPack()
ruta = rospack.get_path('launchers_pkg')
yaml = YAML()
name_file = "system_configuration.yaml"

path_config = os.path.join(ruta, "config")
path_file = os.path.join(path_config, name_file)

with open(path_file) as f:
    data = yaml.load(f)

print(data)



def create_launch_xml(filename=os.path.join(path_config, "system_tracker_gen.launch")):
    launch = ET.Element("launch")

    # Args
    ET.SubElement(launch, "arg", name="camera_width", default=str(data["camera_width"]))
    ET.SubElement(launch, "arg", name="camera_height", default=str(data["camera_height"]))

    # rosparam
    ET.SubElement(
        launch,
        "rosparam",
        file="$(find launchers_pkg)/config/system_configuration.yaml",
        command="load"
    )

    # Nodo tf
    ET.SubElement(
        launch,
        "node",
        pkg="tf2_ros",
        type="static_transform_publisher",
        name="center_camera_tf",
        args=".640 .360 0.0 0 0 0 odom center_camera"
    )

    # camera node
    camera_node = ET.SubElement(
        launch,
        "node",
        pkg="camera_pkg",
        type="camera_node",
        name="camera_node",
        output="screen"
    )
    ET.SubElement(
        camera_node,
        "param",
        name="camera_info_url",
        value="file://$(find camera_pkg)/config/camera_calibration_$(arg camera_width)x$(arg camera_height).yaml"
    )

    # image_proc
    ET.SubElement(
        launch,
        "node",
        pkg="image_proc",
        type="image_proc",
        name="image_proc",
        output="screen"
    )

    # cropper
    cropper = ET.SubElement(
        launch,
        "node",
        pkg="cropper_pkg",
        type="cropper_node",
        name="cropper_node",
        output="screen"
    )
    ET.SubElement(
        cropper,
        "rosparam",
        file="$(find cropper_pkg)/config/config_cropper.yaml"
    )

    # tracker nodes
    ET.SubElement(
        launch,
        "node",
        pkg="tracker_pkg",
        type="tracker_node",
        name="tracker_node",
        output="screen"
    )

    transformer = ET.SubElement(
        launch,
        "node",
        pkg="tracker_pkg",
        type="transformer_node",
        name="transformer_node",
        output="screen"
    )
    ET.SubElement(
        transformer,
        "rosparam",
        file="$(find tracker_pkg)/config/camera_transformer.yaml"
    )

    # Grupos ejemplo (arena)
    arena = ET.SubElement(launch, "group", ns="arena")
    ET.SubElement(
        arena,
        "param",
        name="robot_description",
        command="$(find xacro)/xacro $(find system_descriptor)/urdf/arena_octagon.urdf"
    )
    ET.SubElement(
        arena,
        "node",
        pkg="robot_state_publisher",
        type="robot_state_publisher",
        name="robot_state_publisher"
    )

    # Generar epuck groups dinámicamente (10–13 como ejemplo)
    for i in range(10, (10 + data["num_robots"])):
        group = ET.SubElement(launch, "group", ns=f"epuck_{i}")
        ET.SubElement(
            group,
            "param",
            name="robot_description",
            command=f"$(find xacro)/xacro $(find system_descriptor)/urdf/base.xacro id_robot:={i}"
        )
        ET.SubElement(
            group,
            "node",
            pkg="robot_state_publisher",
            type="robot_state_publisher",
            name="robot_state_publisher"
        )

        node = ET.SubElement(
            launch,
            "node",
            pkg="robot_localization",
            type="ukf_localization_node",
            name="ukf_se",
            clear_params="true",
            ns=f"epuck_{i}",
            output="screen"
        )

        ET.SubElement(
            node,
            "rosparam",
            command="load",
            file="$(find launchers_pkg)/config/ukf_tycho.yaml"
        )

        ET.SubElement(
            node,
            "param",
            name="base_link_frame",
            value=f"base_link_{i}"
        )

        ET.SubElement(
            node,
            "remap",
            **{"from": "camera/odom", "to": f"/epuck_{i}/odom"}
        )

    """
    # Generar ukf_localization (0–38)
    for i in range(39):
        node = ET.SubElement(
            launch,
            "node",
            pkg="robot_localization",
            type="ukf_localization_node",
            name="ukf_se",
            clear_params="true",
            ns=f"epuck_{i}",
            output="screen"
        )

        ET.SubElement(
            node,
            "rosparam",
            command="load",
            file="$(find launchers_pkg)/config/ukf_tycho.yaml"
        )

        ET.SubElement(
            node,
            "param",
            name="base_link_frame",
            value=f"base_link_{i}"
        )

        ET.SubElement(
            node,
            "remap",
            **{"from": "camera/odom", "to": f"/epuck_{i}/odom"}
        )

    """
    # rosserial
    rosserial = ET.SubElement(
        launch,
        "node",
        pkg="rosserial_python",
        type="serial_node.py",
        name="rosserial",
        output="screen"
    )
    ET.SubElement(rosserial, "param", name="port", value=data["port_comunication_dev"])
    ET.SubElement(rosserial, "param", name="baud", value=str(data["com_baut"]))


    tree = ET.ElementTree(launch)
    tree.write(filename, encoding="utf-8", xml_declaration=True)



create_launch_xml()

