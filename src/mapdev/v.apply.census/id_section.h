#define NID 68
struct ID_SEC { /* IDENTIFICATION SECTION OF STF1A FILE */
	char *name;  /* field name */
	int len;       /* field length */
	int col;       /* field starting column #, 1=first char */
} id[NID]={
	"FILEID",8,1,
	"STUSAB",2,9,
	"SUMLEV",3,11,
	"GEOCOMP",2,14,
	"CHARITER",3,16,
	"LOGRECNU",6,19,
	"LOGRECPN",4,25,
	"PARTREC",4,29,
	"ANRC",2,33,
	"AIANACE",4,35,
	"AIANAFP",5,39,
	"AIANACC",2,44,
	"ARTLI",1,46,
	"BLCK",4,47,
	"BLCKGR",1,51,
	"TRACTBNA",6,52,
	"CONGDIS",2,58,
	"CONCITCE",1,60,
	"CONCITFP",5,61,
	"CONCITCC",2,66,
	"CONCITSC",2,68,
	"CMSA",2,70,
	"CNTY",3,72,
	"CNTYSC",2,75,
	"COUSUBCE",3,77,
	"COUSUBFP",5,80,
	"COUSUBCC",2,85,
	"COUSUBSC",2,87,
	"DIVIS",1,89,
	"EXTCITIN",1,90,
	"INTUC",15,91,
	"MSACMSA",4,106,
	"MSACMSAS",2,110,
	"PLACECE",4,112,
	"PLACEFP",5,116,
	"PLACECC",2,121,
	"PLACEDC",1,123,
	"PLACESC",2,124,
	"PMSA",4,126,
	"REG",1,130,
	"STATECE",2,131,
	"STATEFP",2,133,
	"URBANRUR",1,135,
	"URBAREA",4,136,
	"UASC",2,140,
	"SAC1",5,142,
	"SAC2",5,147,
	"SAC3",4,152,
	"SAC4",4,156,
	"SAC5",3,160,
	"SAC6",3,163,
	"SAC7",2,166,
	"SAC8",2,168,
	"SAC9",1,170,
	"SAC10",1,171,
	"AREALAND",10,172,
	"AREAWAT",10,182,
	"ANPSADPI",66,192,
	"FUNCSTAT",1,258,
	"GCUNI",1,259,
	"HU100",9,260,
	"INTPTLAT",9,269,
	"INTPTLNG",10,278,
	"PARTFLAG",1,288,
	"PSADC",2,289,
	"POP100",9,291,
	"SPFLAG",1,300,
	"",0,0
};