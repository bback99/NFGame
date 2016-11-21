/*
// 에러 포맷 : 0x 00 00 00 00 00 00 00 00
//                8 (마이너스로 만들라공)
//                 X XX XX XX -> 에러 위치
//                            XX XX XX XX -> 에러 종류.
*/

////////// 에러 종류.
#define ER_SUCCESS			0x00000000
#define ER_EXECUTE			0x80000001
#define ER_PARAMETER		0x80000002

////////// 함수의 특정 에러 위치 지정 : 같은 에러가 함수내에서도 여러군데 있을수 있음둥.
#define ER_SETLOCATION(LD, RD)		(LD)|(0x7fff0000&((0x7fff&(RD))<<4));
#define ER_GETLOCATION(ADATA)		((ADATA)&0x7fff0000)>>4
#define ER_GETERROR(ADATA)			(ADATA)&0x8000ffff

#define ER_ISSUCCESS(ADATA)			(((ADATA) & 0x8000ffff)==ER_SUCCESS?TRUE:FALSE)
