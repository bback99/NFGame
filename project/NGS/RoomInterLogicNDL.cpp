//
// GRCContainerNDL.cpp
//

#include "stdafx.h"
#include "Room.h"
#include "RoomInternalLogic.h"
#include "RoomTable.h"
#include "UserTable.h"
#include "LrbConnector.h"
#include <NF/ADL/MsgCHSNGS.h>
#include <NF/ADL/MsgNCSNGS.h>
#include <ADL/MsgAMSGLS.h>
#include <ADL/MsgMDS.h>

#include "ErrorLog.h"
#include "Control.h"

///////////////////////////////////////////////////////////////////////////////////
// CRoomInternalLogic
#pragma oMSG("�켱�� LRB�� ���� �� ������ �޽����� �޴� ��찡 ��� �� ������.")

void CRoomInternalLogic::SendAnnounceMsg()
{
	GBuf buf;
	if (!mAnnounceMsg.Pop(buf))
		return;

	int nCharSize = sizeof(xchar);
	if(nCharSize == 0)	nCharSize = 1;		

	MsgNGSCli_AnnounceNtf Msg;
	Msg.Clear();
	Msg.m_sAnnounceMsg.assign((LPCXSTR)buf.GetData(), buf.GetLength()/nCharSize);
	PayloadNGSCli pld(PayloadNGSCli::msgAnnounceNtf_Tag, Msg);
	UserManager->SendToAllUser(pld);
}

void CRoomInternalLogic::OnAnnounceMsg()
{
	RoomEvent e( REV_ANNOUNCE, NULL );
	UserManager->AddRoomEvent( e );
}

void CRoomInternalLogic::MakeOtherAddress(LRBAddress & addr, string m_sServerName, DWORD dwCastType, DWORD dwCategory)
{
	string sApp, sKey;
	switch((LONG)dwCastType)
	{
	case 'M':
		sApp = "MULTICAST";		break;
	case 'U':
		sApp = "UNICAST";		break; // unicasting address�� ��ü���� ����� ����..
	case 'A':
		sApp = "ANYCAST";		break;
	case 'B':
		sApp = "BROADCAST";		break;
	default:
		theLog.Put(WAR_UK, "NGS_GRCContainer_Error"_COMMA, "Unknown CastType: ", dwCastType, " in MakeOtherAddress");
	}

	/**
		sKey = SERVICE_NAME[SVCCAT_NGS];
		�Ʒ��� ��ٶ� �ڵ带 ���� ���� ���� �ϸ� ���?
		���� ���� ó���� �ȵǴ� try, catch ������ �ɾ� ��� ��.
	*/
	switch(dwCategory)
	{
	case SVCCAT_NGS:
        sKey = "NGS";		break;
	case SVCCAT_CHS:
		sKey = "CHS";		break;
	case SVCCAT_NCS:
		sKey = "NCS";		break;
	case SVCCAT_AMS:
		sKey = "AMS";		break;
	case SVCCAT_NLS:
		sKey = "NLS";		break;
	case SVCCAT_BLS:
		sKey = "BLS";		break;
	default:
		theLog.Put(WAR_UK, "NGS_GRCContainer_Error"_COMMA, "Unknown Category: ", dwCategory, " in MakeOtherAddress");
	}

	CHAR szTempAddr[LRBADDRESS_SIZE+1] = {0, };
	memset(szTempAddr, ' ', LRBADDRESS_SIZE);
	CHAR szDefAddr[LRBADDRESS_SIZE/2] = {0, };
	memset(szDefAddr, ' ', LRBADDRESS_SIZE/2);
	addr.Clear();

	string strLRBAddrIni = theControl.m_confPath.GetConfDir();
	strLRBAddrIni += "LRB_ADDR.ini";
	::GetPrivateProfileStringA(sApp.c_str(), sKey.c_str(), sKey.c_str(), szDefAddr, LRBADDRESS_SIZE, strLRBAddrIni.c_str()/*"LRB_ADDR.ini"*/);
	LONG lLen = strlen(szDefAddr);
	memcpy(szTempAddr, szDefAddr, lLen);
	memcpy(szTempAddr + 14, &dwCategory, 1);
	if((CHAR)dwCastType == 'U')
	{ 
		::memcpy(szTempAddr + 15, m_sServerName.c_str(), m_sServerName.size());
	}

	DWORD dwIP = GSocketUtil::GetHostIP(true);
	string sID = adl::LRBAddress::GetIDFromIP(dwIP);
	addr.SetAddress((CHAR)dwCastType, (CHAR)dwCategory, sID);
}



