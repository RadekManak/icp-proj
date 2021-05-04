// file: trafficSimulator.cpp
// specification: ICP project 2021 variant 1

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <atomic>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "mqtt/async_client.h"

//THREAD COMMAND PASSING
std::atomic <int> cmd_valve = {-1};
std::atomic <int> cmd_ts = {-99};

//THREAD SAFE QUEUE
class SafeQueue{
public:
	SafeQueue(void)
		: q()
		, m()
		, c()
	{}

	~SafeQueue(void)
	{}

	//Add an element to end of queue
	void enqueue(mqtt::message_ptr t){
		std::lock_guard<std::mutex> lock(m);
		q.push(t);
		c.notify_one();
	}

	//Get the first element in queue
	//If queue is empty wait until a element is avaiable
	mqtt::message_ptr dequeue(void){
		std::unique_lock<std::mutex> lock(m);
		while(q.empty()){	//release lock as long as the wait and reaquire it afterwards
			c.wait(lock);
		}
		mqtt::message_ptr val = q.front();
		q.pop();
		return val;
	}

private:
	std::queue<mqtt::message_ptr> q;
	mutable std::mutex m;
	std::condition_variable c;
};

//CALLBACK
class callback : public virtual mqtt::callback{
	void message_arrived(mqtt::const_message_ptr msg) override{
		std::string topic = msg->get_topic();

		std::cout << "Message arrived" << std::endl;
		std::cout << "\tTopic: " << topic << std::endl;
		std::cout << "\tPayload: " << msg->to_string() << std::endl;

		if(topic == "valve/cmd"){
			if(msg->to_string() == "open"){
				std::cout << "Valve received open command" << std::endl;
				cmd_valve = {1};
			}
			else if(msg->to_string() == "close"){
				std::cout << "Valve received close command" << std::endl;
				cmd_valve = {0};
			}
			else std::cout << "Valve received invalid command" << std::endl;
		}

		else if(topic == "thermostat/cmd"){
			std::stringstream stream;
			stream << msg->to_string();
			std::string cmd = stream.str();
			if(cmd.substr(0, 4) == "set "){
				try{
					int val = stoi(cmd.substr(4, cmd.length() - 4));
					std::cout << "Thermostat received set command" << std::endl;
					cmd_ts = {val};
				}
				catch(std::invalid_argument){
					std::cout << "Thermostat received invalid set value" << std::endl;
				}
			}
			else std::cout << "Thermostat received invalid command" << std::endl;
		}
	}

	void delivery_complete(mqtt::delivery_token_ptr tok) override{
		std::cout << "Delivered token: " << tok->get_message_id() << std::endl;
	}
};


//////////////////////////////////////////   SENSORS   //////////////////////////////////////////
void intsensor(SafeQueue * Q, const char* topic, const int min, const int max, const int period){
	srand(time(0));
	int range;
	if(min < 0 && max >= 0) range = max - min;
	else if(min < 0 && max < 0) range = -1*(max - min);
	else range = max - min;

	int step = range / 50;	//defines value by which the result changes each iteration 
	step = (step > 0)? step : 1;

	int value = std::rand() % (range + 1) + min;	//set initial value in range

	mqtt::message_ptr msg;
	while(true){
		if(rand() % 2){
			if(value >= max) value -= step;	//stay in range
			else value += step;
		}
		else{
			if(value <= min) value += step;	//stay in range
			else value -= step;
		}
		msg = mqtt::make_message(topic, std::to_string(value));
		Q->enqueue(msg);
		std::this_thread::sleep_for(std::chrono::milliseconds(period));
	}
}

void floatsensor(SafeQueue * Q, const char* topic, const float min, const float max, const int period){
	srand(time(0));
	float range;
	if(min < 0 && max >= 0) range = max - min;
	else if(min < 0 && max < 0) range = -1*(max - min);
	else range = max - min;

	float step = range / 50;	//defines value by which the result changes each iteration
	step = ((float )((int)(step * 10))) / 10;	//round to one decimal
	step = (step > 0.1)? step : 0.1;

	float value = min + static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(range)));
	value = ((float )((int)(value * 10))) / 10;	//set initial value in range

	std::stringstream stream;
	mqtt::message_ptr msg;
	while(true){
		if(rand() % 2){
			if(value >= max) value -= step;	//stay in range
				value += step;
		}
		else{
			if(value <= min) value += step;	//stay in range
			else value -= step;
		}

		stream << std::fixed << std::setprecision(1) << value;	//format
		msg = mqtt::make_message(topic, stream.str());
		Q->enqueue(msg);
		stream.str(std::string());	//empty stream
		std::this_thread::sleep_for(std::chrono::milliseconds(period));
	}
}

void door_switch(SafeQueue * Q, const char* topic, const int period_min, const int period_max){
	mqtt::message_ptr msg = mqtt::make_message(topic, "closed");
	bool opened = false;	//state of door switch
	Q->enqueue(msg);	//publish initial state

	int cmd;
	while(true){
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % (period_max - period_min + 1) + period_min));
		if(opened){
			msg = mqtt::make_message(topic, "closed");
			opened = false;
		}
		else{
			msg = mqtt::make_message(topic, "opened");
			opened = true;
		}
		Q->enqueue(msg);
	}
}

void valve(SafeQueue * Q, const char* topic){
	mqtt::message_ptr msg = mqtt::make_message(topic, "closed");
	Q->enqueue(msg);	//publish initial state

	int cmd;
	while(true){
		while(true){
			cmd = cmd_valve.load();
			if(cmd == 1){
				msg = mqtt::make_message(topic, "opened");
				cmd_valve = {-1};
				break;
			}
			else if(cmd == 0){
				msg = mqtt::make_message(topic, "closed");
				cmd_valve = {-1};
				break;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));	//check for comand cooldown
		}
		Q->enqueue(msg);
	}
}

void thermostat(SafeQueue * Q, const char* topic, const int min, const int max, const int period){
	srand(time(0));
	int range;
	if(min < 0 && max >= 0) range = max - min;
	else if(min < 0 && max < 0) range = -1*(max - min);
	else range = max - min;

	int cmd = std::rand() % (range + 1) + min;	//set initial value in range
	int value = cmd;	//state of thermostat
	mqtt::message_ptr msg = mqtt::make_message(topic, std::to_string(cmd));
	Q->enqueue(msg);

	while(true){
		cmd = cmd_ts.load();
		if(cmd != -99){
			while(value != cmd){
				if(value < cmd) value++;
				else value--;
				msg = mqtt::make_message(topic, std::to_string(value));
				cmd_ts = {-99};
				Q->enqueue(msg);
				std::this_thread::sleep_for(std::chrono::milliseconds(period));
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));	//check for comand cooldown
	}
}


//////////////////////////////////////////   MAIN   //////////////////////////////////////////
int main(){
	int QOS, SERVER_PORT, THERM_MIN, THERM_MAX, THERM_PER, HYGRO_MIN, HYGRO_MAX, HYGRO_PER, WATT_MIN, WATT_MAX, WATT_PER, TS_MIN, TS_MAX, TS_PER, DS_PER_MIN, DS_PER_MAX;
	float PIR_MIN, PIR_MAX, PIR_PER, I_MIN, I_MAX, I_PER, Q_MIN, Q_MAX, Q_PER;
	std::string SERVER_ADDRESS, CLIENT_ID;

	//Load configuration
	std::ifstream file ("traffic.cfg");
	if(file.is_open()){
		std::string line;
		while(getline(file, line)){
			line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
			if(line[0] == '#' || line.empty()) continue;
			auto delimiter = line.find("=");
			auto name = line.substr(0, delimiter);
			auto value = line.substr(delimiter + 1);
			try{
				if(!name.compare("SERVER_ADDRESS")) SERVER_ADDRESS = value;
				else if(!name.compare("CLIENT_ID")) CLIENT_ID = value;
				else if(!name.compare("QOS")) QOS = stoi(value);
				else if(!name.compare("SERVER_PORT")) SERVER_PORT = stoi(value);
				else if(!name.compare("THERM_MIN")) THERM_MIN = stoi(value);
				else if(!name.compare("THERM_MAX")) THERM_MAX = stoi(value);
				else if(!name.compare("THERM_PER")) THERM_PER = stoi(value);
				else if(!name.compare("HYGRO_MIN")) HYGRO_MIN = stoi(value);
				else if(!name.compare("HYGRO_MAX")) HYGRO_MAX = stoi(value);
				else if(!name.compare("HYGRO_PER")) HYGRO_PER = stoi(value);
				else if(!name.compare("WATT_MIN")) WATT_MIN = stoi(value);
				else if(!name.compare("WATT_MAX")) WATT_MAX = stoi(value);
				else if(!name.compare("WATT_PER")) WATT_PER = stoi(value);
				else if(!name.compare("TS_MIN")) TS_MIN = stoi(value);
				else if(!name.compare("TS_MAX")) TS_MAX = stoi(value);
				else if(!name.compare("TS_PER")) TS_PER = stoi(value);
				else if(!name.compare("DS_PER_MIN")) DS_PER_MIN = stoi(value);
				else if(!name.compare("DS_PER_MAX")) DS_PER_MAX = stoi(value);
				else if(!name.compare("PIR_MIN")) PIR_MIN = stof(value);
				else if(!name.compare("PIR_MAX")) PIR_MAX = stof(value);
				else if(!name.compare("PIR_PER")) PIR_PER = stof(value);
				else if(!name.compare("I_MIN")) I_MIN = stof(value);
				else if(!name.compare("I_MAX")) I_MAX = stof(value);
				else if(!name.compare("I_PER")) I_PER = stof(value);
				else if(!name.compare("Q_MIN")) Q_MIN = stof(value);
				else if(!name.compare("Q_MAX")) Q_MAX = stof(value);
				else if(!name.compare("Q_PER")) Q_PER = stof(value);
				else{
					std::cerr << "ERROR: Unrecognized option " << name << " in configuration file.";
					return 1;
				}
			}
			catch(const std::invalid_argument&){
				std::cerr << "ERROR: Invalid value of " << name << " in configuration file.";
				return 1;
			}
			catch(const std::out_of_range&){
				std::cerr << "ERROR: Invalid value of " << name << " in configuration file.";
				return 1;
			}
		}
	}
	else std::cerr << "ERROR: Could not open configuration file.\n";

	SafeQueue Q;

	std::thread therm(intsensor, &Q, "thermometer", THERM_MIN, THERM_MAX, THERM_PER);
	std::thread hygro(intsensor, &Q, "hygrometer", HYGRO_MIN, HYGRO_MAX, HYGRO_PER);
	std::thread watt(intsensor, &Q, "wattmeter", WATT_MIN, WATT_MAX, WATT_PER);

	std::thread pir(floatsensor, &Q, "PIR-sensor", PIR_MIN, PIR_MAX, PIR_PER);
	std::thread iradar(floatsensor, &Q, "radar/in-phase", I_MIN, I_MAX, I_PER);
	std::thread qradar(floatsensor, &Q, "radar/quadrature", Q_MIN, Q_MAX, Q_PER);

	std::thread ds(door_switch, &Q, "door-switch", DS_PER_MIN, DS_PER_MAX);
	std::thread valv(valve, &Q, "valve/state");

	std::thread ts(thermostat, &Q, "thermostat/temp", TS_MIN, TS_MAX, TS_PER);

	mqtt::async_client client(SERVER_ADDRESS+":"+std::to_string(SERVER_PORT), CLIENT_ID);
	auto connOpts = mqtt::connect_options_builder()
		.clean_session()
		.finalize();
	callback cb;
	client.set_callback(cb);

	mqtt::message_ptr msg;
	try{
		mqtt::token_ptr conntok = client.connect(connOpts);
		conntok->wait();

		client.subscribe("valve/cmd", QOS);
		client.subscribe("thermostat/cmd", QOS);

		while(true){
			msg = Q.dequeue();
			msg->set_qos(QOS);
			client.publish(msg);
		}
	}
	catch(const mqtt::exception& exc){
		std::cerr << exc.what() << std::endl;
		return 1;
	}

	return 0;
}
