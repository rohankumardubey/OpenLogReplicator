/* Header for OracleEnvironment class
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

#include <unordered_map>
#include <string>
#include "types.h"
#include "DatabaseEnvironment.h"
#include "TransactionMap.h"
#include "TransactionHeap.h"
#include "TransactionBuffer.h"
#include "JsonBuffer.h"

#ifndef ORACLEENVIRONMENT_H_
#define ORACLEENVIRONMENT_H_

using namespace std;
using namespace OpenLogReplicator;

namespace OpenLogReplicatorOracle {

	class OracleObject;
	class Transaction;

	class OracleEnvironment : public DatabaseEnvironment {
	public:
		unordered_map<uint32_t, OracleObject*> objectMap;
		unordered_map<typexid, Transaction*> xidTransactionMap;
		TransactionMap lastOpTransactionMap;
		TransactionHeap transactionHeap;
		TransactionBuffer transactionBuffer;
		uint8_t *redoBuffer;
		uint8_t *headerBuffer;
		uint8_t *recordBuffer;
		JsonBuffer *jsonBuffer;
		bool dumpLogFile;
		bool dumpData;
		bool directRead;

		OracleObject *checkDict(uint32_t objn, uint32_t objd);
		void addToDict(OracleObject *object);
		void transactionNew(typexid xid);
		void transactionAppend(typexid xid);

		OracleEnvironment(JsonBuffer *jsonBuffer, bool dumpLogFile, bool dumpData, bool directRead);
		virtual ~OracleEnvironment();
	};
}

#endif
