#include "stdafx.h"
#include "AchvDatabank_NF.h"
//#include "NFDBManager.h"


using namespace achv;

Databank_NF::Databank_NF()
{

}

Databank_NF::~Databank_NF()
{

}

ErrorCode Databank_NF::loadRawData()
{

	if( FALSE == dbManager.SelectAchvMeta( vecAchvMeta ) )
	{
		return EC_FAIL;
	}
	
	if( FALSE == dbManager.SelectAchvReward( vecAchvReward ) )
	{
		return EC_FAIL;
	}
	
	if( FALSE == dbManager.SelectAchvRelative( vecAchvRelation ) )	
	{
		return EC_FAIL;
	}

	return EC_OK;
}

ErrorCode Databank_NF::getAchvList(TVecAchv &outref)
{
	outref = vecAchvMeta;
	return EC_OK;
}

ErrorCode Databank_NF::getRewardList(TVecAchvReward &outref)
{
	outref = vecAchvReward;
	return EC_OK;
}

ErrorCode Databank_NF::getRelationList(TVecAchvRelation &outref)
{
	outref = vecAchvRelation;
	return EC_OK;

}

ErrorCode Databank_NF::getProgressList(std::vector<Progress_T> &outref, LONG GSN, LONG CSN)
{
	if( FALSE == dbManager.SelectAchvProgress( outref, GSN, CSN ) )
		return EC_FAIL;
	return EC_OK;
}

ErrorCode Databank_NF::ReqProgressUpdate(const Progress_T &progress, bool isCompleted, time_t *pLastUpdateDate /*= NULL*/)
{
	time_t lastUpdateTime;

	if( FALSE == dbManager.UpdateAchvProgress( lastUpdateTime, progress, isCompleted) )
		return EC_FAIL;
	
	if (pLastUpdateDate != NULL)
		*pLastUpdateDate = lastUpdateTime;

	return EC_OK;
}
