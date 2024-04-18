///	EGM Server
/// 
/// Madeline Gannon | ATONATON
/// Revised 04.17.2024
/// 
/// Connects to EGM-compatible ABB robots.
/// Receives Joint Positions as JSON message over UDP from an external application.
/// Updates and sends as desired EGM Joint Positions to connected ABB robots. 
/// 
/// Adapted from code by:
/**
*********************************************************************
	Author        : Mohith Sakthivel
	Date          : 04.06.2019
	Description   : EGM_Build
					Windows console application to implement
					EGM control Robot and Track Setup
*********************************************************************
**/

#include <thread>
#include "egm_control.h"
#include <json/json.h>

#define T_ROB_1
#define T_ROB_2

using namespace std;

SOCKET sock;
bool receive_external = true;
float position_targets_rob_1[6] = { 90,0,0,0,0,0 };
float position_targets_rob_2[6] = { 0,0,0,0,0,0 };
float position_targets_rob_3[6] = { 0,0,0,0,0,0 };
float position_targets_rob_4[6] = { 0,0,0,0,0,0 };

Json::Value value;

#ifdef T_ROB_1
const std::string ROB1_Name = "Robot 1";
const int ROB1_PORT = 6510;
#endif // T_ROB_1

#ifdef T_ROB_2
const std::string ROB2_Name = "Robot 2";
const int ROB2_PORT = 6511;
#endif // T_ROB_2

bool IsReadable = true;

// Input file parameters
int msg_count = 1482;
int sampling_time = 4;

std::chrono::time_point<std::chrono::steady_clock> StartTime, EndTime, CycleStartTime, PrevCycleStartTime;

float time_data[10] = { 0,0,0,0,0,0,0,0,0,0 };

inline void update_val(Robot* robot, float position_targets[6]) {
	robot->RobotJoint[0] = position_targets[0];
	robot->RobotJoint[1] = position_targets[1];
	robot->RobotJoint[2] = position_targets[2];
	robot->RobotJoint[3] = position_targets[3];
	robot->RobotJoint[4] = position_targets[4];
	robot->RobotJoint[5] = position_targets[5];
}

inline void initUDPReceiver(int port) {
	// create socket to listen from Robot
	sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	struct sockaddr_in serverAddr;
	memset(&serverAddr, sizeof(serverAddr), 0);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);
	// listen on all interfaces
	::bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));

	std::cout << "UDP Receiver setup on port (" << port << ")." << std::endl;
}

inline void receive_external_data() {
	sockaddr_in Addr;
	int len_receive = sizeof(sockaddr);
	char buffer[1400];
	while (receive_external) {
		// receive message from external apps
		int n = recvfrom(sock, buffer, 1400, 0, (struct sockaddr*)&Addr, &len_receive);
		if (n < 0)
		{
			printf("Error receiving message\n");
		}
		else {
			// parse data
			Json::Reader reader;
			Json::Value value;

			if (reader.parse(buffer, value)) {
				//cout << value << endl;
				int i = 0;
#ifdef T_ROB_1
				i = 0;
				auto position_targets_ABB_0 = value["/World/ABB_0"];
				for (auto val : position_targets_ABB_0) {
					position_targets_rob_1[i] = val.asFloat();
					i++;
					//cout << val << ", ";
				}
				//cout << endl;
#endif // T_ROB_1
#ifdef T_ROB_2
				i = 0;
				auto position_targets_ABB_1 = value["/World/ABB_1"];
				for (auto val : position_targets_ABB_1) {
					position_targets_rob_2[i] = val.asFloat();
					i++;
					//cout << val << ", ";
				}
				//cout << endl;
#endif // T_ROB_2
			}
			else {
				cout << reader.getFormattedErrorMessages();
			}
		}
	}
}


int main(int argc, char* argv[]) {
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	initWinSock();

	initUDPReceiver(11999);
	std::thread UDPReceiveThread(&receive_external_data);

#ifdef T_ROB_1
	std::cout << "Initializing Robot Threads on PORT: " << ROB1_PORT << std::endl;
	Robot* Rob1 = new  Robot(ROB1_Name, ROB1_PORT);
	std::thread Robot1InitThread(&Robot::initRobot, Rob1);
	Robot1InitThread.join();
#endif // T_ROB_1

#ifdef T_ROB_2
	Robot* Rob2 = new  Robot(ROB2_Name, ROB2_PORT);
	std::thread Robot2InitThread(&Robot::initRobot, Rob2);
	Robot2InitThread.join();
#endif // T_ROB_2

	std::cout << "Done Initializing Robot Threads." << std::endl;

	PrevCycleStartTime = CycleStartTime = StartTime = std::chrono::steady_clock::now();

	int i = 0;
	while (true) {
		CycleStartTime = std::chrono::steady_clock::now();
		time_data[0] = float(std::chrono::duration_cast<std::chrono::nanoseconds>(CycleStartTime - PrevCycleStartTime).count() / 1e6);
		PrevCycleStartTime = CycleStartTime;

#ifdef T_ROB_1
		update_val(Rob1, position_targets_rob_1);
#endif // T_ROB_1

#ifdef T_ROB_2
		update_val(Rob2, position_targets_rob_2);
#endif // T_ROB_2

		// Send data through Protobuf
#ifdef T_ROB_1
		std::thread WriteRobot1([](Robot* Rob1, float* TimeData) {Rob1->WriteCycleJoint(TimeData); }, Rob1, &time_data[2]);
#endif // T_ROB_1

#ifdef T_ROB_2
		std::thread WriteRobot2([](Robot* Rob2, float* TimeData) {Rob2->WriteCycleJoint(TimeData); }, Rob2, &time_data[2]);
#endif // T_ROB_2

		// wait for read and write threads to join
#ifdef T_ROB_1
		WriteRobot1.join();
#endif // T_ROB_1		

#ifdef T_ROB_2
		WriteRobot2.join();
#endif // T_ROB_2
		i++;
	}
	receive_external = false;
	UDPReceiveThread.join();

	EndTime = std::chrono::steady_clock::now();
	std::cout << std::endl << "Expected Runtime:" << ((msg_count * sampling_time) / 1000.0) << std::endl <<
		"Execution Time:" << std::chrono::duration_cast<std::chrono::microseconds>(EndTime - StartTime).count() / 1e6 << " seconds" << std::endl;

#ifdef T_ROB_1
	delete Rob1;
#endif // T_ROB_1	

#ifdef T_ROB_2
	delete Rob2;
#endif // T_ROB_2
	exit(0);
	return 0;
}