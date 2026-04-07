import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import ExecuteProcess

def generate_launch_description():
    # 1. Locate the URI of your package's share directory
    pkg_share = get_package_share_directory('stereo_concat')
    
    # 2. Define the path to your params.yaml
    # This assumes your YAML is in a folder named 'config' inside your package
    config_path = os.path.join(pkg_share, 'config', 'params.yaml')

    bag_path = os.path.join(pkg_share, 'onboarding.mcap')

    # 3. Create the Node action
    stereo_node = Node(
        package='stereo_concat',
        executable='stereo_concat',
        name='stereo_concat_subscriber',
        output='screen',
        parameters=[config_path]
    )

    bag_play = ExecuteProcess(
        cmd=['ros2', 'bag', 'play', bag_path, '--loop'],
        output='screen'
    )

    return LaunchDescription([
        stereo_node,
        bag_play
    ])