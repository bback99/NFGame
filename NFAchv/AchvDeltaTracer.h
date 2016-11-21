#ifndef __NGS_AchvDeltaTracer_H__
#define __NGS_AchvDeltaTracer_H__


#include <achv/AchvTracer.h>


namespace achv
{
	// Delta, Greater-than or Equal Tracer
	class DeltaTracer : public Tracer
	{
	public:
		explicit DeltaTracer(LONG GSN, LONG CSN, int archid, const AchvMetaData& metaData);
		~DeltaTracer();

		achv::ProcessResult_T Process(EventItem_T &eventItem, Progress_T &progressData);
	};

} // namespace achv


#endif//__NGS_AchvDeltaTracer_H__
