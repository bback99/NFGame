#include "StdAfx.h"
#include ".\achvdbmanager.h"


namespace achv
{
	const string AchvDBManager::QRY_SELECT_ALL_NF_ACHV = "GameDB|Q|SELECT ACHV_ID, CATEGORY, START_DATE, END_DATE, NOTICE_CLASS, ACHV_TYPE, PROGRESS_TYPE, TRACE_TYPE, IS_HIDDEN, GOAL, ALWAYS_NOTI FROM NF_BT_ACHV";
	const string AchvDBManager::QRY_SELECT_ALL_NF_ACHV_REWARD = "GameDB|Q|SELECT ACHV_ID, AP, MONEY, EXP, ITEM_CNT, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4 FROM NF_BT_ACHV_REWARD";
	const string AchvDBManager::QRY_SELECT_ALL_NF_ACHV_RELATIVE = "GameDB|Q|SELECT GROUP_ID, ACHV_ORDER, ACHV_ID FROM NF_BT_ACHV_RELATIVE";
	const string AchvDBManager::QRY_SELECT_NF_CHAR_ACHV = "GameDB|Q|SELECT ACHV_ID, PROGRESS, COMPLETE_DATE, GET_REWARD_DATE, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4, LAST_UPDATE_DATE, REMARK FROM NF_GT_CHAR_ACHV WHERE GSN=? AND CSN=?|%d|%d";
	const string AchvDBManager::PKG_NF_ACHV_UPDATE_ACHV = "GameDB|S|PKG_NF_ACHV.UPDATE_ACHV|%d|%d|%d|%.2f|%s";

	bool Converttime_t2NFDateString( const time_t timeval, std::string & NFDateString )
	{
		if( InvalidTime_t == timeval )
		{
			NFDateString = InvalidDateString;
			return true;
		}
		char buffer[16];
		struct tm * timeinfo = localtime( &timeval );

		if( strftime( buffer, sizeof( buffer), "%Y%m%d%H%M%S" , timeinfo ) )
		{
			NFDateString = buffer;
			return true;
		}

		return false;
	}

	bool ConvertNFDateString2time_t( const std::string & NFDateString, time_t & timeVal )
	{
		if( 0 == atoi( NFDateString.c_str() ) )
		{
			timeVal = InvalidTime_t;
			return true;
		}

		if( NFDateString.length() < 14 )
		{
			timeVal = InvalidTime_t;
			return false;
		}

		struct tm when;

		memset( &when, 0, sizeof(when) );

		std::string Token;

		when.tm_isdst = 0;

		Token = NFDateString.substr( 0, 4 );
		when.tm_year = atoi( Token.c_str() ) - 1900;

		Token = NFDateString.substr( 4, 2 );
		when.tm_mon = atoi( Token.c_str() ) - 1;

		Token = NFDateString.substr( 6, 2 );
		when.tm_mday = atoi( Token.c_str() );

		Token = NFDateString.substr( 8, 2 );
		when.tm_hour = atoi( Token.c_str() );

		Token = NFDateString.substr( 10, 2 );
		when.tm_min = atoi( Token.c_str() );

		Token = NFDateString.substr( 12, 2 );
		when.tm_sec = atoi( Token.c_str() );

		timeVal = mktime( &when );

		return true;
	}

	bool IsNULLField( const std::string& Value )
	{
		return Value.length() == 0;
	}
}//namespace achv

using namespace achv;

AchvDBManager::AchvDBManager(void)
{
}

AchvDBManager::~AchvDBManager(void)
{
}

BOOL AchvDBManager::DBInit( const std::string * iniFileName )
{
	std::string FileName;
	if( NULL == iniFileName )
		FileName = _T("NFAchv.ini");
	else
		FileName = *iniFileName;

	int nErrorCode = 0;
	if ( !::DBGWMInit( FileName.c_str(), &nErrorCode ) )
	{
//		AfxMessageBox(_T("DB와의 연결 실패!!!"));
		return FALSE;
	}

	return TRUE;
}

BOOL AchvDBManager::HasRow(string& strResult)
{
	// 성공한 경우만 아래 코드가 실행되야 한다.
	strResult.erase(0, 4);
	return strResult != ""; 
}

std::string AchvDBManager::GetParseData(std::string& strTarget)
{
	std::string strRet;
	size_t nIndex = strTarget.find_first_of("|");
	if ( nIndex != -1 )
	{
		strRet = strTarget.substr(0, nIndex);
		strTarget.erase( 0, nIndex + 1 );
	}
	return strRet;
}

BOOL AchvDBManager::SucceedQuery(DBGW_String& sResult, int& spResult,BOOL isSP)
{
	if( isSP == TRUE )
	{
		string tempString = (LPCSTR)sResult.GetData();
		BOOL ret = ( GetParseData(tempString) == "S" );
		if( ret == TRUE )
		{
			spResult = atoi(GetParseData(tempString).c_str());
		}
		else 
		{
			GetParseData(tempString); // error code
			spResult = atoi(GetParseData(tempString).c_str()); // sp result
		}
		return ret;
	}
	else
	{
		return ( strncmp((LPCSTR)sResult.GetData(), "S|0|", strlen("S|0|")) == 0 );
	}
}

BOOL AchvDBManager::Exec(const string& qry, DBGW_String& sResult, int& spResult, BOOL isSP)
{
	int nErrCode = 0;

	BOOL bRet = ::ExecuteQuery(QT_NORMAL, qry.c_str(), &sResult, &nErrCode);
	if (!bRet || !SucceedQuery(sResult, spResult, isSP)) 
	{
		return FALSE;
	}

	return TRUE;
}

BOOL AchvDBManager::ExecQry(const string& qry, string& sResult)
{
	DBGW_String dbResult;
	int tempInt = 0;
	BOOL ret = Exec(qry, dbResult, tempInt, FALSE);
	if( !ret )
	{
		return FALSE;
	}

	sResult = (LPCSTR)dbResult.GetData();
	return TRUE;
}

BOOL AchvDBManager::ExecSP(const string& qry, int& nResult, string& sResult)
{
	DBGW_String dbResult;
	BOOL ret = Exec(qry, dbResult, nResult, TRUE);
	if (!ret)
	{
		return FALSE;
	}

	sResult = (LPCSTR)dbResult.GetData();

	
	return (SP_SUCCESS_RESULT == nResult);
}

BOOL AchvDBManager::SelectAchvMeta( achv::TVecAchv & vecAchv )
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_ALL_NF_ACHV.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	std::string Buf;
	while(true)
	{
		achv::Achv_T AchvMetaData;

		// ACHV_ID, CATEGORY, START_DATE, END_DATE, NOTICE_CLASS, ACHV_TYPE, 
		// PROGRESS_TYPE, TRACE_TYPE, IS_HIDDEN, GOAL, RESERVE01

		AchvMetaData.achv_ID		= atoi(GetParseData(strResult).c_str());

		if( !IsNULLField( Buf = GetParseData(strResult ) )  )
			strncpy( &(AchvMetaData.category), ( Buf.c_str()), 1 );
		else
			strncpy( &(AchvMetaData.category), "0", 1 );

		ConvertNFDateString2time_t( GetParseData(strResult), AchvMetaData.start_date );
		ConvertNFDateString2time_t( GetParseData(strResult), AchvMetaData.end_date );

		AchvMetaData.notice_class	= atoi( ( GetParseData(strResult).c_str()) );
		AchvMetaData.achv_type		= achv::convertint2ACHV_TYPE( atoi( ( GetParseData(strResult).c_str() ) )  );

		if( !IsNULLField( Buf = GetParseData(strResult ) )  )
			AchvMetaData.progress_type	= achv::convertint2PROGRESS_TYPE( atoi( ( Buf.c_str() ) )  );
		else
			AchvMetaData.progress_type	= achv::PROGRESS_INVALID;

		if( !IsNULLField( Buf = GetParseData(strResult ) )  )
			AchvMetaData.trace_type		= achv::convertint2TRACE_TYPE( atoi( ( Buf.c_str() ) )  );
		else
			AchvMetaData.trace_type		= achv::TRACE_INVALID;

		if( !IsNULLField( Buf = GetParseData(strResult ) )  )
			AchvMetaData.is_hidden		= ( 0 == ( atoi( ( Buf.c_str() ) )  ) ? false : true );
		else
			AchvMetaData.is_hidden		= 0;

		if( !IsNULLField( Buf = GetParseData(strResult ) )  )
			AchvMetaData.goal			= atof( ( Buf.c_str() ) )  ;
		else
			AchvMetaData.goal			= 0.f;

		AchvMetaData.always_noti		= atoi( ( GetParseData(strResult).c_str()) );

		vecAchv.push_back( AchvMetaData );

		if(strResult.size() <= 0)
			break;
	}

	return TRUE;
}

BOOL AchvDBManager::SelectAchvReward( achv::TVecAchvReward & vecAchvReward )
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_ALL_NF_ACHV_REWARD.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	std::string Buf;
	while(true)
	{
		achv::Reward_T reward;

		// ACHV_ID, AP, MONEY, EXP, ITEM_CNT, ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4

		reward.achv_ID = atoi( GetParseData(strResult).c_str() );

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.AP = atoi( Buf.c_str() );
		else
			reward.AP = 0;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.money = ::_atoi64( Buf.c_str() );
		else
			reward.money = 0LL;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.exp = atoi( Buf.c_str() );
		else
			reward.exp = 0;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.item_cnt = atoi( Buf.c_str() );
		else
			reward.item_cnt = 0;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.item_ID1 = atoi( Buf.c_str() );
		else
			reward.item_ID1 = achv::InvalidRewardItemId;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.item_ID2 = atoi( Buf.c_str() );
		else
			reward.item_ID2 = achv::InvalidRewardItemId;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.item_ID3 = atoi( Buf.c_str() );
		else
			reward.item_ID3 = achv::InvalidRewardItemId;

		if( !IsNULLField( Buf = GetParseData(strResult) ) )
			reward.item_ID4 = atoi( Buf.c_str() );
		else
			reward.item_ID4 = achv::InvalidRewardItemId;

		vecAchvReward.push_back( reward );

		if(strResult.size() <= 0)
			break;
	}

	return TRUE;
}

BOOL AchvDBManager::SelectAchvRelative( achv::TVecAchvRelation & vecAchvRelation )
{
	std::string		strQuery;
	strQuery = format(QRY_SELECT_ALL_NF_ACHV_RELATIVE.c_str());

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret || !HasRow(strResult))
		return FALSE;

	while(true)
	{
		achv::Relation_T AchvRelation;

		// ACHV_ID, ACHV_ORDER, SUB_ACHVID

		AchvRelation.group_ID		= atoi(GetParseData(strResult).c_str());
		AchvRelation.order			= atoi(GetParseData(strResult).c_str());
		AchvRelation.achv_ID			= atoi(GetParseData(strResult).c_str());

		vecAchvRelation.push_back( AchvRelation );

		if(strResult.size() <= 0)
			break;
	}

	return TRUE;
}

BOOL AchvDBManager::SelectAchvProgress( achv::TVecProgress & vecAchvProgress, LONG GSN, LONG CSN )
{
	if( !vecAchvProgress.empty() )
		return FALSE;

	std::string		strQuery;
	strQuery = format(QRY_SELECT_NF_CHAR_ACHV.c_str(), GSN, CSN);

	std::string		strResult;
	BOOL ret = ExecQry(strQuery, strResult);
	if (!ret )	//Query Error
		return FALSE;

	if( !HasRow(strResult) )	//No Entry. i.e There is not achv in progress for this user.
		return TRUE;

	std::string Buf;

	while(true)
	{
		achv::Progress_T AchvProgressData;

		//ACHV_ID, PROGRESS, COMPLETE_DATE, GET_REWARD_DATE,
		//ITEM_ID1, ITEM_ID2, ITEM_ID3, ITEM_ID4, LAST_UPDATE_DATE, REMARK

		AchvProgressData.GSN = GSN;
		AchvProgressData.CSN = CSN;

		//ACHV_ID
		AchvProgressData.achv_ID		= atoi(GetParseData(strResult).c_str());

		//PROGRESS
		if( !IsNULLField( Buf = GetParseData(strResult ) )  )
			AchvProgressData.progress		= atof(Buf.c_str());
		else
			AchvProgressData.progress = 0.f;

		//COMPLETE_DATE
		ConvertNFDateString2time_t( GetParseData(strResult), AchvProgressData.complete_date );

		//REWARD_DATE
		ConvertNFDateString2time_t( GetParseData(strResult), AchvProgressData.get_reward_date );

		//ITEM_ID1 - ITEM_ID4

		for( int i = 0; i < achv::RewardItemIdCnt; ++i )
		{
			if( !IsNULLField( Buf = GetParseData(strResult ) )  )
				AchvProgressData.reward_item_ids[i]		= atoi(Buf.c_str());
			else
				AchvProgressData.reward_item_ids[i] = achv::InvalidRewardItemId;
		}


		//LAST_UPDATE_DATE
		ConvertNFDateString2time_t( GetParseData(strResult), AchvProgressData.last_update_date );

		//REMARK
		AchvProgressData.remark = GetParseData(strResult);

		vecAchvProgress.push_back( AchvProgressData );

		if(strResult.size() <= 0)
			break;
	}

	return TRUE;
}

BOOL AchvDBManager::UpdateAchvProgress( time_t & lastUpdateTime, const achv::Progress_T & AchvProgress, bool isCompleted )
{
	std::string		strQuery;
	strQuery = format(PKG_NF_ACHV_UPDATE_ACHV.c_str(), AchvProgress.GSN, AchvProgress.CSN, AchvProgress.achv_ID, AchvProgress.progress, isCompleted ? "Y" : "N" );

	int lErrorCode;
	std::string		strResult;
	BOOL ret = ExecSP(strQuery, lErrorCode, strResult);
	if (!ret || !HasRow(strResult))
		return lErrorCode;

	ConvertNFDateString2time_t( GetParseData(strResult).c_str(), lastUpdateTime );

	return TRUE;
}
