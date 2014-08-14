#include "apiserver/BasicApiClient.h"

void Susi::Api::BasicApiClient::sendPublish(Susi::Events::Event & event){
	Susi::Util::Any packedEvent = eventToAny(event);
	Susi::Util::Any packet = Susi::Util::Any::Object{
		{"type","publish"},
		{"data",packedEvent}
	};

	JSONTCPClient::send(packet);
}

void Susi::Api::BasicApiClient::sendAck(Susi::Events::Event & event){
	Susi::Util::Any packedEvent = eventToAny(event);
	Susi::Util::Any packet = Susi::Util::Any::Object{
		{"type","ack"},
		{"data",packedEvent}
	};
	//std::cout<<"send ack"<<std::endl;
	JSONTCPClient::send(packet);
	//std::cout<<"sended ack"<<std::endl;
}

void Susi::Api::BasicApiClient::sendRegisterConsumer(std::string topic){
	Susi::Util::Any packet = Susi::Util::Any::Object{
		{"type","registerConsumer"},
		{"data",topic}
	};
	JSONTCPClient::send(packet);	
}

void Susi::Api::BasicApiClient::sendRegisterProcessor(std::string topic){
	Susi::Util::Any packet = Susi::Util::Any::Object{
		{"type","registerProcessor"},
		{"data",topic}
	};
	JSONTCPClient::send(packet);	
}

void Susi::Api::BasicApiClient::onMessage(Susi::Util::Any & message){
	//std::cout<<"got message in basic api client"<<std::endl;
	std::string type = message["type"];
	if(type=="ack"){
		//std::cout<<"got ack"<<std::endl;	
		auto event = std::make_shared<Susi::Events::Event>();
		anyToEvent(message["data"],*event);
		onAck(event);
	}else if(type=="consumerEvent"){
		//std::cout<<"got consumer event"<<std::endl;
		auto event = std::make_shared<Susi::Events::Event>();
		anyToEvent(message["data"],*event);
		onConsumerEvent(event);
	}else if(type=="processorEvent"){
		//std::cout<<"got processor event"<<std::endl;
		auto deleter = [this](Susi::Events::Event *ptr){
			sendAck(*ptr);
			delete ptr;
		};
		auto event = std::unique_ptr<Susi::Events::Event,decltype(deleter)>{new Susi::Events::Event,deleter};
		anyToEvent(message["data"],*event);
		onProcessorEvent(std::move(event));
	}else{
		//std::cout<<"got status"<<std::endl;
	}
}

Susi::Util::Any Susi::Api::BasicApiClient::eventToAny(Susi::Events::Event & event){
	Susi::Util::Any::Array headers;
	for(auto & kv : event.headers){
		headers.push_back(Susi::Util::Any::Object{{kv.first,kv.second}});
	}
	Susi::Util::Any any = Susi::Util::Any::Object{
		{"topic",event.topic},
		{"id",event.id},
		{"sessionid",event.sessionID},
		{"payload",event.payload},
		{"headers",headers}
	};
	return any;
}

void Susi::Api::BasicApiClient::anyToEvent(Susi::Util::Any & any, Susi::Events::Event & event){
	event.topic = static_cast<std::string>(any["topic"]);
	event.id = static_cast<long>(any["id"]);
	event.sessionID = static_cast<std::string>(any["sessionid"]);
	event.payload = any["payload"];
	Susi::Util::Any::Array headers = any["headers"];
	for(Susi::Util::Any::Object & obj : headers){
		for(auto & kv : obj){
			event.headers.push_back(std::make_pair(kv.first,kv.second));
		}
	}
}