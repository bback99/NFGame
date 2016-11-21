#include "stdafx.h"
#include <achv/AchvBureau.h>
#include <achv/AchvDatabank.h>
#include <achv/AchvDef.h>


using namespace achv;


typedef std::map<std::string, Bureau*> BureauMap_T;

/*static*/
PfnLogGetter_T Bureau::pfnLogGetter_ = &GetTheLog;   // @ ServerLog.h

/*static*/
bool Bureau::setLogGetter(PfnLogGetter_T pfnLogGetter)
{
	if (pfnLogGetter == NULL)
		return false;

	pfnLogGetter_ = pfnLogGetter;
	return true;
}

/*static*/
PfnLogGetter_T Bureau::getLogGetter()
{
	return pfnLogGetter_;
}

/*static*/
Bureau* Bureau::instance(const char* name)
{
	static GCRITICAL_SECTION gcs;
	static BureauMap_T bmap;

	const char* noname = "";
	if (name == NULL)
		name = noname;

	GCSLOCK lock(&gcs);

	BureauMap_T::iterator it = bmap.find(name);

	if (it != bmap.end())
		return it->second;

	std::auto_ptr<Bureau> pBureau(new Bureau(name));
	std::pair<BureauMap_T::iterator, bool> result = bmap.insert(std::make_pair(name, pBureau.get()));
	if (result.second == true)
		return pBureau.release();

	return NULL;
}

const char* Bureau::name() const
{
	return name_.c_str();
}

bool Bureau::init(size_t num_thread /*= DEFAULT_NUM_THREAD*/, bool initiailizeDBGWGR, const std::string * pDBGWMGRIniFileName )
{
	if (hPool_ != NULL)
	{
		iLog().Put(WAR_UK, "[achv] Bureau::init() - init'ed already");
		return false;
	}

	hPool_ = ::XtpCreatePool(num_thread);
	if (hPool_ == NULL)
	{
		iLog().Put(ERR_UK, "[achv] Bureau::init() - XtpCreatePool() failed!");
		return false;
	}

	isPoolAutoCreated_ = true;
	return init(hPool_, initiailizeDBGWGR,  pDBGWMGRIniFileName);
}

bool Bureau::init(HTHREADPOOL hPool, bool initiailizeDBGWGR, const std::string * pDBGWMGRIniFileName )
{
	if (hPool == NULL)
		return false;

	if (hPool_ == NULL)
	{
		hPool_ = hPool;
	}
	else if (hPool_ != hPool)   // init again? -_-
	{
		iLog().Put(WAR_UK, "[achv] Bureau::init() - init'ed already");
		return false;
	}

	iLog().Put(INF_UK, "[achv] Bureau::init() - accessing DB...");

	db_.reset(Databank::Create("NF"));

	if (db_.get() == NULL)
	{
		iLog().Put(ERR_UK, "[achv] Bureau::init() - DB access failed!");
		return false;
	}

	if (db_->init( initiailizeDBGWGR, pDBGWMGRIniFileName ) != EC_OK)
	{
		iLog().Put(ERR_UK, "[achv] Bureau::init() - DBGWMGR init failed!");
		return false;
	}

	/* init EvtID-AchvID map*/

	return true;
}

bool Bureau::addEvenAchvMapping( Event evt, int HeadAchvId )
{
	GCSLOCK lock(&gcsBureau_);

	bool ret = (eventMap_.insert( std::make_pair( evt, HeadAchvId ) )).second;

	if (ret == false)
		iLog().Put(ERR_UK, "[achv] Bureau::addEvenAchvMapping() failed!");

	return ret;
}

bool Bureau::addReportCallback(int notice_bits, ReportCallback_T cbReport)
{
	if (notice_bits == 0 || cbReport == NULL)
		return false;

	GCSLOCK lock(&gcsBureau_);

	cbReportVector_.push_back(std::make_pair(notice_bits, cbReport));
	return true;
}

bool Bureau::login(LONG GSN, LONG CSN)
{
	iLog().Put(INF_UK, "[achv] Bureau::login() - CSN: ", CSN);

	GCSLOCK lock(&gcsClerkMap_);

	return g_kAchvDef.LoadCharAchvState(GSN, CSN);

	//// Is this CSN in the logout map?
	//LogoutMap_T::iterator it = logoutMap_.find(CSN);

	//if (it != logoutMap_.end())
	//{
	//	// resurrect it!
	//	cptr = it->second;
	//	activeMap_.insert(std::make_pair(CSN, cptr));
	//	logoutMap_.erase(it);

	//	iLog().Put(INF_UK, "[achv] Bureau::login() - CSN: ", CSN, " found @ logoutMap_[] and resurrected.");
	//}
	//else
	//{
	//	// Is this CSN in the active map?
	//	ActiveMap_T::iterator ait = activeMap_.find(CSN);

	//	if (ait != activeMap_.end())
	//	{
	//		cptr = ait->second;

	//		iLog().Put(INF_UK, "[achv] Bureau::login() - CSN: ", CSN, " found @ activeMap_[] and add-ref'ed.");
	//	}
	//	else
	//	{
	//		iLog().Put(INF_UK, "[achv] Bureau::login() - CSN: ", CSN, " not found; going to create one...");

	//		// create one!
	//		cptr = Clerk::Create(this, db_.get(), GSN, CSN);
	//		if (cptr.m_ptr != NULL)
	//		{
	//			activeMap_.insert(std::make_pair(CSN, cptr));

	//			iLog().Put(INF_UK, "[achv] Bureau::login() - CSN: ", CSN, " Clerk created.");
	//		}
	//		else
	//		{
	//			iLog().Put(ERR_UK, "[achv] Bureau::login() - CSN: ", CSN, " Clerk::Create() failed!");
	//		}
	//	}
	//}

	//return cptr;   // cptr has the Clerk for the CSN now
}

bool Bureau::logout(LONG CSN)
{
	iLog().Put(INF_UK, "[achv] Bureau::logout() - CSN: ", CSN);

	ClerkPtr cptr;

	// limited lock scope
	{
		GCSLOCK lock(&gcsClerkMap_);

		// Is this CSN in the active map?
		ActiveMap_T::iterator it = activeMap_.find(CSN);
		if (it == activeMap_.end())
		{
			iLog().Put(WAR_UK, "[achv] Bureau::logout() - CSN: ", CSN, " not found @ activeMap_[].");

			return false;
		}

		cptr = it->second;   // delays refcnt -> 0 until out of the lock scope

		// move it to logout map
		logoutMap_.insert(std::make_pair(CSN, (Clerk*)(it->second)));
		activeMap_.erase(it);

		iLog().Put(INF_UK, "[achv] Bureau::logout() - CSN: ", CSN, " moved from activeMap_[] to logoutMap_[].");
	}

	return true;
}

ClerkPtr Bureau::getClerk(LONG CSN) const
{
	GCSLOCK lock(&gcsClerkMap_);

	ActiveMap_T::const_iterator cit = activeMap_.find(CSN);

	if (cit != activeMap_.end())
		return cit->second;
	
	iLog().Put(ERR_UK, "[achv] Bureau::getClerk() - CSN: ", CSN, " not found @ activeMap_[]; will return NULL.");
	return ClerkPtr();   // NULL ptr
}

Bureau::Bureau(const char* name) : name_(name), hPool_(NULL), isPoolAutoCreated_(false), db_(NULL)
{
}

Bureau::~Bureau()
{

	if (isPoolAutoCreated_ && hPool_ != NULL)
		::XtpDeletePool(hPool_);
}

HTHREADPOOL Bureau::getThreadPool()
{
	return hPool_;
}

const EventAchvIDMap & Bureau::GetEvtAchvIDMap()
{
	GCSLOCK lock(&gcsBureau_);

	return eventMap_;
}

bool Bureau::GetAchvInfo(int achv_id, Achv_T& achvInfo)
{
	TVecAchv	vecAchv;
	achv::ErrorCode err = db_->getAchvList(vecAchv);
	if (err != EC_OK)
		return false;

	ForEachElmt(TVecAchv, vecAchv, it, ij)
	{
		if (achv_id == (*it).achv_ID) {
			achvInfo = (*it);
			return true;
		}
	}
	return false;
}

bool Bureau::dismissClerk(LONG CSN)
{
	GCSLOCK lock(&gcsClerkMap_);

	LogoutMap_T::iterator it = logoutMap_.find(CSN);

	if (it == logoutMap_.end())
		return false;

	Clerk* pClerk = it->second;
	logoutMap_.erase(it);
	Clerk::Destroy(pClerk);

	return true;
}

bool Bureau::runReportCallback(LONG GSN, LONG CSN, int achv_ID, const EventItem_T *pEvtItem)
{
	GCSLOCK lock(&gcsBureau_);

	const AchvMetaMap &map = db_->getMetaMap();
	AchvMetaMap::const_iterator cit = map.find(achv_ID);
	if (cit == map.end())
		return false;

	int noticeClass = cit->second.GetNoticeClass();
	for (size_t i = 0; i < cbReportVector_.size(); ++i)
	{
		if (noticeClass & cbReportVector_[i].first)
			(*(cbReportVector_[i].second))(GSN, CSN, achv_ID, pEvtItem);
	}
	return true;
}
