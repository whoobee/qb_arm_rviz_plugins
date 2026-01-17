# qb_arm_rviz_plugins

This ROS 2 package provides custom RViz 2 plugins for the `qb_arm` project. It is designed to enhance the visualization and interaction experience within RViz, specifically for controlling the Gemini-based pick-and-place functionality.

## Features

### Trigger Gemini Panel
A custom GUI panel that allows users to trigger a ROS 2 service call directly from the RViz interface.

*   **Service Trigger:** A simple button to initiate the pick-and-place sequence.
*   **Configurable Service:** The service topic name can be edited directly in the panel (default: `/trigger_gemini_pick`).
*   **Status Feedback:** Logs information to the ROS 2 logger when the button is pressed.

## Dependencies

*   ROS 2 (Humble, Iron, or Rolling)
*   `rclcpp`
*   `rviz_common`
*   `rviz_rendering`
*   `std_srvs`
*   `Qt5` (Core, Gui, Widgets)

## Build Instructions

1.  **Clone the repository** (if not already in your workspace):
    ```bash
    cd ~/ros2_ws/src
    # clone command here if it were a git repo, otherwise ensure folders exist
    ```

2.  **Build the package**:
    ```bash
    cd ~/ros2_ws
    colcon build --packages-select qb_arm_rviz_plugins
    ```

3.  **Source the environment**:
    ```bash
    source install/setup.bash
    ```

## Usage

1.  **Start RViz 2**:
    ```bash
    rviz2
    ```

2.  **Add the Panel**:
    *   Go to the top menu: **Panels** -> **Add New Panel**.
    *   Scroll down to **qb_arm_rviz_plugins**.
    *   Select **TriggerGemini** and click **OK**.

3.  **Configuration**:
    *   The panel will appear in your RViz window.
    *   The **Service** text field displays the target service (default: `/trigger_gemini_pick`). You can change this to match your node's service name.
    *   Click the **Trigger** button to call the service.

## Troubleshooting

*   **Plugin not found:** Ensure you have sourced the workspace (`source install/setup.bash`) *before* running RViz.
*   **"The class required for this panel... could not be loaded":** This usually means the library wasn't found or there was an issue with the plugin registration. Try rebuilding cleanly:
    ```bash
    rm -rf build/qb_arm_rviz_plugins install/qb_arm_rviz_plugins
    colcon build --packages-select qb_arm_rviz_plugins
    ```