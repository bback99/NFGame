#ifndef __NGS_AchvDummyTracer_H__
#define __NGS_AchvDummyTracer_H__


#include <achv/AchvTracer.h>


namespace achv
{
	// Dummy Tracer
	class DummyTracer : public Tracer
	{
	public:
		explicit DummyTracer(LONG GSN, LONG CSN, int archid, const AchvMetaData& metaData);
		~DummyTracer();

		achv::ProcessResult_T Process(EventItem_T &eventItem, Progress_T &progressData);
	};

} // namespace achv


#endif//__NGS_AchvDummyTracer_H__
