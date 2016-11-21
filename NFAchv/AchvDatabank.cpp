#include "stdafx.h"
#include <achv/AchvDatabank.h>
#include "AchvDatabank_NF.h"


using namespace achv;


/*static*/
Databank* Databank::Create(const char* name)
{
	if (name == NULL)
		return NULL;

	std::string sname = name;

	if (sname == "NF")
		return new Databank_NF();
	else
		return NULL;
}

/*virtual*/
Databank::~Databank()
{
}

/*private*/
ErrorCode Databank::produceMapData()
{
	TVecAchv AchvDataInDB;
	if( EC_OK != getAchvList( AchvDataInDB ) )
	{
		return EC_FAIL;
	}

	for( std::vector< Achv_T >::const_iterator it = AchvDataInDB.begin(); it != AchvDataInDB.end(); ++it )
	{
		AchvMetaData metaData( (*it) );
		metaMap_.insert( std::make_pair( (*it).achv_ID, metaData ) );
	}

	TVecAchvReward AchvRewardsInDB;
	if( EC_OK != getRewardList( AchvRewardsInDB ) )
	{
		return EC_FAIL;
	}

	for( size_t i = 0; i < AchvRewardsInDB.size(); ++i )
	{
		AchvMetaMap::iterator mit = metaMap_.find( AchvRewardsInDB[i].achv_ID );
		if( mit == metaMap_.end() )
		{
			return EC_FAIL;
		}
		mit->second.SetRewardIndex(i);
	}

	TVecAchvRelation  AchvRelationsInDB;
	if( EC_OK != getRelationList( AchvRelationsInDB ) )
	{
		return EC_FAIL;
	}

	for( std::vector<Relation_T>::const_iterator it = AchvRelationsInDB.begin(); it != AchvRelationsInDB.end(); ++it )
	{
		AchvSeqMetaData SeqMetaData( (*it) );
		seqMap_[ (*it).group_ID ].push_back( SeqMetaData );
	}

	for( AchvSeqMap::iterator it = seqMap_.begin(); it != seqMap_.end(); ++it )
	{
		std::vector<AchvSeqMetaData> &vSeq = it->second;
		std::sort(vSeq.begin(), vSeq.end());
		for (size_t i = 0; i < vSeq.size() - 1; ++i)
		{
			AchvMetaMap::iterator PrevAchvIt = metaMap_.find( vSeq[i].GetAchvId() );
			if( PrevAchvIt == metaMap_.end() )
			{
//				theLog.Put("There is not achv mached for previous achv in seqData" );
				return EC_FAIL;
			}
			(PrevAchvIt->second).SetFollowingAchvId(vSeq[i + 1].GetAchvId());
		}
	}

	return EC_OK;
}
