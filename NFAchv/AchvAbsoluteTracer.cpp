#include "stdafx.h"
#include "AchvAbsoluteTracer.h"


using namespace achv;


AbsoluteTracer::AbsoluteTracer(LONG GSN, LONG CSN, int achvid, const AchvMetaData& metaData)
:
Tracer(GSN, CSN, achvid, metaData )
{
}

AbsoluteTracer::~AbsoluteTracer()
{
}

achv::ProcessResult_T AbsoluteTracer::Process(EventItem_T &eventItem, Progress_T &progressData)
{
	if (progressData_.IsCompleted()) 
		return achv::PR_UNCHANGED;

	progressData = progressData_;

	if (eventItem.val < goal_) {

		if (eventItem.val >= alwaysnoti_)
			return achv::PR_CHANGED_NOTI;

		return achv::PR_UNCHANGED;
	}

	progressData.progress = goal_;
	return achv::PR_COMPLETED;
}