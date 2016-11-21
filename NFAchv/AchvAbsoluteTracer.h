#ifndef __NGS_AchvAbsoluteTracer_H__
#define __NGS_AchvAbsoluteTracer_H__


#include <achv/AchvTracer.h>


namespace achv
{
	// Absolute, Greater-than or Equal Tracer
	class AbsoluteTracer : public Tracer
	{
	public:
		explicit AbsoluteTracer(LONG GSN, LONG CSN, int archid, const AchvMetaData& metaData);
		~AbsoluteTracer();

		achv::ProcessResult_T Process(EventItem_T &eventItem, Progress_T &progressData);
	};

} // namespace achv


#endif//__NGS_AchvAbsoluteTracer_H__
