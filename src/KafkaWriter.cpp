/* Thread writing to Kafka stream
   Copyright (C) 2018 Adam Leszczynski.

This file is part of Open Log Replicator.

Open Log Replicator is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 3, or (at your option)
any later version.

Open Log Replicator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License
along with Open Log Replicator; see the file LICENSE.txt  If not see
<http://www.gnu.org/licenses/>.  */

#include <sys/stat.h>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <mutex>
#include <unistd.h>
#include <string.h>
#include <librdkafka/rdkafkacpp.h>
#include "types.h"
#include "JsonBuffer.h"
#include "KafkaWriter.h"

using namespace std;
using namespace RdKafka;

namespace OpenLogReplicatorKafka {

	KafkaWriter::KafkaWriter(const string alias, const string brokers, const string topic, JsonBuffer *jsonBuffer) :
		Thread(alias, jsonBuffer),
		conf(nullptr),
		tconf(nullptr),
		brokers(brokers.c_str()),
		topic(topic.c_str()),
		producer(nullptr),
		ktopic(nullptr) {
	}

	KafkaWriter::~KafkaWriter() {
	    if (ktopic != nullptr) {
	    	delete ktopic;
	    	ktopic = nullptr;
	    }
	    if (producer != nullptr) {
	    	delete producer;
	    	producer = nullptr;
	    }
		if (tconf != nullptr) {
			delete tconf;
			tconf = nullptr;
		}
		if (conf != nullptr) {
			delete conf;
			conf = nullptr;
		}
	}

	void *KafkaWriter::run() {
		cout << "- Kafka Writer for: " << brokers << " topic: " << topic << endl;

		while (!this->shutdown) {
			uint32_t length;
			{
				unique_lock<mutex> lck(jsonBuffer->mtx);
				while (jsonBuffer->posStart == jsonBuffer->posEnd) {
					jsonBuffer->readers.wait(lck);

					if (this->shutdown)
						break;
				}
				if (this->shutdown)
					break;

				if (jsonBuffer->posStart == jsonBuffer->posSize && jsonBuffer->posSize > 0) {
					jsonBuffer->posStart = 0;
					jsonBuffer->posSize = 0;
				}

				length = *((uint32_t*)(jsonBuffer->intraThreadBuffer + jsonBuffer->posStart));
			}

			if (producer->produce(
					ktopic, Topic::PARTITION_UA, Producer::RK_MSG_COPY, jsonBuffer->intraThreadBuffer + jsonBuffer->posStart + 4,
					length - 4, nullptr, nullptr)) {
				cerr << "ERROR: writing to topic " << endl;
				break;
			}

			{
				unique_lock<mutex> lck(jsonBuffer->mtx);
				jsonBuffer->posStart += (length + 3) & 0xFFFFFFFC;
				jsonBuffer->writer.notify_all();
			}
		}

		return 0;
	}

	int KafkaWriter::initialize() {
		string errstr;
		Conf *conf = Conf::create(Conf::CONF_GLOBAL);
		Conf *tconf = Conf::create(Conf::CONF_TOPIC);
		conf->set("metadata.broker.list", brokers, errstr);

	    producer = Producer::create(conf, errstr);
	    if (producer == nullptr) {
	        std::cerr << "ERROR: creating Kafka producer: " << errstr << endl;
	        return 0;
	    }

	    ktopic = Topic::create(producer, topic, tconf, errstr);
		if (ktopic == nullptr) {
			std::cerr << "ERROR: Failed to create Kafka topic: " << errstr << endl;
			return 0;
		}

		return 1;
	}
}
