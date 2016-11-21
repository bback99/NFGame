#include "stdafx.h"
#include "AchvDeltaTracer.h"


using namespace achv;


DeltaTracer::DeltaTracer(LONG GSN, LONG CSN, int achvid, const AchvMetaData& metaData)
:
Tracer(GSN, CSN, achvid, metaData )
{
}

DeltaTracer::~DeltaTracer()
{
}

achv::ProcessResult_T DeltaTracer::Process(EventItem_T &eventItem, Progress_T &progressData)
{
	ProcessResult_T result = achv::PR_UNCHANGED;

	if (progressData_.IsCompleted() || eventItem.val == 0.0f)
		return achv::PR_UNCHANGED;
	else
	{
		progressData = progressData_;
		eventItem.val += progressData_.progress;

		if (eventItem.val < goal_) 
		{
			progressData.progress = eventItem.val;

			if (eventItem.val >= alwaysnoti_)
				result = achv::PR_CHANGED_NOTI;
			else
				result = achv::PR_CHANGED;
		}
		else
		{
			progressData.progress = goal_;
			result = achv::PR_COMPLETED;
		}
	}
	return result;
}
