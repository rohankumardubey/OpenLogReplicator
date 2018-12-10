/* Header for Thread class
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

#include <string>
#include <pthread.h>
#include "types.h"

#ifndef THREAD_H_
#define THREAD_H_

using namespace std;

namespace OpenLogReplicator {

	class JsonBuffer;

	class Thread {

	public:
		volatile bool shutdown;
		pthread_t thread;
		string alias;
		JsonBuffer *jsonBuffer;

		static void *runStatic(void *context);

		void terminate(void);
		virtual void *run();
		int initialize();

		Thread(const string alias, JsonBuffer *jsonBuffer);
		virtual ~Thread();
	};
}

#endif
