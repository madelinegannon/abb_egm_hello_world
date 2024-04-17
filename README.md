# ABB EGM (Externally Guided Motion)
The respository contains setup instructions and a simple C++ and RAPID program for testing EGM communication between a PC and ABB industrial robot.
- [`egm_hello_world`](https://github.com/madelinegannon/abb_egm/tree/main/egm_hello_world): simple C++ program that connects and reads information off an ABB robot.
- [`egm_template`](https://github.com/madelinegannon/abb_egm/tree/main/egm_template): RAPID program that connects and runs `EGMJoint` moves.

### Tested On
- Windows 10/11
- RobotStudio 2024
- RobotWare 7

  
## Setup Instructions (Windows 10/11)
These setup instructions explain how to build and prepare a C++ application that uses ABB's real-time control interface Externally Guided Motion [3124-1].
> Skip this and double-click `egm_hello_world\egm_hello_world.sln` if you have Microsoft Visual Studio already installed.

### Step 1: Setup C++ Environment
1. Download [Microsoft Visual Studio (Community)](https://visualstudio.microsoft.com/downloads/)
   - When prompted, choose the **C++ desktop development** workload, which includes essential features like the MSVC compiler and C++ tools.
2. Download and install Git from [Git SCM](https://git-scm.com/download/win).
   - Add Git to your PATH during installation.

### Step 2: Install Google Protobuf using `vcpkg`
Install Microsoft's package manager `vcpkg`:
1. Clone `vcpkg` in your `C:` drive:
     ```bash
     cd C:\
     git clone https://github.com/microsoft/vcpkg
     ```
2. Run the bootstrap script:
     ```bash
     cd vcpkg
     .\bootstrap-vcpkg.bat
     ```
3. Integrate vcpkg system wide:
     ```bash
     .\vcpkg integrate install
     ```
4. Use `vcpkg` to install Google Protobuf:
     ```bash
     .\vcpkg install protobuf:x64-windows
     ```
### Step 3: Configure Visual Studio Project for Protobuf
1. Create a new C++ Desktop Application.
2. Configure the project's Include and Library Directories:
   - Right-click on the project in the Solution Explorer and select **Properties**.
   - Navigate to **VC++ Directories**.
   - Set **Include Directories** and **Library Directories** to include paths from your vcpkg installation, typically:
     ```
     [vcpkg root]\installed\x64-windows\include
     [vcpkg root]\installed\x64-windows\lib
     ```
   - In the **Linker** settings, under **Input**, add the necessary lib files (e.g., `libprotobuf.lib`).

### Step 4: Generate EGM Protobuf Files

![Reference Manual](https://github.com/madelinegannon/abb_egm_hello_world/blob/main/assets/egm_proto_file_location.png)

The correct `egm.proto` for your RobotWare OS file can be found through ABB RobotStudio.
1. Download & Install [RobotStudio](https://new.abb.com/products/robotics/robotstudio/downloads).
2. When prompted, download the correct controller and RobotWare for your robot (e.g., Omnicore & RobotWare 7.12.0).
3. Find the `egm.proto` through the RobotStudio **Add-Ins** tab:
    - Right-click on the installed RobotWare version in the Add-Ins browser.
    - Select Open Package Folder.
    - Navigate to *RobotPackages\RobotControl_<RW_Version>\utility\Template\EGM\egm.proto*
![](https://github.com/madelinegannon/abb_egm_hello_world/blob/main/assets/egm_proto_file_location.gif)   
4. Copy `egm.proto` to an easier location for protobuf:
    - Right-click File Explore and select **Open in Terminal**.
     ```bash
     cp egm.proto C:\vcpkg\packages\protobuf_x64-windows\tools\protobuf
     ```
5. Use protobuf's `protoc` tool to build the C++ source and header files:
      ```bash
      cd C:\vcpkg\packages\protobuf_x64-windows\tools\protobuf
      protoc --cpp_out=. egm.proto
      ```
6. This creates a `.pb.h` and `.pb.cc` files in that same location that you can copy & paste into your application folder.

## EGM Setup   
The included RAPID program is configured to work with the default EGM settings on a virtual controller. You will need to update these settings if you want to fine-tune your EGM parameters or work on a real robot.
![](https://github.com/madelinegannon/abb_egm_hello_world/blob/main/assets/egm_config_settings.gif)

| Motion | Communication| 
| :---------: | :------: |
| ![](https://github.com/madelinegannon/abb_egm_hello_world/blob/main/assets/egm_config_settings_motion.gif) | ![](https://github.com/madelinegannon/abb_egm_hello_world/blob/main/assets/egm_config_settings_communication.gif) |
