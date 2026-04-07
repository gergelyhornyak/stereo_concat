import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # 1. Locate the URI of your package's share directory
    pkg_share = get_package_share_directory('stereo_concat')
    
    # 2. Define the path to your params.yaml
    # This assumes your YAML is in a folder named 'config' inside your package
    config_path = os.path.join(pkg_share, 'config', 'params.yaml')

    # 3. Create the Node action
    stereo_node = Node(
        package='stereo_concat',
        executable='stereo_concat',
        name='stereo_concat_subscriber', # Must match the top-level key in your YAML
        output='screen',
        parameters=[config_path]
    )

    return LaunchDescription([
        stereo_node
    ])