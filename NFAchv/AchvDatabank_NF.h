#ifndef __NGS_AchvDatabank_NF_H__
#define __NGS_AchvDatabank_NF_H__


#include <achv/AchvDatabank.h>
#include "AchvDBManager.h"
#include <vector>


namespace achv
{
	// Databank NF Implementation
	class Databank_NF : public Databank
	{
	public:
		Databank_NF();
		~Databank_NF();

		virtual ErrorCode init( bool initiailizeDBGWGR, const std::string * iniFileName )
		{
			if( initiailizeDBGWGR )
			{
				if ( FALSE == dbManager.DBInit( iniFileName ) )
					return EC_DBINIT_FAIL ;
			}
			ErrorCode ec = loadRawData();
			return (ec != EC_OK) ? ec : produceMapData();
		}

		ErrorCode loadRawData();

		ErrorCode getAchvList(TVecAchv &outref);
		ErrorCode getRewardList(TVecAchvReward &outref);
		ErrorCode getRelationList(TVecAchvRelation &outref);

		ErrorCode getProgressList(TVecProgress &outref, LONG GSN, LONG CSN);

		virtual ErrorCode ReqProgressUpdate(const Progress_T &progress, bool isCompleted, time_t *pLastUpdateDate = NULL);

	private:
		TVecAchv vecAchvMeta;
		TVecAchvReward vecAchvReward;
		TVecAchvRelation vecAchvRelation;

		AchvDBManager dbManager;
	};
} // namespace achv


#endif//__NGS_AchvDatabank_NF_H__
