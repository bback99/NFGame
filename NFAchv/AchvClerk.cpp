#include "stdafx.h"
#include <achv/AchvBureau.h>

namespace achv
{
	extern bool ConvertNFDateString2time_t( const std::string & NFDateString, time_t & timeVal );
}
using namespace achv;



/*static*/
Clerk* Clerk::Create(Bureau* pBureau, Databank* pDB, LONG GSN, LONG CSN)
{
	std::auto_ptr<Clerk> ptr(new Clerk(pBureau, pDB, GSN, CSN));
	return (ptr->init()) ? ptr.release() : NULL;
}

/*static*/
void Clerk::Destroy(Clerk *pClerk)
{
	if (pClerk != NULL)
		delete pClerk;
}

LONG Clerk::AddRef()
{
	return ::InterlockedIncrement(&refcnt_);
}

LONG Clerk::Release()
{
	LONG result = ::InterlockedDecrement(&refcnt_);
	if (result == 0)
		pBureau_->dismissClerk(CSN_);

	return result;
}

LONG Clerk::getGSN() const
{
	return GSN_;
}

LONG Clerk::getCSN() const
{
	return CSN_;
}

bool Clerk::report(enum Event evt, double val, const RoomID & rId )
{
	return report(EventItem_T(evt, val, rId ));
}

bool Clerk::report(const EventItem_T& eventItem)
{
	{
		GCSLOCK lock(&gcs_);
		eventList_.push_back(eventItem);
	}
	return (TRUE == ::XtpQueueWorkItem(pBureau_->getThreadPool(), Clerk::__process_event, (LPVOID)this));
}

bool Clerk::setReward(int achv_ID, const std::string & get_reward_date_str, const int reward_item_ids[RewardItemIdCnt])
{
	if (achv_ID == InvalidAchvId )
		return false;

	GCSLOCK lock(&gcs_);

	TracerMap_T::iterator it = tracerMap_.find(achv_ID);

	if (it == tracerMap_.end())
		return false;

	Progress_T* prog_ptr = it->second->GetProgress();

	time_t get_reward_date;
	ConvertNFDateString2time_t( get_reward_date_str, get_reward_date );

	if (prog_ptr->IsCompleted() == false || prog_ptr->get_reward_date != InvalidTime_t)
		return false;

	prog_ptr->get_reward_date = get_reward_date;
	prog_ptr->last_update_date = get_reward_date;
	std::copy(reward_item_ids, reward_item_ids + RewardItemIdCnt, prog_ptr->reward_item_ids);

	return true;
}

const Progress_T* Clerk::getProgress(int achv_ID) const
{
	GCSLOCK lock(&gcs_);

	TracerMap_T::const_iterator cit = tracerMap_.find(achv_ID);

	if (cit != tracerMap_.end())
		return cit->second->GetProgress();
	else
		return NULL;
}

bool Clerk::getProgressMap(ProgressMap_T &outref) const
{
	if (!outref.empty())
		return false;

	GCSLOCK lock(&gcs_);

	for (TracerMap_T::const_iterator cit = tracerMap_.begin(); cit != tracerMap_.end(); ++cit)
		outref.insert(std::make_pair(cit->first, cit->second->GetProgress()));

	return true;
}

Clerk::Clerk(Bureau* pBureau, Databank* pDB, LONG GSN, LONG CSN)
	: refcnt_(0), pBureau_(pBureau), pDB_(pDB), GSN_(GSN), CSN_(CSN)
{
	if (pBureau == NULL || pDB == NULL || GSN == 0 || CSN == 0)
		throw std::bad_alloc("Invalid parameter");
}

Clerk::~Clerk()
{
	for (TracerMap_T::iterator it = tracerMap_.begin(); it != tracerMap_.end(); ++it)
		delete it->second;
}

Tracer * Clerk::GetFirstIncompleteTracer( Tracer * pStart )
{
	Tracer * pRet = pStart ;
	while( pRet && pRet->IsCompleted() )
	{
		pRet = pRet->GetNextTracer();
	}

	return pRet;
}

bool Clerk::init()
{
	// populate tracer map
	const AchvMetaMap& metaMap = pDB_->getMetaMap();
	
	for (AchvMetaMap::const_iterator cit = metaMap.begin(); cit != metaMap.end(); ++cit)
	{
		Tracer *pTracer = Tracer::Create(GSN_, CSN_, cit->first, cit->second);
		tracerMap_.insert(std::make_pair(cit->first, pTracer));
	}

	const AchvSeqMap & seqMap = pDB_->getSeqMap();

	for( AchvSeqMap::const_iterator cit = seqMap.begin(); cit!= seqMap.end(); ++cit )
	{
		const std::vector< AchvSeqMetaData > & vecSeq = cit->second;
		if( vecSeq.empty() )
			continue;
		
		TracerMap_T::iterator tracerIt, NextTracerIt;
		for( size_t i = 0; i < vecSeq.size() - 1; ++i )
		{
			tracerIt = tracerMap_.find( vecSeq[i].GetAchvId() );
			NextTracerIt = tracerMap_.find( vecSeq[ i + 1 ].GetAchvId() );
			
			if( tracerIt != tracerMap_.end() &&  NextTracerIt != tracerMap_.end( ) )
				tracerIt->second->SetNextTracer( NextTracerIt->second );
		}

	}

	TVecProgress vecProgress;
	if( EC_OK == pDB_->getProgressList( vecProgress, getGSN(), getCSN() ) )
	{
		TracerMap_T::iterator tracerIt;
		for(TVecProgress::const_iterator cit = vecProgress.begin(); cit != vecProgress.end(); ++cit )
		{
			tracerIt = tracerMap_.find( cit->achv_ID );
			if( tracerIt != tracerMap_.end() )
			{
				tracerIt->second->SetProgress( *cit );
			}
		}
	}
	else
		return false;

	// load per-user progress data

	const EventAchvIDMap & evtAchvMap = pBureau_->GetEvtAchvIDMap();
	/*Making Event Map*/
	for( EventAchvIDMap::const_iterator cit = evtAchvMap.begin(); cit != evtAchvMap.end(); ++cit )
	{
		TracerMap_T::iterator itTracer = tracerMap_.find( cit->second );
		if( itTracer != tracerMap_.end() )
		{
			Tracer * pHeader = GetFirstIncompleteTracer( itTracer->second );
			eventMap_.insert( make_pair( cit->first, pHeader ) );
		}
	}

	return true;
}

///////////////////////////////////////
// LPTHREAD_START_ROUTINE for report()
//
/*static*/
DWORD WINAPI Clerk::__process_event(LPVOID param)
{
	reinterpret_cast<Clerk*>(param)->processEvent();

	return 0;
}

//void Clerk::processEvent()
//{
//	GCSLOCK lock(&gcs_);
//
//	if (eventList_.empty())
//		return;
//
//	EventItem_T eventItem = *(eventList_.begin());
//	eventList_.pop_front();
//
//	EventMap_T::iterator it = eventMap_.find(eventItem.evt);
//	if (it == eventMap_.end())   // tracer not found
//		return;
//
//	Tracer* pTracer = it->second;
//	Progress_T progress;
//	time_t lastUpdateDate = 0L;
//
//	while (pTracer)
//	{
//		Tracer::ProcessResult_T result = pTracer->Process(eventItem, progress);
//
//		if (result == Tracer::PR_UNCHANGED)
//			return;   // OK
//
//		bool isCompleted = (result == Tracer::PR_COMPLETED);
//
//		if (pDB_->ReqProgressUpdate(progress, isCompleted, &lastUpdateDate) != EC_OK)   // DB update failed!
//			return;   // TO-DO: error handling?
//
//		progress.last_update_date = lastUpdateDate;
//		if (isCompleted)
//			progress.complete_date = lastUpdateDate;
//
//		if (pTracer->SetProgress(progress) == false)   // set progress failed? Impossible.
//			return;   // TO-DO: error handling?
//
//		if (isCompleted || (result == Tracer::PR_CHANGED_NOTI))
//		{
//			pBureau_->runReportCallback(CSN_, pTracer->GetID(), &eventItem);
//			pTracer = it->second = pTracer->GetNextTracer();
//		}
//		else
//			pTracer = NULL;
//	}
//}

void Clerk::processEvent()
{
	GCSLOCK lock(&gcs_);

	if (eventList_.empty())
		return;

	EventItem_T eventItem = *(eventList_.begin());
	eventList_.pop_front();

	EventMap_T::iterator it = eventMap_.find(eventItem.evt);
	if (it == eventMap_.end())   // tracer not found
		return;

	Tracer* pTracer = it->second;
	Progress_T progress;
	time_t lastUpdateDate = 0L;

	while (pTracer)
	{
		achv::ProcessResult_T result = pTracer->Process(eventItem, progress);

		if (result == achv::PR_UNCHANGED)
			return;   // OK

		bool isCompleted = (result == achv::PR_COMPLETED);

		if (pDB_->ReqProgressUpdate(progress, isCompleted, &lastUpdateDate) != EC_OK)   // DB update failed!
			return;   // TO-DO: error handling?

		progress.last_update_date = lastUpdateDate;
		if (isCompleted)
			progress.complete_date = lastUpdateDate;

		if (pTracer->SetProgress(progress) == false)   // set progress failed? Impossible.
			return;   // TO-DO: error handling?

		if (isCompleted || (result == achv::PR_CHANGED_NOTI))
		{
			eventItem.result = result;
			pBureau_->runReportCallback(GSN_, CSN_, pTracer->GetID(), &eventItem);

			if (isCompleted)
				pTracer = it->second = pTracer->GetNextTracer();
			else
				pTracer = NULL;
		}
		else
			pTracer = NULL;
	}
}
