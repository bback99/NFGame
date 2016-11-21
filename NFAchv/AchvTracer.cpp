#include "stdafx.h"
#include <achv/AchvTracer.h>
#include "AchvAbsoluteTracer.h"
#include "AchvDeltaTracer.h"
#include "AchvDummyTracer.h"


using namespace achv;

/*static*/
Tracer* Tracer::Create(LONG GSN, LONG CSN, int achvid, const AchvMetaData& metaData )
{
	switch (metaData.GetProgressType())
	{
	case PROGRESS_ABSOLUTE:
		return new AbsoluteTracer(GSN, CSN, achvid, metaData );
	case PROGRESS_DELTA:
		return new DeltaTracer(GSN, CSN, achvid, metaData);
	case PROGRESS_RECORD_HIGH:
		// ...
	case PROGRESS_RECORD_LOW:
		// ...
	case PROGRESS_ETC:
		// ...
	default:
		return new DummyTracer(GSN, CSN, achvid, metaData);
	}
}

/*virtual*/
Tracer::~Tracer()
{
}

Progress_T* Tracer::GetProgress()
{
	return &progressData_;
}

bool Tracer::SetProgress(const Progress_T& progressData)
{
	if (progressData_.GSN == progressData.GSN &&
		progressData_.CSN == progressData.CSN &&
		progressData_.achv_ID == progressData.achv_ID)
	{
		progressData_ = progressData;
		return true;
	}

	return false;
}

Tracer::Tracer(LONG GSN, LONG CSN, int achvid, const AchvMetaData& metaData) : NextTracer(NULL), goal_(metaData.GetGoal()), alwaysnoti_(metaData.AlwaysNoti()), achvtype_(metaData.GetType())
{
	progressData_.GSN = GSN;
	progressData_.CSN = CSN;
	progressData_.achv_ID = achvid;
}
