/* Definition of types and macros
   Copyright (C) 2018-2020 Adam Leszczynski (aleszczynski@bersler.com)

This file is part of OpenLogReplicator.

OpenLogReplicator is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation; either version 3, or (at your option)
any later version.

OpenLogReplicator is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License
along with OpenLogReplicator; see the file LICENSE;  If not see
<http://www.gnu.org/licenses/>.  */

#include <iomanip>
#include <ostream>
#include <stdint.h>
#include <time.h>

#ifndef TYPES_H_
#define TYPES_H_

#define ONLINE_MODEIMPL_OCCI

typedef uint16_t typesum;
typedef uint32_t typeresetlogs;
typedef uint32_t typeactivation;
typedef uint16_t typeop1;
typedef uint32_t typeop2;
typedef uint16_t typecon;
typedef uint32_t typeblk;
typedef uint32_t typedba;
typedef uint16_t typeslot;
typedef uint8_t typeslt;
typedef uint8_t typerci;
typedef uint32_t typeobj;
typedef uint64_t typeobj2;
typedef uint32_t typeseq;
typedef uint64_t typeuba;
typedef uint64_t typexid;
typedef uint64_t typescn;
typedef uint16_t typesubscn;
typedef uint32_t typeseq;

typedef uint16_t typeunicode16;
typedef uint32_t typeunicode32;
typedef uint64_t typeunicode;

#define ZERO_SCN                    ((typescn)0xFFFFFFFFFFFFFFFF)
#define PROGRAM_VERSION             "0.6.11"
#define MAX_PATH_LENGTH             2048
#define MAX_NO_COLUMNS              1000
#define MAX_TRANSACTIONS_LIMIT      1048576
#define MEMORY_CHUNK_SIZE_MB        1
#define MEMORY_CHUNK_SIZE_MB_CHR    "1"
#define MEMORY_CHUNK_SIZE           (MEMORY_CHUNK_SIZE_MB*1024*1024)
#define MEMORY_CHUNK_MIN_MB         16
#define MEMORY_CHUNK_MIN_MB_CHR     "16"

#define STREAM_JSON                 1
#define STREAM_DBZ_JSON             2

#define MODE_ONLINE                 1
#define MODE_ONLINE_ARCH            2
#define MODE_OFFLINE                3
#define MODE_OFFLINE_ARCH           4
#define MODE_STANDBY                5
#define MODE_STANDBY_ARCH           6
#define MODE_BATCH                  7

#define TRACE_SILENT                0
#define TRACE_WARNING               1
#define TRACE_INFO                  2
#define TRACE_FULL                  3

#define TRACE2_THREADS              0x0000001
#define TRACE2_SQL                  0x0000002
#define TRACE2_FILE                 0x0000004
#define TRACE2_DISK                 0x0000008
#define TRACE2_MEMORY               0x0000010
#define TRACE2_PERFORMANCE          0x0000020
#define TRACE2_VECTOR               0x0000040
#define TRACE2_TRANSACTION          0x0000080
#define TRACE2_COMMIT_ROLLBACK      0x0000100
#define TRACE2_REDO                 0x0000200
#define TRACE2_CHECKPOINT_FLUSH     0x0000400
#define TRACE2_DML                  0x0000800
#define TRACE2_SPLIT                0x0001000
#define TRACE2_ARCHIVE_LIST         0x0002000
#define TRACE2_ROLLBACK             0x0004000
#define TRACE2_DUMP                 0x0008000

#define REDO_RECORD_MAX_SIZE            1048576

#define REDO_FLAGS_DIRECT               0x0000001
#define REDO_FLAGS_NOATIME              0x0000002
#define REDO_FLAGS_ON_ERROR_CONTINUE    0x0000004
#define REDO_FLAGS_TRACK_DDL            0x0000008
#define REDO_FLAGS_DISABLE_READ_VERIFICATION 0x0000010
#define REDO_FLAGS_BLOCK_CHECK_SUM      0x0000020
#define REDO_FLAGS_HIDE_INVISIBLE_COLUMNS 0x0000040
#define REDO_FLAGS_INCOMPLETE_TRANSACTIONS 0x0000080

#define DISABLE_CHECK_GRANTS            0x0000001
#define DISABLE_CHECK_SUPPLEMENTAL_LOG  0x0000002

#define USN(xid)                    ((uint16_t)(((uint64_t)xid)>>48))
#define SLT(xid)                    ((uint16_t)(((((uint64_t)xid)>>32)&0xFFFF)))
#define SQN(xid)                    ((uint32_t)(((xid)&0xFFFFFFFF)))
#define XID(usn,slt,sqn)            ((((uint64_t)(usn))<<48)|(((uint64_t)(slt))<<32)|((uint64_t)(sqn)))
#define PRINTXID(xid)               "0x"<<setfill('0')<<setw(4)<<hex<<USN(xid)<<"."<<setw(3)<<SLT(xid)<<"."<<setw(8)<<SQN(xid)

#define BLOCK(uba)                  ((uint32_t)((uba)&0xFFFFFFFF))
#define SEQUENCE(uba)               ((uint16_t)((((uint64_t)uba)>>32)&0xFFFF))
#define RECORD(uba)                 ((uint8_t)((((uint64_t)uba)>>48)&0xFF))
#define PRINTUBA(uba)               "0x"<<setfill('0')<<setw(8)<<hex<<BLOCK(uba)<<"."<<setfill('0')<<setw(4)<<hex<<SEQUENCE(uba)<<"."<<setfill('0')<<setw(2)<<hex<<(uint32_t)RECORD(uba)

#define SCN(scn1,scn2)              ((((uint64_t)scn1)<<32)|(scn2))
#define PRINTSCN48(scn)             "0x"<<setfill('0')<<setw(4)<<hex<<((uint32_t)((scn)>>32)&0xFFFF)<<"."<<setw(8)<<((scn)&0xFFFFFFFF)
#define PRINTSCN64(scn)             "0x"<<setfill('0')<<setw(16)<<hex<<(scn)

#define DUMP(x) {stringstream s; s << x << endl; cerr << s.str(); }
#define ERROR(x) {stringstream s; s << "ERROR: " << x << endl; cerr << s.str(); }
#define OUT(x) {cerr << x; }

#define WARNING(x) {if (oracleAnalyser->trace >= TRACE_INFO){stringstream s; s << "WARNING: " << x << endl; cerr << s.str();} }
#define INFO(x) {if (oracleAnalyser->trace >= TRACE_INFO){stringstream s; s << "INFO: " << x << endl; cerr << s.str();} }
#define FULL(x) {if (oracleAnalyser->trace >= TRACE_FULL){stringstream s; s << "FULL: " << x << endl; cerr << s.str();} }
#define TRACE(t,x) {if ((oracleAnalyser->trace2 & (t)) != 0) {stringstream s; s << "TRACE: " << x << endl; cerr << s.str();} }

#define WARNING_(x) {if (trace >= TRACE_INFO){stringstream s; s << "WARNING: " << x << endl; cerr << s.str();} }
#define INFO_(x) {if (trace >= TRACE_INFO){stringstream s; s << "INFO: " << x << endl; cerr << s.str();} }
#define FULL_(x) {if (trace >= TRACE_FULL){stringstream s; s << "FULL: " << x << endl; cerr << s.str();} }
#define TRACE_(t,x) {if ((trace2 & (t)) != 0) {stringstream s; s << "TRACE: " << x << endl; cerr << s.str();} }

using namespace std;

namespace OpenLogReplicator {

    class typetime {
        uint32_t val;
    public:
        typetime(void) {
            this->val = 0;
        }

        typetime(uint32_t val) {
            this->val = val;
        }

        uint32_t getVal(void) {
            return this->val;
        }

        typetime& operator= (uint32_t val) {
            this->val = val;
            return *this;
        }

        time_t toTime(void) {
            struct tm epochtime;
            uint64_t rest = val;
            epochtime.tm_sec = rest % 60; rest /= 60;
            epochtime.tm_min = rest % 60; rest /= 60;
            epochtime.tm_hour = rest % 24; rest /= 24;
            epochtime.tm_mday = (rest % 31) + 1; rest /= 31;
            epochtime.tm_mon = (rest % 12); rest /= 12;
            epochtime.tm_year = rest + 88;
            return mktime(&epochtime);
        }

        friend ostream& operator<<(ostream& os, const typetime& time) {
            uint64_t rest = time.val;
            uint64_t ss = rest % 60; rest /= 60;
            uint64_t mi = rest % 60; rest /= 60;
            uint64_t hh = rest % 24; rest /= 24;
            uint64_t dd = (rest % 31) + 1; rest /= 31;
            uint64_t mm = (rest % 12) + 1; rest /= 12;
            uint64_t yy = rest + 1988;
            os << setfill('0') << setw(2) << dec << mm << "/" << setfill('0') << setw(2) << dec << dd << "/" << yy << " " <<
                    setfill('0') << setw(2) << dec << hh << ":" << setfill('0') << setw(2) << dec << mi << ":" << setfill('0') << setw(2) << dec << ss;
            return os;
            //DDDDDDDDDD HHHHHHHH
            //10/15/2018 22:25:36
        }
    };
}

#define CHECKPOINT_SIZE 12

#endif
