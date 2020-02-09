
#define DLOG if(doDebug) RLOG
#define BOTLOG if(GetId() == "fb0") DLOG << "<DB> "