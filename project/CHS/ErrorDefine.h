/*
// ���� ���� : 0x 00 00 00 00 00 00 00 00
//                8 (���̳ʽ��� ������)
//                 X XX XX XX -> ���� ��ġ
//                            XX XX XX XX -> ���� ����.
*/

////////// ���� ����.
#define ER_SUCCESS			0x00000000
#define ER_EXECUTE			0x80000001
#define ER_PARAMETER		0x80000002

////////// �Լ��� Ư�� ���� ��ġ ���� : ���� ������ �Լ��������� �������� ������ ������.
#define ER_SETLOCATION(LD, RD)		(LD)|(0x7fff0000&((0x7fff&(RD))<<4));
#define ER_GETLOCATION(ADATA)		((ADATA)&0x7fff0000)>>4
#define ER_GETERROR(ADATA)			(ADATA)&0x8000ffff

#define ER_ISSUCCESS(ADATA)			(((ADATA) & 0x8000ffff)==ER_SUCCESS?TRUE:FALSE)
