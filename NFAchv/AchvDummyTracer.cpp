#include "stdafx.h"
#include "AchvDummyTracer.h"


using namespace achv;


DummyTracer::DummyTracer(LONG GSN, LONG CSN, int achvid, const AchvMetaData& metaData)
:
Tracer(GSN, CSN, achvid, metaData )
{
	
}

DummyTracer::~DummyTracer()
{
}

achv::ProcessResult_T DummyTracer::Process(EventItem_T &eventItem, Progress_T &progressData)
{
	progressData = progressData_;
	return achv::PR_UNCHANGED;   // no progress change
}
