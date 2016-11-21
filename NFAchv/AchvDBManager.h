#pragma once

#include <DBGWManager.h>
#include <achv/AchvData.h>

namespace achv
{

	class AchvDBManager
	{
		const static int SP_SUCCESS_RESULT = 1;

		const static string QRY_SELECT_ALL_NF_ACHV;
		const static string QRY_SELECT_ALL_NF_ACHV_REWARD;
		const static string QRY_SELECT_ALL_NF_ACHV_RELATIVE;
		const static string QRY_SELECT_NF_CHAR_ACHV;
		const static string PKG_NF_ACHV_UPDATE_ACHV;

	public:
		AchvDBManager(void);
		virtual ~AchvDBManager(void);
		BOOL DBInit( const std::string * iniFileName );

		BOOL SelectAchvMeta( TVecAchv & vecAchv );
		BOOL SelectAchvReward( TVecAchvReward & vecAchvReward );
		BOOL SelectAchvProgress( TVecProgress & AchvProgress, LONG GSN, LONG CSN );
		BOOL SelectAchvRelative( TVecAchvRelation & vecAchvRelation );
		BOOL UpdateAchvProgress( time_t & lastUpdateTime, const achv::Progress_T & AchvProgress, bool isCompleted );

	private:

		BOOL HasRow(string& strResult);
		string GetParseData(std::string& sTarget);
		BOOL SucceedQuery(DBGW_String& sResult, int& spResult,BOOL isSP);
		BOOL Exec(const string& qry, DBGW_String& sResult,int& spResult, BOOL isSP); // function only for inner-use by ExecQry, ExecSP
		BOOL ExecQry(const string& qry, string& sResult);
		BOOL ExecSP(const string& qry, int& nResult, string& outSResult);
	};
}
