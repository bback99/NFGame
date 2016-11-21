#include "stdafx.h"
#include <achv/AchvConst.h>

namespace achv
{
	int convertACHV_TYPE2int( ACHV_TYPE type )
	{
		switch( type )
		{
		case ACHV_INVALID:
			return -1;
			break;
		case ACHV_NORMAL:
			return 0;
			break;
		case ACHV_COMPOSITE:
			return 1;
			break;
		case ACHV_SEQUENCE:
			return 2;
			break;
		default:
			ASSERT(FALSE);
			return -1;
			break;
		}
	}

	ACHV_TYPE convertint2ACHV_TYPE( int val )
	{
		switch( val )
		{
		case -1:
			return ACHV_INVALID;
			break;
		case 0:
			return ACHV_NORMAL;
			break;
		case 1:
			return ACHV_COMPOSITE;
			break;
		case 2:
			return ACHV_SEQUENCE;
			break;
		default:
			ASSERT(FALSE);
			return ACHV_INVALID;
			break;
		}
	}

	int convertPROGRESS_TYPE2int( PROGRESS_TYPE type )
	{
		switch( type )
		{
		case PROGRESS_INVALID:
			return -1;
			break;
		case PROGRESS_ABSOLUTE:
			return 0;
			break;
		case PROGRESS_DELTA:
			return 1;
			break;
		case PROGRESS_RECORD_HIGH:
			return 2;
			break;
		case PROGRESS_RECORD_LOW:
			return 3;
			break;
		case PROGRESS_ETC:
			return 4;
			break;
		default:
			ASSERT(FALSE);
			return -1;
			break;
		}
	}

	PROGRESS_TYPE convertint2PROGRESS_TYPE( int val )
	{
		switch( val )
		{
		case -1:
			return PROGRESS_INVALID;
			break;
		case 0:
			return PROGRESS_ABSOLUTE;
			break;
		case 1:
			return PROGRESS_DELTA;
			break;
		case 2:
			return PROGRESS_RECORD_HIGH;
			break;
		case 3:
			return PROGRESS_RECORD_LOW;
			break;
		case 4:
			return PROGRESS_ETC;
			break;
		default:
			ASSERT(FALSE);
			return PROGRESS_INVALID;
			break;
		}
	}

	int convertTRACE_TYPE2int( TRACE_TYPE type )
	{
		switch( type )
		{
		case TRACE_INVALID:
			return -1;
			break;
		case TRACE_ACHV:
			return 0;
			break;
		case TRACE_GAME:
			return 1;
			break;
		case TRACE_STAT:
			return 2;
			break;
		default:
			ASSERT(FALSE);
			return -1;
			break;
		}
	}

	TRACE_TYPE convertint2TRACE_TYPE( int val )
	{
		switch( val )
		{
		case -1:
			return TRACE_INVALID;
			break;
		case 0:
			return TRACE_ACHV;
			break;
		case 1:
			return TRACE_GAME;
			break;
		case 2:
			return TRACE_STAT;
			break;
		default:
			ASSERT(FALSE);
			return TRACE_INVALID;
			break;
		}

}
}