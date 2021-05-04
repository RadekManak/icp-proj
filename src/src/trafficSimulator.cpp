// file: test.cpp
// specification: ICP project 2021 variant 1

#include <string>
#include <sstream>
#include <iomanip>
#include <atomic>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "mqtt/async_client.h"

//CLIENT
const std::string SERVER_ADDRESS { "localhost:1883" };
const std::string CLIENT_ID { "test" };
const int  QOS = 1;

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
/////// thermometer
const char* THERM_NAME = "thermometer";
const int THERM_MIN = -20;  //celsius
const int THERM_MAX = 35;

/////// hygrometer
const char* HYGRO_NAME = "hygrometer";
const int HYGRO_MIN = 35; //percent water
const int HYGRO_MAX = 65;

/////// wattmeter
const char* WATT_NAME = "wattmeter";
const int WATT_MIN = 0; //watts
const int WATT_MAX = 800;

/////// PIR sensor
const char* PIR_NAME = "PIR sensor";
const float PIR_MIN = 2.2;  //volts
const float PIR_MAX = 5.1;

/////// radar
const char* I_RADAR_NAME = "radar/In-phase";
const char* Q_RADAR_NAME = "radar/Quadrature";
const float I_MIN = 1.5;  //volts
const float I_MAX = 3.5;
const float Q_MIN = 1.5;  //volts
const float Q_MAX = 3.5;

/////// door switch
const char* DS_NAME = "door switch";

/////// valve
const char* VALVE_NAME = "valve/state";

/////// thermostat
const char* TS_NAME = "thermostat/temp";
const int TS_MIN = -20; //celsius
const int TS_MAX = 35;

void intsensor(SafeQueue * Q, const char* topic, const int min, const int max){
	srand(time(0));
	int range;
	if(min < 0 && max >= 0) range = max - min;
	else if(min < 0 && max < 0) range = -1*(max - min);
	else range = max - min;
  
	int step = range / 50; //defines value by which the result changes each iteration 
	step = (step > 0)? step : 1;

	int value = std::rand() % (range + 1) + min;  //set initial value in range
  
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
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void floatsensor(SafeQueue * Q, const char* topic, const float min, const float max){
	srand(time(0));
	float range;
	if(min < 0 && max >= 0) range = max - min;
	else if(min < 0 && max < 0) range = -1*(max - min);
	else range = max - min;

	float step = range / 50; //defines value by which the result changes each iteration
	step = ((float )((int)(step * 10))) / 10; //round to one decimal
	step = (step > 0.1)? step : 0.1;

	float value = min + static_cast <float> (rand()) / ( static_cast <float> (RAND_MAX/(range)));
	value = ((float )((int)(value * 10))) / 10; //set initial value in range

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

		stream << std::fixed << std::setprecision(1) << value;  //format
		msg = mqtt::make_message(topic, stream.str());
		Q->enqueue(msg);
		stream.str(std::string());  //empty stream
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void door_switch(SafeQueue * Q, const char* topic){
	mqtt::message_ptr msg = mqtt::make_message(topic, "closed");
	bool opened = false;	//state of door switch
	Q->enqueue(msg);  //publish initial state

	int cmd;
	while(true){
		std::this_thread::sleep_for(std::chrono::milliseconds(rand() % 10000 + 2000));
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
	Q->enqueue(msg);  //publish initial state

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
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
		Q->enqueue(msg);
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void thermostat(SafeQueue * Q, const char* topic, const int min, const int max){
	srand(time(0));
	int range;
	if(min < 0 && max >= 0) range = max - min;
	else if(min < 0 && max < 0) range = -1*(max - min);
	else range = max - min;

	int cmd = std::rand() % (range + 1) + min;  //set initial value in range
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
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}
//////////////////////////////////////////   SENSORS   //////////////////////////////////////////

int main(){
	SafeQueue Q;

	std::thread therm(intsensor, &Q, THERM_NAME, THERM_MIN, THERM_MAX);
	std::thread hygro(intsensor, &Q, HYGRO_NAME, HYGRO_MIN, HYGRO_MAX);
	std::thread watt(intsensor, &Q, WATT_NAME, WATT_MIN, WATT_MAX);

	std::thread pir(floatsensor, &Q, PIR_NAME, PIR_MIN, PIR_MAX);
	std::thread iradar(floatsensor, &Q, I_RADAR_NAME, I_MIN, I_MAX);
	std::thread qradar(floatsensor, &Q, Q_RADAR_NAME, Q_MIN, Q_MAX);

	std::thread ds(door_switch, &Q, DS_NAME);
	std::thread valv(valve, &Q, VALVE_NAME);

	std::thread ts(thermostat, &Q, TS_NAME, TS_MIN, TS_MAX);

	mqtt::async_client client(SERVER_ADDRESS, CLIENT_ID);
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
