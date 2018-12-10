/* Oracle Redo Generic OpCode
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

#include <iostream>
#include <iomanip>
#include "OpCode.h"
#include "OracleEnvironment.h"
#include "RedoLogException.h"
#include "RedoLogRecord.h"

using namespace std;

namespace OpenLogReplicatorOracle {

	OpCode::OpCode(OracleEnvironment *oracleEnvironment, RedoLogRecord *redoLogRecord, bool fill):
			oracleEnvironment(oracleEnvironment),
			redoLogRecord(redoLogRecord),
			itli(0),
			slot(0) {
		redoLogRecord->fieldLengths = (uint16_t*)(redoLogRecord->data + 24);
	}

	OpCode::OpCode(OracleEnvironment *oracleEnvironment, RedoLogRecord *redoLogRecord):
			oracleEnvironment(oracleEnvironment),
			redoLogRecord(redoLogRecord),
			itli(0),
			slot(0) {

		uint32_t fieldPosTmp = redoLogRecord->fieldPos;
		for (int i = 1; i <= redoLogRecord->fieldNum; ++i) {
			if (oracleEnvironment->dumpData) {
				cout << "  " << dec << i << " (" << redoLogRecord->fieldLengths[i] << "): ";
				for (int j = 0; j < redoLogRecord->fieldLengths[i]; ++j)
					cout << hex << setfill('0') << setw(2) << (unsigned int) redoLogRecord->data[fieldPosTmp + j] << " ";
				cout << endl;
			}
			fieldPosTmp += (redoLogRecord->fieldLengths[i] + 3) & 0xFFFC;
		}
	}

	OpCode::~OpCode() {
	}

	void OpCode::process() {
	}


	uint16_t OpCode::getOpCode(void) {
		return 0xFFFF;
	}

	void OpCode::ktbRedo(uint32_t fieldPos, uint32_t fieldLength) {
		if (fieldLength < 8)
			throw RedoLogException("ERROR: to short field KTB Redo: ", nullptr, fieldLength);

		uint8_t op = redoLogRecord->data[fieldPos + 0];
		if (oracleEnvironment->dumpLogFile) {
			uint8_t ver = redoLogRecord->data[fieldPos + 1] & 0x03;

			cout << "KTB Redo" << endl;
			cout << "op: 0x" << setfill('0') << setw(2) << hex << (int)op << " " <<
					" ver: 0x" << setfill('0') << setw(2) << hex << (int)ver << endl;
			cout << "compat bit: 4 (post-11) padding: 1" << endl;
		}
		char opCode = '?';

		if (op == 0x02) {
			if (fieldLength < 16)
				throw RedoLogException("ERROR: to short field KTB Redo 2: ", nullptr, fieldLength);

			redoLogRecord->uba = oracleEnvironment->read56(redoLogRecord->data + fieldPos + 8);

			if (oracleEnvironment->dumpLogFile) {
				opCode = 'C';
				cout << "op: " << opCode << " " << " uba: " << PRINTUBA(redoLogRecord->uba) << endl;
			}
		} else if (op == 0x03) {
			if (oracleEnvironment->dumpLogFile) {
				opCode = 'Z';
				cout << "op: " << opCode << endl;
			}
		} else if (op == 0x04) {
			if (fieldLength < 32)
				throw RedoLogException("ERROR: to short field KTB Redo 4: ", nullptr, fieldLength);

			redoLogRecord->uba = oracleEnvironment->read56(redoLogRecord->data + fieldPos + 16);

			if (oracleEnvironment->dumpLogFile) {
				opCode = 'L';
				typexid itlXid = XID(oracleEnvironment->read16(redoLogRecord->data + fieldPos + 8),
						oracleEnvironment->read16(redoLogRecord->data + fieldPos + 10),
						oracleEnvironment->read32(redoLogRecord->data + fieldPos + 12));
				uint8_t lkc = redoLogRecord->data[fieldPos + 24]; //FIXME
				//uint8_t flag = redoLogRecord->data[fieldPos + 25];
				typescn scnx = SCN(oracleEnvironment->read16(redoLogRecord->data + fieldPos + 26),
						oracleEnvironment->read32(redoLogRecord->data + fieldPos + 28));

				cout << "op: " << opCode << " " <<
						" itl:" <<
						" xid:  " << PRINTXID(itlXid) <<
						" uba: " << PRINTUBA(redoLogRecord->uba) << endl;
				cout << "                     " <<
						" flg: C---   " <<
						" lkc:  " << (int)lkc << "    " <<
						" scn: " << PRINTSCN(scnx) << endl;
			}

		} else if (op == 0x01 || op == 0x11) {

			if (fieldLength < 24)
				throw RedoLogException("ERROR: to short field KTB Redo F: ", nullptr, fieldLength);

			redoLogRecord->xid = XID(oracleEnvironment->read16(redoLogRecord->data + fieldPos + 8),
					oracleEnvironment->read16(redoLogRecord->data + fieldPos + 10),
					oracleEnvironment->read32(redoLogRecord->data + fieldPos + 12));
			redoLogRecord->uba = oracleEnvironment->read56(redoLogRecord->data + fieldPos + 16);

			if (oracleEnvironment->dumpLogFile) {
				opCode = 'F';

				cout << "op: " << opCode << " " <<
						" itl:" <<
						" xid:  " << PRINTXID(redoLogRecord->xid) <<
						" uba: " << PRINTUBA(redoLogRecord->uba) << endl;
			}

			if (op == 0x11) {
				if (oracleEnvironment->dumpLogFile) {
					typescn scn = oracleEnvironment->read48(redoLogRecord->data + fieldPos + 34);
					uint8_t ver = redoLogRecord->data[fieldPos + 45];
					uint8_t opt = redoLogRecord->data[fieldPos + 46];

					uint8_t itli1 = redoLogRecord->data[fieldPos + 56];
					uint8_t flg1 = redoLogRecord->data[fieldPos + 57];
					typescn scn1 = oracleEnvironment->read48(redoLogRecord->data + fieldPos + 58);
					uint8_t itli2 = redoLogRecord->data[fieldPos + 64];
					uint8_t flg2 = redoLogRecord->data[fieldPos + 65];
					typescn scn2 = oracleEnvironment->read48(redoLogRecord->data + fieldPos + 66);

					cout << "Block cleanout record, scn: " <<
							" " << PRINTSCN(scn) <<
							" ver: 0x" << setfill('0') << setw(2) << hex << (int)ver <<
							" opt: 0x" << setfill('0') << setw(2) << hex << (int)opt <<
							", entries follow..." << endl;
					cout << "  itli: " << (int)itli1 << " " <<
							" flg: " << (int)flg1 << " " <<
							" scn: " << PRINTSCN(scn1) << endl;
					cout << "  itli: " << (int)itli2 << " " <<
							" flg: " << (int)flg2 << " " <<
							" scn: " << PRINTSCN(scn2) << endl;
				}
			}
		}
	}

	void OpCode::kdoOpCode(uint32_t fieldPos, uint32_t fieldLength) {
		if (fieldLength < 16)
			throw RedoLogException("ERROR: to short field KDO OpCode: ", nullptr, fieldLength);

	    itli = redoLogRecord->data[fieldPos + 12];
		uint8_t op = redoLogRecord->data[fieldPos + 10];
		redoLogRecord->bdba = oracleEnvironment->read32(redoLogRecord->data + fieldPos + 0);

		if (oracleEnvironment->dumpLogFile) {
			uint32_t hdba = oracleEnvironment->read32(redoLogRecord->data + fieldPos + 4);
			uint16_t maxFr = oracleEnvironment->read16(redoLogRecord->data + fieldPos + 8);
			uint8_t xtype = redoLogRecord->data[fieldPos + 11];
		    uint8_t ispac = redoLogRecord->data[fieldPos + 13];

		    const char* opCode = "???";
		    switch (op & 0x1F) {
		    	case 0x01: opCode = "IUR"; break; //Interpret Undo Redo
		    	case 0x02: opCode = "IRP"; break; //Insert Row Piece
		    	case 0x03: opCode = "DRP"; break; //Delete Row Piece
		    	case 0x04: opCode = "LKR"; break; //LocK Row
		    	case 0x05: opCode = "UPR"; break; //Update Row Piece
		    	case 0x06: opCode = "ORP"; break; //Overwrite Row Piece
		    	case 0x07: opCode = "MFC"; break; //Manipulate First Column
		    	case 0x08: opCode = "CFA"; break; //Change Forwarding Address
		    	case 0x09: opCode = "CKI"; break; //Change Cluster key Index
		    	case 0x0a: opCode = "SKL"; break; //Set Key Links
		    	case 0x0b: opCode = "QMI"; break; //Quick Multi-row Insert
		    	case 0x0c: opCode = "QMD"; break; //Quick Multi-row Delete
		    	case 0x0d: opCode = "TBF"; break;
		    	case 0x0e: opCode = "DSC"; break;
		    	case 0x10: opCode = "LMN"; break;
		    	case 0x11: opCode = "LLB"; break;
		    }

			string xtypeStr;
		    if (xtype == 1) xtypeStr = "XA"; //redo
		    else if (xtype == 2) xtypeStr = "XR"; //rollback
		    else if (xtype == 2) xtypeStr = "CR"; //unknown
		    else xtypeStr = "??";

		    cout << "KDO Op code: " << opCode << " row dependencies Disabled" << endl;
			cout << "  xtype: " << xtypeStr <<
					" flags: 0x00000000 " <<
					" bdba: 0x" << setfill('0') << setw(8) << hex << redoLogRecord->bdba << " " <<
					" hdba: 0x" << setfill('0') << setw(8) << hex << hdba << endl;
			cout << "itli: " << dec << (int)itli << " " <<
					" ispac: " << dec << (int)ispac << " " <<
					" maxfr: " << dec << (int)maxFr << endl;
		}

		if ((op & 0x1F) == 0x03) { //Delete Row Piece
			if (fieldLength < 20)
				throw RedoLogException("ERROR: to short field KDO OpCode DRP: ", nullptr, fieldLength);

			uint32_t tabn = redoLogRecord->data[fieldPos + 15]; //FIXME
		    slot = redoLogRecord->data[fieldPos + 16];

			if (oracleEnvironment->dumpLogFile) {
				cout << "tabn: "<< tabn <<
						" slot: " << dec << (int)slot << "(0x" << setfill('0') << setw(2) << hex << (int)slot << ")" << endl;
			}

		} else

		if ((op & 0x1F) == 0x02) { //Insert Row Piece
			if (fieldLength < 48) {
				cout << "length is " << fieldLength << endl;
				throw RedoLogException("ERROR: to short field KDO OpCode IRP: ", nullptr, fieldLength);
			}

		    uint32_t tabn = redoLogRecord->data[fieldPos + 15]; //FIXME
		    slot = redoLogRecord->data[fieldPos + 42];

			if (oracleEnvironment->dumpLogFile) {
				cout << "tabn: "<< tabn <<
						" slot: " << dec << (int)slot << "(0x" << setfill('0') << setw(2) << hex << (int)slot << ")" << endl;
			}

			if (oracleEnvironment->dumpLogFile) {
				uint8_t fl = redoLogRecord->data[fieldPos + 16];
				uint8_t lb = redoLogRecord->data[fieldPos + 17];
				uint8_t cc = redoLogRecord->data[fieldPos + 18];
				char flStr[9] = "--------";

				if ((fl & 0x80) == 0x01) flStr[7] = 'N'; //last column continues in Next piece
				if ((fl & 0x80) == 0x02) flStr[6] = 'P'; //first column continues from Previous piece
				if ((fl & 0x80) == 0x04) flStr[5] = 'L'; //Last data piece
				if ((fl & 0x80) == 0x08) flStr[4] = 'F'; //First data piece
				if ((fl & 0x80) == 0x10) flStr[3] = 'D'; //Deleted row
				if ((fl & 0x80) == 0x20) flStr[2] = 'H'; //Head piece of row
				if ((fl & 0x80) == 0x40) flStr[1] = 'C'; //Clustered table member
				if ((fl & 0x80) == 0x80) flStr[0] = 'K'; //cluster Key

				cout << "fb: " << flStr <<
						" lb: 0x" << hex << (int)lb << " " <<
						" cc: " << dec << (int)cc << endl;
				cout << "null:" << endl;
				cout << "0123 4567 8901 2345 6789 0123 4567 8901 2345 67890 123456789012345678901234567890123456789" << endl;
				cout << "---N --N- -NNN NNNN NNNN NNNN -NNN -N-N N--N NN-" << endl;
				//       8    4    e    f    f    f    e    a    9    3
			}
		}
	}

	void OpCode::ktub(uint32_t fieldPos, uint32_t fieldLength) {
		if (fieldLength < 24)
			throw RedoLogException("ERROR: to short field ktub: ", nullptr, fieldLength);

		redoLogRecord->objn = oracleEnvironment->read32(redoLogRecord->data + fieldPos + 0);
		redoLogRecord->objd = oracleEnvironment->read32(redoLogRecord->data + fieldPos + 4);
		redoLogRecord->tsn = oracleEnvironment->read32(redoLogRecord->data + fieldPos + 8);
		redoLogRecord->undo = oracleEnvironment->read32(redoLogRecord->data + fieldPos + 12);
		redoLogRecord->slt = redoLogRecord->data[fieldPos + 18];
		redoLogRecord->rci = redoLogRecord->data[fieldPos + 19];
		redoLogRecord->flg = redoLogRecord->data[fieldPos + 20];
		redoLogRecord->opc = (((uint16_t)redoLogRecord->data[fieldPos + 16]) << 8) |
				redoLogRecord->data[fieldPos + 17];

		if (oracleEnvironment->dumpLogFile) {
			//uint8_t isKtUbl = redoLogRecord->data[fieldPos + 20];
		}
	}


	void OpCode::ktubu(uint32_t fieldPos, uint32_t fieldLength) {
		if (fieldLength < 24)
			throw RedoLogException("ERROR: to short field ktubu.B.1: ", nullptr, fieldLength);

		if (oracleEnvironment->dumpLogFile) {
			string lastBufferSplit = "No"; //FIXME
			string tablespaceUndo = "No"; //FIXME

			cout << "ktubu redo:" <<
					" slt: " << dec << (int)redoLogRecord->slt <<
					" rci: " << dec << (int)redoLogRecord->rci <<
					" opc: " << (int)(redoLogRecord->opc >> 8) << "." << (int)(redoLogRecord->opc & 0xFF) <<
					" objn: " << redoLogRecord->objn << " objd: " << redoLogRecord->objd << " tsn: " << redoLogRecord->tsn << endl;
			cout << "Undo type:  Regular undo       Undo type:  " << getUndoType() << "Last buffer split:  " << lastBufferSplit << endl;
			cout << "Tablespace Undo:  " << tablespaceUndo << endl;
			cout << "             0x" << hex << setfill('0') << setw(8) << redoLogRecord->undo << endl;

			if (isKdoUndo())
				cout << "KDO undo record:" << endl;
		}
	}

	const char* OpCode::getUndoType() {
		return "";
	}

	bool OpCode::isKdoUndo() {
		return false;
	}

	string OpCode::getName() {
		return "?????????? ";
	}

	void OpCode::dumpDetails() {
		cout << "Append " <<
				" dba: 0x" << setfill('0') << setw(8) << hex << redoLogRecord->dba <<
		    	" xid: " << PRINTXID(redoLogRecord->xid) <<
				" uba: " << PRINTUBA(redoLogRecord->uba) <<
				" len: " << dec << redoLogRecord->length <<
				" OP: 0x" << setfill('0') << setw(4) << hex << getOpCode() << endl;
	}

	void OpCode::dump() {
		cout << "  + " << getName() << " " << PRINTXID(redoLogRecord->xid) <<
				" UBA " << PRINTUBA(redoLogRecord->uba) <<
				" BDBA 0x" << setw(8) << hex << redoLogRecord->bdba <<
				" ITLI 0x" << setw(2) << hex << (int)itli << " ";
		dumpDetails();
		cout << endl;
	}
}
