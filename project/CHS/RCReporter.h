//
// RCReporter.h
//

#ifndef RCReporter_h
#define RCReporter_h

///////////////////////////////////////////////////////////////////////////////////
// CRCReporter

class CRCReporter : public IXObject
{
	IMPLEMENT_TISAFEREFCNT(CRCReporter)
private:
	CRCReporter(const CRCReporter&);
public:
	typedef ArcListT<DWORD> ListUSNT;
	typedef vector<DWORD> VecUSN;
	CRCReporter();
	virtual ~CRCReporter();
public:
	virtual BOOL RunCRC();
	virtual BOOL StopCRC();

	// inherited
protected:
	STDMETHOD_(void,OnSignal)(HSIGNAL hSignal, WPARAM wParam, LPARAM lParam);//BOOL bFired);			// TIMER..

	// implementation
protected:
	//Channel에서 RChannelListReq를 받았을 경우...
	BOOL OnCRCInfoReq(LONG lUSN);
	
	//LRB에서 RChannelListAns를 받았을 경우 
	void GetSendUSNList(vector<LONG>& vecUSN);

	void RemCRCUSN(LONG lUSN);
protected:
	GXSigTimer m_timerCRC;	

	BOOL m_bWaitRCList;
	LONG m_lTimerCount;
	ListUSNT m_lstUSN;
	VecUSN m_vecUSNQ;

	friend class CChannel;
	friend class CChannelContext;
};


#endif //!Reporter_h
