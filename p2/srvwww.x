const CHUNK_LEN   = 64;
const MSG_LEN     = 256;
const FILENAME_LEN = 128;

typedef opaque chunk<CHUNK_LEN>;
typedef string long_string<MSG_LEN>;
typedef string filename_string<FILENAME_LEN>;

struct param1 {
   filename_string filename;
   long offset;
   int size;
};

struct chunk_list {
   chunk chunk_data;
   chunk_list *next;
};

union Result switch (int case_id){
   case 0: bool b;
   case 1: long val;
   case 2: chunk_list *list;
   case 3: long_string err;
};

program SRVWWW {
   version FIRST {
     Result existfile(filename_string) = 1;
     Result getsize(filename_string)   = 2;
     Result getchunk(param1)           = 3;
   } = 1;
} = 0x2026f001;
